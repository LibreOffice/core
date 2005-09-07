/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basenode.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:43:57 $
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

#include <basenode.hxx>
#include <basecontainernode.hxx>
#include <delayevent.hxx>
#include <tools.hxx>
#include <nodetools.hxx>
#include <generateevent.hxx>

#ifndef _COM_SUN_STAR_ANIMATIONS_XANIMATE_HPP_
#include <com/sun/star/animations/XAnimate.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_PARAGRAPHTARGET_HPP_
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_ANIMATIONNODETYPE_HPP_
#include <com/sun/star/animations/AnimationNodeType.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_ANIMATIONCALCMODE_HPP_
#include <com/sun/star/animations/AnimationCalcMode.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_TIMING_HPP_
#include <com/sun/star/animations/Timing.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_EVENTTRIGGER_HPP_
#include <com/sun/star/animations/EventTrigger.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_ANIMATIONFILL_HPP_
#include <com/sun/star/animations/AnimationFill.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_EVENT_HPP_
#include <com/sun/star/animations/Event.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_ANIMATIONRESTART_HPP_
#include <com/sun/star/animations/AnimationRestart.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_SHAPEANIMATIONSUBTYPE_HPP_
#include <com/sun/star/presentation/ShapeAnimationSubType.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_EFFECTNODETYPE_HPP_
#include <com/sun/star/presentation/EffectNodeType.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_ANIMATIONADDITIVEMODE_HPP_
#include <com/sun/star/animations/AnimationAdditiveMode.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef BOOST_BIND_HPP_INCLUDED
#include <boost/bind.hpp>
#endif
#ifndef BOOST_MEM_FN_HPP_INCLUDED
#include <boost/mem_fn.hpp>
#endif

#include <vector>
#include <algorithm>
#include <iterator>


using namespace ::com::sun::star;

namespace presentation
{
    namespace internal
    {
        namespace
        {
            // State transition tables
            // =========================================================================

