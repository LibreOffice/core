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

#include <osl/file.h>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/graph.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/cvtgrf.hxx>
#include <memory>
#include <o3tl/make_shared.hxx>

GfxLink::GfxLink()
{
}

GfxLink::GfxLink( std::unique_ptr<sal_uInt8[]> pBuf, sal_uInt32 nSize, GfxLinkType nType )
{
    SAL_WARN_IF( pBuf == nullptr || !nSize, "vcl",
                "GfxLink::GfxLink(): empty/NULL buffer given" );

    meType = nType;
    mnSwapInDataSize = nSize;
    mpSwapInData = std::shared_ptr<sal_uInt8>(pBuf.release(), pBuf.get_deleter());  // std::move(pBuf) does not compile on Jenkins MacOSX (24 May 2016)
}

bool GfxLink::IsEqual( const GfxLink& rGfxLink ) const
{
    bool bIsEqual = false;

    if ( ( mnSwapInDataSize == rGfxLink.mnSwapInDataSize ) && ( meType == rGfxLink.meType ) )
    {
        const sal_uInt8* pSource = GetData();
        const sal_uInt8* pDest = rGfxLink.GetData();
        sal_uInt32 nSourceSize = GetDataSize();
        sal_uInt32 nDestSize = rGfxLink.GetDataSize();
        if ( pSource && pDest && ( nSourceSize == nDestSize ) )
        {
            bIsEqual = memcmp( pSource, pDest, nSourceSize ) == 0;
        }
        else if ( ( pSource == nullptr ) && ( pDest == nullptr ) )
            bIsEqual = true;
    }
    return bIsEqual;
}


bool GfxLink::IsNative() const
{
    return( meType >= GFX_LINK_FIRST_NATIVE_ID && meType <= GFX_LINK_LAST_NATIVE_ID );
}


const sal_uInt8* GfxLink::GetData() const
{
    if( IsSwappedOut() )
        const_cast<GfxLink*>(this)->SwapIn();

    return mpSwapInData.get();
}


void GfxLink::SetPrefSize( const Size& rPrefSize )
{
    maPrefSize = rPrefSize;
    mbPrefSizeValid = true;
}


void GfxLink::SetPrefMapMode( const MapMode& rPrefMapMode )
{
    maPrefMapMode = rPrefMapMode;
    mbPrefMapModeValid = true;
}


bool GfxLink::LoadNative( Graphic& rGraphic )
{
    bool bRet = false;

    if( IsNative() && mnSwapInDataSize )
    {
        const sal_uInt8* pData = GetData();

        if( pData )
        {
            SvMemoryStream    aMemStm;
            ConvertDataFormat nCvtType;

            aMemStm.SetBuffer( const_cast<sal_uInt8*>(pData), mnSwapInDataSize, mnSwapInDataSize );

            switch( meType )
            {
                case GfxLinkType::NativeGif: nCvtType = ConvertDataFormat::GIF; break;
                case GfxLinkType::NativeBmp: nCvtType = ConvertDataFormat::BMP; break;
                case GfxLinkType::NativeJpg: nCvtType = ConvertDataFormat::JPG; break;
                case GfxLinkType::NativePng: nCvtType = ConvertDataFormat::PNG; break;
                case GfxLinkType::NativeTif: nCvtType = ConvertDataFormat::TIF; break;
                case GfxLinkType::NativeWmf: nCvtType = ConvertDataFormat::WMF; break;
                case GfxLinkType::NativeMet: nCvtType = ConvertDataFormat::MET; break;
                case GfxLinkType::NativePct: nCvtType = ConvertDataFormat::PCT; break;
                case GfxLinkType::NativeSvg: nCvtType = ConvertDataFormat::SVG; break;

                default: nCvtType = ConvertDataFormat::Unknown; break;
            }

            if( nCvtType != ConvertDataFormat::Unknown && ( GraphicConverter::Import( aMemStm, rGraphic, nCvtType ) == ERRCODE_NONE ) )
                bRet = true;
        }
    }

    return bRet;
}

void GfxLink::SwapOut()
{
    if( !IsSwappedOut() && mpSwapInData && mnSwapInDataSize )
    {
        ::utl::TempFile aTempFile;

        OUString aURL = aTempFile.GetURL();

        if (!aURL.isEmpty())
        {
            std::shared_ptr<GfxLink::SwapOutData> pSwapOut = std::make_shared<SwapOutData>(aURL);    // aURL is removed in the destructor
            SvStream* pOStm = aTempFile.GetStream(StreamMode::STD_WRITE);
            if (pOStm)
            {
                pOStm->WriteBytes(mpSwapInData.get(), mnSwapInDataSize);
                bool bError = (ERRCODE_NONE != pOStm->GetError());
                aTempFile.CloseStream();

                if( !bError )
                {
                    mpSwapOutData = pSwapOut;
                    mpSwapInData.reset();
                }
            }
        }
    }
}

