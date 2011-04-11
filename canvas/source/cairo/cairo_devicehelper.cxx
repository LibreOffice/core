/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>
#include <canvas/canvastools.hxx>

#include <osl/mutex.hxx>
#include <cppuhelper/compbase1.hxx>

#include <com/sun/star/lang/NoSupportException.hpp>

#include <toolkit/helper/vclunohelper.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/tools/unopolypolygon.hxx>

#include <vcl/canvastools.hxx>

#include <tools/stream.hxx>

#include "cairo_spritecanvas.hxx"
#include "cairo_canvasbitmap.hxx"
#include "cairo_devicehelper.hxx"

using namespace ::cairo;
using namespace ::com::sun::star;

namespace cairocanvas
{
    DeviceHelper::DeviceHelper() :
        mpSurfaceProvider( NULL ),
        mpRefDevice( NULL ),
        mpSurface()
    {
    }

    void DeviceHelper::implInit( SurfaceProvider& rSurfaceProvider,
                                 OutputDevice&    rRefDevice )
    {
        mpSurfaceProvider = &rSurfaceProvider;
        mpRefDevice = &rRefDevice;

        // no own surface, this is handled by derived classes
    }

    void DeviceHelper::init( SurfaceProvider& rSurfaceProvider,
                             OutputDevice&    rRefDevice )
    {
        implInit(rSurfaceProvider, rRefDevice);

        OutputDevice* pOutDev=getOutputDevice();
        mpSurface = cairo::createSurface( *pOutDev,
                                          pOutDev->GetOutOffXPixel(),
                                          pOutDev->GetOutOffYPixel(),
                                          pOutDev->GetOutputWidthPixel(),
                                          pOutDev->GetOutputHeightPixel() );
    }

    void DeviceHelper::disposing()
    {
        // release all references
        mpSurface.reset();
        mpRefDevice = NULL;
        mpSurfaceProvider = NULL;
    }

    void DeviceHelper::setSize( const ::basegfx::B2ISize& rSize )
    {
        OSL_TRACE("DeviceHelper::setSize(): device size %d x %d", rSize.getX(), rSize.getY() );

        if( !mpRefDevice )
            return; // disposed

        OutputDevice* pOutDev=getOutputDevice();

#if defined (UNX) && !defined (QUARTZ)
        // X11 only
        if( mpSurface )
            mpSurface->Resize( rSize.getX() + pOutDev->GetOutOffXPixel(),
                               rSize.getY() + pOutDev->GetOutOffYPixel() );
        else
#endif
            mpSurface = cairo::createSurface(
                *pOutDev,
                pOutDev->GetOutOffXPixel(),
                pOutDev->GetOutOffYPixel(),
                rSize.getX(), rSize.getY() );
    }

    geometry::RealSize2D DeviceHelper::getPhysicalResolution()
    {
        // Map a one-by-one millimeter box to pixel
        const MapMode aOldMapMode( mpRefDevice->GetMapMode() );
        mpRefDevice->SetMapMode( MapMode(MAP_MM) );
        const Size aPixelSize( mpRefDevice->LogicToPixel(Size(1,1)) );
        mpRefDevice->SetMapMode( aOldMapMode );

        return ::vcl::unotools::size2DFromSize( aPixelSize );
    }

    geometry::RealSize2D DeviceHelper::getPhysicalSize()
    {
        if( !mpRefDevice )
            return ::canvas::tools::createInfiniteSize2D(); // we're disposed

        // Map the pixel dimensions of the output window to millimeter
        const MapMode aOldMapMode( mpRefDevice->GetMapMode() );
        mpRefDevice->SetMapMode( MapMode(MAP_MM) );
        const Size aLogSize( mpRefDevice->PixelToLogic(mpRefDevice->GetOutputSizePixel()) );
        mpRefDevice->SetMapMode( aOldMapMode );

        return ::vcl::unotools::size2DFromSize( aLogSize );
    }

    uno::Reference< rendering::XLinePolyPolygon2D > DeviceHelper::createCompatibleLinePolyPolygon(
        const uno::Reference< rendering::XGraphicDevice >&              ,
        const uno::Sequence< uno::Sequence< geometry::RealPoint2D > >&  points )
    {
        // disposed?
        if( !mpSurfaceProvider )
            return uno::Reference< rendering::XLinePolyPolygon2D >(); // we're disposed

        return uno::Reference< rendering::XLinePolyPolygon2D >(
            new ::basegfx::unotools::UnoPolyPolygon(
                ::basegfx::unotools::polyPolygonFromPoint2DSequenceSequence( points ) ) );
    }

