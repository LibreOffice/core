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

#pragma once

#include <cppuhelper/compbase.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>

#include <vcl/bitmapex.hxx>

#include <base/bitmapcanvasbase.hxx>
#include <base/basemutexhelper.hxx>
#include <base/integerbitmapbase.hxx>
#include "canvasbitmaphelper.hxx"

#include "impltools.hxx"
#include "repainttarget.hxx"


/* Definition of CanvasBitmap class */

namespace vclcanvas
{
    typedef ::cppu::WeakComponentImplHelper< css::rendering::XBitmapCanvas,
                                             css::rendering::XIntegerBitmap,
                                             css::lang::XServiceInfo,
                                             css::beans::XFastPropertySet >    CanvasBitmapBase_Base;
    typedef ::canvas::IntegerBitmapBase<
        canvas::BitmapCanvasBase2<
            ::canvas::BaseMutexHelper< CanvasBitmapBase_Base >,
            CanvasBitmapHelper,
            tools::LocalGuard,
            ::cppu::OWeakObject> > CanvasBitmap_Base;

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
        CanvasBitmap( const ::Size&                                rSize,
                      bool                                         bAlphaBitmap,
                      css::rendering::XGraphicDevice&              rDevice,
                      const OutDevProviderSharedPtr&               rOutDevProvider );

        /// Must be called with locked Solar mutex
        CanvasBitmap( const BitmapEx&                              rBitmap,
                      css::rendering::XGraphicDevice&              rDevice,
                      const OutDevProviderSharedPtr&               rOutDevProvider );

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

        // RepaintTarget interface
        virtual bool repaint( const GraphicObjectSharedPtr&                   rGrf,
                              const css::rendering::ViewState&   viewState,
                              const css::rendering::RenderState& renderState,
                              const ::Point&                                  rPt,
                              const ::Size&                                   rSz,
                              const GraphicAttr&                              rAttr ) const override;

        /// Not threadsafe! Returned object is shared!
        BitmapEx getBitmap() const;

        // XFastPropertySet
        // used to retrieve BitmapEx pointer or X Pixmap handles for this bitmap
        // handle values have these meanings:
        // 0 ... get pointer to BitmapEx
        // 1 ... get X pixmap handle to rgb content
        // 2 ... get X pixmap handle to alpha mask
        // returned any contains either BitmapEx pointer or array of three Any value
        //     1st a bool value: true - free the pixmap after used by XFreePixmap, false do nothing, the pixmap is used internally in the canvas
        //     2nd the pixmap handle
        //     3rd the pixmap depth
        virtual css::uno::Any SAL_CALL getFastPropertyValue(sal_Int32 nHandle) override;
        virtual void SAL_CALL setFastPropertyValue(sal_Int32, const css::uno::Any&) override {}

    private:
        /** MUST hold here, too, since CanvasHelper only contains a
            raw pointer (without refcounting)
        */
        css::uno::Reference<css::rendering::XGraphicDevice> mxDevice;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
