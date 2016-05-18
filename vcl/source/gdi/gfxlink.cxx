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
#include <tools/debug.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/tempfile.hxx>
#include <ucbhelper/content.hxx>
#include <vcl/graph.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/cvtgrf.hxx>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <memory>

GfxLink::GfxLink()
{
}

GfxLink::GfxLink( sal_uInt8* pBuf, sal_uInt32 nSize, GfxLinkType nType )
{
    DBG_ASSERT( pBuf != nullptr && nSize,
                "GfxLink::GfxLink(): empty/NULL buffer given" );

    meType = nType;
    mnDataSize = nSize;
    mpSwapInData = std::shared_ptr<sal_uInt8>(pBuf);
}

GfxLink::~GfxLink()
{
}

bool GfxLink::IsEqual( const GfxLink& rGfxLink ) const
{
    bool bIsEqual = false;

    if ( ( mnDataSize == rGfxLink.mnDataSize ) && ( meType == rGfxLink.meType ) )
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

    return( mpSwapInData ? mpSwapInData.get() : nullptr );
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

    if( IsNative() && mnDataSize )
    {
        const sal_uInt8* pData = GetData();

        if( pData )
        {
            SvMemoryStream    aMemStm;
            ConvertDataFormat nCvtType;

            aMemStm.SetBuffer( const_cast<sal_uInt8*>(pData), mnDataSize, mnDataSize );

            switch( meType )
            {
                case GFX_LINK_TYPE_NATIVE_GIF: nCvtType = ConvertDataFormat::GIF; break;
                case GFX_LINK_TYPE_NATIVE_BMP: nCvtType = ConvertDataFormat::BMP; break;
                case GFX_LINK_TYPE_NATIVE_JPG: nCvtType = ConvertDataFormat::JPG; break;
                case GFX_LINK_TYPE_NATIVE_PNG: nCvtType = ConvertDataFormat::PNG; break;
                case GFX_LINK_TYPE_NATIVE_TIF: nCvtType = ConvertDataFormat::TIF; break;
                case GFX_LINK_TYPE_NATIVE_WMF: nCvtType = ConvertDataFormat::WMF; break;
                case GFX_LINK_TYPE_NATIVE_MET: nCvtType = ConvertDataFormat::MET; break;
                case GFX_LINK_TYPE_NATIVE_PCT: nCvtType = ConvertDataFormat::PCT; break;
                case GFX_LINK_TYPE_NATIVE_SVG: nCvtType = ConvertDataFormat::SVG; break;

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
    if( !IsSwappedOut() && mpSwapInData && mnDataSize )
    {
        ::utl::TempFile aTempFile;

        OUString aURL = aTempFile.GetURL();

        if( !aURL.isEmpty() )
        {
            std::shared_ptr<GfxLink::SwapOutData> pSwapOut;
            pSwapOut = std::make_shared<SwapOutData>(aURL);    // aURL is removed in the destructor
            std::unique_ptr<SvStream> xOStm(::utl::UcbStreamHelper::CreateStream( aURL, STREAM_READWRITE | StreamMode::SHARE_DENYWRITE ));
            if( xOStm )
            {
                xOStm->Write( mpSwapInData.get(), mnDataSize );
                bool bError = ( ERRCODE_NONE != xOStm->GetError() );
                xOStm.reset();

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
            rOStream.Write( pData.get(), mnDataSize );
    }

    return ( rOStream.GetError() == ERRCODE_NONE );
}

SvStream& WriteGfxLink( SvStream& rOStream, const GfxLink& rGfxLink )
{
    std::unique_ptr<VersionCompat> pCompat(new VersionCompat( rOStream, StreamMode::WRITE, 2 ));

    // Version 1
    rOStream.WriteUInt16( rGfxLink.GetType() ).WriteUInt32( rGfxLink.GetDataSize() ).WriteUInt32( rGfxLink.GetUserId() );

    // Version 2
    WritePair( rOStream, rGfxLink.GetPrefSize() );
    WriteMapMode( rOStream, rGfxLink.GetPrefMapMode() );

    pCompat.reset(); // destructor writes stuff into the header

    if( rGfxLink.GetDataSize() )
    {
        auto pData = rGfxLink.GetSwapInData();
        if (pData)
            rOStream.Write( pData.get(), rGfxLink.mnDataSize );
    }

    return rOStream;
}

SvStream& ReadGfxLink( SvStream& rIStream, GfxLink& rGfxLink)
{
    Size            aSize;
    MapMode         aMapMode;
    sal_uInt32      nSize;
    sal_uInt32      nUserId;
    sal_uInt16      nType;
    bool            bMapAndSizeValid( false );
    std::unique_ptr<VersionCompat>  pCompat(new VersionCompat( rIStream, StreamMode::READ ));

    // Version 1
    rIStream.ReadUInt16( nType ).ReadUInt32( nSize ).ReadUInt32( nUserId );

    if( pCompat->GetVersion() >= 2 )
    {
        ReadPair( rIStream, aSize );
        ReadMapMode( rIStream, aMapMode );
        bMapAndSizeValid = true;
    }

    pCompat.reset(); // destructor writes stuff into the header

    sal_uInt8* pBuf = new sal_uInt8[ nSize ];
    rIStream.Read( pBuf, nSize );

    rGfxLink = GfxLink( pBuf, nSize, (GfxLinkType) nType );
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

    std::unique_ptr<SvStream> xIStm(::utl::UcbStreamHelper::CreateStream( mpSwapOutData->maURL, STREAM_READWRITE ));
    if( xIStm )
    {
        pData = std::shared_ptr<sal_uInt8>(new sal_uInt8[ mnDataSize ]);
        xIStm->Read( pData.get(), mnDataSize );
        bool bError = ( ERRCODE_NONE != xIStm->GetError() );
        sal_Size nActReadSize = xIStm->Tell();
        if (nActReadSize != mnDataSize)
        {
            bError = true;
        }
        xIStm.reset();

        if( bError )
            pData.reset();
    }
    return pData;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
