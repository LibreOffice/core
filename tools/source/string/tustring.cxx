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
    return rtl::OUString::valueOf(n, nRadix);
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

static sal_Int32 ImplStringICompareWithoutZero( const STRCODE* pStr1, const STRCODE* pStr2,
                                                sal_Int32 nCount )
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

        ++pStr1,
        ++pStr2,
        --nCount;
    }
    while ( nRet == 0 );

    return nRet;
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

STRING& STRING::Insert( STRCODE c, xub_StrLen nIndex )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    // Ist es kein 0-Character
    if ( !c || (mpData->mnLen == STRING_MAXLEN) )
        return *this;

    // Index groesser als Laenge
    if ( nIndex > mpData->mnLen )
        nIndex = static_cast< xub_StrLen >(mpData->mnLen);

    // Neue Laenge ermitteln und neuen String anlegen
    STRINGDATA* pNewData = ImplAllocData( mpData->mnLen+1 );

    // String kopieren
    memcpy( pNewData->maStr, mpData->maStr, nIndex*sizeof( STRCODE ) );
    pNewData->maStr[nIndex] = c;
    memcpy( pNewData->maStr+nIndex+1, mpData->maStr+nIndex,
            (mpData->mnLen-nIndex)*sizeof( STRCODE ) );

    // Alte Daten loeschen und Neue zuweisen
    STRING_RELEASE((STRING_TYPE *)mpData);
    mpData = pNewData;

    return *this;
}

// -----------------------------------------------------------------------

STRING& STRING::ToUpperAscii()
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    sal_Int32 nIndex = 0;
    sal_Int32 nLen = mpData->mnLen;
    STRCODE*    pStr = mpData->maStr;
    while ( nIndex < nLen )
    {
        // Ist das Zeichen zwischen 'a' und 'z' dann umwandeln
        if ( (*pStr >= 97) && (*pStr <= 122) )
        {
            // Daten kopieren, wenn noetig
            pStr = ImplCopyStringData( pStr );
            *pStr -= 32;
        }

        ++pStr,
        ++nIndex;
    }

    return *this;
}

// -----------------------------------------------------------------------

StringCompare STRING::CompareTo( const STRING& rStr, xub_StrLen nLen ) const
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
    sal_Int32 nCompare = ImplStringCompareWithoutZero( mpData->maStr, rStr.mpData->maStr, nLen );

    // Rueckgabewert anpassen
    if ( nCompare == 0 )
        return COMPARE_EQUAL;
    else if ( nCompare < 0 )
        return COMPARE_LESS;
    else
        return COMPARE_GREATER;
}

// -----------------------------------------------------------------------

sal_Bool STRING::Equals( const STRING& rStr ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    // Sind die Daten gleich
    if ( mpData == rStr.mpData )
        return sal_True;

    // Gleiche Laenge
    if ( mpData->mnLen != rStr.mpData->mnLen )
        return sal_False;

    // String vergleichen
    return (ImplStringCompareWithoutZero( mpData->maStr, rStr.mpData->maStr, mpData->mnLen ) == 0);
}

// -----------------------------------------------------------------------

sal_Bool STRING::EqualsIgnoreCaseAscii( const STRING& rStr ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    // Sind die Daten gleich
    if ( mpData == rStr.mpData )
        return sal_True;

    // Gleiche Laenge
    if ( mpData->mnLen != rStr.mpData->mnLen )
        return sal_False;

    // String vergleichen
    return (ImplStringICompareWithoutZero( mpData->maStr, rStr.mpData->maStr, mpData->mnLen ) == 0);
}

// -----------------------------------------------------------------------

sal_Bool STRING::Equals( const STRING& rStr, xub_StrLen nIndex, xub_StrLen nLen ) const
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
    return (ImplStringCompareWithoutZero( mpData->maStr+nIndex, rStr.mpData->maStr, nLen ) == 0);
}

// -----------------------------------------------------------------------

sal_Bool STRING::Equals( const STRCODE* pCharStr, xub_StrLen nIndex, xub_StrLen nLen ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    // Are there enough codes for comparing?
    if ( nIndex > mpData->mnLen )
        return (*pCharStr == 0);

    return (ImplStringCompare( mpData->maStr+nIndex, pCharStr, nLen ) == 0);
}

// -----------------------------------------------------------------------

