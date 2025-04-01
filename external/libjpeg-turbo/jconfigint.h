#include <sal/types.h>

/* libjpeg-turbo build number */
  "20230315"

/* Compiler's inline keyword */
#undef inline

/* How to obtain function inlining. */
#if defined _MSC_VER
 __forceinline
#elif defined __GNUC__
 __attribute__((always_inline)) inline
#else
 inline
#endif

/* Define to the full name of this package. */
  "libjpeg-turbo"

/* Version number of package */
  "2.1.5.1"

/* The size of `size_t', as computed by sizeof. */
/* #undef SIZEOF_SIZE_T */

/* How to obtain thread-local storage */


#if defined(__GNUC__) && SAL_TYPES_SIZEOFLONG == SIZEOF_SIZE_T
/* Define if your compiler has __builtin_ctzl() and sizeof(unsigned long) == sizeof(size_t). */

#endif

/* Define to 1 if you have the <intrin.h> header file. */
/* #undef HAVE_INTRIN_H */

#if defined(_MSC_VER) && defined(HAVE_INTRIN_H)
#if (SIZEOF_SIZE_T == 8)

#elif (SIZEOF_SIZE_T == 4)

#endif
#endif

#if defined(__has_attribute)
#if __has_attribute(fallthrough)
  __attribute__((fallthrough));
#else

#endif
#else

#endif
