/*
 * RELIC is an Efficient LIbrary for Cryptography
 * Copyright (C) 2007-2015 RELIC Authors
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
 * @file
 *
 * Implementation of fixed point multiplication on binary elliptic curves.
 *
 * @ingroup eb
 */

#include "relic_core.h"
#include "relic_eb.h"

/*============================================================================*/
/* Private definitions                                                        */
/*============================================================================*/

#if EB_FIX == LWNAF || !defined(STRIP)

#if defined(EB_KBLTZ)

/**
 * Multiplies a binary elliptic curve point by an integer using the w-TNAF
 * method.
 *
 * @param[out] r 				- the result.
 * @param[in] p					- the point to multiply.
 * @param[in] t					- the precomputed table.
 * @param[in] k					- the integer.
 */
static void eb_mul_fix_kbltz(eb_t r, const eb_t *t, const bn_t k) {
	int i, l, n;
	int8_t u, tnaf[FB_BITS + 8];

	if (bn_is_zero(k)) {
		eb_set_infty(r);
		return;
	}

	/* Compute the w-TNAF representation of k. */
	if (eb_curve_opt_a() == OPT_ZERO) {
		u = -1;
	} else {
		u = 1;
	}

	/* Compute the w-TNAF representation of k. */
	l = sizeof(tnaf);
	bn_rec_tnaf(tnaf, &l, k, u, FB_BITS, EB_DEPTH);

	n = tnaf[l - 1];
	if (n > 0) {
		eb_copy(r, t[n / 2]);
	} else {
		eb_neg(r, t[-n / 2]);
	}

	for (i = l - 2; i >= 0; i--) {
		eb_frb(r, r);

		n = tnaf[i];
		if (n > 0) {
			eb_add(r, r, t[n / 2]);
		}
		if (n < 0) {
			eb_sub(r, r, t[-n / 2]);
		}
	}
	/* Convert r to affine coordinates. */
	eb_norm(r, r);
	if (bn_sign(k) == BN_NEG) {
		eb_neg(r, r);
	}
}

#endif /* EB_KBLTZ */

#if defined(EB_PLAIN)

/**
 * Multiplies a binary elliptic curve point by an integer using the w-NAF
 * method.
 *
 * @param[out] r 				- the result.
 * @param[in] t				- the precomputed table.
 * @param[in] k					- the integer.
 */
static void eb_mul_fix_plain(eb_t r, const eb_t *t, const bn_t k) {
	int l, i, n;
	int8_t naf[FB_BITS + 1];

	if (bn_is_zero(k)) {
		eb_set_infty(r);
		return;
	}

	/* Compute the w-TNAF representation of k. */
	l = FB_BITS + 1;
	bn_rec_naf(naf, &l, k, EB_DEPTH);

	n = naf[l - 1];
	if (n > 0) {
		eb_copy(r, t[n / 2]);
	}

	for (i = l - 2; i >= 0; i--) {
		eb_dbl(r, r);

		n = naf[i];
		if (n > 0) {
			eb_add(r, r, t[n / 2]);
		}
		if (n < 0) {
			eb_sub(r, r, t[-n / 2]);
		}
	}
	/* Convert r to affine coordinates. */
	eb_norm(r, r);
	if (bn_sign(k) == BN_NEG) {
		eb_neg(r, r);
	}
}

#endif /* EB_PLAIN */

#endif /* EB_FIX == LWNAF */

/*============================================================================*/
/* Public definitions                                                         */
/*============================================================================*/

#if EB_FIX == BASIC || !defined(STRIP)

void eb_mul_pre_basic(eb_t *t, const eb_t p) {
	bn_t n;

	bn_null(n);

	TRY {
		bn_new(n);

		eb_curve_get_ord(n);

		eb_copy(t[0], p);
		for (int i = 1; i < bn_bits(n); i++) {
			eb_dbl(t[i], t[i - 1]);
		}

		eb_norm_sim(t + 1, (const eb_t *)t + 1, bn_bits(n) - 1);
	}
	CATCH_ANY {
		THROW(ERR_CAUGHT);
	}
	FINALLY {
		bn_free(n);
	}
}

