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

#include "boost/static_assert.hpp"

#include <osl/interlck.h>
#include <rtl/alloc.h>
#include <rtl/memory.h>
#include <rtl/tencinfo.h>
#include <rtl/instance.hxx>

#include <tools/string.hxx>
#include <impstrg.hxx>

#include <tools/debug.hxx>

// =======================================================================

DBG_NAME( UniString )
DBG_NAMEEX( ByteString )

// -----------------------------------------------------------------------

#define STRCODE         sal_Unicode
#define STRCODEU        sal_Unicode
#define STRING          UniString
#define STRINGDATA      UniStringData
#define DBGCHECKSTRING  DbgCheckUniString
#define STRING_TYPE     rtl_uString
#define STRING_ACQUIRE  rtl_uString_acquire
#define STRING_RELEASE  rtl_uString_release
#define STRING_NEW      rtl_uString_new

// -----------------------------------------------------------------------

#include <strimp.cxx>
#include <strucvt.cxx>
#include <strascii.cxx>

UniString::UniString(char c): mpData(ImplAllocData(1)) { mpData->maStr[0] = c; }

// -----------------------------------------------------------------------

UniString UniString::CreateFromInt32( sal_Int32 n, sal_Int16 nRadix )
{
    sal_Unicode aBuf[RTL_USTR_MAX_VALUEOFINT32];
    BOOST_STATIC_ASSERT(RTL_USTR_MAX_VALUEOFINT32 <= STRING_MAXLEN);
    return UniString(
        aBuf,
        static_cast< xub_StrLen >(rtl_ustr_valueOfInt32( aBuf, n, nRadix )) );
}

// -----------------------------------------------------------------------

UniString UniString::CreateFromInt64( sal_Int64 n, sal_Int16 nRadix )
{
    sal_Unicode aBuf[RTL_USTR_MAX_VALUEOFINT64];
    BOOST_STATIC_ASSERT(RTL_USTR_MAX_VALUEOFINT64 <= STRING_MAXLEN);
    return UniString(
        aBuf,
        static_cast< xub_StrLen >(rtl_ustr_valueOfInt64( aBuf, n, nRadix )) );
}

// -----------------------------------------------------------------------

namespace { struct Empty : public rtl::Static< const UniString, Empty> {}; }
const UniString& UniString::EmptyString()
{
    return Empty::get();
}

// -----------------------------------------------------------------------

sal_Int32 UniString::ToInt32() const
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );

    return rtl_ustr_toInt32( mpData->maStr, 10 );
}

// -----------------------------------------------------------------------

sal_Int64 UniString::ToInt64() const
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );

    return rtl_ustr_toInt64( mpData->maStr, 10 );
}

// -----------------------------------------------------------------------

float UniString::ToFloat() const
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );

    return rtl_ustr_toFloat( mpData->maStr );
}

// -----------------------------------------------------------------------

double UniString::ToDouble() const
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );

    return rtl_ustr_toDouble( mpData->maStr );
}

// -----------------------------------------------------------------------

xub_StrLen STRING::SearchChar( const STRCODE* pChars, xub_StrLen nIndex ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    sal_Int32       nLen = mpData->mnLen;
    const STRCODE*  pStr = mpData->maStr;
    pStr += nIndex;
    while ( nIndex < nLen )
    {
        STRCODE         c = *pStr;
        const STRCODE*  pCompStr = pChars;
        while ( *pCompStr )
        {
            if ( *pCompStr == c )
                return nIndex;
            ++pCompStr;
        }
        ++pStr,
        ++nIndex;
    }

    return STRING_NOTFOUND;
}

// -----------------------------------------------------------------------

xub_StrLen STRING::SearchAndReplace( STRCODE c, STRCODE cRep, xub_StrLen nIndex )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    sal_Int32       nLen = mpData->mnLen;
    const STRCODE*  pStr = mpData->maStr;
    pStr += nIndex;
    while ( nIndex < nLen )
    {
        if ( *pStr == c )
        {
            ImplCopyData();
            mpData->maStr[nIndex] = cRep;
            return nIndex;
        }
        ++pStr,
        ++nIndex;
    }

    return STRING_NOTFOUND;
}

