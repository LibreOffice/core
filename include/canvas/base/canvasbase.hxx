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

#ifndef INCLUDED_CANVAS_BASE_CANVASBASE_HXX
#define INCLUDED_CANVAS_BASE_CANVASBASE_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/TextDirection.hpp>
#include <osl/mutex.hxx>
#include <canvas/verifyinput.hxx>


namespace canvas
{
    /** Helper template to handle XCanvas method forwarding to CanvasHelper

        Use this helper to handle the XCanvas part of your
        implementation. In theory, we could have provided CanvasHelper
        and CanvasBase as a single template, but that would duplicate
        a lot of code now residing in CanvasHelper only.

        This template basically interposes itself between the full
        interface you implement (i.e. not restricted to XCanvas. The
        problem with UNO partial interface implementation actually is,
        that you cannot do it the plain way, since deriving from a
        common base subclass always introduces the whole set of pure
        virtuals, that your baseclass helper just overrided) and your
        implementation class. You then only have to implement the
        functionality <em>besides</em> XCanvas.

        <pre>
        Example:
        typedef ::cppu::WeakComponentImplHelper4< ::com::sun::star::rendering::XSpriteCanvas,
                                                   ::com::sun::star::lang::XInitialization,
                                                  ::com::sun::star::lang::XServiceInfo,
                                                  ::com::sun::star::lang::XServiceName > CanvasBase_Base;
        typedef ::canvas::internal::CanvasBase< CanvasBase_Base, CanvasHelper > ExampleCanvas_Base;

        class ExampleCanvas : public ExampleCanvas_Base,
                              public SpriteSurface,
                               public RepaintTarget
        {
        };
        </pre>

        @tpl Base
        Base class to use, most probably one of the
        WeakComponentImplHelperN templates with the appropriate
        interfaces. At least XCanvas should be among them (why else
        would you use this template, then?). Base class must have an
        Base( const Mutex& ) constructor (like the
        WeakComponentImplHelperN templates have). As the very least,
        the base class must be derived from uno::XInterface, as some
        error reporting mechanisms rely on that.

        @tpl CanvasHelper
        Canvas helper implementation for the backend in question. This
        object will be held as a member of this template class, and
        basically gets forwarded all XCanvas API calls. Furthermore,
        every time the canvas API semantically changes the content of
        the canvas, CanvasHelper::modifying() will get called
        (<em>before</em> the actual modification takes place).

        @tpl Mutex
        Lock strategy to use. Defaults to using the
        OBaseMutex-provided lock.  Every time one of the methods is
        entered, an object of type Mutex is created with m_aMutex as
        the sole parameter, and destroyed again when the method scope
        is left.

        @tpl UnambiguousBase
        Optional unambiguous base class for XInterface of Base. It's
        sometimes necessary to specify this parameter, e.g. if Base
        derives from multiple UNO interface (were each provides its
        own version of XInterface, making the conversion ambiguous)
     */
    template< class Base,
              class CanvasHelper,
              class Mutex=::osl::MutexGuard,
              class UnambiguousBase=::com::sun::star::uno::XInterface > class CanvasBase :
            public Base
    {
    public:
        typedef Base            BaseType;
        typedef CanvasHelper    HelperType;
        typedef Mutex           MutexType;
        typedef UnambiguousBase UnambiguousBaseType;

        /** Create CanvasBase
         */
        CanvasBase() :
            maCanvasHelper(),
            mbSurfaceDirty( true )
        {
        }

        virtual void disposeThis() SAL_OVERRIDE
        {
            MutexType aGuard( BaseType::m_aMutex );

            maCanvasHelper.disposing();

            // pass on to base class
            BaseType::disposeThis();
        }

        // XCanvas
        virtual void SAL_CALL clear() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            maCanvasHelper.clear();
        }