xub_StrLen STRING::Match( const STRING& rStr ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    // Ist dieser String leer
    if ( !mpData->mnLen )
        return STRING_MATCH;

    // Suche bis Stringende nach dem ersten nicht uebereinstimmenden Zeichen
    const STRCODE*  pStr1 = mpData->maStr;
    const STRCODE*  pStr2 = rStr.mpData->maStr;
    xub_StrLen      i = 0;
    while ( i < mpData->mnLen )
    {
        // Stimmt das Zeichen nicht ueberein, dann abbrechen
        if ( *pStr1 != *pStr2 )
            return i;
        ++pStr1,
        ++pStr2,
        ++i;
    }

    return STRING_MATCH;
}

// -----------------------------------------------------------------------

xub_StrLen STRING::SearchBackward( STRCODE c, xub_StrLen nIndex ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    if ( nIndex > mpData->mnLen )
        nIndex = (xub_StrLen)mpData->mnLen;

    const STRCODE* pStr = mpData->maStr;
    pStr += nIndex;

    while ( nIndex )
    {
        nIndex--;
        pStr--;
        if ( *pStr == c )
            return nIndex;
    }

    return STRING_NOTFOUND;
}

// -----------------------------------------------------------------------

void STRING::SearchAndReplaceAll( const STRING& rStr, const STRING& rRepStr )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rRepStr, STRING, DBGCHECKSTRING );

    xub_StrLen nSPos = Search( rStr, 0 );
    while ( nSPos != STRING_NOTFOUND )
    {
        Replace( nSPos, rStr.Len(), rRepStr );
        nSPos = nSPos + rRepStr.Len();
        nSPos = Search( rStr, nSPos );
    }
}

// -----------------------------------------------------------------------

void STRING::SetToken( xub_StrLen nToken, STRCODE cTok, const STRING& rStr,
                       xub_StrLen nIndex )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    const STRCODE*  pStr            = mpData->maStr;
    xub_StrLen      nLen            = (xub_StrLen)mpData->mnLen;
    xub_StrLen      nTok            = 0;
    xub_StrLen      nFirstChar      = nIndex;
    xub_StrLen      i               = nFirstChar;

    // Bestimme die Token-Position und Laenge
    pStr += i;
    while ( i < nLen )
    {
        // Stimmt das Tokenzeichen ueberein, dann erhoehe TokCount
        if ( *pStr == cTok )
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

        ++pStr,
        ++i;
    }

    if ( nTok >= nToken )
        Replace( nFirstChar, i-nFirstChar, rStr );
}

// -----------------------------------------------------------------------

STRING STRING::GetToken( xub_StrLen nToken, STRCODE cTok, xub_StrLen& rIndex ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    const STRCODE*  pStr            = mpData->maStr;
    xub_StrLen      nLen            = (xub_StrLen)mpData->mnLen;
    xub_StrLen      nTok            = 0;
    xub_StrLen      nFirstChar      = rIndex;
    xub_StrLen      i               = nFirstChar;

    // Bestimme die Token-Position und Laenge
    pStr += i;
    while ( i < nLen )
    {
        // Stimmt das Tokenzeichen ueberein, dann erhoehe TokCount
        if ( *pStr == cTok )
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

STRING& STRING::Append( const STRCODE* pCharStr, xub_StrLen nCharLen )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_ASSERT( pCharStr, "String::Append() - pCharStr is NULL" );

    if ( nCharLen == STRING_LEN )
        nCharLen = ImplStringLen( pCharStr );

#ifdef DBG_UTIL
    if ( DbgIsAssert() )
    {
        for ( xub_StrLen i = 0; i < nCharLen; i++ )
        {
            if ( !pCharStr[i] )
            {
                OSL_FAIL( "String::Append() : nLen is wrong" );
            }
        }
    }
#endif

    // Ueberlauf abfangen
    sal_Int32 nLen = mpData->mnLen;
    sal_Int32 nCopyLen = ImplGetCopyLen( nLen, nCharLen );

    // Ist es kein leerer String
    if ( nCopyLen )
    {
        // Neue Datenstruktur und neuen String erzeugen
        STRINGDATA* pNewData = ImplAllocData( nLen+nCopyLen );

        // String kopieren
        memcpy( pNewData->maStr, mpData->maStr, nLen*sizeof( STRCODE ) );
        memcpy( pNewData->maStr+nLen, pCharStr, nCopyLen*sizeof( STRCODE ) );

        // Alte Daten loeschen und Neue zuweisen
        STRING_RELEASE((STRING_TYPE *)mpData);
        mpData = pNewData;
    }

    return *this;
}

// -----------------------------------------------------------------------

STRING& STRING::Append( STRCODE c )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    // kein 0-Character und maximale Stringlaenge nicht ueberschreiten
    sal_Int32 nLen = mpData->mnLen;
    if ( c && (nLen < STRING_MAXLEN) )
    {
        // Neue Datenstruktur und neuen String erzeugen
        STRINGDATA* pNewData = ImplAllocData( nLen+1 );

        // String kopieren
        memcpy( pNewData->maStr, mpData->maStr, nLen*sizeof( STRCODE ) );
        pNewData->maStr[nLen] = c;

        // Alte Daten loeschen und Neue zuweisen
        STRING_RELEASE((STRING_TYPE *)mpData);
        mpData = pNewData;
    }

    return *this;
}

