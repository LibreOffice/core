/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animationbasenode.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:39:27 $
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

// must be first
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#ifndef _COM_SUN_STAR_PRESENTATION_PARAGRAPHTARGET_HPP_
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_TIMING_HPP_
#include <com/sun/star/animations/Timing.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_ANIMATIONADDITIVEMODE_HPP_
#include <com/sun/star/animations/AnimationAdditiveMode.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_SHAPEANIMATIONSUBTYPE_HPP_
#include <com/sun/star/presentation/ShapeAnimationSubType.hpp>
#endif

#include <nodetools.hxx>
#include <doctreenode.hxx>
#include <animationbasenode.hxx>
#include <animationfactory.hxx>

#include <vector>
#include <algorithm>


using namespace ::com::sun::star;

namespace presentation
{
    namespace internal
    {
        namespace
        {
            /** Event, which connects Activity end with AnimationNode::deactivate() methods
             */
            class ActivityEnded : public Event
            {
            public:
                explicit ActivityEnded( const BaseNodeSharedPtr& rNode ) :
                    mpNode( rNode ),
                    mbWasFired( false )
                {
                    ENSURE_AND_THROW( mpNode.get(),
                                      "ActivityEnded::ActivityEnded(): Invalid node" );
                }

                virtual bool fire()
                {
                    if( isCharged() )
                    {
                        mbWasFired = true;

                        if( mpNode.get() )
                            mpNode->deactivate();

                        mpNode.reset();
                    }

                    return true;
                }

                virtual bool isCharged() const
                {
                    return !mbWasFired;
                }

                virtual double getActivationTime( double nCurrentTime ) const
                {
                    return nCurrentTime;
                }

                virtual void dispose()
                {
                    mpNode.reset();
                }

            private:
                AnimationNodeSharedPtr  mpNode;
                bool                    mbWasFired;
            };
        }

