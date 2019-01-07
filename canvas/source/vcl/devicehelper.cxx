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

#include <basegfx/utils/canvastools.hxx>
#include <basegfx/utils/unopolypolygon.hxx>
#include <canvas/canvastools.hxx>
#include <rtl/instance.hxx>
#include <tools/diagnose_ex.h>
#include <tools/stream.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/dibtools.hxx>

#include "canvasbitmap.hxx"
#include "devicehelper.hxx"
#include "spritecanvas.hxx"
#include "spritecanvashelper.hxx"

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
        rOutDev.SetMapMode( MapMode(MapUnit::MapMM) );
        const Size aPixelSize( rOutDev.LogicToPixel(Size(1,1)) );
        rOutDev.SetMapMode( aOldMapMode );

        return vcl::unotools::size2DFromSize( aPixelSize );
    }

    geometry::RealSize2D DeviceHelper::getPhysicalSize()
    {
        if( !mpOutDev )
            return ::canvas::tools::createInfiniteSize2D(); // we're disposed

        // Map the pixel dimensions of the output window to millimeter
        OutputDevice& rOutDev = mpOutDev->getOutDev();
        const MapMode aOldMapMode( rOutDev.GetMapMode() );
        rOutDev.SetMapMode( MapMode(MapUnit::MapMM) );
        const Size aLogSize( rOutDev.PixelToLogic(rOutDev.GetOutputSizePixel()) );
        rOutDev.SetMapMode( aOldMapMode );

        return vcl::unotools::size2DFromSize( aLogSize );
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
            new CanvasBitmap( vcl::unotools::sizeFromIntegerSize2D(size),
                              false,
                              *rDevice,
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
            new CanvasBitmap( vcl::unotools::sizeFromIntegerSize2D(size),
                              true,
                              *rDevice,
                              mpOutDev ) );
    }

    uno::Reference< rendering::XVolatileBitmap > DeviceHelper::createVolatileAlphaBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  ,
        const geometry::IntegerSize2D&                       )
    {
        return uno::Reference< rendering::XVolatileBitmap >();
    }

    void DeviceHelper::disposing()
    {
        // release all references
        mpOutDev.reset();
    }

    uno::Any DeviceHelper::isAccelerated() const
    {
        return css::uno::Any(false);
    }

    uno::Any DeviceHelper::getDeviceHandle() const
    {
        if( !mpOutDev )
            return uno::Any();

        return uno::Any(
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
                uno::Reference< rendering::XColorSpace > xColorSpace( canvas::tools::getStdColorSpace(), uno::UNO_QUERY );
                assert( xColorSpace.is() );
                return xColorSpace;
            }
        };
    }

    uno::Reference<rendering::XColorSpace> const & DeviceHelper::getColorSpace() const
    {
        // always the same
        return DeviceColorSpace::get();
    }

    void DeviceHelper::dumpScreenContent() const
    {
        static sal_Int32 nFilePostfixCount(0);

        if( mpOutDev )
        {
            OUString aFilename = "dbg_frontbuffer" + OUString::number(nFilePostfixCount) + ".bmp";

            SvFileStream aStream( aFilename, StreamMode::STD_READWRITE );

            const ::Point aEmptyPoint;
            OutputDevice& rOutDev = mpOutDev->getOutDev();
            bool bOldMap( rOutDev.IsMapModeEnabled() );
            rOutDev.EnableMapMode( false );
            WriteDIB(rOutDev.GetBitmapEx(aEmptyPoint, rOutDev.GetOutputSizePixel()), aStream, false);
            rOutDev.EnableMapMode( bOldMap );

            ++nFilePostfixCount;
        }
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
