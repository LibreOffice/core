/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#include <string.h>

#include "osl/diagnose.h"
#include <osl/interlck.h>
#include <rtl/alloc.h>
#include <rtl/memory.h>
#include <rtl/tencinfo.h>

#include <tools/string.hxx>
#include <impstrg.hxx>

// For shared byte convert tables
#include <toolsin.hxx>

#include <tools/debug.hxx>

// =======================================================================

DBG_NAME( ByteString )
DBG_NAMEEX( UniString )

// -----------------------------------------------------------------------

#define STRCODE         sal_Char
#define STRCODEU        unsigned char
#define STRING          ByteString
#define STRINGDATA      ByteStringData
#define DBGCHECKSTRING  DbgCheckByteString
#define STRING_TYPE     rtl_String
#define STRING_ACQUIRE  rtl_string_acquire
#define STRING_RELEASE  rtl_string_release
#define STRING_NEW      rtl_string_new


// -----------------------------------------------------------------------

xub_StrLen ImplStringLen( const sal_Char* pStr )
{
    const sal_Char* pTempStr = pStr;
    while( *pTempStr )
        ++pTempStr;
    return (xub_StrLen)(pTempStr-pStr);
}

// -----------------------------------------------------------------------

xub_StrLen ImplStringLen( const sal_Unicode* pStr )
{
    const sal_Unicode* pTempStr = pStr;
    while( *pTempStr )
        ++pTempStr;
    return (xub_StrLen)(pTempStr-pStr);
}

// -----------------------------------------------------------------------

#include <strimp.cxx>
#include <strcvt.cxx>

// -----------------------------------------------------------------------

sal_Bool ByteString::IsLowerAscii() const
{
    DBG_CHKTHIS( ByteString, DbgCheckByteString );

    sal_Int32 nIndex = 0;
    sal_Int32 nLen = mpData->mnLen;
    const sal_Char* pStr = mpData->maStr;
    while ( nIndex < nLen )
    {
        if ( (*pStr >= 65) && (*pStr <= 90) )
            return sal_False;

        ++pStr,
        ++nIndex;
    }

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool ByteString::IsUpperAscii() const
{
    DBG_CHKTHIS( ByteString, DbgCheckByteString );

    sal_Int32 nIndex = 0;
    sal_Int32 nLen = mpData->mnLen;
    const sal_Char* pStr = mpData->maStr;
    while ( nIndex < nLen )
    {
        if ( (*pStr >= 97) && (*pStr <= 122) )
            return sal_False;

        ++pStr,
        ++nIndex;
    }

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool ByteString::IsAlphaAscii() const
{
    DBG_CHKTHIS( ByteString, DbgCheckByteString );

    sal_Int32 nIndex = 0;
    sal_Int32 nLen = mpData->mnLen;
    const sal_Char* pStr = mpData->maStr;
    while ( nIndex < nLen )
    {
        if ( !(((*pStr >= 97) && (*pStr <= 122)) ||
               ((*pStr >= 65) && (*pStr <=  90))) )
            return sal_False;

        ++pStr,
        ++nIndex;
    }

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool ByteString::IsAlphaNumericAscii() const
{
    DBG_CHKTHIS( ByteString, DbgCheckByteString );

    sal_Int32 nIndex = 0;
    sal_Int32 nLen = mpData->mnLen;
    const sal_Char* pStr = mpData->maStr;
    while ( nIndex < nLen )
    {
        if ( !(((*pStr >= 97) && (*pStr <= 122)) ||
               ((*pStr >= 65) && (*pStr <=  90)) ||
               ((*pStr >= 48) && (*pStr <=  57))) )
            return sal_False;

        ++pStr,
        ++nIndex;
    }

    return sal_True;
}

void STRING::SearchAndReplaceAll( const STRCODE* pCharStr, const STRING& rRepStr )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rRepStr, STRING, DBGCHECKSTRING );

    xub_StrLen nCharLen = ImplStringLen( pCharStr );
    xub_StrLen nSPos = Search( pCharStr, 0 );
    while ( nSPos != STRING_NOTFOUND )
    {
        Replace( nSPos, nCharLen, rRepStr );
        nSPos = nSPos + rRepStr.Len();
        nSPos = Search( pCharStr, nSPos );
    }
}

xub_StrLen STRING::SearchAndReplace( const STRCODE* pCharStr, const STRING& rRepStr,
                                     xub_StrLen nIndex )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rRepStr, STRING, DBGCHECKSTRING );

    xub_StrLen nSPos = Search( pCharStr, nIndex );
    if ( nSPos != STRING_NOTFOUND )
        Replace( nSPos, ImplStringLen( pCharStr ), rRepStr );

    return nSPos;
}

