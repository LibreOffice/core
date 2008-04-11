/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cachedprimitivebase.hxx,v $
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

#ifndef INCLUDED_CANVAS_CACHEDPRIMITIVEBASE_HXX
#define INCLUDED_CANVAS_CACHEDPRIMITIVEBASE_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XCachedPrimitive.hpp>
#include <com/sun/star/rendering/ViewState.hpp>
#include <cppuhelper/compbase2.hxx>
#include <comphelper/broadcasthelper.hxx>


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
