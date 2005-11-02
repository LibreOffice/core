/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: devicehelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:00:49 $
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

#include <canvas/debug.hxx>
#include <canvas/canvastools.hxx>
#include <canvas/base/linepolypolygonbase.hxx>

#include <vcl/canvastools.hxx>
#include <basegfx/tools/canvastools.hxx>

#include "devicehelper.hxx"
#include "spritecanvas.hxx"
#include "spritecanvashelper.hxx"
#include "canvasbitmap.hxx"


using namespace ::com::sun::star;

namespace vclcanvas
{
    DeviceHelper::DeviceHelper() :
        mpOutputWindow(NULL),
        mpSpriteCanvas(NULL),
        mpBackBuffer()
    {
    }

    void DeviceHelper::init( Window&        rOutputWindow,
                             SpriteCanvas&  rSpriteCanvas )
    {
        mpOutputWindow = &rOutputWindow;
        mpSpriteCanvas = &rSpriteCanvas;

        // setup back buffer
        mpBackBuffer.reset( new BackBuffer( rOutputWindow ) );
        mpBackBuffer->setSize( rOutputWindow.GetOutputSizePixel() );
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
        const uno::Reference< rendering::XGraphicDevice >&              rDevice,
        const uno::Sequence< uno::Sequence< geometry::RealPoint2D > >&  points )
    {
        if( !mpOutputWindow )
            return uno::Reference< rendering::XLinePolyPolygon2D >(); // we're disposed

        return uno::Reference< rendering::XLinePolyPolygon2D >(
            new ::canvas::LinePolyPolygonBase(
                ::basegfx::unotools::polyPolygonFromPoint2DSequenceSequence( points ) ) );
    }

    uno::Reference< rendering::XBezierPolyPolygon2D > DeviceHelper::createCompatibleBezierPolyPolygon(
        const uno::Reference< rendering::XGraphicDevice >&                      rDevice,
        const uno::Sequence< uno::Sequence< geometry::RealBezierSegment2D > >&  points )
    {
        if( !mpOutputWindow )
            return uno::Reference< rendering::XBezierPolyPolygon2D >(); // we're disposed

        return uno::Reference< rendering::XBezierPolyPolygon2D >(
            new ::canvas::LinePolyPolygonBase(
                ::basegfx::unotools::polyPolygonFromBezier2DSequenceSequence( points ) ) );
    }

    uno::Reference< rendering::XBitmap > DeviceHelper::createCompatibleBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  rDevice,
        const geometry::IntegerSize2D&                      size )
    {
        if( !mpSpriteCanvas )
            return uno::Reference< rendering::XBitmap >(); // we're disposed

        return uno::Reference< rendering::XBitmap >(
            new CanvasBitmap( ::vcl::unotools::sizeFromIntegerSize2D(size),
                              false,
                              mpSpriteCanvas ) );
    }

    uno::Reference< rendering::XVolatileBitmap > DeviceHelper::createVolatileBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  rDevice,
        const geometry::IntegerSize2D&                      size )
    {
        return uno::Reference< rendering::XVolatileBitmap >();
    }

    uno::Reference< rendering::XBitmap > DeviceHelper::createCompatibleAlphaBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  rDevice,
        const geometry::IntegerSize2D&                      size )
    {
        if( !mpSpriteCanvas )
            return uno::Reference< rendering::XBitmap >(); // we're disposed

        return uno::Reference< rendering::XBitmap >( new CanvasBitmap( ::vcl::unotools::sizeFromIntegerSize2D(size),
                                                                       true,
                                                                       mpSpriteCanvas ) );
    }

    uno::Reference< rendering::XVolatileBitmap > DeviceHelper::createVolatileAlphaBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  rDevice,
        const geometry::IntegerSize2D&                      size )
    {
        return uno::Reference< rendering::XVolatileBitmap >();
    }

    sal_Bool DeviceHelper::hasFullScreenMode()
    {
        // TODO(F3): offer fullscreen mode the XCanvas way
        return false;
    }

    sal_Bool DeviceHelper::enterFullScreenMode( sal_Bool bEnter )
    {
        // TODO(F3): offer fullscreen mode the XCanvas way
        return false;
    }

    ::sal_Int32 DeviceHelper::createBuffers( ::sal_Int32 nBuffers )
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
        // no difference for VCL canvas
        return showBuffer( bUpdateAll );
    }

    void DeviceHelper::disposing()
    {
        // release all references
        mpOutputWindow = NULL;
        mpSpriteCanvas = NULL;
        mpBackBuffer.reset();
    }

    ::com::sun::star::uno::Any DeviceHelper::getDeviceHandle() const
    {
        if( !mpOutputWindow )
            return ::com::sun::star::uno::Any();

        return ::com::sun::star::uno::makeAny(
            reinterpret_cast< sal_Int64 >(mpOutputWindow) );
    }

    ::com::sun::star::uno::Any DeviceHelper::getSurfaceHandle() const
    {
        if( !mpBackBuffer )
            return ::com::sun::star::uno::Any();

        return ::com::sun::star::uno::makeAny(
            reinterpret_cast< sal_Int64 >(&mpBackBuffer->getOutDev()) );
    }

    void DeviceHelper::notifySizeUpdate( const awt::Rectangle& rBounds )
    {
        if( mpBackBuffer )
            mpBackBuffer->setSize( ::Size(rBounds.Width,
                                          rBounds.Height) );
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

            if( mpBackBuffer )
            {
                String aFilename2( String::CreateFromAscii("dbg_backbuffer") );
                aFilename2 += String::CreateFromInt32(nFilePostfixCount);
                aFilename2 += String::CreateFromAscii(".bmp");

                SvFileStream aStream2( aFilename2, STREAM_STD_READWRITE );

                const ::Point aEmptyPoint;
                mpBackBuffer->getOutDev().EnableMapMode( FALSE );
                aStream2 << mpBackBuffer->getOutDev().GetBitmap(aEmptyPoint,
                                                                mpBackBuffer->getOutDev().GetOutputSizePixel());
            }

            ++nFilePostfixCount;
        }
    }

}
