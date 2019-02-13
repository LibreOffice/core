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

#ifndef INCLUDED_CANVAS_BASE_CANVASCUSTOMSPRITEBASE_HXX
#define INCLUDED_CANVAS_BASE_CANVASCUSTOMSPRITEBASE_HXX

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/range/b2drange.hxx>
#include <canvas/base/integerbitmapbase.hxx>
#include <canvas/base/bitmapcanvasbase.hxx>

namespace com::sun::star::rendering { class XPolyPolygon2D; }

namespace canvas
{
    /** Helper template to handle XCustomSprite method forwarding to
        CanvasCustomSpriteHelper

        Use this helper to handle the XCustomSprite part of your
        implementation.

        @tpl Base
        Base class to use, most probably one of the
        WeakComponentImplHelperN templates with the appropriate
        interfaces. At least XCustomSprite and Sprite should be among
        them (why else would you use this template, then?). Base class
        must have an Base( const Mutex& ) constructor (like the
        WeakComponentImplHelperN templates have).

        @tpl SpriteHelper
        Sprite helper implementation for the backend in question

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

        @see CanvasCustomSpriteHelper for further contractual
        requirements towards the SpriteHelper type, and some examples.
     */
    template< class Base,
              class SpriteHelper,
              class CanvasHelper,
              class Mutex=::osl::MutexGuard,
              class UnambiguousBase = css::uno::XInterface > class CanvasCustomSpriteBase :
        public IntegerBitmapBase< BitmapCanvasBase2<Base, CanvasHelper, Mutex, UnambiguousBase> >
    {
    public:
        typedef IntegerBitmapBase< BitmapCanvasBase2<Base, CanvasHelper, Mutex, UnambiguousBase> > BaseType;

        CanvasCustomSpriteBase() :
            maSpriteHelper()
        {
        }

        /** Object is being disposed.

            Called from the cppu helper base, to notify disposal of
            this object. Already releases all internal references.

            @derive when overriding this method in derived classes,
            <em>always</em> call the base class' method!
         */
        virtual void disposeThis() override
        {
            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            maSpriteHelper.disposing();

            // pass on to base class
            BaseType::disposeThis();
        }

        // XCanvas: selectively override base's methods here, for opacity tracking
        virtual void SAL_CALL clear() override
        {
            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            maSpriteHelper.clearingContent( this );

            // and forward to base class, which handles the actual rendering
            return BaseType::clear();
        }

        virtual css::uno::Reference< css::rendering::XCachedPrimitive > SAL_CALL
            drawBitmap( const css::uno::Reference< css::rendering::XBitmap >&              xBitmap,
                        const css::rendering::ViewState&                                   viewState,
                        const css::rendering::RenderState&                                 renderState ) override
        {
            tools::verifyArgs(xBitmap, viewState, renderState,
                              OSL_THIS_FUNC,
                              static_cast< typename BaseType::UnambiguousBaseType* >(this));

            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            maSpriteHelper.checkDrawBitmap( this, xBitmap, viewState, renderState );

            // and forward to base class, which handles the actual rendering
            return BaseType::drawBitmap( xBitmap,
                                         viewState,
                                         renderState );
        }

        // TODO(F3): If somebody uses the XIntegerBitmap methods to
        // clear pixel (setting alpha != 1.0 there), or a compositing
        // mode results in similar alpha, maSpriteHelper might
        // erroneously report fully opaque sprites. Effectively, all
        // render methods must be overridden here; or better,
        // functionality provided at the baseclass.

        // XSprite
        virtual void SAL_CALL setAlpha( double alpha ) override
        {
            tools::verifyRange( alpha, 0.0, 1.0 );

            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            maSpriteHelper.setAlpha( this, alpha );
        }

        virtual void SAL_CALL move( const css::geometry::RealPoint2D&  aNewPos,
                                    const css::rendering::ViewState&   viewState,
                                    const css::rendering::RenderState& renderState ) override
        {
            tools::verifyArgs(aNewPos, viewState, renderState,
                              OSL_THIS_FUNC,
                              static_cast< typename BaseType::UnambiguousBaseType* >(this));

            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            maSpriteHelper.move( this, aNewPos, viewState, renderState );
        }

        virtual void SAL_CALL transform( const css::geometry::AffineMatrix2D& aTransformation ) override
        {
            tools::verifyArgs(aTransformation,
                              OSL_THIS_FUNC,
                              static_cast< typename BaseType::UnambiguousBaseType* >(this));

            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            maSpriteHelper.transform( this, aTransformation );
        }

        virtual void SAL_CALL clip( const css::uno::Reference< css::rendering::XPolyPolygon2D >& aClip ) override
        {
            // NULL xClip explicitly allowed here (to clear clipping)

            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            maSpriteHelper.clip( this, aClip );
        }

        virtual void SAL_CALL setPriority( double nPriority ) override
        {
            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            maSpriteHelper.setPriority( this, nPriority );
        }

        virtual void SAL_CALL show() override
        {
            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            maSpriteHelper.show( this );
        }

        virtual void SAL_CALL hide() override
        {
            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            maSpriteHelper.hide( this );
        }

        // XCustomSprite
        virtual css::uno::Reference< css::rendering::XCanvas > SAL_CALL
            getContentCanvas() override
        {
            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            return this;
        }

        // Sprite
        virtual bool isAreaUpdateOpaque( const ::basegfx::B2DRange& rUpdateArea ) const override
        {
            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            return maSpriteHelper.isAreaUpdateOpaque( rUpdateArea );
        }

        virtual bool isContentChanged() const override
        {
            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            return BaseType::mbSurfaceDirty;
        }

        virtual ::basegfx::B2DPoint getPosPixel() const override
        {
            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            return maSpriteHelper.getPosPixel();
        }

        virtual ::basegfx::B2DVector getSizePixel() const override
        {
            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            return maSpriteHelper.getSizePixel();
        }

        virtual ::basegfx::B2DRange getUpdateArea() const override
        {
            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            return maSpriteHelper.getUpdateArea();
        }

        virtual double getPriority() const override
        {
            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            return maSpriteHelper.getPriority();
        }

    protected:
        SpriteHelper maSpriteHelper;
    };
}

#endif // INCLUDED_CANVAS_BASE_CANVASCUSTOMSPRITEBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
