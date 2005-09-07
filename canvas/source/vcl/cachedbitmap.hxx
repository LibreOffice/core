/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cachedbitmap.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 23:16:58 $
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

#ifndef _VCLCANVAS_CACHEDBITMAP_HXX
#define _VCLCANVAS_CACHEDBITMAP_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XCANVAS_HPP_
#include <com/sun/star/rendering/XCanvas.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XCACHEDPRIMITIVE_HPP_
#include <com/sun/star/rendering/XCachedPrimitive.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_VIEWSTATE_HPP__
#include <com/sun/star/rendering/ViewState.hpp>
#endif

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif

#ifndef _GRFMGR_HXX
#include <goodies/grfmgr.hxx>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif


/* Definition of CachedBitmap class */

namespace vclcanvas
{
    typedef ::boost::shared_ptr< GraphicObject > GraphicObjectSharedPtr;

    typedef ::cppu::WeakComponentImplHelper2< ::com::sun::star::rendering::XCachedPrimitive,
                                                   ::com::sun::star::lang::XServiceInfo > CachedBitmap_Base;

    class CachedBitmap : public ::comphelper::OBaseMutex, public CachedBitmap_Base
    {
    public:

        /** Create an XCachedPrimitive for given GraphicObject
         */
        CachedBitmap( const GraphicObjectSharedPtr&                 rGraphicObject,
                      const ::Point&                                rPoint,
                      const ::Size&                                 rSize,
                      const GraphicAttr&                            rAttr,
                      const ::com::sun::star::rendering::ViewState& rUsedViewState,
                      const ::com::sun::star::uno::Reference<
                              ::com::sun::star::rendering::XCanvas >& rTarget   );

        /// Dispose all internal references
        virtual void SAL_CALL disposing();

        // XCachedPrimitive
        virtual ::sal_Int8 SAL_CALL redraw( const ::com::sun::star::rendering::ViewState& aState ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        ~CachedBitmap(); // we're a ref-counted UNO class. _We_ destroy ourselves.

    private:
        // default: disabled copy/assignment
        CachedBitmap(const CachedBitmap&);
        CachedBitmap& operator=( const CachedBitmap& );

        GraphicObjectSharedPtr                                                      mpGraphicObject;
        const ::Point                                                               maPoint;
        const ::Size                                                                maSize;
        const GraphicAttr                                                           maAttributes;
        ::com::sun::star::rendering::ViewState                                      maUsedViewState;
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvas >    mxTarget;
    };
}

#endif /* _VCLCANVAS_CACHEDBITMAP_HXX */
