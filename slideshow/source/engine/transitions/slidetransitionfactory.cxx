/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

#include <cppcanvas/basegfxfactory.hxx>

#include <comphelper/optional.hxx>
#include <comphelper/make_shared_from_uno.hxx>

#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/rendering/IntegerBitmapLayout.hpp>
#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "slidechangebase.hxx"
#include "transitionfactory.hxx"
#include "transitiontools.hxx"
#include "parametricpolypolygonfactory.hxx"
#include "animationfactory.hxx"
#include "clippingfunctor.hxx"
#include "combtransition.hxx"
#include "tools.hxx"

#include <boost/bind.hpp>


/***************************************************
 ***                                             ***
 ***          Slide Transition Effects           ***
 ***                                             ***
 ***************************************************/

using namespace com::sun::star;

namespace slideshow {
namespace internal {

namespace {




void fillPage( const ::cppcanvas::CanvasSharedPtr& rDestinationCanvas,
               const ::basegfx::B2DSize&           rPageSizePixel,
               const RGBColor&                     rFillColor )
{
    
    
    const ::cppcanvas::CanvasSharedPtr pDevicePixelCanvas(
        rDestinationCanvas->clone() );
    pDevicePixelCanvas->setTransformation( ::basegfx::B2DHomMatrix() );

    
    
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

    TransitionViewPair( uno::Reference<presentation::XTransition> xTransition, const UnoViewSharedPtr pView )
    {
        mxTransition = xTransition;
        mpView = pView;
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
        
        UnoViewVector::const_iterator aCurrView (rViewContainer.begin());
        const UnoViewVector::const_iterator aEnd(rViewContainer.end());
        while( aCurrView != aEnd )
        {
            if(! addTransition( *aCurrView ) )
                return;

            ENSURE_OR_THROW(maTransitions.back() && maTransitions.back()->mxTransition.is(),
                            "Failed to create plugin transition");
            ++aCurrView;
        }
        mbSuccess = true;
    }

    ~PluginSlideChange()
    {
        mxFactory.clear();

        ::std::vector< TransitionViewPair* >::const_iterator aCurrView (maTransitions.begin());
        ::std::vector< TransitionViewPair* >::const_iterator aEnd(maTransitions.end());
        while( aCurrView != aEnd )
        {
            delete (*aCurrView);
            ++aCurrView;
        }
        maTransitions.clear();
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
            maTransitions.push_back( new TransitionViewPair( rTransition, rView ) );
        else
            return false;

        return true;
    }

    virtual bool operator()( double t )
    {
        std::for_each(maTransitions.begin(),
                      maTransitions.end(),
                      boost::bind( &TransitionViewPair::update,
                                   _1, t) );
        return true;
    }

    bool Success()
    {
        return mbSuccess;
    }

    
    virtual void viewAdded( const UnoViewSharedPtr& rView )
    {
        OSL_TRACE("PluginSlideChange viewAdded");
        SlideChangeBase::viewAdded( rView );

        ::std::vector< TransitionViewPair* >::const_iterator aCurrView (maTransitions.begin());
        ::std::vector< TransitionViewPair* >::const_iterator aEnd(maTransitions.end());
        bool bKnown = false;
        while( aCurrView != aEnd )
        {
            if( (*aCurrView)->mpView == rView )
            {
                bKnown = true;
                break;
            }
            ++aCurrView;
        }

        if( !bKnown )
        {
            OSL_TRACE("need to be added");
            addTransition( rView );
        }
    }

    virtual void viewRemoved( const UnoViewSharedPtr& rView )
    {
        OSL_TRACE("PluginSlideChange viewRemoved");
        SlideChangeBase::viewRemoved( rView );

        ::std::vector< TransitionViewPair* >::iterator aCurrView (maTransitions.begin());
        ::std::vector< TransitionViewPair* >::const_iterator aEnd(maTransitions.end());
        while( aCurrView != aEnd )
        {
            if( (*aCurrView)->mpView == rView )
            {
                OSL_TRACE( "view removed" );
                delete (*aCurrView);
                maTransitions.erase( aCurrView );
                break;
            }
            ++aCurrView;
        }
    }

    virtual void viewChanged( const UnoViewSharedPtr& rView )
    {
        OSL_TRACE("PluginSlideChange viewChanged");
        SlideChangeBase::viewChanged( rView );

        ::std::vector< TransitionViewPair* >::const_iterator aCurrView (maTransitions.begin());
        ::std::vector< TransitionViewPair* >::const_iterator aEnd(maTransitions.end());
        while( aCurrView != aEnd )
        {
            if( (*aCurrView)->mpView == rView )
            {
                OSL_TRACE( "view changed" );
                (*aCurrView)->mxTransition->viewChanged( rView->getUnoView(),
                                                         getLeavingBitmap(ViewEntry(rView))->getXBitmap(),
                                                         getEnteringBitmap(ViewEntry(rView))->getXBitmap() );
            }
            else
                OSL_TRACE( "view did not changed" );

            ++aCurrView;
        }
    }

    virtual void viewsChanged()
    {
        OSL_TRACE("PluginSlideChange viewsChanged");
        SlideChangeBase::viewsChanged();

        ::std::vector< TransitionViewPair* >::const_iterator aCurrView (maTransitions.begin());
        ::std::vector< TransitionViewPair* >::const_iterator aEnd(maTransitions.end());
        while( aCurrView != aEnd )
        {
            OSL_TRACE( "view changed" );
            (*aCurrView)->mxTransition->viewChanged( (*aCurrView)->mpView->getUnoView(),
                                                     getLeavingBitmap(ViewEntry((*aCurrView)->mpView))->getXBitmap(),
                                                     getEnteringBitmap(ViewEntry((*aCurrView)->mpView))->getXBitmap() );
            ++aCurrView;
        }
    }

private:
    
    std::vector< TransitionViewPair* > maTransitions;

    
    bool mbSuccess;

    sal_Int16 mnTransitionType;
    sal_Int16 mnTransitionSubType;

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
        double                                      t );

