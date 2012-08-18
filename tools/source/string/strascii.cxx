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

#ifdef DBG_UTIL
static sal_Bool ImplDbgCheckAsciiStr( const sal_Char* pAsciiStr, sal_Int32 nLen )
{
    while ( nLen && *pAsciiStr )
    {
        if ( ((unsigned char)*pAsciiStr) > 127 )
            return sal_False;
        ++pAsciiStr,
        --nLen;
    }

    return sal_True;
}
#endif

static void ImplCopyAsciiStr( sal_Unicode* pDest, const sal_Char* pSrc,
                              sal_Int32 nLen )
{
    DBG_ASSERT( ImplDbgCheckAsciiStr( pSrc, nLen ),
                "UniString::CopyAsciiStr() - pAsciiStr include characters > 127" );

    while ( nLen )
    {
        *pDest = (unsigned char)*pSrc;
        ++pDest,
        ++pSrc,
        --nLen;
    }
}

static sal_Int32 ImplStringCompareAscii( const sal_Unicode* pStr1, const sal_Char* pStr2 )
{
    sal_Int32 nRet;
    while ( ((nRet = ((sal_Int32)*pStr1)-((sal_Int32)((unsigned char)*pStr2))) == 0) &&
            *pStr2 )
    {
        ++pStr1,
        ++pStr2;
    }

    return nRet;
}

static sal_Int32 ImplStringCompareAscii( const sal_Unicode* pStr1, const sal_Char* pStr2,
                                         xub_StrLen nCount )
{
    sal_Int32 nRet = 0;
    while ( nCount &&
            ((nRet = ((sal_Int32)*pStr1)-((sal_Int32)((unsigned char)*pStr2))) == 0) &&
            *pStr2 )
    {
        ++pStr1,
        ++pStr2,
        --nCount;
    }

    return nRet;
}

static sal_Int32 ImplStringCompareWithoutZeroAscii( const sal_Unicode* pStr1, const sal_Char* pStr2,
                                                    xub_StrLen nCount )
{
    sal_Int32 nRet = 0;
    while ( nCount &&
            ((nRet = ((sal_Int32)*pStr1)-((sal_Int32)((unsigned char)*pStr2))) == 0) )
    {
        ++pStr1,
        ++pStr2,
        --nCount;
    }

    return nRet;
}

static sal_Int32 ImplStringICompareAscii( const sal_Unicode* pStr1, const sal_Char* pStr2 )
{
    sal_Int32   nRet;
    sal_Unicode c1;
    sal_Char    c2;
    do
    {
        // Ist das Zeichen zwischen 'A' und 'Z' dann umwandeln
        c1 = *pStr1;
        c2 = *pStr2;
        if ( (c1 >= 65) && (c1 <= 90) )
            c1 += 32;
        if ( (c2 >= 65) && (c2 <= 90) )
            c2 += 32;
        nRet = ((sal_Int32)c1)-((sal_Int32)((unsigned char)c2));
        if ( nRet != 0 )
            break;

        ++pStr1,
        ++pStr2;
    }
    while ( c2 );

    return nRet;
}

static sal_Int32 ImplStringICompareAscii( const sal_Unicode* pStr1, const sal_Char* pStr2,
                                          xub_StrLen nCount )
{
    sal_Int32   nRet = 0;
    sal_Unicode c1;
    sal_Char    c2;
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
        nRet = ((sal_Int32)c1)-((sal_Int32)((unsigned char)c2));
        if ( nRet != 0 )
            break;

        ++pStr1,
        ++pStr2,
        --nCount;
    }
    while ( c2 );

    return nRet;
}

UniString& UniString::AssignAscii( const sal_Char* pAsciiStr )
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( pAsciiStr, "UniString::AssignAscii() - pAsciiStr is NULL" );

    // Stringlaenge ermitteln
    xub_StrLen nLen = ImplStringLen( pAsciiStr );

    if ( !nLen )
    {
        STRING_NEW((STRING_TYPE **)&mpData);
    }
    else
    {
        // Wenn String genauso lang ist, wie der String, dann direkt kopieren
        if ( (nLen == mpData->mnLen) && (mpData->mnRefCount == 1) )
            ImplCopyAsciiStr( mpData->maStr, pAsciiStr, nLen );
        else
        {
            // Alte Daten loeschen
            STRING_RELEASE((STRING_TYPE *)mpData);

            // Daten initialisieren und String kopieren
            mpData = ImplAllocData( nLen );
            ImplCopyAsciiStr( mpData->maStr, pAsciiStr, nLen );
        }
    }

    return *this;
}

