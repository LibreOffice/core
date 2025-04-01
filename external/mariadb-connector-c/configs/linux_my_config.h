
/*
 * Include file constants (processed in LibmysqlIncludeFiles.txt 1
 */
/* #undef HAVE_OPENSSL_APPLINK_C */
 1
/* #undef HAVE_BIGENDIAN */
 1
 1
 1
 1
 1
 1
 1
/* #undef HAVE_SELECT_H */
 1
 1
 1
 1
 1
 1
 1
/* #undef HAVE_SYS_STREAM_H */
 1
 1
 1
 1

/*
 * function definitions - processed in LibmysqlFunctions.txt 
 */

/* #undef HAVE_DLERROR */
/* #undef HAVE_DLOPEN */
 1
 1
 1
/* #undef HAVE_STRTOK_R */
/* #undef HAVE_STRTOL */
/* #undef HAVE_STRTOLL */
/* #undef HAVE_STRTOUL */
/* #undef HAVE_STRTOULL */
/* #undef HAVE_TELL */
/* #undef HAVE_THR_SETCONCURRENCY */
/* #undef HAVE_THR_YIELD */
/* #undef HAVE_VASPRINTF */
/* #undef HAVE_VSNPRINTF */
 1

/*
 * types and sizes
 */


 8
#if defined(SIZEOF_CHARP)
# define HAVE_CHARP 1
#endif


 4
#if defined(SIZEOF_INT)
# define HAVE_INT 1
#endif

 8
#if defined(SIZEOF_LONG)
# define HAVE_LONG 1
#endif

 8
#if defined(SIZEOF_LONG_LONG)
# define HAVE_LONG_LONG 1
#endif


 8
#if defined(SIZEOF_SIZE_T)
# define HAVE_SIZE_T 1
#endif


 4
#if defined(SIZEOF_UINT)
# define HAVE_UINT 1
#endif

 8
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

 socklen_t

  0
   1
 2
 LOCAL_INFILE_MODE_AUTO

 "latin1"

