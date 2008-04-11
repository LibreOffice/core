/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: null_devicehelper.cxx,v $
 * $Revision: 1.5 $
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
#include <canvas/base/linepolypolygonbase.hxx>

#include <osl/mutex.hxx>
#include <cppuhelper/compbase1.hxx>

#include <com/sun/star/lang/NoSupportException.hpp>

#include <basegfx/tools/canvastools.hxx>

#include "null_spritecanvas.hxx"
#include "null_canvasbitmap.hxx"
#include "null_devicehelper.hxx"


using namespace ::com::sun::star;

namespace nullcanvas
{
    DeviceHelper::DeviceHelper() :
        mpSpriteCanvas( NULL ),
        maSize(),
        mbFullScreen(false)
    {
    }

    void DeviceHelper::init( SpriteCanvas&              rSpriteCanvas,
                             const ::basegfx::B2ISize&  rSize,
                             bool                       bFullscreen )
    {
        mpSpriteCanvas = &rSpriteCanvas;
        maSize = rSize;
        mbFullScreen = bFullscreen;
    }

    void DeviceHelper::disposing()
    {
        // release all references
        mpSpriteCanvas = NULL;
    }

    geometry::RealSize2D DeviceHelper::getPhysicalResolution()
    {
        return geometry::RealSize2D( 75, 75 );
    }

    geometry::RealSize2D DeviceHelper::getPhysicalSize()
    {
        return geometry::RealSize2D( 210, 280 );
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
        // no difference for VCL canvas
        return showBuffer( bUpdateAll );
    }

    uno::Any DeviceHelper::getDeviceHandle() const
    {
        return uno::Any();
    }

    uno::Any DeviceHelper::getSurfaceHandle() const
    {
        return uno::Any();
    }

    void DeviceHelper::notifySizeUpdate( const awt::Rectangle& /*rBounds*/ )
    {
        // TODO
    }

    void DeviceHelper::dumpScreenContent() const
    {
        OSL_TRACE( "%s\n",
                   BOOST_CURRENT_FUNCTION );
    }
}
