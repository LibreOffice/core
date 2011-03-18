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

// no include "precompiled_tools.hxx" because this is included in other cxx files.

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

// -----------------------------------------------------------------------

static sal_Int32 ImplStringCompare( const STRCODE* pStr1, const STRCODE* pStr2,
                                    xub_StrLen nCount )
{
    sal_Int32 nRet = 0;
    while ( nCount &&
            ((nRet = ((sal_Int32)((STRCODEU)*pStr1))-((sal_Int32)((STRCODEU)*pStr2))) == 0) &&
            *pStr2 )
    {
        ++pStr1,
        ++pStr2,
        --nCount;
    }

    return nRet;
}

// -----------------------------------------------------------------------

static sal_Int32 ImplStringCompareWithoutZero( const STRCODE* pStr1, const STRCODE* pStr2,
                                               sal_Int32 nCount )
{
    sal_Int32 nRet = 0;
    while ( nCount &&
            ((nRet = ((sal_Int32)((STRCODEU)*pStr1))-((sal_Int32)((STRCODEU)*pStr2))) == 0) )
    {
        ++pStr1,
        ++pStr2,
        --nCount;
    }

    return nRet;
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

// =======================================================================

#ifdef DBG_UTIL
const char* DBGCHECKSTRING( const void* pString )
{
    STRING* p = (STRING*)pString;

    if ( p->GetBuffer()[p->Len()] != 0 )
        return "String damaged: aStr[nLen] != 0";

    return NULL;
}
#endif

// =======================================================================

static STRINGDATA* ImplAllocData( sal_Int32 nLen )
{
    // Dann kopiere die Daten
    STRINGDATA* pData   = (STRINGDATA*)rtl_allocateMemory( sizeof(STRINGDATA)+(nLen*sizeof( STRCODE )) );
    pData->mnRefCount   = 1;
    pData->mnLen        = nLen;
    pData->maStr[nLen]  = 0;
    return pData;
}

// -----------------------------------------------------------------------

static STRINGDATA* _ImplCopyData( STRINGDATA* pData )
{
    unsigned int    nSize       = sizeof(STRINGDATA)+(pData->mnLen*sizeof( STRCODE ));
    STRINGDATA*     pNewData    = (STRINGDATA*)rtl_allocateMemory( nSize );
    memcpy( pNewData, pData, nSize );
    pNewData->mnRefCount = 1;
    STRING_RELEASE((STRING_TYPE *)pData);
    return pNewData;
}

// -----------------------------------------------------------------------

inline void STRING::ImplCopyData()
{
    DBG_ASSERT( (mpData->mnRefCount != 0), "String::ImplCopyData() - RefCount == 0" );

    // ist es ein referenzierter String, dann die Daten abkoppeln
    if ( mpData->mnRefCount != 1 )
        mpData = _ImplCopyData( mpData );
}

// -----------------------------------------------------------------------

inline STRCODE* STRING::ImplCopyStringData( STRCODE* pStr )
{
    // Ist der Referenzzaehler groesser 0
    if ( mpData->mnRefCount != 1 ) {
        DBG_ASSERT( (pStr >= mpData->maStr) &&
                    ((pStr-mpData->maStr) < mpData->mnLen),
                    "ImplCopyStringData - pStr from other String-Instanz" );
        unsigned int nIndex = (unsigned int)(pStr-mpData->maStr);
        mpData = _ImplCopyData( mpData );
        pStr = mpData->maStr + nIndex;
    }
    return pStr;
}

// -----------------------------------------------------------------------

inline sal_Int32 ImplGetCopyLen( sal_Int32 nStrLen, sal_Int32 nCopyLen )
{
    OSL_ASSERT(nStrLen <= STRING_MAXLEN && nCopyLen <= STRING_MAXLEN);
    if ( nCopyLen > STRING_MAXLEN-nStrLen )
        nCopyLen = STRING_MAXLEN-nStrLen;
    return nCopyLen;
}

// =======================================================================

STRING::STRING()
    : mpData(NULL)
{
    DBG_CTOR( STRING, DBGCHECKSTRING );

    STRING_NEW((STRING_TYPE **)&mpData);
}

// -----------------------------------------------------------------------

STRING::STRING( const STRING& rStr )
{
    DBG_CTOR( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    // Pointer auf die Daten des uebergebenen Strings setzen und
    // Referenzzaehler erhoehen
    STRING_ACQUIRE((STRING_TYPE *)rStr.mpData);
    mpData = rStr.mpData;
}

// -----------------------------------------------------------------------

STRING::STRING( const STRING& rStr, xub_StrLen nPos, xub_StrLen nLen )
: mpData( NULL )
{
    DBG_CTOR( STRING, DBGCHECKSTRING );
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

    // Ist es kein leerer String
    if ( nLen )
    {
        // Reicht ein einfaches erhoehen des Referenzcounters
        if ( (nPos == 0) && (nLen == rStr.mpData->mnLen) )
        {
            STRING_ACQUIRE((STRING_TYPE *)rStr.mpData);
            mpData = rStr.mpData;
        }
        else
        {
            // Verwaltungsdaten anlegen und String kopieren
            mpData = ImplAllocData( nLen );
            memcpy( mpData->maStr, rStr.mpData->maStr+nPos, nLen*sizeof( STRCODE ) );
        }
    }
    else
    {
        STRING_NEW((STRING_TYPE **)&mpData);
    }
}

// -----------------------------------------------------------------------

STRING::STRING( const STRCODE* pCharStr )
    : mpData(NULL)
{
    DBG_CTOR( STRING, DBGCHECKSTRING );

    // Stringlaenge ermitteln
    // Bei diesem Ctor darf NULL uebergeben werden
    xub_StrLen nLen;
    if ( pCharStr )
        nLen = ImplStringLen( pCharStr );
    else
        nLen = 0;

    // Ist es kein leerer String
    if ( nLen )
    {
        // Verwaltungsdaten anlegen und String kopieren
        mpData = ImplAllocData( nLen );
        memcpy( mpData->maStr, pCharStr, nLen*sizeof( STRCODE ) );
    }
    else
    {
        STRING_NEW((STRING_TYPE **)&mpData);
    }
}

// -----------------------------------------------------------------------

STRING::STRING( const STRCODE* pCharStr, xub_StrLen nLen )
: mpData(NULL)
{
    DBG_CTOR( STRING, DBGCHECKSTRING );
    DBG_ASSERT( pCharStr, "String::String() - pCharStr is NULL" );

    if ( nLen == STRING_LEN )
        nLen = ImplStringLen( pCharStr );

#ifdef DBG_UTIL
    if ( DbgIsAssert() )
    {
        for ( xub_StrLen i = 0; i < nLen; i++ )
        {
            if ( !pCharStr[i] )
            {
                OSL_FAIL( "String::String() : nLen is wrong" );
            }
        }
    }
#endif

    // Ist es kein leerer String
    if ( nLen )
    {
        // Verwaltungsdaten anlegen und String kopieren
        mpData = ImplAllocData( nLen );
        memcpy( mpData->maStr, pCharStr, nLen*sizeof( STRCODE ) );
    }
    else
    {
        STRING_NEW((STRING_TYPE **)&mpData);
    }
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

STRING::~STRING()
{
    DBG_DTOR( STRING, DBGCHECKSTRING );

    // Daten loeschen
    STRING_RELEASE((STRING_TYPE *)mpData);
}

// -----------------------------------------------------------------------

STRING& STRING::Assign( const STRING& rStr )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    STRING_ACQUIRE((STRING_TYPE *)rStr.mpData);
    STRING_RELEASE((STRING_TYPE *)mpData);
    mpData = rStr.mpData;
    return *this;
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

STRING& STRING::Append( const STRING& rStr )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    // Wenn String leer, dann reicht eine Zuweisung
    sal_Int32 nLen = mpData->mnLen;
    if ( !nLen )
    {
        STRING_ACQUIRE((STRING_TYPE *)rStr.mpData);
        STRING_RELEASE((STRING_TYPE *)mpData);
        mpData = rStr.mpData;
    }
    else
    {
        // Ueberlauf abfangen
        sal_Int32 nCopyLen = ImplGetCopyLen( nLen, rStr.mpData->mnLen );

        // Ist der uebergebene String kein Leerstring
        if ( nCopyLen )
        {
            // Neue Datenstruktur und neuen String erzeugen
            STRINGDATA* pNewData = ImplAllocData( nLen+nCopyLen );

            // String kopieren
            memcpy( pNewData->maStr, mpData->maStr, nLen*sizeof( STRCODE ) );
            memcpy( pNewData->maStr+nLen, rStr.mpData->maStr, nCopyLen*sizeof( STRCODE ) );

            // Alte Daten loeschen und Neue zuweisen
            STRING_RELEASE((STRING_TYPE *)mpData);
            mpData = pNewData;
        }
    }

    return *this;
}

// -----------------------------------------------------------------------

STRING& STRING::Append( const STRCODE* pCharStr )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_ASSERT( pCharStr, "String::Append() - pCharStr is NULL" );

    // Stringlaenge ermitteln
    sal_Int32 nLen = mpData->mnLen;
    sal_Int32 nCopyLen = ImplStringLen( pCharStr );

    // Ueberlauf abfangen
    nCopyLen = ImplGetCopyLen( nLen, nCopyLen );

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

void STRING::SetChar( xub_StrLen nIndex, STRCODE c )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_ASSERT( nIndex < mpData->mnLen, "String::SetChar() - nIndex > String.Len()" );

    // Daten kopieren, wenn noetig und Character zuweisen
    ImplCopyData();
    mpData->maStr[nIndex] = c;
}

// -----------------------------------------------------------------------

STRING& STRING::Insert( const STRING& rStr, xub_StrLen nIndex )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    // Ueberlauf abfangen
    sal_Int32 nCopyLen = ImplGetCopyLen( mpData->mnLen, rStr.mpData->mnLen );

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
    memcpy( pNewData->maStr+nIndex, rStr.mpData->maStr, nCopyLen*sizeof( STRCODE ) );
    memcpy( pNewData->maStr+nIndex+nCopyLen, mpData->maStr+nIndex,
            (mpData->mnLen-nIndex)*sizeof( STRCODE ) );

    // Alte Daten loeschen und Neue zuweisen
    STRING_RELEASE((STRING_TYPE *)mpData);
    mpData = pNewData;

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

STRING& STRING::Replace( xub_StrLen nIndex, xub_StrLen nCount, const STRING& rStr )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    // Wenn Index groessergleich Laenge ist, dann ist es ein Append
    if ( nIndex >= mpData->mnLen )
    {
        Append( rStr );
        return *this;
    }

    // Ist es eine Zuweisung
    if ( (nIndex == 0) && (nCount >= mpData->mnLen) )
    {
        Assign( rStr );
        return *this;
    }

    // Reicht ein Erase
    sal_Int32 nStrLen = rStr.mpData->mnLen;
    if ( !nStrLen )
        return Erase( nIndex, nCount );

    // nCount darf nicht ueber das Stringende hinnausgehen
    if ( nCount > mpData->mnLen - nIndex )
        nCount = static_cast< xub_StrLen >(mpData->mnLen-nIndex);

    // Reicht ein Insert
    if ( !nCount )
        return Insert( rStr, nIndex );

    // Reicht eine zeichenweise Zuweisung
    if ( nCount == nStrLen )
    {
        ImplCopyData();
        memcpy( mpData->maStr+nIndex, rStr.mpData->maStr, nCount*sizeof( STRCODE ) );
        return *this;
    }

    // Ueberlauf abfangen
    nStrLen = ImplGetCopyLen( mpData->mnLen-nCount, nStrLen );

    // Neue Daten anlegen
    STRINGDATA* pNewData = ImplAllocData( mpData->mnLen-nCount+nStrLen );

    // String kopieren
    memcpy( pNewData->maStr, mpData->maStr, nIndex*sizeof( STRCODE ) );
    memcpy( pNewData->maStr+nIndex, rStr.mpData->maStr, nStrLen*sizeof( STRCODE ) );
    memcpy( pNewData->maStr+nIndex+nStrLen, mpData->maStr+nIndex+nCount,
            (mpData->mnLen-nIndex-nCount+1)*sizeof( STRCODE ) );

    // Alte Daten loeschen und Neue zuweisen
    STRING_RELEASE((STRING_TYPE *)mpData);
    mpData = pNewData;

    return *this;
}

