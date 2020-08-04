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

#include <com/sun/star/rendering/XCustomSprite.hpp>

#include <base/canvascustomspritehelper.hxx>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2isize.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include "dx_spritecanvas.hxx"
#include "dx_surfacebitmap.hxx"

namespace dxcanvas
{
    /* Definition of SpriteHelper class */

    /** Helper class for canvas sprites.

        This class implements all sprite-related functionality, like
        that available on the XSprite interface.
     */
    class SpriteHelper : public ::canvas::CanvasCustomSpriteHelper
    {
    public:
        /** Create sprite helper
         */
        SpriteHelper();

        /** Late-init the sprite helper

            @param rSpriteSize
            Size of the sprite

            @param rSpriteCanvas
            Sprite canvas this sprite is part of. Object stores
            ref-counted reference to it, thus, don't forget to pass on
            disposing()!

            @param rRenderModule
            rendermodule to use

            @param rSpriteSurface
            The surface of the sprite (not the DX texture, but the
            persistent target of content rendering)

            @param bShowSpriteBounds
            When true, little debug bound rects for sprites are shown
         */
        void init( const css::geometry::RealSize2D&                 rSpriteSize,
                   const SpriteCanvasRef&                           rSpriteCanvas,
                   const IDXRenderModuleSharedPtr&                  rRenderModule,
                   const DXSurfaceBitmapSharedPtr&                  rBitmap,
                   bool                                             bShowSpriteBounds );

        void disposing();

        /** Repaint sprite content via hardware to associated sprite
            canvas

            @param io_bSurfaceDirty
            Input/output parameter, whether the sprite content is
            dirty or not. If texture was updated, set to false

         */
        void redraw( bool& io_bSurfaceDirty ) const;

    private:
        virtual ::basegfx::B2DPolyPolygon polyPolygonFromXPolyPolygon2D(
            css::uno::Reference< css::rendering::XPolyPolygon2D >& xPoly ) const override;

        /// Returns true, if the sprite _really_ needs redraw
        bool needRedraw() const;

        SpriteCanvasRef             mpSpriteCanvas;

        DXSurfaceBitmapSharedPtr    mpBitmap;
        mutable bool                mbTextureDirty;  // when true, texture needs update
        bool                        mbShowSpriteBounds; // when true, debug bound rect for sprites is shown
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
