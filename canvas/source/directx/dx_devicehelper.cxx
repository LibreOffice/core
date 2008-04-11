/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dx_devicehelper.cxx,v $
 * $Revision: 1.3 $
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

#include <ctype.h> // don't ask. msdev breaks otherwise...
#include <vcl/window.hxx>
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>
#include <canvas/canvastools.hxx>

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
        mpSpriteCanvas( NULL ),
        mpSurfaceProxyManager(),
        mpRenderModule(),
        mpBackBuffer()
    {
    }

    void DeviceHelper::init( Window&               rWindow,
                             SpriteCanvas&         rSpriteCanvas,
                             const awt::Rectangle& rRect,
                             bool                  /*bFullscreen*/ )
    {
        // #i60490# ensure backbuffer has sensible minimal size
        const sal_Int32 w( ::std::max(sal_Int32(1),sal_Int32(rRect.Width)));
        const sal_Int32 h( ::std::max(sal_Int32(1),sal_Int32(rRect.Height)));

        rSpriteCanvas.setWindow(
            uno::Reference<awt::XWindow2>(
                VCLUnoHelper::GetInterface(&rWindow),
                uno::UNO_QUERY_THROW) );

        const SystemEnvData *pData = rWindow.GetSystemData();
        const HWND hWnd = reinterpret_cast<HWND>(pData->hWnd);

        ENSURE_AND_THROW( IsWindow( hWnd ),
                          "DeviceHelper::init No valid HWND given." );

        mpSpriteCanvas = &rSpriteCanvas;

        try
        {
            // setup directx rendermodule
            mpRenderModule = createRenderModule( rWindow );
        }
        catch (...) {

            throw lang::NoSupportException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                     "Could not create DirectX device!") ),
                static_cast< ::cppu::OWeakObject* >(&rSpriteCanvas) );
        }

        // create the surfaceproxy manager
        mpSurfaceProxyManager = ::canvas::createSurfaceProxyManager( mpRenderModule );

        // #i60490# ensure backbuffer has sensible minimal size
        mpBackBuffer.reset(new DXBitmap(
                               ::basegfx::B2ISize(w,h),
                               mpSurfaceProxyManager,mpRenderModule,false));
    }

    void DeviceHelper::disposing()
    {
        // release all references
        mpBackBuffer.reset();
        mpSurfaceProxyManager.reset();
        mpRenderModule.reset();
        mpSpriteCanvas = NULL;
    }

    geometry::RealSize2D DeviceHelper::getPhysicalResolution()
    {
        HWND hwnd( getHwnd() );
        if( !hwnd )
            return ::canvas::tools::createInfiniteSize2D(); // we're disposed

        HDC hDC = GetDC( hwnd );
        ENSURE_AND_THROW( hDC,
                          "DeviceHelper::getPhysicalResolution(): cannot retrieve HDC from window" );

        const int nHorzRes( GetDeviceCaps( hDC,
                                           LOGPIXELSX ) );
        const int nVertRes( GetDeviceCaps( hDC,
                                           LOGPIXELSY ) );

        ReleaseDC( hwnd, hDC );

        return geometry::RealSize2D( nHorzRes*25.4,
                                     nVertRes*25.4 );
    }

    geometry::RealSize2D DeviceHelper::getPhysicalSize()
    {
        HWND hwnd( getHwnd() );
        if( !hwnd )
            return ::canvas::tools::createInfiniteSize2D(); // we're disposed

        HDC hDC=GetDC( hwnd );
        ENSURE_AND_THROW( hDC,
                          "DeviceHelper::getPhysicalSize(): cannot retrieve HDC from window" );

        const int nHorzSize( GetDeviceCaps( hDC,
                                            HORZSIZE ) );
        const int nVertSize( GetDeviceCaps( hDC,
                                            VERTSIZE ) );

        ReleaseDC( hwnd, hDC );

        return geometry::RealSize2D( nHorzSize,
                                     nVertSize );
    }

    uno::Reference< rendering::XLinePolyPolygon2D > DeviceHelper::createCompatibleLinePolyPolygon(
        const uno::Reference< rendering::XGraphicDevice >&              /*rDevice*/,
        const uno::Sequence< uno::Sequence< geometry::RealPoint2D > >&  points )
    {
        HWND hwnd( getHwnd() );
        if( !hwnd )
            return uno::Reference< rendering::XLinePolyPolygon2D >(); // we're disposed

        return uno::Reference< rendering::XLinePolyPolygon2D >(
            new LinePolyPolygon(
                ::basegfx::unotools::polyPolygonFromPoint2DSequenceSequence( points ) ) );
    }

    uno::Reference< rendering::XBezierPolyPolygon2D > DeviceHelper::createCompatibleBezierPolyPolygon(
        const uno::Reference< rendering::XGraphicDevice >&                      /*rDevice*/,
        const uno::Sequence< uno::Sequence< geometry::RealBezierSegment2D > >&  points )
    {
        HWND hwnd( getHwnd() );
        if( !hwnd )
            return uno::Reference< rendering::XBezierPolyPolygon2D >(); // we're disposed

        return uno::Reference< rendering::XBezierPolyPolygon2D >(
            new LinePolyPolygon(
                ::basegfx::unotools::polyPolygonFromBezier2DSequenceSequence( points ) ) );
    }

    uno::Reference< rendering::XBitmap > DeviceHelper::createCompatibleBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  /*rDevice*/,
        const geometry::IntegerSize2D&                      size )
    {
        HWND hwnd( getHwnd() );
        if( !hwnd || !mpSpriteCanvas )
            return uno::Reference< rendering::XBitmap >(); // we're disposed

        DXBitmapSharedPtr pBitmap(
            new DXBitmap(
                ::basegfx::unotools::b2ISizeFromIntegerSize2D(size),
                mpSurfaceProxyManager,
                mpRenderModule,
                false));

        // create a 24bit RGB system memory surface
        return uno::Reference< rendering::XBitmap >(new CanvasBitmap(pBitmap,mpSpriteCanvas));
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
        HWND hwnd( getHwnd() );
        if( !hwnd || !mpSpriteCanvas )
            return uno::Reference< rendering::XBitmap >(); // we're disposed

        DXBitmapSharedPtr pBitmap(
            new DXBitmap(
                ::basegfx::unotools::b2ISizeFromIntegerSize2D(size),
                mpSurfaceProxyManager,
                mpRenderModule,
                true));

        // create a 32bit ARGB system memory surface
        return uno::Reference< rendering::XBitmap >(new CanvasBitmap(pBitmap,mpSpriteCanvas));
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
        HWND hwnd( getHwnd() );
        if( hwnd )
            return uno::makeAny( reinterpret_cast< sal_Int64 >(hwnd) );
        else
            return uno::Any();
    }

    uno::Any DeviceHelper::getSurfaceHandle() const
    {
        if( !mpRenderModule )
            return uno::Any();

        // TODO(F1): expose DirectDraw object
        //return mpBackBuffer->getBitmap().get();
        return uno::Any();
    }

    void DeviceHelper::notifySizeUpdate( const awt::Rectangle& rBounds )
    {
        // #i60490# ensure backbuffer has sensible minimal size
        const sal_Int32 x(rBounds.X);
        const sal_Int32 y(rBounds.Y);
        const sal_Int32 w(::std::max(sal_Int32(1),sal_Int32(rBounds.Width)));
        const sal_Int32 h(::std::max(sal_Int32(1),sal_Int32(rBounds.Height)));

        if( mpRenderModule )
            mpRenderModule->resize(::basegfx::B2IRange(x,y,x+w,y+h));

        resizeBackBuffer(::basegfx::B2ISize(w,h));
    }

    void DeviceHelper::resizeBackBuffer( const ::basegfx::B2ISize& rNewSize )
    {
        // disposed?
        if(!(mpBackBuffer))
            return;

        mpBackBuffer->resize(rNewSize);
        mpBackBuffer->clear();
    }

    HWND DeviceHelper::getHwnd() const
    {
        if( mpRenderModule )
            return mpRenderModule->getHWND();
        else
            return 0;
    }

    void DeviceHelper::dumpScreenContent() const
    {
        if( mpRenderModule )
            mpRenderModule->screenShot();
    }
}
