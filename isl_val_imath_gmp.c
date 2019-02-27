#include <string.h>
#include <isl/val_gmp.h>
#include <isl_val_private.h>

// export the gmp number to sioimath
void gmp_to_isl_int(isl_sioimath_ptr dst, const mpz_t src)
{
	int sign = mpz_sgn(src);
	if (sign == 0)
	{
		isl_int_set_si(dst, 0);
	}
	else
	{
		size_t length = (mpz_sizeinbase(src, 2) + (8 * sizeof(long)) - 1) / (8 * sizeof(long));
		if (length <= 10)
		{
			long chunks[10];
			mpz_export(chunks, NULL, -1, sizeof(long), 0, 0, src);
			impz_import(isl_sioimath_reinit_big(dst), length, -1, sizeof(long), 0, 0, chunks);
			isl_sioimath_try_demote(dst);
			if (sign < 0)
				isl_int_neg(dst, dst);
		}
		else
		{
			long *chunks = malloc(length * sizeof(long));
			mpz_export(chunks, NULL, -1, sizeof(long), 0, 0, src);
			impz_import(isl_sioimath_reinit_big(dst), length, -1, sizeof(long), 0, 0, chunks);
			isl_sioimath_try_demote(dst);
			if (sign < 0)
				isl_int_neg(dst, dst);
			free(chunks);
		}
	}
}

// import gmp number to sioimath
void isl_int_to_gmp(mpz_t dst, isl_sioimath_ptr src) 
{
	if (isl_int_is_zero(src))
	{
		mpz_set_ui(dst, 0);
	}
	else
	{
		size_t length = (isl_sioimath_sizeinbase(*src, 2) + (8 * sizeof(long)) - 1) / (8 * sizeof(long));
		isl_sioimath_scratchspace_t scratch;
		if (length <= 10)
		{
			long chunks[10];
			impz_export(chunks, NULL, -1, sizeof(long), 0, 0, isl_sioimath_bigarg_src(*src, &scratch));
			mpz_import(dst, length, -1, sizeof(long), 0, 0, chunks);
			if (isl_int_is_neg(src))
				mpz_neg(dst, dst);			
		}
		else
		{
			long *chunks = malloc(length * sizeof(long));
			impz_export(chunks, NULL, -1, sizeof(long), 0, 0, isl_sioimath_bigarg_src(*src, &scratch));
			mpz_import(dst, length, -1, sizeof(long), 0, 0, chunks);
			if (isl_int_is_neg(src))
				mpz_neg(dst, dst);
			free(chunks);
		}
	}
}

/* Return a reference to an isl_val representing the integer "z".
 */
__isl_give isl_val *isl_val_int_from_gmp(isl_ctx *ctx, mpz_t z)
{
	isl_val *v;

	v = isl_val_alloc(ctx);
	if (!v)
		return NULL;

	gmp_to_isl_int(v->n, z);
	isl_int_set_si(v->d, 1);

	return v;
}

/* Return a reference to an isl_val representing the rational value "n"/"d".
 */
__isl_give isl_val *isl_val_from_gmp(isl_ctx *ctx, const mpz_t n, const mpz_t d)
{
	isl_val *v;

	v = isl_val_alloc(ctx);
	if (!v)
		return NULL;

	gmp_to_isl_int(v->n, n);
	gmp_to_isl_int(v->d, d);

	v = isl_val_normalize(v);

	return v;
}

/* Extract the numerator of a rational value "v" in "z".
 *
 * If "v" is not a rational value, then the result is undefined.
 */
int isl_val_get_num_gmp(__isl_keep isl_val *v, mpz_t z)
{
	if (!v)
		return -1;
	if (!isl_val_is_rat(v))
		isl_die(isl_val_get_ctx(v), isl_error_invalid,
				"expecting rational value", return -1);

	// extract the chunks
	isl_int_to_gmp(z, v->n);

	return 0;
}

/* Extract the denominator of a rational value "v" in "z".
 *
 * If "v" is not a rational value, then the result is undefined.
 */
int isl_val_get_den_gmp(__isl_keep isl_val *v, mpz_t z)
{
	if (!v)
		return -1;
	if (!isl_val_is_rat(v))
		isl_die(isl_val_get_ctx(v), isl_error_invalid,
				"expecting rational value", return -1);

	isl_int_to_gmp(z, v->d);

	return 0;
}
