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

#include <sal/config.h>

#include <basegfx/tools/canvastools.hxx>
#include <basegfx/tools/unopolypolygon.hxx>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <osl/mutex.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/stream.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/dibtools.hxx>

#include <canvas/canvastools.hxx>

#include "cairo_canvasbitmap.hxx"
#include "cairo_devicehelper.hxx"
#include "cairo_spritecanvas.hxx"

using namespace ::cairo;
using namespace ::com::sun::star;

namespace cairocanvas
{
    DeviceHelper::DeviceHelper() :
        mpSurfaceProvider( nullptr ),
        mpRefDevice( nullptr ),
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

        OutputDevice* pOutDev = getOutputDevice();
        mpSurface = pOutDev->CreateSurface(pOutDev->GetOutOffXPixel(),
                                           pOutDev->GetOutOffYPixel(),
                                           pOutDev->GetOutputWidthPixel(),
                                           pOutDev->GetOutputHeightPixel());
    }

    void DeviceHelper::disposing()
    {
        // release all references
        mpSurface.reset();
        mpRefDevice = nullptr;
        mpSurfaceProvider = nullptr;
    }

    void DeviceHelper::setSize( const ::basegfx::B2ISize& rSize )
    {
        SAL_INFO(
            "canvas.cairo",
            "device size " << rSize.getX() << " x " << rSize.getY());

        if( !mpRefDevice )
            return; // disposed

        OutputDevice* pOutDev = getOutputDevice();

        // X11 only
        bool bReuseSurface = mpSurface &&
                             mpSurface->Resize(rSize.getX() + pOutDev->GetOutOffXPixel(),
                                               rSize.getY() + pOutDev->GetOutOffYPixel());

        if (!bReuseSurface)
        {
            mpSurface = pOutDev->CreateSurface(
                pOutDev->GetOutOffXPixel(),
                pOutDev->GetOutOffYPixel(),
                rSize.getX(), rSize.getY() );
        }
    }

    geometry::RealSize2D DeviceHelper::getPhysicalResolution()
    {
        // Map a one-by-one millimeter box to pixel
        const MapMode aOldMapMode( mpRefDevice->GetMapMode() );
        mpRefDevice->SetMapMode( MapMode(MapUnit::MapMM) );
        const Size aPixelSize( mpRefDevice->LogicToPixel(Size(1,1)) );
        mpRefDevice->SetMapMode( aOldMapMode );

        return vcl::unotools::size2DFromSize( aPixelSize );
    }

    geometry::RealSize2D DeviceHelper::getPhysicalSize()
    {
        if( !mpRefDevice )
            return ::canvas::tools::createInfiniteSize2D(); // we're disposed

        // Map the pixel dimensions of the output window to millimeter
        const MapMode aOldMapMode( mpRefDevice->GetMapMode() );
        mpRefDevice->SetMapMode( MapMode(MapUnit::MapMM) );
        const Size aLogSize( mpRefDevice->PixelToLogic(mpRefDevice->GetOutputSizePixel()) );
        mpRefDevice->SetMapMode( aOldMapMode );

        return vcl::unotools::size2DFromSize( aLogSize );
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

    uno::Any DeviceHelper::isAccelerated() const
    {
        return css::uno::Any(false);
    }

    uno::Any DeviceHelper::getDeviceHandle() const
    {
        return uno::Any( reinterpret_cast< sal_Int64 >(mpRefDevice.get()) );
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
            OUString aFilename = "dbg_frontbuffer" + OUString::number(nFilePostfixCount) + ".bmp";

            SvFileStream aStream( aFilename, StreamMode::STD_READWRITE );

            const ::Point aEmptyPoint;
            bool bOldMap( mpRefDevice->IsMapModeEnabled() );
            mpRefDevice->EnableMapMode( false );
            const ::Bitmap aTempBitmap(mpRefDevice->GetBitmap(aEmptyPoint, mpRefDevice->GetOutputSizePixel()));
            WriteDIB(aTempBitmap, aStream, false, true);
            mpRefDevice->EnableMapMode( bOldMap );

            ++nFilePostfixCount;
        }
    }

    SurfaceSharedPtr DeviceHelper::createSurface( const ::basegfx::B2ISize& rSize, int aContent )
    {
        if( mpSurface )
            return mpSurface->getSimilar( aContent, rSize.getX(), rSize.getY() );

        return SurfaceSharedPtr();
    }

    SurfaceSharedPtr DeviceHelper::createSurface( BitmapSystemData& rData, const Size& rSize )
    {
        if (mpRefDevice)
            return mpRefDevice->CreateBitmapSurface(rData, rSize);

        return SurfaceSharedPtr();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