void eb_mul_fix_basic(eb_t r, const eb_t *t, const bn_t k) {
	if (bn_is_zero(k)) {
		eb_set_infty(r);
		return;
	}

	eb_set_infty(r);
	for (int i = 0; i < bn_bits(k); i++) {
		if (bn_get_bit(k, i)) {
			eb_add(r, r, t[i]);
		}
	}
	eb_norm(r, r);
	if (bn_sign(k) == BN_NEG) {
		eb_neg(r, r);
	}
}

#endif

#if EB_FIX == YAOWI || !defined(STRIP)

void eb_mul_pre_yaowi(eb_t *t, const eb_t p) {
	int i, l;
	bn_t n;

	bn_null(n);

	TRY {
		bn_new(n);

		eb_curve_get_ord(n);
		l = CEIL(bn_bits(n), EB_DEPTH);

		eb_copy(t[0], p);
		for (i = 1; i < l; i++) {
			eb_dbl(t[i], t[i - 1]);
			for (int j = 1; j < EB_DEPTH; j++) {
				eb_dbl(t[i], t[i]);
			}
		}

		eb_norm_sim(t + 1, (const eb_t *)t + 1, l - 1);
	} CATCH_ANY {
		THROW(ERR_CAUGHT);
	} FINALLY {
		bn_free(n);
	}
}

void eb_mul_fix_yaowi(eb_t r, const eb_t *t, const bn_t k) {
	int i, j, l;
	eb_t a;
	uint8_t win[CEIL(FB_BITS, EB_DEPTH)];

	if (bn_is_zero(k)) {
		eb_set_infty(r);
		return;
	}

	eb_null(a);

	TRY {
		eb_new(a);

		eb_set_infty(r);
		eb_set_infty(a);

		l = CEIL(FB_BITS, EB_DEPTH);
		bn_rec_win(win, &l, k, EB_DEPTH);

		for (j = (1 << EB_DEPTH) - 1; j >= 1; j--) {
			for (i = 0; i < l; i++) {
				if (win[i] == j) {
					eb_add(a, a, t[i]);
				}
			}
			eb_add(r, r, a);
		}
		eb_norm(r, r);
		if (bn_sign(k) == BN_NEG) {
			eb_neg(r, r);
		}
	}
	CATCH_ANY {
		THROW(ERR_CAUGHT);
	}
	FINALLY {
		eb_free(a);
	}
}

#endif

#if EB_FIX == NAFWI || !defined(STRIP)

void eb_mul_pre_nafwi(eb_t *t, const eb_t p) {
	bn_t n;

	bn_null(n);

	TRY {
		int l;
		bn_new(n);

		eb_curve_get_ord(n);
		l = CEIL(bn_bits(n) + 1, EB_DEPTH);

		eb_copy(t[0], p);
		for (int i = 1; i < l; i++) {
			eb_dbl(t[i], t[i - 1]);
			for (int j = 1; j < EB_DEPTH; j++) {
				eb_dbl(t[i], t[i]);
			}
		}

		eb_norm_sim(t + 1, (const eb_t *)t + 1, l - 1);
	}
	CATCH_ANY {
		THROW(ERR_CAUGHT);
	}
	FINALLY {
		bn_free(n);
	}
}

