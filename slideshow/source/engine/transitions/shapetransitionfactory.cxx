/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_slideshow.hxx"

#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>

#include "transitionfactory.hxx"
#include "transitiontools.hxx"
#include "parametricpolypolygonfactory.hxx"
#include "animationfactory.hxx"
#include "clippingfunctor.hxx"

#include <boost/bind.hpp>


using namespace ::com::sun::star;

namespace slideshow {
namespace internal {

/***************************************************
 ***                                             ***
 ***          Shape Transition Effects           ***
 ***                                             ***
 ***************************************************/

namespace {

class ClippingAnimation : public NumberAnimation
{
public:
    ClippingAnimation(
        const ParametricPolyPolygonSharedPtr&   rPolygon,
        const ShapeManagerSharedPtr&            rShapeManager,
        const TransitionInfo&                   rTransitionInfo,
        bool                                    bDirectionForward,
        bool                                    bModeIn );

    ~ClippingAnimation();

    // Animation interface
    // -------------------
    virtual void prefetch( const AnimatableShapeSharedPtr&     rShape,
                           const ShapeAttributeLayerSharedPtr& rAttrLayer );
    virtual void start( const AnimatableShapeSharedPtr&     rShape,
                        const ShapeAttributeLayerSharedPtr& rAttrLayer );
    virtual void end();

    // NumberAnimation interface
    // -----------------------
    virtual bool operator()( double nValue );
    virtual double getUnderlyingValue() const;

private:
    void end_();

