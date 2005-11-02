/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bitmapcanvasbase.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:41:46 $
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

#ifndef INCLUDED_CANVAS_BITMAPCANVASBASE_HXX
#define INCLUDED_CANVAS_BITMAPCANVASBASE_HXX

#ifndef INCLUDED_CANVAS_CANVASBASE_HXX
#include <canvas/base/canvasbase.hxx>
#endif


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

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapCanvas > SAL_CALL queryBitmapCanvas(  ) throw (::com::sun::star::uno::RuntimeException)
        {
            return this;
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
