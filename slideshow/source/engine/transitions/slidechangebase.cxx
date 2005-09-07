/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slidechangebase.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:58:03 $
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

#include "slidechangebase.hxx"
#include "canvas/debug.hxx"
#include "basegfx/numeric/ftools.hxx"
#include "basegfx/polygon/b2dpolygontools.hxx"
#include "basegfx/polygon/b2dpolypolygontools.hxx"
#include "cppcanvas/basegfxfactory.hxx"
#include "boost/bind.hpp"
#include <algorithm>

using namespace com::sun::star;
namespace css = com::sun::star; // resolving conflicts with ::presentation

namespace presentation {
namespace internal {

// XModifyListener
void SlideChangeBase::ModifyListener::modified( lang::EventObject const & evt )
    throw (uno::RuntimeException)
{
    if (m_pSlideChangeBase != 0) {
        m_pSlideChangeBase->notifyViewChange( evt );
    }
}

// XEventListener
void SlideChangeBase::ModifyListener::disposing( lang::EventObject const & evt )
    throw (uno::RuntimeException)
{
    const uno::Reference<css::presentation::XSlideShowView> xSlideShowView(
        evt.Source, uno::UNO_QUERY );
    if (xSlideShowView.is() && m_pSlideChangeBase != 0)
    {
        const UnoViewSharedPtr pView(
            m_pSlideChangeBase->findUnoView( xSlideShowView ) );
        if (pView.get() != 0)
            m_pSlideChangeBase->removeView( pView );
    }
}

SlideChangeBase::~SlideChangeBase()
{
}

SlideChangeBase::SlideChangeBase(
    boost::optional<SlideSharedPtr> const & leavingSlide,
    const SlideSharedPtr& pEnteringSlide,
    const SoundPlayerSharedPtr& pSoundPlayer,
    bool bCreateLeavingSprites, bool bCreateEnteringSprites )
    : maMutex(),
      maViews(),
      mpSoundPlayer( pSoundPlayer ),
      mLeavingSlide( leavingSlide ),
      mpEnteringSlide( pEnteringSlide ),
      mpLeavingBitmap(),
      mpEnteringBitmap(),
      maOutSprites(),
      maInSprites(),
      mbCreateLeavingSprites(bCreateLeavingSprites),
      mbCreateEnteringSprites(bCreateEnteringSprites),
      mbSpritesVisible(false),
      mxModifyListener()
{
    ENSURE_AND_THROW(
        pEnteringSlide.get(),
        "SlideChangeBase::SlideChangeBase(): Invalid entering slide!" );
}

SlideBitmapSharedPtr SlideChangeBase::getLeavingBitmap() const
{
    osl::ResettableMutexGuard guard(maMutex);
    if (mpLeavingBitmap.get() == 0) {
        guard.clear();
        const SlideBitmapSharedPtr pBitmap( createBitmap(mLeavingSlide) );
        guard.reset();
        if (mpLeavingBitmap.get() == 0)
            mpLeavingBitmap = pBitmap;
    }
    return mpLeavingBitmap;
}

SlideBitmapSharedPtr SlideChangeBase::getEnteringBitmap() const
{
    osl::ResettableMutexGuard guard(maMutex);
    if (mpEnteringBitmap.get() == 0) {
        guard.clear();
        const SlideBitmapSharedPtr pBitmap(
            createBitmap( boost::optional<SlideSharedPtr>(mpEnteringSlide) ) );
        guard.reset();
        if (mpEnteringBitmap.get() == 0)
            mpEnteringBitmap = pBitmap;
    }
    return mpEnteringBitmap;
}

SlideBitmapSharedPtr SlideChangeBase::createBitmap(
    boost::optional<SlideSharedPtr> const & rSlide ) const
{
    SlideBitmapSharedPtr pRet;
    OSL_ASSERT( ! maViews.empty() );
    if (rSlide && !maViews.empty())
    {
        SlideSharedPtr const & pSlide = *rSlide;
        if (pSlide.get() == 0)
        {
            // TODO(F2): Generalize to multiple, multi-device views
            const basegfx::B2ISize slideSizePixel(
                getEnteringSizePixel( maViews.front() ) );

            cppcanvas::CanvasSharedPtr pCanvas(
                maViews.front()->getCanvas() );

            // create a bitmap of appropriate size
            cppcanvas::BitmapSharedPtr pBitmap(
                cppcanvas::BaseGfxFactory::getInstance().createBitmap(
                    pCanvas,
                    slideSizePixel ) );

            ENSURE_AND_THROW(
                pBitmap.get(),
                "SlideChangeBase::createBitmap(): Cannot create page bitmap" );

            cppcanvas::BitmapCanvasSharedPtr pBitmapCanvas(
                pBitmap->getBitmapCanvas() );

            ENSURE_AND_THROW( pBitmapCanvas.get(),
                              "SlideChangeBase::createBitmap(): "
                              "Cannot create page bitmap canvas" );

            // set transformation to identitiy (->device pixel)
            pBitmapCanvas->setTransformation( ::basegfx::B2DHomMatrix() );

            // fill the bounds rectangle in white
            const ::basegfx::B2DPolygon aPoly(
                ::basegfx::tools::createPolygonFromRect(
                    ::basegfx::B2DRectangle( 0.0, 0.0,
                                             slideSizePixel.getX(),
                                             slideSizePixel.getY() ) ) );

            ::cppcanvas::PolyPolygonSharedPtr pPolyPoly(
                ::cppcanvas::BaseGfxFactory::getInstance().createPolyPolygon( pBitmapCanvas,
                                                                              aPoly ) );

            if( pPolyPoly.get() )
            {
                pPolyPoly->setRGBAFillColor( 0x000000FFU );
                pPolyPoly->draw();
            }

            pRet = SlideBitmapSharedPtr( new SlideBitmap( pBitmap ) );
        }
        else
        {
            // TODO(F2): Generalize to multiple, multi-device views
            pRet = pSlide->getCurrentSlideBitmap( *maViews.begin() );
        }
    }
    return pRet;
}

basegfx::B2ISize SlideChangeBase::getEnteringSizePixel(
    UnoViewSharedPtr const & pView ) const
{
    return mpEnteringSlide->getSlideSizePixel( pView );
}

basegfx::B2DSize SlideChangeBase::getEnteringSize() const
{
    return mpEnteringSlide->getSlideSize();
}

void SlideChangeBase::renderBitmap(
    SlideBitmapSharedPtr const & pSlideBitmap,
    cppcanvas::CanvasSharedPtr const & pCanvas )
{
    if (pSlideBitmap.get() != 0 && pCanvas.get() != 0)
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
    // create slide bitmaps
    getEnteringBitmap();
    getLeavingBitmap();

    // create sprites:
    std::for_each( maViews.begin(), maViews.end(),
                   boost::bind( &SlideChangeBase::addSprites, this, _1 ) );

    // start accompanying sound effect, if any
    if( mpSoundPlayer.get() )
        mpSoundPlayer->startPlayback();
}

void SlideChangeBase::end()
{
    // end accompanying sound effect, if any
    if (mpSoundPlayer.get() != 0) {
        mpSoundPlayer->stopPlayback();
        mpSoundPlayer->dispose();
        mpSoundPlayer.reset();
    }

    // draw fully entered bitmap:
    const SlideBitmapSharedPtr pSlideBitmap( getEnteringBitmap() );
    pSlideBitmap->clip( basegfx::B2DPolyPolygon() /* no clipping */ );
    for_each_canvas( boost::bind( &SlideChangeBase::renderBitmap, this,
                                  boost::cref(pSlideBitmap), _1 ) );
    // TODO: Slide::show() initial Sliderendering may be obsolete now

    mbSpritesVisible = false;

    // drop all references
    mLeavingSlide.reset();
    mpEnteringSlide.reset();
    {
        const osl::MutexGuard guard(maMutex);
        maOutSprites.clear();
        maInSprites.clear();
        mpEnteringBitmap.reset();
        mpLeavingBitmap.reset();
    }

    if (mxModifyListener.is())
    {
        UnoViewVector::const_iterator iPos( maViews.begin() );
        const UnoViewVector::const_iterator iEnd( maViews.end() );
        for ( ; iPos != iEnd; ++iPos )
        {
            const uno::Reference<css::presentation::XSlideShowView>
                xSlideShowView( (*iPos)->getUnoView(), uno::UNO_QUERY );
            if (xSlideShowView.is()) {
                xSlideShowView->removeTransformationChangedListener(
                    getModifyListener().get() );
            }
        }
        // for safety:
        mxModifyListener->m_pSlideChangeBase = 0;
        mxModifyListener.clear();
    }
    maViews.clear();
}

bool SlideChangeBase::operator()( double nValue )
{
    SpriteVector aInSprites, aOutSprites;
    {
        const osl::MutexGuard guard(maMutex);
        if (maInSprites.empty() && maOutSprites.empty())
            return false;
        aInSprites = maInSprites;
        aOutSprites = maOutSprites;
    }

    const std::size_t nEntries = std::max<std::size_t>(aInSprites.size(),
                                                       aOutSprites.size());
    ENSURE_AND_RETURN(
        maViews.size() == nEntries,
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

        UnoViewSharedPtr const & pView = maViews[i];
        ::cppcanvas::CanvasSharedPtr const & pCanvas = pView->getCanvas();
        ::cppcanvas::CustomSpriteSharedPtr pInSprite, pOutSprite;

        if (i < aInSprites.size())
            pInSprite = aInSprites[i];
        if (i < aOutSprites.size())
            pOutSprite = aOutSprites[i];

        // TODO(F2): Properly respect clip here.

        // Might have to be transformed, too.
        const ::basegfx::B2DHomMatrix aViewTransform(
            pCanvas->getTransformation() );
        const ::basegfx::B2DPoint aSpritePosPixel(
            aViewTransform * ::basegfx::B2DPoint() );

        // move sprite to final output position, in
        // device coordinates
        if (pOutSprite.get() != 0)
            pOutSprite->movePixel( aSpritePosPixel );
        if (pInSprite.get() != 0)
            pInSprite->movePixel( aSpritePosPixel );

        if (! mbSpritesVisible)
        {
            if (pOutSprite.get() != 0)
            {
                // only render once: clipping is done
                // exclusively with the sprite
                const ::cppcanvas::CanvasSharedPtr pOutContentCanvas(
                    pOutSprite->getContentCanvas() );
                if (pOutContentCanvas.get() != 0) {
                    // TODO(Q2): Use basegfx bitmaps here

                    // TODO(F1): SlideBitmap is not fully portable
                    // between different canvases!

                    // render the content
                    OSL_ASSERT( getLeavingBitmap().get() != 0 );
                    if (getLeavingBitmap().get() != 0)
                        getLeavingBitmap()->draw( pOutContentCanvas );
                }
            }

            if (pInSprite.get() != 0) {
                // only render once: clipping is done
                // exclusively with the sprite
                const ::cppcanvas::CanvasSharedPtr pInContentCanvas(
                    pInSprite->getContentCanvas() );
                if (pInContentCanvas.get() != 0) {
                    // TODO(Q2): Use basegfx bitmaps here

                    // TODO(F1): SlideBitmap is not fully portable
                    // between different canvases!

                    // render the content
                    getEnteringBitmap()->draw( pInContentCanvas );
                }
            }
        }

        if (pOutSprite.get() != 0)
            performOut( pOutSprite, pView, pCanvas, nValue );
        if (pInSprite.get() != 0)
            performIn( pInSprite, pView, pCanvas, nValue );

        // finishing deeds for first run.
        if (! mbSpritesVisible) {
            // enable sprites:
            if (pOutSprite.get() != 0)
                pOutSprite->show();
            if (pInSprite.get() != 0)
                pInSprite->show();
            bSpritesVisible = true;
        }
    } // for_each( sprite )

    mbSpritesVisible = bSpritesVisible;

    return true;
}

void SlideChangeBase::performIn(
    const cppcanvas::CustomSpriteSharedPtr&   rSprite,
    UnoViewSharedPtr const &                  pView,
    const cppcanvas::CanvasSharedPtr&         rDestinationCanvas,
    double                                    t )
{
}

void SlideChangeBase::performOut(
    const cppcanvas::CustomSpriteSharedPtr&  rSprite,
    UnoViewSharedPtr const &                 pView,
    const cppcanvas::CanvasSharedPtr&        rDestinationCanvas,
    double                                   t )
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
    UnoViewSharedPtr const & pView, basegfx::B2DSize const & rSpriteSize ) const
{
    // TODO(P2): change to bitmapsprite once that's working
    const cppcanvas::CustomSpriteSharedPtr pSprite(
        pView->createSprite( rSpriteSize ) );
    // alpha default is 0.0, which seems to be
    // a bad idea when viewing content...
    pSprite->setAlpha( 1.0 );
    if (mbSpritesVisible)
        pSprite->show();
    return pSprite;
}

void SlideChangeBase::addSprites( UnoViewSharedPtr const & pView )
{
    if (mbCreateLeavingSprites && mLeavingSlide)
    {
        // create leaving sprite:
        const basegfx::B2ISize leavingSlideSizePixel(
            getLeavingBitmap()->getSize() );

        const cppcanvas::CustomSpriteSharedPtr pLeavingSprite(
            createSprite( pView, leavingSlideSizePixel ) );

        const osl::MutexGuard guard(maMutex);
        maOutSprites.push_back( pLeavingSprite );
    }

    if (mbCreateEnteringSprites)
    {
        // create entering sprite:
        const basegfx::B2ISize enteringSlideSizePixel(
            mpEnteringSlide->getSlideSizePixel( pView ) );

        const cppcanvas::CustomSpriteSharedPtr pEnteringSprite(
            createSprite( pView, enteringSlideSizePixel ) );

        const osl::MutexGuard guard(maMutex);
        maInSprites.push_back( pEnteringSprite );
    }
}

void SlideChangeBase::notifyViewChange( lang::EventObject const & evt )
{
    const uno::Reference<css::presentation::XSlideShowView> xSlideShowView(
        evt.Source, uno::UNO_QUERY );
    if (xSlideShowView.is())
    {
        const UnoViewSharedPtr pView( findUnoView( xSlideShowView ) );
        OSL_ASSERT( pView.get() );
        if (pView.get() != 0)
        {
            // invalidate bitmaps and sprites for next update():
            {
                const osl::MutexGuard guard(maMutex);
                mpLeavingBitmap.reset();
                mpEnteringBitmap.reset();
                maOutSprites.clear();
                maInSprites.clear();
            }

            // and create new ones (!in same order as added views!):
            std::for_each(
                maViews.begin(), maViews.end(),
                boost::bind( &SlideChangeBase::addSprites, this, _1 ) );
        }
    }
}

UnoViewSharedPtr SlideChangeBase::findUnoView(
    uno::Reference<css::presentation::XSlideShowView> const & xSlideShowView )
    const
{
    UnoViewVector::const_iterator iFind;
    const UnoViewVector::const_iterator iBegin( maViews.begin() );
    const UnoViewVector::const_iterator iEnd( maViews.end() );
    if ((iFind = std::find_if(
             iBegin, iEnd,
             boost::bind(
                 std::equal_to<
                     uno::Reference<css::presentation::XSlideShowView> >(),
                 boost::bind( &UnoView::getUnoView, _1 ),
                 boost::cref(xSlideShowView) ) )) != iEnd)
    {
        return *iFind;
    }
    else
        return UnoViewSharedPtr();
}

void SlideChangeBase::addView( UnoViewSharedPtr const & pView )
{
    const UnoViewVector::iterator iEnd( maViews.end() );
    if (std::find( maViews.begin(), iEnd, pView ) == iEnd)
    {
        // listen for view changes:
        const uno::Reference<css::presentation::XSlideShowView> xSlideShowView(
            pView->getUnoView(), uno::UNO_QUERY_THROW );
        xSlideShowView->addTransformationChangedListener(
            getModifyListener().get() );
        maViews.push_back( pView );
    }
}

bool SlideChangeBase::removeView( UnoViewSharedPtr const & pView )
{
    UnoViewVector::iterator iBegin( maViews.begin() );
    const UnoViewVector::iterator iEnd( maViews.end() );
    UnoViewVector::iterator iPos;
    if ((iPos = std::find( iBegin, iEnd, pView )) == iEnd) {
        // view seemingly was not added, failed
        return false;
    }

    // don't listen for view changes anymore:
    const uno::Reference<css::presentation::XSlideShowView> xSlideShowView(
        pView->getUnoView(), uno::UNO_QUERY_THROW );
    xSlideShowView->removeTransformationChangedListener(
        getModifyListener().get() );

    const std::size_t dist = std::distance( iBegin, iPos );
    maViews.erase( iPos );
    {
        const osl::MutexGuard guard(maMutex);
        if (mLeavingSlide && dist < maOutSprites.size())
            maOutSprites.erase( maOutSprites.begin() + dist );
        if (dist < maInSprites.size())
            maInSprites.erase( maInSprites.begin() + dist );
    }
    return true;
}

} // namespace internal
} // namespace presentation
