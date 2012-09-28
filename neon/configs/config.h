/* Define to specific EGD socket path */
/* #undef EGD_PATH */

/* Define if EGD should be supported */
/* #undef ENABLE_EGD */

/* Define if GSS_C_NT_HOSTBASED_SERVICE is not defined otherwise */
/* #undef GSS_C_NT_HOSTBASED_SERVICE */

/* Define to 1 if you have the <arpa/inet.h> header file. */
#ifdef UNX
#define HAVE_ARPA_INET_H 1
#endif

/* Define to 1 if you have the `bind_textdomain_codeset' function. */
/* #undef HAVE_BIND_TEXTDOMAIN_CODESET 1 */

/* Define to 1 if you have the declaration of `h_errno', and to 0 if you
   don't. */
#ifdef WIN32
#define HAVE_DECL_H_ERRNO 1
#endif

/* Define to 1 if you have the declaration of `stpcpy', and to 0 if you don't.
   */
#ifdef LINUX
#define HAVE_DECL_STPCPY 1
#endif

/* Define to 1 if you have the declaration of `strerror_r', and to 0 if you
   don't. */
#ifdef LINUX
#define HAVE_DECL_STRERROR_R 1
#endif

/* Define to 1 if you have the <dlfcn.h> header file. */
#ifdef UNX
#define HAVE_DLFCN_H 1
#endif

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define if you have expat */
/* #undef HAVE_EXPAT */

/* Define to 1 if you have the `fstat64' function. */
#define HAVE_FSTAT64

/* Define to 1 if you have the `gai_strerror' function. */
#define HAVE_GAI_STRERROR 1

/* Define if GnuTLS support is enabled */
/* #undef HAVE_GNUTLS */

/* Define to 1 if you have the `gnutls_session_get_data2' function. */
/* #undef HAVE_GNUTLS_SESSION_GET_DATA2 */

/* Define if GSSAPI support is enabled */
/* #undef HAVE_GSSAPI */

/* Define to 1 if you have the <gssapi/gssapi_generic.h> header file. */
/* #undef HAVE_GSSAPI_GSSAPI_GENERIC_H */

/* Define to 1 if you have the <gssapi/gssapi.h> header file. */
/* #undef HAVE_GSSAPI_GSSAPI_H */

/* Define to 1 if you have the <gssapi.h> header file. */
/* #undef HAVE_GSSAPI_H */

/* Define to 1 if you have the `gss_init_sec_context' function. */
/* #undef HAVE_GSS_INIT_SEC_CONTEXT */

/* Define to 1 if you have the `hstrerror' function. */
/* #undef HAVE_HSTRERROR */

/* Define to 1 if you have the `inet_ntop' function. */
#ifdef UNX
#define HAVE_INET_NTOP 1
#endif

/* Define to 1 if you have the <inttypes.h> header file. */
#ifdef UNX
#define HAVE_INTTYPES_H 1
#endif

/* Define to 1 if you have the `isatty' function. */
#define HAVE_ISATTY 1

/* Define to 1 if you have the <libintl.h> header file. */
/* #undef HAVE_LIBINTL_H */

/* Define if you have libxml */
#define HAVE_LIBXML 1

/* Define to 1 if you have the <libxml/parser.h> header file. */
#define HAVE_LIBXML_PARSER_H 1

/* Define to 1 if you have the <libxml/xmlversion.h> header file. */
#define HAVE_LIBXML_XMLVERSION_H 1

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if you have the <locale.h> header file. */
#define HAVE_LOCALE_H 1

/* Define to 1 if you have the `lseek64' function. */
#define HAVE_LSEEK64 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <netdb.h> header file. */
#ifdef UNX
#define HAVE_NETDB_H 1
#endif

/* Define to 1 if you have the <netinet/in.h> header file. */
#ifdef UNX
#define HAVE_NETINET_IN_H 1
#endif

/* Define to 1 if you have the <netinet/tcp.h> header file. */
#ifdef UNX
#define HAVE_NETINET_TCP_H 1
#endif

/* Define if OpenSSL support is enabled */
#define HAVE_OPENSSL 1

/* Define to 1 if you have the <openssl/opensslv.h> header file. */
#define HAVE_OPENSSL_OPENSSLV_H 1

/* Define to 1 if you have the <openssl/ssl.h> header file. */
#define HAVE_OPENSSL_SSL_H 1

/* Define to 1 if you have the `pipe' function. */
#define HAVE_PIPE 1

