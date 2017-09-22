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

#include <sal/config.h>

#include <boost/cast.hpp>

#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <comphelper/scopeguard.hxx>
#include <tools/diagnose_ex.h>

#include <canvas/canvastools.hxx>

#include "dx_canvascustomsprite.hxx"
#include "dx_spritecanvashelper.hxx"

using namespace ::com::sun::star;

namespace dxcanvas
{
    namespace
    {
        void repaintBackground( const ::basegfx::B2DRange&      rUpdateArea,
                                const ::basegfx::B2IRange&      rOutputArea,
                                const DXSurfaceBitmapSharedPtr& rBackBuffer )
        {
            // TODO(E1): Use numeric_cast to catch overflow here
            ::basegfx::B2IRange aActualArea( 0, 0,
                                             static_cast<sal_Int32>(rOutputArea.getWidth()),
                                             static_cast<sal_Int32>(rOutputArea.getHeight()) );
            aActualArea.intersect( fround( rUpdateArea ) );

            // repaint the given area of the screen with background content
            rBackBuffer->draw(aActualArea);
        }

        void spriteRedraw( const ::canvas::Sprite::Reference& rSprite )
        {
            // downcast to derived dxcanvas::Sprite interface, which
            // provides the actual redraw methods.
            ::boost::polymorphic_downcast< Sprite* >(
                rSprite.get() )->redraw();
        }
    }

    SpriteCanvasHelper::SpriteCanvasHelper() :
        mpSpriteSurface( nullptr ),
        mpRedrawManager( nullptr ),
        mpRenderModule(),
        mpSurfaceProxy(),
        mpBackBuffer(),
        maUpdateRect(),
        maScrapRect(),
        mbShowSpriteBounds( false )
    {
#if OSL_DEBUG_LEVEL > 0
        // inverse default for verbose debug mode
        mbShowSpriteBounds = true;
#endif
    }

    void SpriteCanvasHelper::init( SpriteCanvas&                                    rParent,
                                   ::canvas::SpriteRedrawManager&                   rManager,
                                   const IDXRenderModuleSharedPtr&                  rRenderModule,
                                   const std::shared_ptr<canvas::ISurfaceProxyManager>&   rSurfaceProxy,
                                   const DXSurfaceBitmapSharedPtr&                  rBackBuffer,
                                   const ::basegfx::B2ISize&                        rOutputOffset )
    {
        // init base
        setDevice( rParent );
        setTarget( rBackBuffer, rOutputOffset );

        mpSpriteSurface = &rParent;
        mpRedrawManager = &rManager;
        mpRenderModule  = rRenderModule;
        mpSurfaceProxy  = rSurfaceProxy;
        mpBackBuffer    = rBackBuffer;
    }

    void SpriteCanvasHelper::disposing()
    {
        if(mpRenderModule)
            mpRenderModule->disposing();

        mpBackBuffer.reset();
        mpRenderModule.reset();
        mpRedrawManager = nullptr;
        mpSpriteSurface = nullptr;

        // forward to base
        CanvasHelper::disposing();
    }

    uno::Reference< rendering::XAnimatedSprite > SpriteCanvasHelper::createSpriteFromAnimation(
        const uno::Reference< rendering::XAnimation >& /*animation*/ )
    {
        return uno::Reference< rendering::XAnimatedSprite >();
    }

    uno::Reference< rendering::XAnimatedSprite > SpriteCanvasHelper::createSpriteFromBitmaps(
        const uno::Sequence< uno::Reference< rendering::XBitmap > >& /*animationBitmaps*/,
        sal_Int8                                                     /*interpolationMode*/ )
    {
        return uno::Reference< rendering::XAnimatedSprite >();
    }

    uno::Reference< rendering::XCustomSprite > SpriteCanvasHelper::createCustomSprite( const geometry::RealSize2D& spriteSize )
    {
        if( !mpRedrawManager )
            return uno::Reference< rendering::XCustomSprite >(); // we're disposed

        return uno::Reference< rendering::XCustomSprite >(
            new CanvasCustomSprite( spriteSize,
                                    mpSpriteSurface,
                                    mpRenderModule,
                                    mpSurfaceProxy,
                                    mbShowSpriteBounds ) );
    }