        virtual void SAL_CALL drawPoint(const css::geometry::RealPoint2D&     aPoint,
                                        const css::rendering::ViewState&      viewState,
                                        const css::rendering::RenderState&    renderState)
            throw (css::lang::IllegalArgumentException,
                   css::uno::RuntimeException,
                   std::exception) SAL_OVERRIDE
        {
            tools::verifyArgs(aPoint, viewState, renderState,
                              BOOST_CURRENT_FUNCTION,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;
        }

        virtual void SAL_CALL drawLine(const css::geometry::RealPoint2D&  aStartPoint,
                                       const css::geometry::RealPoint2D&  aEndPoint,
                                       const css::rendering::ViewState&   viewState,
                                       const css::rendering::RenderState& renderState)
                                            throw (css::lang::IllegalArgumentException,
                                                   css::uno::RuntimeException,
                                                   std::exception) SAL_OVERRIDE
        {
            tools::verifyArgs(aStartPoint, aEndPoint, viewState, renderState,
                              BOOST_CURRENT_FUNCTION,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            maCanvasHelper.drawLine( this, aStartPoint, aEndPoint, viewState, renderState );
        }

        virtual void SAL_CALL drawBezier( const css::geometry::RealBezierSegment2D&    aBezierSegment,
                                          const css::geometry::RealPoint2D&            aEndPoint,
                                          const css::rendering::ViewState&             viewState,
                                          const css::rendering::RenderState&           renderState )
                                            throw (css::lang::IllegalArgumentException,
                                                   css::uno::RuntimeException,
                                                   std::exception) SAL_OVERRIDE
        {
            tools::verifyArgs(aBezierSegment, aEndPoint, viewState, renderState,
                              BOOST_CURRENT_FUNCTION,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            maCanvasHelper.drawBezier( this, aBezierSegment, aEndPoint, viewState, renderState );
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            drawPolyPolygon(const css::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >& xPolyPolygon,
                            const css::rendering::ViewState&                                          viewState,
                            const css::rendering::RenderState&                                        renderState)
                                throw (css::lang::IllegalArgumentException,
                                       css::uno::RuntimeException,
                                       std::exception) SAL_OVERRIDE
        {
            tools::verifyArgs(xPolyPolygon, viewState, renderState,
                              BOOST_CURRENT_FUNCTION,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            return maCanvasHelper.drawPolyPolygon( this, xPolyPolygon, viewState, renderState );
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            strokePolyPolygon(const css::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >&   xPolyPolygon,
                              const css::rendering::ViewState&                                            viewState,
                              const css::rendering::RenderState&                                          renderState,
                              const css::rendering::StrokeAttributes&                                     strokeAttributes)
                                throw (css::lang::IllegalArgumentException,
                                       css::uno::RuntimeException,
                                       std::exception) SAL_OVERRIDE
        {
            tools::verifyArgs(xPolyPolygon, viewState, renderState, strokeAttributes,
                              BOOST_CURRENT_FUNCTION,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            return maCanvasHelper.strokePolyPolygon( this, xPolyPolygon, viewState, renderState, strokeAttributes );
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            strokeTexturedPolyPolygon( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >&   xPolyPolygon,
                                       const ::com::sun::star::rendering::ViewState&                                            viewState,
                                       const ::com::sun::star::rendering::RenderState&                                          renderState,
                                       const ::com::sun::star::uno::Sequence< ::com::sun::star::rendering::Texture >&           textures,
                                       const ::com::sun::star::rendering::StrokeAttributes&                                     strokeAttributes ) throw (::com::sun::star::lang::IllegalArgumentException,
                                                                                                                                                          ::com::sun::star::uno::RuntimeException) SAL_OVERRIDE
        {
            tools::verifyArgs(xPolyPolygon, viewState, renderState, strokeAttributes,
                              BOOST_CURRENT_FUNCTION,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            return maCanvasHelper.strokeTexturedPolyPolygon( this, xPolyPolygon, viewState, renderState, textures, strokeAttributes );
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            strokeTextureMappedPolyPolygon( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >&  xPolyPolygon,
                                            const ::com::sun::star::rendering::ViewState&                                           viewState,
                                            const ::com::sun::star::rendering::RenderState&                                         renderState,
                                            const ::com::sun::star::uno::Sequence< ::com::sun::star::rendering::Texture >&          textures,
                                            const ::com::sun::star::uno::Reference< ::com::sun::star::geometry::XMapping2D >&       xMapping,
                                            const ::com::sun::star::rendering::StrokeAttributes&                                    strokeAttributes ) throw (::com::sun::star::lang::IllegalArgumentException,
                                                                                                                                                              ::com::sun::star::uno::RuntimeException) SAL_OVERRIDE
        {
            tools::verifyArgs(xPolyPolygon, viewState, renderState, textures, xMapping, strokeAttributes,
                              BOOST_CURRENT_FUNCTION,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            return maCanvasHelper.strokeTextureMappedPolyPolygon( this, xPolyPolygon, viewState, renderState, textures, xMapping, strokeAttributes );
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >   SAL_CALL
            queryStrokeShapes( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >&   xPolyPolygon,
                               const ::com::sun::star::rendering::ViewState&                                            viewState,
                               const ::com::sun::star::rendering::RenderState&                                          renderState,
                               const ::com::sun::star::rendering::StrokeAttributes&                                     strokeAttributes ) throw (::com::sun::star::lang::IllegalArgumentException,
                                                                                                                                                  ::com::sun::star::uno::RuntimeException) SAL_OVERRIDE
        {
            tools::verifyArgs(xPolyPolygon, viewState, renderState, strokeAttributes,
                              BOOST_CURRENT_FUNCTION,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            return maCanvasHelper.queryStrokeShapes( this, xPolyPolygon, viewState, renderState, strokeAttributes );
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            fillPolyPolygon(const css::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >& xPolyPolygon,
                             const css::rendering::ViewState&                                          viewState,
                             const css::rendering::RenderState&                                        renderState)
                                throw (css::lang::IllegalArgumentException,
                                       css::uno::RuntimeException,
                                       std::exception) SAL_OVERRIDE
        {
            tools::verifyArgs(xPolyPolygon, viewState, renderState,
                              BOOST_CURRENT_FUNCTION,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            return maCanvasHelper.fillPolyPolygon( this, xPolyPolygon, viewState, renderState );
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            fillTexturedPolyPolygon(const css::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >& xPolyPolygon,
                                    const css::rendering::ViewState&                                          viewState,
                                    const css::rendering::RenderState&                                        renderState,
                                    const css::uno::Sequence< ::com::sun::star::rendering::Texture >&         textures)
                                        throw (css::lang::IllegalArgumentException,
                                               css::uno::RuntimeException,
                                               std::exception) SAL_OVERRIDE
        {
            tools::verifyArgs(xPolyPolygon, viewState, renderState, textures,
                              BOOST_CURRENT_FUNCTION,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            return maCanvasHelper.fillTexturedPolyPolygon( this, xPolyPolygon, viewState, renderState, textures );
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            fillTextureMappedPolyPolygon( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >&    xPolyPolygon,
                                          const ::com::sun::star::rendering::ViewState&                                             viewState,
                                          const ::com::sun::star::rendering::RenderState&                                           renderState,
                                          const ::com::sun::star::uno::Sequence< ::com::sun::star::rendering::Texture >&            textures,
                                          const ::com::sun::star::uno::Reference< ::com::sun::star::geometry::XMapping2D >&         xMapping ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException) SAL_OVERRIDE
        {
            tools::verifyArgs(xPolyPolygon, viewState, renderState, textures, xMapping,
                              BOOST_CURRENT_FUNCTION,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            return maCanvasHelper.fillTextureMappedPolyPolygon( this, xPolyPolygon, viewState, renderState, textures, xMapping );
        }


        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvasFont > SAL_CALL
            createFont( const ::com::sun::star::rendering::FontRequest&                                     fontRequest,
                        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >&    extraFontProperties,
                        const ::com::sun::star::geometry::Matrix2D&                                         fontMatrix ) throw (::com::sun::star::lang::IllegalArgumentException,
                                                                                                                                ::com::sun::star::uno::RuntimeException) SAL_OVERRIDE
        {
            tools::verifyArgs(fontRequest,
                              // dummy, to keep argPos in sync
                              fontRequest,
                              fontMatrix,
                              BOOST_CURRENT_FUNCTION,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            return maCanvasHelper.createFont( this, fontRequest, extraFontProperties, fontMatrix );
        }


        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::rendering::FontInfo > SAL_CALL
            queryAvailableFonts( const ::com::sun::star::rendering::FontInfo&                                       aFilter,
                                 const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >&   aFontProperties ) throw (::com::sun::star::lang::IllegalArgumentException,
                                                                                                                                             ::com::sun::star::uno::RuntimeException) SAL_OVERRIDE
        {
            tools::verifyArgs(aFilter,
                              BOOST_CURRENT_FUNCTION,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            return maCanvasHelper.queryAvailableFonts( this, aFilter, aFontProperties );
        }


        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            drawText(const css::rendering::StringContext&                                     text,
                     const css::uno::Reference< ::com::sun::star::rendering::XCanvasFont >&   xFont,
                     const css::rendering::ViewState&                                         viewState,
                     const css::rendering::RenderState&                                       renderState,
                     sal_Int8                                                                 textDirection)
                throw (css::lang::IllegalArgumentException,
                       css::uno::RuntimeException,
                       std::exception) SAL_OVERRIDE
        {
            tools::verifyArgs(xFont, viewState, renderState,
                              BOOST_CURRENT_FUNCTION,
                              static_cast< UnambiguousBaseType* >(this));
            tools::verifyRange( textDirection,
                                ::com::sun::star::rendering::TextDirection::WEAK_LEFT_TO_RIGHT,
                                ::com::sun::star::rendering::TextDirection::STRONG_RIGHT_TO_LEFT );

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            return maCanvasHelper.drawText( this, text, xFont, viewState, renderState, textDirection );
        }


        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            drawTextLayout(const css::uno::Reference< ::com::sun::star::rendering::XTextLayout >& laidOutText,
                            const css::rendering::ViewState&                                       viewState,
                            const css::rendering::RenderState&                                     renderState)
                            throw (css::lang::IllegalArgumentException,
                                   css::uno::RuntimeException,
                                   std::exception) SAL_OVERRIDE
        {
            tools::verifyArgs(laidOutText, viewState, renderState,
                              BOOST_CURRENT_FUNCTION,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            return maCanvasHelper.drawTextLayout( this, laidOutText, viewState, renderState );
        }


        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            drawBitmap( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap >& xBitmap,
                        const ::com::sun::star::rendering::ViewState&                                   viewState,
                        const ::com::sun::star::rendering::RenderState&                                 renderState ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException) SAL_OVERRIDE
        {
            tools::verifyArgs(xBitmap, viewState, renderState,
                              BOOST_CURRENT_FUNCTION,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            return maCanvasHelper.drawBitmap( this, xBitmap, viewState, renderState );
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            drawBitmapModulated( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap >&    xBitmap,
                                 const ::com::sun::star::rendering::ViewState&                                      viewState,
                                 const ::com::sun::star::rendering::RenderState&                                    renderState ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException) SAL_OVERRIDE
        {
            tools::verifyArgs(xBitmap, viewState, renderState,
                              BOOST_CURRENT_FUNCTION,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            return maCanvasHelper.drawBitmapModulated( this, xBitmap, viewState, renderState );
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >   SAL_CALL
            getDevice() throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE
        {
            MutexType aGuard( BaseType::m_aMutex );

            return maCanvasHelper.getDevice();
        }

    protected:
        ~CanvasBase() {} // we're a ref-counted UNO class. _We_ destroy ourselves.

        HelperType          maCanvasHelper;
        mutable bool        mbSurfaceDirty;

    private:
        CanvasBase( const CanvasBase& ) = delete;
        CanvasBase& operator=( const CanvasBase& ) = delete;
    };
}

#endif // INCLUDED_CANVAS_BASE_CANVASBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