// -----------------------------------------------------------------------

STRING& STRING::Erase( xub_StrLen nIndex, xub_StrLen nCount )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    // Ist der Index ausserhalb des Strings oder ist nCount == 0
    if ( (nIndex >= mpData->mnLen) || !nCount )
        return *this;

    // nCount darf nicht ueber das Stringende hinnausgehen
    if ( nCount > mpData->mnLen - nIndex )
        nCount = static_cast< xub_StrLen >(mpData->mnLen-nIndex);

    // Ist das Ergebnis kein Leerstring
    if ( mpData->mnLen - nCount )
    {
        // Neue Daten anlegen
        STRINGDATA* pNewData = ImplAllocData( mpData->mnLen-nCount );

        // String kopieren
        memcpy( pNewData->maStr, mpData->maStr, nIndex*sizeof( STRCODE ) );
        memcpy( pNewData->maStr+nIndex, mpData->maStr+nIndex+nCount,
                (mpData->mnLen-nIndex-nCount+1)*sizeof( STRCODE ) );

        // Alte Daten loeschen und Neue zuweisen
        STRING_RELEASE((STRING_TYPE *)mpData);
        mpData = pNewData;
    }
    else
    {
        STRING_NEW((STRING_TYPE **)&mpData);
    }

    return *this;
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

