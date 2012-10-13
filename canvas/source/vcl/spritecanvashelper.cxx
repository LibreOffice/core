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


#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>
#include <canvas/verbosetrace.hxx>
#include <canvas/canvastools.hxx>

#include <vcl/canvastools.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <vcl/bitmapex.hxx>

#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/tools/canvastools.hxx>

#include <boost/cast.hpp>

#include "spritecanvashelper.hxx"
#include "canvascustomsprite.hxx"


using namespace ::com::sun::star;

#define FPS_BOUNDS Rectangle(0,0,130,90)
#define INFO_COLOR COL_RED

namespace vclcanvas
{
    namespace
    {
        /** Sprite redraw at original position

            Used to repaint the whole canvas (background and all
            sprites)
         */
        void spriteRedraw( OutputDevice&                      rOutDev,
                           const ::canvas::Sprite::Reference& rSprite )
        {
            // downcast to derived vclcanvas::Sprite interface, which
            // provides the actual redraw methods.
            ::boost::polymorphic_downcast< Sprite* >(rSprite.get())->redraw(rOutDev,
                                                                            true);
        }

        double calcNumPixel( const ::canvas::Sprite::Reference& rSprite )
        {
            const ::basegfx::B2DSize& rSize(
                ::boost::polymorphic_downcast< Sprite* >(rSprite.get())->getSizePixel() );

            return rSize.getX() * rSize.getY();
        }

        void repaintBackground( OutputDevice&               rOutDev,
                                OutputDevice&               rBackBuffer,
                                const ::basegfx::B2DRange&  rArea )
        {
            const ::Point& rPos( ::vcl::unotools::pointFromB2DPoint( rArea.getMinimum()) );
            const ::Size& rSize( ::vcl::unotools::sizeFromB2DSize( rArea.getRange()) );

            rOutDev.DrawOutDev( rPos, rSize, rPos, rSize, rBackBuffer );
        }

        void opaqueUpdateSpriteArea( const ::canvas::Sprite::Reference& rSprite,
                                     OutputDevice&                      rOutDev,
                                     const ::basegfx::B2IRange&         rArea )
        {
            const Rectangle& rRequestedArea(
                ::vcl::unotools::rectangleFromB2IRectangle( rArea ) );

            // clip output to actual update region (otherwise a)
            // wouldn't save much render time, and b) will clutter
            // scrolled sprite content outside this area)
            rOutDev.EnableMapMode( sal_False );
            rOutDev.SetClipRegion( rRequestedArea );

            // repaint affected sprite directly to output device (at
            // the actual screen output position)
            ::boost::polymorphic_downcast< Sprite* >(
                rSprite.get() )->redraw( rOutDev,
                                         false ); // rendering
                                                  // directly to
                                                  // frontbuffer
        }

        /** Repaint sprite at original position

            Used for opaque updates, which render directly to the
            front buffer.
         */
        void spriteRedrawStub( OutputDevice&                      rOutDev,
                               const ::canvas::Sprite::Reference& rSprite )
        {
            if( rSprite.is() )
            {
                ::boost::polymorphic_downcast< Sprite* >(
                    rSprite.get() )->redraw( rOutDev,
                                             false );
            }
        }

        /** Repaint sprite at given position

            Used for generic update, which renders into vdev of
            adapted size.
         */
        void spriteRedrawStub2( OutputDevice&                       rOutDev,
                                const ::basegfx::B2DPoint&          rOutPos,
                                const ::canvas::Sprite::Reference&  rSprite )
        {
            if( rSprite.is() )
            {
                Sprite* pSprite = ::boost::polymorphic_downcast< Sprite* >(
                    rSprite.get() );

                // calc relative sprite position in rUpdateArea (which
                // need not be the whole screen!)
                const ::basegfx::B2DPoint& rSpriteScreenPos( pSprite->getPosPixel() );
                const ::basegfx::B2DPoint& rSpriteRenderPos( rSpriteScreenPos - rOutPos );

                pSprite->redraw( rOutDev, rSpriteRenderPos, true );
            }
        }

