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


#include <com/sun/star/animations/XAnimate.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/animations/AnimationFill.hpp>
#include <com/sun/star/animations/AnimationRestart.hpp>
#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <basenode.hxx>
#include <eventmultiplexer.hxx>
#include <basecontainernode.hxx>
#include <eventqueue.hxx>
#include <delayevent.hxx>
#include <tools.hxx>
#include "nodetools.hxx"
#include "generateevent.hxx"

#include <sal/log.hxx>

#include <vector>
#include <algorithm>
#include <iterator>

using namespace ::com::sun::star;

namespace slideshow {
namespace internal {

namespace {

typedef int StateTransitionTable[17];

// State transition tables
// =========================================================================

const int* getStateTransitionTable( sal_Int16 nRestartMode,
                                    sal_Int16 nFillMode )
{
    // TODO(F2): restart issues in below tables

    // transition table for restart=NEVER, fill=REMOVE
    static const StateTransitionTable stateTransitionTable_Never_Remove = {
        AnimationNode::INVALID,
        AnimationNode::RESOLVED|AnimationNode::ENDED,   // active successors for UNRESOLVED
        AnimationNode::ACTIVE|AnimationNode::ENDED,     // active successors for RESOLVED
        AnimationNode::INVALID,
        AnimationNode::ENDED,                           // active successors for ACTIVE: no freeze here
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,                         // active successors for FROZEN: this state is unreachable here
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::ENDED                            // active successors for ENDED: this state is a sink here (cannot restart)
    };

    // transition table for restart=WHEN_NOT_ACTIVE, fill=REMOVE
    static const StateTransitionTable stateTransitionTable_NotActive_Remove = {
        AnimationNode::INVALID,
        AnimationNode::RESOLVED|AnimationNode::ENDED,                       // active successors for UNRESOLVED
        AnimationNode::ACTIVE|AnimationNode::ENDED,                         // active successors for RESOLVED
        AnimationNode::INVALID,
        AnimationNode::ENDED,                                               // active successors for ACTIVE: no freeze here
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,                                             // active successors for FROZEN:
                                                                            // this state is unreachable here
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::ENDED|AnimationNode::RESOLVED|AnimationNode::ACTIVE  // active successors for ENDED:
                                                                            // restart possible when ended
    };

    // transition table for restart=ALWAYS, fill=REMOVE
    static const StateTransitionTable stateTransitionTable_Always_Remove = {
        AnimationNode::INVALID,
        AnimationNode::RESOLVED|AnimationNode::ENDED,                       // active successors for UNRESOLVED
        AnimationNode::ACTIVE|AnimationNode::ENDED,                         // active successors for RESOLVED
        AnimationNode::INVALID,
        AnimationNode::ENDED|AnimationNode::ACTIVE|AnimationNode::RESOLVED, // active successors for ACTIVE: restart
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,                                             // active successors for FROZEN:
                                                                            // this state is unreachable here
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::ENDED|AnimationNode::ACTIVE|AnimationNode::RESOLVED  // active successors for ENDED: restart
    };

    // transition table for restart=NEVER, fill=FREEZE
    static const StateTransitionTable stateTransitionTable_Never_Freeze = {
        AnimationNode::INVALID,
        AnimationNode::RESOLVED|AnimationNode::ENDED,   // active successors for UNRESOLVED
        AnimationNode::ACTIVE|AnimationNode::ENDED,     // active successors for RESOLVED
        AnimationNode::INVALID,
        AnimationNode::FROZEN|AnimationNode::ENDED,     // active successors for ACTIVE: freeze object
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::ENDED,                           // active successors for FROZEN: end
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::ENDED,                           // active successors for ENDED: this state is a sink here (cannot restart)
    };

    // transition table for restart=WHEN_NOT_ACTIVE, fill=FREEZE
    static const StateTransitionTable stateTransitionTable_NotActive_Freeze = {
        AnimationNode::INVALID,
        AnimationNode::RESOLVED|AnimationNode::ENDED,                       // active successors for UNRESOLVED
        AnimationNode::ACTIVE|AnimationNode::ENDED,                         // active successors for RESOLVED
        AnimationNode::INVALID,
        AnimationNode::FROZEN|AnimationNode::ENDED,                         // active successors for ACTIVE: freeze object
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::ENDED|AnimationNode::RESOLVED|AnimationNode::ACTIVE, // active successors for FROZEN:
                                                                            // restart possible when ended
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::ENDED|AnimationNode::RESOLVED|AnimationNode::ACTIVE  // active successors for ENDED:
                                                                            // restart possible when ended
    };

    // transition table for restart=ALWAYS, fill=FREEZE
    static const StateTransitionTable stateTransitionTable_Always_Freeze = {
        AnimationNode::INVALID,
        AnimationNode::RESOLVED|AnimationNode::ENDED,                       // active successors for UNRESOLVED
        AnimationNode::ACTIVE|AnimationNode::ENDED,                         // active successors for RESOLVED
        AnimationNode::INVALID,
        AnimationNode::FROZEN|AnimationNode::ENDED|AnimationNode::ACTIVE|AnimationNode::RESOLVED, // active successors for ACTIVE:
                                                                                                  // end object, restart
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::ENDED|AnimationNode::RESOLVED|AnimationNode::ACTIVE, // active successors for FROZEN: restart possible
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::INVALID,
        AnimationNode::ENDED|AnimationNode::ACTIVE|AnimationNode::RESOLVED  // active successors for ENDED: restart
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
        // same value: animations::AnimationRestart::INHERIT:
        OSL_FAIL(
            "getStateTransitionTable(): unexpected case for restart" );
        [[fallthrough]];
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
        // same value: animations::AnimationFill::INHERIT:
        OSL_FAIL(
            "getStateTransitionTable(): unexpected case for fill" );
        [[fallthrough]];
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

/// Little helper predicate, to detect main sequence root node
bool isMainSequenceRootNode_(
    const uno::Reference< animations::XAnimationNode >& xNode )
{
    // detect main sequence root node (need that for
    // end-of-mainsequence signalling below)
    beans::NamedValue const aSearchKey(
        "node-type",
        uno::makeAny( presentation::EffectNodeType::MAIN_SEQUENCE ) );

    uno::Sequence<beans::NamedValue> const userData(xNode->getUserData());
    return findNamedValue( userData, aSearchKey );
}

} // anon namespace

// BaseNode implementation
//=========================================================================

/** state transition handling
 */
class BaseNode::StateTransition
{
public:
    enum Options { NONE, FORCE };

    explicit StateTransition( BaseNode * pNode )
        : mpNode(pNode), meToState(INVALID) {}

    ~StateTransition() {
        clear();
    }

    StateTransition(const StateTransition&) = delete;
    StateTransition& operator=(const StateTransition&) = delete;

    bool enter( NodeState eToState, int options = NONE )
    {
        OSL_ENSURE( meToState == INVALID,
                    "### commit() before enter()ing again!" );
        if (meToState != INVALID)
            return false;
        bool const bForce = ((options & FORCE) != 0);
        if (!bForce && !mpNode->isTransition( mpNode->meCurrState, eToState ))
            return false;
        // recursion detection:
        if ((mpNode->meCurrentStateTransition & eToState) != 0)
            return false; // already in wanted transition
        // mark transition:
        mpNode->meCurrentStateTransition |= eToState;
        meToState = eToState;
        return true; // in transition
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
    mpStateTransitionTable( nullptr ),
    mnStartDelay( rContext.mnStartDelay ),
    meCurrState( UNRESOLVED ),
    meCurrentStateTransition( 0 ),
    mpCurrentEvent(),
    mbIsMainSequenceRootNode( isMainSequenceRootNode_( xNode ) )
{
    ENSURE_OR_THROW( mxAnimationNode.is(),
                      "BaseNode::BaseNode(): Invalid XAnimationNode" );

    // setup state transition table
    mpStateTransitionTable = getStateTransitionTable( getRestartMode(),
                                                      getFillMode() );
}

void BaseNode::dispose()
{
    meCurrState = INVALID;

    // discharge a loaded event, if any:
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
    return nTmp != animations::AnimationRestart::DEFAULT
        ? nTmp : getRestartDefaultMode();
}

sal_Int16 BaseNode::getFillMode()
{
    const sal_Int16 nTmp( mxAnimationNode->getFill() );
    const sal_Int16 nFill(nTmp != animations::AnimationFill::DEFAULT
                          ? nTmp : getFillDefaultMode());

    // For AUTO fill mode, SMIL specifies that fill mode is FREEZE,
    // if no explicit active duration is given
    // (no duration, end, repeatCount or repeatDuration given),
    // and REMOVE otherwise
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
        nFillDefault = (mpParent != nullptr
                        ? mpParent->getFillDefaultMode()
                        : animations::AnimationFill::AUTO);
    }
    return nFillDefault;
}

sal_Int16 BaseNode::getRestartDefaultMode() const
{
    sal_Int16 nRestartDefaultMode = mxAnimationNode->getRestartDefault();
    if (nRestartDefaultMode == animations::AnimationRestart::DEFAULT) {
        nRestartDefaultMode = (mpParent != nullptr
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
    // discharge a loaded event, if any:
    if (mpCurrentEvent) {
        mpCurrentEvent->dispose();
        mpCurrentEvent.reset();
    }
    return init_st(); // may call derived class
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
        st.commit(); // changing state

        // discharge a loaded event, if any:
        if (mpCurrentEvent)
            mpCurrentEvent->dispose();

        // schedule activation event:

        // This method takes the NodeContext::mnStartDelay value into account,
        // to cater for iterate container time shifts. We cannot put different
        // iterations of the iterate container's children into different
        // subcontainer (such as a 'DelayContainer', which delays resolving its
        // children by a fixed amount), since all iterations' nodes must be
        // resolved at the same time (otherwise, the delayed subset creation
        // will not work, i.e. deactivate the subsets too late in the master
        // shape).
        uno::Any const aBegin( mxAnimationNode->getBegin() );
        if (aBegin.hasValue()) {
            auto self(mpSelf);
            mpCurrentEvent = generateEvent(
                aBegin, [self] () { self->activate(); },
                maContext, mnStartDelay );
        }
        else {
            // For some leaf nodes, PPT import yields empty begin time,
            // although semantically, it should be 0.0
            // TODO(F3): That should really be provided by the PPT import

            // schedule delayed activation event. Take iterate node
            // timeout into account
            auto self(mpSelf);
            mpCurrentEvent = makeDelay(
                [self] () { self->activate(); },
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


void BaseNode::activate()
{
    if (! checkValidNode())
        return;

    OSL_ASSERT( meCurrState != ACTIVE );
    if (inStateOrTransition( ACTIVE ))
        return;

    StateTransition st(this);
    if (st.enter( ACTIVE )) {

        activate_st(); // calling derived class

        st.commit(); // changing state

        maContext.mrEventMultiplexer.notifyAnimationStart( mpSelf );
    }
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
        // This method need not take the
        // NodeContext::mnStartDelay value into account,
        // because the deactivation event is only scheduled
        // when the effect is started: the timeout is then
        // already respected.

        // xxx todo:
        // think about set node, anim base node!
        // if anim base node has no activity, this is called to schedule deactivation,
        // but what if it does not schedule anything?

        auto self(mpSelf);
        if (mxAnimationNode->getEnd().hasValue())
        {
            // TODO: We may need to calculate the duration if the end value is numeric.
            // We expect that the end value contains EventTrigger::ON_NEXT here.
            // LibreOffice does not generate numeric values, so we can leave it
            // until we find a test case.
            mpCurrentEvent = generateEvent(
                mxAnimationNode->getEnd(),
                [self] () { self->deactivate(); },
                maContext, 0.0 );

        }
        else
        {
            mpCurrentEvent = generateEvent(
                mxAnimationNode->getDuration(),
                [self] () { self->deactivate(); },
                maContext, 0.0 );
        }
    }
}

void BaseNode::deactivate()
{
    if (inStateOrTransition( ENDED | FROZEN ) || !checkValidNode())
        return;

    if (isTransition( meCurrState, FROZEN, false /* no OSL_ASSERT */ )) {
        // do transition to FROZEN:
        StateTransition st(this);
        if (st.enter( FROZEN, StateTransition::FORCE )) {

            deactivate_st( FROZEN );
            st.commit();

            notifyEndListeners();

            // discharge a loaded event, before going on:
            if (mpCurrentEvent) {
                mpCurrentEvent->dispose();
                mpCurrentEvent.reset();
            }
        }
    }
    else {
        // use end instead:
        end();
    }
    // state has changed either to FROZEN or ENDED
}

void BaseNode::deactivate_st( NodeState )
{
}

void BaseNode::end()
{
    bool const bIsFrozenOrInTransitionToFrozen = inStateOrTransition( FROZEN );
    if (inStateOrTransition( ENDED ) || !checkValidNode())
        return;

    // END must always be reachable. If not, that's an error in the
    // transition tables
    OSL_ENSURE( isTransition( meCurrState, ENDED ),
                "end state not reachable in transition table" );

    StateTransition st(this);
    if (!st.enter( ENDED, StateTransition::FORCE ))
        return;

    deactivate_st( ENDED );
    st.commit(); // changing state

    // if is FROZEN or is to be FROZEN, then
    // will/already notified deactivating listeners
    if (!bIsFrozenOrInTransitionToFrozen)
        notifyEndListeners();

    // discharge a loaded event, before going on:
    if (mpCurrentEvent) {
        mpCurrentEvent->dispose();
        mpCurrentEvent.reset();
    }
}

void BaseNode::notifyDeactivating( const AnimationNodeSharedPtr& rNotifier )
{
    OSL_ASSERT( rNotifier->getState() == FROZEN ||
                rNotifier->getState() == ENDED );
    // TODO(F1): for end sync functionality, this might indeed be used some day
}

void BaseNode::notifyEndListeners() const
{
    // notify all listeners
    for( const auto& rListner : maDeactivatingListeners )
        rListner->notifyDeactivating( mpSelf );

    // notify state change
    maContext.mrEventMultiplexer.notifyAnimationEnd( mpSelf );

    // notify main sequence end (iff we're the main
    // sequence root node). This is because the main
    // sequence determines the active duration of the
    // slide. All other sequences are secondary, in that
    // they don't prevent a slide change from happening,
    // even if they have not been completed. In other
    // words, all sequences except the main sequence are
    // optional for the slide lifetime.
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

// Debug


#if defined(DBG_UTIL)
void BaseNode::showState() const
{
    const AnimationNode::NodeState eNodeState( getState() );

    if( eNodeState == AnimationNode::INVALID )
        SAL_INFO("slideshow.verbose", "Node state: n" <<
                 debugGetNodeName(this) <<
                 " [label=\"" <<
                 getDescription() <<
                 "\",style=filled, fillcolor=\"0.5,0.2,0.5\"]");
    else
        SAL_INFO("slideshow.verbose", "Node state: n" <<
                 debugGetNodeName(this) <<
                 " [label=\"" <<
                 getDescription() <<
                 "fillcolor=\"" <<
                 log(double(getState()))/4.0 <<
                 ",1.0,1.0\"]");

    // determine additional node information
    uno::Reference<animations::XAnimate> const xAnimate( mxAnimationNode,
                                                         uno::UNO_QUERY );
    if( !xAnimate.is() )
        return;

    uno::Reference< drawing::XShape > xTargetShape( xAnimate->getTarget(),
                                                    uno::UNO_QUERY );

    if( !xTargetShape.is() )
    {
        css::presentation::ParagraphTarget aTarget;

        // no shape provided. Maybe a ParagraphTarget?
        if( xAnimate->getTarget() >>= aTarget )
            xTargetShape = aTarget.Shape;
    }

    if( !xTargetShape.is() )
        return;

    uno::Reference< beans::XPropertySet > xPropSet( xTargetShape,
                                                    uno::UNO_QUERY );

    // read shape name
    OUString aName;
    if( xPropSet->getPropertyValue("Name") >>= aName )
    {
        SAL_INFO("slideshow.verbose", "Node info: n" <<
                 debugGetNodeName(this) <<
                 ", name \"" <<
                 aName <<
                 "\"");
    }
}

const char* BaseNode::getDescription() const
{
    return "BaseNode";
}

#endif

} // namespace internal
} // namespace slideshow

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
