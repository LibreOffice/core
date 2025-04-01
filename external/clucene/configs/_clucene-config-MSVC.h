#pragma once
 1

/* src/shared/CLucene/_clucene-config.h.
*  Normally generated automatically at end of cmake,
*  but here in LibreOffice this actually is a copy of
*  clucene/configs/_clucene-config-MSVC.h.
*  These are internal definitions, and this file does not need to be distributed
*/

/* CMake will look for these functions: */


/* #undef _CL_HAVE_FUNCTION_WCSCASECMP */
  1
  1
  1
  1
  1

  1
  1
  1
  1
 1
 1
/* #undef _CL_HAVE_FUNCTION_WCSTOLL */
 1
/* #undef _CL_HAVE_FUNCTION_GETTIMEOFDAY */
 1

/* #undef _CL_HAVE_FUNCTION_LLTOA */
/* #undef _CL_HAVE_FUNCTION_LLTOW */
  1
/* #undef _CL_HAVE_FUNCTION_SNPRINTF */
/* #undef _CL_HAVE_FUNCTION_MMAP */
 1
/* #undef _CL_HAVE_FUNCTION_STRTOLL */
 1
/* #undef _CL_HAVE_FUNCTION_GETPAGESIZE */
/* #undef _CL_HAVE_FUNCTION_USLEEP */
 1

 4096
//this is the max filename... for now its just the same,
//but this could change, so we use a different name
 CL_MAX_PATH
//this used to be CL_MAX_NAME * 32, but as Alex Hudson points out, this could come to be 128kb.
//the above logic for CL_MAX_NAME should be correct enough to handle all file names
 CL_MAX_PATH

/* undef _O_RANDOM _O_RANDOM */
/* undef _O_BINARY _O_BINARY */
/* undef _S_IREAD _S_IREAD */
/* undef _S_IWRITE _S_IWRITE */
/* #undef _timeb */

(x) x ## L
(x) x ## LL

 _stati64
 _stati64
 _filelengthi64
 _lseeki64
 _telli64
 _fstati64
(rel,abs) ::_fullpath(abs,rel,CL_MAX_PATH)
 rename
/* undef _close _close */
/* undef _read _read */
 _open
/* undef _write _write */
/* undef _snprintf _snprintf */
/* undef _mkdir _mkdir */
/* undef _unlink _unlink */
/* undef _ftime _ftime */
 Sleep

/* CMake will determine these specifics. Things like bugs, etc */

/* Does not support new float byte<->float conversions */
/* #undef _CL_HAVE_NO_FLOAT_BYTE */

/* Define if recursive pthread mutexes are available */
/* #undef _CL_HAVE_PTHREAD_MUTEX_RECURSIVE */

/** define if you would like to force clucene to use the internal
* character functions.
* Tests may display unpredictable behaviour if this is not defined.
*/
#pragma once
	 1
#endif

/** fix ansi for loop scope */
#if 1==0
  if (0); else for
#endif


/* Compiler oddities */

//not sure why, but cygwin reports _S_IREAD, but doesn't actually work...
//TODO: make this work properly (this bit shouldn't be necessary)
#ifdef __CYGWIN__
     0333
     0333
#endif

#ifdef __BORLANDC__ //borland compiler
     0
#endif

#endif
