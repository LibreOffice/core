/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <com/sun/star/lang/XServiceInfo.hpp>
#include "RenderState.hxx"
#include "XCachedPrimitive.hxx"
#include <comphelper/compbase.hxx>
#include <vcl/GraphicObject.hxx>
#include <memory>
#include "XCanvas.hxx"

/* Definition of CachedBitmap class */

namespace vclcanvas
{
    typedef std::shared_ptr< GraphicObject > GraphicObjectSharedPtr;

    typedef comphelper::WeakComponentImplHelper< vclcanvas::XCachedPrimitive,
                                           css::lang::XServiceInfo > CachedBitmap_Base;
    class CachedBitmap : public CachedBitmap_Base
    {
    public:

        /** Create an XCachedPrimitive for given GraphicObject
         */
        CachedBitmap( GraphicObjectSharedPtr                          xGraphicObject,
                      const ::Point&                                  rPoint,
                      const ::Size&                                   rSize,
                      const GraphicAttr&                              rAttr,
                      const ::vclcanvas::ViewState&                rUsedViewState,
                      ::vclcanvas::RenderState                     aUsedRenderState,
                      const css::uno::Reference< vclcanvas::XCanvas >&   rTarget );

        /// Dispose all internal references
        virtual void disposing(std::unique_lock<std::mutex>& rGuard) override;

        // XCachedPrimitive
        virtual ::sal_Int8 redraw( const ::vclcanvas::ViewState& aState ) override;

        // XServiceInfo
        virtual OUString getImplementationName(  ) override;
        virtual bool supportsService( const OUString& ServiceName ) override;
        virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;

    private:
        ::vclcanvas::ViewState                         maUsedViewState;
        css::uno::Reference< vclcanvas::XCanvas >                      mxTarget;
        GraphicObjectSharedPtr                                         mpGraphicObject;
        const ::vclcanvas::RenderState                              maRenderState;
        const ::Point                                                  maPoint;
        const ::Size                                                   maSize;
        const GraphicAttr                                              maAttributes;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
