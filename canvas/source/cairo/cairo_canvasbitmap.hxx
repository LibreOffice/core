/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cairo_canvasbitmap.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-02-28 10:33:21 $
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

#ifndef _CAIROCANVAS_CANVASBITMAP_HXX
#define _CAIROCANVAS_CANVASBITMAP_HXX

#include <cppuhelper/compbase3.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>

#include <basegfx/vector/b2isize.hxx>

#include <boost/shared_ptr.hpp>

#include <canvas/base/integerbitmapbase.hxx>

#include "cairo_cairo.hxx"
#include "cairo_canvashelper.hxx"
#include "cairo_repainttarget.hxx"
#include "cairo_spritecanvas.hxx"


/* Definition of CanvasBitmap class */

namespace cairocanvas
{
    typedef ::cppu::WeakComponentImplHelper3< ::com::sun::star::rendering::XBitmapCanvas,
                                                                           ::com::sun::star::rendering::XIntegerBitmap,
                                                                                                        ::com::sun::star::lang::XServiceInfo >      CanvasBitmapBase_Base;
    typedef ::canvas::IntegerBitmapBase< ::canvas::BaseMutexHelper< CanvasBitmapBase_Base >,
        CanvasHelper,
        ::osl::MutexGuard,
               ::cppu::OWeakObject >                            CanvasBitmap_Base;

    class CanvasBitmap : public CanvasBitmap_Base,
                         public RepaintTarget
    {
    public:
        /** Create a canvas bitmap for the given surface

            @param rSize
            Size of the bitmap

            @param rDevice
            Reference device, with which bitmap should be compatible
        */
        CanvasBitmap( const ::basegfx::B2ISize& rSize,
                      const DeviceRef&          rDevice,
                      bool                      bHasAlpha );

        /// Dispose all internal references
        virtual void SAL_CALL disposing();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        ::cairo::Surface* getSurface();

        // RepaintTarget
        virtual bool repaint( ::cairo::Surface* pSurface,
                              const ::com::sun::star::rendering::ViewState& viewState,
                              const ::com::sun::star::rendering::RenderState&   renderState );

    private:
        /** MUST hold here, too, since CanvasHelper only contains a
            raw pointer (without refcounting)
        */
        DeviceRef               mpDevice;
        ::cairo::Surface*       mpBufferSurface;
        ::cairo::Cairo*         mpBufferCairo;

        bool mbHasAlpha;
    };
}

#endif /* _CAIROCANVAS_CANVASBITMAP_HXX */
