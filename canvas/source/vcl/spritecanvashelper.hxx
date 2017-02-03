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

#ifndef INCLUDED_CANVAS_SOURCE_VCL_SPRITECANVASHELPER_HXX
#define INCLUDED_CANVAS_SOURCE_VCL_SPRITECANVASHELPER_HXX

#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>

#include <vcl/vclptr.hxx>
#include <vcl/virdev.hxx>

#include <canvas/spriteredrawmanager.hxx>
#include <canvas/elapsedtime.hxx>
#include <canvas/vclwrapper.hxx>
#include "canvashelper.hxx"
#include "impltools.hxx"


namespace vclcanvas
{
    class SpriteCanvas;

    class SpriteCanvasHelper : public CanvasHelper
    {
    public:
        SpriteCanvasHelper();
        ~SpriteCanvasHelper();

        void init( const OutDevProviderSharedPtr& rOutDev,
                   SpriteCanvas&                  rOwningSpriteCanvas,
                   ::canvas::SpriteRedrawManager& rManager,
                   bool                           bProtect,
                   bool                           bHaveAlpha );

        /// Dispose all internal references
        void disposing();

        // XSpriteCanvas
        css::uno::Reference<
              css::rendering::XAnimatedSprite >        createSpriteFromAnimation(
                const css::uno::Reference< css::rendering::XAnimation >& animation );

        css::uno::Reference<
              css::rendering::XAnimatedSprite >        createSpriteFromBitmaps(
                const css::uno::Sequence<
                      css::uno::Reference<
                            css::rendering::XBitmap > >& animationBitmaps,
                sal_Int8                                              interpolationMode );

        css::uno::Reference<
              css::rendering::XCustomSprite >      createCustomSprite(
                const css::geometry::RealSize2D& spriteSize );

        css::uno::Reference<
              css::rendering::XSprite >                createClonedSprite(
                const css::uno::Reference< css::rendering::XSprite >& original );

        /** Actually perform the screen update

            @param bUpdateAll
            sal_True, if everything must be updated, not only changed
            sprites

            @param io_bSurfaceDirty
            In/out parameter, whether backbuffer surface is dirty (if
            yes, we're performing a full update, anyway)
         */
        bool updateScreen( bool bUpdateAll,
                           bool&    io_bSurfaceDirty );

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

        css::uno::Any isUnsafeScrolling() const
        {
            return css::uno::Any(mbIsUnsafeScrolling);
        }
        void enableUnsafeScrolling( const css::uno::Any& rAny )
        {
            mbIsUnsafeScrolling = rAny.get<bool>();
        }

        css::uno::Any isSpriteBounds() const
        {
            return css::uno::Any(mbShowSpriteBounds);
        }
        void enableSpriteBounds( const css::uno::Any& rAny )
        {
            mbShowSpriteBounds = rAny.get<bool>();
        }

    private:
        void renderFrameCounter( OutputDevice& rOutDev );
        void renderSpriteCount( OutputDevice& rOutDev );
        void renderMemUsage( OutputDevice& rOutDev );

        /// Set from the SpriteCanvas: instance coordinating sprite redraw
        ::canvas::SpriteRedrawManager*                  mpRedrawManager;

        /// Set from the init method. used to generate sprites
        SpriteCanvas*                                   mpOwningSpriteCanvas;

        /** Background compositing surface.

            Typically, sprites will be composited in the background,
            before pushing them to screen. This happens here.
         */
        VclPtr< VirtualDevice > maVDev;

        /// For the frame counter timings
        ::canvas::tools::ElapsedTime                    maLastUpdate;

        /// When true, canvas displays debug info on each frame
        bool                                            mbShowFrameInfo;

        /// When true, canvas creates all new sprites with red lines in the corners
        bool                                            mbShowSpriteBounds;

        /// When true, canvas uses the scroll optimization (direct scrolls in front buffer)
        bool                                            mbIsUnsafeScrolling;
    };
}

#endif // INCLUDED_CANVAS_SOURCE_VCL_SPRITECANVASHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
