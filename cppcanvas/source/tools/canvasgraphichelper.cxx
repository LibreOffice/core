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


#include <canvasgraphichelper.hxx>

#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>

#include <canvas/canvastools.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include <cppcanvas/polypolygon.hxx>
#include "tools.hxx"


using namespace ::com::sun::star;

/* Implementation of CanvasGraphicHelper class */

namespace cppcanvas
{

    namespace internal
    {
        CanvasGraphicHelper::CanvasGraphicHelper( const CanvasSharedPtr& rParentCanvas ) :
            maClipPolyPolygon(),
            mpCanvas( rParentCanvas ),
            mxGraphicDevice()
        {
            OSL_ENSURE( mpCanvas.get() != nullptr &&
                        mpCanvas->getUNOCanvas().is(),
                        "CanvasGraphicHelper::CanvasGraphicHelper: no valid canvas" );

            if( mpCanvas.get() != nullptr &&
                mpCanvas->getUNOCanvas().is() )
            {
                mxGraphicDevice = mpCanvas->getUNOCanvas()->getDevice();
            }

            ::canvas::tools::initRenderState( maRenderState );
        }

        void CanvasGraphicHelper::setTransformation( const ::basegfx::B2DHomMatrix& rMatrix )
        {
            ::canvas::tools::setRenderStateTransform( maRenderState, rMatrix );
        }

        void CanvasGraphicHelper::setClip( const ::basegfx::B2DPolyPolygon& rClipPoly )
        {
            // TODO(T3): not thread-safe. B2DPolyPolygon employs copy-on-write
            maClipPolyPolygon.reset( rClipPoly );
            maRenderState.Clip.clear();
        }

        void CanvasGraphicHelper::setClip()
        {
            maClipPolyPolygon.reset();
            maRenderState.Clip.clear();
        }

        const rendering::RenderState& CanvasGraphicHelper::getRenderState() const
        {
            if( maClipPolyPolygon && !maRenderState.Clip.is() )
            {
                uno::Reference< rendering::XCanvas > xCanvas( mpCanvas->getUNOCanvas() );
                if( !xCanvas.is() )
                    return maRenderState;

                maRenderState.Clip = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                    xCanvas->getDevice(),
                    *maClipPolyPolygon );
            }

            return maRenderState;
        }

        void CanvasGraphicHelper::setCompositeOp( sal_Int8 aOp )
        {
            maRenderState.CompositeOperation = aOp;
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