    uno::Reference< rendering::XSprite > SpriteCanvasHelper::createClonedSprite( const uno::Reference< rendering::XSprite >& /*original*/ )
    {
        return uno::Reference< rendering::XSprite >();
    }

    bool SpriteCanvasHelper::updateScreen( const ::basegfx::B2IRectangle& rCurrArea,
                                           bool                           bUpdateAll,
                                           bool&                          io_bSurfaceDirty )
    {
        if( !mpRedrawManager ||
            !mpRenderModule ||
            !mpBackBuffer )
        {
            return false; // disposed, or otherwise dysfunctional
        }

        // store current output area (need to tunnel that to the
        // background, scroll, opaque and general sprite repaint
        // routines)
        maScrapRect = rCurrArea;

        // clear area that needs to be blitted to screen beforehand
        maUpdateRect.reset();

        // TODO(P1): Might be worthwile to track areas of background
        // changes, too.

        // TODO(P2): Might be worthwhile to use page-flipping only if
        // a certain percentage of screen area has changed - and
        // compose directly to the front buffer otherwise.
        if( !bUpdateAll && !io_bSurfaceDirty )
        {
            // background has not changed, so we're free to optimize
            // repaint to areas where a sprite has changed

            // process each independent area of overlapping sprites
            // separately.
            mpRedrawManager->forEachSpriteArea( *this );

            // flip primary surface to screen
            // ==============================

            // perform buffer flipping
            mpRenderModule->flip( maUpdateRect,
                                  rCurrArea );
        }
        else
        {
            // limit update to parent window area (ignored for fullscreen)
            // TODO(E1): Use numeric_cast to catch overflow here
            const ::basegfx::B2IRectangle aUpdateArea( 0,0,
                                                       static_cast<sal_Int32>(rCurrArea.getWidth()),
                                                       static_cast<sal_Int32>(rCurrArea.getHeight()) );

            // background has changed, or called requested full
            // update, or we're performing double buffering via page
            // flipping, so we currently have no choice but repaint
            // everything

            // repaint the whole screen with background content
            mpBackBuffer->draw(aUpdateArea);

            // redraw sprites
            mpRedrawManager->forEachSprite( &spriteRedraw );

            // flip primary surface to screen
            // ==============================

            // perform buffer flipping
            mpRenderModule->flip( aUpdateArea,
                                  rCurrArea );
        }

        // change record vector must be cleared, for the next turn of
        // rendering and sprite changing
        mpRedrawManager->clearChangeRecords();

        io_bSurfaceDirty = false;

        return true;
    }

    void SpriteCanvasHelper::backgroundPaint( const ::basegfx::B2DRange& rUpdateRect )
    {
        ENSURE_OR_THROW( mpRenderModule &&
                          mpBackBuffer,
                          "SpriteCanvasHelper::backgroundPaint(): NULL device pointer " );

        repaintBackground( rUpdateRect,
                           maScrapRect,
                           mpBackBuffer );
    }

