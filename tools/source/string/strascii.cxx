/*************************************************************************
 *
 *  $RCSfile: strascii.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// =======================================================================

#ifdef DBG_UTIL

static BOOL ImplDbgCheckAsciiStr( const sal_Char* pAsciiStr, xub_StrLen nLen )
{
    while ( nLen && *pAsciiStr )
    {
        if ( ((unsigned char)*pAsciiStr) > 127 )
            return FALSE;
        pAsciiStr++;
        nLen--;
    }

    return TRUE;
}

#endif

// =======================================================================

static void ImplCopyAsciiStr( sal_Unicode* pDest, const sal_Char* pSrc,
                              xub_StrLen nLen )
{
    DBG_ASSERT( ImplDbgCheckAsciiStr( pSrc, nLen ),
                "UniString::CopyAsciiStr() - pAsciiStr include characters > 127" );

    while ( nLen )
    {
        *pDest = (unsigned char)*pSrc;
        pDest++;
        pSrc++;
        nLen--;
    }
}

// =======================================================================

static sal_Int32 ImplStringCompareAscii( const sal_Unicode* pStr1, const sal_Char* pStr2 )
{
    sal_Int32 nRet;
    while ( ((nRet = ((sal_Int32)*pStr1)-((sal_Int32)((unsigned char)*pStr2))) == 0) &&
            *pStr2 )
    {
        pStr1++;
        pStr2++;
    }

    return nRet;
}

// -----------------------------------------------------------------------

static sal_Int32 ImplStringCompareAscii( const sal_Unicode* pStr1, const sal_Char* pStr2,
                                         xub_StrLen nCount )
{
    sal_Int32 nRet = 0;
    while ( nCount &&
            ((nRet = ((sal_Int32)*pStr1)-((sal_Int32)((unsigned char)*pStr2))) == 0) &&
            *pStr2 )
    {
        pStr1++;
        pStr2++;
        nCount--;
    }

    return nRet;
}

// -----------------------------------------------------------------------

static sal_Int32 ImplStringCompareWithoutZeroAscii( const sal_Unicode* pStr1, const sal_Char* pStr2,
                                                    xub_StrLen nCount )
{
    sal_Int32 nRet = 0;
    while ( nCount &&
            ((nRet = ((sal_Int32)*pStr1)-((sal_Int32)((unsigned char)*pStr2))) == 0) )
    {
        pStr1++;
        pStr2++;
        nCount--;
    }

    return nRet;
}

// -----------------------------------------------------------------------

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

        pStr1++;
        pStr2++;
    }
    while ( c2 );

    return nRet;
}

// -----------------------------------------------------------------------

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

        pStr1++;
        pStr2++;
        nCount--;
    }
    while ( c2 );

    return nRet;
}

// =======================================================================

UniString UniString::CreateFromAscii( const sal_Char* pAsciiStr )
{
    DBG_ASSERT( pAsciiStr, "UniString::CreateFromAscii() - pAsciiStr is NULL" );

    // Stringlaenge ermitteln
    xub_StrLen nLen = ImplStringLen( pAsciiStr );

    UniString aTempStr;
    if ( nLen )
    {
        if ( nLen > STRING_MAXLEN )
            nLen = STRING_MAXLEN;
        ImplCopyAsciiStr( aTempStr.AllocBuffer( nLen ), pAsciiStr, nLen );
    }
    return aTempStr;
}

// -----------------------------------------------------------------------

UniString UniString::CreateFromAscii( const sal_Char* pAsciiStr, xub_StrLen nLen )
{
    DBG_ASSERT( pAsciiStr, "UniString::CreateFromAscii() - pAsciiStr is NULL" );

    // Stringlaenge ermitteln
    if ( nLen == STRING_LEN )
        nLen = ImplStringLen( pAsciiStr );

    UniString aTempStr;

    if ( nLen )
    {
        if ( nLen > STRING_MAXLEN )
            nLen = STRING_MAXLEN;
        ImplCopyAsciiStr( aTempStr.AllocBuffer( nLen ), pAsciiStr, nLen );
    }
    return aTempStr;
}

// -----------------------------------------------------------------------

UniString& UniString::AssignAscii( const sal_Char* pAsciiStr )
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( pAsciiStr, "UniString::AssignAscii() - pAsciiStr is NULL" );

    // Stringlaenge ermitteln
    xub_StrLen nLen = ImplStringLen( pAsciiStr );

    if ( !nLen )
    {
        ImplDeleteData( mpData );
        ImplIncRefCount( &aImplEmptyStrData );
        mpData = &aImplEmptyStrData;
    }
    else
    {
        // Wenn String genauso lang ist, wie der String, dann direkt kopieren
        if ( (nLen == mpData->mnLen) && (mpData->mnRefCount == 1) )
            ImplCopyAsciiStr( mpData->maStr, pAsciiStr, nLen );
        else
        {
            // Alte Daten loeschen
            ImplDeleteData( mpData );

            // Daten initialisieren und String kopieren
            mpData = ImplAllocData( nLen );
            ImplCopyAsciiStr( mpData->maStr, pAsciiStr, nLen );
        }
    }

    return *this;
}

// -----------------------------------------------------------------------

UniString& UniString::AssignAscii( const sal_Char* pAsciiStr, xub_StrLen nLen )
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( pAsciiStr, "UniString::AssignAscii() - pAsciiStr is NULL" );

    if ( nLen == STRING_LEN )
        nLen = ImplStringLen( pAsciiStr );

#ifdef DBG_UTIL
    if ( DbgIsAssert() )
    {
        for ( xub_StrLen i = 0; i < nLen; i++ )
        {
            if ( !pAsciiStr[i] )
            {
                DBG_ERROR( "UniString::AssignAscii() : nLen is wrong" );
            }
        }
    }
#endif

    if ( !nLen )
    {
        ImplDeleteData( mpData );
        ImplIncRefCount( &aImplEmptyStrData );
        mpData = &aImplEmptyStrData;
    }
    else
    {
        // Wenn String genauso lang ist, wie der String, dann direkt kopieren
        if ( (nLen == mpData->mnLen) && (mpData->mnRefCount == 1) )
            ImplCopyAsciiStr( mpData->maStr, pAsciiStr, nLen );
        else
        {
            // Alte Daten loeschen
            ImplDeleteData( mpData );

            // Daten initialisieren und String kopieren
            mpData = ImplAllocData( nLen );
            ImplCopyAsciiStr( mpData->maStr, pAsciiStr, nLen );
        }
    }

    return *this;
}

// -----------------------------------------------------------------------

UniString& UniString::AppendAscii( const sal_Char* pAsciiStr )
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( pAsciiStr, "UniString::AppendAscii() - pAsciiStr is NULL" );

    // Stringlaenge ermitteln
    xub_StrLen nCopyLen = ImplStringLen( pAsciiStr );

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
        ImplDeleteData( mpData );
        mpData = pNewData;
    }

    return *this;
}

// -----------------------------------------------------------------------

UniString& UniString::AppendAscii( const sal_Char* pAsciiStr, xub_StrLen nLen )
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( pAsciiStr, "UniString::AppendAscii() - pAsciiStr is NULL" );

    if ( nLen == STRING_LEN )
        nLen = ImplStringLen( pAsciiStr );

#ifdef DBG_UTIL
    if ( DbgIsAssert() )
    {
        for ( xub_StrLen i = 0; i < nLen; i++ )
        {
            if ( !pAsciiStr[i] )
            {
                DBG_ERROR( "UniString::AppendAscii() : nLen is wrong" );
            }
        }
    }
#endif

    // Ueberlauf abfangen
    xub_StrLen nCopyLen = ImplGetCopyLen( mpData->mnLen, nLen );

    // Ist es kein leerer String
    if ( nCopyLen )
    {
        // Neue Datenstruktur und neuen String erzeugen
        UniStringData* pNewData = ImplAllocData( mpData->mnLen+nCopyLen );

        // String kopieren
        memcpy( pNewData->maStr, mpData->maStr, mpData->mnLen*sizeof( sal_Unicode ) );
        ImplCopyAsciiStr( pNewData->maStr+mpData->mnLen, pAsciiStr, nCopyLen );

        // Alte Daten loeschen und Neue zuweisen
        ImplDeleteData( mpData );
        mpData = pNewData;
    }

    return *this;
}

// -----------------------------------------------------------------------

UniString& UniString::InsertAscii( const char* pAsciiStr, xub_StrLen nIndex )
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( pAsciiStr, "UniString::InsertAscii() - pAsciiStr is NULL" );

    // Stringlaenge ermitteln
    xub_StrLen nCopyLen = ImplStringLen( pAsciiStr );

    // Ueberlauf abfangen
    nCopyLen = ImplGetCopyLen( mpData->mnLen, nCopyLen );

    // Ist der einzufuegende String ein Leerstring
    if ( !nCopyLen )
        return *this;

    // Index groesser als Laenge
    if ( nIndex > mpData->mnLen )
        nIndex = mpData->mnLen;

    // Neue Laenge ermitteln und neuen String anlegen
    UniStringData* pNewData = ImplAllocData( mpData->mnLen+nCopyLen );

    // String kopieren
    memcpy( pNewData->maStr, mpData->maStr, nIndex*sizeof( sal_Unicode ) );
    ImplCopyAsciiStr( pNewData->maStr+nIndex, pAsciiStr, nCopyLen );
    memcpy( pNewData->maStr+nIndex+nCopyLen, mpData->maStr+nIndex,
            (mpData->mnLen-nIndex)*sizeof( sal_Unicode ) );

    // Alte Daten loeschen und Neue zuweisen
    ImplDeleteData( mpData );
    mpData = pNewData;

    return *this;
}

// -----------------------------------------------------------------------

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
    if ( (ULONG)nIndex+nCount > mpData->mnLen )
        nCount = mpData->mnLen-nIndex;

    // Reicht eine zeichenweise Zuweisung
    if ( nCount == nStrLen )
    {
        ImplCopyData( this );
        ImplCopyAsciiStr( mpData->maStr+nIndex, pAsciiStr, nStrLen );
        return *this;
    }

    // Ueberlauf abfangen
    nStrLen = ImplGetCopyLen( mpData->mnLen-nCount, nStrLen );

    // Neue Daten anlegen
    STRINGDATA* pNewData = ImplAllocData( mpData->mnLen-nCount+nStrLen );

    // String kopieren
    memcpy( pNewData->maStr, mpData->maStr, nIndex*sizeof( STRCODE ) );
    ImplCopyAsciiStr( pNewData->maStr+nIndex, pAsciiStr, nStrLen );
    memcpy( pNewData->maStr+nIndex+nStrLen, mpData->maStr+nIndex+nCount,
            (mpData->mnLen-nIndex-nCount+1)*sizeof( STRCODE ) );

    // Alte Daten loeschen und Neue zuweisen
    ImplDeleteData( mpData );
    mpData = pNewData;

    return *this;
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

BOOL UniString::EqualsAscii( const sal_Char* pAsciiStr ) const
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, STRING_LEN ),
                "UniString::EqualsAscii() - pAsciiStr include characters > 127" );

    return (ImplStringCompareAscii( mpData->maStr, pAsciiStr ) == 0);
}

// -----------------------------------------------------------------------

BOOL UniString::EqualsIgnoreCaseAscii( const sal_Char* pAsciiStr ) const
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, STRING_LEN ),
                "UniString::EqualsIgnoreCaseAscii() - pAsciiStr include characters > 127" );

    return (ImplStringICompareAscii( mpData->maStr, pAsciiStr ) == 0);
}

// -----------------------------------------------------------------------

BOOL UniString::EqualsAscii( const sal_Char* pAsciiStr,
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

// -----------------------------------------------------------------------

BOOL UniString::EqualsIgnoreCaseAscii( const sal_Char* pAsciiStr,
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

// -----------------------------------------------------------------------

xub_StrLen UniString::SearchAscii( const sal_Char* pAsciiStr, xub_StrLen nIndex ) const
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, STRING_LEN ),
                "UniString::SearchAscii() - pAsciiStr include characters > 127" );

    xub_StrLen nLen     = mpData->mnLen;
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
            pStr++;
            nIndex++;
        }
    }
    else
    {
        // Nur innerhalb des Strings suchen
        while ( (ULONG)nIndex+nStrLen <= nLen )
        {
            // Stimmt der String ueberein
            if ( ImplStringCompareWithoutZeroAscii( pStr, pAsciiStr, nStrLen ) == 0 )
                return nIndex;
            pStr++;
            nIndex++;
        }
    }

    return STRING_NOTFOUND;
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

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
        nSPos += rRepStr.Len();
        nSPos = SearchAscii( pAsciiStr, nSPos );
    }
}

// =======================================================================

#ifndef ENABLEUNICODE

ByteString ByteString::CreateFromAscii( const sal_Char* pAsciiStr )
{
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, STRING_LEN ),
                "ByteString::CreateFromAscii() - pAsciiStr include characters > 127" );

    return ByteString( pAsciiStr );
}

// -----------------------------------------------------------------------

ByteString ByteString::CreateFromAscii( const sal_Char* pAsciiStr, xub_StrLen nLen )
{
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, nLen ),
                "ByteString::CreateFromAscii() - pAsciiStr include characters > 127" );

    return ByteString( pAsciiStr, nLen );
}

// -----------------------------------------------------------------------

ByteString& ByteString::AssignAscii( const sal_Char* pAsciiStr )
{
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, STRING_LEN ),
                "ByteString::AssignAscii() - pAsciiStr include characters > 127" );

    return Assign( pAsciiStr );
}

// -----------------------------------------------------------------------

ByteString& ByteString::AssignAscii( const sal_Char* pAsciiStr, xub_StrLen nLen )
{
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, nLen ),
                "ByteString::AssignAscii() - pAsciiStr include characters > 127" );

    return Assign( pAsciiStr, nLen );
}

// -----------------------------------------------------------------------

ByteString& ByteString::AppendAscii( const sal_Char* pAsciiStr, xub_StrLen nLen )
{
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, nLen ),
                "ByteString::AppendAscii() - pAsciiStr include characters > 127" );

    return Append( pAsciiStr, nLen );
}

// -----------------------------------------------------------------------

StringCompare ByteString::CompareToAscii( const sal_Char* pAsciiStr,
                                          xub_StrLen nLen ) const
{
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, nLen ),
                "ByteString::CompareToAscii() - pAsciiStr include characters > 127" );

    return CompareTo( pAsciiStr, nLen );
}

// -----------------------------------------------------------------------

BOOL ByteString::EqualsAscii( const sal_Char* pAsciiStr ) const
{
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, STRING_LEN ),
                "ByteString::EqualsAscii() - pAsciiStr include characters > 127" );

    return Equals( pAsciiStr );
}

// -----------------------------------------------------------------------

xub_StrLen ByteString::SearchAscii( const sal_Char* pAsciiStr, xub_StrLen nIndex ) const
{
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, STRING_LEN ),
                "ByteString::SearchAscii() - pAsciiStr include characters > 127" );

    return Search( pAsciiStr, nIndex );
}

// -----------------------------------------------------------------------

xub_StrLen ByteString::SearchAndReplaceAscii( const sal_Char* pAsciiStr, const ByteString& rRepStr,
                                              xub_StrLen nIndex )
{
    DBG_ASSERT( ImplDbgCheckAsciiStr( pAsciiStr, STRING_LEN ),
                "ByteString::SearchAscii() - pAsciiStr include characters > 127" );

    return SearchAndReplace( pAsciiStr, rRepStr, nIndex );
}

#endif
