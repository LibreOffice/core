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

#include <tools/diagnose_ex.h>
#include <sal/log.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

#include <cppcanvas/basegfxfactory.hxx>
#include <cppcanvas/customsprite.hxx>

#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/rendering/IntegerBitmapLayout.hpp>
#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "slidechangebase.hxx"
#include <transitionfactory.hxx>
#include "transitionfactorytab.hxx"
#include "transitiontools.hxx"
#include "parametricpolypolygonfactory.hxx"
#include <animationfactory.hxx>
#include "clippingfunctor.hxx"
#include "combtransition.hxx"
#include <tools.hxx>
#include <memory>


/***************************************************
 ***                                             ***
 ***          Slide Transition Effects           ***
 ***                                             ***
 ***************************************************/

using namespace com::sun::star;

namespace slideshow {
namespace internal {

namespace {

// helper methods
// =============================================

void fillPage( const ::cppcanvas::CanvasSharedPtr& rDestinationCanvas,
               const ::basegfx::B2DSize&           rPageSizePixel,
               const RGBColor&                     rFillColor )
{
    // need to render without any transformation (we
    // assume rPageSizePixel to represent device units)
    const ::cppcanvas::CanvasSharedPtr pDevicePixelCanvas(
        rDestinationCanvas->clone() );
    pDevicePixelCanvas->setTransformation( ::basegfx::B2DHomMatrix() );

    // TODO(F2): Properly respect clip here.
    // Might have to be transformed, too.
    const ::basegfx::B2DHomMatrix aViewTransform(
        rDestinationCanvas->getTransformation() );
    const ::basegfx::B2DPoint aOutputPosPixel(
        aViewTransform * ::basegfx::B2DPoint() );

    fillRect( pDevicePixelCanvas,
              ::basegfx::B2DRectangle(
                  aOutputPosPixel.getX(),
                  aOutputPosPixel.getY(),
                  aOutputPosPixel.getX() + rPageSizePixel.getX(),
                  aOutputPosPixel.getY() + rPageSizePixel.getY() ),
              rFillColor.getIntegerColor() );
}

class PluginSlideChange: public SlideChangeBase
{
    struct TransitionViewPair {
    uno::Reference<presentation::XTransition> mxTransition;
    UnoViewSharedPtr mpView;

    TransitionViewPair( uno::Reference<presentation::XTransition> const & xTransition, const UnoViewSharedPtr& rView )
    {
        mxTransition = xTransition;
        mpView = rView;
    }

    ~TransitionViewPair()
    {
        mxTransition.clear();
        mpView.reset();
    }

    void update( double t )
    {
        mxTransition->update( t );
    }
    };

public:
    /** Create a new SlideChanger, for the given leaving and
        entering slide bitmaps, which uses super secret OpenGL
        stuff.
    */
    PluginSlideChange( sal_Int16                                nTransitionType,
                       sal_Int16                                nTransitionSubType,
                       boost::optional<SlideSharedPtr> const&   leavingSlide_,
                       const SlideSharedPtr&                    pEnteringSlide,
                       const UnoViewContainer&                  rViewContainer,
                       ScreenUpdater&                           rScreenUpdater,
                       const uno::Reference<
                             presentation::XTransitionFactory>& xFactory,
                       const SoundPlayerSharedPtr&              pSoundPlayer,
                       EventMultiplexer&                        rEventMultiplexer) :
        SlideChangeBase( leavingSlide_,
                         pEnteringSlide,
                         pSoundPlayer,
                         rViewContainer,
                         rScreenUpdater,
                         rEventMultiplexer ),
        maTransitions(),
        mbSuccess( false ),
        mnTransitionType( nTransitionType ),
        mnTransitionSubType( nTransitionSubType ),
        mxFactory( xFactory )
    {
        // create one transition per view
        for( const auto& rView : rViewContainer )
        {
            if( !addTransition( rView ) )
                return;

            ENSURE_OR_THROW(maTransitions.back() && maTransitions.back()->mxTransition.is(),
                            "Failed to create plugin transition");
        }
        mbSuccess = true;
    }

    virtual ~PluginSlideChange() override
    {
        mxFactory.clear();
    }

    bool addTransition( const UnoViewSharedPtr& rView )
    {
        uno::Reference<presentation::XTransition> rTransition = mxFactory->createTransition(
            mnTransitionType,
            mnTransitionSubType,
            rView->getUnoView(),
            getLeavingBitmap(ViewEntry(rView))->getXBitmap(),
            getEnteringBitmap(ViewEntry(rView))->getXBitmap() );

        if( rTransition.is() )
            maTransitions.emplace_back( new TransitionViewPair( rTransition, rView ) );
        else
            return false;

        return true;
    }

    virtual bool operator()( double t ) override
    {
        for( const auto& pTransition : maTransitions )
            pTransition->update( t );
        return true;
    }

