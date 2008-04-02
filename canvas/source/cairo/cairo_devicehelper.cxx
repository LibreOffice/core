/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cairo_devicehelper.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-02 09:42:02 $
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
#include "precompiled_canvas.hxx"

#include "cairo_cairo.hxx"

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>
#include <canvas/canvastools.hxx>
#include <canvas/base/linepolypolygonbase.hxx>

#include <osl/mutex.hxx>
#include <cppuhelper/compbase1.hxx>

#include <com/sun/star/lang/NoSupportException.hpp>

#include <toolkit/helper/vclunohelper.hxx>
#include <basegfx/tools/canvastools.hxx>

#include <vcl/syschild.hxx>
#include <vcl/canvastools.hxx>

#include <tools/stream.hxx>

namespace cairo
{
#  include <cairo.h>
}  // namespace cairo

#include <vcl/sysdata.hxx>

#include "cairo_spritecanvas.hxx"
#include "cairo_canvasbitmap.hxx"
#include "cairo_devicehelper.hxx"


using namespace ::cairo;
using namespace ::com::sun::star;

namespace cairocanvas
{

    const SystemEnvData* GetSysData(Window *pOutputWindow)
    {
        const SystemEnvData* pSysData = NULL;
        // check whether we're a SysChild: have to fetch system data
        // directly from SystemChildWindow, because the GetSystemData
        // method is unfortunately not virtual
        const SystemChildWindow* pSysChild = dynamic_cast< const SystemChildWindow* >( pOutputWindow );
        if( pSysChild )
            pSysData = pSysChild->GetSystemData();
        else
            pSysData = pOutputWindow->GetSystemData();
        return pSysData;
    }


    DeviceHelper::DeviceHelper() :
        mpSpriteCanvas( NULL ),
        maSize(),
        mbFullScreen( false ),
        mpOutputWindow( NULL ),
        mpSysData( NULL ),
        mpWindowSurface( NULL ),
        mpBufferSurface( NULL ),
        mpBufferCairo( NULL )
    {
    }

    void DeviceHelper::init( Window&                    rOutputWindow,
                             SpriteCanvas&              rSpriteCanvas,
                             const ::basegfx::B2ISize&  rSize,
                             bool                       bFullscreen )
    {
        rSpriteCanvas.setWindow(
            uno::Reference<awt::XWindow2>(
                VCLUnoHelper::GetInterface(&rOutputWindow),
                uno::UNO_QUERY_THROW) );

        mpOutputWindow = &rOutputWindow;
        mpSpriteCanvas = &rSpriteCanvas;
        mbFullScreen = bFullscreen;

        mpSysData = GetSysData(mpOutputWindow);

        setSize( rSize );
    }

    void DeviceHelper::disposing()
    {
        // release all references
        mpSpriteCanvas = NULL;

        if( mpWindowSurface ) {
            mpWindowSurface->Unref();
            mpWindowSurface = NULL;
        }

        if( mpBufferCairo ) {
            cairo_destroy( mpBufferCairo );
            mpBufferCairo = NULL;
        }

        if( mpBufferSurface ) {
            mpBufferSurface->Unref();
            mpBufferSurface = NULL;
        }
    }

    geometry::RealSize2D DeviceHelper::getPhysicalResolution()
    {
        if( !mpOutputWindow )
            return ::canvas::tools::createInfiniteSize2D(); // we're disposed

        // Map a one-by-one millimeter box to pixel
        const MapMode aOldMapMode( mpOutputWindow->GetMapMode() );
        mpOutputWindow->SetMapMode( MapMode(MAP_MM) );
        const Size aPixelSize( mpOutputWindow->LogicToPixel(Size(1,1)) );
        mpOutputWindow->SetMapMode( aOldMapMode );

        return ::vcl::unotools::size2DFromSize( aPixelSize );
    }

    geometry::RealSize2D DeviceHelper::getPhysicalSize()
    {
        if( !mpOutputWindow )
            return ::canvas::tools::createInfiniteSize2D(); // we're disposed

        // Map the pixel dimensions of the output window to millimeter
        const MapMode aOldMapMode( mpOutputWindow->GetMapMode() );
        mpOutputWindow->SetMapMode( MapMode(MAP_MM) );
        const Size aLogSize( mpOutputWindow->PixelToLogic(mpOutputWindow->GetOutputSizePixel()) );
        mpOutputWindow->SetMapMode( aOldMapMode );

        return ::vcl::unotools::size2DFromSize( aLogSize );
    }

