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
#include "precompiled_sd.hxx"
#include <propread.hxx>
#include <tools/bigint.hxx>
#include "tools/debug.hxx"
#include "rtl/tencinfo.h"
#include "rtl/textenc.h"

// ------------------------------------------------------------------------

struct PropEntry
{
    sal_uInt32  mnId;
    sal_uInt32  mnSize;
    sal_uInt16  mnTextEnc;
    sal_uInt8*  mpBuf;

                        PropEntry( sal_uInt32 nId, const sal_uInt8* pBuf, sal_uInt32 nBufSize, sal_uInt16 nTextEnc );
                        PropEntry( const PropEntry& rProp );
                        ~PropEntry() { delete[] mpBuf; } ;

    const PropEntry&    operator=(const PropEntry& rPropEntry);
};

PropEntry::PropEntry( sal_uInt32 nId, const sal_uInt8* pBuf, sal_uInt32 nBufSize, sal_uInt16 nTextEnc ) :
    mnId        ( nId ),
    mnSize      ( nBufSize ),
    mnTextEnc   ( nTextEnc ),
    mpBuf       ( new sal_uInt8[ nBufSize ] )
{
    memcpy( (void*)mpBuf, (void*)pBuf, nBufSize );
};

PropEntry::PropEntry( const PropEntry& rProp ) :
    mnId        ( rProp.mnId ),
    mnSize      ( rProp.mnSize ),
    mnTextEnc   ( rProp.mnTextEnc ),
    mpBuf       ( new sal_uInt8[ mnSize ] )
{
    memcpy( (void*)mpBuf, (void*)rProp.mpBuf, mnSize );
};

const PropEntry& PropEntry::operator=(const PropEntry& rPropEntry)
{
    if ( this != &rPropEntry )
    {
        delete[] mpBuf;
        mnId = rPropEntry.mnId;
        mnSize = rPropEntry.mnSize;
        mnTextEnc = rPropEntry.mnTextEnc;
        mpBuf = new sal_uInt8[ mnSize ];
        memcpy( (void*)mpBuf, (void*)rPropEntry.mpBuf, mnSize );
    }
    return *this;
}

//  -----------------------------------------------------------------------

void PropItem::Clear()
{
    Seek( STREAM_SEEK_TO_BEGIN );
    delete[] (sal_uInt8*)SwitchBuffer();
}

//  -----------------------------------------------------------------------

static xub_StrLen lcl_getMaxSafeStrLen(sal_uInt32 nSize)
{
    nSize -= 1; //Drop NULL terminator

    //If it won't fit in a string, clip it to the max size that does
    if (nSize > STRING_MAXLEN)
        nSize = STRING_MAXLEN;

    return static_cast< xub_StrLen >( nSize );
}

BOOL PropItem::Read( String& rString, sal_uInt32 nStringType, sal_Bool bAlign )
{
    sal_uInt32  i, nItemSize, nType, nItemPos;
    sal_Bool    bRetValue = sal_False;

    nItemPos = Tell();

    if ( nStringType == VT_EMPTY )
        *this >> nType;
    else
        nType = nStringType & VT_TYPEMASK;

    *this >> nItemSize;

    switch( nType )
    {
        case VT_LPSTR :
        {
            if ( nItemSize )
            {
                try
                {
                    sal_Char* pString = new sal_Char[ nItemSize ];
                    if ( mnTextEnc == RTL_TEXTENCODING_UCS2 )
                    {
                        nItemSize >>= 1;
                        if ( nItemSize > 1 )
                        {
                            sal_Unicode* pWString = (sal_Unicode*)pString;
                            for ( i = 0; i < nItemSize; i++ )
                                *this >> pWString[ i ];
                            rString = String( pWString, lcl_getMaxSafeStrLen(nItemSize) );
                        }
                        else
                            rString = String();
                        bRetValue = sal_True;
                    }
                    else
                    {
                        SvMemoryStream::Read( pString, nItemSize );
                        if ( pString[ nItemSize - 1 ] == 0 )
                        {
                            if ( nItemSize > 1 )
                                rString = String( ByteString( pString ), mnTextEnc );
                            else
                                rString = String();
                            bRetValue = sal_True;
                        }
                    }
                    delete[] pString;
                }
                catch( const std::bad_alloc& )
                {
                    OSL_FAIL( "sd PropItem::Read bad alloc" );
                }
            }
            if ( bAlign )
                SeekRel( ( 4 - ( nItemSize & 3 ) ) & 3 );       // dword align
        }
        break;

        case VT_LPWSTR :
        {
            if ( nItemSize )
            {
                try
                {
                    sal_Unicode* pString = new sal_Unicode[ nItemSize ];
                    for ( i = 0; i < nItemSize; i++ )
                        *this >> pString[ i ];
                    if ( pString[ i - 1 ] == 0 )
                    {
                        if ( (sal_uInt16)nItemSize > 1 )
                            rString = String( pString, lcl_getMaxSafeStrLen(nItemSize) );
                        else
                            rString = String();
                        bRetValue = sal_True;
                    }
                    delete[] pString;
                }
                catch( const std::bad_alloc& )
                {
                    OSL_FAIL( "sd PropItem::Read bad alloc" );
                }
            }
            if ( bAlign && ( nItemSize & 1 ) )
                SeekRel( 2 );                           // dword align
        }
        break;
    }
    if ( !bRetValue )
        Seek( nItemPos );
    return bRetValue;
}

