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

#include <vcl/rendergraphicrasterizer.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/virdev.hxx>
#include <vcl/unohelp.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/graph.hxx>
#include <tools/stream.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/streamwrap.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#define VCL_SERVICENAME_RASTERIZER_SVG  "com.sun.star.graphic.GraphicRasterizer_RSVG"

using namespace com::sun::star;

namespace vcl
{
// ---------------------------------------------------------
// - maximum extent in pixel for graphics to be rasterized -
// ---------------------------------------------------------

static const sal_uInt32 nRasterizerDefaultExtent = 4096;

// ---------------------------
// - RenderGraphicRasterizer -
// ---------------------------

RenderGraphicRasterizer::RenderGraphicRasterizer( const RenderGraphic& rRenderGraphic ) :
    maRenderGraphic( rRenderGraphic ),
    mfRotateAngle( 0.0 ),
    mfShearAngleX( 0.0 ),
    mfShearAngleY( 0.0 )
{
}

// -------------------------------------------------------------------------

RenderGraphicRasterizer::RenderGraphicRasterizer( const RenderGraphicRasterizer& rRenderGraphicRasterizer ) :
    maRenderGraphic( rRenderGraphicRasterizer.maRenderGraphic ),
    mxRasterizer( rRenderGraphicRasterizer.mxRasterizer ),
    maBitmapEx( rRenderGraphicRasterizer.maBitmapEx ),
    maDefaultSizePixel( rRenderGraphicRasterizer.maDefaultSizePixel ),
    mfRotateAngle( rRenderGraphicRasterizer.mfRotateAngle ),
    mfShearAngleX( rRenderGraphicRasterizer.mfShearAngleX ),
    mfShearAngleY( rRenderGraphicRasterizer.mfShearAngleY )
{
}

// -------------------------------------------------------------------------

RenderGraphicRasterizer::~RenderGraphicRasterizer()
{
}

// -------------------------------------------------------------------------

RenderGraphicRasterizer& RenderGraphicRasterizer::operator=(
    const RenderGraphicRasterizer& rRenderGraphicRasterizer )
{
    maRenderGraphic = rRenderGraphicRasterizer.maRenderGraphic;
    maBitmapEx = rRenderGraphicRasterizer.maBitmapEx;
    maDefaultSizePixel = rRenderGraphicRasterizer.maDefaultSizePixel;
    mfRotateAngle = rRenderGraphicRasterizer.mfRotateAngle;
    mfShearAngleX = rRenderGraphicRasterizer.mfShearAngleX;
    mfShearAngleY = rRenderGraphicRasterizer.mfShearAngleY;
    mxRasterizer = rRenderGraphicRasterizer.mxRasterizer;

    return( *this );
}

// -------------------------------------------------------------------------

const Size& RenderGraphicRasterizer::GetDefaultSizePixel() const
{
    const_cast< RenderGraphicRasterizer* >( this )->InitializeRasterizer();

    return( maDefaultSizePixel );
}

// -------------------------------------------------------------------------

BitmapEx RenderGraphicRasterizer::GetReplacement() const
{
    BitmapEx aRet( Rasterize( GetDefaultSizePixel() ) );

    aRet.SetPrefSize( GetPrefSize() );
    aRet.SetPrefMapMode( GetPrefMapMode() );

    return( aRet );
}

// -------------------------------------------------------------------------

Size RenderGraphicRasterizer::GetPrefSize() const
{
    const Size                      aSizePixel( GetDefaultSizePixel() );
    std::auto_ptr< VirtualDevice >  apCompVDev;
    OutputDevice*                   pCompDev = NULL;

#ifndef NO_GETAPPWINDOW
    pCompDev = Application::GetAppWindow();
#endif

    if( !pCompDev )
    {
        apCompVDev.reset( new VirtualDevice );
        pCompDev = apCompVDev.get();
    }

    return( pCompDev->PixelToLogic( aSizePixel, GetPrefMapMode() ) );
}

// -------------------------------------------------------------------------

MapMode RenderGraphicRasterizer::GetPrefMapMode() const
{
    return( MapMode( MAP_100TH_MM ) );
}

// -------------------------------------------------------------------------

const BitmapEx& RenderGraphicRasterizer::Rasterize( const Size& rSizePixel,
                                                    double fRotateAngle,
                                                    double fShearAngleX,
                                                    double fShearAngleY,
                                                    sal_uInt32 nMaxExtent ) const
{
    const bool bRasterize = !maRenderGraphic.IsEmpty() &&
                            rSizePixel.Width() && rSizePixel.Height() &&
                            ( maBitmapEx.IsEmpty() ||
                              ( rSizePixel != maBitmapEx.GetSizePixel() ) ||
                              ( fRotateAngle != mfRotateAngle ) ||
                              ( fShearAngleX != mfShearAngleX ) ||
                              ( fShearAngleY != mfShearAngleY ) );

    if( bRasterize )
    {
        const_cast< RenderGraphicRasterizer* >( this )->InitializeRasterizer();

        if( mxRasterizer.is() )
        {
            sal_uInt32 nWidth = labs( rSizePixel.Width() );
            sal_uInt32 nHeight = labs( rSizePixel.Height() );

            // limiting the extent of the rastered bitmap
            if( VCL_RASTERIZER_UNLIMITED_EXTENT != nMaxExtent )
            {
                if( VCL_RASTERIZER_DEFAULT_EXTENT == nMaxExtent )
                {
                    nMaxExtent = nRasterizerDefaultExtent;
                }

                if( ( nWidth > nMaxExtent ) || ( nHeight > nMaxExtent ) )
                {
                    const double fScale = static_cast< double >( nMaxExtent ) / ::std::max( nWidth, nHeight );

                    nWidth = FRound( nWidth * fScale );
                    nHeight = FRound( nHeight * fScale );
                }
            }

            if( !ImplRasterizeFromCache( const_cast< RenderGraphicRasterizer& >( *this ),
                                         Size( nWidth, nHeight ), fRotateAngle, fShearAngleX, fShearAngleY ) )
            {
                try
                {
                    const uno::Sequence< beans::PropertyValue > aPropertySeq;
                    const Graphic aRasteredGraphic( mxRasterizer->rasterize( nWidth,
                                                                             nHeight,
                                                                             fRotateAngle,
                                                                             fShearAngleX,
                                                                             fShearAngleY,
                                                                             aPropertySeq ) );

                    maBitmapEx = aRasteredGraphic.GetBitmapEx();
                    mfRotateAngle = fRotateAngle;
                    mfShearAngleX = fShearAngleX;
                    mfShearAngleY = fShearAngleY;

                    ImplUpdateCache( *this );

//                    OSL_TRACE( "Wanted: %d x %d / Got: %d x %d", rSizePixel.Width(), rSizePixel.Height(), maBitmapEx.GetSizePixel().Width(), maBitmapEx.GetSizePixel().Height() );
                }
                catch( ... )
                {
                    OSL_TRACE( "caught exception during rasterization" );
                }
            }
        }
    }

    return( maBitmapEx );
}

// -------------------------------------------------------------------------

void RenderGraphicRasterizer::InitializeRasterizer()
{
    if( !mxRasterizer.is() && !ImplInitializeFromCache( *this ) )
    {
        uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );

        maDefaultSizePixel.Width() = maDefaultSizePixel.Height() = 0;

        if( !maRenderGraphic.IsEmpty() )
        {
            rtl::OUString aServiceName;

            if( 0 == maRenderGraphic.GetGraphicDataMimeType().compareToAscii( "image/svg+xml" ) )
            {
                aServiceName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( VCL_SERVICENAME_RASTERIZER_SVG ) );
            }

            if( aServiceName.getLength() )
            {
                mxRasterizer.set( xFactory->createInstance( aServiceName ), uno::UNO_QUERY );

                if( mxRasterizer.is() )
                {
                    std::auto_ptr< VirtualDevice > apCompVDev;
                    OutputDevice* pCompDev = NULL;

#ifndef NO_GETAPPWINDOW
                    pCompDev = Application::GetAppWindow();
#endif

                    if( !pCompDev )
                    {
                        apCompVDev.reset( new VirtualDevice );
                        pCompDev = apCompVDev.get();
                    }

                    const Size      aDPI( pCompDev->LogicToPixel( Size( 1, 1 ), MAP_INCH ) );
                    awt::Size       aSizePixel;
                    SvMemoryStream  aMemStm( maRenderGraphic.GetGraphicData().get(),
                                             maRenderGraphic.GetGraphicDataLength(),
                                             STREAM_READ );

                    uno::Reference< io::XInputStream > xIStm( new utl::OSeekableInputStreamWrapper( aMemStm ) );

                    try
                    {
                        if( !xIStm.is() || !mxRasterizer->initializeData( xIStm, aDPI.Width(), aDPI.Height(), aSizePixel ) )
                        {
                            mxRasterizer.clear();
                        }
                        else
                        {
                            maDefaultSizePixel.Width() = aSizePixel.Width;
                            maDefaultSizePixel.Height() = aSizePixel.Height;
                        }
                    }
                    catch( ... )
                    {
                        OSL_TRACE( "caught exception during initialization of SVG rasterizer component" );
                        mxRasterizer.clear();
                    }
                }
            }
        }
    }
}

