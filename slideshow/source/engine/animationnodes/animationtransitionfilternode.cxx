/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animationtransitionfilternode.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:42:57 $
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

#include <animationtransitionfilternode.hxx>
#include <transitionfactory.hxx>


using namespace ::com::sun::star;

namespace presentation
{
    namespace internal
    {
        AnimationTransitionFilterNode::AnimationTransitionFilterNode( const uno::Reference< animations::XAnimationNode >&   xNode,
                                                                      const BaseContainerNodeSharedPtr&                     rParent,
                                                                      const NodeContext&                                    rContext ) :
            ActivityAnimationBaseNode( xNode, rParent, rContext ),
            mxTransitionFilterNode( xNode, uno::UNO_QUERY_THROW )
        {
        }

        void AnimationTransitionFilterNode::dispose()
        {
            mxTransitionFilterNode.clear();

            ActivityAnimationBaseNode::dispose();
        }

        bool AnimationTransitionFilterNode::init()
        {
            if( !ActivityAnimationBaseNode::init() )
                return false;

            try
            {
                // TODO(F2): For restart functionality, we must regenerate activities,
                // since they are not able to reset their state (or implement _that_)
                getActivity() =
                    TransitionFactory::createShapeTransition( fillCommonParameters(),
                                                              getShape(),
                                                              getContext().mpLayerManager,
                                                              mxTransitionFilterNode );
            }
            catch( uno::Exception& )
            {
                // catch and ignore. We later handle empty activities, but for
                // other nodes to function properly, the core functionality of
                // this node must remain up and running.
            }

            return true;
        }

#if defined(VERBOSE) && defined(DBG_UTIL)
        const char* AnimationTransitionFilterNode::getDescription() const
        {
            return "AnimationTransitionFilterNode";
        }
#endif
    }
}