UniString& UniString::AssignAscii( const sal_Char* pAsciiStr, xub_StrLen nLen )
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( pAsciiStr, "UniString::AssignAscii() - pAsciiStr is NULL" );

    if ( nLen == STRING_LEN )
        nLen = ImplStringLen( pAsciiStr );

#ifdef DBG_UTIL
    if ( DbgIsAssert() )
    {
        for ( xub_StrLen i = 0; i < nLen; ++i )
        {
            if ( !pAsciiStr[i] )
            {
                OSL_FAIL( "UniString::AssignAscii() : nLen is wrong" );
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
            ImplCopyAsciiStr( mpData->maStr, pAsciiStr, nLen );
        else
        {
            // Alte Daten loeschen
            STRING_RELEASE((STRING_TYPE *)mpData);

            // Daten initialisieren und String kopieren
            mpData = ImplAllocData( nLen );
            ImplCopyAsciiStr( mpData->maStr, pAsciiStr, nLen );
        }
    }

    return *this;
}

UniString& UniString::AppendAscii( const sal_Char* pAsciiStr )
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( pAsciiStr, "UniString::AppendAscii() - pAsciiStr is NULL" );

    // Stringlaenge ermitteln
    sal_Int32 nCopyLen = ImplStringLen( pAsciiStr );

    // Ueberlauf abfangen
    nCopyLen = ImplGetCopyLen( mpData->mnLen, nCopyLen );

    // Ist es kein leerer String
    if ( nCopyLen )
    {
        // Neue Datenstruktur und neuen String erzeugen
        UniStringData* pNewData = ImplAllocData( mpData->mnLen+nCopyLen );

        // String kopieren
        memcpy( pNewData->maStr, mpData->maStr, mpData->mnLen*sizeof( sal_Unicode ) );
        ImplCopyAsciiStr( pNewData->maStr+mpData->mnLen, pAsciiStr, nCopyLen );

        // Alte Daten loeschen und Neue zuweisen
        STRING_RELEASE((STRING_TYPE *)mpData);
        mpData = pNewData;
    }

    return *this;
}

UniString& UniString::AppendAscii( const sal_Char* pAsciiStr, xub_StrLen nLen )
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( pAsciiStr, "UniString::AppendAscii() - pAsciiStr is NULL" );

    if ( nLen == STRING_LEN )
        nLen = ImplStringLen( pAsciiStr );

#ifdef DBG_UTIL
    if ( DbgIsAssert() )
    {
        for ( xub_StrLen i = 0; i < nLen; ++i )
        {
            if ( !pAsciiStr[i] )
            {
                OSL_FAIL( "UniString::AppendAscii() : nLen is wrong" );
            }
        }
    }
#endif

    // Ueberlauf abfangen
    sal_Int32 nCopyLen = ImplGetCopyLen( mpData->mnLen, nLen );

    // Ist es kein leerer String
    if ( nCopyLen )
    {
        // Neue Datenstruktur und neuen String erzeugen
        UniStringData* pNewData = ImplAllocData( mpData->mnLen+nCopyLen );

        // String kopieren
        memcpy( pNewData->maStr, mpData->maStr, mpData->mnLen*sizeof( sal_Unicode ) );
        ImplCopyAsciiStr( pNewData->maStr+mpData->mnLen, pAsciiStr, nCopyLen );

        // Alte Daten loeschen und Neue zuweisen
        STRING_RELEASE((STRING_TYPE *)mpData);
        mpData = pNewData;
    }

    return *this;
}