    bool Success()
    {
        return mbSuccess;
    }

    // ViewEventHandler
    virtual void viewAdded( const UnoViewSharedPtr& rView ) override
    {
        SAL_INFO("slideshow", "PluginSlideChange viewAdded");
        SlideChangeBase::viewAdded( rView );

        for( const auto& pCurrView : maTransitions )
        {
            if( pCurrView->mpView == rView )
                return;
        }

        SAL_INFO("slideshow", "need to be added" );
        addTransition( rView );
    }

    virtual void viewRemoved( const UnoViewSharedPtr& rView ) override
    {
        SAL_INFO("slideshow", "PluginSlideChange viewRemoved");
        SlideChangeBase::viewRemoved( rView );

        auto aIter = std::find_if(maTransitions.begin(), maTransitions.end(),
            [&rView](const std::unique_ptr<TransitionViewPair>& rxTransition) { return rxTransition->mpView == rView; });
        if (aIter != maTransitions.end())
        {
            SAL_INFO("slideshow", "view removed" );
            maTransitions.erase( aIter );
        }
    }

    virtual void viewChanged( const UnoViewSharedPtr& rView ) override
    {
        SAL_INFO("slideshow", "PluginSlideChange viewChanged");
        SlideChangeBase::viewChanged( rView );

        for( const auto& pCurrView : maTransitions )
        {
            if( pCurrView->mpView == rView )
            {
                SAL_INFO("slideshow", "view changed" );
                pCurrView->mxTransition->viewChanged( rView->getUnoView(),
                                                      getLeavingBitmap(ViewEntry(rView))->getXBitmap(),
                                                      getEnteringBitmap(ViewEntry(rView))->getXBitmap() );
            }
            else
                SAL_INFO("slideshow", "view did not change" );
        }
    }

    virtual void viewsChanged() override
    {
        SAL_INFO("slideshow", "PluginSlideChange viewsChanged");
        SlideChangeBase::viewsChanged();

        for( const auto& pCurrView : maTransitions )
        {
            SAL_INFO("slideshow", "view changed" );
            UnoViewSharedPtr pView = pCurrView->mpView;
            pCurrView->mxTransition->viewChanged( pView->getUnoView(),
                                                  getLeavingBitmap(ViewEntry(pView))->getXBitmap(),
                                                  getEnteringBitmap(ViewEntry(pView))->getXBitmap() );
        }
    }

private:
    // One transition object per view
    std::vector< std::unique_ptr<TransitionViewPair> > maTransitions;

    // bool
    bool mbSuccess;

    sal_Int16 const mnTransitionType;
    sal_Int16 const mnTransitionSubType;

    uno::Reference<presentation::XTransitionFactory> mxFactory;
};

class ClippedSlideChange : public SlideChangeBase
{
public:
    /** Create a new SlideChanger, for the given leaving and
        entering slide bitmaps, which applies the given clip
        polygon.
    */
    ClippedSlideChange(
        const SlideSharedPtr&                   pEnteringSlide,
        const ParametricPolyPolygonSharedPtr&   rPolygon,
        const TransitionInfo&                   rTransitionInfo,
        const UnoViewContainer&                 rViewContainer,
        ScreenUpdater&                          rScreenUpdater,
        EventMultiplexer&                       rEventMultiplexer,
        bool                                    bDirectionForward,
        const SoundPlayerSharedPtr&             pSoundPlayer ) :
        SlideChangeBase(
            // leaving bitmap is empty, we're leveraging the fact that the
            // old slide is still displayed in the background:
            boost::optional<SlideSharedPtr>(),
            pEnteringSlide,
            pSoundPlayer,
            rViewContainer,
            rScreenUpdater,
            rEventMultiplexer ),
        maClippingFunctor( rPolygon,
                           rTransitionInfo,
                           bDirectionForward,
                           true )
        {}

    virtual void performIn(
        const ::cppcanvas::CustomSpriteSharedPtr&   rSprite,
        const ViewEntry&                            rViewEntry,
        const ::cppcanvas::CanvasSharedPtr&         rDestinationCanvas,
        double                                      t ) override;

