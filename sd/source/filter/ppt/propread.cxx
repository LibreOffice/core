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
#include <osl/diagnose.h>
#include <o3tl/make_unique.hxx>

PropEntry::PropEntry( sal_uInt32 nId, const sal_uInt8* pBuf, sal_uInt32 nBufSize ) :
    mnId        ( nId ),
    mnSize      ( nBufSize ),
    mpBuf       ( new sal_uInt8[ nBufSize ] )
{
    memcpy( mpBuf.get(), pBuf, nBufSize );
};

PropEntry::PropEntry( const PropEntry& rProp ) :
    mnId        ( rProp.mnId ),
    mnSize      ( rProp.mnSize ),
    mpBuf       ( new sal_uInt8[ mnSize ] )
{
    memcpy( mpBuf.get(), rProp.mpBuf.get(), mnSize );
};

PropEntry& PropEntry::operator=(const PropEntry& rPropEntry)
{
    if ( this != &rPropEntry )
    {
        mnId = rPropEntry.mnId;
        mnSize = rPropEntry.mnSize;
        mpBuf.reset( new sal_uInt8[ mnSize ] );
        memcpy( mpBuf.get(), rPropEntry.mpBuf.get(), mnSize );
    }
    return *this;
}

void PropItem::Clear()
{
    Seek( STREAM_SEEK_TO_BEGIN );
    delete[] static_cast<sal_uInt8*>(SwitchBuffer());
}

static sal_Int32 lcl_getMaxSafeStrLen(sal_uInt32 nSize)
{
    nSize -= 1; //Drop NULL terminator

    //If it won't fit in a string, clip it to the max size that does
    if (nSize > SAL_MAX_INT32)
        nSize = SAL_MAX_INT32;

    return static_cast< sal_Int32 >( nSize );
}