UniString& UniString::InsertAscii( const char* pAsciiStr, xub_StrLen nIndex )
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( pAsciiStr, "UniString::InsertAscii() - pAsciiStr is NULL" );

    // Stringlaenge ermitteln
    sal_Int32 nCopyLen = ImplStringLen( pAsciiStr );

    // Ueberlauf abfangen
    nCopyLen = ImplGetCopyLen( mpData->mnLen, nCopyLen );

    // Ist der einzufuegende String ein Leerstring
    if ( !nCopyLen )
        return *this;

    // Index groesser als Laenge
    if ( nIndex > mpData->mnLen )
        nIndex = static_cast< xub_StrLen >(mpData->mnLen);

    // Neue Laenge ermitteln und neuen String anlegen
    UniStringData* pNewData = ImplAllocData( mpData->mnLen+nCopyLen );

    // String kopieren
    memcpy( pNewData->maStr, mpData->maStr, nIndex*sizeof( sal_Unicode ) );
    ImplCopyAsciiStr( pNewData->maStr+nIndex, pAsciiStr, nCopyLen );
    memcpy( pNewData->maStr+nIndex+nCopyLen, mpData->maStr+nIndex,
            (mpData->mnLen-nIndex)*sizeof( sal_Unicode ) );

    // Alte Daten loeschen und Neue zuweisen
    STRING_RELEASE((STRING_TYPE *)mpData);
    mpData = pNewData;

    return *this;
}

UniString& UniString::ReplaceAscii( xub_StrLen nIndex, xub_StrLen nCount,
                                    const sal_Char* pAsciiStr, xub_StrLen nStrLen )
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( pAsciiStr, "UniString::ReplaceAscii() - pAsciiStr is NULL" );

    // Wenn Index groessergleich Laenge ist, dann ist es ein Append
    if ( nIndex >= mpData->mnLen )
    {
        AppendAscii( pAsciiStr, nStrLen );
        return *this;
    }

    // Ist es eine Zuweisung
    if ( (nIndex == 0) && (nCount >= mpData->mnLen) )
    {
        AssignAscii( pAsciiStr, nStrLen );
        return *this;
    }

    // Reicht ein Erase
    if ( nStrLen == STRING_LEN )
        nStrLen = ImplStringLen( pAsciiStr );
    if ( !nStrLen )
        return Erase( nIndex, nCount );

    // nCount darf nicht ueber das Stringende hinnausgehen
    if ( nCount > mpData->mnLen - nIndex )
        nCount = static_cast< xub_StrLen >(mpData->mnLen-nIndex);

    // Reicht eine zeichenweise Zuweisung
    if ( nCount == nStrLen )
    {
        ImplCopyData();
        ImplCopyAsciiStr( mpData->maStr+nIndex, pAsciiStr, nStrLen );
        return *this;
    }

    // Ueberlauf abfangen
    sal_Int32 n = ImplGetCopyLen( mpData->mnLen-nCount, nStrLen );

    // Neue Daten anlegen
    STRINGDATA* pNewData = ImplAllocData( mpData->mnLen-nCount+n );

    // String kopieren
    memcpy( pNewData->maStr, mpData->maStr, nIndex*sizeof( STRCODE ) );
    ImplCopyAsciiStr( pNewData->maStr+nIndex, pAsciiStr, n );
    memcpy( pNewData->maStr+nIndex+n, mpData->maStr+nIndex+nCount,
            (mpData->mnLen-nIndex-nCount+1)*sizeof( STRCODE ) );

    // Alte Daten loeschen und Neue zuweisen
    STRING_RELEASE((STRING_TYPE *)mpData);
    mpData = pNewData;

    return *this;
}

StringCompare UniString::CompareToAscii( const sal_Char* pAsciiStr,
                                         xub_StrLen nLen ) const
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, nLen ),
                "UniString::CompareToAscii() - pAsciiStr include characters > 127" );

    // String vergleichen
    sal_Int32 nCompare = ImplStringCompareAscii( mpData->maStr, pAsciiStr, nLen );

    // Rueckgabewert anpassen
    if ( nCompare == 0 )
        return COMPARE_EQUAL;
    else if ( nCompare < 0 )
        return COMPARE_LESS;
    else
        return COMPARE_GREATER;
}

StringCompare UniString::CompareIgnoreCaseToAscii( const sal_Char* pAsciiStr,
                                                   xub_StrLen nLen ) const
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, nLen ),
                "UniString::CompareIgnoreCaseToAscii() - pAsciiStr include characters > 127" );

    // String vergleichen
    sal_Int32 nCompare = ImplStringICompareAscii( mpData->maStr, pAsciiStr, nLen );

    // Rueckgabewert anpassen
    if ( nCompare == 0 )
        return COMPARE_EQUAL;
    else if ( nCompare < 0 )
        return COMPARE_LESS;
    else
        return COMPARE_GREATER;
}

