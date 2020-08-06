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

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/rendering/ViewState.hpp>
#include <cppcanvas/canvas.hxx>

#include <optional>
#include <basegfx/polygon/b2dpolypolygon.hxx>


namespace basegfx
{
    class B2DHomMatrix;
    class B2DPolyPolygon;
}

namespace com::sun::star::rendering
{
    class  XCanvas;
}


/* Definition of ImplCanvas */

namespace cppcanvas::internal
{

        class ImplCanvas : public virtual Canvas
        {
        public:
            explicit ImplCanvas( const css::uno::Reference< css::rendering::XCanvas >& rCanvas );
            virtual ~ImplCanvas() override;

            ImplCanvas(ImplCanvas const &) = default;
            ImplCanvas(ImplCanvas &&) = default;
            ImplCanvas & operator =(ImplCanvas const &) = delete; // due to const mxCanvas
            ImplCanvas & operator =(ImplCanvas &&) = delete; // due to const mxCanvas

            virtual void                             setTransformation( const ::basegfx::B2DHomMatrix& rMatrix ) override;
            virtual ::basegfx::B2DHomMatrix          getTransformation() const override;

            virtual void                             setClip( const ::basegfx::B2DPolyPolygon& rClipPoly ) override;
            virtual void                             setClip() override;
            virtual ::basegfx::B2DPolyPolygon const* getClip() const override;

            virtual CanvasSharedPtr                  clone() const override;

            virtual void                             clear() const override;

            virtual css::uno::Reference<
                css::rendering::XCanvas >            getUNOCanvas() const override;

            virtual css::rendering::ViewState        getViewState() const override;

        private:
            mutable css::rendering::ViewState                    maViewState;
            std::optional<basegfx::B2DPolyPolygon>             maClipPolyPolygon;
            const css::uno::Reference< css::rendering::XCanvas > mxCanvas;
        };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
