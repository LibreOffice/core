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

#ifndef INCLUDED_CANVAS_SPRITECANVASBASE_HXX
#define INCLUDED_CANVAS_SPRITECANVASBASE_HXX

#include <rtl/ref.hxx>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/rendering/InterpolationMode.hpp>
#include <canvas/base/integerbitmapbase.hxx>
#include <canvas/spriteredrawmanager.hxx>


namespace canvas
{
    /** Helper template to handle XIntegerBitmap method forwarding to
        BitmapCanvasHelper

        Use this helper to handle the XIntegerBitmap part of your
        implementation.

        @tpl Base
        Base class to use, most probably one of the
        WeakComponentImplHelperN templates with the appropriate
        interfaces. At least XSpriteCanvas and SpriteSurface should be
        among them (why else would you use this template, then?). Base
        class must have an Base( const Mutex& ) constructor (like the
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
              class UnambiguousBase=::com::sun::star::uno::XInterface > class SpriteCanvasBase :
        public IntegerBitmapBase< Base, CanvasHelper, Mutex, UnambiguousBase >
    {
    public:
        typedef IntegerBitmapBase< Base, CanvasHelper, Mutex, UnambiguousBase > BaseType;
        typedef ::rtl::Reference< SpriteCanvasBase >                            Reference;

        SpriteCanvasBase() :
            maRedrawManager()
        {
        }

        virtual void disposeThis()
        {
            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            maRedrawManager.disposing();

            // pass on to base class
            BaseType::disposeThis();
        }

        // XSpriteCanvas
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XAnimatedSprite > SAL_CALL createSpriteFromAnimation( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XAnimation >& animation ) throw (::com::sun::star::lang::IllegalArgumentException,
                                                                                                                                                                                                                                           ::com::sun::star::uno::RuntimeException)
        {
            tools::verifyArgs(animation,
                              BOOST_CURRENT_FUNCTION,
                              static_cast< typename BaseType::UnambiguousBaseType* >(this));

            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            return BaseType::maCanvasHelper.createSpriteFromAnimation(animation);
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XAnimatedSprite > SAL_CALL createSpriteFromBitmaps( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap > >& animationBitmaps,
                                                                                                                                   sal_Int8                                                                                                           interpolationMode ) throw (::com::sun::star::lang::IllegalArgumentException,
                                                                                                                                                                                                                                                                                 ::com::sun::star::rendering::VolatileContentDestroyedException,
                                                                                                                                                                                                                                                                                 ::com::sun::star::uno::RuntimeException)
        {
            tools::verifyArgs(animationBitmaps,
                              BOOST_CURRENT_FUNCTION,
                              static_cast< typename BaseType::UnambiguousBaseType* >(this));
            tools::verifyRange( interpolationMode,
                                ::com::sun::star::rendering::InterpolationMode::NEAREST_NEIGHBOR,
                                ::com::sun::star::rendering::InterpolationMode::BEZIERSPLINE4 );

            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            return BaseType::maCanvasHelper.createSpriteFromBitmaps(animationBitmaps, interpolationMode);
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCustomSprite > SAL_CALL createCustomSprite( const ::com::sun::star::geometry::RealSize2D& spriteSize ) throw (::com::sun::star::lang::IllegalArgumentException,
                                                                                                                                                                                              ::com::sun::star::uno::RuntimeException)
        {
            tools::verifySpriteSize(spriteSize,
                                    BOOST_CURRENT_FUNCTION,
                                    static_cast< typename BaseType::UnambiguousBaseType* >(this));

            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            return BaseType::maCanvasHelper.createCustomSprite(spriteSize);
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XSprite > SAL_CALL createClonedSprite( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XSprite >& original ) throw (::com::sun::star::lang::IllegalArgumentException,
                                                                                                                                                                                                                        ::com::sun::star::uno::RuntimeException)
        {
            tools::verifyArgs(original,
                              BOOST_CURRENT_FUNCTION,
                              static_cast< typename BaseType::UnambiguousBaseType* >(this));

            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            return BaseType::maCanvasHelper.createClonedSprite(original);
        }

        // SpriteSurface
        virtual void showSprite( const Sprite::Reference& rSprite )
        {
            OSL_ASSERT( rSprite.is() );

            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            maRedrawManager.showSprite( rSprite );
        }

        virtual void hideSprite( const Sprite::Reference& rSprite )
        {
            OSL_ASSERT( rSprite.is() );

            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            maRedrawManager.hideSprite( rSprite );
        }

        virtual void moveSprite( const Sprite::Reference&       rSprite,
                                 const ::basegfx::B2DPoint&     rOldPos,
                                 const ::basegfx::B2DPoint&     rNewPos,
                                 const ::basegfx::B2DVector&    rSpriteSize )
        {
            OSL_ASSERT( rSprite.is() );

            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            maRedrawManager.moveSprite( rSprite, rOldPos, rNewPos, rSpriteSize );
        }

        virtual void updateSprite( const Sprite::Reference&     rSprite,
                                   const ::basegfx::B2DPoint&   rPos,
                                   const ::basegfx::B2DRange&   rUpdateArea )
        {
            OSL_ASSERT( rSprite.is() );

            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            maRedrawManager.updateSprite( rSprite, rPos, rUpdateArea );
        }

    protected:
        SpriteRedrawManager maRedrawManager;
    };
}

#endif /* INCLUDED_CANVAS_SPRITECANVASBASE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
