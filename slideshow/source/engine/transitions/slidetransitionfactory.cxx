/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slidetransitionfactory.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:58:38 $
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

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

#include <canvas/debug.hxx>
#include <cppcanvas/basegfxfactory.hxx>

#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>

#include <transitionfactory.hxx>
#include <transitiontools.hxx>
#include <parametricpolypolygonfactory.hxx>
#include <animationfactory.hxx>
#include <clippingfunctor.hxx>
#include <combtransition.hxx>

#include "comphelper/optional.hxx"


/***************************************************
 ***                                             ***
 ***          Slide Transition Effects           ***
 ***                                             ***
 ***************************************************/

using namespace com::sun::star;

namespace presentation {
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

    const ::basegfx::B2DPolygon aPoly(
        ::basegfx::tools::createPolygonFromRect(
            ::basegfx::B2DRectangle(
                aOutputPosPixel.getX(),
                aOutputPosPixel.getY(),
                aOutputPosPixel.getX() + rPageSizePixel.getX(),
                aOutputPosPixel.getY() + rPageSizePixel.getY() ) ) );

    ::cppcanvas::PolyPolygonSharedPtr pPolyPoly(
        ::cppcanvas::BaseGfxFactory::getInstance().createPolyPolygon(
            pDevicePixelCanvas, aPoly ) );

    if( pPolyPoly.get() )
    {
        pPolyPoly->setRGBAFillColor( rFillColor.getIntegerColor() );
        pPolyPoly->draw();
    }
}


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
        bool                                    bDirectionForward,
        const SoundPlayerSharedPtr&             pSoundPlayer ) :
        SlideChangeBase(
            // leaving bitmap is empty, we're leveraging the fact that the
            // old slide is still displayed in the background:
            boost::optional<SlideSharedPtr>(),
            pEnteringSlide,
            pSoundPlayer ),
        maClippingFunctor( rPolygon,
                           rTransitionInfo,
                           bDirectionForward,
                           true )
        {}

    virtual void performIn(
        const ::cppcanvas::CustomSpriteSharedPtr&   rSprite,
        UnoViewSharedPtr const &                    pView,
        const ::cppcanvas::CanvasSharedPtr&         rDestinationCanvas,
        double                                      t );

    virtual void performOut(
        const ::cppcanvas::CustomSpriteSharedPtr&  rSprite,
        UnoViewSharedPtr const &                   pView,
        const ::cppcanvas::CanvasSharedPtr&        rDestinationCanvas,
        double                                     t );

private:
    ClippingFunctor             maClippingFunctor;
};

void ClippedSlideChange::performIn(
    const ::cppcanvas::CustomSpriteSharedPtr&   rSprite,
    UnoViewSharedPtr const &                    pView,
    const ::cppcanvas::CanvasSharedPtr&         rDestinationCanvas,
    double                                      t )
{
    // #i46602# Better work in device coordinate space here,
    // otherwise, we too easily suffer from roundoffs. Apart from
    // that, getEnteringSizePixel() _guarantees_ to cover the whole
    // slide bitmap. There's a catch, though: this removes any effect
    // of the view transformation (e.g. rotation) from the transition.
    rSprite->setClipPixel(
        maClippingFunctor( t,
                           getEnteringSizePixel(pView) ) );
}

void ClippedSlideChange::performOut(
    const ::cppcanvas::CustomSpriteSharedPtr&  rSprite,
    UnoViewSharedPtr const &                   pView,
    const ::cppcanvas::CanvasSharedPtr&        rDestinationCanvas,
    double                                     t )
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
        boost::optional<SlideSharedPtr> const &          leavingSlide,
        const SlideSharedPtr&                            pEnteringSlide,
        const ::comphelper::OptionalValue< RGBColor >&   rFadeColor,
        const SoundPlayerSharedPtr&                      pSoundPlayer )
        : SlideChangeBase( leavingSlide,
                           pEnteringSlide,
                           pSoundPlayer ),
          maFadeColor( rFadeColor ),
          mbFirstTurn( true )
        {}

    virtual void performIn(
        const ::cppcanvas::CustomSpriteSharedPtr&   rSprite,
        UnoViewSharedPtr const &                    pView,
        const ::cppcanvas::CanvasSharedPtr&         rDestinationCanvas,
        double                                      t );

    virtual void performOut(
        const ::cppcanvas::CustomSpriteSharedPtr&  rSprite,
        UnoViewSharedPtr const &                   pView,
        const ::cppcanvas::CanvasSharedPtr&        rDestinationCanvas,
        double                                     t );

