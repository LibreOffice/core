#include <sal/types.h>

/* libjpeg-turbo build number */
#define BUILD  "20230315"

/* How to hide global symbols. */
#if defined __GNUC__
#define HIDDEN  __attribute__((visibility("hidden")))
#endif

/* Compiler's inline keyword */
#undef inline

/* How to obtain function inlining. */
#if defined _MSC_VER
#define INLINE __forceinline
#elif defined __GNUC__
#define INLINE __attribute__((always_inline)) inline
#else
#define INLINE inline
#endif

/* Define to the full name of this package. */
#define PACKAGE_NAME  "libjpeg-turbo"

/* Version number of package */
#define VERSION  "3.0.4"

/* The size of `size_t', as computed by sizeof. */
/* #undef SIZEOF_SIZE_T */

/* How to obtain thread-local storage */
#define THREAD_LOCAL

#if defined(__GNUC__) && SAL_TYPES_SIZEOFLONG == SIZEOF_SIZE_T
/* Define if your compiler has __builtin_ctzl() and sizeof(unsigned long) == sizeof(size_t). */
#define HAVE_BUILTIN_CTZL
#endif

/* Define to 1 if you have the <intrin.h> header file. */
/* #undef HAVE_INTRIN_H */

#if defined(_MSC_VER) && defined(HAVE_INTRIN_H)
#if (SIZEOF_SIZE_T == 8)
#define HAVE_BITSCANFORWARD64
#elif (SIZEOF_SIZE_T == 4)
#define HAVE_BITSCANFORWARD
#endif
#endif

#if defined(__has_attribute)
#if __has_attribute(fallthrough)
#define FALLTHROUGH  __attribute__((fallthrough));
#else
#define FALLTHROUGH
#endif
#else
#define FALLTHROUGH
#endif