sal_Bool UniString::EqualsAscii( const sal_Char* pAsciiStr ) const
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, STRING_LEN ),
                "UniString::EqualsAscii() - pAsciiStr include characters > 127" );

    return (ImplStringCompareAscii( mpData->maStr, pAsciiStr ) == 0);
}

sal_Bool UniString::EqualsIgnoreCaseAscii( const sal_Char* pAsciiStr ) const
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, STRING_LEN ),
                "UniString::EqualsIgnoreCaseAscii() - pAsciiStr include characters > 127" );

    return (ImplStringICompareAscii( mpData->maStr, pAsciiStr ) == 0);
}

sal_Bool UniString::EqualsAscii( const sal_Char* pAsciiStr,
                             xub_StrLen nIndex, xub_StrLen nLen ) const
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, nLen ),
                "UniString::EqualsAscii() - pAsciiStr include characters > 127" );

    // Are there enough codes for comparing?
    if ( nIndex > mpData->mnLen )
        return (*pAsciiStr == 0);

    return (ImplStringCompareAscii( mpData->maStr+nIndex, pAsciiStr, nLen ) == 0);
}

sal_Bool UniString::EqualsIgnoreCaseAscii( const sal_Char* pAsciiStr,
                                       xub_StrLen nIndex, xub_StrLen nLen ) const
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, nLen ),
                "UniString::EqualsIgnoreCaseAscii() - pAsciiStr include characters > 127" );

    // Are there enough codes for comparing?
    if ( nIndex > mpData->mnLen )
        return (*pAsciiStr == 0);

    return (ImplStringICompareAscii( mpData->maStr+nIndex, pAsciiStr, nLen ) == 0);
}

xub_StrLen UniString::SearchAscii( const sal_Char* pAsciiStr, xub_StrLen nIndex ) const
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, STRING_LEN ),
                "UniString::SearchAscii() - pAsciiStr include characters > 127" );

    sal_Int32 nLen = mpData->mnLen;
    xub_StrLen nStrLen  = ImplStringLen( pAsciiStr );

    // Falls die Laenge des uebergebenen Strings 0 ist oder der Index
    // hinter dem String liegt, dann wurde der String nicht gefunden
    if ( !nStrLen || (nIndex >= nLen) )
        return STRING_NOTFOUND;

    const sal_Unicode* pStr = mpData->maStr;
    pStr += nIndex;

    if ( nStrLen == 1 )
    {
        sal_Unicode cSearch = (unsigned char)*pAsciiStr;
        while ( nIndex < nLen )
        {
            if ( *pStr == cSearch )
                return nIndex;
            ++pStr,
            ++nIndex;
        }
    }
    else
    {
        // Nur innerhalb des Strings suchen
        while ( nLen - nIndex >= nStrLen )
        {
            // Stimmt der String ueberein
            if ( ImplStringCompareWithoutZeroAscii( pStr, pAsciiStr, nStrLen ) == 0 )
                return nIndex;
            ++pStr,
            ++nIndex;
        }
    }

    return STRING_NOTFOUND;
}

xub_StrLen UniString::SearchAndReplaceAscii( const sal_Char* pAsciiStr, const UniString& rRepStr,
                                             xub_StrLen nIndex )
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, STRING_LEN ),
                "UniString::SearchAndReplaceAscii() - pAsciiStr include characters > 127" );

    xub_StrLen nSPos = SearchAscii( pAsciiStr, nIndex );
    if ( nSPos != STRING_NOTFOUND )
        Replace( nSPos, ImplStringLen( pAsciiStr ), rRepStr );

    return nSPos;
}

void UniString::SearchAndReplaceAllAscii( const sal_Char* pAsciiStr, const UniString& rRepStr )
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, STRING_LEN ),
                "UniString::SearchAndReplaceAllAscii() - pAsciiStr include characters > 127" );

    xub_StrLen nCharLen = ImplStringLen( pAsciiStr );
    xub_StrLen nSPos = SearchAscii( pAsciiStr, 0 );
    while ( nSPos != STRING_NOTFOUND )
    {
        Replace( nSPos, nCharLen, rRepStr );
        nSPos = nSPos + rRepStr.Len();
        nSPos = SearchAscii( pAsciiStr, nSPos );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
