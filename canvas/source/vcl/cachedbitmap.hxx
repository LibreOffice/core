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

#ifndef INCLUDED_CANVAS_SOURCE_VCL_CACHEDBITMAP_HXX
#define INCLUDED_CANVAS_SOURCE_VCL_CACHEDBITMAP_HXX

#include <com/sun/star/rendering/RenderState.hpp>
#include <canvas/base/cachedprimitivebase.hxx>
#include <vcl/GraphicObject.hxx>
#include <memory>


/* Definition of CachedBitmap class */

namespace vclcanvas
{
    typedef std::shared_ptr< GraphicObject > GraphicObjectSharedPtr;

    class CachedBitmap : public ::canvas::CachedPrimitiveBase
    {
    public:

        /** Create an XCachedPrimitive for given GraphicObject
         */
        CachedBitmap( const GraphicObjectSharedPtr&                   rGraphicObject,
                      const ::Point&                                  rPoint,
                      const ::Size&                                   rSize,
                      const GraphicAttr&                              rAttr,
                      const css::rendering::ViewState&                rUsedViewState,
                      const css::rendering::RenderState&              rUsedRenderState,
                      const css::uno::Reference< css::rendering::XCanvas >&   rTarget );

        /// Dispose all internal references
        virtual void SAL_CALL disposing() override;

    private:
        virtual ::sal_Int8 doRedraw( const css::rendering::ViewState&  rNewState,
                                     const css::rendering::ViewState&  rOldState,
                                     const css::uno::Reference< css::rendering::XCanvas >&    rTargetCanvas,
                                     bool                              bSameViewTransform ) override;


        GraphicObjectSharedPtr                                         mpGraphicObject;
        const css::rendering::RenderState                              maRenderState;
        const ::Point                                                  maPoint;
        const ::Size                                                   maSize;
        const GraphicAttr                                              maAttributes;
    };
}

#endif // INCLUDED_CANVAS_SOURCE_VCL_CACHEDBITMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
