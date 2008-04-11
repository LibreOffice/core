/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: integerbitmapbase.hxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_CANVAS_INTEGERBITMAPBASE_HXX
#define INCLUDED_CANVAS_INTEGERBITMAPBASE_HXX

#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <canvas/base/bitmapcanvasbase.hxx>


namespace canvas
{
    /** Helper template to handle XIntegerBitmap method forwarding to
        BitmapCanvasHelper

        Use this helper to handle the XIntegerBitmap part of your
        implementation.

        @tpl Base
        Base class to use, most probably one of the
        WeakComponentImplHelperN templates with the appropriate
        interfaces. At least XIntegerBitmap should be among them (why
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
              class UnambiguousBase=::com::sun::star::uno::XInterface > class IntegerBitmapBase :
        public BitmapCanvasBase< Base, CanvasHelper, Mutex, UnambiguousBase >
    {
    public:
        typedef BitmapCanvasBase< Base, CanvasHelper, Mutex, UnambiguousBase >  BaseType;

        // XIntegerBitmap
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getData( ::com::sun::star::rendering::IntegerBitmapLayout&     bitmapLayout,
                                                                              const ::com::sun::star::geometry::IntegerRectangle2D& rect ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::rendering::VolatileContentDestroyedException, ::com::sun::star::uno::RuntimeException)
        {
            tools::verifyArgs(rect,
                              BOOST_CURRENT_FUNCTION,
                              static_cast< typename BaseType::UnambiguousBaseType* >(this));
            tools::verifyIndexRange(rect, BaseType::getSize() );

            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            return BaseType::maCanvasHelper.getData( bitmapLayout,
                                                     rect );
        }

        virtual void SAL_CALL setData( const ::com::sun::star::uno::Sequence< sal_Int8 >&      data,
                                       const ::com::sun::star::rendering::IntegerBitmapLayout& bitmapLayout,
                                       const ::com::sun::star::geometry::IntegerRectangle2D&   rect ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
        {
            tools::verifyArgs(bitmapLayout, rect,
                              BOOST_CURRENT_FUNCTION,
                              static_cast< typename BaseType::UnambiguousBaseType* >(this));
            tools::verifyIndexRange(rect, BaseType::getSize() );

            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            BaseType::mbSurfaceDirty = true;
            BaseType::maCanvasHelper.modifying();

            BaseType::maCanvasHelper.setData( data, bitmapLayout, rect );
        }

        virtual void SAL_CALL setPixel( const ::com::sun::star::uno::Sequence< sal_Int8 >&      color,
                                        const ::com::sun::star::rendering::IntegerBitmapLayout& bitmapLayout,
                                        const ::com::sun::star::geometry::IntegerPoint2D&       pos ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
        {
            tools::verifyArgs(bitmapLayout, pos,
                              BOOST_CURRENT_FUNCTION,
                              static_cast< typename BaseType::UnambiguousBaseType* >(this));
            tools::verifyIndexRange(pos, BaseType::getSize() );

            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            BaseType::mbSurfaceDirty = true;
            BaseType::maCanvasHelper.modifying();

            BaseType::maCanvasHelper.setPixel( color, bitmapLayout, pos );
        }

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getPixel( ::com::sun::star::rendering::IntegerBitmapLayout& bitmapLayout,
                                                                               const ::com::sun::star::geometry::IntegerPoint2D& pos ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::rendering::VolatileContentDestroyedException, ::com::sun::star::uno::RuntimeException)
        {
            tools::verifyArgs(pos,
                              BOOST_CURRENT_FUNCTION,
                              static_cast< typename BaseType::UnambiguousBaseType* >(this));
            tools::verifyIndexRange(pos, BaseType::getSize() );

            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            return BaseType::maCanvasHelper.getPixel( bitmapLayout,
                                                      pos );
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapPalette > SAL_CALL getPalette(  ) throw (::com::sun::star::uno::RuntimeException)
        {
            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            return BaseType::maCanvasHelper.getPalette();
        }

        virtual ::com::sun::star::rendering::IntegerBitmapLayout SAL_CALL getMemoryLayout(  ) throw (::com::sun::star::uno::RuntimeException)
        {
            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            return BaseType::maCanvasHelper.getMemoryLayout();
        }
    };
}

#endif /* INCLUDED_CANVAS_INTEGERBITMAPBASE_HXX */