    virtual void performOut(
        const ::cppcanvas::CustomSpriteSharedPtr&  rSprite,
        const ViewEntry&                           rViewEntry,
        const ::cppcanvas::CanvasSharedPtr&        rDestinationCanvas,
        double                                     t );

private:
    ClippingFunctor             maClippingFunctor;
};

void ClippedSlideChange::performIn(
    const ::cppcanvas::CustomSpriteSharedPtr&   rSprite,
    const ViewEntry&                            rViewEntry,
    const ::cppcanvas::CanvasSharedPtr&         /*rDestinationCanvas*/,
    double                                      t )
{
    
    
    
    
    
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
        const cppcanvas::CanvasSharedPtr& rDestinationCanvas );

    virtual void performIn(
        const ::cppcanvas::CustomSpriteSharedPtr&   rSprite,
        const ViewEntry&                            rViewEntry,
        const ::cppcanvas::CanvasSharedPtr&         rDestinationCanvas,
        double                                      t );

    virtual void performOut(
        const ::cppcanvas::CustomSpriteSharedPtr&  rSprite,
        const ViewEntry&                           rViewEntry,
        const ::cppcanvas::CanvasSharedPtr&        rDestinationCanvas,
        double                                     t );

private:
    const boost::optional< RGBColor >               maFadeColor;
};

void FadingSlideChange::prepareForRun(
    const ViewEntry& rViewEntry,
    const cppcanvas::CanvasSharedPtr& rDestinationCanvas )
{
    if ( maFadeColor )
    {
        
        
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
        
        rSprite->setAlpha( t > 0.5 ? 2.0*(t-0.5) : 0.0 );
    else
        
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

    
    if( maFadeColor )
    {
        
        
        
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
        const cppcanvas::CanvasSharedPtr& rDestinationCanvas );

    virtual void performIn(
        const ::cppcanvas::CustomSpriteSharedPtr&   rSprite,
        const ViewEntry&                            rViewEntry,
        const ::cppcanvas::CanvasSharedPtr&         rDestinationCanvas,
        double                                      t );

    virtual void performOut(
        const ::cppcanvas::CustomSpriteSharedPtr&  rSprite,
        const ViewEntry&                           rViewEntry,
        const ::cppcanvas::CanvasSharedPtr&        rDestinationCanvas,
        double                                     t );

private:
    RGBColor maFadeColor;
};

void CutSlideChange::prepareForRun(
    const ViewEntry& rViewEntry,
    const cppcanvas::CanvasSharedPtr& rDestinationCanvas )
{
    
    
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

    
    rSprite->setAlpha( t > 1/3.0 ? 0.0 : 1.0 );
}

class MovingSlideChange : public SlideChangeBase
{
    
    const ::basegfx::B2DVector  maLeavingDirection;

    
    const ::basegfx::B2DVector  maEnteringDirection;

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
            
            
            
            !rLeavingDirection.equalZero() /* bCreateLeavingSprites */,
            !rEnteringDirection.equalZero() /* bCreateEnteringSprites */ ),
          
          
          
          maLeavingDirection( rLeavingDirection ),
          
          
          
          maEnteringDirection( rEnteringDirection )
        {}

    virtual void prepareForRun(
        const ViewEntry& rViewEntry,
        const cppcanvas::CanvasSharedPtr& rDestinationCanvas );

    virtual void performIn(
        const ::cppcanvas::CustomSpriteSharedPtr&   rSprite,
        const ViewEntry&                            rViewEntry,
        const ::cppcanvas::CanvasSharedPtr&         rDestinationCanvas,
        double                                      t );

    virtual void performOut(
        const ::cppcanvas::CustomSpriteSharedPtr&  rSprite,
        const ViewEntry&                           rViewEntry,
        const ::cppcanvas::CanvasSharedPtr&        rDestinationCanvas,
        double                                     t );
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
    

    ENSURE_OR_THROW(
        rSprite,
        "MovingSlideChange::performIn(): Invalid sprite" );
    ENSURE_OR_THROW(
        rDestinationCanvas,
        "MovingSlideChange::performIn(): Invalid dest canvas" );

    
    
    
    
    const basegfx::B2DHomMatrix aViewTransform(
        rDestinationCanvas->getTransformation() );
    const basegfx::B2DPoint aPageOrigin(
        aViewTransform * basegfx::B2DPoint() );

    
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
    

    ENSURE_OR_THROW(
        rSprite,
        "MovingSlideChange::performOut(): Invalid sprite" );
    ENSURE_OR_THROW(
        rDestinationCanvas,
        "MovingSlideChange::performOut(): Invalid dest canvas" );

    
    
    
    
    const basegfx::B2DHomMatrix aViewTransform(
        rDestinationCanvas->getTransformation() );
    const basegfx::B2DPoint aPageOrigin(
        aViewTransform * basegfx::B2DPoint() );

    
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
    boost::optional<SlideSharedPtr> leavingSlide; 
    if (leavingSlide_ && (*leavingSlide_).get() != 0)
    {
        
        
        
        
        
        leavingSlide = leavingSlide_;
    }

    
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
    PluginSlideChange* pTransition =
        new PluginSlideChange(
            nTransitionType,
            nTransitionSubType,
            pLeavingSlide,
            pEnteringSlide,
            rViewContainer,
            rScreenUpdater,
            xFactory,
            pSoundPlayer,
            rEventMultiplexer );

    if( pTransition->Success() )
        return NumberAnimationSharedPtr( pTransition );
    else
    {
        delete pTransition;
        return NumberAnimationSharedPtr();
    }
}

} 


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
    
    if (nTransitionType == 0 && nTransitionSubType == 0) {
        
        if (pSoundPlayer) {
            pSoundPlayer->startPlayback();
            
            
        }
        return NumberAnimationSharedPtr();
    }

    ENSURE_OR_THROW(
        pEnteringSlide,
        "TransitionFactory::createSlideTransition(): Invalid entering slide" );

    if( xOptionalFactory.is() &&
        xOptionalFactory->hasTransition(nTransitionType, nTransitionSubType) )
    {
        
        NumberAnimationSharedPtr pTransition(
            createPluginTransition(
                nTransitionType,
                nTransitionSubType,
                comphelper::make_optional(pLeavingSlide),
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
                return NumberAnimationSharedPtr();


            case TransitionInfo::TRANSITION_CLIP_POLYPOLYGON:
            {
                
                ParametricPolyPolygonSharedPtr pPoly(
                    ParametricPolyPolygonFactory::createClipPolyPolygon(
                        nTransitionType, nTransitionSubType ) );

                
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
                        
                        

                        const TransitionInfo* pRandomTransitionInfo(
                            getRandomTransitionInfo() );

                        ENSURE_OR_THROW(
                            pRandomTransitionInfo != NULL,
                            "TransitionFactory::createSlideTransition(): "
                            "Got invalid random transition info" );

                        ENSURE_OR_THROW(
                            pRandomTransitionInfo->mnTransitionType !=
                            animations::TransitionType::RANDOM,
                            "TransitionFactory::createSlideTransition(): "
                            "Got random again for random input!" );

                        
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
                            comphelper::make_optional(pLeavingSlide),
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
                            comphelper::make_optional(pLeavingSlide),
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
                        
                        boost::optional<SlideSharedPtr> leavingSlide;
                        boost::optional<RGBColor> aFadeColor;

                        switch( nTransitionSubType )
                        {
                            case animations::TransitionSubType::CROSSFADE:
                                
                                
                                
                                break;

                                
                            case animations::TransitionSubType::FADETOCOLOR:
                                
                            case animations::TransitionSubType::FADEFROMCOLOR:
                                
                            case animations::TransitionSubType::FADEOVERCOLOR:
                                if (pLeavingSlide) {
                                    
                                    
                                    leavingSlide.reset( pLeavingSlide );
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

    
    
    OSL_TRACE(
        "TransitionFactory::createSlideTransition(): "
        "Unknown type/subtype (%d/%d) combination encountered",
        nTransitionType,
        nTransitionSubType );
    OSL_FAIL(
        "TransitionFactory::createSlideTransition(): "
        "Unknown type/subtype combination encountered" );

    return NumberAnimationSharedPtr();
}

} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
