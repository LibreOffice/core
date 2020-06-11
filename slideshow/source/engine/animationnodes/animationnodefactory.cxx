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


#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/presentation/TextAnimationType.hpp>
#include <com/sun/star/animations/XIterateContainer.hpp>
#include <com/sun/star/presentation/ShapeAnimationSubType.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <basegfx/numeric/ftools.hxx>
#include <sal/log.hxx>

#include <animationnodefactory.hxx>
#include "paralleltimecontainer.hxx"
#include "sequentialtimecontainer.hxx"
#include "propertyanimationnode.hxx"
#include "animationsetnode.hxx"
#include "animationpathmotionnode.hxx"
#include "animationsimulatednode.hxx"
#include "animationcolornode.hxx"
#include "animationtransformnode.hxx"
#include "animationtransitionfilternode.hxx"
#include "animationaudionode.hxx"
#include "animationcommandnode.hxx"
#include "nodetools.hxx"
#include <tools.hxx>

#include <memory>

using namespace ::com::sun::star;

namespace slideshow::internal {

namespace {

// forward declaration needed by NodeCreator
BaseNodeSharedPtr implCreateAnimationNode(
    const uno::Reference< animations::XAnimationNode >&  xNode,
    const BaseContainerNodeSharedPtr&                    rParent,
    const NodeContext&                                   rContext );

class NodeCreator
{
public:
    NodeCreator( BaseContainerNodeSharedPtr&    rParent,
                 const NodeContext&             rContext )
        : mrParent( rParent ), mrContext( rContext ) {}

    void operator()(
        const uno::Reference< animations::XAnimationNode >& xChildNode ) const
    {
        createChild( xChildNode, mrContext );
    }

protected:
    void createChild(
        const uno::Reference< animations::XAnimationNode >&   xChildNode,
        const NodeContext&                                    rContext ) const
    {
        BaseNodeSharedPtr pChild( implCreateAnimationNode( xChildNode,
                                                           mrParent,
                                                           rContext ) );

        OSL_ENSURE( pChild,
                    "NodeCreator::operator(): child creation failed" );

        // TODO(Q1): This yields circular references, which, it seems, is
        // unavoidable here
        if( pChild )
            mrParent->appendChildNode( pChild );
    }

    BaseContainerNodeSharedPtr&     mrParent;
    const NodeContext&              mrContext;
};

/** Same as NodeCreator, only that NodeContext's
    SubsetShape is cloned for every child node.

    This is used for iterated animation node generation
*/
class CloningNodeCreator : private NodeCreator
{
public:
    CloningNodeCreator( BaseContainerNodeSharedPtr& rParent,
                        const NodeContext&          rContext )
        : NodeCreator( rParent, rContext ) {}

