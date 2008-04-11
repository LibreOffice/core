/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: devicehelper.cxx,v $
 * $Revision: 1.6 $
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
#include "precompiled_canvas.hxx"

#include <canvas/debug.hxx>
#include <canvas/canvastools.hxx>
#include <canvas/base/linepolypolygonbase.hxx>

#include <toolkit/helper/vclunohelper.hxx>
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
        mpSpriteCanvas(NULL),
        mpBackBuffer()
    {
    }

    void DeviceHelper::init( Window&        rOutputWindow,
                             SpriteCanvas&  rSpriteCanvas )
    {
        rSpriteCanvas.setWindow(
            uno::Reference<awt::XWindow2>(
                VCLUnoHelper::GetInterface(&rOutputWindow),
                uno::UNO_QUERY_THROW) );

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
        const uno::Reference< rendering::XGraphicDevice >&              ,
        const uno::Sequence< uno::Sequence< geometry::RealPoint2D > >&  points )
    {
        if( !mpOutputWindow )
            return uno::Reference< rendering::XLinePolyPolygon2D >(); // we're disposed

        return uno::Reference< rendering::XLinePolyPolygon2D >(
            new ::canvas::LinePolyPolygonBase(
                ::basegfx::unotools::polyPolygonFromPoint2DSequenceSequence( points ) ) );
    }

    uno::Reference< rendering::XBezierPolyPolygon2D > DeviceHelper::createCompatibleBezierPolyPolygon(
        const uno::Reference< rendering::XGraphicDevice >&                      ,
        const uno::Sequence< uno::Sequence< geometry::RealBezierSegment2D > >&  points )
    {
        if( !mpOutputWindow )
            return uno::Reference< rendering::XBezierPolyPolygon2D >(); // we're disposed

        return uno::Reference< rendering::XBezierPolyPolygon2D >(
            new ::canvas::LinePolyPolygonBase(
                ::basegfx::unotools::polyPolygonFromBezier2DSequenceSequence( points ) ) );
    }

    uno::Reference< rendering::XBitmap > DeviceHelper::createCompatibleBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  ,
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
        const uno::Reference< rendering::XGraphicDevice >&  ,
        const geometry::IntegerSize2D&                       )
    {
        return uno::Reference< rendering::XVolatileBitmap >();
    }

    uno::Reference< rendering::XBitmap > DeviceHelper::createCompatibleAlphaBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  ,
        const geometry::IntegerSize2D&                      size )
    {
        if( !mpSpriteCanvas )
            return uno::Reference< rendering::XBitmap >(); // we're disposed

        return uno::Reference< rendering::XBitmap >( new CanvasBitmap( ::vcl::unotools::sizeFromIntegerSize2D(size),
                                                                       true,
                                                                       mpSpriteCanvas ) );
    }

    uno::Reference< rendering::XVolatileBitmap > DeviceHelper::createVolatileAlphaBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  ,
        const geometry::IntegerSize2D&                       )
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
        (void)bEnter;

        // TODO(F3): offer fullscreen mode the XCanvas way
        return false;
    }

    ::sal_Int32 DeviceHelper::createBuffers( ::sal_Int32 nBuffers )
    {
        (void)nBuffers;

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
        mpSpriteCanvas = NULL;
        mpBackBuffer.reset();
    }

    uno::Any DeviceHelper::getDeviceHandle() const
    {
        if( !mpOutputWindow )
            return uno::Any();

        return uno::makeAny(
            reinterpret_cast< sal_Int64 >(mpOutputWindow) );
    }

    uno::Any DeviceHelper::getSurfaceHandle() const
    {
        if( !mpBackBuffer )
            return uno::Any();

        return uno::makeAny(
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

                mpBackBuffer->getOutDev().EnableMapMode( FALSE );
                aStream2 << mpBackBuffer->getOutDev().GetBitmap(aEmptyPoint,
                                                                mpBackBuffer->getOutDev().GetOutputSizePixel());
            }

            ++nFilePostfixCount;
        }
    }

}
