/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#define _SNPRINTF_DLLIMPORT __declspec( dllexport )

#include <stdarg.h>
#include <stdio.h>

#include <tchar.h>
#include <systools/win32/snprintf.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning(disable:4273)       // inconsistent dll linkage
#endif

#if (defined(_MSC_VER) && (_MSC_VER < 1300)) || (defined(__MINGW32_VERSION) && ((__MINGW32_MAJOR_VERSION < 3)||((__MINGW32_MAJOR_VERSION == 3)&&(__MINGW32_MINOR_VERSION < 18))))

/*  The non-debug versions of _vscprintf/_scprintf are just calls
    to _vsprintf/_sprintf with string buffer pointer set to NULL,
    requires MSVCRT version 7.0 */
#ifdef __MINGW32__
static int __cdecl _vsctprintf( const TCHAR *format, va_list ap )
#else
static int __cdecl _vsctprintf( const _TXCHAR *format, va_list ap )
#endif
{
    FILE    *fp = _tfopen( _T("NUL"), _T("wb") );

    if ( fp )
    {
        int retval = _vftprintf( fp, format, ap );
        fclose( fp );

        return retval;
    }

    return -1;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