    void SpriteCanvasHelper::scrollUpdate( const ::basegfx::B2DRange&                       /*rMoveStart*/,
                                           const ::basegfx::B2DRange&                       rMoveEnd,
                                           const ::canvas::SpriteRedrawManager::UpdateArea& rUpdateArea )
    {
        ENSURE_OR_THROW( mpRenderModule &&
                          mpBackBuffer,
                          "SpriteCanvasHelper::scrollUpdate(): NULL device pointer " );

        // round rectangles to integer pixel. Note: have to be
        // extremely careful here, to avoid off-by-one errors for
        // the destination area: otherwise, the next scroll update
        // would copy pixel that are not supposed to be part of
        // the sprite.
        const ::basegfx::B2IRange& rDestRect(
            ::canvas::tools::spritePixelAreaFromB2DRange( rMoveEnd ) );

        // not much sense in really implementing scrollUpdate here,
        // since outputting a sprite only partially would result in
        // expensive clipping. Furthermore, we cannot currently render
        // 3D directly to the front buffer, thus, would have to blit
        // the full sprite area, anyway. But at least optimized in the
        // sense that unnecessary background paints behind the sprites
        // are avoided.
        for( const auto& rComponent : rUpdateArea.maComponentList )
        {
            const ::canvas::Sprite::Reference& rSprite( rComponent.second.getSprite() );

            if( rSprite.is() )
            {
                // downcast to derived dxcanvas::Sprite interface, which
                // provides the actual redraw methods.
                ::boost::polymorphic_downcast< Sprite* >( rSprite.get() )->redraw();
            }
        }

        // repaint uncovered areas from backbuffer - take the
        // _rounded_ rectangles from above, to have the update
        // consistent with the scroll above.
        std::vector< ::basegfx::B2DRange > aUncoveredAreas;
        ::basegfx::computeSetDifference( aUncoveredAreas,
                                         rUpdateArea.maTotalBounds,
                                         ::basegfx::B2DRange( rDestRect ) );
        for( const auto& rUncoveredArea : aUncoveredAreas )
            repaintBackground( rUncoveredArea, maScrapRect, mpBackBuffer );

        // TODO(E1): Use numeric_cast to catch overflow here
        ::basegfx::B2IRange aActualArea( 0, 0,
                                         static_cast<sal_Int32>(maScrapRect.getWidth()),
                                         static_cast<sal_Int32>(maScrapRect.getHeight()) );
        aActualArea.intersect( fround( rUpdateArea.maTotalBounds ) );

        // add given update area to the 'blit to foreground' rect
        maUpdateRect.expand( aActualArea );
    }

    void SpriteCanvasHelper::opaqueUpdate( const ::basegfx::B2DRange&                          rTotalArea,
                                           const std::vector< ::canvas::Sprite::Reference >& rSortedUpdateSprites )
    {
        ENSURE_OR_THROW( mpRenderModule &&
                          mpBackBuffer,
                          "SpriteCanvasHelper::opaqueUpdate(): NULL device pointer " );

        // TODO(P2): optimize this by truly rendering to the front
        // buffer. Currently, we've the 3D device only for the back
        // buffer.
        for( const auto& rSprite : rSortedUpdateSprites )
        {
            if( rSprite.is() )
            {
                // downcast to derived dxcanvas::Sprite interface, which
                // provides the actual redraw methods.
                ::boost::polymorphic_downcast< Sprite* >( rSprite.get() )->redraw();
            }
        }

        // TODO(E1): Use numeric_cast to catch overflow here
        ::basegfx::B2IRange aActualArea( 0, 0,
                                         static_cast<sal_Int32>(maScrapRect.getWidth()),
                                         static_cast<sal_Int32>(maScrapRect.getHeight()) );
        aActualArea.intersect( fround( rTotalArea ) );

        // add given update area to the 'blit to foreground' rect
        maUpdateRect.expand( aActualArea );
    }

    void SpriteCanvasHelper::genericUpdate( const ::basegfx::B2DRange&                          rTotalArea,
                                            const std::vector< ::canvas::Sprite::Reference >& rSortedUpdateSprites )
    {
        ENSURE_OR_THROW( mpRenderModule &&
                          mpBackBuffer,
                          "SpriteCanvasHelper::genericUpdate(): NULL device pointer " );

        // paint background
        // ================

        // TODO(E1): Use numeric_cast to catch overflow here
        ::basegfx::B2IRange aActualArea( 0, 0,
                                         static_cast<sal_Int32>(maScrapRect.getWidth()),
                                         static_cast<sal_Int32>(maScrapRect.getHeight()) );
        aActualArea.intersect( fround( rTotalArea ) );

        // repaint the given area of the screen with background content
        mpBackBuffer->draw(aActualArea);

        // paint sprite
        // ============

        for( const auto& rSprite : rSortedUpdateSprites )
        {
            if( rSprite.is() )
            {
                // downcast to derived dxcanvas::Sprite interface, which
                // provides the actual redraw methods.
                ::boost::polymorphic_downcast< Sprite* >( rSprite.get() )->redraw();
            }
        }

        // add given update area to the 'blit to foreground' rect
        maUpdateRect.expand( aActualArea );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
