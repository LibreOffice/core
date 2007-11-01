/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: canvasbitmap.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 14:40:48 $
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

#ifndef _VCLCANVAS_CANVASBITMAP_HXX
#define _VCLCANVAS_CANVASBITMAP_HXX

#include <cppuhelper/compbase3.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>

#include <vcl/virdev.hxx>
#include <vcl/bitmapex.hxx>

#include <canvas/vclwrapper.hxx>

#include <canvas/base/integerbitmapbase.hxx>
#include <canvasbitmaphelper.hxx>

#include "impltools.hxx"
#include "repainttarget.hxx"
#include "spritecanvas.hxx"


/* Definition of CanvasBitmap class */

namespace vclcanvas
{
    typedef ::cppu::WeakComponentImplHelper3< ::com::sun::star::rendering::XBitmapCanvas,
                                               ::com::sun::star::rendering::XIntegerBitmap,
                                                ::com::sun::star::lang::XServiceInfo >      CanvasBitmapBase_Base;
    typedef ::canvas::IntegerBitmapBase< ::canvas::BaseMutexHelper< CanvasBitmapBase_Base >,
                                         CanvasBitmapHelper,
                                         tools::LocalGuard,
                                         ::cppu::OWeakObject >                          CanvasBitmap_Base;

    class CanvasBitmap : public CanvasBitmap_Base,
                         public RepaintTarget
    {
    public:
        /** Must be called with locked Solar mutex

            @param rSize
            Size in pixel of the bitmap to generate

            @param bAlphaBitmap
            When true, bitmap will have an alpha channel

            @param rDevice
            Reference device, with which bitmap should be compatible
         */
        CanvasBitmap( const ::Size&     rSize,
                      bool              bAlphaBitmap,
                      const DeviceRef&  rDevice );

        /// Must be called with locked Solar mutex
        CanvasBitmap( const BitmapEx&   rBitmap,
                      const DeviceRef&  rDevice );

        // overridden because of mpDevice
        virtual void SAL_CALL disposing();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        // RepaintTarget interface
        virtual bool repaint( const GraphicObjectSharedPtr&                   rGrf,
                              const ::com::sun::star::rendering::ViewState&   viewState,
                              const ::com::sun::star::rendering::RenderState& renderState,
                              const ::Point&                                  rPt,
                              const ::Size&                                   rSz,
                              const GraphicAttr&                              rAttr ) const;

        /// Not threadsafe! Returned object is shared!
        BitmapEx getBitmap() const;

    private:
        /** MUST hold here, too, since CanvasHelper only contains a
            raw pointer (without refcounting)
        */
        DeviceRef   mpDevice;
    };
}

#endif /* _VCLCANVAS_CANVASBITMAP_HXX */
