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
#include <tools/diagnose_ex.h>
#include <canvas/canvastools.hxx>

#include <rtl/instance.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/canvastools.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/tools/unopolypolygon.hxx>

#include "devicehelper.hxx"
#include "spritecanvas.hxx"
#include "spritecanvashelper.hxx"
#include "canvasbitmap.hxx"


using namespace ::com::sun::star;

namespace vclcanvas
{
    DeviceHelper::DeviceHelper() :
        mpOutDev()
    {}

    void DeviceHelper::init( const OutDevProviderSharedPtr& rOutDev )
    {
        mpOutDev = rOutDev;
    }

    geometry::RealSize2D DeviceHelper::getPhysicalResolution()
    {
        if( !mpOutDev )
            return ::canvas::tools::createInfiniteSize2D(); // we're disposed

        // Map a one-by-one millimeter box to pixel
        OutputDevice& rOutDev = mpOutDev->getOutDev();
        const MapMode aOldMapMode( rOutDev.GetMapMode() );
        rOutDev.SetMapMode( MapMode(MAP_MM) );
        const Size aPixelSize( rOutDev.LogicToPixel(Size(1,1)) );
        rOutDev.SetMapMode( aOldMapMode );

        return ::vcl::unotools::size2DFromSize( aPixelSize );
    }

    geometry::RealSize2D DeviceHelper::getPhysicalSize()
    {
        if( !mpOutDev )
            return ::canvas::tools::createInfiniteSize2D(); // we're disposed

        // Map the pixel dimensions of the output window to millimeter
        OutputDevice& rOutDev = mpOutDev->getOutDev();
        const MapMode aOldMapMode( rOutDev.GetMapMode() );
        rOutDev.SetMapMode( MapMode(MAP_MM) );
        const Size aLogSize( rOutDev.PixelToLogic(rOutDev.GetOutputSizePixel()) );
        rOutDev.SetMapMode( aOldMapMode );

        return ::vcl::unotools::size2DFromSize( aLogSize );
    }

    uno::Reference< rendering::XLinePolyPolygon2D > DeviceHelper::createCompatibleLinePolyPolygon(
        const uno::Reference< rendering::XGraphicDevice >&              ,
        const uno::Sequence< uno::Sequence< geometry::RealPoint2D > >&  points )
    {
        uno::Reference< rendering::XLinePolyPolygon2D > xPoly;
        if( !mpOutDev )
            return xPoly; // we're disposed

        xPoly.set( new ::basegfx::unotools::UnoPolyPolygon(
                       ::basegfx::unotools::polyPolygonFromPoint2DSequenceSequence( points ) ) );
        // vcl only handles even_odd polygons
        xPoly->setFillRule(rendering::FillRule_EVEN_ODD);

        return xPoly;
    }

    uno::Reference< rendering::XBezierPolyPolygon2D > DeviceHelper::createCompatibleBezierPolyPolygon(
        const uno::Reference< rendering::XGraphicDevice >&                      ,
        const uno::Sequence< uno::Sequence< geometry::RealBezierSegment2D > >&  points )
    {
        uno::Reference< rendering::XBezierPolyPolygon2D > xPoly;
        if( !mpOutDev )
            return xPoly; // we're disposed

        xPoly.set( new ::basegfx::unotools::UnoPolyPolygon(
                       ::basegfx::unotools::polyPolygonFromBezier2DSequenceSequence( points ) ) );
        // vcl only handles even_odd polygons
        xPoly->setFillRule(rendering::FillRule_EVEN_ODD);

        return xPoly;
    }

    uno::Reference< rendering::XBitmap > DeviceHelper::createCompatibleBitmap(
        const uno::Reference< rendering::XGraphicDevice >& rDevice,
        const geometry::IntegerSize2D&                     size )
    {
        if( !mpOutDev )
            return uno::Reference< rendering::XBitmap >(); // we're disposed

        return uno::Reference< rendering::XBitmap >(
            new CanvasBitmap( ::vcl::unotools::sizeFromIntegerSize2D(size),
                              false,
                              *rDevice.get(),
                              mpOutDev ) );
    }

    uno::Reference< rendering::XVolatileBitmap > DeviceHelper::createVolatileBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  ,
        const geometry::IntegerSize2D&                       )
    {
        return uno::Reference< rendering::XVolatileBitmap >();
    }

    uno::Reference< rendering::XBitmap > DeviceHelper::createCompatibleAlphaBitmap(
        const uno::Reference< rendering::XGraphicDevice >& rDevice,
        const geometry::IntegerSize2D&                     size )
    {
        if( !mpOutDev )
            return uno::Reference< rendering::XBitmap >(); // we're disposed

        return uno::Reference< rendering::XBitmap >(
            new CanvasBitmap( ::vcl::unotools::sizeFromIntegerSize2D(size),
                              true,
                              *rDevice.get(),
                              mpOutDev ) );
    }

    uno::Reference< rendering::XVolatileBitmap > DeviceHelper::createVolatileAlphaBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  ,
        const geometry::IntegerSize2D&                       )
    {
        return uno::Reference< rendering::XVolatileBitmap >();
    }

    sal_Bool DeviceHelper::hasFullScreenMode()
    {
        return false;
    }

    sal_Bool DeviceHelper::enterFullScreenMode( sal_Bool bEnter )
    {
        (void)bEnter;
        return false;
    }

    void DeviceHelper::disposing()
    {
        // release all references
        mpOutDev.reset();
    }

    uno::Any DeviceHelper::isAccelerated() const
    {
        return ::com::sun::star::uno::makeAny(false);
    }

    uno::Any DeviceHelper::getDeviceHandle() const
    {
        if( !mpOutDev )
            return uno::Any();

        return uno::makeAny(
            reinterpret_cast< sal_Int64 >(&mpOutDev->getOutDev()) );
    }

    uno::Any DeviceHelper::getSurfaceHandle() const
    {
        return getDeviceHandle();
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

        if( mpOutDev )
        {
            String aFilename( String::CreateFromAscii("dbg_frontbuffer") );
            aFilename += String::CreateFromInt32(nFilePostfixCount);
            aFilename += String::CreateFromAscii(".bmp");

            SvFileStream aStream( aFilename, STREAM_STD_READWRITE );

            const ::Point aEmptyPoint;
            OutputDevice& rOutDev = mpOutDev->getOutDev();
            bool bOldMap( rOutDev.IsMapModeEnabled() );
            rOutDev.EnableMapMode( sal_False );
            aStream << rOutDev.GetBitmap(aEmptyPoint,
                                         rOutDev.GetOutputSizePixel());
            rOutDev.EnableMapMode( bOldMap );

            ++nFilePostfixCount;
        }
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
