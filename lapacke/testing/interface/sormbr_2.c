/*****************************************************************************
  Copyright (c) 2010, Intel Corp.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
  THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/
/*  Contents: test routine for C interface to LAPACK
*   Author: Intel Corporation
*   Created in March, 2010
*
* Purpose
*
* sormbr_2 is the test program for the C interface to LAPACK
* routine sormbr
* The program doesn't require an input, the input data is hardcoded in the
* test program.
* The program tests the C interface in the four combinations:
*   1) column-major layout, middle-level interface
*   2) column-major layout, high-level interface
*   3) row-major layout, middle-level interface
*   4) row-major layout, high-level interface
* The output of the C interface function is compared to those obtained from
* the corresponiding LAPACK routine with the same input data, and the
* comparison diagnostics is then printed on the standard output having PASSED
* keyword if the test is passed, and FAILED keyword if the test isn't passed.
*****************************************************************************/
#include <stdio.h>
#include "lapacke.h"
#include "lapacke_utils.h"
#include "test_utils.h"

static void init_scalars_sormbr( char *vect, char *side, char *trans,
                                 lapack_int *m, lapack_int *n, lapack_int *k,
                                 lapack_int *lda, lapack_int *ldc,
                                 lapack_int *lwork );
static void init_a( lapack_int size, float *a );
static void init_tau( lapack_int size, float *tau );
static void init_c( lapack_int size, float *c );
static void init_work( lapack_int size, float *work );
static int compare_sormbr( float *c, float *c_i, lapack_int info,
                           lapack_int info_i, lapack_int ldc, lapack_int n );