        AnimationBaseNode::AnimationBaseNode( const uno::Reference< animations::XAnimationNode >&   xNode,
                                              const BaseContainerNodeSharedPtr&                     rParent,
                                              const NodeContext&                                    rContext ) :
            BaseNode( xNode, rParent, rContext ),
            mxAnimateNode( xNode, uno::UNO_QUERY_THROW ),
            maAttributeLayerHolder(),
            mpActivity(),// only _held_ by this class. Derived classes must actually init this
                         // (that's because it's factory-generated, and the actual parameters
                         // will vary. See e.g. PropertyAnimationNode and AnimationSetNode)
            mpShape(),
            mpShapeSubset(),
            mbIsIndependentSubset( rContext.mbIsIndependentSubset )
        {
            // extract native node targets
            // ===========================

            // plain shape target
            uno::Reference< drawing::XShape > xShape( mxAnimateNode->getTarget(),
                                                      uno::UNO_QUERY );

            // distinguish 5 cases:
            //
            //  - plain shape target
            //  (NodeContext.mpMasterShapeSubset full set)
            //
            //  - parent-generated subset (generate an
            //  independent subset)
            //
            //  - parent-generated subset from iteration
            //  (generate a dependent subset)
            //
            //  - XShape target at the XAnimatioNode (generate
            //  a plain shape target)
            //
            //  - ParagraphTarget target at the XAnimationNode
            //  (generate an independent shape subset)
            if( rContext.mpMasterShapeSubset.get() )
            {
                if( rContext.mpMasterShapeSubset->isFullSet() )
                {
                    // case 1: plain shape target from parent
                    mpShape = rContext.mpMasterShapeSubset->getSubsetShape();
                }
                else
                {
                    // cases 2 & 3: subset shape
                    mpShapeSubset = rContext.mpMasterShapeSubset;
                }
            }
            else
            {
                // no parent-provided shape, try to extract
                // from XAnimationNode - cases 4 and 5

                // try to extract Shape from parent node's target attribute
                uno::Reference< drawing::XShape > xShape( mxAnimateNode->getTarget(),
                                                          uno::UNO_QUERY );

                if( xShape.is() )
                {
                    mpShape = lookupAttributableShape( getContext().mpLayerManager,
                                                       xShape );
                }
                else
                {
                    // no shape provided. Maybe a ParagraphTarget?
                    ::com::sun::star::presentation::ParagraphTarget aTarget;

                    if( !(mxAnimateNode->getTarget() >>= aTarget) )
                        ENSURE_AND_THROW( false,
                                          "AnimationBaseNode::AnimationBaseNode(): could not extract any target information" );

                    xShape = aTarget.Shape;

                    ENSURE_AND_THROW( xShape.is(),
                                      "AnimationBaseNode::AnimationBaseNode(): invalid shape in ParagraphTarget" );

                    mpShape = lookupAttributableShape( getContext().mpLayerManager,
                                                       xShape );

                    // NOTE: For shapes with ParagraphTarget, we ignore
                    // the SubItem property. We implicitely assume that it
                    // is set to ONLY_TEXT.
                    OSL_ENSURE(
                        mxAnimateNode->getSubItem() ==
                        ::com::sun::star::presentation::ShapeAnimationSubType::ONLY_TEXT ||
                        mxAnimateNode->getSubItem() ==
                        ::com::sun::star::presentation::ShapeAnimationSubType::AS_WHOLE,
                        "AnimationBaseNode::AnimationBaseNode(): ParagraphTarget given, but subitem not AS_TEXT or AS_WHOLE? "
                        "Make up your mind, I'll ignore the subitem." );

                    // okay, found a ParagraphTarget with a valid XShape. Does the shape
                    // provide the given paragraph?
                    const DocTreeNode& rTreeNode(
                        mpShape->getTreeNodeSupplier().getTreeNode(
                            aTarget.Paragraph,
                            DocTreeNode::NODETYPE_LOGICAL_PARAGRAPH ) );

                    // CAUTION: the creation of the subset shape
                    // _must_ stay in the node constructor, since
                    // Slide::prefetchShow() initializes shape
                    // attributes right after animation import (or
                    // the Slide class must be changed).
                    mpShapeSubset.reset( new ShapeSubset( mpShape,
                                                          rTreeNode,
                                                          getContext().mpLayerManager ) );

                    // Override NodeContext, and flag this node as
                    // a special independent subset one. This is
                    // important when applying initial attributes:
                    // independent shape subsets must be setup
                    // when the slide starts, since they, as their
                    // name suggest, can have state independent to
                    // the master shape. The following example
                    // might illustrate that: a master shape has
                    // no effect, one of the text paragraphs
                    // within it has an appear effect. Now, the
                    // respective paragraph must be invisible when
                    // the slide is initially shown, and become
                    // visible only when the effect starts.
                    mbIsIndependentSubset = true;

                    // already enable subset right here, the
                    // setup of initial shape attributes of
                    // course needs the subset shape
                    // generated, to apply e.g. visibility
                    // changes.
                    mpShapeSubset->enableSubsetShape();
                }
            }
        }

        bool AnimationBaseNode::init()
        {
            if( !BaseNode::init() )
                return false;

            // if we've still got an old activity lying
            // around, dispose it, it will soon be overwritten
            // with a new one.
            if( mpActivity.get() )
                mpActivity->dispose();

            return true;
        }

        void AnimationBaseNode::dispose()
        {
            if( mpActivity.get() )
                mpActivity->dispose();

            maAttributeLayerHolder.reset();
            mxAnimateNode.clear();
            mpActivity.reset();
            mpShape.reset();
            mpShapeSubset.reset();

            BaseNode::dispose();
        }

