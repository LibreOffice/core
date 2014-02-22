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

#include <com/sun/star/animations/XAnimate.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/animations/AnimationFill.hpp>
#include <com/sun/star/animations/AnimationRestart.hpp>
#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include "basenode.hxx"
#include "eventmultiplexer.hxx"
#include "basecontainernode.hxx"
#include "eventqueue.hxx"
#include "delayevent.hxx"
#include "tools.hxx"
#include "nodetools.hxx"
#include "generateevent.hxx"

#include <boost/bind.hpp>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace ::com::sun::star;

namespace slideshow {
namespace internal {

namespace {

typedef int StateTransitionTable[17];




const int* getStateTransitionTable( sal_Int16 nRestartMode,
                                    sal_Int16 nFillMode )
{
    

    
    static const StateTransitionTable stateTransitionTable_Never_Remove = {
        AnimationNode::INVALID,
        AnimationNode::RESOLVED|AnimationNode::ENDED,   
        AnimationNode::ACTIVE|AnimationNode::ENDED,     
        AnimationNode::INVALID,
        AnimationNode::ENDED,                           
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,                         
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::ENDED                            
    };

    
    static const StateTransitionTable stateTransitionTable_NotActive_Remove = {
        AnimationNode::INVALID,
        AnimationNode::RESOLVED|AnimationNode::ENDED,                       
        AnimationNode::ACTIVE|AnimationNode::ENDED,                         
        AnimationNode::INVALID,
        AnimationNode::ENDED,                                               
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,                                             
                                                                            
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::ENDED|AnimationNode::RESOLVED|AnimationNode::ACTIVE  
                                                                            
    };

    
    static const StateTransitionTable stateTransitionTable_Always_Remove = {
        AnimationNode::INVALID,
        AnimationNode::RESOLVED|AnimationNode::ENDED,                       
        AnimationNode::ACTIVE|AnimationNode::ENDED,                         
        AnimationNode::INVALID,
        AnimationNode::ENDED|AnimationNode::ACTIVE|AnimationNode::RESOLVED, 
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,                                             
                                                                            
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::ENDED|AnimationNode::ACTIVE|AnimationNode::RESOLVED  
    };

    
    static const StateTransitionTable stateTransitionTable_Never_Freeze = {
        AnimationNode::INVALID,
        AnimationNode::RESOLVED|AnimationNode::ENDED,   
        AnimationNode::ACTIVE|AnimationNode::ENDED,     
        AnimationNode::INVALID,
        AnimationNode::FROZEN|AnimationNode::ENDED,     
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::ENDED,                           
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::ENDED,                           
    };

    
    static const StateTransitionTable stateTransitionTable_NotActive_Freeze = {
        AnimationNode::INVALID,
        AnimationNode::RESOLVED|AnimationNode::ENDED,                       
        AnimationNode::ACTIVE|AnimationNode::ENDED,                         
        AnimationNode::INVALID,
        AnimationNode::FROZEN|AnimationNode::ENDED,                         
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::ENDED|AnimationNode::RESOLVED|AnimationNode::ACTIVE, 
                                                                            
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::ENDED|AnimationNode::RESOLVED|AnimationNode::ACTIVE  
                                                                            
    };

    
    static const StateTransitionTable stateTransitionTable_Always_Freeze = {
        AnimationNode::INVALID,
        AnimationNode::RESOLVED|AnimationNode::ENDED,                       
        AnimationNode::ACTIVE|AnimationNode::ENDED,                         
        AnimationNode::INVALID,
        AnimationNode::FROZEN|AnimationNode::ENDED|AnimationNode::ACTIVE|AnimationNode::RESOLVED, 
                                                                                                  
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::ENDED|AnimationNode::RESOLVED|AnimationNode::ACTIVE, 
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::ENDED|AnimationNode::ACTIVE|AnimationNode::RESOLVED  
    };

    static const StateTransitionTable* tableGuide[] = {
        &stateTransitionTable_Never_Remove,
        &stateTransitionTable_NotActive_Remove,
        &stateTransitionTable_Always_Remove,
        &stateTransitionTable_Never_Freeze,
        &stateTransitionTable_NotActive_Freeze,
        &stateTransitionTable_Always_Freeze
    };

    int nRestartValue;
    switch( nRestartMode ) {
    default:
    case animations::AnimationRestart::DEFAULT:
        
        OSL_FAIL(
            "getStateTransitionTable(): unexpected case for restart" );
        
    case animations::AnimationRestart::NEVER:
        nRestartValue = 0;
        break;
    case animations::AnimationRestart::WHEN_NOT_ACTIVE:
        nRestartValue = 1;
        break;
    case animations::AnimationRestart::ALWAYS:
        nRestartValue = 2;
        break;
    }

    int nFillValue;
    switch( nFillMode ) {
    default:
    case animations::AnimationFill::AUTO:
    case animations::AnimationFill::DEFAULT:
        
        OSL_FAIL(
            "getStateTransitionTable(): unexpected case for fill" );
        
    case animations::AnimationFill::REMOVE:
        nFillValue = 0;
        break;
    case animations::AnimationFill::FREEZE:
    case animations::AnimationFill::HOLD:
    case animations::AnimationFill::TRANSITION:
        nFillValue = 1;
        break;
    }

    return *tableGuide[ 3*nFillValue + nRestartValue ];
}


bool isMainSequenceRootNode_(
    const uno::Reference< animations::XAnimationNode >& xNode )
{
    
    
    beans::NamedValue const aSearchKey(
        OUString( "node-type" ),
        uno::makeAny( presentation::EffectNodeType::MAIN_SEQUENCE ) );

    uno::Sequence<beans::NamedValue> const userData(xNode->getUserData());
    return findNamedValue( userData, aSearchKey );
}

} 




/** state transition handling
 */
class BaseNode::StateTransition : private boost::noncopyable
{
public:
    enum Options { NONE, FORCE };

