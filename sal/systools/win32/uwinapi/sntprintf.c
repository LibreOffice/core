#define _SNPRINTF_DLLIMPORT __declspec( dllexport )

#include <stdarg.h>
#include <stdio.h>

#include <tchar.h>
#include <systools/win32/snprintf.h>

#if _MSC_VER < 1300

#include <limits.h>
#define MAXSTR  INT_MAX

/*  The non-debug versions of _vscprintf/_scprintf are just calls
    to _vsprintf/_sprintf with string buffer pointer set to NULL */

static int __cdecl _vsctprintf( const _TXCHAR *format, va_list ap )
{
    FILE    str = { 0 };

    str._cnt = MAXSTR;
    str._base = str._ptr = NULL;
    str._flag = _IOWRT|_IOSTRG;

    return _vftprintf( &str, format, ap );
}
#endif

/*  This function retrieves the pointer to the last character of a buffer.
    That is the pointer to the last character of the buffer that fits
    completly into that buffer or the position of the terminating zero.

    buffer  Pointer to a _TXCHAR buffer to be examined
    count   size of the buffer to be examined

    return  The pointer to the last character that fits into the buffer or
            NULL if count is zero or count is one and the first byte was a
            leading DBCS character
*/

static _TCHAR *GetLastBufferChar( _TCHAR *buffer, size_t count )
{
    _TCHAR  *last = NULL;
    _TCHAR  *cur = buffer;

    while ( (size_t)(cur - buffer) < count )
    {
        last = cur;

        if ( !*last )
            break;

        cur = _tcsinc(last);
    }

    return last;
}

/* Implementation of snprintf following the ISO/IEC 9899:1999 (ISO C99) standard */

_SNPRINTF_DLLIMPORT int __cdecl vsntprintf( _TCHAR *buffer, size_t count, const _TCHAR *format, va_list list )
{
    int     retval;

    /*  First of all call the existing non POSIX standard function assuming
        the buffer size will be large enough */

    retval = _vsntprintf( buffer, count, format, list );

    if ( retval < 0 )
    {
        /*  If the buffer wasn't large enough ensure that the buffer will be
            zero terminated */

        _TCHAR  *last = GetLastBufferChar( buffer, count );
        if (last )
            *last = 0;

        /*  Retrieve the count of characters that would have been written
            if the buffer were large enough */

        retval = _vsctprintf( format, list );
    }
    else if ( (size_t)retval == count && count )
    {
        /*  If the buffer was large enough but not large enough for the trailing
            zero make the buffer zero terminated */

        _TCHAR  *last = GetLastBufferChar( buffer, count );
        if (last )
            *last = 0;
    }

    return retval;
}

/* Implementation of snprintf following the ISO/IEC 9899:1999 (ISO C99) standard */

_SNPRINTF_DLLIMPORT int __cdecl sntprintf( _TCHAR *buffer, size_t count, const _TCHAR *format, ... )
{
    va_list list;
    int     retval;

    va_start( list, format );
    retval = vsntprintf( buffer, count, format, list );
    va_end( list );

    return retval;
}

