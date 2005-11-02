/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: doublebitmapbase.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:42:54 $
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

#ifndef INCLUDED_CANVAS_DOUBLEBITMAPBASE_HXX
#define INCLUDED_CANVAS_DOUBLEBITMAPBASE_HXX

#ifndef _COM_SUN_STAR_RENDERING_XIEEEDOUBLEBITMAP_HPP_
#include <com/sun/star/rendering/XIeeeDoubleBitmap.hpp>
#endif

#ifndef INCLUDED_CANVAS_BITMAPCANVASBASE_HXX
#include <canvas/bitmapcanvasbase.hxx>
#endif


namespace canvas
{
    /** Helper template to handle XIeeeDoubleBitmap method forwarding to
        BitmapCanvasHelper

        Use this helper to handle the XIeeeDoubleBitmap part of your
        implementation.

        @tpl Base
        Base class to use, most probably one of the
        WeakComponentImplHelperN templates with the appropriate
        interfaces. At least XIeeeDoubleBitmap should be among them (why
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
              class UnambiguousBase=::com::sun::star::uno::XInterface > class DoubleBitmapBase :
        public BitmapCanvasBase< Base, CanvasHelper, Mutex, UnambiguousBase >
    {
    public:
        typedef BitmapCanvasBase< Base, CanvasHelper, Mutex, UnambiguousBase >  BaseType;

        // XIeeeDoubleBitmap
        virtual ::com::sun::star::uno::Sequence< double > SAL_CALL getData( ::com::sun::star::rendering::FloatingPointBitmapLayout& bitmapLayout,
                                                                            const ::com::sun::star::geometry::IntegerRectangle2D&   rect ) throw (::com::sun::star::lang::IndexOutOfBoundsException,
                                                                                                                                                  ::com::sun::star::rendering::VolatileContentDestroyedException,
                                                                                                                                                  ::com::sun::star::uno::RuntimeException)
        {
            verifyInput(rect, this);
            verifyIndexRange(rect, getSize() );

            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            return BaseType::maCanvasHelper.getData( bitmapLayout,
                                                     rect );
        }

        virtual void SAL_CALL setData( const ::com::sun::star::uno::Sequence< double >&              data,
                                       const ::com::sun::star::rendering::FloatingPointBitmapLayout& bitmapLayout,
                                       const ::com::sun::star::geometry::IntegerRectangle2D&         rect ) throw (::com::sun::star::lang::IllegalArgumentException,
                                                                                                                   ::com::sun::star::lang::IndexOutOfBoundsException,
                                                                                                                   ::com::sun::star::uno::RuntimeException)
        {
            verifyInput(bitmapLayout, rect, this);
            verifyIndexRange(rect, getSize() );

            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            BaseType::mbSurfaceDirty = true;
            BaseType::maCanvasHelper.modifying();

            BaseType::maCanvasHelper.setData( data, bitmapLayout, rect );
        }

        virtual void SAL_CALL setPixel( const ::com::sun::star::uno::Sequence< double >&                color,
                                        const ::com::sun::star::rendering::FloatingPointBitmapLayout&   bitmapLayout,
                                        const ::com::sun::star::geometry::IntegerPoint2D&               pos ) throw (::com::sun::star::lang::IllegalArgumentException,
                                                                                                                     ::com::sun::star::lang::IndexOutOfBoundsException,
                                                                                                                     ::com::sun::star::uno::RuntimeException)
        {
            verifyInput(bitmapLayout, pos, this);
            verifyIndexRange(pos, getSize() );

            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            BaseType::mbSurfaceDirty = true;
            BaseType::maCanvasHelper.modifying();

            BaseType::maCanvasHelper.setPixel( color, bitmapLayout, pos );
        }

        virtual ::com::sun::star::uno::Sequence< double > SAL_CALL getPixel( ::com::sun::star::rendering::FloatingPointBitmapLayout&    bitmapLayout,
                                                                             const ::com::sun::star::geometry::IntegerPoint2D&          pos ) throw (::com::sun::star::lang::IndexOutOfBoundsException,
                                                                                                                                                     ::com::sun::star::rendering::VolatileContentDestroyedException,
                                                                                                                                                     ::com::sun::star::uno::RuntimeException)
        {
            verifyInput(pos, this);
            verifyIndexRange(pos, getSize() );

            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            return BaseType::maCanvasHelper.getPixel( bitmapLayout,
                                                      pos );
        }

        virtual ::com::sun::star::rendering::FloatingPointBitmapLayout SAL_CALL getMemoryLayout(  ) throw (::com::sun::star::uno::RuntimeException)
        {
            typename BaseType::MutexType aGuard( BaseType::m_aMutex );

            return BaseType::maCanvasHelper.getMemoryLayout();
        }
    };
}

#endif /* INCLUDED_CANVAS_DOUBLEBITMAPBASE_HXX */
