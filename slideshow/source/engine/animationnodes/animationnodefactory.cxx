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
#include <canvas/verbosetrace.hxx>

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/animations/XAnimate.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <com/sun/star/presentation/TextAnimationType.hpp>
#include <com/sun/star/animations/XAnimateSet.hpp>
#include <com/sun/star/animations/XIterateContainer.hpp>
#include <com/sun/star/presentation/ShapeAnimationSubType.hpp>
#include <com/sun/star/animations/XAnimateMotion.hpp>
#include <com/sun/star/animations/XAnimateColor.hpp>
#include <com/sun/star/animations/XAnimateTransform.hpp>
#include <com/sun/star/animations/AnimationTransformType.hpp>
#include <com/sun/star/animations/XTransitionFilter.hpp>
#include <com/sun/star/animations/XAudio.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <animations/animationnodehelper.hxx>
#include <basegfx/numeric/ftools.hxx>

#include "animationnodefactory.hxx"
#include "paralleltimecontainer.hxx"
#include "sequentialtimecontainer.hxx"
#include "propertyanimationnode.hxx"
#include "animationsetnode.hxx"
#include "animationpathmotionnode.hxx"
#include "animationcolornode.hxx"
#include "animationtransformnode.hxx"
#include "animationtransitionfilternode.hxx"
#include "animationaudionode.hxx"
#include "animationcommandnode.hxx"
#include "nodetools.hxx"
#include "tools.hxx"

#include <boost/shared_ptr.hpp>

using namespace ::com::sun::star;

namespace slideshow {
namespace internal {

namespace {


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

        
        
        
        
        
        
        
        
        
        

        
        
        
        
        
        //
        
        
        
        
        
        
        
        
        aContext.mpMasterShapeSubset.reset(
            new ShapeSubset( *aContext.mpMasterShapeSubset ) );

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

    
    
    
    if( nIntervalTimeout < 0.0 ||
        nIntervalTimeout > 1000.0 )
    {
        return false; 
    }

    if( ::basegfx::fTools::equalZero( nIntervalTimeout ) )
        OSL_TRACE( "implCreateIteratedNodes(): "
                   "iterate interval close to zero, there's "
                   "no point in defining such an effect "
                   "(visually equivalent to whole-shape effect)" );

    
    

    
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
        
        if( !(xIterNode->getTarget() >>= aTarget) )
            ENSURE_OR_RETURN_FALSE(
                false,
                "implCreateIteratedNodes(): could not extract any "
                "target information" );

        xTargetShape = aTarget.Shape;

        ENSURE_OR_RETURN_FALSE(
            xTargetShape.is(),
            "implCreateIteratedNodes(): invalid shape in ParagraphTarget" );

        
        
        
        nSubItem = presentation::ShapeAnimationSubType::ONLY_TEXT;

        bParagraphTarget = true;
    }

    
    

    AttributableShapeSharedPtr  pTargetShape(
        lookupAttributableShape( rContext.maContext.mpSubsettableShapeManager,
                                 xTargetShape ) );

    const DocTreeNodeSupplier& rTreeNodeSupplier(
        pTargetShape->getTreeNodeSupplier() );

    ShapeSubsetSharedPtr pTargetSubset;

    NodeContext aContext( rContext );

    
    
    
    if( bParagraphTarget )
    {
        ENSURE_OR_RETURN_FALSE(
            aTarget.Paragraph >= 0 &&
            rTreeNodeSupplier.getNumberOfTreeNodes(
                DocTreeNode::NODETYPE_LOGICAL_PARAGRAPH ) > aTarget.Paragraph,
            "implCreateIteratedNodes(): paragraph index out of range" );

        pTargetSubset.reset(
            new ShapeSubset(
                pTargetShape,
                
                
                rTreeNodeSupplier.getTreeNode(
                    aTarget.Paragraph,
                    DocTreeNode::NODETYPE_LOGICAL_PARAGRAPH ),
                rContext.maContext.mpSubsettableShapeManager ) );

        
        
        
        
        aContext.mbIsIndependentSubset = true;

        
        
        
        
        
        
        
        
        
        
        pTargetSubset->enableSubsetShape();
    }
    else
    {
        pTargetSubset.reset(
            new ShapeSubset( pTargetShape,
                             rContext.maContext.mpSubsettableShapeManager ));
    }