// -----------------------------------------------------------------------

STRING& STRING::Assign( const STRCODE* pCharStr, xub_StrLen nLen )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_ASSERT( pCharStr, "String::Assign() - pCharStr is NULL" );

    if ( nLen == STRING_LEN )
        nLen = ImplStringLen( pCharStr );

#ifdef DBG_UTIL
    if ( DbgIsAssert() )
    {
        for ( xub_StrLen i = 0; i < nLen; i++ )
        {
            if ( !pCharStr[i] )
            {
                OSL_FAIL( "String::Assign() : nLen is wrong" );
            }
        }
    }
#endif

    if ( !nLen )
    {
        STRING_NEW((STRING_TYPE **)&mpData);
    }
    else
    {
        // Wenn String genauso lang ist, wie der String, dann direkt kopieren
        if ( (nLen == mpData->mnLen) && (mpData->mnRefCount == 1) )
            memcpy( mpData->maStr, pCharStr, nLen*sizeof( STRCODE ) );
        else
        {
            // Alte Daten loeschen
            STRING_RELEASE((STRING_TYPE *)mpData);

            // Daten initialisieren und String kopieren
            mpData = ImplAllocData( nLen );
            memcpy( mpData->maStr, pCharStr, nLen*sizeof( STRCODE ) );
        }
    }

    return *this;
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

// -----------------------------------------------------------------------

xub_StrLen STRING::SearchAndReplace( const STRING& rStr, const STRING& rRepStr,
                                     xub_StrLen nIndex )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rRepStr, STRING, DBGCHECKSTRING );

    xub_StrLen nSPos = Search( rStr, nIndex );
    if ( nSPos != STRING_NOTFOUND )
        Replace( nSPos, rStr.Len(), rRepStr );

    return nSPos;
}

// -----------------------------------------------------------------------

static sal_Int32 ImplStringICompare( const STRCODE* pStr1, const STRCODE* pStr2 )
{
    sal_Int32   nRet;
    STRCODE     c1;
    STRCODE     c2;
    do
    {
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
        ++pStr2;
    }
    while ( c2 );

    return nRet;
}

// -----------------------------------------------------------------------

sal_Bool STRING::EqualsIgnoreCaseAscii( const STRCODE* pCharStr ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    return (ImplStringICompare( mpData->maStr, pCharStr ) == 0);
}

// -----------------------------------------------------------------------

STRING& STRING::Assign( const STRCODE* pCharStr )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_ASSERT( pCharStr, "String::Assign() - pCharStr is NULL" );

    // Stringlaenge ermitteln
    xub_StrLen nLen = ImplStringLen( pCharStr );

    if ( !nLen )
    {
        STRING_NEW((STRING_TYPE **)&mpData);
    }
    else
    {
        // Wenn String genauso lang ist, wie der String, dann direkt kopieren
        if ( (nLen == mpData->mnLen) && (mpData->mnRefCount == 1) )
            memcpy( mpData->maStr, pCharStr, nLen*sizeof( STRCODE ) );
        else
        {
            // Alte Daten loeschen
            STRING_RELEASE((STRING_TYPE *)mpData);

            // Daten initialisieren und String kopieren
            mpData = ImplAllocData( nLen );
            memcpy( mpData->maStr, pCharStr, nLen*sizeof( STRCODE ) );
        }
    }

    return *this;
}

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
