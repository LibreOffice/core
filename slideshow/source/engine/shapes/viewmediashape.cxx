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

#include <comphelper/diagnose_ex.hxx>

#include <sal/log.hxx>
#include <utility>
#include <vcl/canvastools.hxx>
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/window.hxx>
#include <vcl/graph.hxx>

#include <basegfx/utils/canvastools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2irange.hxx>
#include <canvas/canvastools.hxx>
#include <comphelper/DirectoryHelper.hxx>
#include <cppcanvas/canvas.hxx>
#include <avmedia/mediawindow.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdomedia.hxx>

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/media/XPlayer.hpp>
#include <com/sun/star/media/XPlayerWindow.hpp>
#include <com/sun/star/presentation/XSlideShowView.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>

#include "viewmediashape.hxx"
#include <tools.hxx>
#include <unoview.hxx>

using namespace ::com::sun::star;

namespace slideshow::internal
{
        ViewMediaShape::ViewMediaShape( const ViewLayerSharedPtr&                       rViewLayer,
                                        uno::Reference< drawing::XShape >         xShape,
                                        uno::Reference< uno::XComponentContext >  xContext,
                                        const OUString&                           aFallbackDir ) :
            mpViewLayer( rViewLayer ),
            maWindowOffset( 0, 0 ),
            maBounds(),
            mxShape(std::move( xShape )),
            mxPlayer(),
            mxPlayerWindow(),
            mxComponentContext(std::move( xContext )),
            mbIsSoundEnabled(true),
            maFallbackDir(aFallbackDir)
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
            catch (const uno::Exception &)
            {
                TOOLS_WARN_EXCEPTION("slideshow", "");
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
                mxPlayerWindow->dispose();
                mxPlayerWindow.clear();
            }

            mpMediaWindow.disposeAndClear();

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

        void ViewMediaShape::setLooping(bool bLooping)
        {
            if (mxPlayer.is())
            {
                mxPlayer->setPlaybackLoop(bLooping);
            }
        }