    void operator()(
        const uno::Reference< animations::XAnimationNode >& xChildNode ) const
    {
        NodeContext aContext( mrContext );

        // TODO(Q1): There's a catch here. If you clone a
        // subset whose actual subsetting has already been
        // realized (i.e. if enableSubsetShape() has been
        // called already), and the original of your clone
        // goes out of scope, then your subset will be
        // gone (SubsettableShapeManager::revokeSubset() be
        // called). As of now, this behaviour is not
        // triggered here (we either clone, XOR we enable
        // subset initially), but one might consider
        // reworking DrawShape/ShapeSubset to avoid this.

        // clone ShapeSubset, since each node needs their
        // own version of the ShapeSubset (otherwise,
        // e.g. activity counting does not work - subset
        // would be removed after first animation node
        // disables it).

        // NOTE: this is only a problem for animation
        // nodes that explicitly call
        // disableSubsetShape(). Independent shape subsets
        // (like those created for ParagraphTargets)
        // solely rely on the ShapeSubset destructor to
        // normalize things, which does the right thing
        // here: the subset is only removed after _the
        // last_ animation node releases the shared ptr.
        aContext.mpMasterShapeSubset =
            std::make_shared<ShapeSubset>( *aContext.mpMasterShapeSubset );

        createChild( xChildNode, aContext );
    }
};

/** Create animation nodes for text iterations

    This method clones the animation nodes below xIterNode
    for every iterated shape entity.
*/
bool implCreateIteratedNodes(
    const uno::Reference< animations::XIterateContainer >&    xIterNode,
    BaseContainerNodeSharedPtr&                               rParent,
    const NodeContext&                                        rContext )
{
    ENSURE_OR_THROW( xIterNode.is(),
                      "implCreateIteratedNodes(): Invalid node" );

    const double nIntervalTimeout( xIterNode->getIterateInterval() );

    // valid iterate interval? We're ruling out monstrous
    // values here, to avoid pseudo 'hangs' in the
    // presentation
    if( nIntervalTimeout < 0.0 ||
        nIntervalTimeout > 1000.0 )
    {
        return false; // not an active iteration
    }

    if( ::basegfx::fTools::equalZero( nIntervalTimeout ) )
        SAL_INFO("slideshow", "implCreateIteratedNodes(): "
                   "iterate interval close to zero, there's "
                   "no point in defining such an effect "
                   "(visually equivalent to whole-shape effect)" );

    // Determine target shape (or subset)
    // ==================================

    // TODO(E1): I'm not too sure what to expect here...
    ENSURE_OR_RETURN_FALSE(
        xIterNode->getTarget().hasValue(),
        "implCreateIteratedNodes(): no target on ITERATE node" );

    uno::Reference< drawing::XShape > xTargetShape( xIterNode->getTarget(),
                                                    uno::UNO_QUERY );

    presentation::ParagraphTarget aTarget;
    sal_Int16                     nSubItem( xIterNode->getSubItem() );
    bool                          bParagraphTarget( false );

    if( !xTargetShape.is() )
    {
        // no shape provided. Maybe a ParagraphTarget?
        if( !(xIterNode->getTarget() >>= aTarget) )
            ENSURE_OR_RETURN_FALSE(
                false,
                "implCreateIteratedNodes(): could not extract any "
                "target information" );

        xTargetShape = aTarget.Shape;

        ENSURE_OR_RETURN_FALSE(
            xTargetShape.is(),
            "implCreateIteratedNodes(): invalid shape in ParagraphTarget" );

        // we've a paragraph target to iterate over, thus,
        // the whole animation container refers only to
        // the text
        nSubItem = presentation::ShapeAnimationSubType::ONLY_TEXT;

        bParagraphTarget = true;
    }

    // Lookup shape, and fill NodeContext
    // ==================================

    AttributableShapeSharedPtr  pTargetShape(
        lookupAttributableShape( rContext.maContext.mpSubsettableShapeManager,
                                 xTargetShape ) );

    const DocTreeNodeSupplier& rTreeNodeSupplier(
        pTargetShape->getTreeNodeSupplier() );

    ShapeSubsetSharedPtr pTargetSubset;

    NodeContext aContext( rContext );

    // paragraph targets already need a subset as the
    // master shape (they're representing only a single
    // paragraph)
    if( bParagraphTarget )
    {
        ENSURE_OR_RETURN_FALSE(
            aTarget.Paragraph >= 0 &&
            rTreeNodeSupplier.getNumberOfTreeNodes(
                DocTreeNode::NodeType::LogicalParagraph ) > aTarget.Paragraph,
            "implCreateIteratedNodes(): paragraph index out of range" );

        pTargetSubset =
            std::make_shared<ShapeSubset>(
                pTargetShape,
                // retrieve index aTarget.Paragraph of
                // type PARAGRAPH from this shape
                rTreeNodeSupplier.getTreeNode(
                    aTarget.Paragraph,
                    DocTreeNode::NodeType::LogicalParagraph ),
                rContext.maContext.mpSubsettableShapeManager );

        // iterate target is not the whole shape, but only
        // the selected paragraph - subset _must_ be
        // independent, to be able to affect visibility
        // independent of master shape
        aContext.mbIsIndependentSubset = true;

        // already enable parent subset right here, to
        // make potentially generated subsets subtract
        // their content from the parent subset (and not
        // the original shape). Otherwise, already
        // subsetted parents (e.g. paragraphs) would not
        // have their characters removed, when the child
        // iterations start.
        // Furthermore, the setup of initial shape
        // attributes of course needs the subset shape
        // generated, to apply e.g. visibility changes.
        pTargetSubset->enableSubsetShape();
    }
    else
    {
        pTargetSubset =
            std::make_shared<ShapeSubset>( pTargetShape,
                             rContext.maContext.mpSubsettableShapeManager );
    }

    aContext.mpMasterShapeSubset = pTargetSubset;
    uno::Reference< animations::XAnimationNode > xNode( xIterNode,
                                                        uno::UNO_QUERY_THROW );

    // Generate subsets
    // ================

    if( bParagraphTarget ||
        nSubItem != presentation::ShapeAnimationSubType::ONLY_TEXT )
    {
        // prepend with animations for
        // full Shape (will be subtracted
        // from the subset parts within
        // the Shape::createSubset()
        // method). For ONLY_TEXT effects,
        // we skip this part, to animate
        // only the text.

        // OR

        // prepend with subset animation for full
        // _paragraph_, from which the individual
        // paragraph subsets are subtracted. Note that the
        // subitem is superfluous here, we always assume
        // ONLY_TEXT, if a paragraph is referenced as the
        // master of an iteration effect.
        NodeCreator aCreator( rParent, aContext );
        if( !for_each_childNode( xNode, aCreator ) )
        {
            ENSURE_OR_RETURN_FALSE(
                false,
                "implCreateIteratedNodes(): iterated child node creation failed" );
        }
    }

    // TODO(F2): This does not do the correct
    // thing. Having nSubItem be set to ONLY_BACKGROUND
    // should result in the text staying unanimated in the
    // foreground, while the shape moves in the background
    // (this behaviour is perfectly possible with the
    // slideshow engine, only that the text won't be
    // currently visible, because animations are always in
    // the foreground)
    if( nSubItem != presentation::ShapeAnimationSubType::ONLY_BACKGROUND )
    {
        // determine type of subitem iteration (logical
        // text unit to animate)
        DocTreeNode::NodeType eIterateNodeType(
            DocTreeNode::NodeType::LogicalCharacterCell );

        switch( xIterNode->getIterateType() )
        {
        case presentation::TextAnimationType::BY_PARAGRAPH:
            eIterateNodeType = DocTreeNode::NodeType::LogicalParagraph;
            break;

        case presentation::TextAnimationType::BY_WORD:
            eIterateNodeType = DocTreeNode::NodeType::LogicalWord;
            break;

        case presentation::TextAnimationType::BY_LETTER:
            eIterateNodeType = DocTreeNode::NodeType::LogicalCharacterCell;
            break;

        default:
            ENSURE_OR_THROW(
                false, "implCreateIteratedNodes(): "
                "Unexpected IterateType on XIterateContainer");
            break;
        }

        if( bParagraphTarget &&
            eIterateNodeType != DocTreeNode::NodeType::LogicalWord &&
            eIterateNodeType != DocTreeNode::NodeType::LogicalCharacterCell )
        {
            // will not animate the whole paragraph, when
            // only the paragraph is animated at all.
            OSL_FAIL( "implCreateIteratedNodes(): Ignoring paragraph iteration for paragraph master" );
        }
        else
        {
            // setup iteration parameters


            // iterate target is the whole shape (or the
            // whole parent subshape), thus, can save
            // loads of subset shapes by generating them
            // only when the effects become active -
            // before and after the effect active
            // duration, all attributes are shared by
            // master shape and subset (since the iterated
            // effects are all the same).
            aContext.mbIsIndependentSubset = false;

            // determine number of nodes for given subitem
            // type
            sal_Int32 nTreeNodes( 0 );
            if( bParagraphTarget )
            {
                // create the iterated subset _relative_ to
                // the given paragraph index (i.e. animate the
                // given subset type, but only when it's part
                // of the given paragraph)
                nTreeNodes = rTreeNodeSupplier.getNumberOfSubsetTreeNodes(
                    pTargetSubset->getSubset(),
                    eIterateNodeType );
            }
            else
            {
                // generate normal subset
                nTreeNodes = rTreeNodeSupplier.getNumberOfTreeNodes(
                    eIterateNodeType );
            }


            // iterate node, generate copies of the children for each subset


            // NodeContext::mnStartDelay contains additional node delay.
            // This will make the duplicated nodes for each iteration start
            // increasingly later.
            aContext.mnStartDelay = nIntervalTimeout;

            for( sal_Int32 i=0; i<nTreeNodes; ++i )
            {
                // create subset with the corresponding tree nodes
                if( bParagraphTarget )
                {
                    // create subsets relative to paragraph subset
                    aContext.mpMasterShapeSubset =
                        std::make_shared<ShapeSubset>(
                            pTargetSubset,
                            rTreeNodeSupplier.getSubsetTreeNode(
                                pTargetSubset->getSubset(),
                                i,
                                eIterateNodeType ) );
                }
                else
                {
                    // create subsets from main shape
                    aContext.mpMasterShapeSubset =
                        std::make_shared<ShapeSubset>( pTargetSubset,
                                         rTreeNodeSupplier.getTreeNode(
                                             i,
                                             eIterateNodeType ) );
                }

                CloningNodeCreator aCreator( rParent, aContext );
                if( !for_each_childNode( xNode, aCreator ) )
                {
                    ENSURE_OR_RETURN_FALSE(
                        false, "implCreateIteratedNodes(): "
                        "iterated child node creation failed" );
                }

                aContext.mnStartDelay += nIntervalTimeout;
            }
        }
    }

    // done with iterate child generation
    return true;
}

BaseNodeSharedPtr implCreateAnimationNode(
    const uno::Reference< animations::XAnimationNode >&  xNode,
    const BaseContainerNodeSharedPtr&                    rParent,
    const NodeContext&                                   rContext )
{
    ENSURE_OR_THROW( xNode.is(),
                      "implCreateAnimationNode(): invalid XAnimationNode" );

    BaseNodeSharedPtr           pCreatedNode;
    BaseContainerNodeSharedPtr  pCreatedContainer;

    // create the internal node, corresponding to xNode
    switch( xNode->getType() )
    {
    case animations::AnimationNodeType::CUSTOM:
        OSL_FAIL( "implCreateAnimationNode(): "
                    "CUSTOM not yet implemented" );
        return pCreatedNode;

    case animations::AnimationNodeType::PAR:
        pCreatedNode = pCreatedContainer =
            std::make_shared<ParallelTimeContainer>( xNode, rParent, rContext );
        break;

    case animations::AnimationNodeType::ITERATE:
        // map iterate container to ParallelTimeContainer.
        // the iterating functionality is to be found
        // below, (see method implCreateIteratedNodes)
        pCreatedNode = pCreatedContainer =
            std::make_shared<ParallelTimeContainer>( xNode, rParent, rContext );
        break;

    case animations::AnimationNodeType::SEQ:
        pCreatedNode = pCreatedContainer =
            std::make_shared<SequentialTimeContainer>( xNode, rParent, rContext );
        break;

    case animations::AnimationNodeType::ANIMATE:
        pCreatedNode = std::make_shared<PropertyAnimationNode>(
                                xNode, rParent, rContext );
        break;

    case animations::AnimationNodeType::SET:
        pCreatedNode = std::make_shared<AnimationSetNode>(
                                xNode, rParent, rContext );
        break;

    case animations::AnimationNodeType::ANIMATEMOTION:
        pCreatedNode = std::make_shared<AnimationPathMotionNode>(
                                xNode, rParent, rContext );
        break;

    case animations::AnimationNodeType::ANIMATECOLOR:
        pCreatedNode = std::make_shared<AnimationColorNode>(
                                xNode, rParent, rContext );
        break;

    case animations::AnimationNodeType::ANIMATETRANSFORM:
        pCreatedNode = std::make_shared<AnimationTransformNode>(
                                xNode, rParent, rContext );
        break;

    case animations::AnimationNodeType::ANIMATESIMULATED:
        pCreatedNode = std::make_shared<AnimationSimulatedNode>(
                                xNode, rParent, rContext );
        break;

    case animations::AnimationNodeType::TRANSITIONFILTER:
        pCreatedNode = std::make_shared<AnimationTransitionFilterNode>(
                                xNode, rParent, rContext );
        break;

    case animations::AnimationNodeType::AUDIO:
        pCreatedNode = std::make_shared<AnimationAudioNode>(
                                xNode, rParent, rContext );
        break;

    case animations::AnimationNodeType::COMMAND:
        pCreatedNode = std::make_shared<AnimationCommandNode>(
                                xNode, rParent, rContext );
        break;

    default:
        OSL_FAIL( "implCreateAnimationNode(): "
                    "invalid AnimationNodeType" );
        return pCreatedNode;
    }

    // TODO(Q1): This yields circular references, which, it seems, is
    // unavoidable here

    // HACK: node objects need shared_ptr to themselves,
    // which we pass them here.
    pCreatedNode->setSelf( pCreatedNode );

    // if we've got a container node object, recursively add
    // its children
    if( pCreatedContainer )
    {
        uno::Reference< animations::XIterateContainer > xIterNode(
            xNode, uno::UNO_QUERY );

        // when this node is an XIterateContainer with
        // active iterations, this method will generate
        // the appropriate children
        if( xIterNode.is() )
        {
            // note that implCreateIteratedNodes() might
            // choose not to generate any child nodes
            // (e.g. when the iterate timeout is outside
            // sensible limits). Then, no child nodes are
            // generated at all, since typically, child
            // node attribute are incomplete for iteration
            // children.
            implCreateIteratedNodes( xIterNode,
                                     pCreatedContainer,
                                     rContext );
        }
        else
        {
            // no iterate subset node, just plain child generation now
            NodeCreator aCreator( pCreatedContainer, rContext );
            if( !for_each_childNode( xNode, aCreator ) )
            {
                OSL_FAIL( "implCreateAnimationNode(): "
                            "child node creation failed" );
                return BaseNodeSharedPtr();
            }
        }
    }

    return pCreatedNode;
}

} // anon namespace

AnimationNodeSharedPtr AnimationNodeFactory::createAnimationNode(
    const uno::Reference< animations::XAnimationNode >&   xNode,
    const ::basegfx::B2DVector&                           rSlideSize,
    const SlideShowContext&                               rContext )
{
    ENSURE_OR_THROW(
        xNode.is(),
        "AnimationNodeFactory::createAnimationNode(): invalid XAnimationNode" );

    return implCreateAnimationNode(
                                  xNode,
                                  BaseContainerNodeSharedPtr(), // no parent
                                  NodeContext( rContext,
                                               rSlideSize ));
}

#if defined(DBG_UTIL)
void AnimationNodeFactory::showTree( AnimationNodeSharedPtr const & pRootNode )
{
    if( pRootNode )
        DEBUG_NODES_SHOWTREE( std::dynamic_pointer_cast<BaseContainerNode>(
                                  pRootNode).get() );
}
#endif

} // namespace slideshow

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
