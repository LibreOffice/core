/* src/raptor_config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
#undef AC_APPLE_UNIVERSAL_BUILD

/* have to check C99 vsnprintf at runtime because cross compiling */
#undef CHECK_VSNPRINTF_RUNTIME

/* vsnprintf has C99 compatible return value */
#undef HAVE_C99_VSNPRINTF

/* Have curl/curl.h */
#undef HAVE_CURL_CURL_H

/* Define to 1 if you have the <dlfcn.h> header file. */
#undef HAVE_DLFCN_H

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define to 1 if you have the <fcntl.h> header file. */
#undef HAVE_FCNTL_H

/* Define to 1 if you have the <fetch.h> header file. */
#undef HAVE_FETCH_H

/* Define to 1 if you have the `getopt' function. */
#undef HAVE_GETOPT

/* Define to 1 if you have the <getopt.h> header file. */
#undef HAVE_GETOPT_H

/* Define to 1 if you have the `getopt_long' function. */
#undef HAVE_GETOPT_LONG

/* Define to 1 if you have the `gettimeofday' function. */
#undef HAVE_GETTIMEOFDAY

/* INN parsedate function present */
#undef HAVE_INN_PARSEDATE

/* Define to 1 if you have the <inttypes.h> header file. */
#undef HAVE_INTTYPES_H

/* Define to 1 if you have the `isascii' function. */
#undef HAVE_ISASCII

/* Define to 1 if you have the <libxml/hash.h> header file. */
#define HAVE_LIBXML_HASH_H 1

/* Define to 1 if you have the <libxml/HTMLparser.h> header file. */
#define HAVE_LIBXML_HTMLPARSER_H 1

/* Define to 1 if you have the <libxml/nanohttp.h> header file. */
#define HAVE_LIBXML_NANOHTTP_H 1

/* Define to 1 if you have the <libxml/parser.h> header file. */
#define HAVE_LIBXML_PARSER_H 1

/* Define to 1 if you have the <libxml/SAX2.h> header file. */
#define HAVE_LIBXML_SAX2_H 1

/* Define to 1 if you have the <libxslt/xslt.h> header file. */
#define HAVE_LIBXSLT_XSLT_H 1

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if you have the `qsort_r' function. */
#undef HAVE_QSORT_R

/* Define to 1 if you have the `qsort_s' function. */
/* note: MSVC has qsort_s but the sort_r.h code uses advanced features like
 * "inline" that MSVC 2013 doesn't understand; fortunately there's another
 * level of fallback... */
#undef HAVE_QSORT_S

/* Raptor raptor_parse_date available */
#undef HAVE_RAPTOR_PARSE_DATE

/* Define to 1 if you have the `setjmp' function. */
#undef HAVE_SETJMP

/* Define to 1 if you have the <setjmp.h> header file. */
#undef HAVE_SETJMP_H

/* Define to 1 if you have the `stat' function. */
#undef HAVE_STAT

/* Define to 1 if you have the <stddef.h> header file. */
#define HAVE_STDDEF_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#undef HAVE_STDINT_H

/* Define to 1 if you have the <stdio.h> header file. */
#define HAVE_STDIO_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strcasecmp' function. */
#undef HAVE_STRCASECMP

/* Define to 1 if you have the `stricmp' function. */
#define HAVE_STRICMP 1

/* Define to 1 if you have the <strings.h> header file. */
#undef HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* have the strtok_r function */
#undef HAVE_STRTOK_R

/* Define to 1 if you have the <sys/param.h> header file. */
#undef HAVE_SYS_PARAM_H

/* Define to 1 if you have the <sys/stat.h> header file. */
#undef HAVE_SYS_STAT_H

/* Define to 1 if you have the <sys/time.h> header file. */
#undef HAVE_SYS_TIME_H

/* Define to 1 if you have the <sys/types.h> header file. */
#undef HAVE_SYS_TYPES_H

/* Define to 1 if you have the <time.h> header file. */
#define HAVE_TIME_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#undef HAVE_UNISTD_H

