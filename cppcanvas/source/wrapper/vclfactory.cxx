/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vclfactory.cxx,v $
 * $Revision: 1.11 $
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
#include "precompiled_cppcanvas.hxx"
#include <rtl/instance.hxx>
#include <osl/getglobalmutex.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/rendering/InterpolationMode.hpp>
#include <vcl/window.hxx>
#include <vcl/graph.hxx>
#include <vcl/canvastools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

#include <cppcanvas/vclfactory.hxx>

#include <implbitmapcanvas.hxx>
#include <implspritecanvas.hxx>
#include <implpolypolygon.hxx>
#include <implbitmap.hxx>
#include <implrenderer.hxx>
#include <impltext.hxx>
#include <implsprite.hxx>


using namespace ::com::sun::star;

namespace cppcanvas
{
    /* Singleton handling */
    struct InitInstance
    {
        VCLFactory* operator()()
        {
            return new VCLFactory();
        }
    };

    VCLFactory& VCLFactory::getInstance()
    {
        return *rtl_Instance< VCLFactory, InitInstance, ::osl::MutexGuard,
            ::osl::GetGlobalMutex >::create(
                InitInstance(), ::osl::GetGlobalMutex());
    }

    VCLFactory::VCLFactory()
    {
    }

    VCLFactory::~VCLFactory()
    {
    }

    BitmapCanvasSharedPtr VCLFactory::createCanvas( const ::Window& rVCLWindow )
    {
        return BitmapCanvasSharedPtr(
            new internal::ImplBitmapCanvas(
                uno::Reference< rendering::XBitmapCanvas >(
                    rVCLWindow.GetCanvas(),
                    uno::UNO_QUERY) ) );
    }

    BitmapCanvasSharedPtr VCLFactory::createCanvas( const uno::Reference< rendering::XBitmapCanvas >& xCanvas )
    {
        return BitmapCanvasSharedPtr(
            new internal::ImplBitmapCanvas( xCanvas ) );
    }

    SpriteCanvasSharedPtr VCLFactory::createSpriteCanvas( const ::Window& rVCLWindow ) const
    {
        return SpriteCanvasSharedPtr(
            new internal::ImplSpriteCanvas(
                uno::Reference< rendering::XSpriteCanvas >(
                    rVCLWindow.GetCanvas(),
                    uno::UNO_QUERY) ) );
    }

    SpriteCanvasSharedPtr VCLFactory::createSpriteCanvas( const uno::Reference< rendering::XSpriteCanvas >& xCanvas ) const
    {
        return SpriteCanvasSharedPtr(
            new internal::ImplSpriteCanvas( xCanvas ) );
    }

    SpriteCanvasSharedPtr VCLFactory::createFullscreenSpriteCanvas( const ::Window& rVCLWindow,
                                                                    const Size&     rFullscreenSize ) const
    {
        return SpriteCanvasSharedPtr(
            new internal::ImplSpriteCanvas(
                uno::Reference< rendering::XSpriteCanvas >(
                    rVCLWindow.GetFullscreenCanvas( rFullscreenSize ),
                    uno::UNO_QUERY) ) );
    }

    PolyPolygonSharedPtr VCLFactory::createPolyPolygon( const CanvasSharedPtr&  rCanvas,
                                                        const ::Polygon&        rPoly ) const
    {
        OSL_ENSURE( rCanvas.get() != NULL &&
                    rCanvas->getUNOCanvas().is(),
                    "VCLFactory::createPolyPolygon(): Invalid canvas" );

        if( rCanvas.get() == NULL )
            return PolyPolygonSharedPtr();

        uno::Reference< rendering::XCanvas > xCanvas( rCanvas->getUNOCanvas() );
        if( !xCanvas.is() )
            return PolyPolygonSharedPtr();

        return PolyPolygonSharedPtr(
            new internal::ImplPolyPolygon( rCanvas,
                                           ::vcl::unotools::xPolyPolygonFromPolygon(
                                               xCanvas->getDevice(),
                                               rPoly) ) );
    }

    PolyPolygonSharedPtr VCLFactory::createPolyPolygon( const CanvasSharedPtr&  rCanvas,
                                                        const ::PolyPolygon&    rPolyPoly ) const
    {
        OSL_ENSURE( rCanvas.get() != NULL &&
                    rCanvas->getUNOCanvas().is(),
                    "VCLFactory::createPolyPolygon(): Invalid canvas" );

        if( rCanvas.get() == NULL )
            return PolyPolygonSharedPtr();

        uno::Reference< rendering::XCanvas > xCanvas( rCanvas->getUNOCanvas() );
        if( !xCanvas.is() )
            return PolyPolygonSharedPtr();

        return PolyPolygonSharedPtr(
            new internal::ImplPolyPolygon( rCanvas,
                                           ::vcl::unotools::xPolyPolygonFromPolyPolygon(
                                               xCanvas->getDevice(),
                                               rPolyPoly) ) );
    }