        /** Repaint sprite at original position

            Used for opaque updates from scrollUpdate(), which render
            directly to the front buffer.
         */
        void spriteRedrawStub3( OutputDevice&                                       rOutDev,
                                const ::canvas::SpriteRedrawManager::AreaComponent& rComponent )
        {
            const ::canvas::Sprite::Reference& rSprite( rComponent.second.getSprite() );

            if( rSprite.is() )
            {
                ::boost::polymorphic_downcast< Sprite* >(
                    rSprite.get() )->redraw( rOutDev,
                                             false );
            }
        }

        void renderInfoText( OutputDevice&          rOutDev,
                             const ::rtl::OUString& rStr,
                             const Point&           rPos )
        {
            Font aVCLFont;
            aVCLFont.SetHeight( 20 );
            aVCLFont.SetColor( Color( INFO_COLOR ) );

            rOutDev.SetTextAlign(ALIGN_TOP);
            rOutDev.SetTextColor( Color( INFO_COLOR ) );
            rOutDev.SetFont( aVCLFont );

            rOutDev.DrawText( rPos, rStr );
        }

    }

    SpriteCanvasHelper::SpriteCanvasHelper() :
        mpRedrawManager( NULL ),
        mpOwningSpriteCanvas( NULL ),
        maVDev(),
        maLastUpdate(),
        mbShowFrameInfo( false ),
        mbShowSpriteBounds( false ),
        mbIsUnsafeScrolling( false )
    {
#if OSL_DEBUG_LEVEL > 2
        // inverse defaults for verbose debug mode
        mbShowSpriteBounds = mbShowFrameInfo = true;
#endif
    }

    void SpriteCanvasHelper::init( const OutDevProviderSharedPtr& rOutDev,
                                   SpriteCanvas&                  rOwningSpriteCanvas,
                                   ::canvas::SpriteRedrawManager& rManager,
                                   bool                           bProtect,
                                   bool                           bHaveAlpha )
    {
        mpOwningSpriteCanvas = &rOwningSpriteCanvas;
        mpRedrawManager = &rManager;

        CanvasHelper::init(rOwningSpriteCanvas,rOutDev,bProtect,bHaveAlpha);
    }

    void SpriteCanvasHelper::disposing()
    {
        mpRedrawManager = NULL;
        mpOwningSpriteCanvas = NULL;

        // forward to base
        CanvasHelper::disposing();
    }

    uno::Reference< rendering::XAnimatedSprite > SpriteCanvasHelper::createSpriteFromAnimation(
        const uno::Reference< rendering::XAnimation >&  )
    {
        return uno::Reference< rendering::XAnimatedSprite >();
    }

    uno::Reference< rendering::XAnimatedSprite > SpriteCanvasHelper::createSpriteFromBitmaps(
        const uno::Sequence< uno::Reference< rendering::XBitmap > >& ,
        sal_Int8                                                      )
    {
        return uno::Reference< rendering::XAnimatedSprite >();
    }

    uno::Reference< rendering::XCustomSprite > SpriteCanvasHelper::createCustomSprite( const geometry::RealSize2D& spriteSize )
    {
        if( !mpRedrawManager || !mpDevice )
            return uno::Reference< rendering::XCustomSprite >(); // we're disposed

        return uno::Reference< rendering::XCustomSprite >(
            new CanvasCustomSprite( spriteSize,
                                    *mpDevice,
                                    mpOwningSpriteCanvas,
                                    mpOwningSpriteCanvas->getFrontBuffer(),
                                    mbShowSpriteBounds ) );
    }

    uno::Reference< rendering::XSprite > SpriteCanvasHelper::createClonedSprite( const uno::Reference< rendering::XSprite >&  )
    {
        return uno::Reference< rendering::XSprite >();
    }