    uno::Reference< rendering::XBezierPolyPolygon2D > DeviceHelper::createCompatibleBezierPolyPolygon(
        const uno::Reference< rendering::XGraphicDevice >&                      ,
        const uno::Sequence< uno::Sequence< geometry::RealBezierSegment2D > >&  points )
    {
        // disposed?
        if( !mpSurfaceProvider )
            return uno::Reference< rendering::XBezierPolyPolygon2D >(); // we're disposed

        return uno::Reference< rendering::XBezierPolyPolygon2D >(
            new ::basegfx::unotools::UnoPolyPolygon(
                ::basegfx::unotools::polyPolygonFromBezier2DSequenceSequence( points ) ) );
    }

    uno::Reference< rendering::XBitmap > DeviceHelper::createCompatibleBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  rDevice,
        const geometry::IntegerSize2D&                      size )
    {
        // disposed?
        if( !mpSurfaceProvider )
            return uno::Reference< rendering::XBitmap >(); // we're disposed

        return uno::Reference< rendering::XBitmap >(
            new CanvasBitmap(
                ::basegfx::unotools::b2ISizeFromIntegerSize2D( size ),
                SurfaceProviderRef(mpSurfaceProvider),
                rDevice.get(),
                false ));
    }

    uno::Reference< rendering::XVolatileBitmap > DeviceHelper::createVolatileBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  ,
        const geometry::IntegerSize2D&                      /*size*/ )
    {
        return uno::Reference< rendering::XVolatileBitmap >();
    }

    uno::Reference< rendering::XBitmap > DeviceHelper::createCompatibleAlphaBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  rDevice,
        const geometry::IntegerSize2D&                      size )
    {
        // disposed?
        if( !mpSurfaceProvider )
            return uno::Reference< rendering::XBitmap >(); // we're disposed

        return uno::Reference< rendering::XBitmap >(
            new CanvasBitmap(
                ::basegfx::unotools::b2ISizeFromIntegerSize2D( size ),
                SurfaceProviderRef(mpSurfaceProvider),
                rDevice.get(),
                true ));
    }

    uno::Reference< rendering::XVolatileBitmap > DeviceHelper::createVolatileAlphaBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  ,
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

    uno::Any DeviceHelper::isAccelerated() const
    {
        return ::com::sun::star::uno::makeAny(false);
    }

    uno::Any DeviceHelper::getDeviceHandle() const
    {
        return uno::makeAny( reinterpret_cast< sal_Int64 >(mpRefDevice) );
    }

    uno::Any DeviceHelper::getSurfaceHandle() const
    {
        return uno::Any();
    }

    namespace
    {
        struct DeviceColorSpace: public rtl::StaticWithInit<uno::Reference<rendering::XColorSpace>,
                                                            DeviceColorSpace>
        {
            uno::Reference<rendering::XColorSpace> operator()()
            {
                return vcl::unotools::createStandardColorSpace();
            }
        };
    }

    uno::Reference<rendering::XColorSpace> DeviceHelper::getColorSpace() const
    {
        // always the same
        return DeviceColorSpace::get();
    }

    void DeviceHelper::dumpScreenContent() const
    {
        static sal_uInt32 nFilePostfixCount(0);

        if( mpRefDevice )
        {
            String aFilename( String::CreateFromAscii("dbg_frontbuffer") );
            aFilename += String::CreateFromInt32(nFilePostfixCount);
            aFilename += String::CreateFromAscii(".bmp");

            SvFileStream aStream( aFilename, STREAM_STD_READWRITE );

            const ::Point aEmptyPoint;
            bool bOldMap( mpRefDevice->IsMapModeEnabled() );
            mpRefDevice->EnableMapMode( sal_False );
            aStream << mpRefDevice->GetBitmap(aEmptyPoint,
                                              mpRefDevice->GetOutputSizePixel());
            mpRefDevice->EnableMapMode( bOldMap );

            ++nFilePostfixCount;
        }
    }

    SurfaceSharedPtr DeviceHelper::getSurface()
    {
        return mpSurface;
    }

    SurfaceSharedPtr DeviceHelper::createSurface( const ::basegfx::B2ISize& rSize, Content aContent )
    {
        if( mpSurface )
            return mpSurface->getSimilar( aContent, rSize.getX(), rSize.getY() );

        return SurfaceSharedPtr();
    }

    SurfaceSharedPtr DeviceHelper::createSurface( BitmapSystemData& rData, const Size& rSize )
    {
        if( mpRefDevice )
            return createBitmapSurface( *mpRefDevice, rData, rSize );

        return SurfaceSharedPtr();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
