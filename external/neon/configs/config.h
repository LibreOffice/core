/* Contents kept in sync with config.h.in from neon 0.29.5 */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

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
/* #undef HAVE_BIND_TEXTDOMAIN_CODESET */

/* Define to 1 if you have the `CRYPTO_set_idptr_callback' function. */
/* #undef HAVE_CRYPTO_SET_IDPTR_CALLBACK */

/* Define to 1 if you have the declaration of `h_errno', and to 0 if you
   don't. */
#ifdef _WIN32
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

/* Define to 1 if you have the `fcntl' function. */
#ifdef UNX
#define HAVE_FCNTL 1
#endif

/* Define to 1 if you have the <fcntl.h> header file. */
#ifdef UNX
#define HAVE_FCNTL_H 1
#endif

/* Define to 1 if you have the `fstat64' function. */
#define HAVE_FSTAT64

/* Define to 1 if you have the `gai_strerror' function. */
#define HAVE_GAI_STRERROR 1

/* Define to 1 if you have the `gethostname' function. */
/* #undef HAVE_GETHOSTNAME */

/* Define to 1 if you have the `getnameinfo' function. */
/* #undef HAVE_GETNAMEINFO */

/* Define to 1 if you have the `getsockopt' function. */
#ifdef UNX
#define HAVE_GETSOCKOPT 1
#endif

#ifdef DISABLE_OPENSSL
/* Define if GnuTLS support is enabled */
#define HAVE_GNUTLS

/* Define to 1 if you have the `gnutls_certificate_get_x509_cas' function. */
/* #undef HAVE_GNUTLS_CERTIFICATE_GET_X509_CAS */

/* Define to 1 if you have the `gnutls_certificate_verify_peers2' function. */
#define HAVE_GNUTLS_CERTIFICATE_VERIFY_PEERS2

/* Define to 1 if you have the `gnutls_session_get_data2' function. */
#define HAVE_GNUTLS_SESSION_GET_DATA2

/* Define to 1 if you have the `gnutls_sign_callback_set' function. */
#define HAVE_GNUTLS_SIGN_CALLBACK_SET

/* Define to 1 if you have the `gnutls_x509_dn_get_rdn_ava' function. */
#define HAVE_GNUTLS_X509_DN_GET_RDN_AVA
#endif /* DISABLE_OPENSSL */

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

/* Define to 1 if you have the `iconv' function. */
/* #undef HAVE_ICONV */

/* Define to 1 if you have the <iconv.h> header file. */
/* #undef HAVE_ICONV_H */

/* Define to 1 if you have the `inet_ntop' function. */
#ifdef UNX
#define HAVE_INET_NTOP 1
#endif

/* Define to 1 if you have the `inet_pton' function. */
/* #undef HAVE_INET_PTON */

/* Define to 1 if you have the <inttypes.h> header file. */
#ifdef UNX
#define HAVE_INTTYPES_H 1
#endif

/* Define to 1 if you have the `isatty' function. */
#define HAVE_ISATTY 1

/* Define to 1 if you have the <libintl.h> header file. */
/* #undef HAVE_LIBINTL_H */

/* Define if libproxy is supported */
/* #undef HAVE_LIBPROXY */

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

#ifndef DISABLE_OPENSSL
/* Define if NTLM is supported */
#ifndef _WIN32
#define HAVE_NTLM 1
#endif

/* Define if OpenSSL support is enabled */
#define HAVE_OPENSSL 1

/* Define to 1 if you have the <openssl/opensslv.h> header file. */
#define HAVE_OPENSSL_OPENSSLV_H 1

/* Define to 1 if you have the <openssl/ssl.h> header file. */
#define HAVE_OPENSSL_SSL_H 1
#endif /* !DISABLE_OPENSSL */