    sal_Bool SpriteCanvasHelper::updateScreen( sal_Bool bUpdateAll,
                                               bool&    io_bSurfaceDirty )
    {
        if( !mpRedrawManager ||
            !mpOwningSpriteCanvas ||
            !mpOwningSpriteCanvas->getFrontBuffer() ||
            !mpOwningSpriteCanvas->getBackBuffer() )
        {
            return sal_False; // disposed, or otherwise dysfunctional
        }

        // commit to backbuffer
        flush();

        OutputDevice&       rOutDev( mpOwningSpriteCanvas->getFrontBuffer()->getOutDev() );
        BackBufferSharedPtr pBackBuffer( mpOwningSpriteCanvas->getBackBuffer() );
        OutputDevice&       rBackOutDev( pBackBuffer->getOutDev() );

        // actual OutputDevice is a shared resource - restore its
        // state when done.
        tools::OutDevStateKeeper aStateKeeper( rOutDev );

        const Size  aOutDevSize( rBackOutDev.GetOutputSizePixel() );
        const Point aEmptyPoint(0,0);

        Window* pTargetWindow = NULL;
        if( rOutDev.GetOutDevType() == OUTDEV_WINDOW )
        {
            pTargetWindow = &static_cast<Window&>(rOutDev); // TODO(Q3): Evil downcast.

            // we're double-buffered, thus no need for paint area-limiting
            // clips. besides that, will interfere with animations (as for
            // Window-invalidate repaints, only parts of the window will
            // be redrawn otherwise)
            const Region aFullWindowRegion( Rectangle(aEmptyPoint,
                                                      aOutDevSize) );
            pTargetWindow->ExpandPaintClipRegion(aFullWindowRegion);
        }

        // TODO(P1): Might be worthwile to track areas of background
        // changes, too.
        if( !bUpdateAll && !io_bSurfaceDirty )
        {
            if( mbShowFrameInfo )
            {
                // also repaint background below frame counter (fake
                // that as a sprite vanishing in this area)
                mpRedrawManager->updateSprite( ::canvas::Sprite::Reference(),
                                               ::basegfx::B2DPoint(),
                                               ::basegfx::B2DRectangle( 0.0, 0.0,
                                                                        FPS_BOUNDS.Right(),
                                                                        FPS_BOUNDS.Bottom() ) );
            }

            // background has not changed, so we're free to optimize
            // repaint to areas where a sprite has changed

            // process each independent area of overlapping sprites
            // separately.
            mpRedrawManager->forEachSpriteArea( *this );
        }
        else
        {
            // background has changed, so we currently have no choice
            // but repaint everything (or caller requested that)

            maVDev->SetOutputSizePixel( aOutDevSize );
            maVDev->EnableMapMode( sal_False );
            maVDev->DrawOutDev( aEmptyPoint, aOutDevSize,
                                aEmptyPoint, aOutDevSize,
                                rBackOutDev );

            // repaint all active sprites on top of background into
            // VDev.
            mpRedrawManager->forEachSprite(
                ::boost::bind(
                    &spriteRedraw,
                    ::boost::ref( maVDev.get() ),
                    _1 ) );

            // flush to screen
            rOutDev.EnableMapMode( sal_False );
            rOutDev.SetClipRegion();
            rOutDev.DrawOutDev( aEmptyPoint, aOutDevSize,
                                aEmptyPoint, aOutDevSize,
                                *maVDev );
        }

        // change record vector must be cleared, for the next turn of
        // rendering and sprite changing
        mpRedrawManager->clearChangeRecords();

        io_bSurfaceDirty = false;

        if( mbShowFrameInfo )
        {
            renderFrameCounter( rOutDev );
            renderSpriteCount( rOutDev );
            renderMemUsage( rOutDev );
        }

#if OSL_DEBUG_LEVEL > 2
        static ::canvas::tools::ElapsedTime aElapsedTime;

        // log time immediately after surface flip
        OSL_TRACE( "SpriteCanvasHelper::updateScreen(): flip done at %f",
                   aElapsedTime.getElapsedTime() );
#endif

        // sync output with screen, to ensure that we don't queue up
        // render requests (calling code might rely on timing,
        // i.e. assume that things are visible on screen after
        // updateScreen() returns).
        if( pTargetWindow )
        {
            // commit to screen
            pTargetWindow->Sync();
        }

        return sal_True;
    }

