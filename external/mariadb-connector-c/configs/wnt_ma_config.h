
/*
 * Include file constants (processed in LibmysqlIncludeFiles.txt 1
 */
/* #undef HAVE_OPENSSL_APPLINK_C */
/* #undef HAVE_ALLOCA_H */
/* #undef HAVE_BIGENDIAN */
 1
/* #undef HAVE_NL_LANGINFO */
/* #undef HAVE_DLFCN_H */
 1
 1
 1
/* #undef HAVE_PWD_H */
/* #undef HAVE_SELECT_H */
 1
/* #undef HAVE_STDINT_H */
 1
 1
/* #undef HAVE_SYS_IOCTL_H */
/* #undef HAVE_SYS_SELECT_H */
/* #undef HAVE_SYS_SOCKET_H */
/* #undef HAVE_SYS_STREAM_H */
 1
/* #undef HAVE_SYS_UN_H */
/* #undef HAVE_UNISTD_H */
/* #undef HAVE_UCONTEXT_H */

/*
 * function definitions - processed in LibmysqlFunctions.txt 
 */

/* #undef HAVE_DLERROR */
/* #undef HAVE_DLOPEN */
/* #undef HAVE_GETPWUID */
 1
/* #undef HAVE_POLL */
/* #undef HAVE_STRTOK_R */
 1
/* #undef HAVE_STRTOLL */
 1
/* #undef HAVE_STRTOULL */
 1
/* #undef HAVE_THR_SETCONCURRENCY */
/* #undef HAVE_THR_YIELD */
/* #undef HAVE_VASPRINTF */
 1
/* #undef HAVE_CUSERID */

/*
 * types and sizes
 */


#ifdef _M_X64
 8
#else
 4
#endif
#if defined(SIZEOF_CHARP)
# define HAVE_CHARP 1
#endif


 4
#if defined(SIZEOF_INT)
# define HAVE_INT 1
#endif

 4
#if defined(SIZEOF_LONG)
# define HAVE_LONG 1
#endif

 8
#if defined(SIZEOF_LONG_LONG)
# define HAVE_LONG_LONG 1
#endif


#ifdef _M_X64
 8
#else
 4
#endif
#if defined(SIZEOF_SIZE_T)
# define HAVE_SIZE_T 1
#endif


/* #undef SIZEOF_UINT */
#if defined(SIZEOF_UINT)
# define HAVE_UINT 1
#endif

/* #undef SIZEOF_ULONG */
#if defined(SIZEOF_ULONG)
# define HAVE_ULONG 1
#endif

/* #undef SIZEOF_INT8 */
#if defined(SIZEOF_INT8)
# define HAVE_INT8 1
#endif
/* #undef SIZEOF_UINT8 */
#if defined(SIZEOF_UINT8)
# define HAVE_UINT8 1
#endif

/* #undef SIZEOF_INT16 */
#if defined(SIZEOF_INT16)
# define HAVE_INT16 1
#endif
/* #undef SIZEOF_UINT16 */
#if defined(SIZEOF_UINT16)
# define HAVE_UINT16 1
#endif

/* #undef SIZEOF_INT32 */
#if defined(SIZEOF_INT32)
# define HAVE_INT32 1
#endif
/* #undef SIZEOF_UINT32 */
#if defined(SIZEOF_UINT32)
# define HAVE_UINT32 1
#endif

/* #undef SIZEOF_INT64 */
#if defined(SIZEOF_INT64)
# define HAVE_INT64 1
#endif
/* #undef SIZEOF_UINT64 */
#if defined(SIZEOF_UINT64)
# define HAVE_UINT64 1
#endif

/* #undef SIZEOF_SOCKLEN_T */
#if defined(SIZEOF_SOCKLEN_T)
# define HAVE_SOCKLEN_T 1
#endif

 int

  0
   1
 2
 LOCAL_INFILE_MODE_AUTO

 "latin1"

