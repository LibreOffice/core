/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <com/sun/star/geometry/IntegerSize2D.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/rendering/TextDirection.hpp>
#include <cpo/uno/Sequence.hxx>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <verifyinput.hxx>
#include <canvasfont.hxx>

namespace com::sun::star::beans { struct PropertyValue; }
namespace vclcanvas { class CachedBitmap; }
namespace vclcanvas { class CanvasFont; }
namespace vclcanvas { class XGraphicDevice; }
namespace vclcanvas { class TextLayout; }
namespace com::sun::star::rendering { class XPolyPolygon2D; }
namespace com::sun::star::rendering { struct FontInfo; }
namespace com::sun::star::rendering { struct StringContext; }

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
        a Base( const Mutex& ) constructor (like the
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
            public Base
    {
    public:
        typedef Base            BaseType;
        typedef Mutex           MutexType;
        typedef UnambiguousBase UnambiguousBaseType;

        /** Create BitmapCanvasBase
         */
        BitmapCanvasBase() :
            maCanvasHelper(),
            mbSurfaceDirty( true )
        {
        }

        virtual void disposeThis() override
        {
            MutexType aGuard( BaseType::m_aMutex );

            maCanvasHelper.disposing();

            // pass on to base class
            BaseType::disposeThis();
        }

        void clear()
        {
            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            maCanvasHelper.clear();
        }

        void drawPoint(const css::geometry::RealPoint2D&     aPoint,
                                        const ::vclcanvas::ViewState&      viewState,
                                        const ::vclcanvas::RenderState&    renderState)
        {
            canvastools::verifyArgs(aPoint, viewState, renderState,
                              __func__,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;
        }

        css::uno::Reference< vclcanvas::XGraphicDevice > getDevice()
        {
            MutexType aGuard( BaseType::m_aMutex );

            return maCanvasHelper.getDevice();
        }

    protected:
        ~BitmapCanvasBase() {} // we're a ref-counted UNO class. _We_ destroy ourselves.

        css::geometry::IntegerSize2D getSize(  )
        {
            MutexType aGuard( BaseType::m_aMutex );

            return maCanvasHelper.getSize();
        }

        bool hasAlpha(  )
        {
            return true;
        }

        CanvasHelper        maCanvasHelper;
        mutable bool        mbSurfaceDirty;

    private:
        BitmapCanvasBase( const BitmapCanvasBase& ) = delete;
        BitmapCanvasBase& operator=( const BitmapCanvasBase& ) = delete;
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
