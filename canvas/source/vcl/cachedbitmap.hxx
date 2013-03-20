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

#pragma once
#if 1

#include <canvas/base/cachedprimitivebase.hxx>

#include <svtools/grfmgr.hxx>

#include <boost/shared_ptr.hpp>


/* Definition of CachedBitmap class */

namespace vclcanvas
{
    typedef ::boost::shared_ptr< GraphicObject > GraphicObjectSharedPtr;

    class CachedBitmap : public ::canvas::CachedPrimitiveBase
    {
    public:

        /** Create an XCachedPrimitive for given GraphicObject
         */
        CachedBitmap( const GraphicObjectSharedPtr&                   rGraphicObject,
                      const ::Point&                                  rPoint,
                      const ::Size&                                   rSize,
                      const GraphicAttr&                              rAttr,
                      const ::com::sun::star::rendering::ViewState&   rUsedViewState,
                      const ::com::sun::star::rendering::RenderState& rUsedRenderState,
                      const ::com::sun::star::uno::Reference<
                              ::com::sun::star::rendering::XCanvas >&   rTarget );

        /// Dispose all internal references
        virtual void SAL_CALL disposing();

    private:
        virtual ::sal_Int8 doRedraw( const ::com::sun::star::rendering::ViewState&  rNewState,
                                     const ::com::sun::star::rendering::ViewState&  rOldState,
                                     const ::com::sun::star::uno::Reference<
                                         ::com::sun::star::rendering::XCanvas >&    rTargetCanvas,
                                     bool                                           bSameViewTransform );


        GraphicObjectSharedPtr                                                      mpGraphicObject;
        const ::com::sun::star::rendering::RenderState                              maRenderState;
        const ::Point                                                               maPoint;
        const ::Size                                                                maSize;
        const GraphicAttr                                                           maAttributes;
    };
}

#endif /* _VCLCANVAS_CACHEDBITMAP_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
