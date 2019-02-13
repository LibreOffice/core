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

#ifndef INCLUDED_CANVAS_SOURCE_DIRECTX_DX_CANVASBITMAP_HXX
#define INCLUDED_CANVAS_SOURCE_DIRECTX_DX_CANVASBITMAP_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <basegfx/vector/b2isize.hxx>
#include <cppuhelper/compbase.hxx>
#include <canvas/base/basemutexhelper.hxx>
#include <canvas/base/bitmapcanvasbase.hxx>
#include <canvas/base/integerbitmapbase.hxx>

#include "dx_bitmapprovider.hxx"
#include "dx_bitmapcanvashelper.hxx"
#include "dx_devicehelper.hxx"
#include "dx_impltools.hxx"
#include "dx_ibitmap.hxx"


/* Definition of CanvasBitmap class */

namespace dxcanvas
{
    typedef ::cppu::WeakComponentImplHelper< css::rendering::XBitmapCanvas,
                                             css::rendering::XIntegerBitmap,
                                             css::lang::XServiceInfo,
                                             css::beans::XFastPropertySet >      CanvasBitmapBase_Base;
    typedef ::canvas::IntegerBitmapBase<
        canvas::BitmapCanvasBase2<
            ::canvas::BaseMutexHelper< CanvasBitmapBase_Base >,
            BitmapCanvasHelper,
            ::osl::MutexGuard,
            ::cppu::OWeakObject> > CanvasBitmap_Base;

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
        virtual void disposeThis() override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

        // BitmapProvider
        virtual IBitmapSharedPtr getBitmap() const override { return mpBitmap; }

        virtual css::uno::Any SAL_CALL getFastPropertyValue(sal_Int32 nHandle) override;
        virtual void SAL_CALL setFastPropertyValue(sal_Int32, const css::uno::Any&) override {}

    private:
        /** MUST hold here, too, since CanvasHelper only contains a
            raw pointer (without refcounting)
        */
        DeviceRef        mpDevice;
        IBitmapSharedPtr mpBitmap;
    };
}

#endif // INCLUDED_CANVAS_SOURCE_DIRECTX_DX_CANVASBITMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