// -----------------------------------------------------------------------

STRING& STRING::Reverse()
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    if ( !mpData->mnLen )
        return *this;

    // Daten kopieren, wenn noetig
    ImplCopyData();

    // Reverse
    sal_Int32 nCount = mpData->mnLen / 2;
    for ( sal_Int32 i = 0; i < nCount; ++i )
    {
        STRCODE cTemp = mpData->maStr[i];
        mpData->maStr[i] = mpData->maStr[mpData->mnLen-i-1];
        mpData->maStr[mpData->mnLen-i-1] = cTemp;
    }

    return *this;
}

// -----------------------------------------------------------------------

STRING& STRING::Insert( const STRING& rStr, xub_StrLen nPos, xub_StrLen nLen,
                        xub_StrLen nIndex )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    // Stringlaenge ermitteln
    if ( nPos > rStr.mpData->mnLen )
        nLen = 0;
    else
    {
        // Laenge korrigieren, wenn noetig
        sal_Int32 nMaxLen = rStr.mpData->mnLen-nPos;
        if ( nLen > nMaxLen )
            nLen = static_cast< xub_StrLen >(nMaxLen);
    }

    // Ueberlauf abfangen
    sal_Int32 nCopyLen = ImplGetCopyLen( mpData->mnLen, nLen );

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
    memcpy( pNewData->maStr+nIndex, rStr.mpData->maStr+nPos, nCopyLen*sizeof( STRCODE ) );
    memcpy( pNewData->maStr+nIndex+nCopyLen, mpData->maStr+nIndex,
            (mpData->mnLen-nIndex)*sizeof( STRCODE ) );

    // Alte Daten loeschen und Neue zuweisen
    STRING_RELEASE((STRING_TYPE *)mpData);
    mpData = pNewData;

    return *this;
}

// -----------------------------------------------------------------------

xub_StrLen STRING::GetQuotedTokenCount( const STRING& rQuotedPairs, STRCODE cTok ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rQuotedPairs, STRING, DBGCHECKSTRING );
    DBG_ASSERT( !(rQuotedPairs.Len()%2), "String::GetQuotedTokenCount() - QuotedString%2 != 0" );
    DBG_ASSERT( rQuotedPairs.Search(cTok) == STRING_NOTFOUND, "String::GetQuotedTokenCount() - cTok in QuotedString" );

    // Leerer String: TokenCount per Definition 0
    if ( !mpData->mnLen )
        return 0;

    xub_StrLen      nTokCount       = 1;
    sal_Int32       nLen            = mpData->mnLen;
    xub_StrLen      nQuotedLen      = rQuotedPairs.Len();
    STRCODE         cQuotedEndChar  = 0;
    const STRCODE*  pQuotedStr      = rQuotedPairs.mpData->maStr;
    const STRCODE*  pStr            = mpData->maStr;
    sal_Int32       nIndex          = 0;
    while ( nIndex < nLen )
    {
        STRCODE c = *pStr;
        if ( cQuotedEndChar )
        {
            // Ende des Quotes erreicht ?
            if ( c == cQuotedEndChar )
                cQuotedEndChar = 0;
        }
        else
        {
            // Ist das Zeichen ein Quote-Anfang-Zeichen ?
            xub_StrLen nQuoteIndex = 0;
            while ( nQuoteIndex < nQuotedLen )
            {
                if ( pQuotedStr[nQuoteIndex] == c )
                {
                    cQuotedEndChar = pQuotedStr[nQuoteIndex+1];
                    break;
                }
                else
                    nQuoteIndex += 2;
            }

            // Stimmt das Tokenzeichen ueberein, dann erhoehe TokCount
            if ( c == cTok )
                ++nTokCount;
        }

        ++pStr,
        ++nIndex;
    }

    return nTokCount;
}

// -----------------------------------------------------------------------

