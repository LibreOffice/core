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

#include <ctype.h> // don't ask. msdev breaks otherwise...
#include <vcl/window.hxx>
#include <vcl/canvastools.hxx>
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>
#include <canvas/canvastools.hxx>
#include <tools/diagnose_ex.h>

#include <osl/mutex.hxx>
#include <cppuhelper/compbase1.hxx>

#include <com/sun/star/lang/NoSupportException.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <basegfx/tools/canvastools.hxx>
#include "dx_linepolypolygon.hxx"
#include "dx_spritecanvas.hxx"
#include "dx_canvasbitmap.hxx"
#include "dx_devicehelper.hxx"


#undef WB_LEFT
#undef WB_RIGHT
#include "dx_winstuff.hxx"


#include <vcl/sysdata.hxx>

using namespace ::com::sun::star;

namespace dxcanvas
{
    DeviceHelper::DeviceHelper() :
        mpDevice( NULL ),
        mnHDC(0)
    {
    }

    void DeviceHelper::init( HDC                        hdc,
                             rendering::XGraphicDevice& rDevice )
    {
        mnHDC    = hdc;
        mpDevice = &rDevice;
    }

    void DeviceHelper::disposing()
    {
        // release all references
        mnHDC = 0;
        mpDevice = NULL;
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

    sal_Bool DeviceHelper::hasFullScreenMode()
    {
        return false;
    }

    sal_Bool DeviceHelper::enterFullScreenMode( sal_Bool /*bEnter*/ )
    {
        return false;
    }

    uno::Any DeviceHelper::isAccelerated() const
    {
        return ::com::sun::star::uno::makeAny(false);
    }

    uno::Any DeviceHelper::getDeviceHandle() const
    {
        HDC hdc( getHDC() );
        if( hdc )
            return uno::makeAny( reinterpret_cast< sal_Int64 >(hdc) );
        else
            return uno::Any();
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
