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

#ifndef INCLUDED_CANVAS_CACHEDPRIMITIVEBASE_HXX
#define INCLUDED_CANVAS_CACHEDPRIMITIVEBASE_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XCachedPrimitive.hpp>
#include <com/sun/star/rendering/ViewState.hpp>
#include <cppuhelper/compbase2.hxx>
#include <comphelper/broadcasthelper.hxx>

#include <canvas/canvastoolsdllapi.h>

/* Definition of CachedPrimitiveBase class */

namespace canvas
{
    typedef ::cppu::WeakComponentImplHelper2< ::com::sun::star::rendering::XCachedPrimitive,
                                                   ::com::sun::star::lang::XServiceInfo > CachedPrimitiveBase_Base;

    /** Base class, providing common functionality for implementers of
        the XCachedPrimitive interface.
     */
    class CANVASTOOLS_DLLPUBLIC CachedPrimitiveBase : public CachedPrimitiveBase_Base,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