        bool AnimationBaseNode::resolve()
        {
            if( !BaseNode::resolve() )
                return false;

            // enable shape subset for automatically generated
            // subsets. Independent subsets are already setup
            // during construction time. Doing it only here
            // saves us a lot of sprites and shapes lying
            // around. This is especially important for
            // character-wise iterations, since the shape
            // content (e.g.  thousands of characters) would
            // otherwise be painted character-by-character.
            if( isDependentSubsettedShape() )
                enableShapeSubset();

            return true;
        }

        bool AnimationBaseNode::activate()
        {
            if( getState() == ACTIVE )
                return true; // avoid duplicate event generation

            if( !BaseNode::activate() )
                return false;

            // create new attribute layer
            maAttributeLayerHolder.createAttributeLayer( getShape() );

            ENSURE_AND_THROW( maAttributeLayerHolder.get(),
                              "AnimationBaseNode::activate(): Could not generate shape attribute layer" );

            // TODO(Q2): This affects the way mpActivity
            // works, but is performed here because of
            // locality (we're fiddling with the additive mode
            // here, anyway, and it's the only place where we
            // do). OTOH, maybe the complete additive mode
            // setup should be moved to the activities.

            // for simple by-animations, the SMIL spec
            // requires us to emulate "0,by-value" value list
            // behaviour, with additive mode forced to "sum",
            // no matter what the input is
            // (http://www.w3.org/TR/smil20/animation.html#adef-by).
            if( mxAnimateNode->getBy().hasValue() &&
                !mxAnimateNode->getTo().hasValue() &&
                !mxAnimateNode->getFrom().hasValue() )
            {
                // force attribute mode to REPLACE (note the
                // subtle discrepancy to the paragraph above,
                // where SMIL requires SUM. This is internally
                // handled by the FromToByActivity, and is
                // because otherwise DOM values would not be
                // handled correctly: the activity cannot
                // determine whether an
                // Activity::getUnderlyingValue() yields the
                // DOM value, or already a summed-up
                // conglomerate)
                //
                // Note that this poses problems with our
                // hybrid activity duration (time or min
                // number of frames), since if activities
                // exceed their duration, wrong 'by' start
                // values might arise ('Laser effect')
                maAttributeLayerHolder.get()->setAdditiveMode( animations::AnimationAdditiveMode::REPLACE );
            }
            else
            {
                // apply additive mode to newly created Attribute layer
                maAttributeLayerHolder.get()->setAdditiveMode( mxAnimateNode->getAdditive() );
            }

            // fake normal animation behaviour, even if we
            // show nothing.  This is the appropriate way to
            // handle errors on Activity generation, because
            // maybe all other effects on the slide are
            // correctly initialized (but won't run, if we
            // signal an error here)
            if( !mpActivity.get() )
                return true;

            // supply Activity (and the underlying Animation) with
            // it's AttributeLayer, to perform the animation on
            mpActivity->setTargets( getShape(),
                                    maAttributeLayerHolder.get() );

            // add to activities queue
            return getContext().mrActivitiesQueue.addActivity( mpActivity );
        }

        void AnimationBaseNode::endAnimation()
        {
            // revoke attribute layer, when parent finishes
            maAttributeLayerHolder.reset();

            if( !isDependentSubsettedShape() )
            {
                // for all other shapes, removing the
                // attribute layer quite possibly changes
                // shape display. Thus, force update
                AttributableShapeSharedPtr pShape( getShape() );

                // don't anybody dare to check against
                // pShape->isVisible() here, removing the
                // attribute layer might actually make the
                // shape invisible!
                getContext().mpLayerManager->notifyShapeUpdate( pShape );
            }

            if( mpActivity.get() && mpActivity->isActive() )
            {
                // kill activity, if still running
                mpActivity->dispose();
            }

            // destroy activity, we need to re-generate it
            // when animation is restarted
            mpActivity.reset();
        }