    uno::Reference< rendering::XLinePolyPolygon2D > DeviceHelper::createCompatibleLinePolyPolygon(
        const uno::Reference< rendering::XGraphicDevice >&              /*rDevice*/,
        const uno::Sequence< uno::Sequence< geometry::RealPoint2D > >&  points )
    {
        // disposed?
        if( !mpSpriteCanvas )
            return uno::Reference< rendering::XLinePolyPolygon2D >(); // we're disposed

        return uno::Reference< rendering::XLinePolyPolygon2D >(
            new ::canvas::LinePolyPolygonBase(
                ::basegfx::unotools::polyPolygonFromPoint2DSequenceSequence( points ) ) );
    }

    uno::Reference< rendering::XBezierPolyPolygon2D > DeviceHelper::createCompatibleBezierPolyPolygon(
        const uno::Reference< rendering::XGraphicDevice >&                      /*rDevice*/,
        const uno::Sequence< uno::Sequence< geometry::RealBezierSegment2D > >&  points )
    {
        // disposed?
        if( !mpSpriteCanvas )
            return uno::Reference< rendering::XBezierPolyPolygon2D >(); // we're disposed

        return uno::Reference< rendering::XBezierPolyPolygon2D >(
            new ::canvas::LinePolyPolygonBase(
                ::basegfx::unotools::polyPolygonFromBezier2DSequenceSequence( points ) ) );
    }

