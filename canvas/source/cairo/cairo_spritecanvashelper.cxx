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
#include <sal/log.hxx>

#include <boost/cast.hpp>

#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <comphelper/scopeguard.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/canvastools.hxx>

#include <canvas/canvastools.hxx>

#include "cairo_canvascustomsprite.hxx"
#include "cairo_spritecanvashelper.hxx"

using namespace ::cairo;
using namespace ::com::sun::star;

namespace cairocanvas
{
    namespace
    {
        /** Sprite redraw at original position

            Used to repaint the whole canvas (background and all
            sprites)
         */
        void spriteRedraw( const CairoSharedPtr& pCairo,
                           const ::canvas::Sprite::Reference& rSprite )
        {
            // downcast to derived cairocanvas::Sprite interface, which
            // provides the actual redraw methods.
            ::boost::polymorphic_downcast< Sprite* >(rSprite.get())->redraw( pCairo, true);
        }

        void repaintBackground( const CairoSharedPtr&      pCairo,
                                const SurfaceSharedPtr&    pBackgroundSurface,
                                const ::basegfx::B2DRange& rArea )
        {
            cairo_save( pCairo.get() );
            cairo_rectangle( pCairo.get(), ceil( rArea.getMinX() ), ceil( rArea.getMinY() ),
                             floor( rArea.getWidth() ), floor( rArea.getHeight() ) );
            cairo_clip( pCairo.get() );
            cairo_set_source_surface( pCairo.get(), pBackgroundSurface->getCairoSurface().get(), 0, 0 );
            cairo_set_operator( pCairo.get(), CAIRO_OPERATOR_SOURCE );
            cairo_paint( pCairo.get() );
            cairo_restore( pCairo.get() );
        }

        void opaqueUpdateSpriteArea( const ::canvas::Sprite::Reference& rSprite,
                                     const CairoSharedPtr&              pCairo,
                                     const ::basegfx::B2IRange&         rArea )
        {
            // clip output to actual update region (otherwise a)
            // wouldn't save much render time, and b) will clutter
            // scrolled sprite content outside this area)
            cairo_save( pCairo.get() );
            cairo_rectangle( pCairo.get(), rArea.getMinX(), rArea.getMinY(),
                             sal::static_int_cast<sal_Int32>(rArea.getWidth()),
                             sal::static_int_cast<sal_Int32>(rArea.getHeight()) );
            cairo_clip( pCairo.get() );

            // repaint affected sprite directly to output device (at
            // the actual screen output position)
            // rendering directly to device buffer
            ::boost::polymorphic_downcast< Sprite* >( rSprite.get() )->redraw( pCairo, false );

            cairo_restore( pCairo.get() );
        }
    }

    SpriteCanvasHelper::SpriteCanvasHelper() :
        mpRedrawManager( nullptr ),
        mpOwningSpriteCanvas( nullptr ),
        mpCompositingSurface(),
        maCompositingSurfaceSize(),
        mbCompositingSurfaceDirty(true)
    {
    }

    void SpriteCanvasHelper::init( ::canvas::SpriteRedrawManager& rManager,
                                   SpriteCanvas&                  rDevice,
                                   const ::basegfx::B2ISize&      rSize )
    {
        mpRedrawManager = &rManager;
        mpOwningSpriteCanvas = &rDevice;

        CanvasHelper::init( rSize, rDevice, &rDevice );
    }

    void SpriteCanvasHelper::disposing()
    {
        mpCompositingSurface.reset();
        mpOwningSpriteCanvas = nullptr;
        mpRedrawManager = nullptr;

        // forward to base
        CanvasHelper::disposing();
    }

