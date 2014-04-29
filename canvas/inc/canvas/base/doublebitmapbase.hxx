/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef INCLUDED_CANVAS_DOUBLEBITMAPBASE_HXX
#define INCLUDED_CANVAS_DOUBLEBITMAPBASE_HXX

#include <com/sun/star/rendering/XIeeeDoubleBitmap.hpp>
#include <canvas/bitmapcanvasbase.hxx>


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
        OBaseMutex-provided lock.  Every time one of the methods is
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
