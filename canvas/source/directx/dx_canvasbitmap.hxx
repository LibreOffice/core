/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dx_canvasbitmap.hxx,v $
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

#ifndef _DXCANVAS_CANVASBITMAP_HXX
#define _DXCANVAS_CANVASBITMAP_HXX

#include <cppuhelper/compbase3.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>

#include <basegfx/vector/b2isize.hxx>

#include <boost/shared_ptr.hpp>

#include <canvas/base/integerbitmapbase.hxx>

#include "dx_canvashelper.hxx"
#include "dx_spritecanvas.hxx"
#include "dx_impltools.hxx"


/* Definition of CanvasBitmap class */

namespace dxcanvas
{
    typedef ::cppu::WeakComponentImplHelper3< ::com::sun::star::rendering::XBitmapCanvas,
                                              ::com::sun::star::rendering::XIntegerBitmap,
                                               ::com::sun::star::lang::XServiceInfo >   CanvasBitmapBase_Base;
    typedef ::canvas::IntegerBitmapBase< ::canvas::BaseMutexHelper< CanvasBitmapBase_Base >,
                                         CanvasHelper,
                                         ::osl::MutexGuard,
                                         ::cppu::OWeakObject >                          CanvasBitmap_Base;

    class CanvasBitmap : public CanvasBitmap_Base
    {
    public:
        /** Create a canvas bitmap for the given surface

            @param rSurface
            Surface to create API object for.

            @param rDevice
            Reference device, with which bitmap should be compatible
         */
        CanvasBitmap( const DXBitmapSharedPtr&  rSurface,
                      const DeviceRef&          rDevice );

        /// Dispose all internal references
        virtual void SAL_CALL disposing();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        // attention: Only intended client of this method is
        // bitmapFromXBitmap()!
        DXBitmapSharedPtr getSurface() { return mpSurface; };

    private:
        /** MUST hold here, too, since CanvasHelper only contains a
            raw pointer (without refcounting)
        */
        DeviceRef           mpDevice;
        DXBitmapSharedPtr   mpSurface;
    };
}

#endif /* _DXCANVAS_CANVASBITMAP_HXX */
