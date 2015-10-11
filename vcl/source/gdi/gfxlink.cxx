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

GfxLink::GfxLink() :
    meType      ( GFX_LINK_TYPE_NONE ),
    mpBuf       ( NULL ),
    mpSwap      ( NULL ),
    mnBufSize   ( 0 ),
    mnUserId    ( 0UL ),
    mpImpData   ( new ImpGfxLink )
{
}

GfxLink::GfxLink( const GfxLink& rGfxLink ) :
    mpImpData( new ImpGfxLink )
{
    ImplCopy( rGfxLink );
}

GfxLink::GfxLink( sal_uInt8* pBuf, sal_uInt32 nSize, GfxLinkType nType, bool bOwns ) :
    mpImpData( new ImpGfxLink )
{
    DBG_ASSERT( (pBuf != NULL && nSize) || (!bOwns && nSize == 0),
                "GfxLink::GfxLink(): empty/NULL buffer given" );

    meType = nType;
    mnBufSize = nSize;
    mpSwap = NULL;
    mnUserId = 0UL;

    if( bOwns )
        mpBuf = new ImpBuffer( pBuf );
    else if( nSize )
    {
        mpBuf = new ImpBuffer( nSize );
        memcpy( mpBuf->mpBuffer, pBuf, nSize );
    }
    else
        mpBuf = NULL;
}

GfxLink::~GfxLink()
{
    if( mpBuf && !( --mpBuf->mnRefCount ) )
        delete mpBuf;

    if( mpSwap && !( --mpSwap->mnRefCount ) )
        delete mpSwap;

    delete mpImpData;
}

GfxLink& GfxLink::operator=( const GfxLink& rGfxLink )
{
    if( &rGfxLink != this )
    {
        if ( mpBuf && !( --mpBuf->mnRefCount ) )
            delete mpBuf;

        if( mpSwap && !( --mpSwap->mnRefCount ) )
            delete mpSwap;

        ImplCopy( rGfxLink );
    }

    return *this;
}

bool GfxLink::IsEqual( const GfxLink& rGfxLink ) const
{
    bool bIsEqual = false;

    if ( ( mnBufSize == rGfxLink.mnBufSize ) && ( meType == rGfxLink.meType ) )
    {
        const sal_uInt8* pSource = GetData();
        const sal_uInt8* pDest = rGfxLink.GetData();
        sal_uInt32 nSourceSize = GetDataSize();
        sal_uInt32 nDestSize = rGfxLink.GetDataSize();
        if ( pSource && pDest && ( nSourceSize == nDestSize ) )
        {
            bIsEqual = memcmp( pSource, pDest, nSourceSize ) == 0;
        }
        else if ( ( pSource == 0 ) && ( pDest == 0 ) )
            bIsEqual = true;
    }
    return bIsEqual;
}

void GfxLink::ImplCopy( const GfxLink& rGfxLink )
{
    mnBufSize = rGfxLink.mnBufSize;
    meType = rGfxLink.meType;
    mpBuf = rGfxLink.mpBuf;
    mpSwap = rGfxLink.mpSwap;
    mnUserId = rGfxLink.mnUserId;
    *mpImpData = *rGfxLink.mpImpData;

    if( mpBuf )
        mpBuf->mnRefCount++;

    if( mpSwap )
        mpSwap->mnRefCount++;
}


bool GfxLink::IsNative() const
{
    return( meType >= GFX_LINK_FIRST_NATIVE_ID && meType <= GFX_LINK_LAST_NATIVE_ID );
}


const sal_uInt8* GfxLink::GetData() const
{
    if( IsSwappedOut() )
        const_cast<GfxLink*>(this)->SwapIn();

    return( mpBuf ? mpBuf->mpBuffer : NULL );
}


void GfxLink::SetPrefSize( const Size& rPrefSize )
{
    mpImpData->maPrefSize = rPrefSize;
    mpImpData->mbPrefSizeValid = true;
}



void GfxLink::SetPrefMapMode( const MapMode& rPrefMapMode )
{
    mpImpData->maPrefMapMode = rPrefMapMode;
    mpImpData->mbPrefMapModeValid = true;
}