void eb_mul_fix_nafwi(eb_t r, const eb_t *t, const bn_t k) {
	int i, j, l, d, m;
	eb_t a;
	int8_t naf[FB_BITS + 1];
	char w;

	if (bn_is_zero(k)) {
		eb_set_infty(r);
		return;
	}

	eb_null(a);

	TRY {
		eb_new(a);

		eb_set_infty(r);
		eb_set_infty(a);

		l = sizeof(naf);
		bn_rec_naf(naf, &l, k, 2);
		d = CEIL(l, EB_DEPTH);

		for (i = 0; i < d; i++) {
			w = 0;
			for (j = EB_DEPTH - 1; j >= 0; j--) {
				if (i * EB_DEPTH + j < l) {
					w = (char)(w << 1);
					w = (char)(w + naf[i * EB_DEPTH + j]);
				}
			}
			naf[i] = w;
		}

		if (EB_DEPTH % 2 == 0) {
			m = ((1 << (EB_DEPTH + 1)) - 2) / 3;
		} else {
			m = ((1 << (EB_DEPTH + 1)) - 1) / 3;
		}

		for (j = m; j >= 1; j--) {
			for (i = 0; i < d; i++) {
				if (naf[i] == j) {
					eb_add(a, a, t[i]);
				}
				if (naf[i] == -j) {
					eb_sub(a, a, t[i]);
				}
			}
			eb_add(r, r, a);
		}
		eb_norm(r, r);
		if (bn_sign(k) == BN_NEG) {
			eb_neg(r, r);
		}
	}
	CATCH_ANY {
		THROW(ERR_CAUGHT);
	}
	FINALLY {
		eb_free(a);
	}
}

#endif

#if EB_FIX == COMBS || !defined(STRIP)

void eb_mul_pre_combs(eb_t *t, const eb_t p) {
	int i, j, l;
	bn_t ord;

	bn_null(ord);

	TRY {
		bn_new(ord);

		eb_curve_get_ord(ord);
		l = CEIL(bn_bits(ord), EB_DEPTH);

		eb_set_infty(t[0]);

		eb_copy(t[1], p);
		for (j = 1; j < EB_DEPTH; j++) {
			eb_dbl(t[1 << j], t[1 << (j - 1)]);
			for (i = 1; i < l; i++) {
				eb_dbl(t[1 << j], t[1 << j]);
			}
#if defined(EB_MIXED)
			eb_norm(t[1 << j], t[1 << j]);
#endif
			for (i = 1; i < (1 << j); i++) {
				eb_add(t[(1 << j) + i], t[i], t[1 << j]);
			}
		}

		eb_norm_sim(t + 2, (const eb_t *)t + 2, RELIC_EB_TABLE_COMBS - 2);
	}
	CATCH_ANY {
		THROW(ERR_CAUGHT);
	}
	FINALLY {
		bn_free(ord);
	}
}

void eb_mul_fix_combs(eb_t r, const eb_t *t, const bn_t k) {
	int i, j, l, w, n, p0, p1;
	bn_t ord;

	if (bn_is_zero(k)) {
		eb_set_infty(r);
		return;
	}

	bn_null(ord);

	TRY {
		bn_new(ord);

		eb_curve_get_ord(ord);
		l = CEIL(bn_bits(ord), EB_DEPTH);

		n = bn_bits(k);
		p0 = (EB_DEPTH) * l - 1;

		w = 0;
		p1 = p0--;
		for (j = EB_DEPTH - 1; j >= 0; j--, p1 -= l) {
			w = w << 1;
			if (p1 < n && bn_get_bit(k, p1)) {
				w = w | 1;
			}
		}
		eb_copy(r, t[w]);

		for (i = l - 2; i >= 0; i--) {
			eb_dbl(r, r);

			w = 0;
			p1 = p0--;
			for (j = EB_DEPTH - 1; j >= 0; j--, p1 -= l) {
				w = w << 1;
				if (p1 < n && bn_get_bit(k, p1)) {
					w = w | 1;
				}
			}
			if (w > 0) {
				eb_add(r, r, t[w]);
			}
		}
		eb_norm(r, r);
		if (bn_sign(k) == BN_NEG) {
			eb_neg(r, r);
		}
	}
	CATCH_ANY {
		THROW(ERR_CAUGHT);
	}
	FINALLY {
		bn_free(ord);
	}
}

#endif

#if EB_FIX == COMBD || !defined(STRIP)

