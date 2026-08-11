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
#include "ViewState.hxx"
#include <salhelper/simplereferenceobject.hxx>
#include <vcl/GraphicObject.hxx>
#include <rtl/ref.hxx>
#include <memory>

/* Definition of CachedBitmap class */

namespace vclcanvas
{
    class Canvas;

    typedef std::shared_ptr< GraphicObject > GraphicObjectSharedPtr;

    class CachedBitmap : public salhelper::SimpleReferenceObject
    {
    public:

        /** Create an CachedBitmap for given GraphicObject
         */
        CachedBitmap( GraphicObjectSharedPtr                          xGraphicObject,
                      const ::Point&                                  rPoint,
                      const ::Size&                                   rSize,
                      const GraphicAttr&                              rAttr,
                      const ::vclcanvas::ViewState&                rUsedViewState,
                      ::vclcanvas::RenderState                     aUsedRenderState,
                      vclcanvas::Canvas&                           rTarget );

        ::sal_Int8 redraw( const ::vclcanvas::ViewState& aState );

    private:
        ::vclcanvas::ViewState                         maUsedViewState;
        vclcanvas::Canvas&                                             mrTarget;
        GraphicObjectSharedPtr                                         mpGraphicObject;
        const ::vclcanvas::RenderState                              maRenderState;
        const ::Point                                                  maPoint;
        const ::Size                                                   maSize;
        const GraphicAttr                                              maAttributes;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
