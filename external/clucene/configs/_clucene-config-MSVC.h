#ifndef _SRC_CLUCENE_INTERNAL_CLUCENE_CONFIG_H
#define _SRC_CLUCENE_INTERNAL_CLUCENE_CONFIG_H 1

/* src/shared/CLucene/_clucene-config.h.
*  Normally generated automatically at end of cmake,
*  but here in LibreOffice this actually is a copy of
*  clucene/configs/_clucene-config-MSVC.h.
*  These are internal definitions, and this file does not need to be distributed
*/

/* CMake will look for these functions: */
#define _CL_HAVE_FUNCTION__VSNWPRINTF
#define _CL_HAVE_FUNCTION__SNWPRINTF
/* #undef _CL_HAVE_FUNCTION_WCSCASECMP */
#define _CL_HAVE_FUNCTION_WCSCAT  1
#define _CL_HAVE_FUNCTION_WCSCHR  1
#define _CL_HAVE_FUNCTION_WCSCMP  1
#define _CL_HAVE_FUNCTION_WCSCPY  1
#define _CL_HAVE_FUNCTION_WCSCSPN  1
#define _CL_HAVE_FUNCTION_WCSICMP
#define _CL_HAVE_FUNCTION_WCSLEN  1
#define _CL_HAVE_FUNCTION_WCSNCMP  1
#define _CL_HAVE_FUNCTION_WCSNCPY  1
#define _CL_HAVE_FUNCTION_WCSSTR  1
#define _CL_HAVE_FUNCTION_WCSTOD 1
#define _CL_HAVE_FUNCTION_WCSDUP 1
/* #undef _CL_HAVE_FUNCTION_WCSTOLL */
#define _CL_HAVE_FUNCTION_WCSUPR 1
/* #undef _CL_HAVE_FUNCTION_GETTIMEOFDAY */
#define _CL_HAVE_FUNCTION_MAPVIEWOFFILE 1

/* #undef _CL_HAVE_FUNCTION_LLTOA */
/* #undef _CL_HAVE_FUNCTION_LLTOW */
#define _CL_HAVE_FUNCTION_PRINTF  1
/* #undef _CL_HAVE_FUNCTION_SNPRINTF */
/* #undef _CL_HAVE_FUNCTION_MMAP */
#define _CL_HAVE_FUNCTION_STRLWR 1
/* #undef _CL_HAVE_FUNCTION_STRTOLL */
#define _CL_HAVE_FUNCTION_STRUPR 1
/* #undef _CL_HAVE_FUNCTION_GETPAGESIZE */
/* #undef _CL_HAVE_FUNCTION_USLEEP */
#define _CL_HAVE_FUNCTION_SLEEP 1

#define CL_MAX_PATH 4096
//this is the max filename... for now its just the same,
//but this could change, so we use a different name
#define CL_MAX_NAME CL_MAX_PATH
//this used to be CL_MAX_NAME * 32, but as Alex Hudson points out, this could come to be 128kb.
//the above logic for CL_MAX_NAME should be correct enough to handle all file names
#define CL_MAX_DIR CL_MAX_PATH

/* undef _O_RANDOM _O_RANDOM */
/* undef _O_BINARY _O_BINARY */
/* undef _S_IREAD _S_IREAD */
/* undef _S_IWRITE _S_IWRITE */
/* #undef _timeb */

#define _ILONG(x) x ## L
#define _ILONGLONG(x) x ## LL

#define fileStat _stati64
#define cl_stat_t _stati64
#define fileSize _filelengthi64
#define fileSeek _lseeki64
#define fileTell _telli64
#define fileHandleStat _fstati64
#define _realpath(rel,abs) ::_fullpath(abs,rel,CL_MAX_PATH)
#define _rename rename
/* undef _close _close */
/* undef _read _read */
#define _cl_open _open
/* undef _write _write */
/* undef _snprintf _snprintf */
/* undef _mkdir _mkdir */
/* undef _unlink _unlink */
/* undef _ftime _ftime */
#define SLEEPFUNCTION Sleep

/* CMake will determine these specifics. Things like bugs, etc */

/* Does not support new float byte<->float conversions */
/* #undef _CL_HAVE_NO_FLOAT_BYTE */

/* Define if recursive pthread mutexes are available */
/* #undef _CL_HAVE_PTHREAD_MUTEX_RECURSIVE */

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
