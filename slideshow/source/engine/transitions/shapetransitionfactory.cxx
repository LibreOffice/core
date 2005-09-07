/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shapetransitionfactory.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:57:24 $
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

#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <canvas/debug.hxx>
#include <transitionfactory.hxx>
#include <transitiontools.hxx>
#include <parametricpolypolygonfactory.hxx>
#include <animationfactory.hxx>
#include <clippingfunctor.hxx>
#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>

#ifndef BOOST_BIND_HPP_INCLUDED
#include <boost/bind.hpp>
#endif


using namespace ::com::sun::star;

namespace presentation {
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
        const LayerManagerSharedPtr&            rLayerManager,
        const TransitionInfo&                   rTransitionInfo,
        bool                                    bDirectionForward,
        bool                                    bModeIn );

    // Animation interface
    // -------------------
    virtual void start( const AnimatableShapeSharedPtr&     rShape,
                        const ShapeAttributeLayerSharedPtr& rAttrLayer );
    virtual void end();

    // NumberAnimation interface
    // -----------------------
    virtual bool operator()( double nValue );
    virtual double getUnderlyingValue() const;

private:
    AnimatableShapeSharedPtr           mpShape;
    ShapeAttributeLayerSharedPtr       mpAttrLayer;
    LayerManagerSharedPtr              mpLayerManager;
    ClippingFunctor                    maClippingFunctor;
    bool                               mbSpriteActive;
};

ClippingAnimation::ClippingAnimation(
    const ParametricPolyPolygonSharedPtr&   rPolygon,
    const LayerManagerSharedPtr&            rLayerManager,
    const TransitionInfo&                   rTransitionInfo,
    bool                                    bDirectionForward,
    bool                                    bModeIn ) :
        mpShape(),
        mpAttrLayer(),
        mpLayerManager( rLayerManager ),
        maClippingFunctor( rPolygon,
                           rTransitionInfo,
                           bDirectionForward,
                           bModeIn ),
        mbSpriteActive(false)
{
    ENSURE_AND_THROW(
        rLayerManager.get(),
        "ClippingAnimation::ClippingAnimation(): Invalid LayerManager" );
}

void ClippingAnimation::start( const AnimatableShapeSharedPtr&      rShape,
                               const ShapeAttributeLayerSharedPtr&  rAttrLayer )
{
    OSL_ENSURE( !mpShape.get(),
                "ClippingAnimation::start(): Shape already set" );
    OSL_ENSURE( !mpAttrLayer.get(),
                "ClippingAnimation::start(): Attribute layer already set" );

    mpShape = rShape;
    mpAttrLayer = rAttrLayer;

    ENSURE_AND_THROW( rShape.get(),
                      "ClippingAnimation::start(): Invalid shape" );
    ENSURE_AND_THROW( rAttrLayer.get(),
                      "ClippingAnimation::start(): Invalid attribute layer" );

    mpShape = rShape;
    mpAttrLayer = rAttrLayer;

    if( !mbSpriteActive )
    {
        mpLayerManager->enterAnimationMode( mpShape );
        mbSpriteActive = true;
    }
}

void ClippingAnimation::end()
{
    if( mbSpriteActive )
    {
        mbSpriteActive = false;
        mpLayerManager->leaveAnimationMode( mpShape );
    }
}

bool ClippingAnimation::operator()( double nValue )
{
    ENSURE_AND_RETURN(
        mpAttrLayer.get() && mpShape.get(),
        "ClippingAnimation::operator(): Invalid ShapeAttributeLayer" );

    // set new clip
    mpAttrLayer->setClip( maClippingFunctor( nValue,
                                             mpShape->getUpdateArea().getRange() ) );

    if( mpShape->isUpdateNecessary() )
        mpLayerManager->notifyShapeUpdate( mpShape );

    return true;
}

double ClippingAnimation::getUnderlyingValue() const
{
    ENSURE_AND_THROW(
        mpAttrLayer.get(),
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
    const LayerManagerSharedPtr&                        rLayerManager,
    uno::Reference< animations::XTransitionFilter >&    xTransition )
{
    return createShapeTransition( rParms,
                                  rShape,
                                  rLayerManager,
                                  xTransition,
                                  xTransition->getTransition(),
                                  xTransition->getSubtype() );
}

AnimationActivitySharedPtr TransitionFactory::createShapeTransition(
    const ActivitiesFactory::CommonParameters&              rParms,
    const AnimatableShapeSharedPtr&                         rShape,
    const LayerManagerSharedPtr&                            rLayerManager,
    ::com::sun::star::uno::Reference<
        ::com::sun::star::animations::XTransitionFilter >&  xTransition,
    sal_Int16                                               nType,
    sal_Int16                                               nSubType )
{
    ENSURE_AND_THROW(
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
                OSL_ENSURE( false,
                            "TransitionFactory::createShapeTransition(): Invalid transition type. "
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
                            rLayerManager,
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

                        ENSURE_AND_THROW( pRandomTransitionInfo != NULL,
                                          "TransitionFactory::createShapeTransition(): Got invalid random transition info" );

                        ENSURE_AND_THROW( pRandomTransitionInfo->mnTransitionType != animations::TransitionType::RANDOM,
                                          "TransitionFactory::createShapeTransition(): Got random again for random input!" );

                        // and recurse
                        pGeneratedActivity = createShapeTransition( rParms,
                                                                    rShape,
                                                                    rLayerManager,
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
                                ENSURE_AND_THROW( false,
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
                                    rLayerManager,
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
                                rLayerManager ),
                            xTransition->getMode() );
                    }
                    break;
                }
            }
            break;
        }
    }

    if( !pGeneratedActivity.get() )
    {
        // No animation generated, maybe no table entry for given
        // transition?
        OSL_TRACE(
            "TransitionFactory::createShapeTransition(): Unknown type/subtype (%d/%d) "
            "combination encountered",
            xTransition->getTransition(),
            xTransition->getSubtype() );
        OSL_ENSURE(
            false,
            "TransitionFactory::createShapeTransition(): Unknown type/subtype "
            "combination encountered" );
    }

    return pGeneratedActivity;
}

}
}