int main(void)
{
    /* Local scalars */
    char vect, vect_i;
    char side, side_i;
    char trans, trans_i;
    lapack_int m, m_i;
    lapack_int n, n_i;
    lapack_int k, k_i;
    lapack_int lda, lda_i;
    lapack_int lda_r;
    lapack_int ldc, ldc_i;
    lapack_int ldc_r;
    lapack_int lwork, lwork_i;
    lapack_int info, info_i;
    /* Declare scalars */
    lapack_int nq;
    lapack_int r;
    lapack_int i;
    int failed;

    /* Local arrays */
    float *a = NULL, *a_i = NULL;
    float *tau = NULL, *tau_i = NULL;
    float *c = NULL, *c_i = NULL;
    float *work = NULL, *work_i = NULL;
    float *c_save = NULL;
    float *a_r = NULL;
    float *c_r = NULL;

    /* Iniitialize the scalar parameters */
    init_scalars_sormbr( &vect, &side, &trans, &m, &n, &k, &lda, &ldc, &lwork );
    nq = LAPACKE_lsame( side, 'l' ) ? m : n;
    r = LAPACKE_lsame( vect, 'q' ) ? nq : MIN(nq,k);
    lda_r = MIN(nq,k)+2;
    ldc_r = n+2;
    vect_i = vect;
    side_i = side;
    trans_i = trans;
    m_i = m;
    n_i = n;
    k_i = k;
    lda_i = lda;
    ldc_i = ldc;
    lwork_i = lwork;

    /* Allocate memory for the LAPACK routine arrays */
    a = (float *)LAPACKE_malloc( (lda*(MIN(nq,k))) * sizeof(float) );
    tau = (float *)LAPACKE_malloc( MIN(nq,k) * sizeof(float) );
    c = (float *)LAPACKE_malloc( ldc*n * sizeof(float) );
    work = (float *)LAPACKE_malloc( lwork * sizeof(float) );

    /* Allocate memory for the C interface function arrays */
    a_i = (float *)LAPACKE_malloc( (lda*(MIN(nq,k))) * sizeof(float) );
    tau_i = (float *)LAPACKE_malloc( MIN(nq,k) * sizeof(float) );
    c_i = (float *)LAPACKE_malloc( ldc*n * sizeof(float) );
    work_i = (float *)LAPACKE_malloc( lwork * sizeof(float) );

    /* Allocate memory for the backup arrays */
    c_save = (float *)LAPACKE_malloc( ldc*n * sizeof(float) );

    /* Allocate memory for the row-major arrays */
    a_r = (float *)LAPACKE_malloc( (r*(MIN(nq,k)+2)) * sizeof(float) );
    c_r = (float *)LAPACKE_malloc( m*(n+2) * sizeof(float) );

    /* Initialize input arrays */
    init_a( lda*(MIN(nq,k)), a );
    init_tau( (MIN(nq,k)), tau );
    init_c( ldc*n, c );
    init_work( lwork, work );

    /* Backup the ouptut arrays */
    for( i = 0; i < ldc*n; i++ ) {
        c_save[i] = c[i];
    }

    /* Call the LAPACK routine */
    sormbr_( &vect, &side, &trans, &m, &n, &k, a, &lda, tau, c, &ldc, work,
             &lwork, &info );

    /* Initialize input data, call the column-major middle-level
     * interface to LAPACK routine and check the results */
    for( i = 0; i < lda*(MIN(nq,k)); i++ ) {
        a_i[i] = a[i];
    }
    for( i = 0; i < (MIN(nq,k)); i++ ) {
        tau_i[i] = tau[i];
    }
    for( i = 0; i < ldc*n; i++ ) {
        c_i[i] = c_save[i];
    }
    for( i = 0; i < lwork; i++ ) {
        work_i[i] = work[i];
    }
    info_i = LAPACKE_sormbr_work( LAPACK_COL_MAJOR, vect_i, side_i, trans_i,
                                  m_i, n_i, k_i, a_i, lda_i, tau_i, c_i, ldc_i,
                                  work_i, lwork_i );

    failed = compare_sormbr( c, c_i, info, info_i, ldc, n );
    if( failed == 0 ) {
        printf( "PASSED: column-major middle-level interface to sormbr\n" );
    } else {
        printf( "FAILED: column-major middle-level interface to sormbr\n" );
    }

    /* Initialize input data, call the column-major high-level
     * interface to LAPACK routine and check the results */
    for( i = 0; i < lda*(MIN(nq,k)); i++ ) {
        a_i[i] = a[i];
    }
    for( i = 0; i < (MIN(nq,k)); i++ ) {
        tau_i[i] = tau[i];
    }
    for( i = 0; i < ldc*n; i++ ) {
        c_i[i] = c_save[i];
    }
    for( i = 0; i < lwork; i++ ) {
        work_i[i] = work[i];
    }
    info_i = LAPACKE_sormbr( LAPACK_COL_MAJOR, vect_i, side_i, trans_i, m_i,
                             n_i, k_i, a_i, lda_i, tau_i, c_i, ldc_i );

    failed = compare_sormbr( c, c_i, info, info_i, ldc, n );
    if( failed == 0 ) {
        printf( "PASSED: column-major high-level interface to sormbr\n" );
    } else {
        printf( "FAILED: column-major high-level interface to sormbr\n" );
    }

    /* Initialize input data, call the row-major middle-level
     * interface to LAPACK routine and check the results */
    for( i = 0; i < lda*(MIN(nq,k)); i++ ) {
        a_i[i] = a[i];
    }
    for( i = 0; i < (MIN(nq,k)); i++ ) {
        tau_i[i] = tau[i];
    }
    for( i = 0; i < ldc*n; i++ ) {
        c_i[i] = c_save[i];
    }
    for( i = 0; i < lwork; i++ ) {
        work_i[i] = work[i];
    }

    LAPACKE_sge_trans( LAPACK_COL_MAJOR, r, MIN(nq, k ), a_i, lda, a_r, MIN(nq,
                       k)+2);
    LAPACKE_sge_trans( LAPACK_COL_MAJOR, m, n, c_i, ldc, c_r, n+2 );
    info_i = LAPACKE_sormbr_work( LAPACK_ROW_MAJOR, vect_i, side_i, trans_i,
                                  m_i, n_i, k_i, a_r, lda_r, tau_i, c_r, ldc_r,
                                  work_i, lwork_i );

    LAPACKE_sge_trans( LAPACK_ROW_MAJOR, m, n, c_r, n+2, c_i, ldc );

    failed = compare_sormbr( c, c_i, info, info_i, ldc, n );
    if( failed == 0 ) {
        printf( "PASSED: row-major middle-level interface to sormbr\n" );
    } else {
        printf( "FAILED: row-major middle-level interface to sormbr\n" );
    }

    /* Initialize input data, call the row-major high-level
     * interface to LAPACK routine and check the results */
    for( i = 0; i < lda*(MIN(nq,k)); i++ ) {
        a_i[i] = a[i];
    }
    for( i = 0; i < (MIN(nq,k)); i++ ) {
        tau_i[i] = tau[i];
    }
    for( i = 0; i < ldc*n; i++ ) {
        c_i[i] = c_save[i];
    }
    for( i = 0; i < lwork; i++ ) {
        work_i[i] = work[i];
    }

    /* Init row_major arrays */
    LAPACKE_sge_trans( LAPACK_COL_MAJOR, r, MIN(nq, k ), a_i, lda, a_r, MIN(nq,
                       k)+2);
    LAPACKE_sge_trans( LAPACK_COL_MAJOR, m, n, c_i, ldc, c_r, n+2 );
    info_i = LAPACKE_sormbr( LAPACK_ROW_MAJOR, vect_i, side_i, trans_i, m_i,
                             n_i, k_i, a_r, lda_r, tau_i, c_r, ldc_r );

    LAPACKE_sge_trans( LAPACK_ROW_MAJOR, m, n, c_r, n+2, c_i, ldc );

    failed = compare_sormbr( c, c_i, info, info_i, ldc, n );
    if( failed == 0 ) {
        printf( "PASSED: row-major high-level interface to sormbr\n" );
    } else {
        printf( "FAILED: row-major high-level interface to sormbr\n" );
    }

    /* Release memory */
    if( a != NULL ) {
        LAPACKE_free( a );
    }
    if( a_i != NULL ) {
        LAPACKE_free( a_i );
    }
    if( a_r != NULL ) {
        LAPACKE_free( a_r );
    }
    if( tau != NULL ) {
        LAPACKE_free( tau );
    }
    if( tau_i != NULL ) {
        LAPACKE_free( tau_i );
    }
    if( c != NULL ) {
        LAPACKE_free( c );
    }
    if( c_i != NULL ) {
        LAPACKE_free( c_i );
    }
    if( c_r != NULL ) {
        LAPACKE_free( c_r );
    }
    if( c_save != NULL ) {
        LAPACKE_free( c_save );
    }
    if( work != NULL ) {
        LAPACKE_free( work );
    }
    if( work_i != NULL ) {
        LAPACKE_free( work_i );
    }

    return 0;
}