//  -----------------------------------------------------------------------

PropItem& PropItem::operator=( PropItem& rPropItem )
{
    if ( this != &rPropItem )
    {
        Seek( STREAM_SEEK_TO_BEGIN );
        delete[] (sal_uInt8*)SwitchBuffer();

        mnTextEnc = rPropItem.mnTextEnc;
        sal_uInt32 nItemPos = rPropItem.Tell();
        rPropItem.Seek( STREAM_SEEK_TO_END );
        SvMemoryStream::Write( rPropItem.GetData(), rPropItem.Tell() );
        rPropItem.Seek( nItemPos );
    }
    return *this;
}

//  -----------------------------------------------------------------------

struct Dict
{
    sal_uInt32  mnId;
    String      aString;

            Dict( sal_uInt32 nId, String rString ) { mnId = nId; aString = rString; };
};

//  -----------------------------------------------------------------------

Dictionary::~Dictionary()
{
    for ( void* pPtr = First(); pPtr; pPtr = Next() )
        delete (Dict*)pPtr;
}

//  -----------------------------------------------------------------------

void Dictionary::AddProperty( sal_uInt32 nId, const String& rString )
{
    if ( rString.Len() )        // eindeutige namen bei properties
    {
        // pruefen, ob es die Propertybeschreibung in der Dictionary schon gibt
        for ( Dict* pDict = (Dict*)First(); pDict; pDict = (Dict*)Next() )
        {
            if ( pDict->mnId == nId )
            {
                pDict->aString = rString;
                return;
            }
        }
        Insert( new Dict( nId, rString ), LIST_APPEND );
    }
}

//  -----------------------------------------------------------------------

UINT32 Dictionary::GetProperty( const String& rString )
{
    for ( Dict* pDict = (Dict*)First(); pDict; pDict = (Dict*)Next() )
    {
        if ( pDict->aString == rString )
            return pDict->mnId;
    }
    return 0;
}

//  -----------------------------------------------------------------------

Dictionary& Dictionary::operator=( Dictionary& rDictionary )
{
    void* pPtr;

    if ( this != &rDictionary )
    {
        for ( pPtr = First(); pPtr; pPtr = Next() )
            delete (Dict*)pPtr;

        for ( pPtr = rDictionary.First(); pPtr; pPtr = rDictionary.Next() )
            Insert( new Dict( ((Dict*)pPtr)->mnId, ((Dict*)pPtr)->aString ), LIST_APPEND );
    }
    return *this;
}

//  -----------------------------------------------------------------------

Section::Section( Section& rSection )
: List()
{
    mnTextEnc = rSection.mnTextEnc;
    for ( int i = 0; i < 16; i++ )
        aFMTID[ i ] = rSection.aFMTID[ i ];
    for ( PropEntry* pProp = (PropEntry*)rSection.First(); pProp; pProp = (PropEntry*)rSection.Next() )
        Insert( new PropEntry( *pProp ), LIST_APPEND );
}

//  -----------------------------------------------------------------------

Section::Section( const sal_uInt8* pFMTID )
{
    mnTextEnc = RTL_TEXTENCODING_MS_1252;
    for ( int i = 0; i < 16; i++ )
        aFMTID[ i ] = pFMTID[ i ];
}

//  -----------------------------------------------------------------------