private:
    const ::comphelper::OptionalValue< RGBColor >   maFadeColor;
    bool                                            mbFirstTurn;
};

void FadingSlideChange::performIn(
    const ::cppcanvas::CustomSpriteSharedPtr&   rSprite,
    UnoViewSharedPtr const &                    pView,
    const ::cppcanvas::CanvasSharedPtr&         rDestinationCanvas,
    double                                      t )
{
    ENSURE_AND_THROW(
        rSprite.get(),
        "FadingSlideChange::performIn(): Invalid sprite" );

    if( maFadeColor.isValid() )
        // After half of the active time, fade in new slide
        rSprite->setAlpha( t > 0.5 ? 2.0*(t-0.5) : 0.0 );
    else
        // Fade in new slide over full active time
        rSprite->setAlpha( t );
}

void FadingSlideChange::performOut(
    const ::cppcanvas::CustomSpriteSharedPtr&  rSprite,
    UnoViewSharedPtr const &                   pView,
    const ::cppcanvas::CanvasSharedPtr&        rDestinationCanvas,
    double                                     t )
{
    ENSURE_AND_THROW(
        rSprite.get(),
        "FadingSlideChange::performOut(): Invalid sprite" );
    ENSURE_AND_THROW(
        rDestinationCanvas.get(),
        "FadingSlideChange::performOut(): Invalid dest canvas" );

    // only needed for color fades
    if( maFadeColor.isValid() )
    {
        if( mbFirstTurn )
        {
            mbFirstTurn = false;

            // clear page to given fade color. 'Leaving' slide is
            // painted atop of that, but slowly fading out.
            fillPage( rDestinationCanvas,
                      getEnteringSizePixel( pView ),
                      maFadeColor.getValue() );
        }

        // Until half of the active time, fade out old
        // slide. After half of the active time, old slide
        // will be invisible.
        rSprite->setAlpha( t > 0.5 ? 0.0 : 2.0*(0.5-t) );
    }
}

class MovingSlideChange : public SlideChangeBase
{
    /// Direction vector for leaving slide,
    const ::basegfx::B2DVector  maLeavingDirection;

    /// Direction vector for entering slide,
    const ::basegfx::B2DVector  maEnteringDirection;

    bool                        mbFirstPerformCall;

public:
    /** Create a new SlideChanger, for the given entering slide
        bitmaps, which performes a moving slide change effect

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
        boost::optional<SlideSharedPtr> const & leavingSlide,
        const SlideSharedPtr& pEnteringSlide,
        const SoundPlayerSharedPtr& pSoundPlayer,
        const ::basegfx::B2DVector& rLeavingDirection,
        const ::basegfx::B2DVector& rEnteringDirection )
        : SlideChangeBase(
            leavingSlide, pEnteringSlide, pSoundPlayer,
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
          maEnteringDirection( rEnteringDirection ),
          mbFirstPerformCall( true )
        {}

    virtual void performIn(
        const ::cppcanvas::CustomSpriteSharedPtr&   rSprite,
        UnoViewSharedPtr const &                    pView,
        const ::cppcanvas::CanvasSharedPtr&         rDestinationCanvas,
        double                                      t );

    virtual void performOut(
        const ::cppcanvas::CustomSpriteSharedPtr&  rSprite,
        UnoViewSharedPtr const &                   pView,
        const ::cppcanvas::CanvasSharedPtr&        rDestinationCanvas,
        double                                     t );
};

void MovingSlideChange::performIn(
    const ::cppcanvas::CustomSpriteSharedPtr&   rSprite,
    UnoViewSharedPtr const &                    pView,
    const ::cppcanvas::CanvasSharedPtr&         rDestinationCanvas,
    double                                      t )
{
    // intro sprite moves:

    ENSURE_AND_THROW(
        rSprite.get(),
        "MovingSlideChange::performIn(): Invalid sprite" );
    ENSURE_AND_THROW(
        rDestinationCanvas.get(),
        "MovingSlideChange::performIn(): Invalid dest canvas" );

    if (mbFirstPerformCall && maLeavingDirection.equalZero()) {
        mbFirstPerformCall = false;
        renderBitmap( getLeavingBitmap(), rDestinationCanvas );
    }

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
         ::basegfx::B2DSize( getEnteringSizePixel(pView) ) *
         maEnteringDirection) );
}

void MovingSlideChange::performOut(
    const ::cppcanvas::CustomSpriteSharedPtr&  rSprite,
    UnoViewSharedPtr const &                   pView,
    const ::cppcanvas::CanvasSharedPtr&        rDestinationCanvas,
    double                                     t )
{
    // outro sprite moves:

    ENSURE_AND_THROW(
        rSprite.get(),
        "MovingSlideChange::performOut(): Invalid sprite" );
    ENSURE_AND_THROW(
        rDestinationCanvas.get(),
        "MovingSlideChange::performOut(): Invalid dest canvas" );

    if (mbFirstPerformCall && maEnteringDirection.equalZero()) {
        mbFirstPerformCall = false;
        renderBitmap( getEnteringBitmap(), rDestinationCanvas );
    }

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
                       ::basegfx::B2DSize( getEnteringSizePixel(pView) ) *
                       maLeavingDirection) );
}


SlideChangeAnimationSharedPtr createPushWipeTransition(
    boost::optional<SlideSharedPtr> const &         leavingSlide_,
    const SlideSharedPtr&                           pEnteringSlide,
    sal_Int16                                       nTransitionType,
    sal_Int16                                       nTransitionSubType,
    bool                                            bTransitionDirection,
    const SoundPlayerSharedPtr&                     pSoundPlayer )
{
    boost::optional<SlideSharedPtr> leavingSlide; // no bitmap
    if (leavingSlide_ && (*leavingSlide_).get() != 0)
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
        OSL_ENSURE(
            false,
            "createPushWipeTransition(): Unexpected transition "
            "subtype for animations::TransitionType::PUSHWIPE "
            "transitions" );
        return SlideChangeAnimationSharedPtr();

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
        return SlideChangeAnimationSharedPtr(
            new CombTransition( leavingSlide,
                                pEnteringSlide,
                                pSoundPlayer,
                                aDirection,
                                24 /* comb with 12 stripes */ ) );
    }
    else
    {
        return SlideChangeAnimationSharedPtr(
            new MovingSlideChange( leavingSlide,
                                   pEnteringSlide,
                                   pSoundPlayer,
                                   aDirection,
                                   aDirection ) );
    }
}