/* Auxiliary function: sormbr scalar parameters initialization */
static void init_scalars_sormbr( char *vect, char *side, char *trans,
                                 lapack_int *m, lapack_int *n, lapack_int *k,
                                 lapack_int *lda, lapack_int *ldc,
                                 lapack_int *lwork )
{
    *vect = 'P';
    *side = 'L';
    *trans = 'T';
    *m = 4;
    *n = 6;
    *k = 4;
    *lda = 8;
    *ldc = 8;
    *lwork = 1024;

    return;
}

/* Auxiliary functions: sormbr array parameters initialization */
static void init_a( lapack_int size, float *a ) {
    lapack_int i;
    for( i = 0; i < size; i++ ) {
        a[i] = 0;
    }
    a[0] = -7.772435188e+000;  /* a[0,0] */
    a[8] = 1.192641139e+000;  /* a[0,1] */
    a[16] = 6.896195002e-003;  /* a[0,2] */
    a[24] = 9.197353721e-001;  /* a[0,3] */
    a[1] = 7.835548371e-003;  /* a[1,0] */
    a[9] = 6.157293320e+000;  /* a[1,1] */
    a[17] = 5.733590722e-001;  /* a[1,2] */
    a[25] = -1.803648025e-001;  /* a[1,3] */
    a[2] = 6.632159464e-003;  /* a[2,0] */
    a[10] = -2.797574364e-002;  /* a[2,1] */
    a[18] = -6.057568073e+000;  /* a[2,2] */
    a[26] = -1.914320111e+000;  /* a[2,3] */
    a[3] = 9.587492794e-002;  /* a[3,0] */
    a[11] = 9.166325927e-001;  /* a[3,1] */
    a[19] = 2.011592835e-001;  /* a[3,2] */
    a[27] = 5.793256283e+000;  /* a[3,3] */
}
static void init_tau( lapack_int size, float *tau ) {
    lapack_int i;
    for( i = 0; i < size; i++ ) {
        tau[i] = 0;
    }
    tau[0] = 1.083446622e+000;
    tau[1] = 1.936987042e+000;
    tau[2] = 0.000000000e+000;
    tau[3] = 0.000000000e+000;
}
static void init_c( lapack_int size, float *c ) {
    lapack_int i;
    for( i = 0; i < size; i++ ) {
        c[i] = 0;
    }
    c[0] = -7.104246616e-001;  /* c[0,0] */
    c[8] = 4.299248457e-001;  /* c[0,1] */
    c[16] = -4.823547602e-001;  /* c[0,2] */
    c[24] = 3.539015725e-002;  /* c[0,3] */
    c[32] = 2.700137794e-001;  /* c[0,4] */
    c[40] = 6.029434502e-002;  /* c[0,5] */
    c[1] = -2.412205487e-001;  /* c[1,0] */
    c[9] = -5.323043466e-001;  /* c[1,1] */
    c[17] = -4.844856858e-001;  /* c[1,2] */
    c[25] = -1.594917476e-001;  /* c[1,3] */
    c[33] = -6.311050057e-001;  /* c[1,4] */
    c[41] = -2.664134838e-003;  /* c[1,5] */
    c[2] = 1.286828667e-001;  /* c[2,0] */
    c[10] = -2.619454265e-001;  /* c[2,1] */
    c[18] = -2.108367532e-001;  /* c[2,2] */
    c[26] = -7.447289824e-001;  /* c[2,3] */
    c[34] = 5.226845741e-001;  /* c[2,4] */
    c[42] = -2.062834352e-001;  /* c[2,5] */
    c[3] = -3.403473496e-001;  /* c[3,0] */
    c[11] = -9.212616831e-002;  /* c[3,1] */
    c[19] = 4.546407461e-001;  /* c[3,2] */
    c[27] = -3.869233131e-001;  /* c[3,3] */
    c[35] = -4.647991061e-002;  /* c[3,4] */
    c[43] = 7.191025019e-001;  /* c[3,5] */
}
static void init_work( lapack_int size, float *work ) {
    lapack_int i;
    for( i = 0; i < size; i++ ) {
        work[i] = 0;
    }
}

/* Auxiliary function: C interface to sormbr results check */
/* Return value: 0 - test is passed, non-zero - test is failed */
static int compare_sormbr( float *c, float *c_i, lapack_int info,
                           lapack_int info_i, lapack_int ldc, lapack_int n )
{
    lapack_int i;
    int failed = 0;
    for( i = 0; i < ldc*n; i++ ) {
        failed += compare_floats(c[i],c_i[i]);
    }
    failed += (info == info_i) ? 0 : 1;
    if( info != 0 || info_i != 0 ) {
        printf( "info=%d, info_i=%d\n",(int)info,(int)info_i );
    }

    return failed;
}