/* Define to 1 if you have the `vasprintf' function. */
#undef HAVE_VASPRINTF

/* Define to 1 if you have the `vsnprintf' function. */
#define HAVE_VSNPRINTF 1

/* Flex version as a decimal */
#define FLEX_VERSION_DECIMAL 20539

/* Define to 1 if you have the `xmlCtxtUseOptions' function. */
#define HAVE_XMLCTXTUSEOPTIONS 1

/* Define to 1 if you have the `xmlSAX2InternalSubset' function. */
#define HAVE_XMLSAX2INTERNALSUBSET 1

/* YAJL has API version 2 */
#undef HAVE_YAJL2

/* Define to 1 if you have the <yajl/yajl_parse.h> header file. */
#undef HAVE_YAJL_YAJL_PARSE_H

/* Is __FUNCTION__ available */
#define HAVE___FUNCTION__ 1

/* ICU UC major version */
#define ICU_UC_MAJOR_VERSION ICU_MAJOR

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#undef LT_OBJDIR

/* Define to 1 if maintainer mode is enabled. */
#undef MAINTAINER_MODE

/* need 'extern int optind' declaration? */
#undef NEED_OPTIND_DECLARATION

/* Name of package */
#undef PACKAGE

/* Define to the address where bug reports for this package should be sent. */
#undef PACKAGE_BUGREPORT

/* Define to the full name of this package. */
#undef PACKAGE_NAME

/* Define to the full name and version of this package. */
#undef PACKAGE_STRING

/* Define to the one symbol short name of this package. */
#undef PACKAGE_TARNAME

/* Define to the home page for this package. */
#undef PACKAGE_URL

/* Define to the version of this package. */
#undef PACKAGE_VERSION

/* Define to 1 if debug messages are enabled. */
#undef RAPTOR_DEBUG

/* Use ICU for Unicode NFC check */
#define RAPTOR_ICU_NFC 1

/* does libxml struct xmlEntity have a field etype */
#define RAPTOR_LIBXML_ENTITY_ETYPE 1

/* does libxml struct xmlEntity have a field name_length */
#undef RAPTOR_LIBXML_ENTITY_NAME_LENGTH

/* does libxml have HTML_PARSE_NONET */
#define RAPTOR_LIBXML_HTML_PARSE_NONET 1

/* does libxml xmlSAXHandler have externalSubset field */
#define RAPTOR_LIBXML_XMLSAXHANDLER_EXTERNALSUBSET 1

/* does libxml xmlSAXHandler have initialized field */
#define RAPTOR_LIBXML_XMLSAXHANDLER_INITIALIZED 1

/* does libxml have XML_PARSE_NONET */
#define RAPTOR_LIBXML_XML_PARSE_NONET 1

/* Minimum supported package version */
#define RAPTOR_MIN_VERSION_DECIMAL 20000

/* Building GRDDL parser */
#undef RAPTOR_PARSER_GRDDL

/* Building guess parser */
#undef RAPTOR_PARSER_GUESS

/* Building JSON parser */
#undef RAPTOR_PARSER_JSON

/* Building N-Quads parser */
#undef RAPTOR_PARSER_NQUADS

/* Building N-Triples parser */
#define RAPTOR_PARSER_NTRIPLES 1

/* Building RDFA parser */
#undef RAPTOR_PARSER_RDFA

/* Building RDF/XML parser */
#define RAPTOR_PARSER_RDFXML 1

/* Building RSS Tag Soup parser */
#undef RAPTOR_PARSER_RSS

/* Building TRiG parser */
#undef RAPTOR_PARSER_TRIG

/* Building Turtle parser */
#undef RAPTOR_PARSER_TURTLE

/* Building Atom 1.0 serializer */
#undef RAPTOR_SERIALIZER_ATOM

/* Building GraphViz DOT serializer */
#undef RAPTOR_SERIALIZER_DOT

/* Building HTML Table serializer */
#undef RAPTOR_SERIALIZER_HTML

/* Building JSON serializer */
#undef RAPTOR_SERIALIZER_JSON

