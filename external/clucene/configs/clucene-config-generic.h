#pragma once
 1

/* src/shared/CLucene/clucene-config.h.
*  Normally generated automatically at end of cmake,
*  but here in LibreOffice this actually is a copy of
*  clucene/configs/clucene-config-generic.h.
*/

/* CMake will look for these headers: */
  1
  1
  1
/* #undef _CL_HAVE_IO_H */
/* #undef _CL_HAVE_DIRECT_H */
  1

/* #undef _CL_HAVE_SYS_NDIR_H */
  1
  1

  1
/* #undef _CL_HAVE_WINDOWS_H */
/* #undef _CL_HAVE_WINDEF_H */
  1
/* #undef _CL_HAVE_DLFCN_H */
  1
/* #undef _CL_HAVE_EXT_HASH_SET */
 1
  1


/* #undef _CL_HAVE_NDIR_H */
  1
  1
 1
/* #undef _CL_HAVE_TCHAR_H */
 1
/* #undef _CL_HAVE_WINERROR_H */
 1

// our needed types
/* undef int8_t int8_t */
/* undef uint8_t uint8_t */
/* undef int16_t int16_t */
/* undef uint16_t uint16_t */
/* undef int32_t int32_t */
/* undef uint32_t uint32_t */
/* undef int64_t int64_t */
/* undef uint64_t uint64_t */

/* undef float_t*/

/* undef size_t size_t */

/* tchar & _T definitions... */
typedef  wchar_t TCHAR;
(x) L ## x

/* CMake will determine these specifics. Things like bugs, etc */

/* if we can't support the map/set hashing */
/* #undef LUCENE_DISABLE_HASHING */

/* Define if you have POSIX threads libraries and header files. */
  1

/* Define if you have Win32 threads libraries and header files. */
/* #undef _CL_HAVE_WIN32_THREADS */

/* Define if we have gcc atomic functions */
/* #undef _CL_HAVE_GCC_ATOMIC_FUNCTIONS */

/* Define what eval method is required for float_t to be defined (for GCC). */
/* #undef _FLT_EVAL_METHOD */

/* If we use hashmaps, which namespace do we use: */
(func) std::tr1::func
/* If we use hashmaps, which classes do we use: */
 unordered_map
 unordered_set

/* define if the compiler implements namespaces */


/* Defined if the snprintf overflow test fails */
/* #undef _CL_HAVE_SNPRINTF_BUG */

/* Defined if the swprintf test fails */
/* #undef _CL_HAVE_SNWPRINTF_BUG */

/* How to define a static const in a class */
(type, assignment)  static const type assignment

/* Define to the necessary symbol if this constant uses a non-standard name on
   your system. */
//todo: not checked
/* #undef _CL_PTHREAD_CREATE_JOINABLE */

/* Define to 1 if the `S_IS*' macros in <sys/stat.h> do not work properly. */
//todo: not being checked for...
/* #undef _CL_STAT_MACROS_BROKEN */

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
//not actually used for anything...
//  1

/* Define that we will be using -fvisibility=hidden, and
 * make public classes visible using __attribute__ ((visibility("default")))
 */
 1


/* Versions, etc */

/* Name of package */
  "clucene-core"

/* Version number of package */
  "2.3.3.4"

/* So-Version number of package */
  "1"

/* A comparable version number */
  2030304

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
    (type, assignment) enum { assignment }
#endif


#endif