// ------------------------------------------------------------------------------

RenderGraphicRasterizer::RenderGraphicRasterizerCache& RenderGraphicRasterizer::ImplGetCache()
{
    static RenderGraphicRasterizerCache* pCache = NULL;

    if( !pCache )
    {
        pCache = new RenderGraphicRasterizerCache;
    }

    return( *pCache );
}

// ------------------------------------------------------------------------------

bool RenderGraphicRasterizer::ImplInitializeFromCache( RenderGraphicRasterizer& rRasterizer )
{
    RenderGraphicRasterizerCache&   rCache = ImplGetCache();
    bool                            bRet = false;

    for( sal_uInt32 i = 0; i < rCache.size(); ++i )
    {
        const RenderGraphicRasterizer* pCheck = &rCache[ i ];

        if( pCheck && pCheck->mxRasterizer.is() && ( pCheck->maRenderGraphic == rRasterizer.maRenderGraphic ) )
        {
//            OSL_TRACE( "Hit RenderGraphicRasterizer cache for initialization" );

            rRasterizer.mxRasterizer = pCheck->mxRasterizer;
            rRasterizer.maDefaultSizePixel = pCheck->maDefaultSizePixel;

            // put found Rasterizer at begin of deque
            const RenderGraphicRasterizer aFound( rCache[ i ] );

            rCache.erase( rCache.begin() + i );
            rCache.push_front( aFound );


            bRet = true;
        }
    }

    return( bRet );
}

