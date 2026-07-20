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


#include <canvasgraphichelper.hxx>

#include <com/sun/star/rendering/XCanvas.hpp>

#include <canvas/canvastools.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <osl/diagnose.h>
#include <utility>


using namespace ::com::sun::star;

/* Implementation of CanvasGraphicHelper class */

namespace cppcanvas
{
        CanvasGraphicHelper::CanvasGraphicHelper( CanvasSharedPtr xParentCanvas ) :
            mpCanvas(std::move( xParentCanvas ))
        {
            OSL_ENSURE( mpCanvas && mpCanvas->getUNOCanvas().is(),
                        "CanvasGraphicHelper::CanvasGraphicHelper: no valid canvas" );

            ::canvastools::initRenderState( maRenderState );
        }

        void CanvasGraphicHelper::setTransformation( const ::basegfx::B2DHomMatrix& rMatrix )
        {
            ::canvastools::setRenderStateTransform( maRenderState, rMatrix );
        }

        const rendering::RenderState& CanvasGraphicHelper::getRenderState() const
        {
            return maRenderState;
        }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