bool GfxLink::LoadNative( Graphic& rGraphic )
{
    bool bRet = false;

    if( IsNative() && mnBufSize )
    {
        const sal_uInt8* pData = GetData();

        if( pData )
        {
            SvMemoryStream    aMemStm;
            ConvertDataFormat nCvtType;

            aMemStm.SetBuffer( const_cast<sal_uInt8*>(pData), mnBufSize, false, mnBufSize );

            switch( meType )
            {
                case( GFX_LINK_TYPE_NATIVE_GIF ): nCvtType = ConvertDataFormat::GIF; break;

                // #i15508# added BMP type for better exports (reload when swapped - checked, works)
                case( GFX_LINK_TYPE_NATIVE_BMP ): nCvtType = ConvertDataFormat::BMP; break;

                case( GFX_LINK_TYPE_NATIVE_JPG ): nCvtType = ConvertDataFormat::JPG; break;
                case( GFX_LINK_TYPE_NATIVE_PNG ): nCvtType = ConvertDataFormat::PNG; break;
                case( GFX_LINK_TYPE_NATIVE_TIF ): nCvtType = ConvertDataFormat::TIF; break;
                case( GFX_LINK_TYPE_NATIVE_WMF ): nCvtType = ConvertDataFormat::WMF; break;
                case( GFX_LINK_TYPE_NATIVE_MET ): nCvtType = ConvertDataFormat::MET; break;
                case( GFX_LINK_TYPE_NATIVE_PCT ): nCvtType = ConvertDataFormat::PCT; break;
                case( GFX_LINK_TYPE_NATIVE_SVG ): nCvtType = ConvertDataFormat::SVG; break;

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
    if( !IsSwappedOut() && mpBuf )
    {
        mpSwap = new ImpSwap( mpBuf->mpBuffer, mnBufSize );

        if( !mpSwap->IsSwapped() )
        {
            delete mpSwap;
            mpSwap = NULL;
        }
        else
        {
            if( !( --mpBuf->mnRefCount ) )
                delete mpBuf;

            mpBuf = NULL;
        }
    }
}

void GfxLink::SwapIn()
{
    if( IsSwappedOut() )
    {
        mpBuf = new ImpBuffer( mpSwap->GetData() );

        if( !( --mpSwap->mnRefCount ) )
            delete mpSwap;

        mpSwap = NULL;
    }
}

bool GfxLink::ExportNative( SvStream& rOStream ) const
{
    if( GetDataSize() )
    {
        if( IsSwappedOut() )
            mpSwap->WriteTo( rOStream );
        else if( GetData() )
            rOStream.Write( GetData(), GetDataSize() );
    }

    return ( rOStream.GetError() == ERRCODE_NONE );
}

SvStream& WriteGfxLink( SvStream& rOStream, const GfxLink& rGfxLink )
{
    VersionCompat* pCompat = new VersionCompat( rOStream, StreamMode::WRITE, 2 );

    // Version 1
    rOStream.WriteUInt16( rGfxLink.GetType() ).WriteUInt32( rGfxLink.GetDataSize() ).WriteUInt32( rGfxLink.GetUserId() );

    // Version 2
    WritePair( rOStream, rGfxLink.GetPrefSize() );
    WriteMapMode( rOStream, rGfxLink.GetPrefMapMode() );

    delete pCompat;

    if( rGfxLink.GetDataSize() )
    {
        if( rGfxLink.IsSwappedOut() )
            rGfxLink.mpSwap->WriteTo( rOStream );
        else if( rGfxLink.GetData() )
            rOStream.Write( rGfxLink.GetData(), rGfxLink.GetDataSize() );
    }

    return rOStream;
}

SvStream& ReadGfxLink( SvStream& rIStream, GfxLink& rGfxLink)
{
    Size            aSize;
    MapMode         aMapMode;
    sal_uInt32      nSize;
    sal_uInt32      nUserId;
    sal_uInt16          nType;
    sal_uInt8*          pBuf;
    bool            bMapAndSizeValid( false );
    VersionCompat*  pCompat = new VersionCompat( rIStream, StreamMode::READ );

    // Version 1
    rIStream.ReadUInt16( nType ).ReadUInt32( nSize ).ReadUInt32( nUserId );

    if( pCompat->GetVersion() >= 2 )
    {
        ReadPair( rIStream, aSize );
        ReadMapMode( rIStream, aMapMode );
        bMapAndSizeValid = true;
    }

    delete pCompat;

    pBuf = new sal_uInt8[ nSize ];
    rIStream.Read( pBuf, nSize );

    rGfxLink = GfxLink( pBuf, nSize, (GfxLinkType) nType, true );
    rGfxLink.SetUserId( nUserId );

    if( bMapAndSizeValid )
    {
        rGfxLink.SetPrefSize( aSize );
        rGfxLink.SetPrefMapMode( aMapMode );
    }

    return rIStream;
}

ImpSwap::ImpSwap( sal_uInt8* pData, sal_uLong nDataSize ) :
            mnDataSize( nDataSize ),
            mnRefCount( 1UL )
{
    if( pData && mnDataSize )
    {
        ::utl::TempFile aTempFile;

        maURL = aTempFile.GetURL();
        if( !maURL.isEmpty() )
        {
            std::unique_ptr<SvStream> xOStm(::utl::UcbStreamHelper::CreateStream( maURL, STREAM_READWRITE | StreamMode::SHARE_DENYWRITE ));
            if( xOStm )
            {
                xOStm->Write( pData, mnDataSize );
                bool bError = ( ERRCODE_NONE != xOStm->GetError() );
                xOStm.reset();

                if( bError )
                {
                    osl_removeFile( maURL.pData );
                    maURL.clear();
                }
            }
        }
    }
}

ImpSwap::~ImpSwap()
{
    if( IsSwapped() )
        osl_removeFile( maURL.pData );
}

sal_uInt8* ImpSwap::GetData() const
{
    sal_uInt8* pData;

    if( IsSwapped() )
    {
        std::unique_ptr<SvStream> xIStm(::utl::UcbStreamHelper::CreateStream( maURL, STREAM_READWRITE ));
        if( xIStm )
        {
            pData = new sal_uInt8[ mnDataSize ];
            xIStm->Read( pData, mnDataSize );
            bool bError = ( ERRCODE_NONE != xIStm->GetError() );
            sal_Size nActReadSize = xIStm->Tell();
            if (nActReadSize != mnDataSize)
            {
                bError = true;
            }
            xIStm.reset();

            if( bError )
                delete[] pData, pData = NULL;
        }
        else
            pData = NULL;
    }
    else
        pData = NULL;

    return pData;
}

void ImpSwap::WriteTo( SvStream& rOStm ) const
{
    sal_uInt8* pData = GetData();

    if( pData )
    {
        rOStm.Write( pData, mnDataSize );
        delete[] pData;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
