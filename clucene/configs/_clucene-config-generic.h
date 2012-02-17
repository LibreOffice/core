#ifndef _SRC_CLUCENE_INTERNAL_CLUCENE_CONFIG_H
#define _SRC_CLUCENE_INTERNAL_CLUCENE_CONFIG_H 1
 
/* src/shared/CLucene/_clucene-config.h. 
*  Generated automatically at end of cmake.
*  These are internal definitions, and this file does not need to be distributed
*/

/* CMake will look for these functions: */
/* #undef _CL_HAVE_FUNCTION__VSNWPRINTF */
/* #undef _CL_HAVE_FUNCTION__SNWPRINTF */
#define _CL_HAVE_FUNCTION_WCSCASECMP
#define _CL_HAVE_FUNCTION_WCSCAT  1 
#define _CL_HAVE_FUNCTION_WCSCHR  1 
#define _CL_HAVE_FUNCTION_WCSCMP  1 
#define _CL_HAVE_FUNCTION_WCSCPY  1 
#define _CL_HAVE_FUNCTION_WCSCSPN  1 
/* #undef _CL_HAVE_FUNCTION_WCSICMP */
#define _CL_HAVE_FUNCTION_WCSLEN  1 
#define _CL_HAVE_FUNCTION_WCSNCMP  1 
#define _CL_HAVE_FUNCTION_WCSNCPY  1 
#define _CL_HAVE_FUNCTION_WCSSTR  1 
#define _CL_HAVE_FUNCTION_WCSTOD 1
#define _CL_HAVE_FUNCTION_WCSDUP 1
#define _CL_HAVE_FUNCTION_WCSTOLL 1
/* #undef _CL_HAVE_FUNCTION_WCSUPR */
#define _CL_HAVE_FUNCTION_GETTIMEOFDAY 1
/* #undef _CL_HAVE_FUNCTION_MAPVIEWOFFILE */

/* #undef _CL_HAVE_FUNCTION_LLTOA */
/* #undef _CL_HAVE_FUNCTION_LLTOW */
#define _CL_HAVE_FUNCTION_PRINTF  1 
#define _CL_HAVE_FUNCTION_SNPRINTF  1 
#define _CL_HAVE_FUNCTION_MMAP  1 
/* #undef _CL_HAVE_FUNCTION_STRLWR */
#define _CL_HAVE_FUNCTION_STRTOLL 1
/* #undef _CL_HAVE_FUNCTION_STRUPR */
/* #undef _CL_HAVE_FUNCTION_GETPAGESIZE */
#define _CL_HAVE_FUNCTION_USLEEP 1
/* #undef _CL_HAVE_FUNCTION_SLEEP */

#define CL_MAX_PATH 4096
//this is the max filename... for now its just the same,
//but this could change, so we use a different name
#define CL_MAX_NAME CL_MAX_PATH
//this used to be CL_MAX_NAME * 32, but as Alex Hudson points out, this could come to be 128kb.
//the above logic for CL_MAX_NAME should be correct enough to handle all file names
#define CL_MAX_DIR CL_MAX_PATH

#define _O_RANDOM 0
#define _O_BINARY 0
#define _S_IREAD S_IREAD
#define _S_IWRITE S_IWRITE
#define _timeb timeb

#define _ILONG(x) x ## L
#define _ILONGLONG(x) x ## LL

#define fileStat stat64
#define cl_stat_t stat64
#define fileSize CL_NS(util)::Misc::filelength
#define fileSeek lseek64
#define fileTell(fhandle) fileSeek(fhandle, 0, SEEK_CUR)
#define fileHandleStat fstat64
#define _realpath realpath
#define _rename rename
#define _close close
#define _read read
#define _cl_open open
#define _write write
#define _snprintf snprintf
#define _mkdir(x) mkdir(x,0777)
#define _unlink unlink
#define _ftime ftime
#define SLEEPFUNCTION usleep

/* CMake will determine these specifics. Things like bugs, etc */

/* Does not support new float byte<->float conversions */
/* #undef _CL_HAVE_NO_FLOAT_BYTE */

/* Define if recursive pthread mutexes are available */
#define _CL_HAVE_PTHREAD_MUTEX_RECURSIVE  1 

/** define if you would like to force clucene to use the internal
* character functions.
* Tests may display unpredictable behaviour if this is not defined.
*/
#ifndef LUCENE_USE_INTERNAL_CHAR_FUNCTIONS
	#define LUCENE_USE_INTERNAL_CHAR_FUNCTIONS 1
#endif

/** fix ansi for loop scope */
#if 1==0
 #define for if (0); else for
#endif


/* Compiler oddities */

//not sure why, but cygwin reports _S_IREAD, but doesn't actually work...
//TODO: make this work properly (this bit shouldn't be necessary)
#ifdef __CYGWIN__
    #define _S_IREAD 0333
    #define _S_IWRITE 0333
#endif

#ifdef __BORLANDC__ //borland compiler
    #define O_RANDOM 0
#endif

#endif
