/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sequentialtimecontainer.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:46:40 $
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

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include <sequentialtimecontainer.hxx>
#include <tools.hxx>
#include <nodetools.hxx>

#ifndef BOOST_BIND_HPP_INCLUDED
#include <boost/bind.hpp>
#endif
#ifndef BOOST_MEM_FN_HPP_INCLUDED
#include <boost/mem_fn.hpp>
#endif

#include <algorithm>

using namespace ::com::sun::star;

namespace presentation
{
    namespace internal
    {
        SequentialTimeContainer::SequentialTimeContainer( const uno::Reference< animations::XAnimationNode >&   xNode,
                                                          const BaseContainerNodeSharedPtr&                     rParent,
                                                          const NodeContext&                                    rContext ) :
            BaseContainerNode( xNode, rParent, rContext )
        {
        }

        bool SequentialTimeContainer::activate()
        {
            if( getState() == ACTIVE )
                return true; // avoid duplicate event generation

            if( !BaseContainerNode::activate() )
                return false;

            // resolve first child
            if( !getChildren().empty() )
                return getChildren().front()->resolve();

            return true;
        }

        void SequentialTimeContainer::notifyDeactivating( const AnimationNodeSharedPtr& rNotifier )
        {
            // early exit on invalid nodes
            if( getState() == INVALID )
                return;

            // find given notifier in child vector
            const VectorOfNodes::const_iterator aBegin( getChildren().begin() );
            const VectorOfNodes::const_iterator aEnd( getChildren().end() );
            VectorOfNodes::const_iterator       aIter;
            if( (aIter=::std::find( aBegin,
                                    aEnd,
                                    rNotifier )) == aEnd )
            {
                OSL_ENSURE( false,
                            "SequentialTimeContainer::notifyDeactivating(): unknown notifier" );
                return;
            }

            const ::std::size_t nIndex( ::std::distance(aBegin, aIter) );

            // prevent duplicate actions (for children that notify
            // more than once)
            if( getFinishedStates()[ nIndex ] == false )
            {
                // store new child state
                ++getFinishedCount();
                getFinishedStates()[ nIndex ] = true;

                // resolve next child
                if( isDurationInfinite() &&
                    getFinishedCount() < getChildren().size() &&
                    !getChildren()[getFinishedCount()]->resolve() )
                {
                    // could not resolve child - since we risk to
                    // stall the chain of events here, play it safe
                    // and deactivate this node (only if we have
                    // indefinite duration - otherwise, we'll get a
                    // deactivation event, anyways).
                    deactivate();
                    return;
                }
            }

            // all children finished, and we've got indefinite duration?
            if( isDurationInfinite() &&
                getFinishedCount() == getChildren().size() )
            {
                // yep. deactivate this node, too
                deactivate();
            }
        }

#if defined(VERBOSE) && defined(DBG_UTIL)
        const char* SequentialTimeContainer::getDescription() const
        {
            return "SequentialTimeContainer";
        }
#endif

    }
}
