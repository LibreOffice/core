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

#ifndef INCLUDED_CANVAS_SOURCE_CAIRO_CAIRO_SPRITECANVASHELPER_HXX
#define INCLUDED_CANVAS_SOURCE_CAIRO_CAIRO_SPRITECANVASHELPER_HXX

#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>

#include <canvas/spriteredrawmanager.hxx>

#include <vcl/cairo.hxx>
#include "cairo_canvashelper.hxx"

namespace basegfx
{
    class B2IRange;
}

namespace cairocanvas
{
    class SpriteCanvas;

    class SpriteCanvasHelper : public CanvasHelper
    {
    public:
        SpriteCanvasHelper();

        void init( ::canvas::SpriteRedrawManager& rManager,
                   SpriteCanvas&                  rOwningSpriteCanvas,
                   const ::basegfx::B2ISize&      rSize );

        /// Dispose all internal references
        void disposing();

        // XSpriteCanvas
        css::uno::Reference< css::rendering::XAnimatedSprite >        createSpriteFromAnimation(
                const css::uno::Reference< css::rendering::XAnimation >& animation );

        css::uno::Reference< css::rendering::XAnimatedSprite >        createSpriteFromBitmaps(
                const css::uno::Sequence<
                      css::uno::Reference<
                            css::rendering::XBitmap > >& animationBitmaps,
                sal_Int8                                              interpolationMode );

        css::uno::Reference< css::rendering::XCustomSprite >      createCustomSprite(
                const css::geometry::RealSize2D& spriteSize );

        css::uno::Reference< css::rendering::XSprite >                createClonedSprite(
                const css::uno::Reference< css::rendering::XSprite >& original );

        /** Actually perform the screen update

            @param rCurrArea
            Current window area in absolute screen coordinates

            @param bUpdateAll
            sal_True, if everything must be updated, not only changed
            sprites

            @param io_bSurfaceDirty
            In/out parameter, whether backbuffer surface is dirty (if
            yes, we're performing a full update, anyway)
         */
        bool updateScreen( const ::basegfx::B2IRange& rCurrArea,
                           bool                       bUpdateAll,
                           bool&                      io_bSurfaceDirty );


        // SpriteRedrawManager functor calls


        /** Gets called for simple background repaints
         */
        void backgroundPaint( const ::basegfx::B2DRange& rUpdateRect );

        /** Gets called when area can be handled by scrolling.

            Called method must copy screen content from rMoveStart to
            rMoveEnd, and restore the background in the uncovered
            areas.

            @param rMoveStart
            Source rect of the scroll

            @param rMoveEnd
            Dest rect of the scroll

            @param rUpdateArea
            All info necessary, should rMoveStart be partially or
            fully outside the outdev
         */
        void scrollUpdate( const ::basegfx::B2DRange&                           rMoveStart,
                           const ::basegfx::B2DRange&                           rMoveEnd,
                           const ::canvas::SpriteRedrawManager::UpdateArea&     rUpdateArea );

        void opaqueUpdate( const ::basegfx::B2DRange&                          rTotalArea,
                           const ::std::vector< ::canvas::Sprite::Reference >& rSortedUpdateSprites );

        void genericUpdate( const ::basegfx::B2DRange&                          rTotalArea,
                            const ::std::vector< ::canvas::Sprite::Reference >& rSortedUpdateSprites );

    private:
        ::cairo::SurfaceSharedPtr const & getCompositingSurface( const ::basegfx::B2ISize& rNeededSize );
        ::cairo::SurfaceSharedPtr const & getTemporarySurface();
        ::cairo::SurfaceSharedPtr createSurface( const ::basegfx::B2ISize& rNeededSize ) const;

        /// Set from the SpriteCanvas: instance coordinating sprite redraw
        ::canvas::SpriteRedrawManager*  mpRedrawManager;

        /// Set from the init method. used to generate sprites
        SpriteCanvas*                   mpOwningSpriteCanvas;

        /// a surface used to composite the frontbuffer image
        ::cairo::SurfaceSharedPtr       mpCompositingSurface;
        ::basegfx::B2ISize              maCompositingSurfaceSize;
        bool                            mbCompositingSurfaceDirty;
        /// a temporary surface that is guaranteed to be the same size
        //as the compositing surface
        ::cairo::SurfaceSharedPtr       mpTemporarySurface;
    };
}

#endif // INCLUDED_CANVAS_SOURCE_CAIRO_CAIRO_SPRITECANVASHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
