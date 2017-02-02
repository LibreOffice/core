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

#ifndef INCLUDED_CPPCANVAS_SOURCE_INC_CANVASGRAPHICHELPER_HXX
#define INCLUDED_CPPCANVAS_SOURCE_INC_CANVASGRAPHICHELPER_HXX

#include <com/sun/star/rendering/RenderState.hpp>
#include <basegfx/polygon/b2dpolypolygon.hxx>

#include <cppcanvas/canvasgraphic.hxx>

#include <boost/optional.hpp>

namespace com { namespace sun { namespace star { namespace rendering
{
    class  XGraphicDevice;
} } } }


/* Definition of CanvasGraphicHelper class */

namespace cppcanvas
{

    namespace internal
    {

        class CanvasGraphicHelper : public virtual CanvasGraphic
        {
        public:
            CanvasGraphicHelper( const CanvasSharedPtr& rParentCanvas );

            // CanvasGraphic implementation
            virtual void                             setTransformation( const ::basegfx::B2DHomMatrix& rMatrix ) override;
            virtual void                             setClip( const ::basegfx::B2DPolyPolygon& rClipPoly ) override;
            virtual void                             setClip() override;
            virtual void                             setCompositeOp( sal_Int8 aOp ) override;

        protected:
            // for our clients
            // ===============
            const CanvasSharedPtr&                                               getCanvas() const {  return mpCanvas; }
            const css::uno::Reference< css::rendering::XGraphicDevice >&         getGraphicDevice() const {  return mxGraphicDevice; }
            const css::rendering::RenderState&                                   getRenderState() const;

        private:
            mutable css::rendering::RenderState                                   maRenderState;

            boost::optional<basegfx::B2DPolyPolygon>                              maClipPolyPolygon;
            CanvasSharedPtr                                                       mpCanvas;
            css::uno::Reference< css::rendering::XGraphicDevice >                 mxGraphicDevice;
        };

    }
}

#endif // INCLUDED_CPPCANVAS_SOURCE_INC_CANVASGRAPHICHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