bool PropItem::Read( OUString& rString, sal_uInt32 nStringType, bool bAlign )
{
    sal_uInt32 nType, nItemPos;
    bool    bRetValue = false;

    nItemPos = Tell();

    if ( nStringType == VT_EMPTY )
    {
        nType = VT_NULL; // Initialize in case stream fails.
        ReadUInt32( nType );
    }
    else
        nType = nStringType & VT_TYPEMASK;

    sal_uInt32 nItemSize(0); // Initialize in case stream fails.
    ReadUInt32(nItemSize);

    switch( nType )
    {
        case VT_LPSTR :
        {
            if (nItemSize)
            {
                auto nMaxSizePossible = remainingSize();
                if (nItemSize > nMaxSizePossible)
                {
                    SAL_WARN("sd.filter", "String of Len " << nItemSize << " claimed, only " << nMaxSizePossible << " possible");
                    nItemSize = nMaxSizePossible;
                }
            }

            if (nItemSize)
            {
                try
                {
                    std::unique_ptr<sal_Char[]> pString( new sal_Char[ nItemSize ] );
                    if ( mnTextEnc == RTL_TEXTENCODING_UCS2 )
                    {
                        nItemSize >>= 1;
                        if ( nItemSize > 1 )
                        {
                            sal_Unicode* pWString = reinterpret_cast<sal_Unicode*>(pString.get());
                            for (sal_uInt32 i = 0; i < nItemSize; ++i)
                                ReadUtf16( pWString[ i ] );
                            rString = OUString(pWString, lcl_getMaxSafeStrLen(nItemSize));
                        }
                        else
                            rString.clear();
                        bRetValue = true;
                    }
                    else
                    {
                        SvMemoryStream::ReadBytes(pString.get(), nItemSize);
                        if ( pString[ nItemSize - 1 ] == 0 )
                        {
                            if ( nItemSize > 1 )
                                rString = OUString(pString.get(), rtl_str_getLength(pString.get()), mnTextEnc);
                            else
                                rString.clear();
                            bRetValue = true;
                        }
                    }
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
            if (nItemSize)
            {
                auto nMaxSizePossible = remainingSize() / sizeof(sal_Unicode);
                if (nItemSize > nMaxSizePossible)
                {
                    SAL_WARN("sd.filter", "String of Len " << nItemSize << " claimed, only " << nMaxSizePossible << " possible");
                    nItemSize = nMaxSizePossible;
                }
            }

            if (nItemSize)
            {
                try
                {
                    std::unique_ptr<sal_Unicode[]> pString( new sal_Unicode[ nItemSize ] );
                    for (sal_uInt32 i = 0; i < nItemSize; ++i)
                        ReadUtf16( pString[ i ] );
                    if ( pString[ nItemSize - 1 ] == 0 )
                    {
                        if ( (sal_uInt16)nItemSize > 1 )
                            rString = OUString(pString.get(), lcl_getMaxSafeStrLen(nItemSize));
                        else
                            rString.clear();
                        bRetValue = true;
                    }
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

PropItem& PropItem::operator=( PropItem& rPropItem )
{
    if ( this != &rPropItem )
    {
        Seek( STREAM_SEEK_TO_BEGIN );
        delete[] static_cast<sal_uInt8*>(SwitchBuffer());

        mnTextEnc = rPropItem.mnTextEnc;
        sal_uInt32 nItemPos = rPropItem.Tell();
        rPropItem.Seek( STREAM_SEEK_TO_END );
        SvMemoryStream::WriteBytes(rPropItem.GetData(), rPropItem.Tell());
        rPropItem.Seek( nItemPos );
    }
    return *this;
}

Section::Section( const Section& rSection )
    : mnTextEnc(rSection.mnTextEnc)
{
    for ( int i = 0; i < 16; i++ )
        aFMTID[ i ] = rSection.aFMTID[ i ];
    for(const std::unique_ptr<PropEntry>& rEntry : rSection.maEntries)
        maEntries.push_back(o3tl::make_unique<PropEntry>(*rEntry.get()));
}

Section::Section( const sal_uInt8* pFMTID )
{
    mnTextEnc = RTL_TEXTENCODING_MS_1252;
    for ( int i = 0; i < 16; i++ )
        aFMTID[ i ] = pFMTID[ i ];
}

bool Section::GetProperty( sal_uInt32 nId, PropItem& rPropItem )
{
    if ( nId )
    {
        std::vector<std::unique_ptr<PropEntry> >::const_iterator iter;
        for (iter = maEntries.begin(); iter != maEntries.end(); ++iter)
        {
            if ((*iter)->mnId == nId)
                break;
        }

        if (iter != maEntries.end())
        {
            rPropItem.Clear();
            rPropItem.SetTextEncoding( mnTextEnc );
            rPropItem.WriteBytes( (*iter)->mpBuf.get(), (*iter)->mnSize );
            rPropItem.Seek( STREAM_SEEK_TO_BEGIN );
            return true;
        }
    }
    return false;
}

void Section::AddProperty( sal_uInt32 nId, const sal_uInt8* pBuf, sal_uInt32 nBufSize )
{
    // just a simple id check

    if ( !nId )
        return;
    if ( nId == 0xffffffff )
        nId = 0;

    // do not allow same PropId's, sort
    std::vector<std::unique_ptr<PropEntry> >::iterator iter;
    for ( iter = maEntries.begin(); iter != maEntries.end(); ++iter )
    {
        if ( (*iter)->mnId == nId )
            (*iter).reset(new PropEntry( nId, pBuf, nBufSize ));
        else if ( (*iter)->mnId > nId )
            maEntries.insert( iter, o3tl::make_unique<PropEntry>( nId, pBuf, nBufSize ));
        else
            continue;
        return;
    }

    maEntries.push_back( o3tl::make_unique<PropEntry>( nId, pBuf, nBufSize ) );
}

void Section::GetDictionary(Dictionary& rDict)
{
    std::vector<std::unique_ptr<PropEntry> >::iterator iter;
    for (iter = maEntries.begin(); iter != maEntries.end(); ++iter)
    {
        if ( (*iter)->mnId == 0 )
            break;
    }

    if (iter == maEntries.end())
        return;

    SvMemoryStream aStream( (*iter)->mpBuf.get(), (*iter)->mnSize, StreamMode::READ );
    aStream.Seek( STREAM_SEEK_TO_BEGIN );
    sal_uInt32 nDictCount(0);
    aStream.ReadUInt32( nDictCount );
    for (sal_uInt32 i = 0; i < nDictCount; ++i)
    {
        sal_uInt32 nId(0), nSize(0);
        aStream.ReadUInt32(nId).ReadUInt32(nSize);
        if (!aStream.good() || nSize > aStream.remainingSize())
            break;
        if (mnTextEnc == RTL_TEXTENCODING_UCS2)
            nSize >>= 1;
        if (!nSize)
            continue;
        OUString aString;
        try
        {
            if ( mnTextEnc == RTL_TEXTENCODING_UCS2 )
            {
                std::unique_ptr<sal_Unicode[]> pWString( new sal_Unicode[nSize] );
                for (sal_uInt32 j = 0; j < nSize; ++j)
                    aStream.ReadUtf16(pWString[j]);
                aString = OUString(pWString.get(), lcl_getMaxSafeStrLen(nSize));
            }
            else
            {
                std::unique_ptr<sal_Char[]> pString( new sal_Char[nSize] );
                aStream.ReadBytes(pString.get(), nSize);
                aString = OUString(pString.get(), lcl_getMaxSafeStrLen(nSize), mnTextEnc);
            }
        }
        catch( const std::bad_alloc& )
        {
            OSL_FAIL( "sd Section::GetDictionary bad alloc" );
        }
        if (aString.isEmpty())
            break;
        rDict.insert( std::make_pair(aString,nId) );
    }
}

void Section::Read( SotStorageStream *pStrm )
{
    sal_uInt32 nSecOfs, nPropSize, nStrmSize;
    nSecOfs = pStrm->Tell();

    pStrm->Seek( STREAM_SEEK_TO_END );
    nStrmSize = pStrm->Tell();
    pStrm->Seek( nSecOfs );

    mnTextEnc = RTL_TEXTENCODING_MS_1252;
    sal_uInt32 nSecSize(0), nPropCount(0);
    pStrm->ReadUInt32( nSecSize ).ReadUInt32( nPropCount );
    while (nPropCount--)
    {
        sal_uInt32 nPropId(0), nPropOfs(0);
        pStrm->ReadUInt32(nPropId).ReadUInt32(nPropOfs);
        if (!pStrm->good())
            break;
        auto nCurrent = pStrm->Tell();
        sal_uInt64 nOffset = nPropOfs + nSecOfs;
        if (nOffset != pStrm->Seek(nOffset))
            break;
        if ( nPropId )                  // do not read dictionary
        {
            sal_uInt32 nPropType(0), nVectorCount(0);
            pStrm->ReadUInt32(nPropType);

            nPropSize = 4;

            if ( nPropType & VT_VECTOR )
            {
                pStrm->ReadUInt32( nVectorCount );
                nPropType &=~VT_VECTOR;
                nPropSize += 4;
            }
            else
                nVectorCount = 1;

            bool bVariant = ( nPropType == VT_VARIANT );

            for (sal_uInt32 i = 0; nPropSize && ( i < nVectorCount ); ++i)
            {
                if ( bVariant )
                {
                    pStrm->ReadUInt32( nPropType );
                    nPropSize += 4;
                }
                sal_uInt32 nTemp(0);
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
                        pStrm->ReadUInt32( nTemp );
                        nPropSize += ( nTemp + 4 );
                    break;

                    case VT_LPSTR :
                        pStrm->ReadUInt32( nTemp );
                        nPropSize += ( nTemp + 4 );
                    break;

                    case VT_LPWSTR :
                        {
                        pStrm->ReadUInt32( nTemp );
                        // looks like these are aligned to 4 bytes
                        sal_uInt32 nLength = nPropOfs + nSecOfs + nPropSize + ( nTemp << 1 ) + 4;
                        nPropSize += ( nTemp << 1 ) + 4 + (nLength % 4);
                        }
                    break;

                    case VT_BLOB_OBJECT :
                    case VT_BLOB :
                    case VT_CF :
                        pStrm->ReadUInt32( nTemp );
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
                    {
                        nOffset = nPropOfs + nSecOfs + nPropSize;
                        if (nOffset != pStrm->Seek(nOffset))
                            break;
                    }
                }
                else
                    break;
            }
            if ( nPropSize )
            {
                if ( nPropSize > nStrmSize )
                {
                    break;
                }
                pStrm->Seek( nPropOfs + nSecOfs );
                // make sure we don't overflow the section size
                if( nPropSize > nSecSize - nSecOfs )
                    nPropSize = nSecSize - nSecOfs;
                std::unique_ptr<sal_uInt8[]> pBuf( new sal_uInt8[ nPropSize ] );
                nPropSize = pStrm->ReadBytes(pBuf.get(), nPropSize);
                AddProperty( nPropId, pBuf.get(), nPropSize );
            }
            if ( nPropId == 1 )
            {
                PropItem aPropItem;
                if ( GetProperty( 1, aPropItem ) )
                {
                    aPropItem.ReadUInt32( nPropType );
                    if ( nPropType == VT_I2 )
                    {
                        sal_uInt16 nCodePage(0);
                        aPropItem.ReadUInt16(nCodePage);

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
            sal_uInt32 nDictCount(0);
            pStrm->ReadUInt32(nDictCount);
            auto nMaxRecordsPossible = pStrm->remainingSize() / (sizeof(sal_uInt32)*2);
            if (nDictCount > nMaxRecordsPossible)
            {
                SAL_WARN("sd.filter", "Dictionary count of " << nDictCount << " claimed, only " << nMaxRecordsPossible << " possible");
                nDictCount = nMaxRecordsPossible;
            }
            for (sal_uInt32 i = 0; i < nDictCount; ++i)
            {
                sal_uInt32 nSize(0);
                pStrm->ReadUInt32( nSize ).ReadUInt32( nSize );
                if (!pStrm->good())
                    break;
                sal_uInt64 nPos = pStrm->Tell() + nSize;
                if (nPos != pStrm->Seek(nPos))
                    break;
            }
            sal_uInt32 nSize = pStrm->Tell();
            pStrm->Seek( nPropOfs + nSecOfs );
            nSize -= pStrm->Tell();
            if ( nSize > nStrmSize )
            {
                break;
            }
            std::unique_ptr<sal_uInt8[]> pBuf( new sal_uInt8[ nSize ] );
            nSize = pStrm->ReadBytes(pBuf.get(), nSize);
            AddProperty( 0xffffffff, pBuf.get(), nSize );
        }
        pStrm->Seek(nCurrent);
    }
    pStrm->Seek( nSecOfs + nSecSize );
}

Section& Section::operator=( const Section& rSection )
{
    if ( this != &rSection )
    {
        memcpy( static_cast<void*>(aFMTID), static_cast<void const *>(rSection.aFMTID), 16 );

        for(const std::unique_ptr<PropEntry>& rEntry : rSection.maEntries)
            maEntries.push_back(o3tl::make_unique<PropEntry>(*rEntry.get()));
    }
    return *this;
}

PropRead::PropRead( SotStorage& rStorage, const OUString& rName ) :
        mbStatus            ( false ),
        mnByteOrder         ( 0xfffe ),
        mnFormat            ( 0 ),
        mnVersionLo         ( 4 ),
        mnVersionHi         ( 2 )
{
    if ( rStorage.IsStream( rName ) )
    {
        mpSvStream = rStorage.OpenSotStream( rName, StreamMode::STD_READ );
        if ( mpSvStream.is() )
        {
            mpSvStream->SetEndian( SvStreamEndian::LITTLE );
            memset( mApplicationCLSID, 0, 16 );
            mbStatus = true;
        }
    }
}

const Section* PropRead::GetSection( const sal_uInt8* pFMTID )
{
    std::vector<std::unique_ptr<Section> >::iterator it;
    for ( it = maSections.begin(); it != maSections.end(); ++it)
    {
        if ( memcmp( (*it)->GetFMTID(), pFMTID, 16 ) == 0 )
            return it->get();
    }
    return nullptr;
}

void PropRead::Read()
{
    maSections.clear();

    if ( mbStatus )
    {
        sal_uInt32  nSections;
        sal_uInt32  nSectionOfs;
        sal_uInt32  nCurrent;
        mpSvStream->ReadUInt16( mnByteOrder ).ReadUInt16( mnFormat ).ReadUInt16( mnVersionLo ).ReadUInt16( mnVersionHi );
        if ( mnByteOrder == 0xfffe )
        {
            std::unique_ptr<sal_uInt8[]> pSectCLSID( new sal_uInt8[ 16 ] );
            mpSvStream->ReadBytes(mApplicationCLSID, 16);
            mpSvStream->ReadUInt32( nSections );
            if ( nSections > 2 )                // sj: PowerPoint documents are containing max 2 sections
            {
                mbStatus = false;
            }
            else for ( sal_uInt32 i = 0; i < nSections; i++ )
            {
                mpSvStream->ReadBytes(pSectCLSID.get(), 16);
                mpSvStream->ReadUInt32( nSectionOfs );
                nCurrent = mpSvStream->Tell();
                mpSvStream->Seek( nSectionOfs );
                Section aSection( pSectCLSID.get() );
                aSection.Read( mpSvStream.get() );
                maSections.push_back( o3tl::make_unique<Section>( aSection ) );
                mpSvStream->Seek( nCurrent );
            }
        }
    }
}

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

        for(const std::unique_ptr<Section>& rSection : rPropRead.maSections)
            maSections.push_back(o3tl::make_unique<Section>(*rSection.get()));
    }
    return *this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
