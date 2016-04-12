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

#include <config_features.h>

#include <tools/diagnose_ex.h>

#include <math.h>

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <vcl/canvastools.hxx>
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/window.hxx>
#include <vcl/graph.hxx>

#include <basegfx/tools/canvastools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/range/b2irange.hxx>
#include <canvas/canvastools.hxx>
#include <cppcanvas/vclfactory.hxx>
#include <cppcanvas/basegfxfactory.hxx>
#include <avmedia/mediawindow.hxx>
#include <avmedia/modeltools.hxx>

#if HAVE_FEATURE_OPENGL
#include <vcl/opengl/OpenGLContext.hxx>
#endif

#include <com/sun/star/media/XManager.hpp>
#include <com/sun/star/media/XPlayer.hpp>
#include <com/sun/star/media/XPlayerWindow.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include "viewmediashape.hxx"
#include "mediashape.hxx"
#include "tools.hxx"
#include "unoview.hxx"

using namespace ::com::sun::star;

namespace slideshow
{
    namespace internal
    {
        ViewMediaShape::ViewMediaShape( const ViewLayerSharedPtr&                       rViewLayer,
                                        const uno::Reference< drawing::XShape >&        rxShape,
                                        const uno::Reference< uno::XComponentContext >& rxContext ) :
            mpViewLayer( rViewLayer ),
            mpEventHandlerParent(nullptr),
            maWindowOffset( 0, 0 ),
            maBounds(),
            mxShape( rxShape ),
            mxPlayer(),
            mxPlayerWindow(),
            mxComponentContext( rxContext ),
            mbIsSoundEnabled(true)
        {
            ENSURE_OR_THROW( mxShape.is(), "ViewMediaShape::ViewMediaShape(): Invalid Shape" );
            ENSURE_OR_THROW( mpViewLayer, "ViewMediaShape::ViewMediaShape(): Invalid View" );
            ENSURE_OR_THROW( mpViewLayer->getCanvas(), "ViewMediaShape::ViewMediaShape(): Invalid ViewLayer canvas" );
            ENSURE_OR_THROW( mxComponentContext.is(), "ViewMediaShape::ViewMediaShape(): Invalid component context" );

            UnoViewSharedPtr xUnoView(std::dynamic_pointer_cast<UnoView>(rViewLayer));
            if (xUnoView)
            {
                mbIsSoundEnabled = xUnoView->isSoundEnabled();
            }
        }


        ViewMediaShape::~ViewMediaShape()
        {
            try
            {
                endMedia();
            }
            catch (uno::Exception &)
            {
                OSL_FAIL( OUStringToOString(
                                comphelper::anyToString(
                                    cppu::getCaughtException() ),
                                RTL_TEXTENCODING_UTF8 ).getStr() );
            }
        }


        const ViewLayerSharedPtr& ViewMediaShape::getViewLayer() const
        {
            return mpViewLayer;
        }


        void ViewMediaShape::startMedia()
        {
            if( !mxPlayer.is() )
                implInitialize( maBounds );

            if (mxPlayer.is())
                mxPlayer->start();
        }


        void ViewMediaShape::endMedia()
        {
            // shutdown player window
            if( mxPlayerWindow.is() )
            {
                uno::Reference< lang::XComponent > xComponent( mxPlayerWindow, uno::UNO_QUERY );

                if( xComponent.is() )
                    xComponent->dispose();

                mxPlayerWindow.clear();
            }

            mpMediaWindow.disposeAndClear();
            mpEventHandlerParent.disposeAndClear();

            // shutdown player
            if( mxPlayer.is() )
            {
                mxPlayer->stop();

                uno::Reference< lang::XComponent > xComponent( mxPlayer, uno::UNO_QUERY );

                if( xComponent.is() )
                    xComponent->dispose();

                mxPlayer.clear();
            }
        }


        void ViewMediaShape::pauseMedia()
        {
            if (mxPlayer.is())
                mxPlayer->stop();
        }


        void ViewMediaShape::setMediaTime(double fTime)
        {
            if (mxPlayer.is())
                mxPlayer->setMediaTime(fTime);
        }


