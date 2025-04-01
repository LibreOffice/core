#pragma once
 1

/* src/shared/CLucene/_clucene-config.h.
*  Normally generated automatically at end of cmake,
*  but here in LibreOffice this actually is a copy of
*  clucene/configs/_clucene-config-generic.h.
*  These are internal definitions, and this file does not need to be distributed
*/

/* CMake will look for these functions: */
/* #undef _CL_HAVE_FUNCTION__VSNWPRINTF */
/* #undef _CL_HAVE_FUNCTION__SNWPRINTF */

  1
  1
  1
  1
  1
/* #undef _CL_HAVE_FUNCTION_WCSICMP */
  1
  1
  1
  1
 1
/* #undef _CL_HAVE_FUNCTION_WCSDUP 1 */
 1
/* #undef _CL_HAVE_FUNCTION_WCSUPR */
 1
/* #undef _CL_HAVE_FUNCTION_MAPVIEWOFFILE */

/* #undef _CL_HAVE_FUNCTION_LLTOA */
/* #undef _CL_HAVE_FUNCTION_LLTOW */
  1
  1
  1
/* #undef _CL_HAVE_FUNCTION_STRLWR */
 1
/* #undef _CL_HAVE_FUNCTION_STRUPR */
/* #undef _CL_HAVE_FUNCTION_GETPAGESIZE */
 1
/* #undef _CL_HAVE_FUNCTION_SLEEP */

 4096
//this is the max filename... for now its just the same,
//but this could change, so we use a different name
 CL_MAX_PATH
//this used to be CL_MAX_NAME * 32, but as Alex Hudson points out, this could come to be 128kb.
//the above logic for CL_MAX_NAME should be correct enough to handle all file names
 CL_MAX_PATH

 0
 0
 S_IREAD
 S_IWRITE
 timeb

(x) x ## L
(x) x ## LL

 stat
 stat
 CL_NS(util)::Misc::filelength
 lseek
(fhandle) fileSeek(fhandle, 0, SEEK_CUR)
 fstat
 realpath
 rename
 close
 read
 open
 write
 snprintf
(x) mkdir(x,0777)
 unlink
 ftime
 usleep

/* CMake will determine these specifics. Things like bugs, etc */

/* Does not support new float byte<->float conversions */
/* #undef _CL_HAVE_NO_FLOAT_BYTE */

/* Define if recursive pthread mutexes are available */
  1

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
