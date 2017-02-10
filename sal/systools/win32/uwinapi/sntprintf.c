/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#define LO_SNPRINTF_DLLIMPORT __declspec( dllexport )

#include <stdarg.h>
#include <stdio.h>

#include <tchar.h>
#include <systools/win32/snprintf.h>

#ifdef _MSC_VER
#pragma warning(disable:4273)       // inconsistent dll linkage
#endif

/* VS 2015 and above support ISO C snprintf */
#if _MSC_VER < 1900

/*  This function retrieves the pointer to the last character of a buffer.
    That is the pointer to the last character of the buffer that fits
    completely into that buffer or the position of the terminating zero.

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

LO_SNPRINTF_DLLIMPORT int __cdecl vsntprintf( _TCHAR *buffer, size_t count, const _TCHAR *format, va_list list )
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

LO_SNPRINTF_DLLIMPORT int __cdecl sntprintf( _TCHAR *buffer, size_t count, const _TCHAR *format, ... )
{
    va_list list;
    int     retval;

    va_start( list, format );
    retval = vsntprintf( buffer, count, format, list );
    va_end( list );

    return retval;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
