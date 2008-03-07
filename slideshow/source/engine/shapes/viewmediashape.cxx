/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewmediashape.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 17:03:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

// must be first
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include <math.h>

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <vcl/window.hxx>
#include <vcl/javachild.hxx>
#include <vcl/salbtype.hxx>

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
#include <com/sun/star/lang/XComponent.hdl>

#include "viewmediashape.hxx"
#include "mediashape.hxx"
#include "tools.hxx"

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
            mxComponentContext( rxContext )
        {
            ENSURE_AND_THROW( mxShape.is(), "ViewMediaShape::ViewMediaShape(): Invalid Shape" );
            ENSURE_AND_THROW( mpViewLayer, "ViewMediaShape::ViewMediaShape(): Invalid View" );
            ENSURE_AND_THROW( mpViewLayer->getCanvas(), "ViewMediaShape::ViewMediaShape(): Invalid ViewLayer canvas" );
            ENSURE_AND_THROW( mxComponentContext.is(), "ViewMediaShape::ViewMediaShape(): Invalid component context" );
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
                OSL_ENSURE( false, rtl::OUStringToOString(
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

            mpMediaWindow = ::std::auto_ptr< JavaChildWindow >();

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

        bool ViewMediaShape::render( const ::basegfx::B2DRectangle& rBounds ) const
        {
            ::cppcanvas::CanvasSharedPtr pCanvas = mpViewLayer->getCanvas();;

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

            ::cppcanvas::CanvasSharedPtr pCanvas = mpViewLayer->getCanvas();;

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
                                  ::rtl::OUString::createFromAscii( "Window" )) )
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
                ENSURE_AND_RETURN( mpViewLayer->getCanvas(),
                                   "ViewMediaShape::update(): Invalid layer canvas" );

                uno::Reference< rendering::XCanvas > xCanvas( mpViewLayer->getCanvas()->getUNOCanvas() );

                if( xCanvas.is() )
                {
                    uno::Reference< beans::XPropertySet >   xPropSet;
                    ::rtl::OUString                         aURL;

                    try
                    {
                        xPropSet.set( mxShape, uno::UNO_QUERY );

                        // create Player
                        if( xPropSet.is() &&
                            ( xPropSet->getPropertyValue(
                                  ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaURL" ) ) ) >>=aURL ) )
                        {
                            implInitializeMediaPlayer( aURL );
                        }

                        // create visible object
                        uno::Sequence< uno::Any > aDeviceParams;

                        if( ::canvas::tools::getDeviceInfo( xCanvas, aDeviceParams ).getLength() > 1 )
                        {
                            ::rtl::OUString aImplName;

                            aDeviceParams[ 0 ] >>= aImplName;

                            if( aImplName.endsWithIgnoreAsciiCaseAsciiL(
                                    RTL_CONSTASCII_STRINGPARAM("VCLCanvas") ))
                            {
                                implInitializeVCLBasedPlayerWindow( rBounds, aDeviceParams );
                            }
                            else if( aImplName.endsWithIgnoreAsciiCaseAsciiL(
                                         RTL_CONSTASCII_STRINGPARAM("DXCanvas")) ||
                                     aImplName.endsWithIgnoreAsciiCaseAsciiL(
                                         RTL_CONSTASCII_STRINGPARAM("DX9Canvas")))
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
                        OSL_ENSURE( false,
                                    rtl::OUStringToOString(
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
                                      ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Loop" )));
                    mxPlayer->setPlaybackLoop( bLoop );

                    sal_Bool bMute( false );
                    getPropertyValue( bMute,
                                      rxProps,
                                      ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Mute" )));
                    mxPlayer->setMute( bMute );

                    sal_Int16 nVolumeDB(0);
                    getPropertyValue( nVolumeDB,
                                      rxProps,
                                      ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "VolumeDB" )));
                    mxPlayer->setVolumeDB( nVolumeDB );

                    if( mxPlayerWindow.is() )
                    {
                        media::ZoomLevel eZoom(media::ZoomLevel_FIT_TO_WINDOW);
                        getPropertyValue( eZoom,
                                          rxProps,
                                          ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Zoom" )));
                        mxPlayerWindow->setZoomLevel( eZoom );
                    }
                }
            }
        }

        // ---------------------------------------------------------------------

        void ViewMediaShape::implInitializeMediaPlayer( const ::rtl::OUString& rMediaURL )
        {
            if( !mxPlayer.is() )
            {
                try
                {
                    if( rMediaURL.getLength() )
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
                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                           "No video support for ") ) + rMediaURL,
                        uno::Reference<uno::XInterface>() );
                }
            }
        }

        // ---------------------------------------------------------------------

        bool ViewMediaShape::implInitializeVCLBasedPlayerWindow( const ::basegfx::B2DRectangle&   rBounds,
                                                                 const uno::Sequence< uno::Any >& rVCLDeviceParams)
        {
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
                            uno::Sequence< uno::Any >   aArgs( 2 );
                            awt::Rectangle              aAWTRect( rRangePix.getMinX(),
                                                                  rRangePix.getMinY(),
                                                                    rRangePix.getMaxX() - rRangePix.getMinX(),
                                                                    rRangePix.getMaxY() - rRangePix.getMinY() );

                            mpMediaWindow = ::std::auto_ptr< JavaChildWindow >( new JavaChildWindow( pWindow, WB_CLIPCHILDREN ) );
                            mpMediaWindow->SetBackground( Color( COL_BLACK ) );
                            mpMediaWindow->SetPosSizePixel( Point( aAWTRect.X,
                                                                   aAWTRect.Y ),
                                                            Size( aAWTRect.Width,
                                                                  aAWTRect.Height ));
                            mpMediaWindow->Show();

                            if( mxPlayer.is() )
                            {
                                aArgs[ 0 ] = uno::makeAny(
                                    sal::static_int_cast<sal_IntPtr>(
                                        mpMediaWindow->getParentWindowHandleForJava()) );

                                aAWTRect.X = aAWTRect.Y = 0;
                                aArgs[ 1 ] = uno::makeAny( aAWTRect );

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
                    OSL_ENSURE( false,
                                rtl::OUStringToOString(
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
                                    aArgs[ 0 ] = uno::makeAny(
                                        sal::static_int_cast<sal_Int32>(
                                            aWNDVal) );
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
                    OSL_ENSURE( false,
                                rtl::OUStringToOString(
                                    comphelper::anyToString( cppu::getCaughtException() ),
                                    RTL_TEXTENCODING_UTF8 ).getStr() );
                }
            }

            return mxPlayerWindow.is();
        }
    }
}
