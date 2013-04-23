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

#ifndef INCLUDED_CANVAS_BITMAPCANVASBASE_HXX
#define INCLUDED_CANVAS_BITMAPCANVASBASE_HXX

#include <canvas/base/canvasbase.hxx>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>

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
        OBaseMutex-provided lock.  Everytime one of the methods is
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
              class UnambiguousBase=::com::sun::star::uno::XInterface > class BitmapCanvasBase :
        public CanvasBase< Base, CanvasHelper, Mutex, UnambiguousBase >
    {
    public:
        typedef CanvasBase< Base, CanvasHelper, Mutex, UnambiguousBase >    BaseType;

        // XBitmapCanvas
        virtual void SAL_CALL copyRect( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapCanvas >&   sourceCanvas,
                                        const ::com::sun::star::geometry::RealRectangle2D&                                      sourceRect,
                                        const ::com::sun::star::rendering::ViewState&                                           sourceViewState,
                                        const ::com::sun::star::rendering::RenderState&                                         sourceRenderState,
                                        const ::com::sun::star::geometry::RealRectangle2D&                                      destRect,
                                        const ::com::sun::star::rendering::ViewState&                                           destViewState,
                                        const ::com::sun::star::rendering::RenderState&                                         destRenderState ) throw (::com::sun::star::lang::IllegalArgumentException,
                                                                                                                                                         ::com::sun::star::uno::RuntimeException)
        {
            tools::verifyArgs(sourceCanvas, sourceRect, sourceViewState, sourceRenderState,
                              destRect, destViewState, destRenderState,
                              BOOST_CURRENT_FUNCTION,
                              static_cast< typename BaseType::UnambiguousBaseType* >(this));

            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            BaseType::mbSurfaceDirty = true;
            BaseType::maCanvasHelper.modifying();

            BaseType::maCanvasHelper.copyRect( this,
                                               sourceCanvas,
                                               sourceRect,
                                               sourceViewState,
                                               sourceRenderState,
                                               destRect,
                                               destViewState,
                                               destRenderState );
        }

        // XBitmap
        virtual ::com::sun::star::geometry::IntegerSize2D SAL_CALL getSize(  ) throw (::com::sun::star::uno::RuntimeException)
        {
            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            return BaseType::maCanvasHelper.getSize();
        }

        virtual ::sal_Bool SAL_CALL hasAlpha(  ) throw (::com::sun::star::uno::RuntimeException)
        {
            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            return BaseType::maCanvasHelper.hasAlpha();
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap > SAL_CALL getScaledBitmap( const ::com::sun::star::geometry::RealSize2D& newSize,
                                                                                                                   sal_Bool                                      beFast ) throw (::com::sun::star::uno::RuntimeException)
        {
            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            return BaseType::maCanvasHelper.getScaledBitmap( newSize, beFast );
        }

    };
}

#endif /* INCLUDED_CANVAS_BITMAPCANVASBASE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