    BitmapSharedPtr VCLFactory::createBitmap( const CanvasSharedPtr&    rCanvas,
                                              const ::Size&             rSize ) const
    {
        OSL_ENSURE( rCanvas.get() != NULL &&
                    rCanvas->getUNOCanvas().is(),
                    "VCLFactory::createBitmap(): Invalid canvas" );

        if( rCanvas.get() == NULL )
            return BitmapSharedPtr();

        uno::Reference< rendering::XCanvas > xCanvas( rCanvas->getUNOCanvas() );
        if( !xCanvas.is() )
            return BitmapSharedPtr();

        return BitmapSharedPtr(
            new internal::ImplBitmap( rCanvas,
                                      xCanvas->getDevice()->createCompatibleBitmap(
                                          ::vcl::unotools::integerSize2DFromSize(rSize) ) ) );
    }

    BitmapSharedPtr VCLFactory::createAlphaBitmap( const CanvasSharedPtr&   rCanvas,
                                                   const ::Size&            rSize ) const
    {
        OSL_ENSURE( rCanvas.get() != NULL &&
                    rCanvas->getUNOCanvas().is(),
                    "VCLFactory::createBitmap(): Invalid canvas" );

        if( rCanvas.get() == NULL )
            return BitmapSharedPtr();

        uno::Reference< rendering::XCanvas > xCanvas( rCanvas->getUNOCanvas() );
        if( !xCanvas.is() )
            return BitmapSharedPtr();

        return BitmapSharedPtr(
            new internal::ImplBitmap( rCanvas,
                                      xCanvas->getDevice()->createCompatibleAlphaBitmap(
                                          ::vcl::unotools::integerSize2DFromSize(rSize) ) ) );
    }

    BitmapSharedPtr VCLFactory::createBitmap( const CanvasSharedPtr&    rCanvas,
                                              const ::Bitmap&           rBitmap ) const
    {
        OSL_ENSURE( rCanvas.get() != NULL &&
                    rCanvas->getUNOCanvas().is(),
                    "VCLFactory::createBitmap(): Invalid canvas" );

        if( rCanvas.get() == NULL )
            return BitmapSharedPtr();

        uno::Reference< rendering::XCanvas > xCanvas( rCanvas->getUNOCanvas() );
        if( !xCanvas.is() )
            return BitmapSharedPtr();

        return BitmapSharedPtr( new internal::ImplBitmap( rCanvas,
                                                          ::vcl::unotools::xBitmapFromBitmap(
                                                              xCanvas->getDevice(),
                                                              rBitmap) ) );
    }

    BitmapSharedPtr VCLFactory::createBitmap( const CanvasSharedPtr&    rCanvas,
                                              const ::BitmapEx&         rBmpEx ) const
    {
        OSL_ENSURE( rCanvas.get() != NULL &&
                    rCanvas->getUNOCanvas().is(),
                    "VCLFactory::createBitmap(): Invalid canvas" );

        if( rCanvas.get() == NULL )
            return BitmapSharedPtr();

        uno::Reference< rendering::XCanvas > xCanvas( rCanvas->getUNOCanvas() );
        if( !xCanvas.is() )
            return BitmapSharedPtr();

        return BitmapSharedPtr( new internal::ImplBitmap( rCanvas,
                                                          ::vcl::unotools::xBitmapFromBitmapEx(
                                                              xCanvas->getDevice(),
                                                              rBmpEx) ) );
    }

    RendererSharedPtr VCLFactory::createRenderer( const CanvasSharedPtr&        rCanvas,
                                                  const ::Graphic&              rGraphic,
                                                  const Renderer::Parameters&   rParms ) const
    {
        OSL_ENSURE( rCanvas.get() != NULL &&
                    rCanvas->getUNOCanvas().is(),
                    "VCLFactory::createRenderer(): Invalid canvas" );

        if( rCanvas.get() == NULL )
            return RendererSharedPtr();

        uno::Reference< rendering::XCanvas > xCanvas( rCanvas->getUNOCanvas() );
        if( !xCanvas.is() )
            return RendererSharedPtr();

        if( rGraphic.GetType() == GRAPHIC_GDIMETAFILE )
            return RendererSharedPtr( new internal::ImplRenderer( rCanvas,
                                                                  rGraphic.GetGDIMetaFile(),
                                                                  rParms ) );
        else
            return RendererSharedPtr( new internal::ImplRenderer( rCanvas,
                                                                  rGraphic.GetBitmapEx(),
                                                                  rParms ) );
    }