void GfxLink::SwapIn()
{
    if( IsSwappedOut() )
    {
        auto pData = GetSwapInData();
        if (pData)
        {
            mpSwapInData = pData;
            mpSwapOutData.reset();
        }
    }
}

bool GfxLink::ExportNative( SvStream& rOStream ) const
{
    if( GetDataSize() )
    {
        auto pData = GetSwapInData();
        if (pData)
            rOStream.WriteBytes( pData.get(), mnSwapInDataSize );
    }

    return ( rOStream.GetError() == ERRCODE_NONE );
}

SvStream& WriteGfxLink( SvStream& rOStream, const GfxLink& rGfxLink )
{
    std::unique_ptr<VersionCompat> pCompat(new VersionCompat( rOStream, StreamMode::WRITE, 2 ));

    // Version 1
    rOStream.WriteUInt16( (sal_uInt16)rGfxLink.GetType() ).WriteUInt32( rGfxLink.GetDataSize() ).WriteUInt32( rGfxLink.GetUserId() );

    // Version 2
    WritePair( rOStream, rGfxLink.GetPrefSize() );
    WriteMapMode( rOStream, rGfxLink.GetPrefMapMode() );

    pCompat.reset(); // destructor writes stuff into the header

    if( rGfxLink.GetDataSize() )
    {
        auto pData = rGfxLink.GetSwapInData();
        if (pData)
            rOStream.WriteBytes( pData.get(), rGfxLink.mnSwapInDataSize );
    }

    return rOStream;
}

SvStream& ReadGfxLink( SvStream& rIStream, GfxLink& rGfxLink)
{
    Size            aSize;
    MapMode         aMapMode;
    bool            bMapAndSizeValid( false );
    std::unique_ptr<VersionCompat>  pCompat(new VersionCompat( rIStream, StreamMode::READ ));

    // Version 1
    sal_uInt16 nType(0);
    sal_uInt32 nSize(0), nUserId(0);
    rIStream.ReadUInt16(nType).ReadUInt32(nSize).ReadUInt32(nUserId);

    if( pCompat->GetVersion() >= 2 )
    {
        ReadPair( rIStream, aSize );
        ReadMapMode( rIStream, aMapMode );
        bMapAndSizeValid = true;
    }

    pCompat.reset(); // destructor writes stuff into the header

    auto nRemainingData = rIStream.remainingSize();
    if (nSize > nRemainingData)
    {
        SAL_WARN("vcl", "graphic link stream is smaller than requested size");
        nSize = nRemainingData;
    }

    std::unique_ptr<sal_uInt8[]> pBuf(new sal_uInt8[ nSize ]);
    rIStream.ReadBytes( pBuf.get(), nSize );

    rGfxLink = GfxLink( std::move(pBuf), nSize, (GfxLinkType) nType );
    rGfxLink.SetUserId( nUserId );

    if( bMapAndSizeValid )
    {
        rGfxLink.SetPrefSize( aSize );
        rGfxLink.SetPrefMapMode( aMapMode );
    }

    return rIStream;
}

GfxLink::SwapOutData::SwapOutData(const OUString &aURL) : maURL(aURL)
{
}

GfxLink::SwapOutData::~SwapOutData()
{
    if( maURL.getLength() > 0 )
        osl_removeFile( maURL.pData );
}

std::shared_ptr<sal_uInt8> GfxLink::GetSwapInData() const
{
    if( !IsSwappedOut() )
        return mpSwapInData;

    std::shared_ptr<sal_uInt8> pData;

    SvFileStream aFileStream(mpSwapOutData->maURL, StreamMode::STD_READ);
    pData = o3tl::make_shared_array<sal_uInt8>(mnSwapInDataSize);
    aFileStream.ReadBytes(pData.get(), mnSwapInDataSize);
    bool bError = (ERRCODE_NONE != aFileStream.GetError());
    sal_uInt64 const nActReadSize = aFileStream.Tell();
    if (nActReadSize != mnSwapInDataSize)
        bError = true;
    if (bError)
        pData.reset();
    return pData;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