STRING STRING::GetQuotedToken( xub_StrLen nToken, const STRING& rQuotedPairs,
                               STRCODE cTok, xub_StrLen& rIndex ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rQuotedPairs, STRING, DBGCHECKSTRING );
    DBG_ASSERT( !(rQuotedPairs.Len()%2), "String::GetQuotedToken() - QuotedString%2 != 0" );
    DBG_ASSERT( rQuotedPairs.Search(cTok) == STRING_NOTFOUND, "String::GetQuotedToken() - cTok in QuotedString" );

    const STRCODE*  pStr            = mpData->maStr;
    const STRCODE*  pQuotedStr      = rQuotedPairs.mpData->maStr;
    STRCODE         cQuotedEndChar  = 0;
    xub_StrLen      nQuotedLen      = rQuotedPairs.Len();
    xub_StrLen      nLen            = (xub_StrLen)mpData->mnLen;
    xub_StrLen      nTok            = 0;
    xub_StrLen      nFirstChar      = rIndex;
    xub_StrLen      i               = nFirstChar;

    // Bestimme die Token-Position und Laenge
    pStr += i;
    while ( i < nLen )
    {
        STRCODE c = *pStr;
        if ( cQuotedEndChar )
        {
            // Ende des Quotes erreicht ?
            if ( c == cQuotedEndChar )
                cQuotedEndChar = 0;
        }
        else
        {
            // Ist das Zeichen ein Quote-Anfang-Zeichen ?
            xub_StrLen nQuoteIndex = 0;
            while ( nQuoteIndex < nQuotedLen )
            {
                if ( pQuotedStr[nQuoteIndex] == c )
                {
                    cQuotedEndChar = pQuotedStr[nQuoteIndex+1];
                    break;
                }
                else
                    nQuoteIndex += 2;
            }

            // Stimmt das Tokenzeichen ueberein, dann erhoehe TokCount
            if ( c == cTok )
            {
                ++nTok;

                if ( nTok == nToken )
                    nFirstChar = i+1;
                else
                {
                    if ( nTok > nToken )
                        break;
                }
            }
        }

        ++pStr,
        ++i;
    }

    if ( nTok >= nToken )
    {
        if ( i < nLen )
            rIndex = i+1;
        else
            rIndex = STRING_NOTFOUND;
        return Copy( nFirstChar, i-nFirstChar );
    }
    else
    {
        rIndex = STRING_NOTFOUND;
        return STRING();
    }
}

// -----------------------------------------------------------------------

sal_Bool STRING::EqualsIgnoreCaseAscii( const STRING& rStr, xub_StrLen nIndex, xub_StrLen nLen ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    // Are there enough codes for comparing?
    if ( nIndex > mpData->mnLen )
        return (rStr.mpData->mnLen == 0);
    sal_Int32 nMaxLen = mpData->mnLen-nIndex;
    if ( nMaxLen < nLen )
    {
        if ( rStr.mpData->mnLen != nMaxLen )
            return sal_False;
        nLen = static_cast< xub_StrLen >(nMaxLen);
    }

    // String vergleichen
    return (ImplStringICompareWithoutZero( mpData->maStr+nIndex, rStr.mpData->maStr, nLen ) == 0);
}

// -----------------------------------------------------------------------

StringCompare STRING::CompareIgnoreCaseToAscii( const STRING& rStr,
                                                xub_StrLen nLen ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    // Auf Gleichheit der Pointer testen
    if ( mpData == rStr.mpData )
        return COMPARE_EQUAL;

    // Maximale Laenge ermitteln
    if ( mpData->mnLen < nLen )
        nLen = static_cast< xub_StrLen >(mpData->mnLen+1);
    if ( rStr.mpData->mnLen < nLen )
        nLen = static_cast< xub_StrLen >(rStr.mpData->mnLen+1);

    // String vergleichen
    sal_Int32 nCompare = ImplStringICompareWithoutZero( mpData->maStr, rStr.mpData->maStr, nLen );

    // Rueckgabewert anpassen
    if ( nCompare == 0 )
        return COMPARE_EQUAL;
    else if ( nCompare < 0 )
        return COMPARE_LESS;
    else
        return COMPARE_GREATER;
}

// -----------------------------------------------------------------------