        bool ViewMediaShape::render( const ::basegfx::B2DRectangle& rBounds ) const
        {
#if !HAVE_FEATURE_AVMEDIA
            (void) rBounds;
#else
            ::cppcanvas::CanvasSharedPtr pCanvas = mpViewLayer->getCanvas();

            if( !pCanvas )
                return false;

            if( !mpMediaWindow && !mxPlayerWindow.is() )
            {
                uno::Reference< graphic::XGraphic > xGraphic;
                uno::Reference< beans::XPropertySet > xPropSet( mxShape, uno::UNO_QUERY );
                if (xPropSet.is())
                {
                    xPropSet->getPropertyValue(u"FallbackGraphic"_ustr) >>= xGraphic;
                }

                Graphic aGraphic(xGraphic);
                const BitmapEx aBmp = aGraphic.GetBitmapEx();

                uno::Reference< rendering::XBitmap > xBitmap(vcl::unotools::xBitmapFromBitmapEx(aBmp));

                rendering::ViewState aViewState;
                aViewState.AffineTransform = pCanvas->getViewState().AffineTransform;

                rendering::RenderState aRenderState;
                ::canvas::tools::initRenderState( aRenderState );

                const ::Size aBmpSize( aBmp.GetSizePixel() );

                const ::basegfx::B2DVector aScale( rBounds.getWidth() / aBmpSize.Width(),
                                                   rBounds.getHeight() / aBmpSize.Height() );
                const basegfx::B2DHomMatrix aTranslation(basegfx::utils::createScaleTranslateB2DHomMatrix(
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
                                  u"Window"_ustr) )
            {
                const awt::Rectangle aRect( xParentWindow->getPosSize() );

                maWindowOffset.X = aRect.X;
                maWindowOffset.Y = aRect.Y;
            }

            ::basegfx::B2DRange aTmpRange = ::canvas::tools::calcTransformedRectBounds(
                                                        rNewBounds,
                                                        mpViewLayer->getTransformation() );
            const ::basegfx::B2IRange aRangePix(
                ::basegfx::unotools::b2ISurroundingRangeFromB2DRange( aTmpRange ));

            mxPlayerWindow->setEnable( !aRangePix.isEmpty() );

            if( aRangePix.isEmpty() )
                return true;

            awt::Rectangle aCanvasArea;
            UnoViewSharedPtr xUnoView(std::dynamic_pointer_cast<UnoView>(mpViewLayer));
            if (xUnoView)
                aCanvasArea = xUnoView->getUnoView()->getCanvasArea();

            const Point aPosPixel( aRangePix.getMinX() + maWindowOffset.X + aCanvasArea.X,
                                   aRangePix.getMinY() + maWindowOffset.Y + aCanvasArea.Y );
            const Size  aSizePixel( aRangePix.getMaxX() - aRangePix.getMinX(),
                                    aRangePix.getMaxY() - aRangePix.getMinY() );

            if( mpMediaWindow )
            {
                mpMediaWindow->SetPosSizePixel( aPosPixel, aSizePixel );
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
                            xPropSet->getPropertyValue(u"MediaMimeType"_ustr) >>= sMimeType;
                            if ((xPropSet->getPropertyValue(u"PrivateTempFileURL"_ustr) >>= aURL)
                                && !aURL.isEmpty())
                            {
                                implInitializeMediaPlayer( aURL, sMimeType );
                            }
                            else if (xPropSet->getPropertyValue(u"MediaURL"_ustr) >>= aURL)
                            {
                                if ( maFallbackDir.getLength() &&
                                     aURL.startsWith("file:///") &&
                                     !comphelper::DirectoryHelper::fileExists(aURL) )
                                {
                                    auto fileNameStartIdx = aURL.lastIndexOf("/");
                                    if (fileNameStartIdx != -1)
                                    {
                                        aURL = OUString::Concat(maFallbackDir) + aURL.subView(fileNameStartIdx + 1);
                                    }
                                }
                                implInitializeMediaPlayer( aURL, sMimeType );
                            }
                        }

                        // create visible object
                        uno::Sequence< uno::Any > aDeviceParams;

                        if( ::canvas::tools::getDeviceInfo( xCanvas, aDeviceParams ).getLength() > 1 )
                        {
                            implInitializePlayerWindow( rBounds, aDeviceParams );
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
                        TOOLS_WARN_EXCEPTION( "slideshow", "" );
                    }
                }
            }