/* Define to 1 if you have the `poll' function. */
#define HAVE_POLL 1

/* Define to 1 if you have the `pthread_mutex_init' function. */
/* #undef HAVE_PTHREAD_MUTEX_INIT */

/* Define to 1 if you have the `pthread_mutex_lock' function. */
/* #undef HAVE_PTHREAD_MUTEX_LOCK */

/* Define to 1 if you have the `setlocale' function. */
#define HAVE_SETLOCALE 1

/* Define to 1 if you have the `setsockopt' function. */
#if defined (LINUX) || defined (WIN32)
#define HAVE_SETSOCKOPT 1
#endif

/* Define to 1 if you have the `setvbuf' function. */
#define HAVE_SETVBUF 1

/* Define to 1 if you have the `shutdown' function. */
#ifdef LINUX
#define HAVE_SHUTDOWN 1
#endif

/* Define to 1 if you have the `signal' function. */
#define HAVE_SIGNAL 1

/* Define to 1 if you have the <signal.h> header file. */
#define HAVE_SIGNAL_H 1

/* Define to 1 if you have the `snprintf' function. */
#ifdef UNX
#define HAVE_SNPRINTF 1
#endif

#ifdef WIN32
#define snprintf _snprintf
#endif

/* Define to 1 if you have the <socks.h> header file. */
/* #undef HAVE_SOCKS_H */

/* Define to 1 if you have the <stdarg.h> header file. */
#define HAVE_STDARG_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `stpcpy' function. */
#ifdef LINUX
#define HAVE_STPCPY 1
#endif

/* Define to 1 if you have the `strcasecmp' function. */
#ifdef UNX
#define HAVE_STRCASECMP 1
#endif

#ifdef WIN32
#define strcasecmp strcmpi
#endif

/* Define to 1 if you have the `strerror_r' function. */
#ifdef LINUX
#define HAVE_STRERROR_R 1
#endif

/* Define to 1 if you have the <strings.h> header file. */
#ifdef UNX
#define HAVE_STRINGS_H 1
#endif

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strtoll' function. */
#define HAVE_STRTOLL 1

/* Define to 1 if you have the `strtoq' function. */
/* #undef HAVE_STRTOQ */

/* Define to 1 if `tm_gmtoff' is member of `struct tm'. */
#ifdef LINUX
#define HAVE_STRUCT_TM_TM_GMTOFF 1
#endif

/* Define to 1 if `__tm_gmtoff' is member of `struct tm'. */
/* #undef HAVE_STRUCT_TM___TM_GMTOFF */

/* Define to 1 if you have the <sys/limits.h> header file. */
/* #undef HAVE_SYS_LIMITS_H */

/* Define to 1 if you have the <sys/poll.h> header file. */
#ifdef UNX
#define HAVE_SYS_POLL_H 1
#endif

/* Define to 1 if you have the <sys/select.h> header file. */
#ifdef UNX
#define HAVE_SYS_SELECT_H 1
#endif

/* Define to 1 if you have the <sys/socket.h> header file. */
#ifdef UNX
#define HAVE_SYS_SOCKET_H 1
#endif

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#ifdef UNX
#define HAVE_SYS_TIME_H 1
#endif

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define if the timezone global is available */
#ifndef MACOSX
#define HAVE_TIMEZONE 1
#endif

/* Use trio printf replacement library */
/* #undef HAVE_TRIO */

/* Define to 1 if you have the <trio.h> header file. */
/* #undef HAVE_TRIO_H */

/* Define to 1 if you have the <unistd.h> header file. */
#ifdef UNX
#define HAVE_UNISTD_H 1
#endif

/* Define to 1 if you have the `usleep' function. */
#ifdef UNX
#define HAVE_USLEEP 1
#endif

/* Define to 1 if you have the `vsnprintf' function. */
#if defined( UNX ) || (defined(_MSC_VER) && (_MSC_VER > 1400))
#define HAVE_VSNPRINTF 1
#endif

#ifdef WIN32
#if (defined(_MSC_VER) && (_MSC_VER < 1500)) || defined(__MINGW32__)
#define vsnprintf _vsnprintf
#endif
#endif

/* Define to be location of localedir */
/* #undef LOCALEDIR */

/* Defined when neon is built as a library */
#define NEON_IS_LIBRARY 1

/* Define to be the neon version string */
#define NEON_VERSION "0.29.5"

/* Define to enable debugging */
/* #undef NE_DEBUGGING */

