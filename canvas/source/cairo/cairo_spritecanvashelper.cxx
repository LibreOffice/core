/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cairo_spritecanvashelper.cxx,v $
 * $Revision: 1.6 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>
#include <canvas/canvastools.hxx>

#include <vcl/canvastools.hxx>

#include <comphelper/scopeguard.hxx>

#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/tools/canvastools.hxx>

#include <boost/cast.hpp>

#include "cairo_spritecanvashelper.hxx"
#include "cairo_canvascustomsprite.hxx"

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
        void spriteRedraw( Cairo* pCairo,
                           const ::canvas::Sprite::Reference& rSprite )
        {
            // downcast to derived vclcanvas::Sprite interface, which
            // provides the actual redraw methods.
            ::boost::polymorphic_downcast< Sprite* >(rSprite.get())->redraw( pCairo, true);
        }

        void repaintBackground( Cairo*                     pCairo,
                                Surface*                   pBackgroundSurface,
                                const ::basegfx::B2DRange& rArea )
        {
        cairo_save( pCairo );
        cairo_rectangle( pCairo, ceil( rArea.getMinX() ), ceil( rArea.getMinY() ), floor( rArea.getWidth() ), floor( rArea.getHeight() ) );
        cairo_clip( pCairo );
        cairo_set_source_surface( pCairo, pBackgroundSurface->mpSurface, 0, 0 );
        cairo_set_operator( pCairo, CAIRO_OPERATOR_SOURCE );
        cairo_paint( pCairo );
        cairo_restore( pCairo );
        }

        void opaqueUpdateSpriteArea( const ::canvas::Sprite::Reference& rSprite,
                                     Cairo*                             pCairo,
                                     const ::basegfx::B2IRange&         rArea )
        {
            // clip output to actual update region (otherwise a)
            // wouldn't save much render time, and b) will clutter
            // scrolled sprite content outside this area)
        cairo_save( pCairo );
        cairo_rectangle( pCairo, rArea.getMinX(), rArea.getMinY(),
                         sal::static_int_cast<sal_Int32>(rArea.getWidth()),
                         sal::static_int_cast<sal_Int32>(rArea.getHeight()) );
        cairo_clip( pCairo );

            // repaint affected sprite directly to output device (at
            // the actual screen output position)
        // rendering directly to device buffer
            ::boost::polymorphic_downcast< Sprite* >( rSprite.get() )->redraw( pCairo, false );

        cairo_restore( pCairo );
        }

        /** Repaint sprite at original position

            Used for opaque updates, which render directly to the
            device buffer.
         */
        void spriteRedrawStub( Cairo* pCairo,
                               const ::canvas::Sprite::Reference& rSprite )
        {
            if( rSprite.is() )
            {
                ::boost::polymorphic_downcast< Sprite* >( rSprite.get() )->redraw( pCairo, false );
            }
        }

        /** Repaint sprite at given position

            Used for generic update, which renders into device buffer.
         */
        void spriteRedrawStub2( Cairo* pCairo,
                                const ::canvas::Sprite::Reference&  rSprite )
        {
            if( rSprite.is() )
            {
                ::boost::polymorphic_downcast< Sprite* >( rSprite.get() )->redraw( pCairo, true );;
            }
        }

        /** Repaint sprite at original position

            Used for opaque updates from scrollUpdate(), which render
            directly to the front buffer.
         */
        void spriteRedrawStub3( Cairo* pCairo,
                                const ::canvas::SpriteRedrawManager::AreaComponent& rComponent )
        {
            const ::canvas::Sprite::Reference& rSprite( rComponent.second.getSprite() );

            if( rSprite.is() )
                ::boost::polymorphic_downcast< Sprite* >( rSprite.get() )->redraw( pCairo, false );
        }
    }

    SpriteCanvasHelper::SpriteCanvasHelper() :
        mpRedrawManager( NULL )
    {
    }

    void SpriteCanvasHelper::init( ::canvas::SpriteRedrawManager& rManager,
                                   SpriteCanvas&                  rDevice,
                                   const ::basegfx::B2ISize&      rSize )
    {
        mpRedrawManager = &rManager;

        CanvasHelper::init( rSize, rDevice );
    }

    void SpriteCanvasHelper::disposing()
    {
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
                                    mpDevice ) );
    }

    uno::Reference< rendering::XSprite > SpriteCanvasHelper::createClonedSprite( const uno::Reference< rendering::XSprite >& /*original*/ )
    {
        return uno::Reference< rendering::XSprite >();
    }

    sal_Bool SpriteCanvasHelper::updateScreen( const ::basegfx::B2IRange& /*rCurrArea*/,
                                               sal_Bool                   bUpdateAll,
                                               bool&                      io_bSurfaceDirty )
    {
        if( !mpRedrawManager ||
            !mpDevice ||
            !mpDevice->getWindowSurface() ||
            !mpDevice->getBufferSurface() )
        {
            return sal_False; // disposed, or otherwise dysfunctional
        }

    OSL_TRACE("SpriteCanvasHelper::updateScreen called");

    Cairo* pBufferCairo = mpDevice->getBufferSurface()->getCairo();
    Cairo* pWindowCairo = mpDevice->getWindowSurface()->getCairo();

    const ::basegfx::B2ISize& rSize = mpDevice->getSizePixel();

        // TODO(P1): Might be worthwile to track areas of background
        // changes, too.
        if( !bUpdateAll && !io_bSurfaceDirty )
        {
            // background has not changed, so we're free to optimize
            // repaint to areas where a sprite has changed

            // process each independent area of overlapping sprites
            // separately.
            mpRedrawManager->forEachSpriteArea( *this );
        }
        else
        {
        OSL_TRACE("SpriteCanvasHelper::updateScreen update ALL");

            // background has changed, so we currently have no choice
            // but repaint everything (or caller requested that)

        cairo_rectangle( pBufferCairo, 0, 0, rSize.getX(), rSize.getY() );
        cairo_clip( pBufferCairo );
        cairo_save( pBufferCairo );
        cairo_set_source_surface( pBufferCairo, mpDevice->getBackgroundSurface()->mpSurface, 0, 0 );
        cairo_set_operator( pBufferCairo, CAIRO_OPERATOR_SOURCE );
        cairo_paint( pBufferCairo );
        cairo_restore( pBufferCairo );

            // repaint all active sprites on top of background into
            // VDev.
            mpRedrawManager->forEachSprite(
                ::boost::bind(
                    &spriteRedraw,
                    pBufferCairo,
                    _1 ) );

            // flush to screen
        cairo_rectangle( pWindowCairo, 0, 0, rSize.getX(), rSize.getY() );
        cairo_clip( pWindowCairo );
        cairo_set_source_surface( pWindowCairo, mpDevice->getBufferSurface()->mpSurface, 0, 0 );
        cairo_set_operator( pWindowCairo, CAIRO_OPERATOR_SOURCE );
        cairo_paint( pWindowCairo );
        }

    cairo_destroy( pBufferCairo );
    cairo_destroy( pWindowCairo );

        // change record vector must be cleared, for the next turn of
        // rendering and sprite changing
        mpRedrawManager->clearChangeRecords();

        io_bSurfaceDirty = false;

        // commit to screen
        mpDevice->flush();

        return sal_True;
    }

    void SpriteCanvasHelper::backgroundPaint( const ::basegfx::B2DRange& rUpdateRect )
    {
    if( mpDevice ) {
        Cairo* pBufferCairo = mpDevice->getBufferSurface()->getCairo();

        repaintBackground( pBufferCairo, mpDevice->getBackgroundSurface(), rUpdateRect );
        cairo_destroy( pBufferCairo );
    }
    }

    void SpriteCanvasHelper::scrollUpdate( const ::basegfx::B2DRange&                       rMoveStart,
                                           const ::basegfx::B2DRange&                       rMoveEnd,
                                           const ::canvas::SpriteRedrawManager::UpdateArea& rUpdateArea )
    {
        ENSURE_AND_THROW( mpDevice &&
                          mpDevice->getBufferSurface(),
                          "SpriteCanvasHelper::scrollUpdate(): NULL device pointer " );

    Cairo* pBufferCairo = mpDevice->getBufferSurface()->getCairo();

    OSL_TRACE("SpriteCanvasHelper::scrollUpdate called");

    const ::basegfx::B2ISize& rSize = mpDevice->getSizePixel();
        const ::basegfx::B2IRange  aOutputBounds( 0,0,
                                                  rSize.getX(),
                                                  rSize.getY() );

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

        ::std::vector< ::basegfx::B2IRange > aUnscrollableAreas;

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
            ::std::for_each( rUpdateArea.maComponentList.begin(),
                             rUpdateArea.maComponentList.end(),
                             ::boost::bind(
                                 &spriteRedrawStub3,
                                 pBufferCairo,
                                 _1 ) );
        }
        else
        {
        const ::basegfx::B2IVector aSourceUpperLeftPos( aSourceRect.getMinimum() );

        // clip dest area (which must be inside rDestBounds)
        ::basegfx::B2IRange aDestRect( rDestRect );
        aDestRect.intersect( aOutputBounds );

        cairo_save( pBufferCairo );
            // scroll content in device back buffer
        cairo_set_source_surface( pBufferCairo, mpDevice->getBufferSurface()->mpSurface,
                      aDestPos.getX() - aSourceUpperLeftPos.getX(),
                      aDestPos.getY() - aSourceUpperLeftPos.getY() );
        cairo_rectangle( pBufferCairo,
                         aDestPos.getX(), aDestPos.getY(),
                         sal::static_int_cast<sal_Int32>(aDestRect.getWidth()),
                         sal::static_int_cast<sal_Int32>(aDestRect.getHeight()) );
        cairo_clip( pBufferCairo );
        cairo_set_operator( pBufferCairo, CAIRO_OPERATOR_SOURCE );
        cairo_paint( pBufferCairo );
        cairo_restore( pBufferCairo );

            const ::canvas::SpriteRedrawManager::SpriteConnectedRanges::ComponentListType::const_iterator
                aFirst( rUpdateArea.maComponentList.begin() );
            ::canvas::SpriteRedrawManager::SpriteConnectedRanges::ComponentListType::const_iterator
                  aSecond( aFirst ); ++aSecond;

            ENSURE_AND_THROW( aFirst->second.getSprite().is(),
                              "VCLCanvas::scrollUpdate(): no sprite" );

            // repaint uncovered areas from sprite. Need to actually
            // clip here, since we're only repainting _parts_ of the
            // sprite
            ::std::for_each( aUnscrollableAreas.begin(),
                             aUnscrollableAreas.end(),
                             ::boost::bind( &opaqueUpdateSpriteArea,
                                            ::boost::cref(aFirst->second.getSprite()),
                                            pBufferCairo,
                                            _1 ) );
        }

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
                                        pBufferCairo,
                                        mpDevice->getBackgroundSurface(),
                                        _1 ) );

    Cairo* pWindowCairo = mpDevice->getWindowSurface()->getCairo();

    cairo_rectangle( pWindowCairo, 0, 0, rSize.getX(), rSize.getY() );
    cairo_clip( pWindowCairo );
    cairo_set_source_surface( pWindowCairo, mpDevice->getBufferSurface()->mpSurface, 0, 0 );
    cairo_set_operator( pWindowCairo, CAIRO_OPERATOR_SOURCE );
    cairo_paint( pWindowCairo );

    cairo_destroy( pBufferCairo );
    cairo_destroy( pWindowCairo );
    }

    void SpriteCanvasHelper::opaqueUpdate( const ::basegfx::B2DRange&                          rTotalArea,
                                           const ::std::vector< ::canvas::Sprite::Reference >& rSortedUpdateSprites )
    {
        ENSURE_AND_THROW( mpDevice &&
                          mpDevice->getBufferSurface(),
                          "SpriteCanvasHelper::opaqueUpdate(): NULL device pointer " );

    OSL_TRACE("SpriteCanvasHelper::opaqueUpdate called");

    Cairo* pBufferCairo = mpDevice->getBufferSurface()->getCairo();
    const ::basegfx::B2ISize& rDeviceSize = mpDevice->getSizePixel();

    cairo_rectangle( pBufferCairo, 0, 0, rDeviceSize.getX(), rDeviceSize.getY() );
    cairo_clip( pBufferCairo );

    ::basegfx::B2DVector aPos( ceil( rTotalArea.getMinX() ), ceil( rTotalArea.getMinY() ) );
    ::basegfx::B2DVector aSize( floor( rTotalArea.getMaxX() - aPos.getX() ), floor( rTotalArea.getMaxY() - aPos.getY() ) );

    cairo_rectangle( pBufferCairo, aPos.getX(), aPos.getY(), aSize.getX(), aSize.getY() );
    cairo_clip( pBufferCairo );

        // repaint all affected sprites directly to output device
        ::std::for_each( rSortedUpdateSprites.begin(),
                         rSortedUpdateSprites.end(),
                         ::boost::bind(
                             &spriteRedrawStub,
                             pBufferCairo,
                             _1 ) );

        // flush to screen
    Cairo* pWindowCairo = mpDevice->getWindowSurface()->getCairo();

    cairo_rectangle( pWindowCairo, 0, 0, rDeviceSize.getX(), rDeviceSize.getY() );
    cairo_clip( pWindowCairo );
    cairo_rectangle( pWindowCairo, aPos.getX(), aPos.getY(), aSize.getX(), aSize.getY() );
    cairo_clip( pWindowCairo );
    cairo_set_source_surface( pWindowCairo, mpDevice->getBufferSurface()->mpSurface, 0, 0 );
    cairo_set_operator( pWindowCairo, CAIRO_OPERATOR_SOURCE );
    cairo_paint( pWindowCairo );

    cairo_destroy( pBufferCairo );
    cairo_destroy( pWindowCairo );
    }

    void SpriteCanvasHelper::genericUpdate( const ::basegfx::B2DRange&                          rRequestedArea,
                                            const ::std::vector< ::canvas::Sprite::Reference >& rSortedUpdateSprites )
    {
        // TODO
    OSL_TRACE("SpriteCanvasHelper::genericUpdate called");

        ENSURE_AND_THROW( mpDevice &&
                          mpDevice->getBufferSurface(),
                          "SpriteCanvasHelper::genericUpdate(): NULL device pointer " );

    Cairo* pBufferCairo = mpDevice->getBufferSurface()->getCairo();

        // limit size of update VDev to target outdev's size
    const ::basegfx::B2ISize& rSize = mpDevice->getSizePixel();

        // round output position towards zero. Don't want to truncate
        // a fraction of a sprite pixel...  Clip position at origin,
        // otherwise, truncation of size below might leave visible
        // areas uncovered by VDev.
        const Point aOutputPosition(
            ::std::max( sal_Int32( 0 ),
                        static_cast< sal_Int32 >(rRequestedArea.getMinX()) ),
            ::std::max( sal_Int32( 0 ),
                        static_cast< sal_Int32 >(rRequestedArea.getMinY()) ) );
        // round output size towards +infty. Don't want to truncate a
        // fraction of a sprite pixel... Limit size of VDev to output
        // device's area.
        const Size  aOutputSize(
            ::std::min( rSize.getX(),
                        ::canvas::tools::roundUp( rRequestedArea.getMaxX() - aOutputPosition.X()) ),
            ::std::min( rSize.getY(),
                        ::canvas::tools::roundUp( rRequestedArea.getMaxY() - aOutputPosition.Y()) ) );

    cairo_rectangle( pBufferCairo, aOutputPosition.X(), aOutputPosition.Y(), aOutputSize.Width(), aOutputSize.Height() );
    cairo_clip( pBufferCairo );

        // paint background
    cairo_save( pBufferCairo );
    cairo_set_source_surface( pBufferCairo, mpDevice->getBackgroundSurface()->mpSurface, 0, 0 );
    cairo_set_operator( pBufferCairo, CAIRO_OPERATOR_SOURCE );
    cairo_paint( pBufferCairo );
    cairo_restore( pBufferCairo );

        // repaint all affected sprites on top of background into
        // VDev.
        ::std::for_each( rSortedUpdateSprites.begin(),
                         rSortedUpdateSprites.end(),
                         ::boost::bind( &spriteRedrawStub2,
                                        pBufferCairo,
                                        _1 ) );

        // flush to screen
    Cairo* pWindowCairo = mpDevice->getWindowSurface()->getCairo();

     cairo_rectangle( pWindowCairo, aOutputPosition.X(), aOutputPosition.Y(), aOutputSize.Width(), aOutputSize.Height() );
     cairo_clip( pWindowCairo );
    cairo_set_source_surface( pWindowCairo, mpDevice->getBufferSurface()->mpSurface, 0, 0 );
    cairo_set_operator( pWindowCairo, CAIRO_OPERATOR_SOURCE );
    cairo_paint( pWindowCairo );

    cairo_destroy( pBufferCairo );
    cairo_destroy( pWindowCairo );
    }
}
