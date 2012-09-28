#ifndef _SRC_CLUCENE_CLUCENE_CONFIG_H
#define _SRC_CLUCENE_CLUCENE_CONFIG_H 1

/* src/shared/CLucene/clucene-config.h.
*  Generated automatically at end of cmake.
*/

/* CMake will look for these headers: */
#define _CL_HAVE_STRING_H  1
#define _CL_HAVE_MEMORY_H  1
/* #undef _CL_HAVE_UNISTD_H */
#define _CL_HAVE_IO_H  1
#define _CL_HAVE_DIRECT_H  1
/* #undef _CL_HAVE_DIRENT_H */
/* #undef _CL_HAVE_SYS_DIR_H */
/* #undef _CL_HAVE_SYS_NDIR_H */
#define _CL_HAVE_ERRNO_H  1
#define _CL_HAVE_WCHAR_H  1
#define _CL_HAVE_WCTYPE_H
#define _CL_HAVE_CTYPE_H  1
#define _CL_HAVE_WINDOWS_H  1
/* #undef _CL_HAVE_WINDEF_H */
#define _CL_HAVE_SYS_TYPES_H  1
/* #undef _CL_HAVE_DLFCN_H */
/* #undef _CL_HAVE_EXT_HASH_MAP */
/* #undef _CL_HAVE_EXT_HASH_SET */
/* #undef _CL_HAVE_TR1_UNORDERED_MAP */
/* #undef _CL_HAVE_TR1_UNORDERED_SET */
/* #undef _CL_HAVE_HASH_MAP */
/* #undef _CL_HAVE_HASH_SET */
/* #undef _CL_HAVE_NDIR_H */
#define _CL_HAVE_SYS_STAT_H  1
#define _CL_HAVE_SYS_TIMEB_H  1
/* #undef _CL_HAVE_SYS_TIME_H */
#define _CL_HAVE_TCHAR_H 1
/* #undef _CL_HAVE_SYS_MMAN_H */
#define _CL_HAVE_WINERROR_H 1
/* #undef _CL_HAVE_STDINT_H */

// our needed types
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long long int64_t;
typedef unsigned long long uint64_t;

/* undef float_t*/
typedef unsigned long _cl_dword_t;
/* undef size_t size_t */

/* tchar & _T definitions... */
/* undef TCHAR TCHAR */
/* #undef _T */

/* CMake will determine these specifics. Things like bugs, etc */

/* if we can't support the map/set hashing */
/* #undef LUCENE_DISABLE_HASHING */

/* Define if you have POSIX threads libraries and header files. */
/* #undef _CL_HAVE_PTHREAD */

/* Define if you have Win32 threads libraries and header files. */
#define _CL_HAVE_WIN32_THREADS  1

/* Define if we have gcc atomic functions */
#define _CL_HAVE_GCC_ATOMIC_FUNCTIONS 1

/* Define what eval method is required for float_t to be defined (for GCC). */
/* #undef _FLT_EVAL_METHOD */

/* If we use hashmaps, which namespace do we use: */
#define CL_NS_HASHING(func)
/* If we use hashmaps, which classes do we use: */
#define _CL_HASH_MAP
#define _CL_HASH_SET

/* define if the compiler implements namespaces */
#define _CL_HAVE_NAMESPACES

/* Defined if the snprintf overflow test fails */
/* #undef _CL_HAVE_SNPRINTF_BUG */

/* Defined if the swprintf test fails */
/* #undef _CL_HAVE_SNWPRINTF_BUG */

/* How to define a static const in a class */
#define LUCENE_STATIC_CONSTANT(type, assignment)  static const type assignment

/* Define to the necessary symbol if this constant uses a non-standard name on
   your system. */
//todo: not checked
/* #undef _CL_PTHREAD_CREATE_JOINABLE */

/* Define to 1 if the `S_IS*' macros in <sys/stat.h> do not work properly. */
//todo: not being checked for...
/* #undef _CL_STAT_MACROS_BROKEN */

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
//not actually used for anything...
/* #undef _CL_TIME_WITH_SYS_TIME */

/* Define that we will be using -fvisibility=hidden, and
 * make public classes visible using __attribute__ ((visibility("default")))
 */
/* #undef _CL_HAVE_GCCVISIBILITYPATCH */


/* Versions, etc */

/* Name of package */
#define _CL_PACKAGE  "clucene-core"

/* Version number of package */
#define _CL_VERSION  "2.3.3.4"

/* So-Version number of package */
#define _CL_SOVERSION  "1"

/* A comparable version number */
#define _CL_INT_VERSION  2030304

/* Configured options (from command line) */

/* Forces into Ascii mode */
/* #undef _ASCII */

/* Conditional Debugging */
/* #undef _CL__CND_DEBUG */

/* debuging option */
/* #undef _DEBUG */

/* Disable multithreading */
/* #undef _CL_DISABLE_MULTITHREADING */


#ifdef __BORLANDC__ //borland compiler
    //todo: bcc incorrectly detects this... fix this in cmake
    #undef LUCENE_STATIC_CONSTANT
    #define LUCENE_STATIC_CONSTANT(type, assignment) enum { assignment }
#endif


#endif