        bool ViewMediaShape::render( const ::basegfx::B2DRectangle& rBounds ) const
        {
#if !HAVE_FEATURE_AVMEDIA
            (void) rBounds;
#else
            ::cppcanvas::CanvasSharedPtr pCanvas = mpViewLayer->getCanvas();

            if( !pCanvas )
                return false;

            if( !mpMediaWindow.get() && !mxPlayerWindow.is() )
            {
                uno::Reference< graphic::XGraphic > xGraphic;
                uno::Reference< beans::XPropertySet > xPropSet( mxShape, uno::UNO_QUERY );
                if (xPropSet.is())
                {
                    xPropSet->getPropertyValue("FallbackGraphic") >>= xGraphic;
                }

                Graphic aGraphic(xGraphic);
                const BitmapEx aBmp = aGraphic.GetBitmapEx();

                uno::Reference< rendering::XBitmap > xBitmap(vcl::unotools::xBitmapFromBitmapEx(
                    pCanvas->getUNOCanvas()->getDevice(), aBmp));

                rendering::ViewState aViewState;
                aViewState.AffineTransform = pCanvas->getViewState().AffineTransform;

                rendering::RenderState aRenderState;
                ::canvas::tools::initRenderState( aRenderState );

                const ::Size aBmpSize( aBmp.GetSizePixel() );

                const ::basegfx::B2DVector aScale( rBounds.getWidth() / aBmpSize.Width(),
                                                   rBounds.getHeight() / aBmpSize.Height() );
                const basegfx::B2DHomMatrix aTranslation(basegfx::tools::createScaleTranslateB2DHomMatrix(
                    aScale, rBounds.getMinimum()));
                ::canvas::tools::setRenderStateTransform( aRenderState, aTranslation );

                pCanvas->getUNOCanvas()->drawBitmap( xBitmap,
                                                     aViewState,
                                                     aRenderState );
            }
#endif
            return true;
        }

        bool ViewMediaShape::resize( const ::basegfx::B2DRectangle& rNewBounds ) const
        {
            maBounds = rNewBounds;

            ::cppcanvas::CanvasSharedPtr pCanvas = mpViewLayer->getCanvas();

            if( !pCanvas )
                return false;

            if( !mxPlayerWindow.is() )
                return true;

            uno::Reference< beans::XPropertySet > xPropSet( pCanvas->getUNOCanvas()->getDevice(),
                                                            uno::UNO_QUERY );

            uno::Reference< awt::XWindow > xParentWindow;
            if( xPropSet.is() &&
                getPropertyValue( xParentWindow,
                                  xPropSet,
                                  "Window") )
            {
                const awt::Rectangle aRect( xParentWindow->getPosSize() );

                maWindowOffset.X = aRect.X;
                maWindowOffset.Y = aRect.Y;
            }

            ::basegfx::B2DRange aTmpRange;
            ::canvas::tools::calcTransformedRectBounds( aTmpRange,
                                                        rNewBounds,
                                                        mpViewLayer->getTransformation() );
            const ::basegfx::B2IRange& rRangePix(
                ::basegfx::unotools::b2ISurroundingRangeFromB2DRange( aTmpRange ));

            mxPlayerWindow->setEnable( !rRangePix.isEmpty() );

            if( rRangePix.isEmpty() )
                return true;

            const Point aPosPixel( rRangePix.getMinX() + maWindowOffset.X,
                                   rRangePix.getMinY() + maWindowOffset.Y );
            const Size  aSizePixel( rRangePix.getMaxX() - rRangePix.getMinX(),
                                    rRangePix.getMaxY() - rRangePix.getMinY() );

            if( mpMediaWindow.get() )
            {
                if( mpEventHandlerParent )
                {
                    mpEventHandlerParent->SetPosSizePixel( aPosPixel, aSizePixel );
                    mpMediaWindow->SetPosSizePixel( Point(0,0), aSizePixel );
                }
                else
                {
                    mpMediaWindow->SetPosSizePixel( aPosPixel, aSizePixel );
                }
                mxPlayerWindow->setPosSize( 0, 0,
                                            aSizePixel.Width(), aSizePixel.Height(),
                                            0 );
            }
            else
            {
                mxPlayerWindow->setPosSize( aPosPixel.X(), aPosPixel.Y(),
                                            aSizePixel.Width(), aSizePixel.Height(),
                                            0 );
            }

            return true;
        }