void eb_mul_pre_combd(eb_t *t, const eb_t p) {
	int i, j, d, e;
	bn_t n;

	bn_null(n);

	TRY {
		bn_new(n);

		eb_curve_get_ord(n);
		d = CEIL(bn_bits(n), EB_DEPTH);
		e = (d % 2 == 0 ? (d / 2) : (d / 2) + 1);

		eb_set_infty(t[0]);
		eb_copy(t[1], p);
		for (j = 1; j < EB_DEPTH; j++) {
			eb_dbl(t[1 << j], t[1 << (j - 1)]);
			for (i = 1; i < d; i++) {
				eb_dbl(t[1 << j], t[1 << j]);
			}
			for (i = 1; i < (1 << j); i++) {
				eb_add(t[(1 << j) + i], t[1 << j], t[i]);
			}
		}
		eb_set_infty(t[1 << EB_DEPTH]);
		for (j = 1; j < (1 << EB_DEPTH); j++) {
			eb_dbl(t[(1 << EB_DEPTH) + j], t[j]);
			for (i = 1; i < e; i++) {
				eb_dbl(t[(1 << EB_DEPTH) + j], t[(1 << EB_DEPTH) + j]);
			}
		}

		eb_norm_sim(t + 2, (const eb_t *)t + 2, (1 << EB_DEPTH) - 2);
		eb_norm_sim(t + (1 << EB_DEPTH) + 1,
				(const eb_t *)t + (1 << EB_DEPTH) + 1, (1 << EB_DEPTH) - 1);
	}
	CATCH_ANY {
		THROW(ERR_CAUGHT);
	}
	FINALLY {
		bn_free(n);
	}
}

void eb_mul_fix_combd(eb_t r, const eb_t *t, const bn_t k) {
	int i, j, d, e, w0, w1, n0, p0, p1;
	bn_t n;

	if (bn_is_zero(k)) {
		eb_set_infty(r);
		return;
	}

	bn_null(n);

	TRY {
		bn_new(n);

		eb_curve_get_ord(n);

		d = CEIL(bn_bits(n), EB_DEPTH);
		e = (d % 2 == 0 ? (d / 2) : (d / 2) + 1);

		eb_set_infty(r);
		n0 = bn_bits(k);

		p1 = (e - 1) + (EB_DEPTH - 1) * d;
		for (i = e - 1; i >= 0; i--) {
			eb_dbl(r, r);

			w0 = 0;
			p0 = p1;
			for (j = EB_DEPTH - 1; j >= 0; j--, p0 -= d) {
				w0 = w0 << 1;
				if (p0 < n0 && bn_get_bit(k, p0)) {
					w0 = w0 | 1;
				}
			}

			w1 = 0;
			p0 = p1-- + e;
			for (j = EB_DEPTH - 1; j >= 0; j--, p0 -= d) {
				w1 = w1 << 1;
				if (i + e < d && p0 < n0 && bn_get_bit(k, p0)) {
					w1 = w1 | 1;
				}
			}

			eb_add(r, r, t[w0]);
			eb_add(r, r, t[(1 << EB_DEPTH) + w1]);
		}
		eb_norm(r, r);
		if (bn_sign(k) == BN_NEG) {
			eb_neg(r, r);
		}
	}
	CATCH_ANY {
		THROW(ERR_CAUGHT);
	}
	FINALLY {
		bn_free(n);
	}
}

#endif

#if EB_FIX == LWNAF || !defined(STRIP)

void eb_mul_pre_lwnaf(eb_t *t, const eb_t p) {
	eb_tab(t, p, EB_DEPTH);
}

void eb_mul_fix_lwnaf(eb_t r, const eb_t *t, const bn_t k) {
#if defined(EB_KBLTZ)
	if (eb_curve_is_kbltz()) {
		eb_mul_fix_kbltz(r, t, k);
		return;
	}
#endif

#if defined(EB_PLAIN)
	eb_mul_fix_plain(r, t, k);
#endif
}
#endif
