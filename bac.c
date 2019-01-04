#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>  /* -lm */

/*
 *  Hexadecimal, Decimal and Binary converter to several numeric bases
 *  supports Output conversions from Base {2,...,10,16}
 *
 *	Orestes Leal Rodríguez, 2016, <olealrd1981@gmail.com>
 *
 *      2016-19-03 - converter from decimal input, first revision
 *      2016-21-03 - hex input added
 *      2016-28-03 - binary input added
 *		2016-07-04 - 64 bit input/output supported, negative 
 *                   inputs/output for bin and hex, several bug fixes
 */

typedef unsigned int uint32;
typedef unsigned short ushort16;
typedef unsigned char uchar8;

#define HEXADECIMAL 0
#define DECIMAL     1
#define BINARY      2

unsigned short d2charb(long long n, uchar8 dst[], uchar8 base);
long long ascii2int(char str[]);
long long hex2d(char *hstr);
long long bin2d(char inp[]);
uchar8 wbase(char inp[]);
void thrown_nan();

int main(int argc, char *argv[])
{
	uchar8 A[65];
	long long input_number;
	unsigned short base;
	long long n;

	if (argc < 3) {
		printf("give me more arguments, please, two needed (input, base)\n");
		return EXIT_FAILURE;
	}

	base = ascii2int(argv[2]);

	if (base < 2)
		return EXIT_FAILURE;

	/* handle decimal,hex or binary input */	
	switch (wbase(argv[1]))
	{
		case DECIMAL:
			input_number = ascii2int(argv[1]);
			break;

		case HEXADECIMAL:
			input_number = hex2d(argv[1]);
			break;

		case BINARY:
			input_number = bin2d(argv[1]);
			break;
		
		default:
			break;
	}
			
	if (base > 10) {
		switch(base) {
		case 16:
			break;
		default:
			fprintf(stderr, "Error: unsupported numeric base, symbols are not defined for it\n");
			exit(EXIT_FAILURE);
		}
	}

	n = d2charb(input_number, A, base);

	if (base == 16)
		printf("0x");
	else if (A[n] == '-')
		printf("-");

	for (int i = n-1; i >= 0; i--)
		printf("%c", A[i]);
	

	printf("\n");
	return EXIT_SUCCESS;
}

/*
 * ASCII decimal to long integer
 * converts a numeric string (i.e., "8192") into an integer.
 */
long long ascii2int(char str[])
{
	short len;
	unsigned long long positional = 1;
	long long number = 0;

	len = strnlen(str, 64); /* max = 64 digits */

	for (int i = len-1; i >= 0 ; i--)
	{
		if (str[i] < '0' || str[i] > '9')
			thrown_nan();

		number += ((str[i] - 0x30) * positional);
		positional *= 10;
	}

	return number;
}


/* 
 *  Decimal to character encoded to the Base specified 
 *
 *	will store the remainders of each division by `base'
 *  until the quotient is zero, remainders will be stored
 *	in the array named dst, supplied by the user, since the
 *	remainders together	conform the same input number (n),
 *	this routine can be used to 'slice' the digits in 'n'
 *	and to treat them separately in the destination array,
 *	for computing or conversion purposes.
 *
 *  for example if you call d2charb like this:
 *
 *		d2charb(32, array, 2) then array will contains
 *		the binary representation of 32, that is 100000,
 *		since selected base is 2
 *
 *  In other words, this function is also a converter from 
 *  decimal to `any base' and is also a integer to char converter
 *
 *
 *  NOTE: the storage method of the numeric symbols is ASCII, 
 *		  to manipulate them correctly you must convert them back 
 *		  by substracting 0x30 from each symbol stored in the
 *		  destination array.
 */  
unsigned short d2charb(long long n, uchar8 dst[], uchar8 base)
{
	long long quot = n; /* quotient of the division */
	unsigned short i;
	uchar8 *b16syms = (uchar8 *)"0123456789abcdef";
	int sign = 0;

	if (n < 0) 
		sign = -1; /* negative sign */

	for (i = 0; quot != 0; i++)
	{
		if (base == 16) /* handle symbols > 9 (base 16) */
		{
			if (quot < 0)
				dst[i] = b16syms[(quot*(-1)) % base];
			else
				dst[i] = b16syms[quot % base];
		}
		else
		{
			if (quot < 0)
				dst[i] = ((quot*(-1)) % base) + 0x30;	
			else
				dst[i] = (quot % base) + 0x30; /* encode and store the remainder */
		}

		quot = quot/base; /* could be optimized with a left shift by 4 when base=16 */
	}

	if (sign < 0) dst[i] = '-';

	return i; /* # of digits in n */
}

/* ASCII based converter from hexadecimal string to long long integer */
long long hex2d(char *hstr)
{
	unsigned long long hpos = 1;
	long long number = 0;
	unsigned char cnum;
	int len;

	if (*hstr == '0' && 
		*(hstr+1) == 'x') /* if format is 0x.. chop that part */
		hstr += 2;

	len = strnlen(hstr, 16); /* 16 digits max for hex number (64 bits) */

	for (int i = len-1; i >= 0; i--)
	{
	//	if (hstr[i] > '9' && islower(hstr[i]))
		cnum = tolower(hstr[i]);

		if (hstr[i] >= '0')
		{
			if (hstr[i] <= '9') /* decimal digit */
				number += (hstr[i]-0x30) * hpos;

			else if (cnum >= 'a' && cnum <= 'f') /* hexadecimal digit */
				number += ((cnum-0x57) * hpos);

			else  /* not an hexadecimal symbol */ thrown_nan();
		}

		hpos *= 16;
	}
	return number;
}

/* 
 * What Base? 
 * detects if the input format is decimal, hexadecimal or binary
 * 
 * An intended hex input without 0x and that all his digits
 * are <= 9 is considered Decimal, in this case the
 * prefix 0x is needed because this routine needs some context.
 *
 * NOTE that this is a naive guess mechanism and only looks for
 * simple patterns, the real checking to each element of the string
 * is done on the conversion routines.
 */
uchar8 wbase(char inp[])
{
	if (inp[0] == '0' &&
		inp[1] == 'x')
		return HEXADECIMAL; /* hex */

	int lc = strnlen(inp,128)-1;

	if (tolower(inp[lc]) == 'b')
		return BINARY;

	uchar8 len = strnlen(inp, 16);

	for (int i = 0; i < len; i++) {
		unsigned char c = tolower(inp[i]);

		if (c >= 'a' && c <= 'f')
			return HEXADECIMAL;
	}

	return DECIMAL; /* decimal otherwise */
}

/*
 * ASCII binary string to long long integer
 */
long long bin2d(char inp[])
{
	int len = strnlen(inp,65); /* only 64 digits (bits) max */
	long long n = 0;
	unsigned long long exp = 0;
	int i;

	for (i = len-2; i >= 0; i--)
	{
		if (inp[i] < '0' || inp[i] > '1') /* check if it's binary */
			thrown_nan();

		else if (inp[i] == '1')
			n = n + (long long)pow(2, exp);
		exp++;
	}
	return n;
}

void thrown_nan()
{
	fprintf(stderr, "NaN\n");
	exit(EXIT_FAILURE);
}