    uno::Reference< rendering::XBitmap > DeviceHelper::createCompatibleBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  /*rDevice*/,
        const geometry::IntegerSize2D&                      size )
    {
        // disposed?
        if( !mpSpriteCanvas )
            return uno::Reference< rendering::XBitmap >(); // we're disposed

        return uno::Reference< rendering::XBitmap >(
            new CanvasBitmap(
                ::basegfx::unotools::b2ISizeFromIntegerSize2D( size ),
                mpSpriteCanvas,
                false ));
    }

    uno::Reference< rendering::XVolatileBitmap > DeviceHelper::createVolatileBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  /*rDevice*/,
        const geometry::IntegerSize2D&                      /*size*/ )
    {
        return uno::Reference< rendering::XVolatileBitmap >();
    }

    uno::Reference< rendering::XBitmap > DeviceHelper::createCompatibleAlphaBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  /*rDevice*/,
        const geometry::IntegerSize2D&                      size )
    {
        // disposed?
        if( !mpSpriteCanvas )
            return uno::Reference< rendering::XBitmap >(); // we're disposed

        return uno::Reference< rendering::XBitmap >(
            new CanvasBitmap(
                ::basegfx::unotools::b2ISizeFromIntegerSize2D( size ),
                mpSpriteCanvas,
                true ));
    }

    uno::Reference< rendering::XVolatileBitmap > DeviceHelper::createVolatileAlphaBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  /*rDevice*/,
        const geometry::IntegerSize2D&                      /*size*/ )
    {
        return uno::Reference< rendering::XVolatileBitmap >();
    }

    sal_Bool DeviceHelper::hasFullScreenMode()
    {
        // TODO(F3): offer fullscreen mode the XCanvas way
        return false;
    }

    sal_Bool DeviceHelper::enterFullScreenMode( sal_Bool /*bEnter*/ )
    {
        // TODO(F3): offer fullscreen mode the XCanvas way
        return false;
    }

    ::sal_Int32 DeviceHelper::createBuffers( ::sal_Int32 /*nBuffers*/ )
    {
        // TODO(F3): implement XBufferStrategy interface. For now, we
        // _always_ will have exactly one backbuffer
        return 1;
    }

    void DeviceHelper::destroyBuffers()
    {
        // TODO(F3): implement XBufferStrategy interface. For now, we
        // _always_ will have exactly one backbuffer
    }

    ::sal_Bool DeviceHelper::showBuffer( ::sal_Bool bUpdateAll )
    {
        // forward to sprite canvas helper
        if( !mpSpriteCanvas )
            return false;

        return mpSpriteCanvas->updateScreen( bUpdateAll );
    }

    ::sal_Bool DeviceHelper::switchBuffer( ::sal_Bool bUpdateAll )
    {
        // no difference for Cairo canvas
        return showBuffer( bUpdateAll );
    }

    uno::Any DeviceHelper::getDeviceHandle() const
    {
        return uno::makeAny( reinterpret_cast< sal_Int64 >(mpOutputWindow) );
    }

    uno::Any DeviceHelper::getSurfaceHandle() const
    {
        return uno::Any();
    }

    void DeviceHelper::dumpScreenContent() const
    {
        static sal_uInt32 nFilePostfixCount(0);

        if( mpOutputWindow )
        {
            String aFilename( String::CreateFromAscii("dbg_frontbuffer") );
            aFilename += String::CreateFromInt32(nFilePostfixCount);
            aFilename += String::CreateFromAscii(".bmp");

            SvFileStream aStream( aFilename, STREAM_STD_READWRITE );

            const ::Point aEmptyPoint;
            bool bOldMap( mpOutputWindow->IsMapModeEnabled() );
            mpOutputWindow->EnableMapMode( FALSE );
            aStream << mpOutputWindow->GetBitmap(aEmptyPoint,
                                                 mpOutputWindow->GetOutputSizePixel());
            mpOutputWindow->EnableMapMode( bOldMap );

            ++nFilePostfixCount;
        }
    }

    void DeviceHelper::setSize( const ::basegfx::B2ISize& rSize )
    {
        OSL_TRACE("set device size %d x %d", rSize.getX(), rSize.getY() );

        if( mpWindowSurface )
        {
#if defined (UNX) && !defined (QUARTZ)
            // X11 only
            mpWindowSurface->Resize( rSize.getX() + mpOutputWindow->GetOutOffXPixel(), rSize.getY() + mpOutputWindow->GetOutOffYPixel() );
#endif
        } else {
            mpWindowSurface = new Surface( mpSysData,
                                           mpOutputWindow->GetOutOffXPixel(), mpOutputWindow->GetOutOffYPixel(),
                                           rSize.getX(), rSize.getY() );
        }

        if( mpBufferSurface && maSize != rSize )
        {
                mpBufferSurface->Unref();
                mpBufferSurface = NULL;
        }
        if( !mpBufferSurface )
            mpBufferSurface = mpWindowSurface->getSimilar( CAIRO_CONTENT_COLOR, rSize.getX(), rSize.getY() );

        if( mpBufferCairo && maSize != rSize )
        {
            cairo_destroy( mpBufferCairo );
            mpBufferCairo = NULL;
        }
        if( !mpBufferCairo )
            mpBufferCairo = mpBufferSurface->getCairo();

        if( maSize != rSize )
            maSize = rSize;

        mpSpriteCanvas->setSizePixel( maSize );
    }

    const ::basegfx::B2ISize& DeviceHelper::getSizePixel()
    {
        return maSize;
    }

    void DeviceHelper::notifySizeUpdate( const awt::Rectangle& rBounds )
    {
        setSize( ::basegfx::B2ISize(rBounds.Width, rBounds.Height) );
    }

    Surface* DeviceHelper::getBufferSurface()
    {
        return mpBufferSurface;
    }

    Surface* DeviceHelper::getWindowSurface()
    {
        return mpWindowSurface;
    }

    Surface* DeviceHelper::getSurface( const ::basegfx::B2ISize& rSize, Content aContent )
    {
        if( mpBufferSurface )
            return mpBufferSurface->getSimilar( aContent, rSize.getX(), rSize.getY() );

        return NULL;
    }

    Surface* DeviceHelper::getSurface( Content aContent )
    {
        return getSurface( maSize, aContent );
    }

    Surface* DeviceHelper::getSurface( BitmapSystemData& rData, const Size& rSize )
    {
#ifdef CAIRO_HAS_WIN32_SURFACE
        if (rData.pDIB != NULL) {
            OSL_ENSURE(false, "DeviceHelper::getSurface(): cannot provide Surface!");
            // Using cairo will not work anyway, as most (?) DIBs that come here
            // will be upside-down and different order of colour channels
            // compared to what cairo expects.
           return NULL;
        }
#endif
        OSL_TRACE( "requested size: %d x %d available size: %d x %d", rSize.Width (), rSize.Height (), rData.mnWidth, rData.mnHeight );
        if ( rData.mnWidth == rSize.Width() && rData.mnHeight == rSize.Height() )
            return new Surface ( mpSysData, &rData, rSize.Width(), rSize.Height() );
        else {
            return NULL;
        }
    }

  /** DeviceHelper::flush  Flush the platform native window
   *
   * Flushes the window by using the internally stored mpSysData.
   *
   **/
    void DeviceHelper::flush()
    {
#ifdef UNX
        // Only used by Xlib and the current Mac OS X Quartz implementation
        mpWindowSurface->flush(mpSysData);
#endif
    }

} // namespace cairocanvas
