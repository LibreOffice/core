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

#ifndef INCLUDED_CANVAS_BASE_BITMAPCANVASBASE_HXX
#define INCLUDED_CANVAS_BASE_BITMAPCANVASBASE_HXX

#include <canvas/base/canvasbase.hxx>
#include <com/sun/star/geometry/IntegerSize2D.hpp>

namespace com::sun::star::rendering { class XBitmapCanvas; }

namespace canvas
{
    /** Helper template to handle XBitmapCanvas method forwarding to
        BitmapCanvasHelper

        Use this helper to handle the XBitmapCanvas part of your
        implementation.

        @tpl Base
        Base class to use, most probably one of the
        WeakComponentImplHelperN templates with the appropriate
        interfaces. At least XBitmapCanvas should be among them (why
        else would you use this template, then?). Base class must have
        an Base( const Mutex& ) constructor (like the
        WeakComponentImplHelperN templates have).

        @tpl CanvasHelper
        Canvas helper implementation for the backend in question

        @tpl Mutex
        Lock strategy to use. Defaults to using the
        BaseMutex-provided lock.  Every time one of the methods is
        entered, an object of type Mutex is created with m_aMutex as
        the sole parameter, and destroyed again when the method scope
        is left.

        @tpl UnambiguousBase
        Optional unambiguous base class for XInterface of Base. It's
        sometimes necessary to specify this parameter, e.g. if Base
        derives from multiple UNO interface (were each provides its
        own version of XInterface, making the conversion ambiguous)

        @see CanvasBase for further contractual requirements towards
        the CanvasHelper type, and some examples.
     */
    template< class Base,
              class CanvasHelper,
              class Mutex=::osl::MutexGuard,
              class UnambiguousBase=css::uno::XInterface > class BitmapCanvasBase :
        public CanvasBase< Base, CanvasHelper, Mutex, UnambiguousBase >
    {
    public:
        typedef CanvasBase< Base, CanvasHelper, Mutex, UnambiguousBase >    BaseType;

        // XBitmap
        virtual css::geometry::IntegerSize2D SAL_CALL getSize(  ) override
        {
            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            return BaseType::maCanvasHelper.getSize();
        }

        virtual sal_Bool SAL_CALL hasAlpha(  ) override
        {
            return true;
        }

        virtual css::uno::Reference< css::rendering::XBitmap > SAL_CALL getScaledBitmap( const css::geometry::RealSize2D& newSize,
                                                                                                                   sal_Bool                                      beFast ) override
        {
            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            return BaseType::maCanvasHelper.getScaledBitmap( newSize, beFast );
        }

    };

    template< class Base,
              class CanvasHelper,
              class Mutex=::osl::MutexGuard,
              class UnambiguousBase = css::uno::XInterface > class BitmapCanvasBase2 :
        public BitmapCanvasBase< Base, CanvasHelper, Mutex, UnambiguousBase >
    {
        typedef BitmapCanvasBase< Base, CanvasHelper, Mutex, UnambiguousBase >
            BaseType;

    public:
        // XBitmapCanvas
        virtual void SAL_CALL copyRect( const css::uno::Reference< css::rendering::XBitmapCanvas >&   sourceCanvas,
                                        const css::geometry::RealRectangle2D&                                      sourceRect,
                                        const css::rendering::ViewState&                                           sourceViewState,
                                        const css::rendering::RenderState&                                         sourceRenderState,
                                        const css::geometry::RealRectangle2D&                                      destRect,
                                        const css::rendering::ViewState&                                           destViewState,
                                        const css::rendering::RenderState&                                         destRenderState ) override
        {
            tools::verifyArgs(sourceCanvas, sourceRect, sourceViewState, sourceRenderState,
                              destRect, destViewState, destRenderState,
                              OSL_THIS_FUNC,
                              static_cast< typename BaseType::UnambiguousBaseType* >(this));

            typename BaseType::BaseType::MutexType aGuard( BaseType::m_aMutex );

            BaseType::BaseType::mbSurfaceDirty = true;
        }
    };
}

#endif // INCLUDED_CANVAS_BASE_BITMAPCANVASBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