        void AnimationBaseNode::deactivate()
        {
            if( isDependentSubsettedShape() )
            {
                // for dependent subsets, remove subset shape
                // from layer, re-integrate subsetted part
                // back into original shape. For independent
                // subsets, we cannot make any assumptions
                // about subset attribute state relative to
                // master shape, thus, have to keep it. This
                // will effectively re-integrate the subsetted
                // part into the original shape (whose
                // animation will hopefully have ended, too)

                // this statement will save a whole lot of
                // sprites for iterated text effects, since
                // those sprites will only exist during the
                // actual lifetime of the effects
                disableShapeSubset();

                // TODO(P1): We still hold the attribute
                // layer, until the effect fully ends. This
                // might be freed here, too, since the shape
                // still holds it by shared_ptr (we don't even
                // have to care about calling
                // revokeAttributeLayer() later on, since the
                // shape will be deleted, anyway (it's a temp
                // subset shape)).
            }

            BaseNode::deactivate();
        }

        void AnimationBaseNode::end()
        {
            endAnimation();

            BaseNode::end();
        }

        void AnimationBaseNode::notifyDeactivating( const AnimationNodeSharedPtr& rNotifier )
        {
            // NO-OP for all leaf nodes (which typically don't register nowhere)

            // TODO(F1): for end sync functionality, this might indeed be used some day
        }

        bool AnimationBaseNode::hasPendingAnimation() const
        {
            // TODO(F1): This might not always be true. Are there 'inactive'
            // animation nodes?
            return true;
        }

#if defined(VERBOSE) && defined(DBG_UTIL)
        void AnimationBaseNode::showState() const
        {
            BaseNode::showState();

            VERBOSE_TRACE( "AnimationBaseNode info: independent subset=%s",
                           mbIsIndependentSubset ? "y" : "n" );
        }
#endif

        ActivitiesFactory::CommonParameters AnimationBaseNode::fillCommonParameters() const
        {
            double nDuration=0.0;

            // TODO(F3): Duration/End handling is barely there
            if( !(mxAnimateNode->getDuration() >>= nDuration) )
            {
                mxAnimateNode->getEnd() >>= nDuration; // Wah.
            }

            // minimal duration we fallback to (avoid 0 here!)
            nDuration = ::std::max( 0.001, nDuration );

            const bool bAutoReverse( mxAnimateNode->getAutoReverse() );

            ::comphelper::OptionalValue<double> aRepeats;
            double nRepeats;
            if( (mxAnimateNode->getRepeatCount() >>= nRepeats) )
            {
                aRepeats.setValue( nRepeats );
            }
            else
            {
                if( (mxAnimateNode->getRepeatDuration() >>= nRepeats) )
                {
                    // when repeatDuration is given,
                    // autoreverse does _not_ modify the
                    // active duration. Thus, calc repeat
                    // count with already adapted simple
                    // duration (twice the specified duration)

                    // convert duration back to repeat counts
                    if( bAutoReverse )
                        aRepeats.setValue( nRepeats / 2.0*nDuration );
                    else
                        aRepeats.setValue( nRepeats / nDuration );
                }
                else
                {
                    // no double value for both values - Timing::INDEFINITE?
                    animations::Timing eTiming;

                    if( !(mxAnimateNode->getRepeatDuration() >>= eTiming) ||
                        eTiming != animations::Timing_INDEFINITE )
                    {
                        if( !(mxAnimateNode->getRepeatCount() >>= eTiming) ||
                            eTiming != animations::Timing_INDEFINITE )
                        {
                            // no indefinite timing, no other values given -
                            // use simple run, i.e. repeat of 1.0
                            aRepeats.setValue( 1.0 );
                        }
                    }
                }
            }

            // calc accel/decel:
            double nAcceleration = 0.0;
            double nDeceleration = 0.0;
            for ( boost::shared_ptr<BaseNode> node( getSelf() );
                  node.get() != 0; node = node->getParentNode() )
            {
                const uno::Reference<animations::XAnimationNode> xAnimationNode(
                    node->getXAnimationNode() );
                nAcceleration = std::max(
                    nAcceleration, xAnimationNode->getAcceleration() );
                nDeceleration = std::max(
                    nDeceleration, xAnimationNode->getDecelerate() );
            }

            return ActivitiesFactory::CommonParameters(
                EventSharedPtr( new ActivityEnded( getSelf() ) ),
                getContext().mrEventQueue,
                getContext().mrActivitiesQueue,
                nDuration,
                10, // always display at least 10 frames
                bAutoReverse,
                aRepeats,
                nAcceleration,
                nDeceleration,
                getShape(),
                getContext().mpLayerManager );
        }

