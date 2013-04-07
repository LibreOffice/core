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


// must be first
#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>
#include <canvas/verbosetrace.hxx>

#include <math.h>

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <vcl/window.hxx>
#include <vcl/syschild.hxx>

#include <basegfx/tools/canvastools.hxx>
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
            mpMediaWindow(),
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

            UnoViewSharedPtr pUnoView (::boost::dynamic_pointer_cast<UnoView>(rViewLayer));
            if (pUnoView)
            {
                mbIsSoundEnabled = pUnoView->isSoundEnabled();
            }
        }

        // ---------------------------------------------------------------------

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

        // ---------------------------------------------------------------------

        ViewLayerSharedPtr ViewMediaShape::getViewLayer() const
        {
            return mpViewLayer;
        }

        // ---------------------------------------------------------------------

        bool ViewMediaShape::startMedia()
        {
            if( !mxPlayer.is() )
                implInitialize( maBounds );

            if( mxPlayer.is() && ( mxPlayer->getDuration() > 0.0 ) )
                mxPlayer->start();

            return true;
        }

        // ---------------------------------------------------------------------

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

            mpMediaWindow = ::std::auto_ptr< SystemChildWindow >();

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

        // ---------------------------------------------------------------------

        void ViewMediaShape::pauseMedia()
        {
            if( mxPlayer.is() && ( mxPlayer->getDuration() > 0.0 ) )
                mxPlayer->stop();
        }

        // ---------------------------------------------------------------------

        void ViewMediaShape::setMediaTime(double fTime)
        {
            if( mxPlayer.is() && ( mxPlayer->getDuration() > 0.0 ) )
                mxPlayer->setMediaTime(fTime);
        }

        // ---------------------------------------------------------------------

        bool ViewMediaShape::render( const ::basegfx::B2DRectangle& rBounds ) const
        {
            ::cppcanvas::CanvasSharedPtr pCanvas = mpViewLayer->getCanvas();

            if( !pCanvas )
                return false;

            if( !mpMediaWindow.get() && !mxPlayerWindow.is() )
            {
                // fill the shape background with black
                fillRect( pCanvas,
                          rBounds,
                          0x000000FFU );
            }

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
                                  OUString("Window" )) )
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

        // ---------------------------------------------------------------------

        bool ViewMediaShape::implInitialize( const ::basegfx::B2DRectangle& rBounds )
        {
            if( !mxPlayer.is() && mxShape.is() )
            {
                ENSURE_OR_RETURN_FALSE( mpViewLayer->getCanvas(),
                                   "ViewMediaShape::update(): Invalid layer canvas" );

                uno::Reference< rendering::XCanvas > xCanvas( mpViewLayer->getCanvas()->getUNOCanvas() );

                if( xCanvas.is() )
                {
                    uno::Reference< beans::XPropertySet >   xPropSet;
                    OUString                         aURL;

                    try
                    {
                        xPropSet.set( mxShape, uno::UNO_QUERY );

                        // create Player
                        if (xPropSet.is())
                        {
                            if ((xPropSet->getPropertyValue(
                                  OUString( "PrivateTempFileURL")) >>= aURL)
                                && !aURL.isEmpty())
                            {
                                implInitializeMediaPlayer( aURL );
                            }
                            else if (xPropSet->getPropertyValue(
                                  OUString( "MediaURL")) >>= aURL)
                            {
                                implInitializeMediaPlayer( aURL );
                            }
                        }

                        // create visible object
                        uno::Sequence< uno::Any > aDeviceParams;

                        if( ::canvas::tools::getDeviceInfo( xCanvas, aDeviceParams ).getLength() > 1 )
                        {
                            OUString aImplName;

                            aDeviceParams[ 0 ] >>= aImplName;

                            if( aImplName.endsWithIgnoreAsciiCaseAsciiL(
                                    RTL_CONSTASCII_STRINGPARAM("VCL") ) || aImplName.endsWithIgnoreAsciiCaseAsciiL(
                                    RTL_CONSTASCII_STRINGPARAM("Cairo") ) )
                            {
                                implInitializeVCLBasedPlayerWindow( rBounds, aDeviceParams );
                            }
                            else if( aImplName.endsWithIgnoreAsciiCaseAsciiL(
                                         RTL_CONSTASCII_STRINGPARAM("DX")) ||
                                     aImplName.endsWithIgnoreAsciiCaseAsciiL(
                                         RTL_CONSTASCII_STRINGPARAM("DX9")))
                            {
                                implInitializeDXBasedPlayerWindow( rBounds, aDeviceParams );
                            }
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

        // ---------------------------------------------------------------------

        void ViewMediaShape::implSetMediaProperties( const uno::Reference< beans::XPropertySet >& rxProps )
        {
            if( mxPlayer.is() )
            {
                mxPlayer->setMediaTime( 0.0 );

                if( rxProps.is() )
                {
                    sal_Bool bLoop( false );
                    getPropertyValue( bLoop,
                                      rxProps,
                                      OUString( "Loop" ));
                    mxPlayer->setPlaybackLoop( bLoop );

                    sal_Bool bMute( false );
                    getPropertyValue( bMute,
                                      rxProps,
                                      OUString( "Mute" ));
                    mxPlayer->setMute( bMute || !mbIsSoundEnabled);

                    sal_Int16 nVolumeDB(0);
                    getPropertyValue( nVolumeDB,
                                      rxProps,
                                      OUString( "VolumeDB" ));
                    mxPlayer->setVolumeDB( nVolumeDB );

                    if( mxPlayerWindow.is() )
                    {
                        media::ZoomLevel eZoom(media::ZoomLevel_FIT_TO_WINDOW);
                        getPropertyValue( eZoom,
                                          rxProps,
                                          OUString( "Zoom" ));
                        mxPlayerWindow->setZoomLevel( eZoom );
                    }
                }
            }
        }

        // ---------------------------------------------------------------------

        void ViewMediaShape::implInitializeMediaPlayer( const OUString& rMediaURL )
        {
            if( !mxPlayer.is() )
            {
                try
                {
                    if( !rMediaURL.isEmpty() )
                    {
                        mxPlayer.set( avmedia::MediaWindow::createPlayer( rMediaURL ),
                            uno::UNO_QUERY );
                    }
                }
                catch( uno::RuntimeException& )
                {
                    throw;
                }
                catch( const uno::Exception& )
                {
                    throw lang::NoSupportException(
                        OUString( "No video support for " ) + rMediaURL,
                        uno::Reference<uno::XInterface>() );
                }
            }
        }

        // ---------------------------------------------------------------------

        bool ViewMediaShape::implInitializeVCLBasedPlayerWindow( const ::basegfx::B2DRectangle&   rBounds,
                                                                 const uno::Sequence< uno::Any >& rVCLDeviceParams)
        {
                    OSL_TRACE( "ViewMediaShape::implInitializeVCLBasedPlayerWindow" );
            if( !mpMediaWindow.get() && !rBounds.isEmpty() )
            {
                try
                {
                    sal_Int64 aVal=0;

                    rVCLDeviceParams[ 1 ] >>= aVal;

                    Window* pWindow = reinterpret_cast< Window* >( aVal );

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

                            mpMediaWindow = ::std::auto_ptr< SystemChildWindow >( new
                                                SystemChildWindow( pWindow, WB_CLIPCHILDREN ) );
                            mpMediaWindow->SetBackground( Color( COL_BLACK ) );
                            mpMediaWindow->SetPosSizePixel( Point( aAWTRect.X, aAWTRect.Y ),
                                                           Size( aAWTRect.Width, aAWTRect.Height ) );
                            mpMediaWindow->SetParentClipMode( PARENTCLIPMODE_NOCLIP );
                            mpMediaWindow->EnableEraseBackground( sal_False );
                            mpMediaWindow->EnablePaint( sal_False );
                            mpMediaWindow->SetForwardKey( sal_True );
                            mpMediaWindow->SetMouseTransparent( sal_True );
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

            return mxPlayerWindow.is();
        }

        // ---------------------------------------------------------------------

        bool ViewMediaShape::implInitializeDXBasedPlayerWindow( const ::basegfx::B2DRectangle&   rBounds,
                                                                const uno::Sequence< uno::Any >& rDXDeviceParams )
        {
            if( !mxPlayerWindow.is() )
            {
                try
                {
                    if( rDXDeviceParams.getLength() == 2 )
                    {
                        sal_Int64 aWNDVal=0;

                        rDXDeviceParams[ 1 ] >>= aWNDVal;

                        if( aWNDVal )
                        {
                            ::basegfx::B2DRange aTmpRange;
                            ::canvas::tools::calcTransformedRectBounds( aTmpRange,
                                                                        rBounds,
                                                                        mpViewLayer->getTransformation() );
                            const ::basegfx::B2IRange& rRangePix(
                                ::basegfx::unotools::b2ISurroundingRangeFromB2DRange( aTmpRange ));

                            if( !rRangePix.isEmpty() )
                            {
                                uno::Sequence< uno::Any >   aArgs( 2 );
                                awt::Rectangle              aAWTRect( rRangePix.getMinX() + maWindowOffset.X,
                                                                      rRangePix.getMinY() + maWindowOffset.Y,
                                                                      rRangePix.getMaxX() - rRangePix.getMinX(),
                                                                      rRangePix.getMaxY() - rRangePix.getMinY() );

                                if( mxPlayer.is() )
                                {
                                    aArgs[ 0 ] = uno::makeAny( sal::static_int_cast< sal_Int32 >( aWNDVal) );
                                    aArgs[ 1 ] = uno::makeAny( aAWTRect );

                                    mxPlayerWindow.set( mxPlayer->createPlayerWindow( aArgs ) );
                                }
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

            return mxPlayerWindow.is();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