STRING& STRING::EraseLeadingChars( STRCODE c )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    if ( mpData->maStr[0] != c )
        return *this;

    xub_StrLen nStart = 0;
    while ( mpData->maStr[nStart] == c )
        ++nStart;

    return Erase( 0, nStart );
}

// -----------------------------------------------------------------------

STRING& STRING::EraseTrailingChars( STRCODE c )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    sal_Int32 nEnd = mpData->mnLen;
    while ( nEnd && (mpData->maStr[nEnd-1] == c) )
        nEnd--;

    if ( nEnd != mpData->mnLen )
        Erase( static_cast< xub_StrLen >(nEnd) );

    return *this;
}

// -----------------------------------------------------------------------

STRING& STRING::EraseLeadingAndTrailingChars( STRCODE c )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    xub_StrLen nStart = 0;
    while ( mpData->maStr[nStart] == c )
        ++nStart;
    if ( nStart )
        Erase( 0, nStart );

    sal_Int32 nEnd = mpData->mnLen;
    while ( nEnd && (mpData->maStr[nEnd-1] == c) )
        nEnd--;
    if ( nEnd != mpData->mnLen )
        Erase( static_cast< xub_StrLen >(nEnd) );

    return *this;
}

// -----------------------------------------------------------------------

STRING& STRING::EraseAllChars( STRCODE c )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    sal_Int32 nCount = 0;
    for (sal_Int32 i = 0; i < mpData->mnLen; ++i) {
        if ( mpData->maStr[i] == c )
            ++nCount;
    }

    if ( nCount )
    {
        if ( nCount == mpData->mnLen )
        {
            STRING_NEW((STRING_TYPE **)&mpData);
        }
        else
        {
            // Neuen String anlegen
            STRINGDATA* pNewData = ImplAllocData( mpData->mnLen-nCount );

            // Alten String kopieren und initialisieren
            nCount = 0;
            for( xub_StrLen j = 0; j < mpData->mnLen; ++j )
            {
                if ( mpData->maStr[j] != c )
                {
                    pNewData->maStr[nCount] = mpData->maStr[j];
                    ++nCount;
                }
            }

            // Alte Daten loeschen und Neue zuweisen
            STRING_RELEASE((STRING_TYPE *)mpData);
            mpData = pNewData;
        }
    }

    return *this;
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