    void SpriteCanvasHelper::backgroundPaint( const ::basegfx::B2DRange& rUpdateRect )
    {
        ENSURE_OR_THROW( mpOwningSpriteCanvas &&
                         mpOwningSpriteCanvas->getBackBuffer() &&
                         mpOwningSpriteCanvas->getFrontBuffer(),
                         "SpriteCanvasHelper::backgroundPaint(): NULL device pointer " );

        OutputDevice&       rOutDev( mpOwningSpriteCanvas->getFrontBuffer()->getOutDev() );
        BackBufferSharedPtr pBackBuffer( mpOwningSpriteCanvas->getBackBuffer() );
        OutputDevice&       rBackOutDev( pBackBuffer->getOutDev() );

        repaintBackground( rOutDev, rBackOutDev, rUpdateRect );
    }

    void SpriteCanvasHelper::scrollUpdate( const ::basegfx::B2DRange&                       rMoveStart,
                                           const ::basegfx::B2DRange&                       rMoveEnd,
                                           const ::canvas::SpriteRedrawManager::UpdateArea& rUpdateArea )
    {
        ENSURE_OR_THROW( mpOwningSpriteCanvas &&
                         mpOwningSpriteCanvas->getBackBuffer() &&
                         mpOwningSpriteCanvas->getFrontBuffer(),
                         "SpriteCanvasHelper::scrollUpdate(): NULL device pointer " );

        OutputDevice&       rOutDev( mpOwningSpriteCanvas->getFrontBuffer()->getOutDev() );
        BackBufferSharedPtr pBackBuffer( mpOwningSpriteCanvas->getBackBuffer() );
        OutputDevice&       rBackOutDev( pBackBuffer->getOutDev() );

        const Size&                rTargetSizePixel( rOutDev.GetOutputSizePixel() );
        const ::basegfx::B2IRange  aOutputBounds( 0,0,
                                                  rTargetSizePixel.Width(),
                                                  rTargetSizePixel.Height() );

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

        // Since strictly speaking, this scroll algorithm is plain
        // buggy, the scrolled area might actually lie _below_ another
        // window - we've made this feature configurable via
        // mbIsUnsafeScrolling.

        // clip to output bounds (cannot properly scroll stuff
        // _outside_ our screen area)
        if( !mbIsUnsafeScrolling ||
            !::canvas::tools::clipScrollArea( aSourceRect,
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
                                 ::boost::ref( rOutDev ),
                                 _1 ) );
        }
        else
        {
            // scroll rOutDev content
            rOutDev.CopyArea( ::vcl::unotools::pointFromB2IPoint( aDestPos ),
                              ::vcl::unotools::pointFromB2IPoint( aSourceRect.getMinimum() ),
                              // TODO(Q2): use numeric_cast to check range
                              ::Size( static_cast<sal_Int32>(aSourceRect.getRange().getX()),
                                      static_cast<sal_Int32>(aSourceRect.getRange().getY()) ) );

            const ::canvas::SpriteRedrawManager::SpriteConnectedRanges::ComponentListType::const_iterator
                aFirst( rUpdateArea.maComponentList.begin() );

            ENSURE_OR_THROW( aFirst->second.getSprite().is(),
                              "VCLCanvas::scrollUpdate(): no sprite" );

            // repaint uncovered areas from sprite. Need to actually
            // clip here, since we're only repainting _parts_ of the
            // sprite
            rOutDev.Push( PUSH_CLIPREGION );
            ::std::for_each( aUnscrollableAreas.begin(),
                             aUnscrollableAreas.end(),
                             ::boost::bind( &opaqueUpdateSpriteArea,
                                            ::boost::cref(aFirst->second.getSprite()),
                                            ::boost::ref(rOutDev),
                                            _1 ) );
            rOutDev.Pop();
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
                                        ::boost::ref(rOutDev),
                                        ::boost::ref(rBackOutDev),
                                        _1 ) );
    }

    void SpriteCanvasHelper::opaqueUpdate( SAL_UNUSED_PARAMETER const ::basegfx::B2DRange&,
                                           const ::std::vector< ::canvas::Sprite::Reference >& rSortedUpdateSprites )
    {
        ENSURE_OR_THROW( mpOwningSpriteCanvas &&
                         mpOwningSpriteCanvas->getBackBuffer() &&
                         mpOwningSpriteCanvas->getFrontBuffer(),
                         "SpriteCanvasHelper::opaqueUpdate(): NULL device pointer " );

        OutputDevice& rOutDev( mpOwningSpriteCanvas->getFrontBuffer()->getOutDev() );

        // no need to clip output to actual update region - there will
        // always be ALL sprites contained in the rectangular update
        // area containd in rTotalArea (that's the way
        // B2DConnectedRanges work). If rTotalArea appears to be
        // smaller than the sprite - then this sprite carries a clip,
        // and the update will be constrained to that rect.

        // repaint all affected sprites directly to output device
        ::std::for_each( rSortedUpdateSprites.begin(),
                         rSortedUpdateSprites.end(),
                         ::boost::bind(
                             &spriteRedrawStub,
                             ::boost::ref( rOutDev ),
                             _1 ) );
    }

    void SpriteCanvasHelper::genericUpdate( const ::basegfx::B2DRange&                          rRequestedArea,
                                            const ::std::vector< ::canvas::Sprite::Reference >& rSortedUpdateSprites )
    {
        ENSURE_OR_THROW( mpOwningSpriteCanvas &&
                         mpOwningSpriteCanvas->getBackBuffer() &&
                         mpOwningSpriteCanvas->getFrontBuffer(),
                         "SpriteCanvasHelper::genericUpdate(): NULL device pointer " );

        OutputDevice&       rOutDev( mpOwningSpriteCanvas->getFrontBuffer()->getOutDev() );
        BackBufferSharedPtr pBackBuffer( mpOwningSpriteCanvas->getBackBuffer() );
        OutputDevice&       rBackOutDev( pBackBuffer->getOutDev() );

        // limit size of update VDev to target outdev's size
        const Size& rTargetSizePixel( rOutDev.GetOutputSizePixel() );

        // round output position towards zero. Don't want to truncate
        // a fraction of a sprite pixel...  Clip position at origin,
        // otherwise, truncation of size below might leave visible
        // areas uncovered by VDev.
        const ::Point aOutputPosition(
            ::std::max( sal_Int32( 0 ),
                        static_cast< sal_Int32 >(rRequestedArea.getMinX()) ),
            ::std::max( sal_Int32( 0 ),
                        static_cast< sal_Int32 >(rRequestedArea.getMinY()) ) );
        // round output size towards +infty. Don't want to truncate a
        // fraction of a sprite pixel... Limit coverage of VDev to
        // output device's area (i.e. not only to total size, but to
        // cover _only_ the visible parts).
        const ::Size aOutputSize(
            ::std::max( sal_Int32( 0 ),
                        ::std::min( static_cast< sal_Int32 >(rTargetSizePixel.Width() - aOutputPosition.X()),
                                    ::canvas::tools::roundUp( rRequestedArea.getMaxX() - aOutputPosition.X() ))),
            ::std::max( sal_Int32( 0 ),
                        ::std::min( static_cast< sal_Int32 >(rTargetSizePixel.Height() - aOutputPosition.Y()),
                                    ::canvas::tools::roundUp( rRequestedArea.getMaxY() - aOutputPosition.Y() ))));

        // early exit for empty output area.
        if( aOutputSize.Width() == 0 &&
            aOutputSize.Height() == 0 )
        {
            return;
        }

        const Point aEmptyPoint(0,0);
        const Size  aCurrOutputSize( maVDev->GetOutputSizePixel() );

        // adapt maVDev's size to the area that actually needs the
        // repaint.
        if( aCurrOutputSize.Width() < aOutputSize.Width() ||
            aCurrOutputSize.Height() < aOutputSize.Height() )
        {
            // TODO(P1): Come up with a clever tactic to reduce maVDev
            // from time to time. Reduction with threshold (say, if
            // maVDev is more than twice too large) is not wise, as
            // this might then toggle within the same updateScreen(),
            // but for different disjunct sprite areas.
            maVDev->SetOutputSizePixel( aOutputSize );
        }

        // paint background
        maVDev->EnableMapMode( sal_False );
        maVDev->SetClipRegion();
        maVDev->DrawOutDev( aEmptyPoint, aOutputSize,
                            aOutputPosition, aOutputSize,
                            rBackOutDev );

        // repaint all affected sprites on top of background into
        // VDev.
        ::std::for_each( rSortedUpdateSprites.begin(),
                         rSortedUpdateSprites.end(),
                         ::boost::bind( &spriteRedrawStub2,
                                        ::boost::ref( maVDev.get() ),
                                        ::boost::cref(
                                            ::vcl::unotools::b2DPointFromPoint(aOutputPosition)),
                                        _1 ) );

        // flush to screen
        rOutDev.EnableMapMode( sal_False );
        rOutDev.DrawOutDev( aOutputPosition, aOutputSize,
                            aEmptyPoint, aOutputSize,
                            *maVDev );
    }

    void SpriteCanvasHelper::renderFrameCounter( OutputDevice& rOutDev )
    {
        const double denominator( maLastUpdate.getElapsedTime() );
        maLastUpdate.reset();

        ::rtl::OUString text( ::rtl::math::doubleToUString( denominator == 0.0 ? 100.0 : 1.0/denominator,
                                                            rtl_math_StringFormat_F,
                                                            2,'.',NULL,' ') );

        // pad with leading space
        while( text.getLength() < 6 )
            text = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM (" ")) + text;

        text += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM (" fps"));

        renderInfoText( rOutDev,
                        text,
                        Point(0, 0) );
    }

    namespace
    {
        template< typename T > struct Adder
        {
            typedef void result_type;

            Adder( T& rAdderTarget,
                   T  nIncrement ) :
                mpTarget( &rAdderTarget ),
                mnIncrement( nIncrement )
            {
            }

            void operator()() { *mpTarget += mnIncrement; }
            void operator()( const ::canvas::Sprite::Reference& ) { *mpTarget += mnIncrement; }
            void operator()( T nIncrement ) { *mpTarget += nIncrement; }

            T* mpTarget;
            T  mnIncrement;
        };

        template< typename T> Adder<T> makeAdder( T& rAdderTarget,
                                                  T  nIncrement )
        {
            return Adder<T>(rAdderTarget, nIncrement);
        }
    }

    void SpriteCanvasHelper::renderSpriteCount( OutputDevice& rOutDev )
    {
        if( mpRedrawManager )
        {
            sal_Int32 nCount(0);

            mpRedrawManager->forEachSprite( makeAdder(nCount,sal_Int32(1)) );
            ::rtl::OUString text(
                ::rtl::OUString::valueOf(
                    // disambiguate overload...
                    static_cast<sal_Int64>(nCount) ) );

            // pad with leading space
            while( text.getLength() < 3 )
                text = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM (" ")) + text;

            text = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("Sprites: ")) + text;

            renderInfoText( rOutDev,
                            text,
                            Point(0, 30) );
        }
    }

    void SpriteCanvasHelper::renderMemUsage( OutputDevice& rOutDev )
    {
        BackBufferSharedPtr pBackBuffer( mpOwningSpriteCanvas->getBackBuffer() );

        if( mpRedrawManager &&
            pBackBuffer )
        {
            double nPixel(0.0);

            // accumulate pixel count for each sprite into fCount
            mpRedrawManager->forEachSprite( ::boost::bind(
                                                makeAdder(nPixel,1.0),
                                                ::boost::bind(
                                                    &calcNumPixel,
                                                    _1 ) ) );

            static const int NUM_VIRDEV(2);
            static const int BYTES_PER_PIXEL(3);

            const Size& rVDevSize( maVDev->GetOutputSizePixel() );
            const Size& rBackBufferSize( pBackBuffer->getOutDev().GetOutputSizePixel() );

            const double nMemUsage( nPixel * NUM_VIRDEV * BYTES_PER_PIXEL +
                                    rVDevSize.Width()*rVDevSize.Height() * BYTES_PER_PIXEL +
                                    rBackBufferSize.Width()*rBackBufferSize.Height() * BYTES_PER_PIXEL );

            ::rtl::OUString text( ::rtl::math::doubleToUString( nMemUsage / 1048576.0,
                                                                rtl_math_StringFormat_F,
                                                                2,'.',NULL,' ') );

            // pad with leading space
            while( text.getLength() < 4 )
                text = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM (" ")) + text;

            text = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("Mem: ")) +
                text +
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("MB"));

            renderInfoText( rOutDev,
                            text,
                            Point(0, 60) );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
