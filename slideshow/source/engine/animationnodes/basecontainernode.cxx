/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basecontainernode.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:43:25 $
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

#include <basecontainernode.hxx>
#include <tools.hxx>
#include <nodetools.hxx>
#include <delayevent.hxx>

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
        BaseContainerNode::BaseContainerNode( const uno::Reference< animations::XAnimationNode >&   xNode,
                                              const BaseContainerNodeSharedPtr&                     rParent,
                                              const NodeContext&                                    rContext ) :
            BaseNode( xNode, rParent, rContext ),
            maChildren(),
            maFinishedStates(),
            mnFinishedChildren(0),
            mbOverrideIndefiniteBegin( false ),
            mbDurationIndefinite( isIndefiniteTiming( xNode->getEnd() ) ||
                                  isIndefiniteTiming( xNode->getDuration() ) )
        {
        }

        bool BaseContainerNode::activate()
        {
            if( getState() == ACTIVE )
                return true; // avoid duplicate event generation

            if( !BaseNode::activate() )
                return false;

            if( isDurationInfinite() &&
                maChildren.empty() )
            {
                // schedule deactivation:
                maContext.mrEventQueue.addEvent(
                    makeEvent( boost::bind( &BaseNode::deactivate,
                                            getSelf() ) ) );
            }

            return true;
        }

        void BaseContainerNode::dispose()
        {
            ::std::for_each( maChildren.begin(),
                             maChildren.end(),
                             ::boost::mem_fn(&BaseNode::dispose) );
            maChildren.clear();
            maFinishedStates.clear();

            BaseNode::dispose();
        }

        bool BaseContainerNode::init()
        {
            if( !BaseNode::init() )
                return false;

            // init local state
            ::std::fill( maFinishedStates.begin(),
                         maFinishedStates.end(),
                         false );
            mnFinishedChildren = 0;
            mbOverrideIndefiniteBegin = false;

            // initialize all children
            if( ::std::count_if( maChildren.begin(),
                                 maChildren.end(),
                                 ::boost::mem_fn(&AnimationNode::init) )
                != static_cast<VectorOfNodes::difference_type>(maChildren.size()) )
            {
                return false; // not all children have been correctly initialized
            }

            return true;
        }

        void BaseContainerNode::end()
        {
            BaseNode::end();

            // end all children
            ::std::for_each( maChildren.begin(),
                             maChildren.end(),
                             ::boost::mem_fn(&AnimationNode::end) );
        }

        bool BaseContainerNode::hasPendingAnimation() const
        {
            const VectorOfNodes::const_iterator aEnd( maChildren.end() );

            // does any of our children returns "true" on AnimationNode::hasPendingAnimation()?
            // If yes, we, too, return true
            return ::std::find_if( maChildren.begin(),
                                   aEnd,
                                   ::boost::mem_fn(&AnimationNode::hasPendingAnimation) ) != aEnd;
        }

        void BaseContainerNode::appendChildNode( const BaseNodeSharedPtr& rNode )
        {
            ENSURE_AND_THROW( getSelf().get(),
                              "BaseContainerNode::appendChildNode(): no self set" );

            // early exit on invalid nodes
            if( getState() == INVALID )
                return;

            // register ourself as end listeners at all children.
            // this is necessary to control the children animation
            // sequence, and to determine our own end event
            if( rNode->registerDeactivatingListener( getSelf() ) )
            {
                maChildren.push_back( rNode );
                maFinishedStates.push_back( false );
            }
        }

        void BaseContainerNode::scheduleActivationEvent()
        {
            // buffer value for check below
            const bool bOverrideIndefiniteBegin( mbOverrideIndefiniteBegin );

            // request will be fulfilled, but normalize state early
            // (prevent callback errors)
            mbOverrideIndefiniteBegin = false;

            // for indefinite start event - our begin event
            // becomes definite, if a child has issued a
            // requestResolveOnChildren(). Otherwise, we'll never
            // become active.
            if( bOverrideIndefiniteBegin &&
                isIndefiniteTiming( getXNode()->getBegin() ) )
            {
                // become active immediately.
                if( !activate() )
                    OSL_ENSURE( false,
                                "BaseContainerNode::scheduleActivationEvent(): Could not activate ourselves" );
            }
            else
            {
                // no override necessary - use normal BaseNode method
                BaseNode::scheduleActivationEvent();
            }
        }

        void BaseContainerNode::requestResolveOnChildren()
        {
            // early exit on invalid nodes
            if( getState() == INVALID )
                return;

            // coerce our resolve() to override indefinite begins.
            // After all, we now _have_ defined begin time, which is
            // _now_.
            mbOverrideIndefiniteBegin = true;

            if( getState() == UNRESOLVED )
            {
                ENSURE_AND_THROW( getParentNode().get(),
                                  "BaseContainerNode::requestResolveOnChildren(): No parent, and still unresolved" );

                // if we ourselves are unresolved, request parent
                // to resolve us
                getParentNode()->requestResolveOnChildren();
            }
            else
            {
                // otherwise, try to (re)start ourselves.
                if( !activate() )
                    OSL_ENSURE( false,
                                "BaseContainerNode::requestResolveOnChildren(): Could not (re)start" );
            }
        }

        // Debug
        //=========================================================================

#if defined(VERBOSE) && defined(DBG_UTIL)
        void BaseContainerNode::showState() const
        {
            for( ::std::size_t i=0; i<maChildren.size(); ++i )
            {
                VERBOSE_TRACE( "Node connection: n0x%X -> n0x%X",
                               (const char*)this+debugGetCurrentOffset(),
                               (const char*)maChildren[i].get()+debugGetCurrentOffset() );
                maChildren[i]->showState();
            }

            BaseNode::showState();
        }

        const char* BaseContainerNode::getDescription() const
        {
            return "BaseNode";
        }
#endif

    }
}
