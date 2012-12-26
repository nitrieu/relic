/*
 * RELIC is an Efficient LIbrary for Cryptography
 * Copyright (C) 2007-2012 RELIC Authors
 *
 * This file is part of RELIC. RELIC is legal property of its developers,
 * whose names are not listed here. Please refer to the COPYRIGHT file
 * for contact information.
 *
 * RELIC is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * RELIC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with RELIC. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @defgroup relic Core functions.
 */

/**
 * @file
 *
 * Interface of the library core functions.
 *
 * @version $Id$
 * @ingroup relic
 */

#ifndef RELIC_CORE_H
#define RELIC_CORE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "relic_error.h"
#include "relic_bn.h"
#include "relic_eb.h"
#include "relic_ft.h"
#include "relic_epx.h"
#include "relic_conf.h"
#include "relic_rand.h"
#include "relic_pool.h"

#ifdef MULTI
#include <omp.h>
#include <math.h>
#endif

/*============================================================================*/
/* Constant definitions                                                       */
/*============================================================================*/

/**
 * Indicates that the function executed correctly.
 */
#define STS_OK			0

/**
 * Indicates that an error occurred during the function execution.
 */
#define STS_ERR			1

/**
 * Indicates that a comparison returned that the first argument was lesser than
 * the second argument.
 */
#define CMP_LT			-1

/**
 * Indicates that a comparison returned that the first argument was equal to
 * the second argument.
 */
#define CMP_EQ			0

/**
 * Indicates that a comparison returned that the first argument was greater than
 * the second argument.
 */
#define CMP_GT			1

/**
 * Indicates that two incomparable elements are not equal.
 */
#define CMP_NE			2

/**
 * Optimization identifer for the case where a coefficient is 0.
 */
#define OPT_ZERO		0

/**
 * Optimization identifer for the case where a coefficient is 1.
 */
#define OPT_ONE			1

/**
 * Optimization identifer for the case where a coefficient is 1.
 */
#define OPT_TWO			2

/**
 * Optimization identifer for the case where a coefficient is small.
 */
#define OPT_DIGIT		3

/**
 * Optimization identifier for the case where a coefficient is -3.
 */
#define OPT_MINUS3		4

/**
 * Optimization identifier for the case where the coefficient is random
 */
#define OPT_NONE		5

/**
 * Maximum number of terms to describe a sparse object.
 */
#define MAX_TERMS		16

/*============================================================================*/
/* Type definitions                                                           */
/*============================================================================*/

/**
 * Library context.
 */