    aContext.mpMasterShapeSubset = pTargetSubset;
    uno::Reference< animations::XAnimationNode > xNode( xIterNode,
                                                        uno::UNO_QUERY_THROW );

    
    

    if( bParagraphTarget ||
        nSubItem != presentation::ShapeAnimationSubType::ONLY_TEXT )
    {
        
        
        
        
        
        
        
        //
        
        //
        
        
        
        
        
        
        NodeCreator aCreator( rParent, aContext );
        if( !::anim::for_each_childNode( xNode,
                                         aCreator ) )
        {
            ENSURE_OR_RETURN_FALSE(
                false,
                "implCreateIteratedNodes(): iterated child node creation failed" );
        }
    }

    
    
    
    
    
    
    
    
    if( nSubItem != presentation::ShapeAnimationSubType::ONLY_BACKGROUND )
    {
        
        
        DocTreeNode::NodeType eIterateNodeType(
            DocTreeNode::NODETYPE_LOGICAL_CHARACTER_CELL );

        switch( xIterNode->getIterateType() )
        {
        case presentation::TextAnimationType::BY_PARAGRAPH:
            eIterateNodeType = DocTreeNode::NODETYPE_LOGICAL_PARAGRAPH;
            break;

        case presentation::TextAnimationType::BY_WORD:
            eIterateNodeType = DocTreeNode::NODETYPE_LOGICAL_WORD;
            break;

        case presentation::TextAnimationType::BY_LETTER:
            eIterateNodeType = DocTreeNode::NODETYPE_LOGICAL_CHARACTER_CELL;
            break;

        default:
            ENSURE_OR_THROW(
                false, "implCreateIteratedNodes(): "
                "Unexpected IterateType on XIterateContainer");
            break;
        }

        if( bParagraphTarget &&
            eIterateNodeType != DocTreeNode::NODETYPE_LOGICAL_WORD &&
            eIterateNodeType != DocTreeNode::NODETYPE_LOGICAL_CHARACTER_CELL )
        {
            
            
            OSL_FAIL( "implCreateIteratedNodes(): Ignoring paragraph iteration for paragraph master" );
        }
        else
        {
            
            

            
            
            
            
            
            
            
            
            aContext.mbIsIndependentSubset = false;

            
            
            sal_Int32 nTreeNodes( 0 );
            if( bParagraphTarget )
            {
                
                
                
                
                nTreeNodes = rTreeNodeSupplier.getNumberOfSubsetTreeNodes(
                    pTargetSubset->getSubset(),
                    eIterateNodeType );
            }
            else
            {
                
                nTreeNodes = rTreeNodeSupplier.getNumberOfTreeNodes(
                    eIterateNodeType );
            }


            
            

            
            
            
            aContext.mnStartDelay = nIntervalTimeout;

            for( sal_Int32 i=0; i<nTreeNodes; ++i )
            {
                
                if( bParagraphTarget )
                {
                    
                    aContext.mpMasterShapeSubset.reset(
                        new ShapeSubset(
                            pTargetSubset,
                            rTreeNodeSupplier.getSubsetTreeNode(
                                pTargetSubset->getSubset(),
                                i,
                                eIterateNodeType ) ) );
                }
                else
                {
                    
                    aContext.mpMasterShapeSubset.reset(
                        new ShapeSubset( pTargetSubset,
                                         rTreeNodeSupplier.getTreeNode(
                                             i,
                                             eIterateNodeType ) ) );
                }

                CloningNodeCreator aCreator( rParent, aContext );
                if( !::anim::for_each_childNode( xNode,
                                                 aCreator ) )
                {
                    ENSURE_OR_RETURN_FALSE(
                        false, "implCreateIteratedNodes(): "
                        "iterated child node creation failed" );
                }

                aContext.mnStartDelay += nIntervalTimeout;
            }
        }
    }

    
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

    
    switch( xNode->getType() )
    {
    case animations::AnimationNodeType::CUSTOM:
        OSL_FAIL( "implCreateAnimationNode(): "
                    "CUSTOM not yet implemented" );
        return pCreatedNode;

    case animations::AnimationNodeType::PAR:
        pCreatedNode = pCreatedContainer = BaseContainerNodeSharedPtr(
            new ParallelTimeContainer( xNode, rParent, rContext ) );
        break;

    case animations::AnimationNodeType::ITERATE:
        
        
        
        pCreatedNode = pCreatedContainer = BaseContainerNodeSharedPtr(
            new ParallelTimeContainer( xNode, rParent, rContext ) );
        break;

    case animations::AnimationNodeType::SEQ:
        pCreatedNode = pCreatedContainer = BaseContainerNodeSharedPtr(
            new SequentialTimeContainer( xNode, rParent, rContext ) );
        break;

    case animations::AnimationNodeType::ANIMATE:
        pCreatedNode.reset( new PropertyAnimationNode(
                                xNode, rParent, rContext ) );
        break;

    case animations::AnimationNodeType::SET:
        pCreatedNode.reset( new AnimationSetNode(
                                xNode, rParent, rContext ) );
        break;

    case animations::AnimationNodeType::ANIMATEMOTION:
        pCreatedNode.reset( new AnimationPathMotionNode(
                                xNode, rParent, rContext ) );
        break;

    case animations::AnimationNodeType::ANIMATECOLOR:
        pCreatedNode.reset( new AnimationColorNode(
                                xNode, rParent, rContext ) );
        break;

    case animations::AnimationNodeType::ANIMATETRANSFORM:
        pCreatedNode.reset( new AnimationTransformNode(
                                xNode, rParent, rContext ) );
        break;

    case animations::AnimationNodeType::TRANSITIONFILTER:
        pCreatedNode.reset( new AnimationTransitionFilterNode(
                                xNode, rParent, rContext ) );
        break;

    case animations::AnimationNodeType::AUDIO:
        pCreatedNode.reset( new AnimationAudioNode(
                                xNode, rParent, rContext ) );
        break;

    case animations::AnimationNodeType::COMMAND:
        pCreatedNode.reset( new AnimationCommandNode(
                                xNode, rParent, rContext ) );
        break;

    default:
        OSL_FAIL( "implCreateAnimationNode(): "
                    "invalid AnimationNodeType" );
        return pCreatedNode;
    }

    
    

    
    