    virtual void performOut(
        const ::cppcanvas::CustomSpriteSharedPtr&  rSprite,
        const ViewEntry&                           rViewEntry,
        const ::cppcanvas::CanvasSharedPtr&        rDestinationCanvas,
        double                                     t ) override;

private:
    ClippingFunctor             maClippingFunctor;
};

void ClippedSlideChange::performIn(
    const ::cppcanvas::CustomSpriteSharedPtr&   rSprite,
    const ViewEntry&                            rViewEntry,
    const ::cppcanvas::CanvasSharedPtr&         /*rDestinationCanvas*/,
    double                                      t )
{
    // #i46602# Better work in device coordinate space here,
    // otherwise, we too easily suffer from roundoffs. Apart from
    // that, getEnteringSizePixel() _guarantees_ to cover the whole
    // slide bitmap. There's a catch, though: this removes any effect
    // of the view transformation (e.g. rotation) from the transition.
    rSprite->setClipPixel(
        maClippingFunctor( t,
                           ::basegfx::B2DSize( getEnteringSlideSizePixel(rViewEntry.mpView) ) ) );
}

void ClippedSlideChange::performOut(
    const ::cppcanvas::CustomSpriteSharedPtr&  /*rSprite*/,
    const ViewEntry&                           /*rViewEntry*/,
    const ::cppcanvas::CanvasSharedPtr&        /*rDestinationCanvas*/,
    double                                     /*t*/ )
{
    // not needed here
}


class FadingSlideChange : public SlideChangeBase
{
public:
    /** Create a new SlideChanger, for the given leaving and
        entering slides, which applies a fade effect.
    */
    FadingSlideChange(
        boost::optional<SlideSharedPtr> const & leavingSlide,
        const SlideSharedPtr&                   pEnteringSlide,
        boost::optional<RGBColor> const&        rFadeColor,
        const SoundPlayerSharedPtr&             pSoundPlayer,
        const UnoViewContainer&                 rViewContainer,
        ScreenUpdater&                          rScreenUpdater,
        EventMultiplexer&                       rEventMultiplexer )
        : SlideChangeBase( leavingSlide,
                           pEnteringSlide,
                           pSoundPlayer,
                           rViewContainer,
                           rScreenUpdater,
                           rEventMultiplexer ),
          maFadeColor( rFadeColor )
        {}

    virtual void prepareForRun(
        const ViewEntry& rViewEntry,
        const cppcanvas::CanvasSharedPtr& rDestinationCanvas ) override;

    virtual void performIn(
        const ::cppcanvas::CustomSpriteSharedPtr&   rSprite,
        const ViewEntry&                            rViewEntry,
        const ::cppcanvas::CanvasSharedPtr&         rDestinationCanvas,
        double                                      t ) override;

    virtual void performOut(
        const ::cppcanvas::CustomSpriteSharedPtr&  rSprite,
        const ViewEntry&                           rViewEntry,
        const ::cppcanvas::CanvasSharedPtr&        rDestinationCanvas,
        double                                     t ) override;

private:
    const boost::optional< RGBColor >               maFadeColor;
};

void FadingSlideChange::prepareForRun(
    const ViewEntry& rViewEntry,
    const cppcanvas::CanvasSharedPtr& rDestinationCanvas )
{
    if ( maFadeColor )
    {
        // clear page to given fade color. 'Leaving' slide is
        // painted atop of that, but slowly fading out.
        fillPage( rDestinationCanvas,
                ::basegfx::B2DSize( getEnteringSlideSizePixel( rViewEntry.mpView ) ),
                *maFadeColor );
    }
}

void FadingSlideChange::performIn(
    const ::cppcanvas::CustomSpriteSharedPtr&   rSprite,
    const ViewEntry&                            /*rViewEntry*/,
    const ::cppcanvas::CanvasSharedPtr&         /*rDestinationCanvas*/,
    double                                      t )
{
    ENSURE_OR_THROW(
        rSprite,
        "FadingSlideChange::performIn(): Invalid sprite" );

    if( maFadeColor )
        // After half of the active time, fade in new slide
        rSprite->setAlpha( t > 0.5 ? 2.0*(t-0.5) : 0.0 );
    else
        // Fade in new slide over full active time
        rSprite->setAlpha( t );
}

void FadingSlideChange::performOut(
    const ::cppcanvas::CustomSpriteSharedPtr&  rSprite,
    const ViewEntry&                           /* rViewEntry */,
    const ::cppcanvas::CanvasSharedPtr&        rDestinationCanvas,
    double                                     t )
{
    ENSURE_OR_THROW(
        rSprite,
        "FadingSlideChange::performOut(): Invalid sprite" );
    ENSURE_OR_THROW(
        rDestinationCanvas,
        "FadingSlideChange::performOut(): Invalid dest canvas" );

    // only needed for color fades
    if( maFadeColor )
    {
        // Until half of the active time, fade out old
        // slide. After half of the active time, old slide
        // will be invisible.
        rSprite->setAlpha( t > 0.5 ? 0.0 : 2.0*(0.5-t) );
    }
}

class CutSlideChange : public SlideChangeBase
{
public:
    /** Create a new SlideChanger, for the given leaving and
        entering slides, which applies a cut effect.
    */
    CutSlideChange(
        boost::optional<SlideSharedPtr> const & leavingSlide,
        const SlideSharedPtr&                   pEnteringSlide,
        const RGBColor&                          rFadeColor,
        const SoundPlayerSharedPtr&             pSoundPlayer,
        const UnoViewContainer&                 rViewContainer,
        ScreenUpdater&                          rScreenUpdater,
        EventMultiplexer&                       rEventMultiplexer )
        : SlideChangeBase( leavingSlide,
                           pEnteringSlide,
                           pSoundPlayer,
                           rViewContainer,
                           rScreenUpdater,
                           rEventMultiplexer ),
          maFadeColor( rFadeColor )
        {}