    uno::Reference< rendering::XAnimatedSprite > SpriteCanvasHelper::createSpriteFromAnimation(
        const uno::Reference< rendering::XAnimation >& )
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
                                    mpOwningSpriteCanvas ) );
    }

    uno::Reference< rendering::XSprite > SpriteCanvasHelper::createClonedSprite(
        const uno::Reference< rendering::XSprite >& )
    {
        return uno::Reference< rendering::XSprite >();
    }

    bool SpriteCanvasHelper::updateScreen( const ::basegfx::B2IRange& /*rCurrArea*/,
                                                 bool                   bUpdateAll,
                                                 bool&                  io_bSurfaceDirty )
    {
        if( !mpRedrawManager ||
            !mpOwningSpriteCanvas ||
            !mpOwningSpriteCanvas->getWindowSurface() ||
            !mpOwningSpriteCanvas->getBufferSurface() )
        {
            return false; // disposed, or otherwise dysfunctional
        }

        SAL_INFO("canvas.cairo", "SpriteCanvasHelper::updateScreen called");

        const ::basegfx::B2ISize& rSize = mpOwningSpriteCanvas->getSizePixel();

        // force compositing surface to be available before using it
        // inside forEachSpriteArea
        SurfaceSharedPtr pCompositingSurface = getCompositingSurface(rSize);
        SurfaceSharedPtr pWindowSurface = mpOwningSpriteCanvas->getWindowSurface();
        CairoSharedPtr pCompositingCairo = pCompositingSurface->getCairo();
        CairoSharedPtr pWindowCairo = pWindowSurface->getCairo();

        // TODO(P1): Might be worthwhile to track areas of background
        // changes, too.
        if( !bUpdateAll && !io_bSurfaceDirty && !mbCompositingSurfaceDirty )
        {
            // background has not changed, so we're free to optimize
            // repaint to areas where a sprite has changed

            // process each independent area of overlapping sprites
            // separately.
            mpRedrawManager->forEachSpriteArea( *this );
        }
        else
        {
            SAL_INFO("canvas.cairo", "SpriteCanvasHelper::updateScreen update ALL");

            // background has changed, so we currently have no choice
            // but repaint everything (or caller requested that)

            cairo_rectangle( pCompositingCairo.get(), 0, 0, rSize.getX(), rSize.getY() );
            cairo_clip( pCompositingCairo.get() );
            cairo_save( pCompositingCairo.get() );
            cairo_set_source_surface( pCompositingCairo.get(),
                                      mpOwningSpriteCanvas->getBufferSurface()->getCairoSurface().get(),
                                      0, 0 );
            cairo_set_operator( pCompositingCairo.get(), CAIRO_OPERATOR_SOURCE );
            cairo_paint( pCompositingCairo.get() );
            cairo_restore( pCompositingCairo.get() );

            // repaint all active sprites on top of background into
            // VDev.
            mpRedrawManager->forEachSprite(
                    [&pCompositingCairo]( const Sprite::Reference rSprite )
                    { spriteRedraw( pCompositingCairo, rSprite ); }
                    );

            // flush to screen
            cairo_rectangle( pWindowCairo.get(), 0, 0, rSize.getX(), rSize.getY() );
            cairo_clip( pWindowCairo.get() );
            cairo_set_source_surface( pWindowCairo.get(),
                                      pCompositingSurface->getCairoSurface().get(),
                                      0, 0 );
            cairo_set_operator( pWindowCairo.get(), CAIRO_OPERATOR_SOURCE );
            cairo_paint( pWindowCairo.get() );
        }

        // change record vector must be cleared, for the next turn of
        // rendering and sprite changing
        mpRedrawManager->clearChangeRecords();

        mbCompositingSurfaceDirty = false;
        io_bSurfaceDirty = false;

        // commit to screen
        mpOwningSpriteCanvas->flush();

        return true;
    }

    void SpriteCanvasHelper::backgroundPaint( const ::basegfx::B2DRange& rUpdateRect )
    {
        if( mpOwningSpriteCanvas && mpCompositingSurface )
            repaintBackground( mpCompositingSurface->getCairo(),
                               mpOwningSpriteCanvas->getBufferSurface(),
                               rUpdateRect );
    }

    void SpriteCanvasHelper::scrollUpdate( const ::basegfx::B2DRange&                       rMoveStart,
                                           const ::basegfx::B2DRange&                       rMoveEnd,
                                           const ::canvas::SpriteRedrawManager::UpdateArea& rUpdateArea )
    {
        ENSURE_OR_THROW( mpOwningSpriteCanvas &&
                          mpOwningSpriteCanvas->getBufferSurface(),
                          "SpriteCanvasHelper::scrollUpdate(): NULL device pointer " );

        SAL_INFO("canvas.cairo", "SpriteCanvasHelper::scrollUpdate called");

        const ::basegfx::B2ISize& rSize = mpOwningSpriteCanvas->getSizePixel();
        const ::basegfx::B2IRange  aOutputBounds( 0,0,
                                                  rSize.getX(),
                                                  rSize.getY() );

        SurfaceSharedPtr pCompositingSurface = getCompositingSurface(rSize);
        SurfaceSharedPtr pWindowSurface = mpOwningSpriteCanvas->getWindowSurface();
        CairoSharedPtr pCompositingCairo = pCompositingSurface->getCairo();
        CairoSharedPtr pWindowCairo = pWindowSurface->getCairo();

        // round rectangles to integer pixel. Note: have to be
        // extremely careful here, to avoid off-by-one errors for
        // the destination area: otherwise, the next scroll update
        // would copy pixel that are not supposed to be part of
        // the sprite.
        ::basegfx::B2IRange aSourceRect(
            ::canvas::tools::spritePixelAreaFromB2DRange( rMoveStart ) );
        const ::basegfx::B2IRange& rDestRect(
            ::canvas::tools::spritePixelAreaFromB2DRange( rMoveEnd ) );
        ::basegfx::B2IPoint aDestPos( rDestRect.getMinimum() );

        std::vector< ::basegfx::B2IRange > aUnscrollableAreas;

        // TODO(E3): This is plain buggy (but copies the behaviour of
        // the old Impress slideshow) - the scrolled area might
        // actually lie _below_ another window!

        // clip to output bounds (cannot properly scroll stuff
        // _outside_ our screen area)
        if( !::canvas::tools::clipScrollArea( aSourceRect,
                                              aDestPos,
                                              aUnscrollableAreas,
                                              aOutputBounds ) )
        {
            // fully clipped scroll area: cannot simply scroll
            // then. Perform normal opaque update (can use that, since
            // one of the preconditions for scrollable update is
            // opaque sprite content)

            // repaint all affected sprites directly to output device
            for( const auto& rComponent : rUpdateArea.maComponentList )
            {
                const ::canvas::Sprite::Reference& rSprite( rComponent.second.getSprite() );
                if( rSprite.is() )
                    ::boost::polymorphic_downcast< Sprite* >( rSprite.get() )->redraw(
                        pCompositingCairo, true );
            }
        }
        else
        {
            const ::basegfx::B2IVector aSourceUpperLeftPos( aSourceRect.getMinimum() );

            // clip dest area (which must be inside rDestBounds)
            ::basegfx::B2IRange aDestRect( rDestRect );
            aDestRect.intersect( aOutputBounds );

            ::basegfx::B2ISize aScrollSize( aDestRect.getWidth(), aDestRect.getHeight() );
            SurfaceSharedPtr pScrollSurface( getTemporarySurface() );
            CairoSharedPtr pScrollCairo( pScrollSurface->getCairo() );

            cairo_save( pScrollCairo.get() );
            // scroll the current content of the compositing surface (and,
            // thus, of the window) in temp. surface
            cairo_set_source_surface( pScrollCairo.get(),
                                      pCompositingSurface->getCairoSurface().get(),
                                      aDestPos.getX() - aSourceUpperLeftPos.getX(),
                                      aDestPos.getY() - aSourceUpperLeftPos.getY() );
            cairo_rectangle( pScrollCairo.get(),
                    aDestPos.getX(), aDestPos.getY(),
                    aScrollSize.getX(), aScrollSize.getY() );
            cairo_clip( pScrollCairo.get() );
            cairo_set_operator( pScrollCairo.get(), CAIRO_OPERATOR_SOURCE );
            cairo_paint( pScrollCairo.get() );
            cairo_restore( pScrollCairo.get() );

            cairo_save( pCompositingCairo.get() );
            // copy the scrolled area back onto the compositing surface
            cairo_set_source_surface( pCompositingCairo.get(),
                                      pScrollSurface->getCairoSurface().get(),
                                      0, 0 );
            cairo_rectangle( pCompositingCairo.get(),
                             aDestPos.getX(), aDestPos.getY(),
                             aScrollSize.getX(), aScrollSize.getY() );
            cairo_clip( pCompositingCairo.get() );
            cairo_set_operator( pCompositingCairo.get(), CAIRO_OPERATOR_SOURCE );
            cairo_paint( pCompositingCairo.get() );
            cairo_restore( pCompositingCairo.get() );

            const ::canvas::SpriteRedrawManager::SpriteConnectedRanges::ComponentListType::const_iterator
                aFirst( rUpdateArea.maComponentList.begin() );
            ::canvas::SpriteRedrawManager::SpriteConnectedRanges::ComponentListType::const_iterator
                  aSecond( aFirst );
            ++aSecond;

            ENSURE_OR_THROW( aFirst->second.getSprite().is(),
                             "VCLCanvas::scrollUpdate(): no sprite" );

            // repaint uncovered areas from sprite. Need to actually
            // clip here, since we're only repainting _parts_ of the
            // sprite
            for( const auto& rArea : aUnscrollableAreas )
                opaqueUpdateSpriteArea( aFirst->second.getSprite(),
                                        pCompositingCairo, rArea );
        }

        // repaint uncovered areas from backbuffer - take the
        // _rounded_ rectangles from above, to have the update
        // consistent with the scroll above.
        std::vector< ::basegfx::B2DRange > aUncoveredAreas;
        ::basegfx::computeSetDifference( aUncoveredAreas,
                                         rUpdateArea.maTotalBounds,
                                         ::basegfx::B2DRange( rDestRect ) );
        for( const auto& rArea : aUncoveredAreas )
            repaintBackground( pCompositingCairo,
                               mpOwningSpriteCanvas->getBufferSurface(), rArea );

        cairo_rectangle( pWindowCairo.get(), 0, 0, rSize.getX(), rSize.getY() );
        cairo_clip( pWindowCairo.get() );
        cairo_set_source_surface( pWindowCairo.get(),
                                  pCompositingSurface->getCairoSurface().get(),
                                  0, 0 );
        cairo_set_operator( pWindowCairo.get(), CAIRO_OPERATOR_SOURCE );
        cairo_paint( pWindowCairo.get() );
    }

    void SpriteCanvasHelper::opaqueUpdate( const ::basegfx::B2DRange&                          rTotalArea,
                                           const std::vector< ::canvas::Sprite::Reference >& rSortedUpdateSprites )
    {
        ENSURE_OR_THROW( mpOwningSpriteCanvas &&
                          mpOwningSpriteCanvas->getBufferSurface(),
                          "SpriteCanvasHelper::opaqueUpdate(): NULL device pointer " );

        SAL_INFO("canvas.cairo", "SpriteCanvasHelper::opaqueUpdate called");

        const ::basegfx::B2ISize& rDeviceSize = mpOwningSpriteCanvas->getSizePixel();

        SurfaceSharedPtr pCompositingSurface = getCompositingSurface(rDeviceSize);
        SurfaceSharedPtr pWindowSurface = mpOwningSpriteCanvas->getWindowSurface();
        CairoSharedPtr pCompositingCairo = pCompositingSurface->getCairo();
        CairoSharedPtr pWindowCairo = pWindowSurface->getCairo();

        cairo_rectangle( pCompositingCairo.get(), 0, 0, rDeviceSize.getX(), rDeviceSize.getY() );
        cairo_clip( pCompositingCairo.get() );

        ::basegfx::B2DVector aPos( ceil( rTotalArea.getMinX() ), ceil( rTotalArea.getMinY() ) );
        ::basegfx::B2DVector aSize( floor( rTotalArea.getMaxX() - aPos.getX() ), floor( rTotalArea.getMaxY() - aPos.getY() ) );

        cairo_rectangle( pCompositingCairo.get(), aPos.getX(), aPos.getY(), aSize.getX(), aSize.getY() );
        cairo_clip( pCompositingCairo.get() );

        // repaint all affected sprites directly to output device
        for( const auto& rSprite : rSortedUpdateSprites )
        {
            if( rSprite.is() )
                ::boost::polymorphic_downcast< Sprite* >( rSprite.get() )->redraw(
                    pCompositingCairo, false );
        }

        // flush to screen
        cairo_rectangle( pWindowCairo.get(), 0, 0, rDeviceSize.getX(), rDeviceSize.getY() );
        cairo_clip( pWindowCairo.get() );
        cairo_rectangle( pWindowCairo.get(), aPos.getX(), aPos.getY(), aSize.getX(), aSize.getY() );
        cairo_clip( pWindowCairo.get() );
        cairo_set_source_surface( pWindowCairo.get(),
                                  pCompositingSurface->getCairoSurface().get(),
                                  0, 0 );
        cairo_set_operator( pWindowCairo.get(), CAIRO_OPERATOR_SOURCE );
        cairo_paint( pWindowCairo.get() );
    }

    void SpriteCanvasHelper::genericUpdate( const ::basegfx::B2DRange&                          rRequestedArea,
                                            const std::vector< ::canvas::Sprite::Reference >& rSortedUpdateSprites )
    {
        // TODO
        SAL_INFO("canvas.cairo", "SpriteCanvasHelper::genericUpdate called");

        ENSURE_OR_THROW( mpOwningSpriteCanvas &&
                         mpOwningSpriteCanvas->getBufferSurface(),
                         "SpriteCanvasHelper::genericUpdate(): NULL device pointer " );

        // limit size of update VDev to target outdev's size
        const ::basegfx::B2ISize& rSize = mpOwningSpriteCanvas->getSizePixel();

        SurfaceSharedPtr pCompositingSurface = getCompositingSurface(rSize);
        SurfaceSharedPtr pWindowSurface = mpOwningSpriteCanvas->getWindowSurface();
        CairoSharedPtr pCompositingCairo = pCompositingSurface->getCairo();
        CairoSharedPtr pWindowCairo = pWindowSurface->getCairo();

        // round output position towards zero. Don't want to truncate
        // a fraction of a sprite pixel...  Clip position at origin,
        // otherwise, truncation of size below might leave visible
        // areas uncovered by VDev.
        const Point aOutputPosition(
            std::max( sal_Int32( 0 ),
                        static_cast< sal_Int32 >(rRequestedArea.getMinX()) ),
            std::max( sal_Int32( 0 ),
                        static_cast< sal_Int32 >(rRequestedArea.getMinY()) ) );
        // round output size towards +infty. Don't want to truncate a
        // fraction of a sprite pixel... Limit size of VDev to output
        // device's area.
        const Size  aOutputSize(
            std::min( rSize.getX(),
                        ::canvas::tools::roundUp( rRequestedArea.getMaxX() - aOutputPosition.X()) ),
            std::min( rSize.getY(),
                        ::canvas::tools::roundUp( rRequestedArea.getMaxY() - aOutputPosition.Y()) ) );

        cairo_rectangle( pCompositingCairo.get(), aOutputPosition.X(), aOutputPosition.Y(), aOutputSize.Width(), aOutputSize.Height() );
        cairo_clip( pCompositingCairo.get() );

        // paint background
        cairo_save( pCompositingCairo.get() );
        cairo_set_source_surface( pCompositingCairo.get(),
                                  mpOwningSpriteCanvas->getBufferSurface()->getCairoSurface().get(),
                                  0, 0 );
        cairo_set_operator( pCompositingCairo.get(), CAIRO_OPERATOR_SOURCE );
        cairo_paint( pCompositingCairo.get() );
        cairo_restore( pCompositingCairo.get() );

        // repaint all affected sprites on top of background into
        // VDev.
        for( const auto& rSprite : rSortedUpdateSprites )
        {
            if( rSprite.is() )
                ::boost::polymorphic_downcast< Sprite* >( rSprite.get() )->redraw(
                    pCompositingCairo, true );
        }

        // flush to screen
        cairo_rectangle( pWindowCairo.get(), aOutputPosition.X(), aOutputPosition.Y(), aOutputSize.Width(), aOutputSize.Height() );
        cairo_clip( pWindowCairo.get() );
        cairo_set_source_surface( pWindowCairo.get(),
                                  pCompositingSurface->getCairoSurface().get(),
                                  0, 0 );
        cairo_set_operator( pWindowCairo.get(), CAIRO_OPERATOR_SOURCE );
        cairo_paint( pWindowCairo.get() );
    }

    ::cairo::SurfaceSharedPtr const & SpriteCanvasHelper::getCompositingSurface( const ::basegfx::B2ISize& rNeededSize )
    {
        if( rNeededSize.getX() > maCompositingSurfaceSize.getX() ||
            rNeededSize.getY() > maCompositingSurfaceSize.getY() )
        {
            // need to give buffer more size
            mpCompositingSurface.reset();
        }

        if( !mpCompositingSurface )
        {
            mpCompositingSurface = createSurface( rNeededSize );
            maCompositingSurfaceSize = rNeededSize;
            mbCompositingSurfaceDirty = true;
            mpTemporarySurface.reset();
        }

        return mpCompositingSurface;
    }

    ::cairo::SurfaceSharedPtr const & SpriteCanvasHelper::getTemporarySurface()
    {
        if ( !mpTemporarySurface )
            mpTemporarySurface = createSurface( maCompositingSurfaceSize );
        return mpTemporarySurface;
    }

    ::cairo::SurfaceSharedPtr SpriteCanvasHelper::createSurface( const ::basegfx::B2ISize& rNeededSize ) const
    {
        return mpOwningSpriteCanvas->getWindowSurface()->getSimilar(
                    CAIRO_CONTENT_COLOR,
                    rNeededSize.getX(), rNeededSize.getY() );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