    explicit StateTransition( BaseNode * pNode )
        : mpNode(pNode), meToState(INVALID) {}

    ~StateTransition() {
        clear();
    }

    bool enter( NodeState eToState, int options = NONE )
    {
        OSL_ENSURE( meToState == INVALID,
                    "### commit() before enter()ing again!" );
        if (meToState != INVALID)
            return false;
        bool const bForce = ((options & FORCE) != 0);
        if (!bForce && !mpNode->isTransition( mpNode->meCurrState, eToState ))
            return false;
        
        if ((mpNode->meCurrentStateTransition & eToState) != 0)
            return false; 
        
        mpNode->meCurrentStateTransition |= eToState;
        meToState = eToState;
        return true; 
    }

    void commit() {
        OSL_ENSURE( meToState != INVALID, "### nothing to commit!" );
        if (meToState != INVALID) {
            mpNode->meCurrState = meToState;
            clear();
        }
    }

    void clear() {
        if (meToState != INVALID) {
            OSL_ASSERT( (mpNode->meCurrentStateTransition & meToState) != 0 );
            mpNode->meCurrentStateTransition &= ~meToState;
            meToState = INVALID;
        }
    }

private:
    BaseNode *const mpNode;
    NodeState meToState;
};

BaseNode::BaseNode( const uno::Reference< animations::XAnimationNode >& xNode,
                    const BaseContainerNodeSharedPtr&                   rParent,
                    const NodeContext&                                  rContext ) :
    maContext( rContext.maContext ),
    maDeactivatingListeners(),
    mxAnimationNode( xNode ),
    mpParent( rParent ),
    mpSelf(),
    mpStateTransitionTable( NULL ),
    mnStartDelay( rContext.mnStartDelay ),
    meCurrState( UNRESOLVED ),
    meCurrentStateTransition( 0 ),
    mpCurrentEvent(),
    mbIsMainSequenceRootNode( isMainSequenceRootNode_( xNode ) )
{
    ENSURE_OR_THROW( mxAnimationNode.is(),
                      "BaseNode::BaseNode(): Invalid XAnimationNode" );

    
    mpStateTransitionTable = getStateTransitionTable( getRestartMode(),
                                                      getFillMode() );
}

void BaseNode::dispose()
{
    meCurrState = INVALID;

    
    if (mpCurrentEvent) {
        mpCurrentEvent->dispose();
        mpCurrentEvent.reset();
    }
    maDeactivatingListeners.clear();
    mxAnimationNode.clear();
    mpParent.reset();
    mpSelf.reset();
    maContext.dispose();
}


sal_Int16 BaseNode::getRestartMode()
{
    const sal_Int16 nTmp( mxAnimationNode->getRestart() );
    return (nTmp != animations::AnimationRestart::DEFAULT &&
            nTmp != animations::AnimationRestart::INHERIT)
        ? nTmp : getRestartDefaultMode();
}

sal_Int16 BaseNode::getFillMode()
{
    const sal_Int16 nTmp( mxAnimationNode->getFill() );
    const sal_Int16 nFill((nTmp != animations::AnimationFill::DEFAULT &&
                           nTmp != animations::AnimationFill::INHERIT)
                          ? nTmp : getFillDefaultMode());

    
    
    
    
    if( nFill == animations::AnimationFill::AUTO ) {
        return (isIndefiniteTiming( mxAnimationNode->getDuration() ) &&
                isIndefiniteTiming( mxAnimationNode->getEnd() ) &&
                !mxAnimationNode->getRepeatCount().hasValue() &&
                isIndefiniteTiming( mxAnimationNode->getRepeatDuration() ))
            ? animations::AnimationFill::FREEZE
            : animations::AnimationFill::REMOVE;
    }
    else {
        return nFill;
    }
}

sal_Int16 BaseNode::getFillDefaultMode() const
{
    sal_Int16 nFillDefault = mxAnimationNode->getFillDefault();
    if (nFillDefault  == animations::AnimationFill::DEFAULT) {
        nFillDefault = (mpParent != 0
                        ? mpParent->getFillDefaultMode()
                        : animations::AnimationFill::AUTO);
    }
    return nFillDefault;
}

sal_Int16 BaseNode::getRestartDefaultMode() const
{
    sal_Int16 nRestartDefaultMode = mxAnimationNode->getRestartDefault();
    if (nRestartDefaultMode == animations::AnimationRestart::DEFAULT) {
        nRestartDefaultMode = (mpParent != 0
                               ? mpParent->getRestartDefaultMode()
                               : animations::AnimationRestart::ALWAYS);
    }
    return nRestartDefaultMode;
}

uno::Reference<animations::XAnimationNode> BaseNode::getXAnimationNode() const
{
    return mxAnimationNode;
}

bool BaseNode::init()
{
    if (! checkValidNode())
        return false;
    meCurrState = UNRESOLVED;
    
    if (mpCurrentEvent) {
        mpCurrentEvent->dispose();
        mpCurrentEvent.reset();
    }
    return init_st(); 
}

bool BaseNode::init_st()
{
    return true;
}

bool BaseNode::resolve()
{
    if (! checkValidNode())
        return false;

    OSL_ASSERT( meCurrState != RESOLVED );
    if (inStateOrTransition( RESOLVED ))
        return true;

    StateTransition st(this);
    if (st.enter( RESOLVED ) &&
        isTransition( RESOLVED, ACTIVE ) &&
        resolve_st() /* may call derived class */)
    {
        st.commit(); 

        
        if (mpCurrentEvent)
            mpCurrentEvent->dispose();

        

        
        
        
        
        
        
        
        
        uno::Any const aBegin( mxAnimationNode->getBegin() );
        if (aBegin.hasValue()) {
            mpCurrentEvent = generateEvent(
                aBegin, boost::bind( &AnimationNode::activate, mpSelf ),
                maContext, mnStartDelay );
        }
        else {
            
            
            

            
            
            mpCurrentEvent = makeDelay(
                boost::bind( &AnimationNode::activate, mpSelf ),
                mnStartDelay,
                "AnimationNode::activate with delay");
            maContext.mrEventQueue.addEvent( mpCurrentEvent );
        }

        return true;
    }
    return false;
}

bool BaseNode::resolve_st()
{
    return true;
}


bool BaseNode::activate()
{
    if (! checkValidNode())
        return false;

    OSL_ASSERT( meCurrState != ACTIVE );
    if (inStateOrTransition( ACTIVE ))
        return true;

    StateTransition st(this);
    if (st.enter( ACTIVE )) {

        activate_st(); 

        st.commit(); 

        maContext.mrEventMultiplexer.notifyAnimationStart( mpSelf );

        return true;
    }

    return false;
}

void BaseNode::activate_st()
{
    scheduleDeactivationEvent();
}

void BaseNode::scheduleDeactivationEvent( EventSharedPtr const& pEvent )
{
    if (mpCurrentEvent) {
        mpCurrentEvent->dispose();
        mpCurrentEvent.reset();
    }
    if (pEvent) {
        if (maContext.mrEventQueue.addEvent( pEvent ))
            mpCurrentEvent = pEvent;
    }
    else {
        
        
        
        
        

        
        
        
        

        
        mpCurrentEvent = generateEvent(
            mxAnimationNode->getDuration(),
            boost::bind( &AnimationNode::deactivate, mpSelf ),
            maContext, 0.0 );
    }
}

void BaseNode::deactivate()
{
    if (inStateOrTransition( ENDED | FROZEN ) || !checkValidNode())
        return;

    if (isTransition( meCurrState, FROZEN, false /* no OSL_ASSERT */ )) {
        
        StateTransition st(this);
        if (st.enter( FROZEN, StateTransition::FORCE )) {

            deactivate_st( FROZEN );
            st.commit();

            notifyEndListeners();

            
            if (mpCurrentEvent) {
                mpCurrentEvent->dispose();
                mpCurrentEvent.reset();
            }
        }
    }
    else {
        
        end();
    }
    
}

void BaseNode::deactivate_st( NodeState )
{
}

void BaseNode::end()
{
    bool const bIsFrozenOrInTransitionToFrozen = inStateOrTransition( FROZEN );
    if (inStateOrTransition( ENDED ) || !checkValidNode())
        return;

    
    
    OSL_ENSURE( isTransition( meCurrState, ENDED ),
                "end state not reachable in transition table" );

    StateTransition st(this);
    if (st.enter( ENDED, StateTransition::FORCE )) {

        deactivate_st( ENDED );
        st.commit(); 

        
        
        if (!bIsFrozenOrInTransitionToFrozen)
            notifyEndListeners();

        
        if (mpCurrentEvent) {
            mpCurrentEvent->dispose();
            mpCurrentEvent.reset();
        }
    }
}

void BaseNode::notifyDeactivating( const AnimationNodeSharedPtr& rNotifier )
{
    (void) rNotifier; 
    OSL_ASSERT( rNotifier->getState() == FROZEN ||
                rNotifier->getState() == ENDED );
    
}

void BaseNode::notifyEndListeners() const
{
    
    std::for_each( maDeactivatingListeners.begin(),
                   maDeactivatingListeners.end(),
                   boost::bind( &AnimationNode::notifyDeactivating, _1,
                                boost::cref(mpSelf) ) );

    
    maContext.mrEventMultiplexer.notifyAnimationEnd( mpSelf );

    
    
    
    
    
    
    
    
    if (isMainSequenceRootNode())
        maContext.mrEventMultiplexer.notifySlideAnimationsEnd();
}

AnimationNode::NodeState BaseNode::getState() const
{
    return meCurrState;
}

bool BaseNode::registerDeactivatingListener(
    const AnimationNodeSharedPtr& rNotifee )
{
    if (! checkValidNode())
        return false;

    ENSURE_OR_RETURN_FALSE(
        rNotifee,
        "BaseNode::registerDeactivatingListener(): invalid notifee" );
    maDeactivatingListeners.push_back( rNotifee );

    return true;
}

void BaseNode::setSelf( const BaseNodeSharedPtr& rSelf )
{
    ENSURE_OR_THROW( rSelf.get() == this,
                      "BaseNode::setSelf(): got ptr to different object" );
    ENSURE_OR_THROW( !mpSelf,
                      "BaseNode::setSelf(): called multiple times" );

    mpSelf = rSelf;
}




#if OSL_DEBUG_LEVEL >= 2 && defined(DBG_UTIL)
void BaseNode::showState() const
{
    const AnimationNode::NodeState eNodeState( getState() );

    if( eNodeState == AnimationNode::INVALID )
        VERBOSE_TRACE( "Node state: n0x%X [label=\"%s\",style=filled,"
                       "fillcolor=\"0.5,0.2,0.5\"]",
                       (const char*)this+debugGetCurrentOffset(),
                       getDescription() );
    else
        VERBOSE_TRACE( "Node state: n0x%X [label=\"%s\",style=filled,"
                       "fillcolor=\"%f,1.0,1.0\"]",
                       (const char*)this+debugGetCurrentOffset(),
                       getDescription(),
                       log(double(getState()))/4.0 );

    
    uno::Reference<animations::XAnimate> const xAnimate( mxAnimationNode,
                                                         uno::UNO_QUERY );
    if( xAnimate.is() )
    {
        uno::Reference< drawing::XShape > xTargetShape( xAnimate->getTarget(),
                                                        uno::UNO_QUERY );

        if( !xTargetShape.is() )
        {
            ::com::sun::star::presentation::ParagraphTarget aTarget;

            
            if( (xAnimate->getTarget() >>= aTarget) )
                xTargetShape = aTarget.Shape;
        }

        if( xTargetShape.is() )
        {
            uno::Reference< beans::XPropertySet > xPropSet( xTargetShape,
                                                            uno::UNO_QUERY );

            
            OUString aName;
            if( (xPropSet->getPropertyValue(
                     OUString("Name") )
                 >>= aName) )
            {
                const OString& rAsciiName(
                    OUStringToOString( aName,
                                              RTL_TEXTENCODING_ASCII_US ) );

                VERBOSE_TRACE( "Node info: n0x%X, name \"%s\"",
                               (const char*)this+debugGetCurrentOffset(),
                               rAsciiName.getStr() );
            }
        }
    }
}

const char* BaseNode::getDescription() const
{
    return "BaseNode";
}

#endif

} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