    virtual void prepareForRun(
        const ViewEntry& rViewEntry,
        const cppcanvas::CanvasSharedPtr& rDestinationCanvas ) override;

    virtual void performIn(
        const ::cppcanvas::CustomSpriteSharedPtr&   rSprite,
        const ViewEntry&                            rViewEntry,
        const ::cppcanvas::CanvasSharedPtr&         rDestinationCanvas,
        double                                      t ) override;

    virtual void performOut(
        const ::cppcanvas::CustomSpriteSharedPtr&  rSprite,
        const ViewEntry&                           rViewEntry,
        const ::cppcanvas::CanvasSharedPtr&        rDestinationCanvas,
        double                                     t ) override;

private:
    RGBColor const maFadeColor;
};

void CutSlideChange::prepareForRun(
    const ViewEntry& rViewEntry,
    const cppcanvas::CanvasSharedPtr& rDestinationCanvas )
{
    // clear page to given fade color. 'Leaving' slide is
    // painted atop of that
    fillPage( rDestinationCanvas,
              ::basegfx::B2DSize( getEnteringSlideSizePixel( rViewEntry.mpView ) ),
              maFadeColor );
}

void CutSlideChange::performIn(
    const ::cppcanvas::CustomSpriteSharedPtr&   rSprite,
    const ViewEntry&                            /*rViewEntry*/,
    const ::cppcanvas::CanvasSharedPtr&         /*rDestinationCanvas*/,
    double                                      t )
{
    ENSURE_OR_THROW(
        rSprite,
        "CutSlideChange::performIn(): Invalid sprite" );

    // After 2/3rd of the active time, display new slide
    rSprite->setAlpha( t > 2/3.0 ? 1.0 : 0.0 );
}

void CutSlideChange::performOut(
    const ::cppcanvas::CustomSpriteSharedPtr&  rSprite,
    const ViewEntry&                           /* rViewEntry */,
    const ::cppcanvas::CanvasSharedPtr&        rDestinationCanvas,
    double                                     t )
{
    ENSURE_OR_THROW(
        rSprite,
        "CutSlideChange::performOut(): Invalid sprite" );
    ENSURE_OR_THROW(
        rDestinationCanvas,
        "CutSlideChange::performOut(): Invalid dest canvas" );

    // Until 1/3rd of the active time, display old slide.
    rSprite->setAlpha( t > 1/3.0 ? 0.0 : 1.0 );
}

class MovingSlideChange : public SlideChangeBase
{
    /// Direction vector for leaving slide,
    const ::basegfx::B2DVector  maLeavingDirection;

    /// Direction vector for entering slide,
    const ::basegfx::B2DVector  maEnteringDirection;

public:
    /** Create a new SlideChanger, for the given entering slide
        bitmaps, which performs a moving slide change effect

        @param rLeavingDirection
        Direction vector. The move is performed along this
        direction vector, starting at a position where the leaving
        slide is fully visible, and ending at a position where the
        leaving slide is just not visible. The vector must have
        unit length.

        @param rEnteringDirection
        Direction vector. The move is performed along this
        direction vector, starting at a position where the
        entering slide is just not visible, and ending at the
        final slide position. The vector must have unit length.
    */
    MovingSlideChange(
        const boost::optional<SlideSharedPtr>& leavingSlide,
        const SlideSharedPtr&                  pEnteringSlide,
        const SoundPlayerSharedPtr&            pSoundPlayer,
        const UnoViewContainer&                rViewContainer,
        ScreenUpdater&                         rScreenUpdater,
        EventMultiplexer&                      rEventMultiplexer,
        const ::basegfx::B2DVector&            rLeavingDirection,
        const ::basegfx::B2DVector&            rEnteringDirection )
        : SlideChangeBase(
            leavingSlide, pEnteringSlide, pSoundPlayer,
            rViewContainer, rScreenUpdater, rEventMultiplexer,
            // Optimization: when leaving bitmap is given,
            // but it does not move, don't create sprites for it,
            // we simply paint it once at startup:
            !rLeavingDirection.equalZero() /* bCreateLeavingSprites */,
            !rEnteringDirection.equalZero() /* bCreateEnteringSprites */ ),
          // TODO(F1): calc correct length of direction
          // vector. Directions not strictly horizontal or vertical
          // must travel a longer distance.
          maLeavingDirection( rLeavingDirection ),
          // TODO(F1): calc correct length of direction
          // vector. Directions not strictly horizontal or vertical
          // must travel a longer distance.
          maEnteringDirection( rEnteringDirection )
        {}

