/* Version ID for the JPEG library.
 * Might be useful for tests like "#if JPEG_LIB_VERSION >= 60".
 */
#define JPEG_LIB_VERSION 62

/* libjpeg-turbo version */
#define LIBJPEG_TURBO_VERSION 2.1.1

/* libjpeg-turbo version in integer form */
#define LIBJPEG_TURBO_VERSION_NUMBER 2001001

/* Support arithmetic encoding */
#define C_ARITH_CODING_SUPPORTED 1

/* Support arithmetic decoding */
#define D_ARITH_CODING_SUPPORTED 1

/* Support in-memory source/destination managers */
#define MEM_SRCDST_SUPPORTED 1

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

#define BITS_IN_JSAMPLE 8 /* use 8 or 12 */

/* Define to 1 if you have the <locale.h> header file. */
/* undef HAVE_LOCALE_H */

/* Define to 1 if you have the <stddef.h> header file. */
#define HAVE_STDDEF_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define if you need to include <sys/types.h> to get size_t. */
#define NEED_SYS_TYPES_H 1

/* Define if you have BSD-like bzero and bcopy in <strings.h> rather than
   memset/memcpy in <string.h>. */
/* #undef NEED_BSD_STRINGS */

/* Define to 1 if the system has the type `unsigned char'. */
#define HAVE_UNSIGNED_CHAR 1

/* Define to 1 if the system has the type `unsigned short'. */
#define HAVE_UNSIGNED_SHORT 1

/* Compiler does not support pointers to undefined structures. */
/* #undef INCOMPLETE_TYPES_BROKEN */

/* Define if your (broken) compiler shifts signed values as if they were
   unsigned. */
/* #undef RIGHT_SHIFT_IS_UNSIGNED */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Extra Libreoffice config for windows */
#ifdef _MSC_VER

/* Define "boolean" as unsigned char, not int, per Windows custom */
#ifndef __RPCNDR_H__ /* don't conflict if rpcndr.h already read */
typedef unsigned char boolean;
#endif
#define HAVE_BOOLEAN /* prevent jmorecfg.h from redefining it */

/* Define "INT32" as int, not long, per Windows custom */
#if !(defined(_BASETSD_H_) || defined(_BASETSD_H)) /* don't conflict if basetsd.h already read */
typedef short INT16;
typedef signed int INT32;
#endif
#define XMD_H /* prevent jmorecfg.h from redefining it */

#endif /* _MSC_VER */
