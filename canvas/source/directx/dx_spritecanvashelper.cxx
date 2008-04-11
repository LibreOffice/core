/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dx_spritecanvashelper.cxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>
#include <canvas/canvastools.hxx>

#include <comphelper/scopeguard.hxx>

#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/tools/canvastools.hxx>

#include <boost/cast.hpp>

#include "dx_spritecanvashelper.hxx"
#include "dx_canvascustomsprite.hxx"

using namespace ::com::sun::star;

namespace dxcanvas
{
    namespace
    {
        void repaintBackground( const ::basegfx::B2DRange&      rUpdateArea,
                                const ::basegfx::B2IRange&      rOutputArea,
                                const DXBitmapSharedPtr&        rBackBuffer )
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

        void spriteRedrawStub( const ::canvas::Sprite::Reference& rSprite )
        {
            if( rSprite.is() )
            {
                // downcast to derived dxcanvas::Sprite interface, which
                // provides the actual redraw methods.
                ::boost::polymorphic_downcast< Sprite* >(
                    rSprite.get() )->redraw();
            }
        }

        void spriteRedrawStub2( const ::canvas::SpriteRedrawManager::AreaComponent& rComponent )
        {
            if( rComponent.second.getSprite().is() )
            {
                // downcast to derived dxcanvas::Sprite interface, which
                // provides the actual redraw methods.
                ::boost::polymorphic_downcast< Sprite* >(
                    rComponent.second.getSprite().get() )->redraw();
            }
        }
    }

    SpriteCanvasHelper::SpriteCanvasHelper() :
        mpRedrawManager( NULL ),
        mpRenderModule(),
        mpSurfaceProxy(),
        mpBackBuffer(),
        maUpdateRect(),
        maScrapRect(),
        mbShowSpriteBounds( false )
    {
#if defined(VERBOSE) && defined(DBG_UTIL)
        // inverse default for verbose debug mode
        mbShowSpriteBounds = true;
#endif
    }

    void SpriteCanvasHelper::init( ::canvas::SpriteRedrawManager&                   rManager,
                                   const IDXRenderModuleSharedPtr&                  rRenderModule,
                                   const ::canvas::ISurfaceProxyManagerSharedPtr&   rSurfaceProxy,
                                   const DXBitmapSharedPtr&                         rBackBuffer,
                                   const ::basegfx::B2ISize&                        rOutputOffset )
    {
        // init base
        setTarget( rBackBuffer, rOutputOffset );

        mpRedrawManager = &rManager;
        mpRenderModule = rRenderModule;
        mpSurfaceProxy = rSurfaceProxy;
        mpBackBuffer = rBackBuffer;
    }

    void SpriteCanvasHelper::disposing()
    {
        if(mpRenderModule)
            mpRenderModule->disposing();

        mpBackBuffer.reset();
        mpRenderModule.reset();
        mpRedrawManager = NULL;

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
                                    mpDevice,
                                    mpRenderModule,
                                    mpSurfaceProxy,
                                    mbShowSpriteBounds ) );
    }

    uno::Reference< rendering::XSprite > SpriteCanvasHelper::createClonedSprite( const uno::Reference< rendering::XSprite >& /*original*/ )
    {
        return uno::Reference< rendering::XSprite >();
    }

    sal_Bool SpriteCanvasHelper::updateScreen( const ::basegfx::B2IRectangle& rCurrArea,
                                               sal_Bool                       bUpdateAll,
                                               bool&                          io_bSurfaceDirty )
    {
        if( !mpRedrawManager ||
            !mpRenderModule ||
            !mpBackBuffer )
        {
            return sal_False; // disposed, or otherwise dysfunctional
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
            mpRedrawManager->forEachSprite(::std::ptr_fun( &spriteRedraw ) );

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

        return sal_True;
    }

    void SpriteCanvasHelper::backgroundPaint( const ::basegfx::B2DRange& rUpdateRect )
    {
        ENSURE_AND_THROW( mpRenderModule &&
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
        ENSURE_AND_THROW( mpRenderModule &&
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
        ::std::for_each( rUpdateArea.maComponentList.begin(),
                         rUpdateArea.maComponentList.end(),
                         ::std::ptr_fun( &spriteRedrawStub2 ) );

        // repaint uncovered areas from backbuffer - take the
        // _rounded_ rectangles from above, to have the update
        // consistent with the scroll above.
        ::std::vector< ::basegfx::B2DRange > aUncoveredAreas;
        ::basegfx::computeSetDifference( aUncoveredAreas,
                                         rUpdateArea.maTotalBounds,
                                         ::basegfx::B2DRange( rDestRect ) );
        ::std::for_each( aUncoveredAreas.begin(),
                         aUncoveredAreas.end(),
                         ::boost::bind( &repaintBackground,
                                        _1,
                                        ::boost::cref(maScrapRect),
                                        ::boost::cref(mpBackBuffer) ) );

        // TODO(E1): Use numeric_cast to catch overflow here
        ::basegfx::B2IRange aActualArea( 0, 0,
                                         static_cast<sal_Int32>(maScrapRect.getWidth()),
                                         static_cast<sal_Int32>(maScrapRect.getHeight()) );
        aActualArea.intersect( fround( rUpdateArea.maTotalBounds ) );

        // add given update area to the 'blit to foreground' rect
        maUpdateRect.expand( aActualArea );
    }

    void SpriteCanvasHelper::opaqueUpdate( const ::basegfx::B2DRange&                          rTotalArea,
                                           const ::std::vector< ::canvas::Sprite::Reference >& rSortedUpdateSprites )
    {
        ENSURE_AND_THROW( mpRenderModule &&
                          mpBackBuffer,
                          "SpriteCanvasHelper::opaqueUpdate(): NULL device pointer " );

        // TODO(P2): optimize this by truly rendering to the front
        // buffer. Currently, we've the 3D device only for the back
        // buffer.
        ::std::for_each( rSortedUpdateSprites.begin(),
                         rSortedUpdateSprites.end(),
                         ::std::ptr_fun( &spriteRedrawStub ) );

        // TODO(E1): Use numeric_cast to catch overflow here
        ::basegfx::B2IRange aActualArea( 0, 0,
                                         static_cast<sal_Int32>(maScrapRect.getWidth()),
                                         static_cast<sal_Int32>(maScrapRect.getHeight()) );
        aActualArea.intersect( fround( rTotalArea ) );

        // add given update area to the 'blit to foreground' rect
        maUpdateRect.expand( aActualArea );
    }

    void SpriteCanvasHelper::genericUpdate( const ::basegfx::B2DRange&                          rTotalArea,
                                            const ::std::vector< ::canvas::Sprite::Reference >& rSortedUpdateSprites )
    {
        ENSURE_AND_THROW( mpRenderModule &&
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

        ::std::for_each( rSortedUpdateSprites.begin(),
                         rSortedUpdateSprites.end(),
                         ::std::ptr_fun( &spriteRedrawStub ) );

        // add given update area to the 'blit to foreground' rect
        maUpdateRect.expand( aActualArea );
    }
}