        bool ViewMediaShape::implInitialize( const ::basegfx::B2DRectangle& rBounds )
        {
            if( !mxPlayer.is() && mxShape.is() )
            {
                ENSURE_OR_RETURN_FALSE( mpViewLayer->getCanvas(),
                                   "ViewMediaShape::implInitialize(): Invalid layer canvas" );

                uno::Reference< rendering::XCanvas > xCanvas( mpViewLayer->getCanvas()->getUNOCanvas() );

                if( xCanvas.is() )
                {
                    uno::Reference< beans::XPropertySet >   xPropSet;
                    try
                    {
                        xPropSet.set( mxShape, uno::UNO_QUERY );
                        OUString sMimeType;

                        // create Player
                        if (xPropSet.is())
                        {
                            OUString aURL;
                            xPropSet->getPropertyValue("MediaMimeType") >>= sMimeType;
                            if ((xPropSet->getPropertyValue("PrivateTempFileURL") >>= aURL)
                                && !aURL.isEmpty())
                            {
                                implInitializeMediaPlayer( aURL, sMimeType );
                            }
                            else if (xPropSet->getPropertyValue("MediaURL") >>= aURL)
                            {
                                implInitializeMediaPlayer( aURL, sMimeType );
                            }
                        }

                        // create visible object
                        uno::Sequence< uno::Any > aDeviceParams;

                        if( ::canvas::tools::getDeviceInfo( xCanvas, aDeviceParams ).getLength() > 1 )
                        {
                            implInitializePlayerWindow( rBounds, aDeviceParams, sMimeType );
                        }

                        // set player properties
                        implSetMediaProperties( xPropSet );
                    }
                    catch( uno::RuntimeException& )
                    {
                        throw;
                    }
                    catch( uno::Exception& )
                    {
                        OSL_FAIL( OUStringToOString(
                                        comphelper::anyToString( cppu::getCaughtException() ),
                                        RTL_TEXTENCODING_UTF8 ).getStr() );
                    }
                }
            }

            return mxPlayer.is() || mxPlayerWindow.is();
        }


        void ViewMediaShape::implSetMediaProperties( const uno::Reference< beans::XPropertySet >& rxProps )
        {
            if( mxPlayer.is() )
            {
                mxPlayer->setMediaTime( 0.0 );

                if( rxProps.is() )
                {
                    bool bLoop( false );
                    getPropertyValue( bLoop,
                                      rxProps,
                                      "Loop");
                    mxPlayer->setPlaybackLoop( bLoop );

                    bool bMute( false );
                    getPropertyValue( bMute,
                                      rxProps,
                                      "Mute");
                    mxPlayer->setMute( bMute || !mbIsSoundEnabled);

                    sal_Int16 nVolumeDB(0);
                    getPropertyValue( nVolumeDB,
                                      rxProps,
                                      "VolumeDB");
                    mxPlayer->setVolumeDB( nVolumeDB );

                    if( mxPlayerWindow.is() )
                    {
                        media::ZoomLevel eZoom(media::ZoomLevel_FIT_TO_WINDOW);
                        getPropertyValue( eZoom,
                                          rxProps,
                                          "Zoom");
                        mxPlayerWindow->setZoomLevel( eZoom );
                    }
                }
            }
        }


        void ViewMediaShape::implInitializeMediaPlayer( const OUString& rMediaURL, const OUString& rMimeType )
        {
#if !HAVE_FEATURE_AVMEDIA
            (void) rMediaURL;
            (void) rMimeType;
#else
            if( !mxPlayer.is() )
            {
                try
                {
                    if( !rMediaURL.isEmpty() )
                    {
                        mxPlayer.set( avmedia::MediaWindow::createPlayer( rMediaURL, ""/*TODO!*/, &rMimeType ),
                            uno::UNO_QUERY );
                    }
                }
                catch( uno::RuntimeException& )
                {
                    throw;
                }
                catch( const uno::Exception& )
                {
                    throw lang::NoSupportException( "No video support for " + rMediaURL );
                }
            }
#endif
        }


