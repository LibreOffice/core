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

#ifndef INCLUDED_CANVAS_BASE_CACHEDPRIMITIVEBASE_HXX
#define INCLUDED_CANVAS_BASE_CACHEDPRIMITIVEBASE_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/XCachedPrimitive.hpp>
#include <com/sun/star/rendering/ViewState.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include <canvas/canvastoolsdllapi.h>

namespace com::sun::star::rendering { class XCanvas; }

/* Definition of CachedPrimitiveBase class */

namespace canvas
{
    typedef cppu::WeakComponentImplHelper< css::rendering::XCachedPrimitive,
                                           css::lang::XServiceInfo > CachedPrimitiveBase_Base;

    /** Base class, providing common functionality for implementers of
        the XCachedPrimitive interface.
     */
    class CANVASTOOLS_DLLPUBLIC CachedPrimitiveBase:
        public cppu::BaseMutex, public CachedPrimitiveBase_Base
    {
    public:

        /** Create an XCachedPrimitive for given target canvas

            @param rUsedViewState
            The viewstate the original object was rendered with

            @param rTarget
            The target canvas the repaint should happen on.
         */
        CachedPrimitiveBase( const css::rendering::ViewState&  rUsedViewState,
                             const css::uno::Reference< css::rendering::XCanvas >& rTarget );

        /// Dispose all internal references
        virtual void SAL_CALL disposing() override;

        // XCachedPrimitive
        virtual ::sal_Int8 SAL_CALL redraw( const css::rendering::ViewState& aState ) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    protected:
        virtual ~CachedPrimitiveBase() override; // we're a ref-counted UNO class. _We_ destroy ourselves.

    private:
        CachedPrimitiveBase( const CachedPrimitiveBase& ) = delete;
        CachedPrimitiveBase& operator=( const CachedPrimitiveBase& ) = delete;

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
        virtual ::sal_Int8 doRedraw( const css::rendering::ViewState&  rNewState,
                                     const css::rendering::ViewState&  rOldState,
                                     const css::uno::Reference< css::rendering::XCanvas >&    rTargetCanvas,
                                     bool                              bSameViewTransform ) = 0;

        css::rendering::ViewState                         maUsedViewState;
        css::uno::Reference< css::rendering::XCanvas >    mxTarget;
    };
}

#endif // INCLUDED_CANVAS_BASE_CACHEDPRIMITIVEBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