    AnimatableShapeSharedPtr           mpShape;
    ShapeAttributeLayerSharedPtr       mpAttrLayer;
    ShapeManagerSharedPtr              mpShapeManager;
    ClippingFunctor                    maClippingFunctor;
    bool                               mbSpriteActive;
};

ClippingAnimation::ClippingAnimation(
    const ParametricPolyPolygonSharedPtr&   rPolygon,
    const ShapeManagerSharedPtr&            rShapeManager,
    const TransitionInfo&                   rTransitionInfo,
    bool                                    bDirectionForward,
    bool                                    bModeIn ) :
        mpShape(),
        mpAttrLayer(),
        mpShapeManager( rShapeManager ),
        maClippingFunctor( rPolygon,
                           rTransitionInfo,
                           bDirectionForward,
                           bModeIn ),
        mbSpriteActive(false)
{
    ENSURE_OR_THROW(
        rShapeManager,
        "ClippingAnimation::ClippingAnimation(): Invalid ShapeManager" );
}

ClippingAnimation::~ClippingAnimation()
{
    try
    {
        end_();
    }
    catch (uno::Exception &)
    {
        OSL_FAIL( rtl::OUStringToOString(
                        comphelper::anyToString(
                            cppu::getCaughtException() ),
                        RTL_TEXTENCODING_UTF8 ).getStr() );
    }
}

void ClippingAnimation::prefetch( const AnimatableShapeSharedPtr&,
                                  const ShapeAttributeLayerSharedPtr& )
{
}

void ClippingAnimation::start( const AnimatableShapeSharedPtr&      rShape,
                               const ShapeAttributeLayerSharedPtr&  rAttrLayer )
{
    OSL_ENSURE( !mpShape,
                "ClippingAnimation::start(): Shape already set" );
    OSL_ENSURE( !mpAttrLayer,
                "ClippingAnimation::start(): Attribute layer already set" );

    mpShape = rShape;
    mpAttrLayer = rAttrLayer;

    ENSURE_OR_THROW( rShape,
                      "ClippingAnimation::start(): Invalid shape" );
    ENSURE_OR_THROW( rAttrLayer,
                      "ClippingAnimation::start(): Invalid attribute layer" );

    mpShape = rShape;
    mpAttrLayer = rAttrLayer;

    if( !mbSpriteActive )
    {
        mpShapeManager->enterAnimationMode( mpShape );
        mbSpriteActive = true;
    }
}

void ClippingAnimation::end()
{
    end_();
}

void ClippingAnimation::end_()
{
    if( mbSpriteActive )
    {
        mbSpriteActive = false;
        mpShapeManager->leaveAnimationMode( mpShape );

        if( mpShape->isContentChanged() )
            mpShapeManager->notifyShapeUpdate( mpShape );
    }
}

bool ClippingAnimation::operator()( double nValue )
{
    ENSURE_OR_RETURN_FALSE(
        mpAttrLayer && mpShape,
        "ClippingAnimation::operator(): Invalid ShapeAttributeLayer" );

    // set new clip
    mpAttrLayer->setClip( maClippingFunctor( nValue,
                                             mpShape->getDomBounds().getRange() ) );

    if( mpShape->isContentChanged() )
        mpShapeManager->notifyShapeUpdate( mpShape );

    return true;
}

double ClippingAnimation::getUnderlyingValue() const
{
    ENSURE_OR_THROW(
        mpAttrLayer,
        "ClippingAnimation::getUnderlyingValue(): Invalid ShapeAttributeLayer" );

    return 0.0;     // though this should be used in concert with
                    // ActivitiesFactory::createSimpleActivity, better
                    // explicitely name our start value.
                    // Permissible range for operator() above is [0,1]
}

} // anon namespace


AnimationActivitySharedPtr TransitionFactory::createShapeTransition(
    const ActivitiesFactory::CommonParameters&          rParms,
    const AnimatableShapeSharedPtr&                     rShape,
    const ShapeManagerSharedPtr&                        rShapeManager,
    const ::basegfx::B2DVector&                         rSlideSize,
    uno::Reference< animations::XTransitionFilter > const& xTransition )
{
    return createShapeTransition( rParms,
                                  rShape,
                                  rShapeManager,
                                  rSlideSize,
                                  xTransition,
                                  xTransition->getTransition(),
                                  xTransition->getSubtype() );
}

AnimationActivitySharedPtr TransitionFactory::createShapeTransition(
    const ActivitiesFactory::CommonParameters&              rParms,
    const AnimatableShapeSharedPtr&                         rShape,
    const ShapeManagerSharedPtr&                            rShapeManager,
    const ::basegfx::B2DVector&                             rSlideSize,
    ::com::sun::star::uno::Reference<
        ::com::sun::star::animations::XTransitionFilter > const& xTransition,
    sal_Int16                                               nType,
    sal_Int16                                               nSubType )
{
    ENSURE_OR_THROW(
        xTransition.is(),
        "TransitionFactory::createShapeTransition(): Invalid XTransition" );

    const TransitionInfo* pTransitionInfo(
        getTransitionInfo( nType, nSubType ) );

    AnimationActivitySharedPtr pGeneratedActivity;
    if( pTransitionInfo != NULL )
    {
        switch( pTransitionInfo->meTransitionClass )
        {
            default:
            case TransitionInfo::TRANSITION_INVALID:
                OSL_FAIL( "TransitionFactory::createShapeTransition(): Invalid transition type. "
                            "Don't ask me for a 0 TransitionType, have no XTransitionFilter node instead!" );
                return AnimationActivitySharedPtr();


            case TransitionInfo::TRANSITION_CLIP_POLYPOLYGON:
            {
                // generate parametric poly-polygon
                ParametricPolyPolygonSharedPtr pPoly(
                    ParametricPolyPolygonFactory::createClipPolyPolygon(
                        nType, nSubType ) );

                // create a clip activity from that
                pGeneratedActivity = ActivitiesFactory::createSimpleActivity(
                    rParms,
                    NumberAnimationSharedPtr(
                        new ClippingAnimation(
                            pPoly,
                            rShapeManager,
                            *pTransitionInfo,
                            xTransition->getDirection(),
                            xTransition->getMode() ) ),
                    true );
            }
            break;

            case TransitionInfo::TRANSITION_SPECIAL:
            {
                switch( nType )
                {
                    case animations::TransitionType::RANDOM:
                    {
                        // select randomly one of the effects from the
                        // TransitionFactoryTable

                        const TransitionInfo* pRandomTransitionInfo( getRandomTransitionInfo() );

                        ENSURE_OR_THROW( pRandomTransitionInfo != NULL,
                                          "TransitionFactory::createShapeTransition(): Got invalid random transition info" );

                        ENSURE_OR_THROW( pRandomTransitionInfo->mnTransitionType != animations::TransitionType::RANDOM,
                                          "TransitionFactory::createShapeTransition(): Got random again for random input!" );

                        // and recurse
                        pGeneratedActivity = createShapeTransition( rParms,
                                                                    rShape,
                                                                    rShapeManager,
                                                                    rSlideSize,
                                                                    xTransition,
                                                                    pRandomTransitionInfo->mnTransitionType,
                                                                    pRandomTransitionInfo->mnTransitionSubType );
                    }
                    break;

                    // TODO(F3): Implement slidewipe for shape
                    case animations::TransitionType::SLIDEWIPE:
                    {
                        sal_Int16 nBarWipeSubType(0);
                        bool      bDirectionForward(true);

                        // map slidewipe to BARWIPE, for now
                        switch( nSubType )
                        {
                            case animations::TransitionSubType::FROMLEFT:
                                nBarWipeSubType = animations::TransitionSubType::LEFTTORIGHT;
                                bDirectionForward = true;
                                break;

                            case animations::TransitionSubType::FROMRIGHT:
                                nBarWipeSubType = animations::TransitionSubType::LEFTTORIGHT;
                                bDirectionForward = false;
                                break;

                            case animations::TransitionSubType::FROMTOP:
                                nBarWipeSubType = animations::TransitionSubType::TOPTOBOTTOM;
                                bDirectionForward = true;
                                break;

                            case animations::TransitionSubType::FROMBOTTOM:
                                nBarWipeSubType = animations::TransitionSubType::TOPTOBOTTOM;
                                bDirectionForward = false;
                                break;

                            default:
                                ENSURE_OR_THROW( false,
                                                  "TransitionFactory::createShapeTransition(): Unexpected subtype for SLIDEWIPE" );
                                break;
                        }

                        // generate parametric poly-polygon
                        ParametricPolyPolygonSharedPtr pPoly(
                            ParametricPolyPolygonFactory::createClipPolyPolygon(
                                animations::TransitionType::BARWIPE,
                                nBarWipeSubType ) );

                        // create a clip activity from that
                        pGeneratedActivity = ActivitiesFactory::createSimpleActivity(
                            rParms,
                            NumberAnimationSharedPtr(
                                new ClippingAnimation(
                                    pPoly,
                                    rShapeManager,
                                    *getTransitionInfo( animations::TransitionType::BARWIPE,
                                                        nBarWipeSubType ),
                                    bDirectionForward,
                                    xTransition->getMode() ) ),
                            true );
                    }
                    break;

                    default:
                    {
                        // TODO(F1): Check whether there's anything left, anyway,
                        // for _shape_ transitions. AFAIK, there are no special
                        // effects for shapes...

                        // for now, map all to fade effect
                        pGeneratedActivity = ActivitiesFactory::createSimpleActivity(
                            rParms,
                            AnimationFactory::createNumberPropertyAnimation(
                                ::rtl::OUString(
                                    RTL_CONSTASCII_USTRINGPARAM("Opacity") ),
                                rShape,
                                rShapeManager,
                                rSlideSize ),
                            xTransition->getMode() );
                    }
                    break;
                }
            }
            break;
        }
    }

    if( !pGeneratedActivity )
    {
        // No animation generated, maybe no table entry for given
        // transition?
        OSL_TRACE(
            "TransitionFactory::createShapeTransition(): Unknown type/subtype (%d/%d) "
            "combination encountered",
            xTransition->getTransition(),
            xTransition->getSubtype() );
        OSL_FAIL(
            "TransitionFactory::createShapeTransition(): Unknown type/subtype "
            "combination encountered" );
    }

    return pGeneratedActivity;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