sal_Bool Section::GetProperty( sal_uInt32 nId, PropItem& rPropItem )
{
    PropEntry* pProp;
    if ( nId )
    {
        for ( pProp = (PropEntry*)First(); pProp; pProp = (PropEntry*)Next() )
        {
            if ( pProp->mnId == nId )
                break;
        }
        if ( pProp )
        {
            rPropItem.Clear();
            rPropItem.SetTextEncoding( mnTextEnc );
            rPropItem.Write( pProp->mpBuf, pProp->mnSize );
            rPropItem.Seek( STREAM_SEEK_TO_BEGIN );
            return sal_True;
        }
    }
    return sal_False;
}

//  -----------------------------------------------------------------------

void Section::AddProperty( sal_uInt32 nId, const sal_uInt8* pBuf, sal_uInt32 nBufSize )
{
    // kleiner id check

    if ( !nId )
        return;
    if ( nId == 0xffffffff )
        nId = 0;

    // keine doppelten PropId's zulassen, sortieren
    for ( sal_uInt32 i = 0; i < Count(); i++ )
    {
        PropEntry* pPropEntry = (PropEntry*)GetObject( i );
        if ( pPropEntry->mnId == nId )
            delete (PropEntry*)Replace( new PropEntry( nId, pBuf, nBufSize, mnTextEnc ), i );
        else if ( pPropEntry->mnId > nId )
            Insert( new PropEntry( nId, pBuf, nBufSize, mnTextEnc ), i );
        else
            continue;
        return;
    }
    Insert( new PropEntry( nId, pBuf, nBufSize, mnTextEnc ), LIST_APPEND );
}

//  -----------------------------------------------------------------------

sal_Bool Section::GetDictionary( Dictionary& rDict )
{
    sal_Bool bRetValue = sal_False;

    Dictionary aDict;
    PropEntry* pProp;

    for ( pProp = (PropEntry*)First(); pProp; pProp = (PropEntry*)Next() )
    {
        if ( pProp->mnId == 0 )
            break;
    }
    if ( pProp )
    {
        sal_uInt32 nDictCount, nId, nSize, nPos;
        SvMemoryStream aStream( (sal_Int8*)pProp->mpBuf, pProp->mnSize, STREAM_READ );
        aStream.Seek( STREAM_SEEK_TO_BEGIN );
        aStream >> nDictCount;
        for ( sal_uInt32 i = 0; i < nDictCount; i++ )
        {
            aStream >> nId >> nSize;
            if ( nSize )
            {
                String aString;
                nPos = aStream.Tell();
                try
                {
                    sal_Char* pString = new sal_Char[ nSize ];
                    aStream.Read( pString, nSize );
                    if ( mnTextEnc == RTL_TEXTENCODING_UCS2 )
                    {
                        nSize >>= 1;
                        aStream.Seek( nPos );
                        sal_Unicode* pWString = (sal_Unicode*)pString;
                        for ( i = 0; i < nSize; i++ )
                            aStream >> pWString[ i ];
                        aString = String( pWString, lcl_getMaxSafeStrLen(nSize) );
                    }
                    else
                        aString = String( ByteString( pString, lcl_getMaxSafeStrLen(nSize) ), mnTextEnc );
                    delete[] pString;
                }
                catch( const std::bad_alloc& )
                {
                    OSL_FAIL( "sd Section::GetDictionary bad alloc" );
                }
                if ( !aString.Len() )
                    break;
                aDict.AddProperty( nId, aString );
            }
            bRetValue = sal_True;
        }
    }
    rDict = aDict;
    return bRetValue;
}

//  -----------------------------------------------------------------------

Section::~Section()
{
    for ( PropEntry* pProp = (PropEntry*)First(); pProp; pProp = (PropEntry*)Next() )
        delete pProp;
}

//  -----------------------------------------------------------------------