STRING& STRING::Fill( xub_StrLen nCount, STRCODE cFillChar )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    if ( !nCount )
        return *this;

    // Ist nCount groesser wie der jetzige String, dann verlaengern
    if ( nCount > mpData->mnLen )
    {
        // dann neuen String mit der neuen Laenge anlegen
        STRINGDATA* pNewData = ImplAllocData( nCount );
        STRING_RELEASE((STRING_TYPE *)mpData);
        mpData = pNewData;
    }
    else
        ImplCopyData();

    STRCODE* pStr = mpData->maStr;
    do
    {
        *pStr = cFillChar;
        ++pStr,
        --nCount;
    }
    while ( nCount );

    return *this;
}

// -----------------------------------------------------------------------

STRING& STRING::Expand( xub_StrLen nCount, STRCODE cExpandChar )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    // Muss der String erweitert werden
    sal_Int32 nLen = mpData->mnLen;
    if ( nCount <= nLen )
        return *this;

    // Neuen String anlegen
    STRINGDATA* pNewData = ImplAllocData( nCount );

    // Alten String kopieren
    memcpy( pNewData->maStr, mpData->maStr, nLen*sizeof( STRCODE ) );

    // und initialisieren
    STRCODE* pStr = pNewData->maStr;
    pStr += nLen;
    for (sal_Int32 i = nCount - nLen; i > 0; --i) {
        *pStr++ = cExpandChar;
    }

    // Alte Daten loeschen und Neue zuweisen
    STRING_RELEASE((STRING_TYPE *)mpData);
    mpData = pNewData;

    return *this;
}

// -----------------------------------------------------------------------

STRCODE* STRING::GetBufferAccess()
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    // Daten kopieren, wenn noetig
    if ( mpData->mnLen )
        ImplCopyData();

    // Pointer auf den String zurueckgeben
    return mpData->maStr;
}

// -----------------------------------------------------------------------

void STRING::ReleaseBufferAccess( xub_StrLen nLen )
{
    // Hier ohne Funktionstest, da String nicht konsistent
    DBG_CHKTHIS( STRING, NULL );
    DBG_ASSERT( mpData->mnRefCount == 1, "String::ReleaseCharStr() called for String with RefCount" );

    if ( nLen > mpData->mnLen )
        nLen = ImplStringLen( mpData->maStr );
    OSL_ASSERT(nLen <= mpData->mnLen);
    if ( !nLen )
    {
        STRING_NEW((STRING_TYPE **)&mpData);
    }
    // Bei mehr als 8 Zeichen unterschied, kuerzen wir den Buffer
    else if ( mpData->mnLen - nLen > 8 )
    {
        STRINGDATA* pNewData = ImplAllocData( nLen );
        memcpy( pNewData->maStr, mpData->maStr, nLen*sizeof( STRCODE ) );
        STRING_RELEASE((STRING_TYPE *)mpData);
        mpData = pNewData;
    }
    else
        mpData->mnLen = nLen;
}

// -----------------------------------------------------------------------

STRCODE* STRING::AllocBuffer( xub_StrLen nLen )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    STRING_RELEASE((STRING_TYPE *)mpData);
    if ( nLen )
        mpData = ImplAllocData( nLen );
    else
    {
        mpData = NULL;
        STRING_NEW((STRING_TYPE **)&mpData);
    }

    return mpData->maStr;
}

// -----------------------------------------------------------------------

STRING::STRING( STRCODE c )
{
    DBG_CTOR( STRING, DBGCHECKSTRING );
    DBG_ASSERT( c, "String::String() - c is 0" );

    // Verwaltungsdaten anlegen und initialisieren
    mpData = ImplAllocData( 1 );
    mpData->maStr[0] = c;
}

// -----------------------------------------------------------------------

STRING& STRING::Assign( STRCODE c )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_ASSERT( c, "String::Assign() - c is 0" );

    // Verwaltungsdaten anlegen und initialisieren
    STRING_RELEASE((STRING_TYPE *)mpData);
    mpData = ImplAllocData( 1 );
    mpData->maStr[0] = c;
    return *this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