typedef struct _ctx_t {
	/** The value returned by the last call, can be STS_OK or STS_ERR. */
	int code;

#ifdef CHECK
	/** The state of the last error caught. */
	sts_t *last;
	/** The error message respective to the last error. */
	char *reason[ERR_MAX];
	/** A flag to indicate if the last error was already caught. */
	int caught;
#endif /* CHECK */

#if defined(CHECK) || defined(TRACE)
	/** The current trace size. */
	int trace;
#endif /* CHECK || TRACE */

#if ALLOC == STATIC
	/** The static pool of digit vectors. */
	pool_t pool[POOL_SIZE];
	/** The index of the next free digit vector in the pool. */
	int next;
#endif /* ALLOC == STATIC */

#ifdef WITH_FB
	/** Currently configured binary field identifier. */
	int fb_id;
	/** Currently configured irreducible binary polynomial. */
	fb_st fb_poly;
	/** Non-zero coefficients of a trinomial or pentanomial. */
	int fb_pa, fb_pb, fb_pc;
	/** Positions of the non-zero coefficients of trinomials or pentanomials. */
	int fb_na, fb_nb, fb_nc;
#if FB_TRC == QUICK || !defined(STRIP)
	/** Powers of z with non-zero traces. */
	int fb_ta, fb_tb, fb_tc;
#endif /* FB_TRC == QUICK */
#if FB_SLV == QUICK || !defined(STRIP)
	/** Table of precomputed half-traces. */
	fb_st fb_half[(FB_DIGIT / 8 + 1) * FB_DIGS][16];
#endif /* FB_SLV == QUICK */
#if FB_SRT == QUICK || !defined(STRIP)
	/** Square root of z. */
	fb_st fb_srz;
#ifdef FB_PRECO
	/** Multiplication table for the z^(1/2). */
	fb_st fb_tab_srz[256];
#endif /* FB_PRECO */
#endif /* FB_SRT == QUICK */
#if FB_INV == ITOHT || !defined(STRIP)
	/** Stores an addition chain for (FB_BITS - 1). */
	int chain[MAX_TERMS + 1];
	/** Stores the length of the addition chain. */
	int chain_len;
	/** Tables for repeated squarings. */
	fb_st fb_tab_sqr[MAX_TERMS][FB_TABLE];
	/** Pointers to the elements in the tables of repeated squarings. */
	fb_st *fb_tab_ptr[MAX_TERMS][FB_TABLE];
#endif /* FB_INV == ITOHT */
#endif /* WITH_FB */

#ifdef WITH_EB
	/** Currently configured binary elliptic curve identifier. */
	int eb_id;
	/** The a-coefficient of the elliptic curve. */
	fb_st eb_a;
	/** The b-coefficient of the elliptic curve. */
	fb_st eb_b;
	/** Optimization identifier for the a-coefficient. */
	int eb_opt_a;
	/** Optimization identifier for the b-coefficient. */
	int eb_opt_b;
#if defined(EB_SUPER)
	/** The b-coefficient of the elliptic curve. */
	fb_st eb_c;
	/** Optimization identifier for the a-coefficient. */
	int eb_opt_c;
#endif
	/** The generator of the elliptic curve. */
	eb_st eb_g;
	/** The order of the group of points in the elliptic curve. */
	bn_st eb_r;
	/** The cofactor of the group order in the elliptic curve. */
	bn_st eb_h;
#ifdef EB_KBLTZ
#if (EB_MUL == LWNAF || EB_MUL == RWNAF || EB_FIX == LWNAF || EB_SIM == INTER || !defined(STRIP))
	/** Parameters required by Koblitz curves. @{ */
	bn_st eb_vm;
	bn_st eb_s0;
	bn_st eb_s1;
#endif /* EB_KBLTZ */
#endif /* EB_MUL */
	/** Flag that stores if the prime curve has efficient endomorphisms. */
	int eb_is_kbltz;
	/** Flag that stores if the prime curve is supersingular. */
	int eb_is_super;
#ifdef EB_PRECO
	/** Precomputation table for generator multiplication. */
	eb_st eb_pre[EB_TABLE];
	/** Array of pointers to the precomputation table. */
	eb_st *eb_ptr[EB_TABLE];
#endif /* EB_PRECO */
#endif /* WITH_EB */

#ifdef WITH_FT
	/** Currently configured ternary field identifier. */
	int ft_id;
	/** Currently configured irreducible ternary polynomial. */
	ft_st ft_poly;
	/** Non-zero coefficients of a trinomial or pentanomial. */
	int ft_pa, ft_pb, ft_pc, ft_pd;
	/** Positions of the non-zero coefficients of trinomials or pentanomials. */
	int ft_na, ft_nb, ft_nc;
#if FT_CRT == QUICK || !defined(STRIP)
	/** Non-zero bits of polynomial stored in sparse form. */
	int crz[MAX_TERMS + 1];
	/** Number of bits of special form polynomial. */
	int crz_len;
	/** Cube root of z. */
	ft_st ft_crz;
	/** Non-zero bits of polynomial stored in sparse form. */
	int srz[MAX_TERMS + 1];
	/** Number of bits of special form polynomial. */
	int srz_len;
	/** Square of cube root of z. */
	ft_st ft_srz;
#ifdef FT_PRECO
	/** Multiplication table for the z^(1/3). */
	ft_st ft_tab_crz[256];
	/** Multiplication table for the z^(2/3). */
	ft_st ft_tab_srz[256];
#endif /* FT_PRECO */
#endif /* FT_CRT == QUICK */
#endif /* WITH_FT */

#ifdef WITH_FP
	/** Currently configured prime field identifier. */
	int fp_id;
	/** Currently configured prime modulus. */
	bn_st prime;
	/** Value (R^2 mod p) for converting small integers to Montgomery form. */
	bn_st conv;
	/** Value of constant one in Montgomery form. */
	bn_st one;
	/** Prime modulus modulo 8. */
	dig_t mod8;
	/** Value derived from the prime used for modular reduction. */
	dig_t u;
	/** Quadratic non-residue. */
	int qnr;
	/** Cubic non-residue. */
	int cnr;
	/** Sparse representation of prime modulus. */
	int sps[MAX_TERMS + 1];
	/** Length of sparse prime representation. */
	int sps_len;
	/** Sparse representation of parameter used to generate prime. */
	int var[MAX_TERMS + 1];
#endif /* WITH_FP */

#ifdef WITH_EP
	/** Currently configured prime elliptic curve identifier. */
	int ep_id;
	/** The a-coefficient of the elliptic curve. */
	fp_st ep_a;
	/** The b-coefficient of the elliptic curve. */
	fp_st ep_b;
	/** The generator of the elliptic curve. */
	ep_st ep_g;
	/** The order of the group of points in the elliptic curve. */
	bn_st ep_r;
	/** The cofactor of the group order in the elliptic curve. */
	bn_st ep_h;
#ifdef EP_KBLTZ
#if EP_MUL == LWNAF || EP_FIX == COMBS || EP_FIX == LWNAF || EP_SIM == INTER || !defined(STRIP)
	/** Parameters required by the GLV method. @{ */
	fp_st beta;
	bn_st ep_v1[3];
	bn_st ep_v2[3];
	/** @} */
#endif /* EP_KBLTZ */
#endif /* EP_MUL */
	/** Optimization identifier for the a-coefficient. */
	int ep_opt_a;
	/** Optimization identifier for the b-coefficient. */
	int ep_opt_b;
	/** Flag that stores if the prime curve has efficient endomorphisms. */
	int ep_is_kbltz;
	/** Flag that stores if the prime curve is supersingular. */
	int ep_is_super;
#ifdef EP_PRECO
	/** Precomputation table for generator multiplication. */
	ep_st ep_pre[EP_TABLE];
	/** Array of pointers to the precomputation table. */
	ep_st *ep_ptr[EP_TABLE];
#endif /* EP_PRECO */
#endif /* WITH_EP */

#ifdef WITH_PP
	/** The generator of the elliptic curve. */
	ep2_st ep2_g;
#if ALLOC == STATIC || ALLOC == DYNAMIC || ALLOC == STACK
	/** The first coordinate of the generator. */
	fp2_st ep2_gx;
	/** The second coordinate of the generator. */
	fp2_st ep2_gy;
	/** The third coordinate of the generator. */
	fp2_st ep2_gz;
#endif
	/** The a parameter of the curve. */
	fp2_st ep2_a;
	/** The b parameter of the curve. */
	fp2_st ep2_b;
	/** The order of the group of points in the elliptic curve. */
	bn_st ep2_r;
	/** Flag that stores if the prime curve is twisted. */
	int ep2_is_twist;
#ifdef EP_PRECO
	/** Precomputation table for generator multiplication.*/
	ep2_st ep2_pre[EP_TABLE];
	/** Array of pointers to the precomputation table. */
	ep2_st *ep2_ptr[EP_TABLE];
#endif /* EP_PRECO */
#if ALLOC == STACK
/** In case of stack allocation, we need to get global memory for the table. */
	fp2_st _ep2_pre[3 * EP_TABLE];
#endif /* ALLOC == STACK */
	/** Constants for computing Frobenius maps in higher extensions. @{ */
	fp2_st fp2_p[5];
	fp_st fp2_p2[3];
	fp2_st fp2_p3[5];
	/** @} */
	/** Constants for computing Frobenius maps in higher extensions. @{ */
	fp_st fp3_base[2];
	fp_st fp3_p[5];
	fp_st fp3_p2[5];
	fp_st fp3_p3[5];
	fp_st fp3_p4[5];
	fp_st fp3_p5[5];
	/** @} */
#endif /* WITH_PP */

	/** Internal state of the PRNG. */
	unsigned char rand[RAND_SIZE];
} ctx_t;

/*============================================================================*/
/* Function prototypes                                                        */
/*============================================================================*/

/**
 * Initializes the library.
 *
 * @return STS_OK if no error occurs, STS_ERR otherwise.
 */
int core_init(void);

/**
 * Finalizes the library.
 *
 * @return STS_OK if no error occurs, STS_ERR otherwise.
 */
int core_clean(void);

/**
 * Returns a pointer to the current library context.
 *
 * @return a pointer to the library context.
 */
ctx_t *core_get(void);

/**
 * Switched the library context to a new context.
 *
 * @param[in] ctx					- the new library context.
 */
void core_set(ctx_t *ctx);

#endif /* !RELIC_CORE_H */