void Section::Read( SvStorageStream *pStrm )
{
    sal_uInt32 i, nSecOfs, nSecSize, nPropCount, nPropId, nPropOfs, nPropType, nPropSize, nCurrent, nVectorCount, nTemp, nStrmSize;
    nSecOfs = pStrm->Tell();

    pStrm->Seek( STREAM_SEEK_TO_END );
    nStrmSize = pStrm->Tell();
    pStrm->Seek( nSecOfs );

    mnTextEnc = RTL_TEXTENCODING_MS_1252;
    *pStrm >> nSecSize >> nPropCount;
    while( nPropCount-- && ( pStrm->GetError() == ERRCODE_NONE ) )
    {
        *pStrm >> nPropId >> nPropOfs;
        nCurrent = pStrm->Tell();
        pStrm->Seek( nPropOfs + nSecOfs );
        if ( nPropId )                  // dictionary wird nicht eingelesen
        {

            *pStrm >> nPropType;

            nPropSize = 4;

            if ( nPropType & VT_VECTOR )
            {
                *pStrm >> nVectorCount;
                nPropType &=~VT_VECTOR;
                nPropSize += 4;
            }
            else
                nVectorCount = 1;


            sal_Bool bVariant = ( nPropType == VT_VARIANT );

            for ( i = 0; nPropSize && ( i < nVectorCount ); i++ )
            {
                if ( bVariant )
                {
                    *pStrm >> nPropType;
                    nPropSize += 4;
                }
                switch( nPropType )
                {
                    case VT_UI1 :
                        nPropSize++;
                    break;

                    case VT_I2 :
                    case VT_UI2 :
                    case VT_BOOL :
                        nPropSize += 2;
                    break;

                    case VT_I4 :
                    case VT_R4 :
                    case VT_UI4 :
                    case VT_ERROR :
                        nPropSize += 4;
                    break;

                    case VT_I8 :
                    case VT_R8 :
                    case VT_CY :
                    case VT_UI8 :
                    case VT_DATE :
                    case VT_FILETIME :
                        nPropSize += 8;
                    break;

                    case VT_BSTR :
                        *pStrm >> nTemp;
                        nPropSize += ( nTemp + 4 );
                    break;

                    case VT_LPSTR :
                        *pStrm >> nTemp;
                        nPropSize += ( nTemp + 4 );
                    break;

                    case VT_LPWSTR :
                        *pStrm >> nTemp;
                        nPropSize += ( nTemp << 1 ) + 4;
                    break;

                    case VT_BLOB_OBJECT :
                    case VT_BLOB :
                    case VT_CF :
                        *pStrm >> nTemp;
                        nPropSize += ( nTemp + 4 );
                    break;

                    case VT_CLSID :
                    case VT_STREAM :
                    case VT_STORAGE :
                    case VT_STREAMED_OBJECT :
                    case VT_STORED_OBJECT :
                    case VT_VARIANT :
                    case VT_VECTOR :
                    default :
                        nPropSize = 0;
                }
                if ( nPropSize )
                {
                    if ( ( nVectorCount - i ) > 1 )
                        pStrm->Seek( nPropOfs + nSecOfs + nPropSize );
                }
                else
                    break;
            }
            if ( nPropSize )
            {
                if ( nPropSize > nStrmSize )
                {
                    nPropCount = 0;
                    break;
                }
                pStrm->Seek( nPropOfs + nSecOfs );
                sal_uInt8* pBuf = new sal_uInt8[ nPropSize ];
                pStrm->Read( pBuf, nPropSize );
                AddProperty( nPropId, pBuf, nPropSize );
                delete[] pBuf;
            }
            if ( nPropId == 1 )
            {
                PropItem aPropItem;
                if ( GetProperty( 1, aPropItem ) )
                {
                    sal_uInt16 nCodePage;
                    aPropItem >> nPropType;
                    if ( nPropType == VT_I2 )
                    {
                        aPropItem >> nCodePage;

                        if ( nCodePage == 1200 )
                        {
                            mnTextEnc = RTL_TEXTENCODING_UCS2;
                        }
                        else
                        {
                            mnTextEnc = rtl_getTextEncodingFromWindowsCodePage( nCodePage );
                            if ( mnTextEnc == RTL_TEXTENCODING_DONTKNOW )
                                mnTextEnc = RTL_TEXTENCODING_MS_1252;
                        }
                    }
                    else
                    {
                        mnTextEnc = RTL_TEXTENCODING_MS_1252;
                    }
                }
            }
        }
        else
        {
            sal_uInt32 nDictCount, nSize;
            *pStrm >> nDictCount;
            for ( i = 0; i < nDictCount; i++ )
            {
                *pStrm >> nSize >> nSize;
                pStrm->SeekRel( nSize );
            }
            nSize = pStrm->Tell();
            pStrm->Seek( nPropOfs + nSecOfs );
            nSize -= pStrm->Tell();
            if ( nSize > nStrmSize )
            {
                nPropCount = 0;
                break;
            }
            sal_uInt8* pBuf = new sal_uInt8[ nSize ];
            pStrm->Read( pBuf, nSize );
            AddProperty( 0xffffffff, pBuf, nSize );
            delete[] pBuf;
        }
        pStrm->Seek( nCurrent );
    }
    pStrm->Seek( nSecOfs + nSecSize );
}