/* Define if pakchois library supported */
/* #undef HAVE_PAKCHOIS */

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
#if defined (LINUX) || defined (_WIN32)
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
#if defined( UNX ) || defined(_MSC_VER)
#define HAVE_SNPRINTF 1
#endif

/* Define to 1 if the system has the type `socklen_t'. */
#ifdef UNX
#define HAVE_SOCKLEN_T 1
#endif

/* Define to 1 if you have the `SSL_SESSION_cmp' function. */
/* #undef HAVE_SSL_SESSION_CMP */

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

#ifdef _WIN32
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
#if defined(_MSC_VER)
#define timezone _timezone
#endif
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
#if defined(UNX) || defined(_MSC_VER)
#define HAVE_VSNPRINTF 1
#endif

#ifdef __MINGW32__
#define vsnprintf _vsnprintf
#endif

/* Define to be location of localedir */
/* #undef LOCALEDIR */

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
/* #undef LT_OBJDIR */

/* Defined when neon is built as a library */
#define NEON_IS_LIBRARY 1

/* Define to be the neon version string */
#define NEON_VERSION "0.29.5"

/* Define to enable debugging */
/* #undef NE_DEBUGGING */

/* Define to be printf format string for ne_off_t */
/* #undef NE_FMT_NE_OFF_T */

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

/* Define to be printf format string for XML_Size */
/* #undef NE_FMT_XML_SIZE */

/* Defined if DAV is supported */
#define NE_HAVE_DAV 1

/* Defined if I18N is supported */
/* #undef NE_HAVE_I18N */

/* Defined if IPV6 is supported */
#define NE_HAVE_IPV6 1

/* Defined if LFS is supported */
#define NE_HAVE_LFS 1

/* Defined if LIBPXY is supported */
/* #undef NE_HAVE_LIBPXY */

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

/* The size of `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of `long', as computed by sizeof. */
#define SIZEOF_LONG 4

/* The size of `long long', as computed by sizeof. */
#define SIZEOF_LONG_LONG 8

/* The size of `off64_t', as computed by sizeof. */
#define SIZEOF_OFF64_T 8

/* The size of `off_t', as computed by sizeof. */
#define SIZEOF_OFF_T 4

/* The size of `size_t', as computed by sizeof. */
#define SIZEOF_SIZE_T 4

/* The size of `ssize_t', as computed by sizeof. */
#define SIZEOF_SSIZE_T 4

/* The size of `time_t', as computed by sizeof. */
#define SIZEOF_TIME_T 4

/* The size of `XML_Size', as computed by sizeof. */
/* #undef SIZEOF_XML_SIZE */

/* Define to 1 if you have the ANSI C header files. */
#ifdef __sun
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

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# undef _ALL_SOURCE
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# undef _GNU_SOURCE
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# undef _POSIX_PTHREAD_SEMANTICS
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# undef _TANDEM_SOURCE
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# undef __EXTENSIONS__
#endif


/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined (__sun) && defined (SPARC)
#define WORDS_BIGENDIAN 1
#endif

/* Always defined to enable GNU extensions */
#define _GNU_SOURCE 1

/* Define to 1 if on MINIX. */
/* #undef _MINIX */

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
/* #undef _POSIX_1_SOURCE */

/* Define to 1 if you need to in order for `stat' and other things to work. */
/* #undef _POSIX_SOURCE */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define if in_addr_t is not available */
/* #undef in_addr_t */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#if defined (__sun) && defined (__SUNPRO_C)
#define inline
#endif
#endif

/* Define to `long int' if <sys/types.h> does not define. */
/* #undef off_t */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef pid_t */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define if socklen_t is not available */
/* #undef socklen_t */

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

#ifdef _WIN32

#include <io.h>

#define HAVE_MEMCPY 1

#define strncasecmp strnicmp
#define inline      __inline
#define WIN32_LEAN_AND_MEAN
#define NOUSER
#define NOGDI
#define NONLS
#define NOCRYPT

#define HAVE_SSPI 1

#endif