SlideChangeAnimationSharedPtr createSlideWipeTransition(
    boost::optional<SlideSharedPtr> const &         leavingSlide,
    const SlideSharedPtr&                           pEnteringSlide,
    sal_Int16                                       nTransitionType,
    sal_Int16                                       nTransitionSubType,
    bool                                            bTransitionDirection,
    const SoundPlayerSharedPtr&                     pSoundPlayer )
{
    // setup 'in' direction vector
    ::basegfx::B2DVector aInDirection;
    switch( nTransitionSubType )
    {
    default:
        OSL_ENSURE(
            false,
            "createSlideWipeTransition(): Unexpected transition "
            "subtype for animations::TransitionType::SLIDEWIPE "
            "transitions" );
        return SlideChangeAnimationSharedPtr();

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
        // =======================================================

        return SlideChangeAnimationSharedPtr(
            new MovingSlideChange(
                boost::optional<SlideSharedPtr>() /* no slide */,
                pEnteringSlide,
                pSoundPlayer,
                basegfx::B2DVector(),
                aInDirection ) );
    }
    else
    {
        // 'reversed' slide wipe effect. Reverse for slide wipes
        // means, that the new slide is in the back, statically,
        // and the old one is moving off in the foreground.
        // =======================================================

        return SlideChangeAnimationSharedPtr(
            new MovingSlideChange(
                leavingSlide,
                pEnteringSlide,
                pSoundPlayer,
                aInDirection,
                basegfx::B2DVector() ) );
    }
}

} // anon namespace