/* Define to be printf format string for off64_t */
#define NE_FMT_OFF64_T "lld"

/* Define to be printf format string for off_t */
#define NE_FMT_OFF_T "ld"

/* Define to be printf format string for size_t */
#define NE_FMT_SIZE_T "u"

/* Define to be printf format string for ssize_t */
#define NE_FMT_SSIZE_T "d"

/* Define to be printf format string for time_t */
#define NE_FMT_TIME_T "ld"

/* Defined if DAV is supported */
#define NE_HAVE_DAV 1

/* Defined if I18N is supported */
/* #undef NE_HAVE_I18N 1 */

/* Defined if IPV6 is supported */
#define NE_HAVE_IPV6 1

/* Defined if LFS is supported */
#define NE_HAVE_LFS 1

/* Defined if SOCKS is supported */
/* #undef NE_HAVE_SOCKS */

/* Defined if SSL is supported */
#define NE_HAVE_SSL

/* Defined if TS_SSL is supported */
/* #undef NE_HAVE_TS_SSL */

/* Defined if ZLIB is supported */
#define NE_HAVE_ZLIB 1

/* Define to be filename of an SSL CA root bundle */
/* #undef NE_SSL_CA_BUNDLE */

/* Define if poll() should be used */
#ifdef UNX
#define NE_USE_POLL 1
#endif

/* Define to be neon library major version */
#define NE_VERSION_MAJOR (0)

/* Define to be neon library minor version */
#define NE_VERSION_MINOR (29)

/* Define to be neon library patch version */
#define NE_VERSION_PATCH (5)

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "neon@webdav.org"

/* Define to the full name of this package. */
#define PACKAGE_NAME "neon"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "neon 0.29.5"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "neon"

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.29.5"

/* The size of a `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of a `long', as computed by sizeof. */
#define SIZEOF_LONG 4

/* The size of a `long long', as computed by sizeof. */
#define SIZEOF_LONG_LONG 8

/* The size of a `off64_t', as computed by sizeof. */
#define SIZEOF_OFF64_T 8

/* The size of a `off_t', as computed by sizeof. */
#define SIZEOF_OFF_T 4

/* The size of a `size_t', as computed by sizeof. */
#define SIZEOF_SIZE_T 4

/* The size of a `ssize_t', as computed by sizeof. */
#define SIZEOF_SSIZE_T 4

/* The size of a `time_t', as computed by sizeof. */
#define SIZEOF_TIME_T 4

/* Define to 1 if you have the ANSI C header files. */
#ifdef SOLARIS
#define STDC_HEADERS 1
#endif

/* Define to 1 if strerror_r returns char *. */
#ifdef LINUX
#define STRERROR_R_CHAR_P 1
#endif

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* Define if getaddrinfo supports AI_ADDRCONFIG */
/* #undef USE_GAI_ADDRCONFIG */

/* Define if getaddrinfo() should be used */
#define USE_GETADDRINFO 1

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
#if defined (SOLARIS) && defined (SPARC)
#define WORDS_BIGENDIAN 1
#endif

/* Always defined to enable GNU extensions */
#define _GNU_SOURCE 1

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define if in_addr_t is not available */
/* #undef in_addr_t */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#if defined (SOLARIS) && defined (__SUNPRO_C)
#define inline
#endif
#endif

/* Define to `long' if <sys/types.h> does not define. */
/* #undef off_t */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef pid_t */

/* Define to `unsigned' if <sys/types.h> does not define. */
/* #undef size_t */

/* MS IIS does not send Lock-Token response header after creation of a */
/* new lock. This violates RFC, but... if we want to talk with this   */
/* beast we need a workaround */
#define IIS_LOCK_BUG_WORKAROUND 1

/* Enable leak-tracking versions of ne_*alloc when NEON_MEMLEAK is enabled */
#ifdef NEON_MEMLEAK
# include "memleak.h"
#endif

#if defined(HAVE_STPCPY) && defined(HAVE_DECL_STPCPY) && !HAVE_DECL_STPCPY && !defined(stpcpy)
char *stpcpy(char *, const char *);
#endif

#ifdef WIN32

#include <io.h>

#define HAVE_MEMCPY 1

#define strncasecmp strnicmp
#define inline      __inline
#define WIN32_LEAN_AND_MEAN
#define NOUSER
#define NOGDI
#define NONLS
#define NOCRYPT

#endif

#ifdef WIN32
#define HAVE_SSPI 1
#else
#define HAVE_NTLM 1
#endif
