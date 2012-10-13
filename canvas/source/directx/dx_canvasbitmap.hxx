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

#ifndef _DXCANVAS_CANVASBITMAP_HXX
#define _DXCANVAS_CANVASBITMAP_HXX

#include <cppuhelper/compbase4.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>

#include <basegfx/vector/b2isize.hxx>

#include <boost/shared_ptr.hpp>

#include <cppuhelper/compbase3.hxx>
#include <comphelper/uno3.hxx>
#include <canvas/base/disambiguationhelper.hxx>
#include <canvas/base/integerbitmapbase.hxx>

#include "dx_bitmapprovider.hxx"
#include "dx_bitmapcanvashelper.hxx"
#include "dx_devicehelper.hxx"
#include "dx_impltools.hxx"
#include "dx_ibitmap.hxx"


/* Definition of CanvasBitmap class */

namespace dxcanvas
{
    typedef ::cppu::WeakComponentImplHelper4< ::com::sun::star::rendering::XBitmapCanvas,
                                              ::com::sun::star::rendering::XIntegerBitmap,
                                               ::com::sun::star::lang::XServiceInfo,
                                               ::com::sun::star::beans::XFastPropertySet >      CanvasBitmapBase_Base;
    typedef ::canvas::IntegerBitmapBase< ::canvas::DisambiguationHelper< CanvasBitmapBase_Base >,
                                         BitmapCanvasHelper,
                                         ::osl::MutexGuard,
                                         ::cppu::OWeakObject >                          CanvasBitmap_Base;

    class CanvasBitmap : public CanvasBitmap_Base, public BitmapProvider
    {
    public:
        /** Create a canvas bitmap for the given surface

            @param rSurface
            Surface to create API object for.

            @param rDevice
            Reference device, with which bitmap should be compatible
         */
        CanvasBitmap( const IBitmapSharedPtr& rSurface,
                      const DeviceRef&        rDevice );

        /// Dispose all internal references
        virtual void disposeThis();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        // BitmapProvider
        virtual IBitmapSharedPtr getBitmap() const { return mpBitmap; }

        virtual ::com::sun::star::uno::Any SAL_CALL getFastPropertyValue(sal_Int32 nHandle)  throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setFastPropertyValue(sal_Int32, const ::com::sun::star::uno::Any&)  throw (::com::sun::star::uno::RuntimeException) {}

    private:
        /** MUST hold here, too, since CanvasHelper only contains a
            raw pointer (without refcounting)
        */
        DeviceRef        mpDevice;
        IBitmapSharedPtr mpBitmap;
    };
}

#endif /* _DXCANVAS_CANVASBITMAP_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
