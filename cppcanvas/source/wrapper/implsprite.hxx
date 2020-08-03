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
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/rendering/XSprite.hpp>
#include <cppcanvas/sprite.hxx>
#include "implspritecanvas.hxx"

namespace cppcanvas::internal
{
        class ImplSprite : public virtual Sprite
        {
        public:
            ImplSprite( const css::uno::Reference<
                                  css::rendering::XSpriteCanvas >&                    rParentCanvas,
                        const css::uno::Reference<
                                  css::rendering::XSprite >&                          rSprite,
                        const ImplSpriteCanvas::TransformationArbiterSharedPtr&       rTransformArbiter );
            virtual ~ImplSprite() override;

            virtual void setAlpha( const double& rAlpha ) override;
            virtual void movePixel( const ::basegfx::B2DPoint& rNewPos ) override;
            virtual void move( const ::basegfx::B2DPoint& rNewPos ) override;
            virtual void transform( const ::basegfx::B2DHomMatrix& rMatrix ) override;
            virtual void setClipPixel( const ::basegfx::B2DPolyPolygon& rClipPoly ) override;
            virtual void setClip( const ::basegfx::B2DPolyPolygon& rClipPoly ) override;
            virtual void setClip() override;

            virtual void show() override;
            virtual void hide() override;

            virtual void setPriority( double fPriority ) override;

        private:
            ImplSprite(const ImplSprite&) = delete;
            ImplSprite& operator=( const ImplSprite& ) = delete;

            css::uno::Reference< css::rendering::XGraphicDevice >         mxGraphicDevice;
            const css::uno::Reference< css::rendering::XSprite >          mxSprite;
            ImplSpriteCanvas::TransformationArbiterSharedPtr              mpTransformArbiter;
        };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
