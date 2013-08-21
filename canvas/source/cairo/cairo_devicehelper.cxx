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
#include <vcl/dibtools.hxx>

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

#if defined (UNX) && !defined (MACOSX)
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
        static sal_Int32 nFilePostfixCount(0);

        if( mpRefDevice )
        {
            OUString aFilename("dbg_frontbuffer");
            aFilename += OUString::number(nFilePostfixCount);
            aFilename += ".bmp";

            SvFileStream aStream( aFilename, STREAM_STD_READWRITE );

            const ::Point aEmptyPoint;
            bool bOldMap( mpRefDevice->IsMapModeEnabled() );
            mpRefDevice->EnableMapMode( sal_False );
            const ::Bitmap aTempBitmap(mpRefDevice->GetBitmap(aEmptyPoint, mpRefDevice->GetOutputSizePixel()));
            WriteDIB(aTempBitmap, aStream, false, true);
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