            return mxPlayer.is() || mxPlayerWindow.is();
        }


        void ViewMediaShape::implSetMediaProperties( const uno::Reference< beans::XPropertySet >& rxProps )
        {
            if( !mxPlayer.is() )
                return;

            mxPlayer->setMediaTime( 0.0 );

            if( !rxProps.is() )
                return;

            bool bLoop( false );
            getPropertyValue( bLoop,
                              rxProps,
                              u"Loop"_ustr);
            mxPlayer->setPlaybackLoop( bLoop );

            bool bMute( false );
            getPropertyValue( bMute,
                              rxProps,
                              u"Mute"_ustr);
            mxPlayer->setMute( bMute || !mbIsSoundEnabled);

            sal_Int16 nVolumeDB(0);
            getPropertyValue( nVolumeDB,
                              rxProps,
                              u"VolumeDB"_ustr);
            mxPlayer->setVolumeDB( nVolumeDB );

            if( mxPlayerWindow.is() )
            {
                media::ZoomLevel eZoom(media::ZoomLevel_FIT_TO_WINDOW);
                getPropertyValue( eZoom,
                                  rxProps,
                                  u"Zoom"_ustr);
                mxPlayerWindow->setZoomLevel( eZoom );
            }
        }


        void ViewMediaShape::implInitializeMediaPlayer( const OUString& rMediaURL, const OUString& rMimeType )
        {
#if !HAVE_FEATURE_AVMEDIA
            (void) rMediaURL;
            (void) rMimeType;
#else
            if( mxPlayer.is() )
                return;

            try
            {
                if( !rMediaURL.isEmpty() )
                {
                    mxPlayer = avmedia::MediaWindow::createPlayer( rMediaURL, u""_ustr/*TODO!*/, &rMimeType );
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
#endif
        }


        void ViewMediaShape::implInitializePlayerWindow( const ::basegfx::B2DRectangle&   rBounds,
                                                                 const uno::Sequence< uno::Any >& rVCLDeviceParams )
        {
            SAL_INFO("slideshow", "ViewMediaShape::implInitializePlayerWindow" );
            if( mpMediaWindow || rBounds.isEmpty() )
                return;

            try
            {
                sal_Int64 aVal=0;

                rVCLDeviceParams[ 1 ] >>= aVal;

                OutputDevice* pDevice = reinterpret_cast<OutputDevice*>(aVal);
                vcl::Window* pWindow = pDevice ? pDevice->GetOwnerWindow() : nullptr;

                if( pWindow )
                {
                    ::basegfx::B2DRange aTmpRange = ::canvas::tools::calcTransformedRectBounds( rBounds,
                                                                mpViewLayer->getTransformation() );
                    const ::basegfx::B2IRange aRangePix(
                        ::basegfx::unotools::b2ISurroundingRangeFromB2DRange( aTmpRange ));

                    if( !aRangePix.isEmpty() )
                    {
                        awt::Rectangle              aAWTRect( aRangePix.getMinX(),
                                                              aRangePix.getMinY(),
                                                                aRangePix.getMaxX() - aRangePix.getMinX(),
                                                                aRangePix.getMaxY() - aRangePix.getMinY() );
                        {
                            mpMediaWindow.disposeAndClear();
                            mpMediaWindow = VclPtr<SystemChildWindow>::Create( pWindow, WB_CLIPCHILDREN );
                            UnoViewSharedPtr xUnoView(std::dynamic_pointer_cast<UnoView>(mpViewLayer));
                            if (xUnoView)
                            {
                                awt::Rectangle aCanvasArea = xUnoView->getUnoView()->getCanvasArea();
                                aAWTRect.X += aCanvasArea.X;
                                aAWTRect.Y += aCanvasArea.Y;
                            }
                            mpMediaWindow->SetPosSizePixel( Point( aAWTRect.X, aAWTRect.Y ),
                                                       Size( aAWTRect.Width, aAWTRect.Height ) );
                        }
                        mpMediaWindow->SetBackground( COL_BLACK );
                        mpMediaWindow->SetParentClipMode( ParentClipMode::NoClip );
                        mpMediaWindow->EnableEraseBackground( false );
                        mpMediaWindow->SetForwardKey( true );
                        mpMediaWindow->SetMouseTransparent( true );
                        mpMediaWindow->Show();

                        if( mxPlayer.is() )
                        {
                            sal_IntPtr nParentWindowHandle(0);
                            const SystemEnvData* pEnvData = mpMediaWindow->GetSystemData();
                            // tdf#139609 gtk doesn't need the handle, and fetching it is undesirable
                            if (!pEnvData || pEnvData->toolkit != SystemEnvData::Toolkit::Gtk)
                                nParentWindowHandle = mpMediaWindow->GetParentWindowHandle();

                            aAWTRect.X = aAWTRect.Y = 0;

                            SdrObject* pObj = SdrObject::getSdrObjectFromXShape(mxShape);
                            auto pMediaObj = dynamic_cast<SdrMediaObj*>(pObj);
                            avmedia::MediaItem* pMediaItem = nullptr;
                            if (pMediaObj)
                            {
                                pMediaItem = pMediaObj->getMediaProperties().Clone();
                            }

                            // TODO In slideshow we cannot play/pause/stop the video
                            if (pMediaItem)
                                pMediaItem->setState(avmedia::MediaState::Play);

                            uno::Sequence< uno::Any >   aArgs{
                                uno::Any(nParentWindowHandle),
                                uno::Any(aAWTRect),
                                uno::Any(reinterpret_cast< sal_IntPtr >( mpMediaWindow.get() )),
                                // Media item contains media properties, e.g. cropping.
                                uno::Any(reinterpret_cast< sal_IntPtr >( pMediaItem ))
                            };

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
                TOOLS_WARN_EXCEPTION( "slideshow", "" );
            }
        }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