/* Building mKR serializer */
#undef RAPTOR_SERIALIZER_MKR

/* Building N-Quads serializer */
#undef RAPTOR_SERIALIZER_NQUADS

/* Building N-Triples serializer */
#define RAPTOR_SERIALIZER_NTRIPLES 1

/* Building RDF/XML serializer */
#define RAPTOR_SERIALIZER_RDFXML 1

/* Building RDF/XML-abbreviated serializer */
#define RAPTOR_SERIALIZER_RDFXML_ABBREV 1

/* Building RSS 1.0 serializer */
#undef RAPTOR_SERIALIZER_RSS_1_0

/* Building Turtle serializer */
#undef RAPTOR_SERIALIZER_TURTLE

/* Release version as a decimal */
#define RAPTOR_VERSION_DECIMAL 20016

/* Major version number */
#define RAPTOR_VERSION_MAJOR 2

/* Minor version number */
#define RAPTOR_VERSION_MINOR 0

/* Release version number */
#define RAPTOR_VERSION_RELEASE 16

/* Have libcurl WWW library */
#undef RAPTOR_WWW_LIBCURL

/* Have libfetch WWW library */
#undef RAPTOR_WWW_LIBFETCH

/* Have libxml available as a WWW library */
#undef RAPTOR_WWW_LIBXML

/* No WWW library */
#define RAPTOR_WWW_NONE

/* Check XML 1.1 Names */
#undef RAPTOR_XML_1_1

/* Use libxml XML parser */
#define RAPTOR_XML_LIBXML 1

/* Define to 1 if all of the C90 standard headers exist (not just the ones
   required in a freestanding environment). This macro is provided for
   backward compatibility; new code need not use it. */
#undef STDC_HEADERS

/* Version number of package */
#define VERSION "2.0.16"

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
#  undef WORDS_BIGENDIAN
# endif
#endif

/* Define to 1 if `lex' declares `yytext' as a `char *' by default, not a
   `char[]'. */
#undef YYTEXT_POINTER

/* Number of bits in a file offset, on hosts where this is settable. */
#undef _FILE_OFFSET_BITS

/* Define for large files, on AIX-style hosts. */
#undef _LARGE_FILES

/* Define to empty if `const' does not conform to ANSI C. */
#undef const

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#undef inline
#endif

// from raptor_config_cmake.h.in ////////////////////////////////////////////

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  ifndef _CRT_NONSTDC_NO_DEPRECATE
#  define _CRT_NONSTDC_NO_DEPRECATE
#  endif
#  ifndef _CRT_SECURE_NO_DEPRECATE
#  define _CRT_SECURE_NO_DEPRECATE
#  endif

#  ifdef _MSC_VER
#      define __func__    __FUNCTION__
#  endif

#  define RAPTOR_INLINE  __inline

#  define S_ISTYPE(mode, mask)  (((mode) & _S_IFMT) == (mask))
#  define S_ISDIR(mode)    S_ISTYPE((mode), _S_IFDIR)
#  define S_ISREG(mode)    S_ISTYPE((mode), _S_IFREG)

   /* Mode bits for access() */
#  define R_OK 04
#  define W_OK 02

#  if !defined(HAVE_ACCESS) && defined(HAVE__ACCESS)
#    define access(p,m)    _access(p,m)
#  endif
#  ifndef HAVE_STRCASECMP
#    if defined(HAVE__STRICMP)
#      define strcasecmp(a,b)  _stricmp(a,b)
#    elif defined(HAVE_STRICMP)
#      define strcasecmp(a,b)  stricmp(a,b)
#    endif
#  endif
#  if !defined(HAVE_SNPRINTF) && defined(HAVE__SNPRINTF)
#    define snprintf    _snprintf
#  endif
#  if !defined(HAVE_VSNPRINTF) && defined(HAVE__VSNPRINTF)
#    define vsnprintf    _vsnprintf
#  endif

   /* These prevent parsedate.c from declaring malloc() and free() */
#  define YYMALLOC malloc
#  define YYFREE   free
#endif


