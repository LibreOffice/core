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
#include <canvas/canvastools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <cppcanvas/basegfxfactory.hxx>

#include "slidechangebase.hxx"
#include "tools.hxx"

#include <boost/bind.hpp>
#include <algorithm>

using namespace com::sun::star;

namespace slideshow {
namespace internal {

SlideChangeBase::SlideChangeBase( boost::optional<SlideSharedPtr> const & leavingSlide,
                                  const SlideSharedPtr&                   pEnteringSlide,
                                  const SoundPlayerSharedPtr&             pSoundPlayer,
                                  const UnoViewContainer&                 rViewContainer,
                                  ScreenUpdater&                          rScreenUpdater,
                                  EventMultiplexer&                       rEventMultiplexer,
                                  bool                                    bCreateLeavingSprites,
                                  bool                                    bCreateEnteringSprites ) :
      mpSoundPlayer( pSoundPlayer ),
      mrEventMultiplexer(rEventMultiplexer),
      mrScreenUpdater(rScreenUpdater),
      maLeavingSlide( leavingSlide ),
      mpEnteringSlide( pEnteringSlide ),
      maViewData(),
      mrViewContainer(rViewContainer),
      mbCreateLeavingSprites(bCreateLeavingSprites),
      mbCreateEnteringSprites(bCreateEnteringSprites),
      mbSpritesVisible(false),
      mbFinished(false),
      mbPrefetched(false)
{
    ENSURE_OR_THROW(
        pEnteringSlide,
        "SlideChangeBase::SlideChangeBase(): Invalid entering slide!" );
}

SlideBitmapSharedPtr SlideChangeBase::getLeavingBitmap( const ViewEntry& rViewEntry ) const
{
    if( !rViewEntry.mpLeavingBitmap )
        rViewEntry.mpLeavingBitmap = createBitmap(rViewEntry.mpView,
                                                  maLeavingSlide);

    return rViewEntry.mpLeavingBitmap;
}

SlideBitmapSharedPtr SlideChangeBase::getEnteringBitmap( const ViewEntry& rViewEntry ) const
{
    if( !rViewEntry.mpEnteringBitmap )
        rViewEntry.mpEnteringBitmap = createBitmap( rViewEntry.mpView,
                                                    boost::optional<SlideSharedPtr>(mpEnteringSlide) );

    return rViewEntry.mpEnteringBitmap;
}

SlideBitmapSharedPtr SlideChangeBase::createBitmap( const UnoViewSharedPtr&                rView,
                                                    const boost::optional<SlideSharedPtr>& rSlide ) const
{
    SlideBitmapSharedPtr pRet;
    if( !rSlide )
        return pRet;

    SlideSharedPtr const & pSlide = *rSlide;
    if( !pSlide )
    {
        // TODO(P3): No need to generate a bitmap here. This only made
        // the code more uniform. Faster would be to simply clear the
        // sprite to black.

        // create empty, black-filled bitmap
        const basegfx::B2ISize slideSizePixel(
            getSlideSizePixel( basegfx::B2DSize( mpEnteringSlide->getSlideSize() ),
                               rView ));

        cppcanvas::CanvasSharedPtr pCanvas( rView->getCanvas() );

        // create a bitmap of appropriate size
        cppcanvas::BitmapSharedPtr pBitmap(
            cppcanvas::BaseGfxFactory::getInstance().createBitmap(
                pCanvas,
                slideSizePixel ) );

        ENSURE_OR_THROW(
            pBitmap,
            "SlideChangeBase::createBitmap(): Cannot create page bitmap" );

        cppcanvas::BitmapCanvasSharedPtr pBitmapCanvas(
            pBitmap->getBitmapCanvas() );

        ENSURE_OR_THROW( pBitmapCanvas,
                          "SlideChangeBase::createBitmap(): "
                          "Cannot create page bitmap canvas" );

        // set transformation to identitiy (->device pixel)
        pBitmapCanvas->setTransformation( ::basegfx::B2DHomMatrix() );

        // clear bitmap to black
        fillRect( pBitmapCanvas,
                  ::basegfx::B2DRectangle( 0.0, 0.0,
                                           slideSizePixel.getX(),
                                           slideSizePixel.getY() ),
                  0x000000FFU );

        pRet.reset( new SlideBitmap( pBitmap ));
    }
    else
    {
        pRet = pSlide->getCurrentSlideBitmap( rView );
    }

    return pRet;
}

::basegfx::B2ISize SlideChangeBase::getEnteringSlideSizePixel( const UnoViewSharedPtr& pView ) const
{
    return getSlideSizePixel( basegfx::B2DSize( mpEnteringSlide->getSlideSize() ),
                              pView );
}

void SlideChangeBase::renderBitmap(
    SlideBitmapSharedPtr const & pSlideBitmap,
    cppcanvas::CanvasSharedPtr const & pCanvas )
{
    if( pSlideBitmap && pCanvas )
    {
        // need to render without any transformation (we
        // assume device units):
        const basegfx::B2DHomMatrix viewTransform(
            pCanvas->getTransformation() );
        const basegfx::B2DPoint pageOrigin(
            viewTransform * basegfx::B2DPoint() );
        const cppcanvas::CanvasSharedPtr pDevicePixelCanvas(
            pCanvas->clone() );

        // render at output position, don't modify bitmap object (no move!):
        const basegfx::B2DHomMatrix transform(basegfx::tools::createTranslateB2DHomMatrix(
            pageOrigin.getX(), pageOrigin.getY()));

        pDevicePixelCanvas->setTransformation( transform );
        pSlideBitmap->draw( pDevicePixelCanvas );
    }
}

void SlideChangeBase::prefetch( const AnimatableShapeSharedPtr&,
                                const ShapeAttributeLayerSharedPtr& )
{
    // we're a one-shot activity, and already finished
    if( mbFinished || mbPrefetched )
        return;

    // register ourselves for view change events
    mrEventMultiplexer.addViewHandler( shared_from_this() );

    // init views and create slide bitmaps
    std::for_each( mrViewContainer.begin(),
                   mrViewContainer.end(),
                   boost::bind( &SlideChangeBase::viewAdded,
                                this,
                                _1 ));

    mbPrefetched = true;
}

void SlideChangeBase::start( const AnimatableShapeSharedPtr&     rShape,
                             const ShapeAttributeLayerSharedPtr& rLayer )
{
    // we're a one-shot activity, and already finished
    if( mbFinished )
        return;

    prefetch(rShape,rLayer); // no-op, if already done

    // get the subclasses a chance to do any specific initialization before run
    for ( ViewsVecT::const_iterator aCurr( beginViews() ), aEnd( endViews() ); aCurr != aEnd; ++aCurr )
        prepareForRun( *aCurr, aCurr->mpView->getCanvas() );

    // start accompanying sound effect, if any
    if( mpSoundPlayer )
    {
        mpSoundPlayer->startPlayback();
        // xxx todo: for now, presentation.cxx takes care about the slide
        // #i50492#  transition sound object, so just release it here
        mpSoundPlayer.reset();
    }
}

void SlideChangeBase::end()
{
    // we're a one-shot activity, and already finished
    if( mbFinished )
        return;

    try
    {
        // draw fully entered bitmap:
        ViewsVecT::const_iterator aCurr( beginViews() );
        const ViewsVecT::const_iterator aEnd( endViews() );
        while( aCurr != aEnd )
        {
            // fully clear view content to background color
            aCurr->mpView->clearAll();

            const SlideBitmapSharedPtr pSlideBitmap( getEnteringBitmap( *aCurr ));
            pSlideBitmap->clip( basegfx::B2DPolyPolygon() /* no clipping */ );
            renderBitmap( pSlideBitmap,
                          aCurr->mpView->getCanvas() );

            ++aCurr;
        }
    }
    catch( uno::Exception& )
    {
        // make sure releasing below happens
    }

    // swap changes to screen
    mrScreenUpdater.notifyUpdate();

    // make object dysfunctional
    mbFinished = true;
    ViewsVecT().swap(maViewData);
    maLeavingSlide.reset();
    mpEnteringSlide.reset();

    // sprites have been binned above
    mbSpritesVisible = false;

    // remove also from event multiplexer, we're dead anyway
    mrEventMultiplexer.removeViewHandler( shared_from_this() );
}

bool SlideChangeBase::operator()( double nValue )
{
    if( mbFinished )
        return false;

    const std::size_t nEntries( maViewData.size() );
    bool bSpritesVisible( mbSpritesVisible );

    for( ::std::size_t i=0; i<nEntries; ++i )
    {
        // calc sprite offsets. The enter/leaving bitmaps are only
        // as large as the actual slides. For scaled-down
        // presentations, we have to move the left, top edge of
        // those bitmaps to the actual position, governed by the
        // given view transform. The aSpritePosPixel local
        // variable is already in device coordinate space
        // (i.e. pixel).

        ViewEntry& rViewEntry( maViewData[i] );
        const ::cppcanvas::CanvasSharedPtr& rCanvas( rViewEntry.mpView->getCanvas() );
        ::cppcanvas::CustomSpriteSharedPtr& rInSprite( rViewEntry.mpInSprite );
        ::cppcanvas::CustomSpriteSharedPtr& rOutSprite( rViewEntry.mpOutSprite );

        // TODO(F2): Properly respect clip here.

        // Might have to be transformed, too.
        const ::basegfx::B2DHomMatrix aViewTransform(
            rViewEntry.mpView->getTransformation() );
        const ::basegfx::B2DPoint aSpritePosPixel(
            aViewTransform * ::basegfx::B2DPoint() );

        // move sprite to final output position, in
        // device coordinates
        if( rOutSprite )
            rOutSprite->movePixel( aSpritePosPixel );
        if( rInSprite )
            rInSprite->movePixel( aSpritePosPixel );

        if( !mbSpritesVisible )
        {
            if( rOutSprite )
            {
                // only render once: clipping is done
                // exclusively with the sprite
                const ::cppcanvas::CanvasSharedPtr pOutContentCanvas(
                    rOutSprite->getContentCanvas() );
                if( pOutContentCanvas)
                {
                    // TODO(Q2): Use basegfx bitmaps here

                    // TODO(F1): SlideBitmap is not fully portable
                    // between different canvases!

                    // render the content
                    OSL_ASSERT( getLeavingBitmap( rViewEntry ) );
                    if( getLeavingBitmap( rViewEntry ) )
                        getLeavingBitmap( rViewEntry )->draw( pOutContentCanvas );
                }
            }

            if( rInSprite )
            {
                // only render once: clipping is done
                // exclusively with the sprite
                const ::cppcanvas::CanvasSharedPtr pInContentCanvas(
                    rInSprite->getContentCanvas() );
                if( pInContentCanvas )
                {
                    // TODO(Q2): Use basegfx bitmaps here

                    // TODO(F1): SlideBitmap is not fully portable
                    // between different canvases!

                    // render the content
                    getEnteringBitmap( rViewEntry )->draw( pInContentCanvas );
                }
            }
        }

        if( rOutSprite )
            performOut( rOutSprite, rViewEntry, rCanvas, nValue );
        if( rInSprite )
            performIn( rInSprite, rViewEntry, rCanvas, nValue );

        // finishing deeds for first run.
        if( !mbSpritesVisible)
        {
            // enable sprites:
            if( rOutSprite )
                rOutSprite->show();
            if( rInSprite )
                rInSprite->show();
            bSpritesVisible = true;
        }
    } // for_each( sprite )

    mbSpritesVisible = bSpritesVisible;
    mrScreenUpdater.notifyUpdate();

    return true;
}

void SlideChangeBase::prepareForRun(
    const ViewEntry& /* rViewEntry */,
    const boost::shared_ptr<cppcanvas::Canvas>& /* rDestinationCanvas */ )
{
}

void SlideChangeBase::performIn(
    const cppcanvas::CustomSpriteSharedPtr&   /*rSprite*/,
    const ViewEntry&                          /*rViewEntry*/,
    const cppcanvas::CanvasSharedPtr&         /*rDestinationCanvas*/,
    double                                    /*t*/ )
{
}

void SlideChangeBase::performOut(
    const cppcanvas::CustomSpriteSharedPtr&  /*rSprite*/,
    const ViewEntry&                         /*rViewEntry*/,
    const cppcanvas::CanvasSharedPtr&        /*rDestinationCanvas*/,
    double                                   /*t*/ )
{
}

double SlideChangeBase::getUnderlyingValue() const
{
    return 0.0;     // though this should be used in concert with
                    // ActivitiesFactory::createSimpleActivity, better
                    // explicitly name our start value.
                    // Permissible range for operator() above is [0,1]
}

void SlideChangeBase::viewAdded( const UnoViewSharedPtr& rView )
{
    // we're a one-shot activity, and already finished
    if( mbFinished )
        return;

    maViewData.push_back( ViewEntry(rView) );

    ViewEntry& rEntry( maViewData.back() );
    getEnteringBitmap( rEntry );
    getLeavingBitmap( rEntry );
    addSprites( rEntry );
}

void SlideChangeBase::viewRemoved( const UnoViewSharedPtr& rView )
{
    // we're a one-shot activity, and already finished
    if( mbFinished )
        return;

    // erase corresponding entry from maViewData
    maViewData.erase(
        std::remove_if(
            maViewData.begin(),
            maViewData.end(),
            boost::bind(
                std::equal_to<UnoViewSharedPtr>(),
                rView,
                // select view:
                boost::bind( &ViewEntry::getView, _1 ))),
        maViewData.end() );
}

void SlideChangeBase::viewChanged( const UnoViewSharedPtr& rView )
{
    // we're a one-shot activity, and already finished
    if( mbFinished )
        return;

    // find entry corresponding to modified view
    ViewsVecT::iterator aModifiedEntry(
        std::find_if(
            maViewData.begin(),
            maViewData.end(),
            boost::bind(
                std::equal_to<UnoViewSharedPtr>(),
                rView,
                // select view:
                boost::bind( &ViewEntry::getView, _1 ) )));

    OSL_ASSERT( aModifiedEntry != maViewData.end() );
    if( aModifiedEntry == maViewData.end() )
        return;

    // clear stale info (both bitmaps and sprites prolly need a
    // resize)
    clearViewEntry( *aModifiedEntry );
    addSprites( *aModifiedEntry );
}

void SlideChangeBase::viewsChanged()
{
    // we're a one-shot activity, and already finished
    if( mbFinished )
        return;

    ViewsVecT::iterator       aIter( maViewData.begin() );
    ViewsVecT::iterator const aEnd ( maViewData.end() );
    while( aIter != aEnd )
    {
        // clear stale info (both bitmaps and sprites prolly need a
        // resize)
        clearViewEntry( *aIter );
        addSprites( *aIter );

        ++aIter;
    }
}

cppcanvas::CustomSpriteSharedPtr SlideChangeBase::createSprite(
    UnoViewSharedPtr const & pView,
    basegfx::B2DSize const & rSpriteSize,
    double                   nPrio ) const
{
    // TODO(P2): change to bitmapsprite once that's working
    const cppcanvas::CustomSpriteSharedPtr pSprite(
        pView->createSprite( rSpriteSize,
                             nPrio ));

    // alpha default is 0.0, which seems to be
    // a bad idea when viewing content...
    pSprite->setAlpha( 1.0 );
    if (mbSpritesVisible)
        pSprite->show();

    return pSprite;
}

void SlideChangeBase::addSprites( ViewEntry& rEntry )
{
    if( mbCreateLeavingSprites && maLeavingSlide )
    {
        // create leaving sprite:
        const basegfx::B2ISize leavingSlideSizePixel(
            getLeavingBitmap( rEntry )->getSize() );

        rEntry.mpOutSprite = createSprite( rEntry.mpView,
                                           basegfx::B2DSize( leavingSlideSizePixel ),
                                           100 );
    }

    if( mbCreateEnteringSprites )
    {
        // create entering sprite:
        const basegfx::B2ISize enteringSlideSizePixel(
            getSlideSizePixel( basegfx::B2DSize( mpEnteringSlide->getSlideSize() ),
                               rEntry.mpView ));

        rEntry.mpInSprite = createSprite( rEntry.mpView,
                                          basegfx::B2DSize( enteringSlideSizePixel ),
                                          101 );
    }
}

void SlideChangeBase::clearViewEntry( ViewEntry& rEntry )
{
    // clear stale info (both bitmaps and sprites prolly need a
    // resize)
    rEntry.mpEnteringBitmap.reset();
    rEntry.mpLeavingBitmap.reset();
    rEntry.mpInSprite.reset();
    rEntry.mpOutSprite.reset();
}

} // namespace internal
} // namespace presentation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
