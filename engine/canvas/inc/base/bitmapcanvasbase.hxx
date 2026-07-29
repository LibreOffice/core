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
#include <verifyinput.hxx>

namespace com::sun::star::beans { struct PropertyValue; }
namespace com::sun::star::rendering { class XBitmap; }
namespace com::sun::star::rendering { class XCachedPrimitive; }
namespace com::sun::star::rendering { class XCanvasFont; }
namespace vclcanvas { class XGraphicDevice; }
namespace com::sun::star::rendering { class XPolyPolygon2D; }
namespace com::sun::star::rendering { class XTextLayout; }
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

        // XCanvas
        virtual void clear() override
        {
            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            maCanvasHelper.clear();
        }

        virtual void drawPoint(const css::geometry::RealPoint2D&     aPoint,
                                        const css::rendering::ViewState&      viewState,
                                        const css::rendering::RenderState&    renderState) override
        {
            canvastools::verifyArgs(aPoint, viewState, renderState,
                              __func__,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;
        }

        virtual void drawLine(const css::geometry::RealPoint2D&  aStartPoint,
                                       const css::geometry::RealPoint2D&  aEndPoint,
                                       const css::rendering::ViewState&   viewState,
                                       const css::rendering::RenderState& renderState) override
        {
            canvastools::verifyArgs(aStartPoint, aEndPoint, viewState, renderState,
                              __func__,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            maCanvasHelper.drawLine( this, aStartPoint, aEndPoint, viewState, renderState );
        }

        virtual void
            drawPolyPolygon(const css::uno::Reference< css::rendering::XPolyPolygon2D >& xPolyPolygon,
                            const css::rendering::ViewState&                             viewState,
                            const css::rendering::RenderState&                           renderState) override
        {
            canvastools::verifyArgs(xPolyPolygon, viewState, renderState,
                              __func__,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            maCanvasHelper.drawPolyPolygon( this, xPolyPolygon, viewState, renderState );
        }

        virtual void
            strokePolyPolygon(const css::uno::Reference< css::rendering::XPolyPolygon2D >&   xPolyPolygon,
                              const css::rendering::ViewState&                               viewState,
                              const css::rendering::RenderState&                             renderState,
                              const css::rendering::StrokeAttributes&                        strokeAttributes) override
        {
            canvastools::verifyArgs(xPolyPolygon, viewState, renderState, strokeAttributes,
                              __func__,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            maCanvasHelper.strokePolyPolygon( this, xPolyPolygon, viewState, renderState, strokeAttributes );
        }

        virtual css::uno::Reference< css::rendering::XCachedPrimitive >
            fillPolyPolygon(const css::uno::Reference< css::rendering::XPolyPolygon2D >&               xPolyPolygon,
                             const css::rendering::ViewState&                                          viewState,
                             const css::rendering::RenderState&                                        renderState) override
        {
            canvastools::verifyArgs(xPolyPolygon, viewState, renderState,
                              __func__,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            return maCanvasHelper.fillPolyPolygon( this, xPolyPolygon, viewState, renderState );
        }

        virtual css::uno::Reference< css::rendering::XCachedPrimitive >
            fillTexturedPolyPolygon(const css::uno::Reference< css::rendering::XPolyPolygon2D >& xPolyPolygon,
                                    const css::rendering::ViewState&                             viewState,
                                    const css::rendering::RenderState&                           renderState,
                                    const cpo::uno::Sequence< css::rendering::Texture >&         textures) override
        {
            canvastools::verifyArgs(xPolyPolygon, viewState, renderState, textures,
                              __func__,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            return maCanvasHelper.fillTexturedPolyPolygon( this, xPolyPolygon, viewState, renderState, textures );
        }

        virtual css::uno::Reference< css::rendering::XCanvasFont >
            createFont( const css::rendering::FontRequest&                                     fontRequest,
                        const cpo::uno::Sequence< css::beans::PropertyValue >&                 extraFontProperties,
                        const css::geometry::Matrix2D&                                         fontMatrix ) override
        {
            canvastools::verifyArgs(fontRequest,
                              // dummy, to keep argPos in sync
                              fontRequest,
                              fontMatrix,
                              __func__,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            return maCanvasHelper.createFont( this, fontRequest, extraFontProperties, fontMatrix );
        }


        virtual void
            drawText(const css::rendering::StringContext&                                     text,
                     const css::uno::Reference< css::rendering::XCanvasFont >&                xFont,
                     const css::rendering::ViewState&                                         viewState,
                     const css::rendering::RenderState&                                       renderState,
                     sal_Int8                                                                 textDirection) override
        {
            canvastools::verifyArgs(xFont, viewState, renderState,
                              __func__,
                              static_cast< UnambiguousBaseType* >(this));
            canvastools::verifyRange( textDirection,
                                css::rendering::TextDirection::WEAK_LEFT_TO_RIGHT,
                                css::rendering::TextDirection::STRONG_RIGHT_TO_LEFT );

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            maCanvasHelper.drawText( this, text, xFont, viewState, renderState, textDirection );
        }


        virtual void
            drawTextLayout(const css::uno::Reference< css::rendering::XTextLayout >&               laidOutText,
                            const css::rendering::ViewState&                                       viewState,
                            const css::rendering::RenderState&                                     renderState) override
        {
            canvastools::verifyArgs(laidOutText, viewState, renderState,
                              __func__,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            maCanvasHelper.drawTextLayout( this, laidOutText, viewState, renderState );
        }


        virtual css::uno::Reference< css::rendering::XCachedPrimitive >
            drawBitmap( const css::uno::Reference< css::rendering::XBitmap >&              xBitmap,
                        const css::rendering::ViewState&                                   viewState,
                        const css::rendering::RenderState&                                 renderState ) override
        {
            canvastools::verifyArgs(xBitmap, viewState, renderState,
                              __func__,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            return maCanvasHelper.drawBitmap( this, xBitmap, viewState, renderState );
        }

        virtual css::uno::Reference< vclcanvas::XGraphicDevice >
            getDevice() override
        {
            MutexType aGuard( BaseType::m_aMutex );

            return maCanvasHelper.getDevice();
        }

    protected:
        ~BitmapCanvasBase() {} // we're a ref-counted UNO class. _We_ destroy ourselves.

        // XBitmap
        virtual css::geometry::IntegerSize2D getSize(  ) override
        {
            MutexType aGuard( BaseType::m_aMutex );

            return maCanvasHelper.getSize();
        }

        virtual bool hasAlpha(  ) override
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