        AnimationActivitySharedPtr AnimationBaseNode::createActivity() const
        {
            const ::rtl::OUString& rAttrName( mxAnimateNode->getAttributeName() );

            ActivitiesFactory::CommonParameters aParms( fillCommonParameters() );

            const AttributableShapeSharedPtr& rShape( getShape() );

            switch( AnimationFactory::classifyAttributeName( rAttrName ) )
            {
                default:
                case AnimationFactory::CLASS_UNKNOWN_PROPERTY:
                    ENSURE_AND_THROW( false,
                                      "AnimationBaseNode::createActivity(): Unexpected attribute class (unknown or empty attribute name)" );
                    break;

                case AnimationFactory::CLASS_NUMBER_PROPERTY:
                    return ActivitiesFactory::createAnimateActivity( aParms,
                                                                     AnimationFactory::createNumberPropertyAnimation(
                                                                         rAttrName,
                                                                         rShape,
                                                                         getContext().mpLayerManager ),
                                                                     mxAnimateNode );

                case AnimationFactory::CLASS_ENUM_PROPERTY:
                    return ActivitiesFactory::createAnimateActivity( aParms,
                                                                     AnimationFactory::createEnumPropertyAnimation(
                                                                         rAttrName,
                                                                         rShape,
                                                                         getContext().mpLayerManager ),
                                                                     mxAnimateNode );

                case AnimationFactory::CLASS_COLOR_PROPERTY:
                    return ActivitiesFactory::createAnimateActivity( aParms,
                                                                     AnimationFactory::createColorPropertyAnimation(
                                                                         rAttrName,
                                                                         rShape,
                                                                         getContext().mpLayerManager ),
                                                                     mxAnimateNode );

                case AnimationFactory::CLASS_STRING_PROPERTY:
                    return ActivitiesFactory::createAnimateActivity( aParms,
                                                                     AnimationFactory::createStringPropertyAnimation(
                                                                         rAttrName,
                                                                         rShape,
                                                                         getContext().mpLayerManager ),
                                                                     mxAnimateNode );

                case AnimationFactory::CLASS_BOOL_PROPERTY:
                    return ActivitiesFactory::createAnimateActivity( aParms,
                                                                     AnimationFactory::createBoolPropertyAnimation(
                                                                         rAttrName,
                                                                         rShape,
                                                                         getContext().mpLayerManager ),
                                                                     mxAnimateNode );
            }

            return AnimationActivitySharedPtr();
        }

        bool AnimationBaseNode::isSubsettedShape() const
        {
            return mpShapeSubset.get() != NULL;
        }

        bool AnimationBaseNode::isDependentSubsettedShape() const
        {
            return mpShapeSubset.get() != NULL && !mbIsIndependentSubset;
        }

        void AnimationBaseNode::enableShapeSubset()
        {
            if( mpShapeSubset.get() )
                mpShapeSubset->enableSubsetShape();
        }

        void AnimationBaseNode::disableShapeSubset()
        {
            if( mpShapeSubset.get() )
                mpShapeSubset->disableSubsetShape();
        }

        AttributableShapeSharedPtr AnimationBaseNode::getShape() const
        {
            // any subsetting at all?
            if( !mpShapeSubset.get() )
                return mpShape; // nope, plain shape always
            else
                return mpShapeSubset->getSubsetShape();
        }
    }
}