    RendererSharedPtr VCLFactory::createRenderer( const CanvasSharedPtr&        rCanvas,
                                                  const ::GDIMetaFile&          rMtf,
                                                  const Renderer::Parameters&   rParms ) const
    {
        return RendererSharedPtr( new internal::ImplRenderer( rCanvas,
                                                              rMtf,
                                                              rParms ) );
    }

    SpriteSharedPtr VCLFactory::createAnimatedSprite( const SpriteCanvasSharedPtr& rCanvas, const ::Animation& rAnim ) const
    {
        OSL_ENSURE( rCanvas.get() != NULL &&
                    rCanvas->getUNOCanvas().is(),
                    "VCLFactory::createAnimatedSprite(): Invalid canvas" );

        if( rCanvas.get() == NULL )
            return SpriteSharedPtr();

        uno::Reference< rendering::XCanvas > xCanvas( rCanvas->getUNOCanvas() );
        if( !xCanvas.is() )
            return SpriteSharedPtr();

        uno::Reference< rendering::XSpriteCanvas > xSpriteCanvas( rCanvas->getUNOSpriteCanvas() );
        if( !xSpriteCanvas.is() )
            return SpriteSharedPtr();

        if( rAnim.IsEmpty() )
            return SpriteSharedPtr();

        internal::ImplSpriteCanvas* pSpriteCanvas = dynamic_cast< internal::ImplSpriteCanvas* >( rCanvas.get() );
        if( !pSpriteCanvas )
            return SpriteSharedPtr();

        const USHORT nBitmaps( rAnim.Count() );
        uno::Sequence< uno::Reference< rendering::XBitmap > > aBitmapSequence( nBitmaps );
        uno::Reference< rendering::XBitmap >* pBitmaps = aBitmapSequence.getArray();

        unsigned int i;
        BitmapEx aBmpEx;
        BitmapEx aRestoreBuffer;
        aBmpEx.SetSizePixel( rAnim.GetDisplaySizePixel() );
        aRestoreBuffer.SetSizePixel( rAnim.GetDisplaySizePixel() );
        aBmpEx.Erase( ::Color( 255, 0,0,0 ) ); // clear alpha channel
        aRestoreBuffer = aBmpEx;
        const Point aEmptyPoint;

        for( i=0; i<nBitmaps; ++i )
        {
            const AnimationBitmap& rAnimBmp( rAnim.Get((USHORT)i) );

            // Handle dispose according to GIF spec: a
            // DISPOSE_PREVIOUS does _not_ mean to revert to the
            // previous frame, but to revert to the last frame with
            // DISPOSE_NOT

            // dispose previous
            if( rAnimBmp.eDisposal == DISPOSE_BACK )
            {
                // simply clear bitmap to transparent
                aBmpEx.Erase( ::Color( 255, 0,0,0 ) );
            }
            else if( rAnimBmp.eDisposal == DISPOSE_PREVIOUS )
            {
                // copy in last known full frame
                aBmpEx = aRestoreBuffer;
            }
            // I have exactly _no_ idea what DISPOSE_FULL is supposed
            // to do. It's apparently not set anywhere in our code
            OSL_ENSURE( rAnimBmp.eDisposal!=DISPOSE_FULL,
                        "VCLFactory::createAnimatedSprite(): Somebody set the deprecated DISPOSE_FULL at the Animation" );

            // update display
            aBmpEx.CopyPixel( Rectangle( rAnimBmp.aPosPix,
                                         rAnimBmp.aSizePix ),
                              Rectangle( aEmptyPoint,
                                         rAnimBmp.aSizePix ),
                              &rAnimBmp.aBmpEx );

            // store last DISPOSE_NOT frame, for later
            // DISPOSE_PREVIOUS updates
            if( rAnimBmp.eDisposal == DISPOSE_NOT )
                aRestoreBuffer = aBmpEx;

            pBitmaps[i] = ::vcl::unotools::xBitmapFromBitmapEx(
                xCanvas->getDevice(),
                aBmpEx);
        }

        return pSpriteCanvas->createSpriteFromBitmaps( aBitmapSequence,
                                                       rendering::InterpolationMode::NEAREST_NEIGHBOR );
    }

    TextSharedPtr VCLFactory::createText( const CanvasSharedPtr& rCanvas, const ::rtl::OUString& rText ) const
    {
        return TextSharedPtr( new internal::ImplText( rCanvas,
                                                      rText ) );
    }

}
