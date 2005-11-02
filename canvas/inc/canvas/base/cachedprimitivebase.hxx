/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cachedprimitivebase.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:41:56 $
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

#ifndef INCLUDED_CANVAS_CACHEDPRIMITIVEBASE_HXX
#define INCLUDED_CANVAS_CACHEDPRIMITIVEBASE_HXX

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


/* Definition of CachedPrimitiveBase class */

namespace canvas
{
    typedef ::cppu::WeakComponentImplHelper2< ::com::sun::star::rendering::XCachedPrimitive,
                                                   ::com::sun::star::lang::XServiceInfo > CachedPrimitiveBase_Base;

    /** Base class, providing common functionality for implementers of
        the XCachedPrimitive interface.
     */
    class CachedPrimitiveBase : public CachedPrimitiveBase_Base,
                                public ::comphelper::OBaseMutex
    {
    public:

        /** Create an XCachedPrimitive for given target canvas

            @param rUsedViewState
            The viewstate the original object was rendered with

            @param rTarget
            The target canvas the repaint should happen on.

            @param bFailForChangedViewTransform
            When true, derived classes will never receive doRedraw()
            calls with dissimilar view transformations and
            bSameViewTransform set to false. This is useful for cached
            objects where re-transforming the generated output is not
            desirable, e.g. for hinted font output.
         */
        CachedPrimitiveBase( const ::com::sun::star::rendering::ViewState&  rUsedViewState,
                             const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::rendering::XCanvas >& rTarget,
                             bool                                           bFailForChangedViewTransform );

        /// Dispose all internal references
        virtual void SAL_CALL disposing();

        // XCachedPrimitive
        virtual ::sal_Int8 SAL_CALL redraw( const ::com::sun::star::rendering::ViewState& aState ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        ~CachedPrimitiveBase(); // we're a ref-counted UNO class. _We_ destroy ourselves.

    private:
        CachedPrimitiveBase( const CachedPrimitiveBase& );
        CachedPrimitiveBase& operator=( const CachedPrimitiveBase& );

        /** Actually perform the requested redraw.

            Clients must override this method, instead of the public
            redraw() one.

            @param rNewState
            The viewstate to redraw with

            @param rOldState
            The viewstate this cache object was created with.

            @param rTargetCanvas
            Target canvas to render to.

            @param bSameViewTransform
            When true, rNewState and rOldState have the same transformation.
         */
        virtual ::sal_Int8 doRedraw( const ::com::sun::star::rendering::ViewState&  rNewState,
                                     const ::com::sun::star::rendering::ViewState&  rOldState,
                                     const ::com::sun::star::uno::Reference<
                                         ::com::sun::star::rendering::XCanvas >&    rTargetCanvas,
                                     bool                                           bSameViewTransform ) = 0;

        ::com::sun::star::rendering::ViewState                                      maUsedViewState;
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvas >    mxTarget;
        const bool                                                                  mbFailForChangedViewTransform;
    };
}

#endif /* INCLUDED_CANVAS_CACHEDPRIMITIVEBASE_HXX */