            const int* getStateTransitionTable( sal_Int16 nRestartMode,
                                                sal_Int16 nFillMode )
            {
                // transition table for restart=NEVER, fill=REMOVE
                static const StateTransitionTable stateTransitionTable_Never_Remove =
                    {
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
                static const StateTransitionTable stateTransitionTable_NotActive_Remove =
                    {
                        AnimationNode::INVALID,
                        AnimationNode::RESOLVED|AnimationNode::ENDED,                       // active successors for UNRESOLVED
                        AnimationNode::ACTIVE|AnimationNode::ENDED,                         // active successors for RESOLVED
                        AnimationNode::INVALID,
                        AnimationNode::ENDED,                                               // active successors for ACTIVE: no freeze here
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,                                             // active successors for FROZEN: this state is unreachable here
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,
                        AnimationNode::ENDED|AnimationNode::RESOLVED|AnimationNode::ACTIVE  // active successors for ENDED: restart possible when ended
                    };

                // transition table for restart=ALWAYS, fill=REMOVE
                static const StateTransitionTable stateTransitionTable_Always_Remove =
                    {
                        AnimationNode::INVALID,
                        AnimationNode::RESOLVED|AnimationNode::ENDED,                       // active successors for UNRESOLVED
                        AnimationNode::ACTIVE|AnimationNode::ENDED,                         // active successors for RESOLVED
                        AnimationNode::INVALID,
                        AnimationNode::ENDED|AnimationNode::ACTIVE|AnimationNode::RESOLVED, // active successors for ACTIVE: restart
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,                                             // active successors for FROZEN: this state is unreachable here
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
                static const StateTransitionTable stateTransitionTable_Never_Freeze =
                    {
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
                static const StateTransitionTable stateTransitionTable_NotActive_Freeze =
                    {
                        AnimationNode::INVALID,
                        AnimationNode::RESOLVED|AnimationNode::ENDED,                       // active successors for UNRESOLVED
                        AnimationNode::ACTIVE|AnimationNode::ENDED,                         // active successors for RESOLVED
                        AnimationNode::INVALID,
                        AnimationNode::FROZEN|AnimationNode::ENDED,                         // active successors for ACTIVE: freeze object
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,
                        AnimationNode::ENDED|AnimationNode::RESOLVED|AnimationNode::ACTIVE, // active successors for FROZEN: restart possible when ended
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,
                        AnimationNode::ENDED|AnimationNode::RESOLVED|AnimationNode::ACTIVE  // active successors for ENDED: restart possible when ended
                    };

                // transition table for restart=ALWAYS, fill=FREEZE
                static const StateTransitionTable stateTransitionTable_Always_Freeze =
                    {
                        AnimationNode::INVALID,
                        AnimationNode::RESOLVED|AnimationNode::ENDED,                                               // active successors for UNRESOLVED
                        AnimationNode::ACTIVE|AnimationNode::ENDED,                                                 // active successors for RESOLVED
                        AnimationNode::INVALID,
                        AnimationNode::FROZEN|AnimationNode::ENDED|AnimationNode::ACTIVE|AnimationNode::RESOLVED,   // active successors for ACTIVE: end object, restart
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,
                        AnimationNode::ENDED|AnimationNode::RESOLVED|AnimationNode::ACTIVE,                         // active successors for FROZEN: restart possible
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,
                        AnimationNode::INVALID,
                        AnimationNode::ENDED|AnimationNode::ACTIVE|AnimationNode::RESOLVED  // active successors for ENDED: restart
                    };

                static const StateTransitionTable* tableGuide[] =
                    {
                        &stateTransitionTable_Never_Remove,
                        &stateTransitionTable_NotActive_Remove,
                        &stateTransitionTable_Always_Remove,
                        &stateTransitionTable_Never_Freeze,
                        &stateTransitionTable_NotActive_Freeze,
                        &stateTransitionTable_Always_Freeze
                    };

                int nRestartValue;
                switch( nRestartMode )
                {
                    default:
                    case animations::AnimationRestart::DEFAULT:
                    // same value: animations::AnimationRestart::INHERIT:
                        OSL_ENSURE( false, "getStateTransitionTable(): unexpected case for restart" );
                        // FALLTHROUGH intended
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
                switch( nFillMode )
                {
                    default:
                    case animations::AnimationFill::AUTO:
                    case animations::AnimationFill::DEFAULT:
                    // same value: animations::AnimationFill::INHERIT:
                        OSL_ENSURE( false, "getStateTransitionTable(): unexpected case for fill" );
                        // FALLTHROUGH intended
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
            bool isMainSequenceRootNode( const uno::Reference< animations::XAnimationNode >& xNode )
            {
                // detect main sequence root node (need that for
                // end-of-mainsequence signalling below)
                beans::NamedValue aSearchKey(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "node-type" ) ),
                    uno::makeAny( ::com::sun::star::presentation::EffectNodeType::MAIN_SEQUENCE ) );

                uno::Sequence<beans::NamedValue> userData(xNode->getUserData());
                return findNamedValue( userData,
                                       aSearchKey );
            }
        }

        // BaseNode implementation
        //=========================================================================

        BaseNode::BaseNode( const uno::Reference< animations::XAnimationNode >& xNode,
                            const BaseContainerNodeSharedPtr&                   rParent,
                            const NodeContext&                                  rContext ) :
            maContext( rContext.maContext ),
            maDeactivatingListeners(),
            mxNode( xNode ),
            mpParent( rParent ),
            mpSelf(),
            mpStateTransitionTable( NULL ),
            mnStartDelay( rContext.mnStartDelay ),
            meCurrState( UNRESOLVED ),
            mbIsMainSequenceRootNode( isMainSequenceRootNode( xNode ) )
        {
            ENSURE_AND_THROW( mxNode.is(),
                                "BaseNode::BaseNode(): Invalid XAnimationNode" );

            // setup state transition table
            mpStateTransitionTable = getStateTransitionTable( getRestartMode(),
                                                              getFillMode() );
        }

        void BaseNode::dispose()
        {
            meCurrState = INVALID;

            maDeactivatingListeners.clear();
            mxNode.clear();
            mpParent.reset();
            mpSelf.reset();
            maContext.dispose();
        }

        uno::Reference< animations::XAnimationNode > BaseNode::getXAnimationNode() const
        {
            return mxNode;
        }

        bool BaseNode::init()
        {
            // early exit on invalid nodes
            if( meCurrState == INVALID )
                return false;

            ENSURE_AND_THROW( mpSelf.get(),
                                "BaseNode::init(): no self set" );

            meCurrState = UNRESOLVED;

            return true;
        }

        bool BaseNode::resolve()
        {
            if( meCurrState == RESOLVED )
                return true; // avoid duplicate event generation

            // is this state reachable from meCurrState?
            if( !(mpStateTransitionTable[meCurrState] & RESOLVED) )
                return false; // nope, cannot resolv

            ENSURE_AND_THROW( mpSelf.get(),
                                "BaseNode::resolve(): no self set" );

            // change state
            meCurrState = RESOLVED;

            // schedule begin event (if ACTIVE state is reachable)
            if( !(mpStateTransitionTable[meCurrState] & ACTIVE) )
                return false; // nope, cannot become active

            scheduleActivationEvent();

            return true;
        }

        bool BaseNode::activate()
        {
            if( meCurrState == ACTIVE )
                return true; // avoid duplicate event generation

            // is the requested state reachable from meCurrState?
            if( !(mpStateTransitionTable[meCurrState] & ACTIVE) )
                return false; // nope, cannot become active

            // change state
            meCurrState = ACTIVE;

            // notify state change
            maContext.mrEventMultiplexer.notifyAnimationStart( mpSelf );

            scheduleDeactivationEvent();

            return true;
        }

        void BaseNode::deactivate()
        {
            // early exit on invalid nodes
            if( meCurrState == INVALID )
                return;

            // is state FROZEN reachable from meCurrState?
            if( !(mpStateTransitionTable[meCurrState] & FROZEN) )
                end(); // nope, use end instead
            else
                meCurrState = FROZEN; // change state

            // notify all listeners
            ::std::for_each( maDeactivatingListeners.begin(),
                                maDeactivatingListeners.end(),
                                ::boost::bind(
                                    &AnimationNode::notifyDeactivating,
                                    _1,
                                    ::boost::cref(mpSelf) ) );

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
            if( mbIsMainSequenceRootNode )
                maContext.mrEventMultiplexer.notifySlideAnimationsEnd();
        }

        void BaseNode::end()
        {
            // early exit on invalid nodes
            if( meCurrState == INVALID )
                return;

            // END must always be reachable. If not, that's an error in the
            // transition tables
            OSL_ENSURE( mpStateTransitionTable[meCurrState] & ENDED,
                        "BaseNode::end(): end state not reachable in transition table" );

            // change state
            meCurrState = ENDED;
        }

        AnimationNode::NodeState BaseNode::getState() const
        {
            return meCurrState;
        }

        bool BaseNode::registerDeactivatingListener( const AnimationNodeSharedPtr& rNotifee )
        {
            // early exit on invalid nodes
            if( meCurrState == INVALID )
                return false;

            ENSURE_AND_RETURN( rNotifee.get(),
                                "BaseNode::registerDeactivatingListener(): invalid notifee" );
            maDeactivatingListeners.push_back( rNotifee );

            return true;
        }

        void BaseNode::setSelf( const BaseNodeSharedPtr& rSelf )
        {
            // early exit on invalid nodes
            if( meCurrState == INVALID )
                return;

            ENSURE_AND_THROW( rSelf.get() == this,
                                "BaseNode::setSelf(): got ptr to different object" );
            ENSURE_AND_THROW( !mpSelf.get(),
                                "BaseNode::setSelf(): called multiple times" );

            mpSelf = rSelf;
        }

        sal_Int16 BaseNode::getFillDefaultMode() const
        {
            // early exit on invalid nodes
            if( meCurrState == INVALID )
                return 0;

            if( mxNode->getFillDefault() == animations::AnimationFill::DEFAULT )
            {
                return mpParent.get() ? mpParent->getFillDefaultMode() : animations::AnimationFill::AUTO;
            }

            return mxNode->getFillDefault();
        }

        sal_Int16 BaseNode::getRestartDefaultMode() const
        {
            // early exit on invalid nodes
            if( meCurrState == INVALID )
                return 0;

            if( mxNode->getRestartDefault() == animations::AnimationRestart::DEFAULT )
            {
                return mpParent.get() ? mpParent->getRestartDefaultMode() : animations::AnimationRestart::ALWAYS;
            }

            return mxNode->getRestartDefault();
        }

        void BaseNode::notifyUserEvent()
        {
            // early exit on invalid nodes
            if( meCurrState == INVALID )
                return;

            // TODO(F2): Check whether this here goes conform with SMILs
            // user event and restart specifications.

            // we're called from e.g. OnClick, try to become active
            if( !activate() )
            {
                // could not start, call parent to activate us
                if( mpParent.get() )
                    mpParent->requestResolveOnChildren();
            }

            // DEBUG_NODES_SHOWTREE_WITHIN(this);
        }

        void BaseNode::scheduleActivationEvent()
        {
            // This method takes the NodeContext::mnStartDelay value into account,
            // to cater for iterate container time shifts. We cannot put different
            // iterations of the iterate container's children into different
            // subcontainer (such as a 'DelayContainer', which delays resolving its
            // children by a fixed amount), since all iterations' nodes must be
            // resolved at the same time (otherwise, the delayed subset creation
            // will not work, i.e. deactivate the subsets too late in the master
            // shape).

            // For some leaf nodes, PPT import yields empty begin time,
            // although semantically, it should be 0.0
            if( !mxNode->getBegin().hasValue() )
            {
                // TODO(F3): That should really be provided by the PPT import

                // schedule delayed activation event. Take iterate node
                // timeout into account
                maContext.mrEventQueue.addEvent(
                    makeDelay( boost::bind( &BaseNode::activate, mpSelf ),
                               mnStartDelay ) );
            }
            else
            {
                generateEvent( mxNode->getBegin(),
                               boost::bind( &BaseNode::activate, mpSelf ),
                               maContext, mnStartDelay );
            }
        }

        void BaseNode::scheduleDeactivationEvent() const
        {
            // This method need not take the
            // NodeContext::mnStartDelay value into account,
            // because the deactivation event is only scheduled
            // when the effect is started: the timeout is then
            // already respected.

            // TODO(F2): Handle end time attribute, too
            generateEvent( mxNode->getDuration(),
                           boost::bind( &BaseNode::deactivate, mpSelf ),
                           maContext, 0.0 );
        }

        // Helper
        // ------

        sal_Int16 BaseNode::getRestartMode()
        {
            // early exit on invalid nodes
            if( meCurrState == INVALID )
                return 0;

            const sal_Int16 nTmp( mxNode->getRestart() );
            return (nTmp != animations::AnimationRestart::DEFAULT &&
                    nTmp != animations::AnimationRestart::INHERIT) ? nTmp : getRestartDefaultMode();
        }

        sal_Int16 BaseNode::getFillMode()
        {
            // early exit on invalid nodes
            if( meCurrState == INVALID )
                return 0;

            const sal_Int16 nTmp( mxNode->getFill() );
            const sal_Int16 nFill((nTmp != animations::AnimationFill::DEFAULT &&
                                    nTmp != animations::AnimationFill::INHERIT) ? nTmp : getFillDefaultMode());

            // For AUTO fill mode, SMIL specifies that fill mode is FREEZE, if no explicit
            // active duration is given (no duration, end, repeatCound or repeatDuration given),
            // and REMOVE otherwise
            if( nFill == animations::AnimationFill::AUTO )
            {
                return (isIndefiniteTiming( mxNode->getDuration() ) &&
                        isIndefiniteTiming( mxNode->getEnd() ) &&
                        !mxNode->getRepeatCount().hasValue() &&
                        isIndefiniteTiming( mxNode->getRepeatDuration() )) ?
                    animations::AnimationFill::FREEZE : animations::AnimationFill::REMOVE;}
            else
            {
                return nFill;
            }
        }


        // Debug
        //=========================================================================

#if defined(VERBOSE) && defined(DBG_UTIL)
        void BaseNode::showState() const
        {
            const AnimationNode::NodeState eNodeState( getState() );

            if( eNodeState == AnimationNode::INVALID )
                VERBOSE_TRACE( "Node state: n0x%X [label=\"%s\",style=filled,fillcolor=\"0.5,0.2,0.5\"]",
                               (const char*)this+debugGetCurrentOffset(),
                               getDescription() );
            else
                VERBOSE_TRACE( "Node state: n0x%X [label=\"%s\",style=filled,fillcolor=\"%f,1.0,1.0\"]",
                               (const char*)this+debugGetCurrentOffset(),
                               getDescription(),
                               log((double)getState())/4.0 );

            // determine additional node information
            uno::Reference< animations::XAnimate > xAnimate( mxNode,
                                                             uno::UNO_QUERY );
            if( xAnimate.is() )
            {
                uno::Reference< drawing::XShape > xTargetShape( xAnimate->getTarget(),
                                                                uno::UNO_QUERY );

                if( !xTargetShape.is() )
                {
                    ::com::sun::star::presentation::ParagraphTarget aTarget;

                    // no shape provided. Maybe a ParagraphTarget?
                    if( (xAnimate->getTarget() >>= aTarget) )
                        xTargetShape = aTarget.Shape;
                }

                if( xTargetShape.is() )
                {
                    uno::Reference< beans::XPropertySet > xPropSet( xTargetShape,
                                                                    uno::UNO_QUERY );

                    // read shape name
                    ::rtl::OUString aName;
                    if( (xPropSet->getPropertyValue(
                             ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Name") ) ) >>= aName) )
                    {
                        const ::rtl::OString& rAsciiName(
                            ::rtl::OUStringToOString( aName,
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

        void BaseNode::showTreeFromWithin() const
        {
            // find root node
            BaseNodeSharedPtr pCurrNode( mpSelf );
            while( pCurrNode->mpParent.get() ) pCurrNode = pCurrNode->mpParent;

            pCurrNode->showState();
        }
#endif

    }
}
