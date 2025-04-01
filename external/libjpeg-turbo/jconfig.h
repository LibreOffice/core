/* Version ID for the JPEG library.
 * Might be useful for tests like "#if JPEG_LIB_VERSION >= 60".
 */
  62

/* libjpeg-turbo version */
  2.1.5.1

/* libjpeg-turbo version in integer form */
  2001005

/* Support arithmetic encoding */
 1

/* Support arithmetic decoding */
 1

/* Support in-memory source/destination managers */
 1

/* Use accelerated SIMD routines. */
/* #undef WITH_SIMD */

/*
 * Define BITS_IN_JSAMPLE as either
 *   8   for 8-bit sample values (the usual setting)
 *   12  for 12-bit sample values
 * Only 8 and 12 are legal data precisions for lossy JPEG according to the
 * JPEG standard, and the IJG code does not support anything else!
 * We do not support run-time selection of data precision, sorry.
 */

  8      /* use 8 or 12 */

/* Define if your (broken) compiler shifts signed values as if they were
   unsigned. */
/* #undef RIGHT_SHIFT_IS_UNSIGNED */

/* Extra Libreoffice config for windows */
#ifdef _MSC_VER

/* Define "boolean" as unsigned char, not int, per Windows custom */
#pragma once /* don't conflict if rpcndr.h already read */
typedef unsigned char boolean;
#endif
 /* prevent jmorecfg.h from redefining it */

/* Define "INT32" as int, not long, per Windows custom */
#if !(defined(_BASETSD_H_) || defined(_BASETSD_H)) /* don't conflict if basetsd.h already read */
typedef short INT16;
typedef signed int INT32;
#endif
 /* prevent jmorecfg.h from redefining it */


