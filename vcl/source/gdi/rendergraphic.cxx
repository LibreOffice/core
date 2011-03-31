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

#include <vcl/rendergraphic.hxx>
#include <vcl/rendergraphicrasterizer.hxx>
#include <vcl/mapmod.hxx>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>

#include <cstring>
#include <cstdio>

namespace vcl
{

// -----------------
// - RenderGraphic -
// -----------------
RenderGraphic::RenderGraphic( ) :
    mnGraphicDataLength( 0 )
{
}

// -------------------------------------------------------------------------

RenderGraphic::RenderGraphic( const RenderGraphic& rRenderGraphic ) :
    maGraphicData( rRenderGraphic.maGraphicData ),
    mnGraphicDataLength( rRenderGraphic.mnGraphicDataLength ),
    maGraphicDataMimeType( rRenderGraphic.maGraphicDataMimeType ),
    mapPrefMapMode( rRenderGraphic.mapPrefMapMode.get() ? new MapMode( *rRenderGraphic.mapPrefMapMode ) : NULL ),
    mapPrefSize( rRenderGraphic.mapPrefSize.get() ? new Size( *rRenderGraphic.mapPrefSize ) : NULL )
{
}

// -------------------------------------------------------------------------

RenderGraphic::RenderGraphic( const GraphicData& rGraphicData,
                              sal_uInt32 nGraphicDataLength,
                              const rtl::OUString& rGraphicDataMimeType ) :
    maGraphicData( rGraphicData ),
    mnGraphicDataLength( nGraphicDataLength ),
    maGraphicDataMimeType( rGraphicDataMimeType )
{
    ImplCheckData( );
}

// -------------------------------------------------------------------------

RenderGraphic::RenderGraphic( const rtl::OUString& rGraphicDataMimeType,
                              sal_uInt32 nGraphicDataLength,
                              const sal_uInt8* pGraphicData ) :
    maGraphicData(),
    mnGraphicDataLength( nGraphicDataLength ),
    maGraphicDataMimeType( rGraphicDataMimeType )
{
    if( rGraphicDataMimeType.getLength( ) && nGraphicDataLength )
    {
        maGraphicData.reset( new sal_uInt8[ nGraphicDataLength ] );

        if( pGraphicData )
        {
            memcpy( maGraphicData.get(), pGraphicData, nGraphicDataLength );
        }
    }
    else
    {
        ImplCheckData( );
    }
}

// -------------------------------------------------------------------------

RenderGraphic::~RenderGraphic( )
{
}

// -------------------------------------------------------------------------

RenderGraphic& RenderGraphic::operator=(const RenderGraphic& rRenderGraphic )
{
    maGraphicData = rRenderGraphic.maGraphicData;
    mnGraphicDataLength = rRenderGraphic.mnGraphicDataLength;
    maGraphicDataMimeType = rRenderGraphic.maGraphicDataMimeType;
    mapPrefMapMode.reset( rRenderGraphic.mapPrefMapMode.get() ? new MapMode( *rRenderGraphic.mapPrefMapMode ) : NULL );
    mapPrefSize.reset( rRenderGraphic.mapPrefSize.get() ? new Size( *rRenderGraphic.mapPrefSize ) : NULL );

    return( *this );
}

// -------------------------------------------------------------------------

bool RenderGraphic::operator==(const RenderGraphic& rRenderGraphic ) const
{
    return( ( rRenderGraphic.mnGraphicDataLength == mnGraphicDataLength ) &&
            ( rRenderGraphic.maGraphicData == maGraphicData ) &&
            ( rRenderGraphic.maGraphicDataMimeType.equalsIgnoreAsciiCase( maGraphicDataMimeType ) ) );
}

// -------------------------------------------------------------------------

bool RenderGraphic::operator!() const
{
    return( ( 0 == maGraphicDataMimeType.getLength( ) ) ||
            ( 0 == mnGraphicDataLength ) ||
            !maGraphicData.get( ) );
}

// -------------------------------------------------------------------------

bool RenderGraphic::IsEqual( const RenderGraphic& rRenderGraphic ) const
{
    bool bRet = ( rRenderGraphic.mnGraphicDataLength == mnGraphicDataLength ) &&
                ( rRenderGraphic.maGraphicDataMimeType.equalsIgnoreAsciiCase( maGraphicDataMimeType ) );

    if( bRet && mnGraphicDataLength && ( rRenderGraphic.maGraphicData != maGraphicData ) )
    {
        bRet = ( 0 == memcmp( rRenderGraphic.maGraphicData.get( ),
                              maGraphicData.get( ),
                              mnGraphicDataLength ) );
    }

    return( bRet );
}

// -------------------------------------------------------------------------

const MapMode& RenderGraphic::GetPrefMapMode() const
{
    ImplGetDefaults();
    return( *mapPrefMapMode );
}

// -------------------------------------------------------------------------

const Size& RenderGraphic::GetPrefSize() const
{
    ImplGetDefaults();
    return( *mapPrefSize );
}

// -------------------------------------------------------------------------

BitmapEx RenderGraphic::GetReplacement() const
{
    const RenderGraphicRasterizer aRasterizer( *this );

    return( aRasterizer.GetReplacement() );
}

// -------------------------------------------------------------------------

void RenderGraphic::ImplCheckData( )
{
    if( !( *this ) )
    {
        maGraphicData.reset( );
        mnGraphicDataLength = 0;
        maGraphicDataMimeType = ::rtl::OUString();
        mapPrefMapMode.reset();
        mapPrefSize.reset();
    }
}

// -------------------------------------------------------------------------

void RenderGraphic::ImplGetDefaults() const
{
    if( !mapPrefMapMode.get() || !mapPrefSize.get() )
    {
        const RenderGraphicRasterizer aRasterizer( *this );

        mapPrefMapMode.reset( new MapMode( aRasterizer.GetPrefMapMode() ) );
        mapPrefSize.reset( new Size( aRasterizer.GetPrefSize() ) );
    }
}

// -------------------------------------------------------------------------

::SvStream& operator>>( ::SvStream& rIStm, RenderGraphic& rRenderGraphic )
{
    ::VersionCompat aVCompat( rIStm, STREAM_READ );
    String          aGraphicDataMimeType;
    sal_uInt32      nGraphicDataLength = 0;

    rIStm.ReadByteString( aGraphicDataMimeType, RTL_TEXTENCODING_ASCII_US );
    rIStm >> nGraphicDataLength;

    rRenderGraphic = RenderGraphic( aGraphicDataMimeType, nGraphicDataLength );

    if( !rRenderGraphic.IsEmpty() )
    {
        rIStm.Read( rRenderGraphic.GetGraphicData().get(), nGraphicDataLength );
    }

    return rIStm;
}

// ------------------------------------------------------------------

::SvStream& operator<<( ::SvStream& rOStm, const RenderGraphic& rRenderGraphic )
{
    ::VersionCompat     aVCompat( rOStm, STREAM_WRITE, 1 );
    const sal_uInt32    nGraphicDataLength = rRenderGraphic.GetGraphicDataLength();

    rOStm.WriteByteString( rRenderGraphic.GetGraphicDataMimeType(), RTL_TEXTENCODING_ASCII_US );
    rOStm << nGraphicDataLength;

    if( nGraphicDataLength )
    {
        rOStm.Write( rRenderGraphic.GetGraphicData().get(), nGraphicDataLength );
    }

    return rOStm;
}

} // VCL
