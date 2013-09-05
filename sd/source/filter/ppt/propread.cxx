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

#include <propread.hxx>
#include "rtl/tencinfo.h"
#include "rtl/textenc.h"

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

PropEntry& PropEntry::operator=(const PropEntry& rPropEntry)
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

static sal_Int32 lcl_getMaxSafeStrLen(sal_uInt32 nSize)
{
    nSize -= 1; //Drop NULL terminator

    //If it won't fit in a string, clip it to the max size that does
    if (nSize > SAL_MAX_INT32)
        nSize = SAL_MAX_INT32;

    return static_cast< sal_Int32 >( nSize );
}

sal_Bool PropItem::Read( OUString& rString, sal_uInt32 nStringType, sal_Bool bAlign )
{
    sal_uInt32  i, nItemSize, nType, nItemPos;
    sal_Bool    bRetValue = sal_False;

    nItemPos = Tell();

    if ( nStringType == VT_EMPTY )
    {
        nType = VT_NULL; // Initialize in case stream fails.
        *this >> nType;
    }
    else
        nType = nStringType & VT_TYPEMASK;

    nItemSize = 0; // Initialize in case stream fails.
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
                            rString = OUString(pWString, lcl_getMaxSafeStrLen(nItemSize));
                        }
                        else
                            rString = OUString();
                        bRetValue = sal_True;
                    }
                    else
                    {
                        SvMemoryStream::Read( pString, nItemSize );
                        if ( pString[ nItemSize - 1 ] == 0 )
                        {
                            if ( nItemSize > 1 )
                                rString = OUString(pString, rtl_str_getLength(pString), mnTextEnc);
                            else
                                rString = OUString();
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
                            rString = OUString(pString, lcl_getMaxSafeStrLen(nItemSize));
                        else
                            rString = OUString();
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

Section::Section( const Section& rSection )
    : mnTextEnc(rSection.mnTextEnc),
    maEntries(rSection.maEntries.clone())
{
    for ( int i = 0; i < 16; i++ )
        aFMTID[ i ] = rSection.aFMTID[ i ];
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
    if ( nId )
    {
        boost::ptr_vector<PropEntry>::const_iterator iter;
        for (iter = maEntries.begin(); iter != maEntries.end(); ++iter)
        {
            if (iter->mnId == nId)
                break;
        }

        if (iter != maEntries.end())
        {
            rPropItem.Clear();
            rPropItem.SetTextEncoding( mnTextEnc );
            rPropItem.Write( iter->mpBuf,iter->mnSize );
            rPropItem.Seek( STREAM_SEEK_TO_BEGIN );
            return sal_True;
        }
    }
    return sal_False;
}

//  -----------------------------------------------------------------------

void Section::AddProperty( sal_uInt32 nId, const sal_uInt8* pBuf, sal_uInt32 nBufSize )
{
    // just a simple id check

    if ( !nId )
        return;
    if ( nId == 0xffffffff )
        nId = 0;

    // do not allow same PropId's, sort
    boost::ptr_vector<PropEntry>::iterator iter;
    for ( iter = maEntries.begin(); iter != maEntries.end(); ++iter )
    {
        if ( iter->mnId == nId )
            maEntries.replace( iter, new PropEntry( nId, pBuf, nBufSize, mnTextEnc ));
        else if ( iter->mnId > nId )
            maEntries.insert( iter, new PropEntry( nId, pBuf, nBufSize, mnTextEnc ));
        else
            continue;
        return;
    }

    maEntries.push_back( new PropEntry( nId, pBuf, nBufSize, mnTextEnc ) );
}

//  -----------------------------------------------------------------------

sal_Bool Section::GetDictionary( Dictionary& rDict )
{
    sal_Bool bRetValue = sal_False;

    boost::ptr_vector<PropEntry>::iterator iter;
    for (iter = maEntries.begin(); iter != maEntries.end(); ++iter)
    {
        if ( iter->mnId == 0 )
            break;
    }

    if ( iter != maEntries.end() )
    {
        sal_uInt32 nDictCount, nId, nSize, nPos;
        SvMemoryStream aStream( (sal_Int8*)iter->mpBuf, iter->mnSize, STREAM_READ );
        aStream.Seek( STREAM_SEEK_TO_BEGIN );
        aStream >> nDictCount;
        for ( sal_uInt32 i = 0; i < nDictCount; i++ )
        {
            aStream >> nId >> nSize;
            if ( nSize )
            {
                OUString aString;
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
                        aString = OUString(pWString, lcl_getMaxSafeStrLen(nSize));
                    }
                    else
                        aString = OUString(pString, lcl_getMaxSafeStrLen(nSize), mnTextEnc);
                    delete[] pString;
                }
                catch( const std::bad_alloc& )
                {
                    OSL_FAIL( "sd Section::GetDictionary bad alloc" );
                }
                if ( aString.isEmpty() )
                    break;
                rDict.insert( std::make_pair(aString,nId) );
            }
            bRetValue = sal_True;
        }
    }
    return bRetValue;
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
        if ( nPropId )                  // do not read dictionary
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
                        {
                        *pStrm >> nTemp;
                        // looks like these are aligned to 4 bytes
                        sal_uInt32 nLength = nPropOfs + nSecOfs + nPropSize + ( nTemp << 1 ) + 4;
                        nPropSize += ( nTemp << 1 ) + 4 + (nLength % 4);
                        }
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
                // make sure we don't overflow the section size
                if( nPropSize > nSecSize - nSecOfs )
                    nPropSize = nSecSize - nSecOfs;
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

Section& Section::operator=( const Section& rSection )
{
    if ( this != &rSection )
    {
        memcpy( (void*)aFMTID, (void*)rSection.aFMTID, 16 );

        maEntries = rSection.maEntries.clone();
    }
    return *this;
}

//  -----------------------------------------------------------------------

PropRead::PropRead( SvStorage& rStorage, const OUString& rName ) :
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
    maSections.push_back( new Section( rSection ) );
}

//  -----------------------------------------------------------------------

const Section* PropRead::GetSection( const sal_uInt8* pFMTID )
{
    boost::ptr_vector<Section>::iterator it;
    for ( it = maSections.begin(); it != maSections.end(); ++it)
    {
        if ( memcmp( it->GetFMTID(), pFMTID, 16 ) == 0 )
            return &(*it);
    }
    return NULL;
}

//  -----------------------------------------------------------------------

void PropRead::Read()
{
    maSections.clear();

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

PropRead& PropRead::operator=( const PropRead& rPropRead )
{
    if ( this != &rPropRead )
    {
        mbStatus = rPropRead.mbStatus;
        mpSvStream = rPropRead.mpSvStream;

        mnByteOrder = rPropRead.mnByteOrder;
        mnFormat = rPropRead.mnFormat;
        mnVersionLo = rPropRead.mnVersionLo;
        mnVersionHi = rPropRead.mnVersionHi;
        memcpy( mApplicationCLSID, rPropRead.mApplicationCLSID, 16 );

        maSections = rPropRead.maSections.clone();
    }
    return *this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