    pCreatedNode->setSelf( pCreatedNode );

    
    
    if( pCreatedContainer )
    {
        uno::Reference< animations::XIterateContainer > xIterNode(
            xNode, uno::UNO_QUERY );

        
        
        
        if( xIterNode.is() )
        {
            
            
            
            
            
            
            
            implCreateIteratedNodes( xIterNode,
                                     pCreatedContainer,
                                     rContext );
        }
        else
        {
            
            NodeCreator aCreator( pCreatedContainer, rContext );
            if( !::anim::for_each_childNode( xNode, aCreator ) )
            {
                OSL_FAIL( "implCreateAnimationNode(): "
                            "child node creation failed" );
                return BaseNodeSharedPtr();
            }
        }
    }

    return pCreatedNode;
}

} 

AnimationNodeSharedPtr AnimationNodeFactory::createAnimationNode(
    const uno::Reference< animations::XAnimationNode >&   xNode,
    const ::basegfx::B2DVector&                           rSlideSize,
    const SlideShowContext&                               rContext )
{
    ENSURE_OR_THROW(
        xNode.is(),
        "AnimationNodeFactory::createAnimationNode(): invalid XAnimationNode" );

    return BaseNodeSharedPtr( implCreateAnimationNode(
                                  xNode,
                                  BaseContainerNodeSharedPtr(), 
                                  NodeContext( rContext,
                                               rSlideSize )));
}

#if OSL_DEBUG_LEVEL >= 2 && defined(DBG_UTIL)
void AnimationNodeFactory::showTree( AnimationNodeSharedPtr& pRootNode )
{
    if( pRootNode )
        DEBUG_NODES_SHOWTREE( boost::dynamic_pointer_cast<BaseContainerNode>(
                                  pRootNode).get() );
}
#endif

} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
