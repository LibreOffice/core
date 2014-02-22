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
#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/anytostring.hxx>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/animations/Timing.hpp>
#include <com/sun/star/animations/AnimationAdditiveMode.hpp>
#include <com/sun/star/presentation/ShapeAnimationSubType.hpp>

#include "nodetools.hxx"
#include "doctreenode.hxx"
#include "animationbasenode.hxx"
#include "delayevent.hxx"
#include "framerate.hxx"

#include <boost/bind.hpp>
#include <boost/optional.hpp>
#include <algorithm>

using namespace com::sun::star;

namespace slideshow {
namespace internal {

AnimationBaseNode::AnimationBaseNode(
    const uno::Reference< animations::XAnimationNode >&   xNode,
    const BaseContainerNodeSharedPtr&                     rParent,
    const NodeContext&                                    rContext )
    : BaseNode( xNode, rParent, rContext ),
      mxAnimateNode( xNode, uno::UNO_QUERY_THROW ),
      maAttributeLayerHolder(),
      maSlideSize( rContext.maSlideSize ),
      mpActivity(),
      mpShape(),
      mpShapeSubset(),
      mpSubsetManager(rContext.maContext.mpSubsettableShapeManager),
      mbIsIndependentSubset( rContext.mbIsIndependentSubset )
{
    
    

    
    uno::Reference< drawing::XShape > xShape( mxAnimateNode->getTarget(),
                                              uno::UNO_QUERY );

    
    //
    
    
    //
    
    
    //
    
    
    //
    
    
    //
    
    
    if( rContext.mpMasterShapeSubset )
    {
        if( rContext.mpMasterShapeSubset->isFullSet() )
        {
            
            mpShape = rContext.mpMasterShapeSubset->getSubsetShape();
        }
        else
        {
            
            mpShapeSubset = rContext.mpMasterShapeSubset;
        }
    }
    else
    {
        
        

        if( xShape.is() )
        {
            mpShape = lookupAttributableShape( getContext().mpSubsettableShapeManager,
                                               xShape );
        }
        else
        {
            
            presentation::ParagraphTarget aTarget;

            if( !(mxAnimateNode->getTarget() >>= aTarget) )
                ENSURE_OR_THROW(
                    false, "could not extract any target information" );

            xShape = aTarget.Shape;

            ENSURE_OR_THROW( xShape.is(), "invalid shape in ParagraphTarget" );

            mpShape = lookupAttributableShape( getContext().mpSubsettableShapeManager,
                                               xShape );

            
            
            
            OSL_ENSURE(
                mxAnimateNode->getSubItem() ==
                presentation::ShapeAnimationSubType::ONLY_TEXT ||
                mxAnimateNode->getSubItem() ==
                presentation::ShapeAnimationSubType::AS_WHOLE,
                "ParagraphTarget given, but subitem not AS_TEXT or AS_WHOLE? "
                "Make up your mind, I'll ignore the subitem." );

            
            
            if( aTarget.Paragraph >= 0 &&
                mpShape->getTreeNodeSupplier().getNumberOfTreeNodes(
                    DocTreeNode::NODETYPE_LOGICAL_PARAGRAPH) > aTarget.Paragraph )
            {
                const DocTreeNode& rTreeNode(
                    mpShape->getTreeNodeSupplier().getTreeNode(
                        aTarget.Paragraph,
                        DocTreeNode::NODETYPE_LOGICAL_PARAGRAPH ) );

                
                
                
                
                
                mpShapeSubset.reset(
                    new ShapeSubset( mpShape,
                                     rTreeNode,
                                     mpSubsetManager ));

                
                
                
                
                
                
                
                
                
                
                
                
                
                mbIsIndependentSubset = true;

                
                
                
                
                
                mpShapeSubset->enableSubsetShape();
            }
        }
    }
}

void AnimationBaseNode::dispose()
{
    if (mpActivity) {
        mpActivity->dispose();
        mpActivity.reset();
    }

    maAttributeLayerHolder.reset();
    mxAnimateNode.clear();
    mpShape.reset();
    mpShapeSubset.reset();

    BaseNode::dispose();
}

bool AnimationBaseNode::init_st()
{
    
    if (mpActivity) {
        mpActivity->dispose();
        mpActivity.reset();
    }

    
    
    
    

    try {
        
        
        mpActivity = createActivity();
    }
    catch (uno::Exception const&) {
        OSL_FAIL( OUStringToOString(
                        comphelper::anyToString(cppu::getCaughtException()),
                        RTL_TEXTENCODING_UTF8).getStr() );
        
        
        
    }
    return true;
}

bool AnimationBaseNode::resolve_st()
{
    
    
    
    
    
    
    
    
    if (isDependentSubsettedShape() && mpShapeSubset) {
        mpShapeSubset->enableSubsetShape();
    }
    return true;
}

void AnimationBaseNode::activate_st()
{
    
    maAttributeLayerHolder.createAttributeLayer( getShape() );

    ENSURE_OR_THROW( maAttributeLayerHolder.get(),
                      "Could not generate shape attribute layer" );

    
    
    
    
    
    

    
    
    
    
    
    if( mxAnimateNode->getBy().hasValue() &&
        !mxAnimateNode->getTo().hasValue() &&
        !mxAnimateNode->getFrom().hasValue() )
    {
        
        
        
        
        
        
        
        
        
        //
        
        
        
        
        
        maAttributeLayerHolder.get()->setAdditiveMode(
            animations::AnimationAdditiveMode::REPLACE );
    }
    else
    {
        
        maAttributeLayerHolder.get()->setAdditiveMode(
            mxAnimateNode->getAdditive() );
    }

    
    
    
    
    
    
    if (mpActivity) {
        
        
        mpActivity->setTargets( getShape(), maAttributeLayerHolder.get() );

        
        getContext().mrActivitiesQueue.addActivity( mpActivity );
    }
    else {
        
        
        BaseNode::scheduleDeactivationEvent();
    }
}

void AnimationBaseNode::deactivate_st( NodeState eDestState )
{
    if (eDestState == FROZEN) {
        if (mpActivity)
            mpActivity->end();
    }

    if (isDependentSubsettedShape()) {
        
        
        
        
        
        
        
        
        

        
        
        
        
        if (mpShapeSubset) {
            mpShapeSubset->disableSubsetShape();
        }
    }

    if (eDestState == ENDED) {

        
        maAttributeLayerHolder.reset();

        if (! isDependentSubsettedShape()) {

            
            
            
            AttributableShapeSharedPtr const pShape( getShape() );

            
            
            
            
            getContext().mpSubsettableShapeManager->notifyShapeUpdate( pShape );
        }

        if (mpActivity) {
            
            mpActivity->dispose();
            mpActivity.reset();
        }
    }
}

bool AnimationBaseNode::hasPendingAnimation() const
{
    
    
    return true;
}

#if OSL_DEBUG_LEVEL >= 2 && defined(DBG_UTIL)
void AnimationBaseNode::showState() const
{
    BaseNode::showState();

    VERBOSE_TRACE( "AnimationBaseNode info: independent subset=%s",
                   mbIsIndependentSubset ? "y" : "n" );
}
#endif

ActivitiesFactory::CommonParameters
AnimationBaseNode::fillCommonParameters() const
{
    double nDuration = 0.0;

    
    if( !(mxAnimateNode->getDuration() >>= nDuration) ) {
        mxAnimateNode->getEnd() >>= nDuration; 
    }

    
    nDuration = ::std::max( 0.001, nDuration );

    const bool bAutoReverse( mxAnimateNode->getAutoReverse() );

    boost::optional<double> aRepeats;
    double nRepeats = 0;
    if( (mxAnimateNode->getRepeatCount() >>= nRepeats) ) {
        aRepeats.reset( nRepeats );
    }
    else {
        if( (mxAnimateNode->getRepeatDuration() >>= nRepeats) ) {
            
            
            
            
            

            
            if( bAutoReverse )
                aRepeats.reset( nRepeats / (2.0 * nDuration) );
            else
                aRepeats.reset( nRepeats / nDuration );
        }
        else
        {
            
            animations::Timing eTiming;

            if( !(mxAnimateNode->getRepeatDuration() >>= eTiming) ||
                eTiming != animations::Timing_INDEFINITE )
            {
                if( !(mxAnimateNode->getRepeatCount() >>= eTiming) ||
                    eTiming != animations::Timing_INDEFINITE )
                {
                    
                    
                    aRepeats.reset( 1.0 );
                }
            }
        }
    }

    
    double nAcceleration = 0.0;
    double nDeceleration = 0.0;
    BaseNodeSharedPtr const pSelf( getSelf() );
    for ( boost::shared_ptr<BaseNode> pNode( pSelf );
          pNode; pNode = pNode->getParentNode() )
    {
        uno::Reference<animations::XAnimationNode> const xAnimationNode(
            pNode->getXAnimationNode() );
        nAcceleration = std::max( nAcceleration,
                                  xAnimationNode->getAcceleration() );
        nDeceleration = std::max( nDeceleration,
                                  xAnimationNode->getDecelerate() );
    }

    EventSharedPtr pEndEvent;
    if (pSelf) {
        pEndEvent = makeEvent(
            boost::bind( &AnimationNode::deactivate, pSelf ),
            "AnimationBaseNode::deactivate");
    }

    
    
    const sal_Int32 nMinFrameCount (basegfx::clamp<sal_Int32>(
        basegfx::fround(nDuration * FrameRate::MinimumFramesPerSecond), 1, 10));

    return ActivitiesFactory::CommonParameters(
        pEndEvent,
        getContext().mrEventQueue,
        getContext().mrActivitiesQueue,
        nDuration,
        nMinFrameCount,
        bAutoReverse,
        aRepeats,
        nAcceleration,
        nDeceleration,
        getShape(),
        getSlideSize());
}

AttributableShapeSharedPtr AnimationBaseNode::getShape() const
{
    
    if (mpShapeSubset)
        return mpShapeSubset->getSubsetShape();
    else
        return mpShape; 
}

} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