SlideChangeAnimationSharedPtr TransitionFactory::createSlideTransition(
    const SlideSharedPtr&       pLeavingSlide,
    const SlideSharedPtr&       pEnteringSlide,
    sal_Int16                   nTransitionType,
    sal_Int16                   nTransitionSubType,
    bool                        bTransitionDirection,
    const RGBColor&             rTransitionFadeColor,
    const SoundPlayerSharedPtr& pSoundPlayer            )
{
    ENSURE_AND_THROW(
        pEnteringSlide.get(),
        "TransitionFactory::createSlideTransition(): Invalid entering slide" );

    const TransitionInfo* pTransitionInfo(
        getTransitionInfo( nTransitionType, nTransitionSubType ) );

    if( pTransitionInfo != NULL )
    {
        switch( pTransitionInfo->meTransitionClass )
        {
            default:
            case TransitionInfo::TRANSITION_INVALID:
                OSL_TRACE(
                    "TransitionFactory::createSlideTransition(): "
                    "Invalid type/subtype (%d/%d) combination encountered.",
                    nTransitionType,
                    nTransitionSubType );
                return SlideChangeAnimationSharedPtr();


            case TransitionInfo::TRANSITION_CLIP_POLYPOLYGON:
            {
                // generate parametric poly-polygon
                ParametricPolyPolygonSharedPtr pPoly(
                    ParametricPolyPolygonFactory::createClipPolyPolygon(
                        nTransitionType, nTransitionSubType ) );

                // create a clip transition from that
                return SlideChangeAnimationSharedPtr(
                    new ClippedSlideChange( pEnteringSlide,
                                            pPoly,
                                            *pTransitionInfo,
                                            bTransitionDirection,
                                            pSoundPlayer ) );
            }
            break;

            case TransitionInfo::TRANSITION_SPECIAL:
            {
                switch( nTransitionType )
                {
                    default:
                        OSL_ENSURE(
                            false,
                            "TransitionFactory::createSlideTransition(): "
                            "Unexpected transition type for "
                            "TRANSITION_SPECIAL transitions" );
                        return SlideChangeAnimationSharedPtr();

                    case animations::TransitionType::RANDOM:
                    {
                        // select randomly one of the effects from the
                        // TransitionFactoryTable

                        const TransitionInfo* pRandomTransitionInfo(
                            getRandomTransitionInfo() );

                        ENSURE_AND_THROW(
                            pRandomTransitionInfo != NULL,
                            "TransitionFactory::createSlideTransition(): "
                            "Got invalid random transition info" );

                        ENSURE_AND_THROW(
                            pRandomTransitionInfo->mnTransitionType !=
                            animations::TransitionType::RANDOM,
                            "TransitionFactory::createSlideTransition(): "
                            "Got random again for random input!" );

                        // and recurse
                        return createSlideTransition(
                            pLeavingSlide,
                            pEnteringSlide,
                            pRandomTransitionInfo->mnTransitionType,
                            pRandomTransitionInfo->mnTransitionSubType,
                            bTransitionDirection,
                            rTransitionFadeColor,
                            pSoundPlayer );
                    }

                    case animations::TransitionType::PUSHWIPE:
                    {
                        return createPushWipeTransition(
                            comphelper::make_optional(pLeavingSlide),
                            pEnteringSlide,
                            nTransitionType,
                            nTransitionSubType,
                            bTransitionDirection,
                            pSoundPlayer );
                    }

                    case animations::TransitionType::SLIDEWIPE:
                    {
                        return createSlideWipeTransition(
                            comphelper::make_optional(pLeavingSlide),
                            pEnteringSlide,
                            nTransitionType,
                            nTransitionSubType,
                            bTransitionDirection,
                            pSoundPlayer );
                    }

                    case animations::TransitionType::FADE:
                    {
                        // black page:
                        boost::optional<SlideSharedPtr> leavingSlide;

                        switch( nTransitionSubType )
                        {
                            case animations::TransitionSubType::CROSSFADE:
                                // crossfade needs no further setup,
                                // just blend new slide over existing
                                // background.
                                break;

                                // TODO(F1): Implement toColor/fromColor fades
                            case animations::TransitionSubType::FADETOCOLOR:
                                // FALLTHROUGH intended
                            case animations::TransitionSubType::FADEFROMCOLOR:
                                // FALLTHROUGH intended
                            case animations::TransitionSubType::FADEOVERCOLOR:
                                if (pLeavingSlide.get() != 0)
                                {
                                    // only generate, if fade
                                    // effect really needs it.
                                    leavingSlide.reset( pLeavingSlide );
                                }
                                break;

                            default:
                                ENSURE_AND_THROW( false,
                                                  "SlideTransitionFactory::createSlideTransition(): Unknown FADE subtype" );
                        }

                        return SlideChangeAnimationSharedPtr(
                            new FadingSlideChange( leavingSlide,
                                                   pEnteringSlide,
                                                   rTransitionFadeColor,
                                                   pSoundPlayer ) );
                    }
                }
            }
            break;
        }
    }

    // No animation generated, maybe no table entry for given
    // transition?
    OSL_TRACE(
        "TransitionFactory::createSlideTransition(): "
        "Unknown type/subtype (%d/%d) combination encountered",
        nTransitionType,
        nTransitionSubType );
    OSL_ENSURE(
        false,
        "TransitionFactory::createSlideTransition(): "
        "Unknown type/subtype combination encountered" );

    return SlideChangeAnimationSharedPtr();
}

} // namespace internal
} // namespace presentation
