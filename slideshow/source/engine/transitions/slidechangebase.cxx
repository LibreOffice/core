/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slidechangebase.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 15:45:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

#include <canvas/debug.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <cppcanvas/basegfxfactory.hxx>
#include "slidechangebase.hxx"

#include <boost/bind.hpp>
#include <algorithm>

using namespace com::sun::star;

namespace slideshow {
namespace internal {

SlideChangeBase::SlideChangeBase( boost::optional<SlideSharedPtr> const & leavingSlide,
                                  const SlideSharedPtr&                   pEnteringSlide,
                                  const SoundPlayerSharedPtr&             pSoundPlayer,
                                  const UnoViewContainer&                 rViewContainer,
                                  EventMultiplexer&                       rEventMultiplexer,
                                  bool                                    bCreateLeavingSprites,
                                  bool                                    bCreateEnteringSprites ) :
      mpSoundPlayer( pSoundPlayer ),
      mrViewContainer(rViewContainer),
      mrEventMultiplexer(rEventMultiplexer),
      mLeavingSlide( leavingSlide ),
      mpEnteringSlide( pEnteringSlide ),
      maViewData(),
      mbCreateLeavingSprites(bCreateLeavingSprites),
      mbCreateEnteringSprites(bCreateEnteringSprites),
      mbSpritesVisible(false)
{
    ENSURE_AND_THROW(
        pEnteringSlide,
        "SlideChangeBase::SlideChangeBase(): Invalid entering slide!" );
}

SlideBitmapSharedPtr SlideChangeBase::getLeavingBitmap( const ViewEntry& rViewEntry ) const
{
    if( !rViewEntry.mpLeavingBitmap )
        rViewEntry.mpLeavingBitmap = createBitmap(rViewEntry.mpView,
                                                  mLeavingSlide);

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
            getEnteringSizePixel( rView ));

        cppcanvas::CanvasSharedPtr pCanvas( rView->getCanvas() );

        // create a bitmap of appropriate size
        cppcanvas::BitmapSharedPtr pBitmap(
            cppcanvas::BaseGfxFactory::getInstance().createBitmap(
                pCanvas,
                slideSizePixel ) );

        ENSURE_AND_THROW(
            pBitmap,
            "SlideChangeBase::createBitmap(): Cannot create page bitmap" );

        cppcanvas::BitmapCanvasSharedPtr pBitmapCanvas(
            pBitmap->getBitmapCanvas() );

        ENSURE_AND_THROW( pBitmapCanvas,
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

basegfx::B2ISize SlideChangeBase::getEnteringSizePixel( const UnoViewSharedPtr& pView ) const
{
    return mpEnteringSlide->getSlideSizePixel( pView );
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
        basegfx::B2DHomMatrix transform;
        // render at output position, don't modify bitmap object (no move!):
        transform.translate( pageOrigin.getX(), pageOrigin.getY() );

        pDevicePixelCanvas->setTransformation( transform );
        pSlideBitmap->draw( pDevicePixelCanvas );
    }
}

void SlideChangeBase::start( const AnimatableShapeSharedPtr&,
                             const ShapeAttributeLayerSharedPtr& )
{
    // register ourselves for view change events
    mrEventMultiplexer.addViewHandler( shared_from_this() );

    // init views and create slide bitmaps
    std::for_each( mrViewContainer.begin(),
                   mrViewContainer.end(),
                   boost::bind( &SlideChangeBase::viewAdded,
                                this,
                                _1 ));

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
    try
    {
        // draw fully entered bitmap:
        ViewsVecT::const_iterator aCurr( beginViews() );
        const ViewsVecT::const_iterator aEnd( endViews() );
        while( aCurr != aEnd )
        {
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

    // TODO(P3): Slide::show() initial Sliderendering may be obsolete
    // now
    mbSpritesVisible = false;

    // drop all references
    ViewsVecT().swap(maViewData);
    mLeavingSlide.reset();
    mpEnteringSlide.reset();
}

bool SlideChangeBase::operator()( double nValue )
{
    if( maViewData.empty() )
        return false;

    const std::size_t nEntries( maViewData.size() );
    ENSURE_AND_RETURN(
        mrViewContainer.size() == nEntries,
        "SlideChangeBase::operator(): Mismatching sprite/view numbers" );

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
            rCanvas->getTransformation() );
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

    return true;
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
                    // explicitely name our start value.
                    // Permissible range for operator() above is [0,1]
}

cppcanvas::CustomSpriteSharedPtr SlideChangeBase::createSprite(
    UnoViewSharedPtr const & pView,
    basegfx::B2DSize const & rSpriteSize,
    double                   nPrio ) const
{
    // TODO(P2): change to bitmapsprite once that's working
    const cppcanvas::CustomSpriteSharedPtr pSprite(
        pView->createSprite( rSpriteSize ));

    // alpha default is 0.0, which seems to be
    // a bad idea when viewing content...
    pSprite->setAlpha( 1.0 );
    pSprite->setPriority( nPrio );
    if (mbSpritesVisible)
        pSprite->show();

    return pSprite;
}

void SlideChangeBase::addSprites( ViewEntry& rEntry )
{
    if( mbCreateLeavingSprites && mLeavingSlide )
    {
        // create leaving sprite:
        const basegfx::B2ISize leavingSlideSizePixel(
            getLeavingBitmap( rEntry )->getSize() );

        rEntry.mpOutSprite = createSprite( rEntry.mpView,
                                           leavingSlideSizePixel,
                                           100 );
    }

    if( mbCreateEnteringSprites )
    {
        // create entering sprite:
        const basegfx::B2ISize enteringSlideSizePixel(
            mpEnteringSlide->getSlideSizePixel( rEntry.mpView ) );

        rEntry.mpInSprite = createSprite( rEntry.mpView,
                                          enteringSlideSizePixel,
                                          101 );
    }
}

void SlideChangeBase::viewAdded( const UnoViewSharedPtr& rView )
{
    maViewData.push_back( ViewEntry(rView) );

    ViewEntry& rEntry( maViewData.back() );
    getEnteringBitmap( rEntry );
    getLeavingBitmap( rEntry );
    addSprites( rEntry );
}

void SlideChangeBase::viewRemoved( const UnoViewSharedPtr& rView )
{
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
    if( aModifiedEntry != maViewData.end() )
        return;

    // clear stale info (both bitmaps and sprites prolly need a
    // resize)
    aModifiedEntry->mpEnteringBitmap.reset();
    aModifiedEntry->mpLeavingBitmap.reset();
    aModifiedEntry->mpInSprite.reset();
    aModifiedEntry->mpOutSprite.reset();

    addSprites( *aModifiedEntry );
}

} // namespace internal
} // namespace presentation