        void ViewMediaShape::implInitializePlayerWindow( const ::basegfx::B2DRectangle&   rBounds,
                                                                 const uno::Sequence< uno::Any >& rVCLDeviceParams,
                                                                 const OUString& rMimeType )
        {
            OSL_TRACE( "ViewMediaShape::implInitializePlayerWindow" );
            if( !mpMediaWindow.get() && !rBounds.isEmpty() )
            {
                try
                {
                    sal_Int64 aVal=0;

                    rVCLDeviceParams[ 1 ] >>= aVal;

                    vcl::Window* pWindow = reinterpret_cast< vcl::Window* >( aVal );

                    if( pWindow )
                    {
                        ::basegfx::B2DRange aTmpRange;
                        ::canvas::tools::calcTransformedRectBounds( aTmpRange,
                                                                    rBounds,
                                                                    mpViewLayer->getTransformation() );
                        const ::basegfx::B2IRange& rRangePix(
                            ::basegfx::unotools::b2ISurroundingRangeFromB2DRange( aTmpRange ));

                        if( !rRangePix.isEmpty() )
                        {
                            uno::Sequence< uno::Any >   aArgs( 3 );
                            awt::Rectangle              aAWTRect( rRangePix.getMinX(),
                                                                  rRangePix.getMinY(),
                                                                    rRangePix.getMaxX() - rRangePix.getMinX(),
                                                                    rRangePix.getMaxY() - rRangePix.getMinY() );
#if !HAVE_FEATURE_GLTF || !HAVE_FEATURE_OPENGL
                            (void)rMimeType;
#else
                            if( avmedia::IsModel(rMimeType) )
                            {
                                mpMediaWindow.disposeAndClear();
                                mpEventHandlerParent.disposeAndClear();
                                mpEventHandlerParent = VclPtr<vcl::Window>::Create(pWindow, WB_NOBORDER|WB_NODIALOGCONTROL);
                                mpEventHandlerParent->SetPosSizePixel( Point( aAWTRect.X, aAWTRect.Y ),
                                                           Size( aAWTRect.Width, aAWTRect.Height ) );
                                mpEventHandlerParent->EnablePaint(false);
                                mpEventHandlerParent->Show();
                                SystemWindowData aWinData = OpenGLContext::generateWinData(mpEventHandlerParent.get(), false);
                                mpMediaWindow = VclPtr<SystemChildWindow>::Create(mpEventHandlerParent.get(), 0, &aWinData);
                                mpMediaWindow->SetPosSizePixel( Point( 0, 0 ),
                                                           Size( aAWTRect.Width, aAWTRect.Height ) );
                            }
                            else
#endif
                            {
                                mpMediaWindow.disposeAndClear();
                                mpMediaWindow = VclPtr<SystemChildWindow>::Create( pWindow, WB_CLIPCHILDREN );
                                mpMediaWindow->SetPosSizePixel( Point( aAWTRect.X, aAWTRect.Y ),
                                                           Size( aAWTRect.Width, aAWTRect.Height ) );
                            }
                            mpMediaWindow->SetBackground( Color( COL_BLACK ) );
                            mpMediaWindow->SetParentClipMode( ParentClipMode::NoClip );
                            mpMediaWindow->EnableEraseBackground( false );
                            mpMediaWindow->EnablePaint( false );
                            mpMediaWindow->SetForwardKey( true );
                            mpMediaWindow->SetMouseTransparent( true );
                            mpMediaWindow->Show();

                            if( mxPlayer.is() )
                            {
                                aArgs[ 0 ] = uno::makeAny(
                                    sal::static_int_cast< sal_IntPtr >( mpMediaWindow->GetParentWindowHandle() ) );

                                aAWTRect.X = aAWTRect.Y = 0;
                                aArgs[ 1 ] = uno::makeAny( aAWTRect );
                                aArgs[ 2 ] = uno::makeAny( reinterpret_cast< sal_IntPtr >( mpMediaWindow.get() ) );

                                mxPlayerWindow.set( mxPlayer->createPlayerWindow( aArgs ) );

                                if( mxPlayerWindow.is() )
                                {
                                    mxPlayerWindow->setVisible( true );
                                    mxPlayerWindow->setEnable( true );
                                }
                            }

                            if( !mxPlayerWindow.is() )
                            {
                                //if there was no playerwindow, then clear the mpMediaWindow too
                                //so that we can draw a placeholder instead in that space
                                mpMediaWindow.disposeAndClear();
                                mpEventHandlerParent.disposeAndClear();
                            }
                        }
                    }
                }
                catch( uno::RuntimeException& )
                {
                    throw;
                }
                catch( uno::Exception& )
                {
                    OSL_FAIL( OUStringToOString(
                                    comphelper::anyToString( cppu::getCaughtException() ),
                                    RTL_TEXTENCODING_UTF8 ).getStr() );
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