// ------------------------------------------------------------------------------

bool RenderGraphicRasterizer::ImplRasterizeFromCache( RenderGraphicRasterizer& rRasterizer,
                                                      const Size& rSizePixel,
                                                      double fRotateAngle,
                                                      double fShearAngleX,
                                                      double fShearAngleY )
{
    RenderGraphicRasterizerCache&   rCache = ImplGetCache();
    bool                            bRet = false;

    for( sal_uInt32 i = 0; i < rCache.size(); ++i )
    {
        const RenderGraphicRasterizer& rCheck = rCache[ i ];

        if( rCheck.mxRasterizer.is() && rRasterizer.mxRasterizer.is() &&
            ( ( rCheck.mxRasterizer == rRasterizer.mxRasterizer ) ||
              ( rRasterizer.maRenderGraphic == rCheck.maRenderGraphic ) ) &&
            ( rCheck.maBitmapEx.GetSizePixel() == rSizePixel ) &&
            ( rCheck.mfRotateAngle == fRotateAngle ) &&
            ( rCheck.mfShearAngleX == fShearAngleX ) &&
            ( rCheck.mfShearAngleY == fShearAngleY ) )
        {
//            OSL_TRACE( "Hit RenderGraphicRasterizer cache for rasterizing" );

            rRasterizer.maBitmapEx = rCheck.maBitmapEx;
            rRasterizer.mfRotateAngle = fRotateAngle;
            rRasterizer.mfShearAngleX = fShearAngleX;
            rRasterizer.mfShearAngleY = fShearAngleY;

            // put found Rasterizer at begin of deque
            const RenderGraphicRasterizer aFound( rCache[ i ] );

            rCache.erase( rCache.begin() + i );
            rCache.push_front( aFound );

            bRet = true;
        }
    }

    return( bRet );
}

// ------------------------------------------------------------------------------

void RenderGraphicRasterizer::ImplUpdateCache( const RenderGraphicRasterizer& rRasterizer )
{
    RenderGraphicRasterizerCache&   rCache = ImplGetCache();
    const sal_uInt32                nMaxCacheSize = 8;

    if( rCache.size() < nMaxCacheSize )
    {
        rCache.push_front( rRasterizer );
    }
    else
    {
        rCache.pop_back();
        rCache.push_front( rRasterizer );
    }
}

} // VCL