STRING& STRING::ToLowerAscii()
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    sal_Int32 nIndex = 0;
    sal_Int32 nLen = mpData->mnLen;
    STRCODE*    pStr = mpData->maStr;
    while ( nIndex < nLen )
    {
        // Ist das Zeichen zwischen 'A' und 'Z' dann umwandeln
        if ( (*pStr >= 65) && (*pStr <= 90) )
        {
            // Daten kopieren, wenn noetig
            pStr = ImplCopyStringData( pStr );
            *pStr += 32;
        }

        ++pStr,
        ++nIndex;
    }

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

STRING& STRING::ConvertLineEnd( LineEnd eLineEnd )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    // Zeilenumbrueche ermitteln und neue Laenge berechnen
    sal_Bool            bConvert    = sal_False;            // Muss konvertiert werden
    const STRCODE*  pStr        = mpData->maStr;    // damit es schneller geht
    xub_StrLen      nLineEndLen = (eLineEnd == LINEEND_CRLF) ? 2 : 1;
    xub_StrLen      nLen        = 0;                // Ziel-Laenge
    xub_StrLen      i           = 0;                // Source-Zaehler

    while ( i < mpData->mnLen )
    {
        // Bei \r oder \n gibt es neuen Zeilenumbruch
        if ( (pStr[i] == _CR) || (pStr[i] == _LF) )
        {
            nLen = nLen + nLineEndLen;

            // Wenn schon gesetzt, dann brauchen wir keine aufwendige Abfrage
            if ( !bConvert )
            {
                // Muessen wir Konvertieren
                if ( ((eLineEnd != LINEEND_LF) && (pStr[i] == _LF)) ||
                     ((eLineEnd == LINEEND_CRLF) && (pStr[i+1] != _LF)) ||
                     ((eLineEnd == LINEEND_LF) &&
                      ((pStr[i] == _CR) || (pStr[i+1] == _CR))) ||
                     ((eLineEnd == LINEEND_CR) &&
                      ((pStr[i] == _LF) || (pStr[i+1] == _LF))) )
                    bConvert = sal_True;
            }

            // \r\n oder \n\r, dann Zeichen ueberspringen
            if ( ((pStr[i+1] == _CR) || (pStr[i+1] == _LF)) &&
                 (pStr[i] != pStr[i+1]) )
                ++i;
        }
        else
            ++nLen;
        ++i;

        // Wenn String zu lang, dann konvertieren wir nicht
        if ( nLen >= STRING_MAXLEN )
            return *this;
    }

    // Zeilenumbrueche konvertieren
    if ( bConvert )
    {
        // Neuen String anlegen
        STRINGDATA* pNewData = ImplAllocData( nLen );
        xub_StrLen  j = 0;
        i = 0;
        while ( i < mpData->mnLen )
        {
            // Bei \r oder \n gibt es neuen Zeilenumbruch
            if ( (pStr[i] == _CR) || (pStr[i] == _LF) )
            {
                if ( eLineEnd == LINEEND_CRLF )
                {
                    pNewData->maStr[j]   = _CR;
                    pNewData->maStr[j+1] = _LF;
                    j += 2;
                }
                else
                {
                    if ( eLineEnd == LINEEND_CR )
                        pNewData->maStr[j] = _CR;
                    else
                        pNewData->maStr[j] = _LF;
                    ++j;
                }

                if ( ((pStr[i+1] == _CR) || (pStr[i+1] == _LF)) &&
                     (pStr[i] != pStr[i+1]) )
                    ++i;
            }
            else
            {
                pNewData->maStr[j] = mpData->maStr[i];
                ++j;
            }

            ++i;
        }

        // Alte Daten loeschen und Neue zuweisen
        STRING_RELEASE((STRING_TYPE *)mpData);
        mpData = pNewData;
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

sal_Bool STRING::Equals( const STRCODE* pCharStr ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    return (ImplStringCompare( mpData->maStr, pCharStr ) == 0);
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

sal_Bool STRING::EqualsIgnoreCaseAscii( const STRCODE* pCharStr ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    return (ImplStringICompare( mpData->maStr, pCharStr ) == 0);
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

sal_Bool STRING::EqualsIgnoreCaseAscii( const STRCODE* pCharStr, xub_StrLen nIndex, xub_StrLen nLen ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    // Are there enough codes for comparing?
    if ( nIndex > mpData->mnLen )
        return (*pCharStr == 0);

    return (ImplStringICompare( mpData->maStr+nIndex, pCharStr, nLen ) == 0);
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

xub_StrLen STRING::Match( const STRCODE* pCharStr ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    // Ist dieser String leer
    if ( !mpData->mnLen )
        return STRING_MATCH;

    // Suche bis Stringende nach dem ersten nicht uebereinstimmenden Zeichen
    const STRCODE*  pStr = mpData->maStr;
    xub_StrLen      i = 0;
    while ( i < mpData->mnLen )
    {
        // Stimmt das Zeichen nicht ueberein, dann abbrechen
        if ( *pStr != *pCharStr )
            return i;
        ++pStr,
        ++pCharStr,
        ++i;
    }

    return STRING_MATCH;
}

// -----------------------------------------------------------------------

xub_StrLen STRING::Search( STRCODE c, xub_StrLen nIndex ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    sal_Int32       nLen = mpData->mnLen;
    const STRCODE*  pStr = mpData->maStr;
    pStr += nIndex;
    while ( nIndex < nLen )
    {
        if ( *pStr == c )
            return nIndex;
        ++pStr,
        ++nIndex;
    }

    return STRING_NOTFOUND;
}

// -----------------------------------------------------------------------

xub_StrLen STRING::Search( const STRING& rStr, xub_StrLen nIndex ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );
    DBG_CHKOBJ( &rStr, STRING, DBGCHECKSTRING );

    sal_Int32 nLen = mpData->mnLen;
    sal_Int32 nStrLen = rStr.mpData->mnLen;

    // Falls die Laenge des uebergebenen Strings 0 ist oder der Index
    // hinter dem String liegt, dann wurde der String nicht gefunden
    if ( !nStrLen || (nIndex >= nLen) )
        return STRING_NOTFOUND;

    const STRCODE* pStr1 = mpData->maStr;
    pStr1 += nIndex;

    if ( nStrLen == 1 )
    {
        STRCODE cSearch = rStr.mpData->maStr[0];
        while ( nIndex < nLen )
        {
            if ( *pStr1 == cSearch )
                return nIndex;
            ++pStr1,
            ++nIndex;
        }
    }
    else
    {
        const STRCODE* pStr2 = rStr.mpData->maStr;

        // Nur innerhalb des Strings suchen
        while ( nLen - nIndex >= nStrLen )
        {
            // Stimmt der String ueberein
            if ( ImplStringCompareWithoutZero( pStr1, pStr2, nStrLen ) == 0 )
                return nIndex;
            ++pStr1,
            ++nIndex;
        }
    }

    return STRING_NOTFOUND;
}

// -----------------------------------------------------------------------

xub_StrLen STRING::Search( const STRCODE* pCharStr, xub_StrLen nIndex ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    sal_Int32 nLen = mpData->mnLen;
    xub_StrLen nStrLen  = ImplStringLen( pCharStr );

    // Falls die Laenge des uebergebenen Strings 0 ist oder der Index
    // hinter dem String liegt, dann wurde der String nicht gefunden
    if ( !nStrLen || (nIndex >= nLen) )
        return STRING_NOTFOUND;

    const STRCODE* pStr = mpData->maStr;
    pStr += nIndex;

    if ( nStrLen == 1 )
    {
        STRCODE cSearch = *pCharStr;
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
            if ( ImplStringCompareWithoutZero( pStr, pCharStr, nStrLen ) == 0 )
                return nIndex;
            ++pStr,
            ++nIndex;
        }
    }

    return STRING_NOTFOUND;
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

xub_StrLen STRING::SearchCharBackward( const STRCODE* pChars, xub_StrLen nIndex ) const
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

        STRCODE         c =*pStr;
        const STRCODE*  pCompStr = pChars;
        while ( *pCompStr )
        {
            if ( *pCompStr == c )
                return nIndex;
            ++pCompStr;
        }
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

// -----------------------------------------------------------------------

void STRING::SearchAndReplaceAll( STRCODE c, STRCODE cRep )
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    sal_Int32       nLen    = mpData->mnLen;
    const STRCODE*  pStr    = mpData->maStr;
    sal_Int32       nIndex  = 0;
    while ( nIndex < nLen )
    {
        if ( *pStr == c )
        {
            ImplCopyData();
            mpData->maStr[nIndex] = cRep;
        }
        ++pStr,
        ++nIndex;
    }
}

// -----------------------------------------------------------------------

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

xub_StrLen STRING::GetTokenCount( STRCODE cTok ) const
{
    DBG_CHKTHIS( STRING, DBGCHECKSTRING );

    // Leerer String: TokenCount per Definition 0
    if ( !mpData->mnLen )
        return 0;

    xub_StrLen      nTokCount       = 1;
    sal_Int32       nLen            = mpData->mnLen;
    const STRCODE*  pStr            = mpData->maStr;
    sal_Int32       nIndex          = 0;
    while ( nIndex < nLen )
    {
        // Stimmt das Tokenzeichen ueberein, dann erhoehe TokCount
        if ( *pStr == cTok )
            ++nTokCount;
        ++pStr,
        ++nIndex;
    }

    return nTokCount;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
