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

#include <cctype>

#include <basegfx/tools/canvastools.hxx>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <osl/mutex.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/canvastools.hxx>
#include <vcl/outdev.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/window.hxx>

#include <canvas/canvastools.hxx>

#include "dx_canvasbitmap.hxx"
#include "dx_devicehelper.hxx"
#include "dx_linepolypolygon.hxx"
#include "dx_spritecanvas.hxx"
#include "dx_winstuff.hxx"

using namespace ::com::sun::star;

namespace dxcanvas
{
    DeviceHelper::DeviceHelper() :
        mpDevice( nullptr ),
        mnHDC(nullptr),
        mpOutDev(nullptr)
    {
    }

    DeviceHelper::~DeviceHelper()
    {
    }

    void DeviceHelper::init( HDC hdc, OutputDevice* pOutDev,
                             rendering::XGraphicDevice& rDevice )
    {
        mnHDC    = hdc;
        mpDevice = &rDevice;
        mpOutDev = pOutDev;
    }

    void DeviceHelper::disposing()
    {
        // release all references
        mnHDC = nullptr;
        mpDevice = nullptr;
        mpOutDev = nullptr;
    }

    geometry::RealSize2D DeviceHelper::getPhysicalResolution()
    {
        if( !mpDevice )
            return ::canvas::tools::createInfiniteSize2D(); // we're disposed

        HDC hDC = getHDC();
        ENSURE_OR_THROW( hDC,
                          "DeviceHelper::getPhysicalResolution(): cannot retrieve HDC from window" );

        const int nHorzRes( GetDeviceCaps( hDC,
                                           LOGPIXELSX ) );
        const int nVertRes( GetDeviceCaps( hDC,
                                           LOGPIXELSY ) );

        return geometry::RealSize2D( nHorzRes*25.4,
                                     nVertRes*25.4 );
    }

    geometry::RealSize2D DeviceHelper::getPhysicalSize()
    {
        if( !mpDevice )
            return ::canvas::tools::createInfiniteSize2D(); // we're disposed

        HDC hDC=getHDC();
        ENSURE_OR_THROW( hDC,
                          "DeviceHelper::getPhysicalSize(): cannot retrieve HDC from window" );

        const int nHorzSize( GetDeviceCaps( hDC,
                                            HORZSIZE ) );
        const int nVertSize( GetDeviceCaps( hDC,
                                            VERTSIZE ) );

        return geometry::RealSize2D( nHorzSize,
                                     nVertSize );
    }

    uno::Reference< rendering::XLinePolyPolygon2D > DeviceHelper::createCompatibleLinePolyPolygon(
        const uno::Reference< rendering::XGraphicDevice >&              /*rDevice*/,
        const uno::Sequence< uno::Sequence< geometry::RealPoint2D > >&  points )
    {
        if( !mpDevice )
            return uno::Reference< rendering::XLinePolyPolygon2D >(); // we're disposed

        return uno::Reference< rendering::XLinePolyPolygon2D >(
            new LinePolyPolygon(
                ::basegfx::unotools::polyPolygonFromPoint2DSequenceSequence( points ) ) );
    }

    uno::Reference< rendering::XBezierPolyPolygon2D > DeviceHelper::createCompatibleBezierPolyPolygon(
        const uno::Reference< rendering::XGraphicDevice >&                      /*rDevice*/,
        const uno::Sequence< uno::Sequence< geometry::RealBezierSegment2D > >&  points )
    {
        if( !mpDevice )
            return uno::Reference< rendering::XBezierPolyPolygon2D >(); // we're disposed

        return uno::Reference< rendering::XBezierPolyPolygon2D >(
            new LinePolyPolygon(
                ::basegfx::unotools::polyPolygonFromBezier2DSequenceSequence( points ) ) );
    }

    uno::Reference< rendering::XBitmap > DeviceHelper::createCompatibleBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  /*rDevice*/,
        const geometry::IntegerSize2D&                      size )
    {
        if( !mpDevice )
            return uno::Reference< rendering::XBitmap >(); // we're disposed

        DXBitmapSharedPtr pBitmap(
            new DXBitmap(
                ::basegfx::unotools::b2ISizeFromIntegerSize2D(size),
                false));

        // create a 24bit RGB system memory surface
        return uno::Reference< rendering::XBitmap >(new CanvasBitmap(pBitmap,mpDevice));
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
        if( !mpDevice )
            return uno::Reference< rendering::XBitmap >(); // we're disposed

        DXBitmapSharedPtr pBitmap(
            new DXBitmap(
                ::basegfx::unotools::b2ISizeFromIntegerSize2D(size),
                true));

        // create a 32bit ARGB system memory surface
        return uno::Reference< rendering::XBitmap >(new CanvasBitmap(pBitmap,mpDevice));
    }

    uno::Reference< rendering::XVolatileBitmap > DeviceHelper::createVolatileAlphaBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  /*rDevice*/,
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
        return uno::Any( reinterpret_cast< sal_Int64 >(mpOutDev.get()) );
    }

    uno::Any DeviceHelper::getSurfaceHandle() const
    {
        // TODO(F1): expose DirectDraw object
        //return mpBackBuffer->getBitmap().get();
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
