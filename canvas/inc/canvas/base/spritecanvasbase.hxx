/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: spritecanvasbase.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:44:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_CANVAS_SPRITECANVASBASE_HXX
#define INCLUDED_CANVAS_SPRITECANVASBASE_HXX

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _COM_SUN_STAR_RENDERING_XSPRITECANVAS_HPP_
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_INTERPOLATIONMODE_HPP_
#include <com/sun/star/rendering/InterpolationMode.hpp>
#endif

#ifndef INCLUDED_CANVAS_INTEGERBITMAPBASE_HXX
#include <canvas/base/integerbitmapbase.hxx>
#endif
#ifndef INCLUDED_CANVAS_SPRITEREDRAWMANAGER_HXX
#include <canvas/spriteredrawmanager.hxx>
#endif


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

        virtual void SAL_CALL disposing()
        {
            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            maRedrawManager.disposing();

            // pass on to base class
            BaseType::disposing();
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