    virtual void prepareForRun(
        const ViewEntry& rViewEntry,
        const cppcanvas::CanvasSharedPtr& rDestinationCanvas ) override;

    virtual void performIn(
        const ::cppcanvas::CustomSpriteSharedPtr&   rSprite,
        const ViewEntry&                            rViewEntry,
        const ::cppcanvas::CanvasSharedPtr&         rDestinationCanvas,
        double                                      t ) override;

    virtual void performOut(
        const ::cppcanvas::CustomSpriteSharedPtr&  rSprite,
        const ViewEntry&                           rViewEntry,
        const ::cppcanvas::CanvasSharedPtr&        rDestinationCanvas,
        double                                     t ) override;
};

void MovingSlideChange::prepareForRun(
    const ViewEntry& rViewEntry,
    const cppcanvas::CanvasSharedPtr& rDestinationCanvas )
{
    if ( maLeavingDirection.equalZero() )
        renderBitmap( getLeavingBitmap( rViewEntry ), rDestinationCanvas );
    else if ( maEnteringDirection.equalZero() )
        renderBitmap( getEnteringBitmap( rViewEntry ), rDestinationCanvas );
}

void MovingSlideChange::performIn(
    const ::cppcanvas::CustomSpriteSharedPtr&   rSprite,
    const ViewEntry&                            rViewEntry,
    const ::cppcanvas::CanvasSharedPtr&         rDestinationCanvas,
    double                                      t )
{
    // intro sprite moves:

    ENSURE_OR_THROW(
        rSprite,
        "MovingSlideChange::performIn(): Invalid sprite" );
    ENSURE_OR_THROW(
        rDestinationCanvas,
        "MovingSlideChange::performIn(): Invalid dest canvas" );

    // TODO(F1): This does not account for non-translational
    // transformations! If the canvas is rotated, we still
    // move the sprite unrotated (which might or might not
    // produce the intended effect).
    const basegfx::B2DHomMatrix aViewTransform(
        rDestinationCanvas->getTransformation() );
    const basegfx::B2DPoint aPageOrigin(
        aViewTransform * basegfx::B2DPoint() );

    // move sprite
    rSprite->movePixel(
        aPageOrigin +
        ((t - 1.0) *
         ::basegfx::B2DSize( getEnteringSlideSizePixel(rViewEntry.mpView) ) *
         maEnteringDirection) );
}

void MovingSlideChange::performOut(
    const ::cppcanvas::CustomSpriteSharedPtr&  rSprite,
    const ViewEntry&                           rViewEntry,
    const ::cppcanvas::CanvasSharedPtr&        rDestinationCanvas,
    double                                     t )
{
    // outro sprite moves:

    ENSURE_OR_THROW(
        rSprite,
        "MovingSlideChange::performOut(): Invalid sprite" );
    ENSURE_OR_THROW(
        rDestinationCanvas,
        "MovingSlideChange::performOut(): Invalid dest canvas" );

    // TODO(F1): This does not account for non-translational
    // transformations! If the canvas is rotated, we still
    // move the sprite unrotated (which might or might not
    // produce the intended effect).
    const basegfx::B2DHomMatrix aViewTransform(
        rDestinationCanvas->getTransformation() );
    const basegfx::B2DPoint aPageOrigin(
        aViewTransform * basegfx::B2DPoint() );

    // move sprite
    rSprite->movePixel(
        aPageOrigin + (t *
                       ::basegfx::B2DSize( getEnteringSlideSizePixel(rViewEntry.mpView) ) *
                       maLeavingDirection) );
}


NumberAnimationSharedPtr createPushWipeTransition(
    boost::optional<SlideSharedPtr> const &         leavingSlide_,
    const SlideSharedPtr&                           pEnteringSlide,
    const UnoViewContainer&                         rViewContainer,
    ScreenUpdater&                                  rScreenUpdater,
    EventMultiplexer&                               rEventMultiplexer,
    sal_Int16                                       /*nTransitionType*/,
    sal_Int16                                       nTransitionSubType,
    bool                                            /*bTransitionDirection*/,
    const SoundPlayerSharedPtr&                     pSoundPlayer )
{
    boost::optional<SlideSharedPtr> leavingSlide; // no bitmap
    if (leavingSlide_ && *leavingSlide_ != nullptr)
    {
        // opt: only page, if we've an
        // actual slide to move out here. We
        // _don't_ need a fake black background
        // bitmap, neither for push nor for comb
        // wipes.
        leavingSlide = leavingSlide_;
    }

    // setup direction vector
    bool bComb( false );
    ::basegfx::B2DVector aDirection;
    switch( nTransitionSubType )
    {
    default:
        OSL_FAIL(
            "createPushWipeTransition(): Unexpected transition "
            "subtype for animations::TransitionType::PUSHWIPE "
            "transitions" );
        return NumberAnimationSharedPtr();

    case animations::TransitionSubType::FROMTOP:
        aDirection = ::basegfx::B2DVector( 0.0, 1.0 );
        break;

    case animations::TransitionSubType::FROMBOTTOM:
        aDirection = ::basegfx::B2DVector( 0.0, -1.0 );
        break;

    case animations::TransitionSubType::FROMLEFT:
        aDirection = ::basegfx::B2DVector( 1.0, 0.0 );
        break;

    case animations::TransitionSubType::FROMRIGHT:
        aDirection = ::basegfx::B2DVector( -1.0, 0.0 );
        break;

    case animations::TransitionSubType::FROMBOTTOMRIGHT:
        aDirection = ::basegfx::B2DVector( -1.0, -1.0 );
        break;

    case animations::TransitionSubType::FROMBOTTOMLEFT:
        aDirection = ::basegfx::B2DVector( 1.0, -1.0 );
        break;

    case animations::TransitionSubType::FROMTOPRIGHT:
        aDirection = ::basegfx::B2DVector( -1.0, 1.0 );
        break;

    case animations::TransitionSubType::FROMTOPLEFT:
        aDirection = ::basegfx::B2DVector( 1.0, 1.0 );
        break;

    case animations::TransitionSubType::COMBHORIZONTAL:
        aDirection = ::basegfx::B2DVector( 1.0, 0.0 );
        bComb = true;
        break;

    case animations::TransitionSubType::COMBVERTICAL:
        aDirection = ::basegfx::B2DVector( 0.0, 1.0 );
        bComb = true;
        break;
    }

    if( bComb )
    {
        return NumberAnimationSharedPtr(
            new CombTransition( leavingSlide,
                                pEnteringSlide,
                                pSoundPlayer,
                                rViewContainer,
                                rScreenUpdater,
                                rEventMultiplexer,
                                aDirection,
                                24 /* comb with 12 stripes */ ));
    }
    else
    {
        return NumberAnimationSharedPtr(
            new MovingSlideChange( leavingSlide,
                                   pEnteringSlide,
                                   pSoundPlayer,
                                   rViewContainer,
                                   rScreenUpdater,
                                   rEventMultiplexer,
                                   aDirection,
                                   aDirection ));
    }
}

NumberAnimationSharedPtr createSlideWipeTransition(
    boost::optional<SlideSharedPtr> const &         leavingSlide,
    const SlideSharedPtr&                           pEnteringSlide,
    const UnoViewContainer&                         rViewContainer,
    ScreenUpdater&                                  rScreenUpdater,
    EventMultiplexer&                               rEventMultiplexer,
    sal_Int16                                       /*nTransitionType*/,
    sal_Int16                                       nTransitionSubType,
    bool                                            bTransitionDirection,
    const SoundPlayerSharedPtr&                     pSoundPlayer )
{
    // setup 'in' direction vector
    ::basegfx::B2DVector aInDirection;
    switch( nTransitionSubType )
    {
    default:
        OSL_FAIL(
            "createSlideWipeTransition(): Unexpected transition "
            "subtype for animations::TransitionType::SLIDEWIPE "
            "transitions" );
        return NumberAnimationSharedPtr();

    case animations::TransitionSubType::FROMTOP:
        aInDirection = ::basegfx::B2DVector( 0.0, 1.0 );
        break;

    case animations::TransitionSubType::FROMRIGHT:
        aInDirection = ::basegfx::B2DVector( -1.0, 0.0 );
        break;

    case animations::TransitionSubType::FROMLEFT:
        aInDirection = ::basegfx::B2DVector( 1.0, 0.0 );
        break;

    case animations::TransitionSubType::FROMBOTTOM:
        aInDirection = ::basegfx::B2DVector( 0.0, -1.0 );
        break;

    case animations::TransitionSubType::FROMBOTTOMRIGHT:
        aInDirection = ::basegfx::B2DVector( -1.0, -1.0 );
        break;

    case animations::TransitionSubType::FROMBOTTOMLEFT:
        aInDirection = ::basegfx::B2DVector( 1.0, -1.0 );
        break;

    case animations::TransitionSubType::FROMTOPRIGHT:
        aInDirection = ::basegfx::B2DVector( -1.0, 1.0 );
        break;

    case animations::TransitionSubType::FROMTOPLEFT:
        aInDirection = ::basegfx::B2DVector( 1.0, 1.0 );
        break;
    }

    if( bTransitionDirection )
    {
        // normal, 'forward' slide wipe effect. Since the old
        // content is still on screen (and does not move), we omit
        // the 'leaving' slide.


        return NumberAnimationSharedPtr(
            new MovingSlideChange(
                boost::optional<SlideSharedPtr>() /* no slide */,
                pEnteringSlide,
                pSoundPlayer,
                rViewContainer,
                rScreenUpdater,
                rEventMultiplexer,
                basegfx::B2DVector(),
                aInDirection ));
    }
    else
    {
        // 'reversed' slide wipe effect. Reverse for slide wipes
        // means, that the new slide is in the back, statically,
        // and the old one is moving off in the foreground.


        return NumberAnimationSharedPtr(
            new MovingSlideChange( leavingSlide,
                                   pEnteringSlide,
                                   pSoundPlayer,
                                   rViewContainer,
                                   rScreenUpdater,
                                   rEventMultiplexer,
                                   aInDirection,
                                   basegfx::B2DVector() ));
    }
}

NumberAnimationSharedPtr createPluginTransition(
    sal_Int16                                nTransitionType,
    sal_Int16                                nTransitionSubType,
    boost::optional<SlideSharedPtr> const&   pLeavingSlide,
    const SlideSharedPtr&                    pEnteringSlide,
    const UnoViewContainer&                  rViewContainer,
    ScreenUpdater&                           rScreenUpdater,
    const uno::Reference<
          presentation::XTransitionFactory>& xFactory,
    const SoundPlayerSharedPtr&              pSoundPlayer,
    EventMultiplexer&                        rEventMultiplexer)
{
    std::unique_ptr<PluginSlideChange> pTransition(
        new PluginSlideChange(
            nTransitionType,
            nTransitionSubType,
            pLeavingSlide,
            pEnteringSlide,
            rViewContainer,
            rScreenUpdater,
            xFactory,
            pSoundPlayer,
            rEventMultiplexer ));

    if( pTransition->Success() )
        return NumberAnimationSharedPtr( pTransition.release() );
    else
    {
        return NumberAnimationSharedPtr();
    }
}

} // anon namespace


NumberAnimationSharedPtr TransitionFactory::createSlideTransition(
    const SlideSharedPtr&                                   pLeavingSlide,
    const SlideSharedPtr&                                   pEnteringSlide,
    const UnoViewContainer&                                 rViewContainer,
    ScreenUpdater&                                          rScreenUpdater,
    EventMultiplexer&                                       rEventMultiplexer,
    const uno::Reference<presentation::XTransitionFactory>& xOptionalFactory,
    sal_Int16                                               nTransitionType,
    sal_Int16                                               nTransitionSubType,
    bool                                                    bTransitionDirection,
    const RGBColor&                                         rTransitionFadeColor,
    const SoundPlayerSharedPtr&                             pSoundPlayer            )
{
    // xxx todo: change to TransitionType::NONE, TransitionSubType::NONE:
    if (nTransitionType == 0 && nTransitionSubType == 0) {
        // just play sound, no slide transition:
        if (pSoundPlayer) {
            pSoundPlayer->startPlayback();
            // xxx todo: for now, presentation.cxx takes care about the slide
            // #i50492#  transition sound object, so just release it here
        }
        return NumberAnimationSharedPtr();
    }

    ENSURE_OR_THROW(
        pEnteringSlide,
        "TransitionFactory::createSlideTransition(): Invalid entering slide" );

    if( xOptionalFactory.is() &&
        xOptionalFactory->hasTransition(nTransitionType, nTransitionSubType) )
    {
        // #i82460# - optional plugin factory claims this transition. delegate.
        NumberAnimationSharedPtr pTransition(
            createPluginTransition(
                nTransitionType,
                nTransitionSubType,
                boost::make_optional(pLeavingSlide),
                pEnteringSlide,
                rViewContainer,
                rScreenUpdater,
                xOptionalFactory,
                pSoundPlayer,
                rEventMultiplexer ));

        if( pTransition.get() )
            return pTransition;
    }

    const TransitionInfo* pTransitionInfo(
        getTransitionInfo( nTransitionType, nTransitionSubType ) );

    if( pTransitionInfo != nullptr )
    {
        switch( pTransitionInfo->meTransitionClass )
        {
            default:
            case TransitionInfo::TRANSITION_INVALID:
                SAL_WARN("slideshow",
                    "TransitionFactory::createSlideTransition(): "
                    "Invalid type/subtype combination encountered."
                    << nTransitionType << " " << nTransitionSubType );
                return NumberAnimationSharedPtr();


            case TransitionInfo::TRANSITION_CLIP_POLYPOLYGON:
            {
                // generate parametric poly-polygon
                ParametricPolyPolygonSharedPtr pPoly(
                    ParametricPolyPolygonFactory::createClipPolyPolygon(
                        nTransitionType, nTransitionSubType ) );

                // create a clip transition from that
                return NumberAnimationSharedPtr(
                    new ClippedSlideChange( pEnteringSlide,
                                            pPoly,
                                            *pTransitionInfo,
                                            rViewContainer,
                                            rScreenUpdater,
                                            rEventMultiplexer,
                                            bTransitionDirection,
                                            pSoundPlayer ));
            }

            case TransitionInfo::TRANSITION_SPECIAL:
            {
                switch( nTransitionType )
                {
                    default:
                        OSL_FAIL(
                            "TransitionFactory::createSlideTransition(): "
                            "Unexpected transition type for "
                            "TRANSITION_SPECIAL transitions" );
                        return NumberAnimationSharedPtr();

                    case animations::TransitionType::RANDOM:
                    {
                        // select randomly one of the effects from the
                        // TransitionFactoryTable

                        const TransitionInfo* pRandomTransitionInfo(
                            getRandomTransitionInfo() );

                        ENSURE_OR_THROW(
                            pRandomTransitionInfo != nullptr,
                            "TransitionFactory::createSlideTransition(): "
                            "Got invalid random transition info" );

                        ENSURE_OR_THROW(
                            pRandomTransitionInfo->mnTransitionType !=
                            animations::TransitionType::RANDOM,
                            "TransitionFactory::createSlideTransition(): "
                            "Got random again for random input!" );

                        // and recurse
                        return createSlideTransition(
                            pLeavingSlide,
                            pEnteringSlide,
                            rViewContainer,
                            rScreenUpdater,
                            rEventMultiplexer,
                            xOptionalFactory,
                            pRandomTransitionInfo->mnTransitionType,
                            pRandomTransitionInfo->mnTransitionSubType,
                            bTransitionDirection,
                            rTransitionFadeColor,
                            pSoundPlayer );
                    }

                    case animations::TransitionType::PUSHWIPE:
                    {
                        return createPushWipeTransition(
                            boost::make_optional(pLeavingSlide),
                            pEnteringSlide,
                            rViewContainer,
                            rScreenUpdater,
                            rEventMultiplexer,
                            nTransitionType,
                            nTransitionSubType,
                            bTransitionDirection,
                            pSoundPlayer );
                    }

                    case animations::TransitionType::SLIDEWIPE:
                    {
                        return createSlideWipeTransition(
                            boost::make_optional(pLeavingSlide),
                            pEnteringSlide,
                            rViewContainer,
                            rScreenUpdater,
                            rEventMultiplexer,
                            nTransitionType,
                            nTransitionSubType,
                            bTransitionDirection,
                            pSoundPlayer );
                    }

                    case animations::TransitionType::BARWIPE:
                    case animations::TransitionType::FADE:
                    {
                        // black page:
                        boost::optional<SlideSharedPtr> leavingSlide;
                        boost::optional<RGBColor> aFadeColor;

                        switch( nTransitionSubType )
                        {
                            case animations::TransitionSubType::CROSSFADE:
                                // crossfade needs no further setup,
                                // just blend new slide over current
                                // slide.
                                break;

                                // TODO(F1): Implement toColor/fromColor fades
                            case animations::TransitionSubType::FADETOCOLOR:
                            case animations::TransitionSubType::FADEFROMCOLOR:
                            case animations::TransitionSubType::FADEOVERCOLOR:
                                if (pLeavingSlide) {
                                    // only generate, if fade
                                    // effect really needs it.
                                    leavingSlide = pLeavingSlide;
                                }
                                aFadeColor = rTransitionFadeColor;
                                break;

                            default:
                                ENSURE_OR_THROW( false,
                                                  "SlideTransitionFactory::createSlideTransition(): Unknown FADE subtype" );
                        }

                        if( nTransitionType == animations::TransitionType::FADE )
                            return NumberAnimationSharedPtr(
                                new FadingSlideChange(
                                    leavingSlide,
                                    pEnteringSlide,
                                    aFadeColor,
                                    pSoundPlayer,
                                    rViewContainer,
                                    rScreenUpdater,
                                    rEventMultiplexer ));
                        else
                            return NumberAnimationSharedPtr(
                                new CutSlideChange(
                                    leavingSlide,
                                    pEnteringSlide,
                                    rTransitionFadeColor,
                                    pSoundPlayer,
                                    rViewContainer,
                                    rScreenUpdater,
                                    rEventMultiplexer ));
                    }
                }
            }
            break;
        }
    }

    // No animation generated, maybe no table entry for given
    // transition?
    SAL_WARN("slideshow",
        "TransitionFactory::createSlideTransition(): "
        "Unknown type/subtype combination encountered "
        << nTransitionType << " " << nTransitionSubType );
    OSL_FAIL(
        "TransitionFactory::createSlideTransition(): "
        "Unknown type/subtype combination encountered" );

    return NumberAnimationSharedPtr();
}

} // namespace internal
} // namespace presentation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
