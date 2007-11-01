/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dx_devicehelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 17:55:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _DXCANVAS_DEVICEHELPER_HXX
#define _DXCANVAS_DEVICEHELPER_HXX

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XBufferController.hpp>

#include "dx_rendermodule.hxx"
#include "dx_bitmap.hxx"

#include <canvas/rendering/isurfaceproxymanager.hxx>

#include <boost/utility.hpp>


/* Definition of DeviceHelper class */

namespace dxcanvas
{
    class SpriteCanvas;
    class SpriteCanvasHelper;

    class DeviceHelper : private ::boost::noncopyable
    {
    public:
        DeviceHelper();

        void init( Window&                                  rWindow,
                   SpriteCanvas&                            rSpriteCanvas,
                   const ::com::sun::star::awt::Rectangle&  rRect,
                   bool                                     bFullscreen );

        /// Dispose all internal references
        void disposing();

        // XWindowGraphicDevice
        ::com::sun::star::geometry::RealSize2D getPhysicalResolution();
        ::com::sun::star::geometry::RealSize2D getPhysicalSize();
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XLinePolyPolygon2D > createCompatibleLinePolyPolygon(
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >&                               rDevice,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealPoint2D > >& points );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBezierPolyPolygon2D > createCompatibleBezierPolyPolygon(
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >&                                       rDevice,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealBezierSegment2D > >& points );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap > createCompatibleBitmap(
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >&  rDevice,
            const ::com::sun::star::geometry::IntegerSize2D&                                        size );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XVolatileBitmap > createVolatileBitmap(
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >&  rDevice,
            const ::com::sun::star::geometry::IntegerSize2D&                                        size );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap > createCompatibleAlphaBitmap(
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >&  rDevice,
            const ::com::sun::star::geometry::IntegerSize2D&                                        size );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XVolatileBitmap > createVolatileAlphaBitmap(
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >&  rDevice,
            const ::com::sun::star::geometry::IntegerSize2D&                                        size );
        sal_Bool hasFullScreenMode(  );
        sal_Bool enterFullScreenMode( sal_Bool bEnter );

        ::sal_Int32 createBuffers( ::sal_Int32 nBuffers );
        void        destroyBuffers(  );
        ::sal_Bool  showBuffer( ::sal_Bool bUpdateAll );
        ::sal_Bool  switchBuffer( ::sal_Bool bUpdateAll );

        ::com::sun::star::uno::Any getDeviceHandle() const;
        ::com::sun::star::uno::Any getSurfaceHandle() const;

        const IDXRenderModuleSharedPtr& getRenderModule() const { return mpRenderModule; }
        const DXBitmapSharedPtr&        getBackBuffer() const { return mpBackBuffer; }
        const ::canvas::ISurfaceProxyManagerSharedPtr &getSurfaceProxy() const { return mpSurfaceProxyManager; }

        void notifySizeUpdate( const ::com::sun::star::awt::Rectangle& rBounds );

        /** called when DumpScreenContent property is enabled on
            XGraphicDevice, and writes out bitmaps of current screen.
         */
        void dumpScreenContent() const;

    private:
        void        resizeBackBuffer( const ::basegfx::B2ISize& rNewSize );
        HWND getHwnd() const;


        /// Pointer to sprite canvas (owner of this helper), needed to create bitmaps
        SpriteCanvas*                           mpSpriteCanvas;

        DXBitmapSharedPtr                       mpBackBuffer;

        /// Instance passing out HW textures
        ::canvas::ISurfaceProxyManagerSharedPtr mpSurfaceProxyManager;

        /// Our encapsulation interface to DirectX
        IDXRenderModuleSharedPtr                mpRenderModule;
    };
}

#endif /* _DXCANVAS_WINDOWGRAPHICDEVICE_HXX */