static sal_Int32 ImplStringICompare( const STRCODE* pStr1, const STRCODE* pStr2,
                                     xub_StrLen nCount )
{
    sal_Int32   nRet = 0;
    STRCODE     c1;
    STRCODE     c2;
    do
    {
        if ( !nCount )
            break;

        // Ist das Zeichen zwischen 'A' und 'Z' dann umwandeln
        c1 = *pStr1;
        c2 = *pStr2;
        if ( (c1 >= 65) && (c1 <= 90) )
            c1 += 32;
        if ( (c2 >= 65) && (c2 <= 90) )
            c2 += 32;
        nRet = ((sal_Int32)((STRCODEU)c1))-((sal_Int32)((STRCODEU)c2));
        if ( nRet != 0 )
            break;

        ++pStr1,
        ++pStr2,
        --nCount;
    }
    while ( c2 );

    return nRet;
}

StringCompare STRING::CompareIgnoreCaseToAscii( const STRCODE* pCharStr,
                                                xub_StrLen nLen ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    // String vergleichen
    sal_Int32 nCompare = ImplStringICompare( mpData->maStr, pCharStr, nLen );

    // Rueckgabewert anpassen
    if ( nCompare == 0 )
        return COMPARE_EQUAL;
    else if ( nCompare < 0 )
        return COMPARE_LESS;
    else
        return COMPARE_GREATER;
}

StringCompare STRING::CompareTo( const STRCODE* pCharStr, xub_StrLen nLen ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    // String vergleichen
    sal_Int32 nCompare = ImplStringCompare( mpData->maStr, pCharStr, nLen );

    // Rueckgabewert anpassen
    if ( nCompare == 0 )
        return COMPARE_EQUAL;
    else if ( nCompare < 0 )
        return COMPARE_LESS;
    else
        return COMPARE_GREATER;
}

// =======================================================================

static sal_Int32 ImplStringCompare( const STRCODE* pStr1, const STRCODE* pStr2 )
{
    sal_Int32 nRet;
    while ( ((nRet = ((sal_Int32)((STRCODEU)*pStr1))-((sal_Int32)((STRCODEU)*pStr2))) == 0) &&
            *pStr2 )
    {
        ++pStr1,
        ++pStr2;
    }

    return nRet;
}

sal_Bool STRING::Equals( const STRCODE* pCharStr ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    return (ImplStringCompare( mpData->maStr, pCharStr ) == 0);
}

STRING& STRING::Insert( const STRCODE* pCharStr, xub_StrLen nIndex )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_ASSERT( pCharStr, "String::Insert() - pCharStr is NULL" );

    // Stringlaenge ermitteln
    sal_Int32 nCopyLen = ImplStringLen( pCharStr );

    // Ueberlauf abfangen
    nCopyLen = ImplGetCopyLen( mpData->mnLen, nCopyLen );

    // Ist der einzufuegende String ein Leerstring
    if ( !nCopyLen )
        return *this;

    // Index groesser als Laenge
    if ( nIndex > mpData->mnLen )
        nIndex = static_cast< xub_StrLen >(mpData->mnLen);

    // Neue Laenge ermitteln und neuen String anlegen
    STRINGDATA* pNewData = ImplAllocData( mpData->mnLen+nCopyLen );

    // String kopieren
    memcpy( pNewData->maStr, mpData->maStr, nIndex*sizeof( STRCODE ) );
    memcpy( pNewData->maStr+nIndex, pCharStr, nCopyLen*sizeof( STRCODE ) );
    memcpy( pNewData->maStr+nIndex+nCopyLen, mpData->maStr+nIndex,
            (mpData->mnLen-nIndex)*sizeof( STRCODE ) );

    // Alte Daten loeschen und Neue zuweisen
    STRING_RELEASE((STRING_TYPE *)mpData);
    mpData = pNewData;

    return *this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
