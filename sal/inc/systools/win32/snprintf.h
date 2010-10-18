#ifndef _SNPRINTF_H
#define _SNPRINTF_H

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

/* Macros for Unicode/ANSI support like in TCHAR.H */

#ifdef _UNICODE
#define sntprintf   snwprintf
#define vsntprintf  vsnwprintf
#else
#define sntprintf   snprintf
#define vsntprintf  vsnprintf
#endif

/* Define needed types if they are not yet defined */


#   ifndef _VA_LIST_DEFINED
    typedef char *  va_list;
#   define _VA_LIST_DEFINED
#   endif


#   ifndef _WCHAR_T_DEFINED
    typedef unsigned short wchar_t;
#   define _WCHAR_T_DEFINED
#   endif


#ifndef _SNPRINTF_DLLIMPORT
#define _SNPRINTF_DLLIMPORT __declspec( dllimport )
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*  Implementations of snprintf following the ISO/IEC 9899:1999 (ISO C99)
    standard.
    The difference compared to _snprintf is that the buffer always is zero
    terminated (unless count is zero) and the return value is the number of
    characters (not including terminating zero) that would have been written
    even if the buffer wasn't large
    enough to hold the string. */



/* UNICODE version */
_SNPRINTF_DLLIMPORT int __cdecl snwprintf( wchar_t *buffer, size_t count, const wchar_t *format, ... );

/* SBCS and MBCS version */
_SNPRINTF_DLLIMPORT int __cdecl snprintf( char *buffer, size_t count, const char *format, ... );

/* Conflict with STL_port inline implementation */

#ifdef __cplusplus
}
#endif

#endif /* _SNPRINTF_H */