//  -----------------------------------------------------------------------

Section& Section::operator=( Section& rSection )
{
    PropEntry* pProp;

    if ( this != &rSection )
    {
        memcpy( (void*)aFMTID, (void*)rSection.aFMTID, 16 );
        for ( pProp = (PropEntry*)First(); pProp; pProp = (PropEntry*)Next() )
            delete pProp;
        Clear();
        for ( pProp = (PropEntry*)rSection.First(); pProp; pProp = (PropEntry*)rSection.Next() )
            Insert( new PropEntry( *pProp ), LIST_APPEND );
    }
    return *this;
}

//  -----------------------------------------------------------------------

PropRead::PropRead( SvStorage& rStorage, const String& rName ) :
        mbStatus            ( sal_False ),
        mnByteOrder         ( 0xfffe ),
        mnFormat            ( 0 ),
        mnVersionLo         ( 4 ),
        mnVersionHi         ( 2 )
{
    if ( rStorage.IsStream( rName ) )
    {
        mpSvStream = rStorage.OpenSotStream( rName, STREAM_STD_READ );
        if ( mpSvStream )
        {
            mpSvStream->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
            memset( mApplicationCLSID, 0, 16 );
            mbStatus = sal_True;
        }
    }
}

//  -----------------------------------------------------------------------

void PropRead::AddSection( Section& rSection )
{
    Insert( new Section( rSection ), LIST_APPEND );
}

//  -----------------------------------------------------------------------

const Section* PropRead::GetSection( const sal_uInt8* pFMTID )
{
    Section* pSection;

    for ( pSection = (Section*)First(); pSection; pSection = (Section*)Next() )
    {
        if ( memcmp( pSection->GetFMTID(), pFMTID, 16 ) == 0 )
            break;
    }
    return pSection;
}

//  -----------------------------------------------------------------------

PropRead::~PropRead()
{
    for ( Section* pSection = (Section*)First(); pSection; pSection = (Section*)Next() )
        delete pSection;
}

//  -----------------------------------------------------------------------

void PropRead::Read()
{
    for ( Section* pSection = (Section*)First(); pSection; pSection = (Section*)Next() )
        delete pSection;
    Clear();
    if ( mbStatus )
    {
        sal_uInt32  nSections;
        sal_uInt32  nSectionOfs;
        sal_uInt32  nCurrent;
        *mpSvStream >> mnByteOrder >> mnFormat >> mnVersionLo >> mnVersionHi;
        if ( mnByteOrder == 0xfffe )
        {
            sal_uInt8*  pSectCLSID = new sal_uInt8[ 16 ];
            mpSvStream->Read( mApplicationCLSID, 16 );
            *mpSvStream >> nSections;
            if ( nSections > 2 )                // sj: PowerPoint documents are containing max 2 sections
            {
                mbStatus = sal_False;
            }
            else for ( sal_uInt32 i = 0; i < nSections; i++ )
            {
                mpSvStream->Read( pSectCLSID, 16 );
                *mpSvStream >> nSectionOfs;
                nCurrent = mpSvStream->Tell();
                mpSvStream->Seek( nSectionOfs );
                Section aSection( pSectCLSID );
                aSection.Read( mpSvStream );
                AddSection( aSection );
                mpSvStream->Seek( nCurrent );
            }
            delete[] pSectCLSID;
        }
    }
}

//  -----------------------------------------------------------------------

PropRead& PropRead::operator=( PropRead& rPropRead )
{
    Section* pSection;

    if ( this != &rPropRead )
    {
        mbStatus = rPropRead.mbStatus;
        mpSvStream = rPropRead.mpSvStream;

        mnByteOrder = rPropRead.mnByteOrder;
        mnFormat = rPropRead.mnFormat;
        mnVersionLo = rPropRead.mnVersionLo;
        mnVersionHi = rPropRead.mnVersionHi;
        memcpy( mApplicationCLSID, rPropRead.mApplicationCLSID, 16 );

        for ( pSection = (Section*)First(); pSection; pSection = (Section*)Next() )
            delete pSection;
        Clear();
        for ( pSection = (Section*)rPropRead.First(); pSection; pSection = (Section*)rPropRead.Next() )
            Insert( new Section( *pSection ), LIST_APPEND );
    }
    return *this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
