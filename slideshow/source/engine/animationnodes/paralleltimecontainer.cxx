/*************************************************************************
 *
 *  $RCSfile: paralleltimecontainer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 17:06:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include <paralleltimecontainer.hxx>
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
        ParallelTimeContainer::ParallelTimeContainer( const uno::Reference< animations::XAnimationNode >&   xNode,
                                                      const BaseContainerNodeSharedPtr&                     rParent,
                                                      const NodeContext&                                    rContext ) :
            BaseContainerNode( xNode, rParent, rContext )
        {
        }

        bool ParallelTimeContainer::activate()
        {
            if( getState() == ACTIVE )
                return true; // avoid duplicate event generation

            if( !BaseNode::activate() )
                return false;

            // resolve all children, only return true, if _all_
            // children were resolvable.
            return ::std::count_if( getChildren().begin(),
                                    getChildren().end(),
                                    ::boost::mem_fn(&AnimationNode::resolve) )
                == static_cast<VectorOfNodes::difference_type>(getChildren().size());
        }

        void ParallelTimeContainer::notifyDeactivating( const AnimationNodeSharedPtr& rNotifier )
        {
            // early exit on invalid nodes
            if( getState() == INVALID )
                return;

            // if we don't have indefinite duration, ignore this message
            if( !isDurationInfinite() )
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
                            "ParallelTimeContainer::notifyDeactivating(): unknown notifier" );
                return;
            }

            const ::std::size_t nIndex( ::std::distance(aBegin, aIter) );

            // prevent duplicate increments (for children that notify
            // more than once)
            if( getFinishedStates()[ nIndex ] == false )
                ++getFinishedCount();

            getFinishedStates()[ nIndex ] = true;

            // all children finished?
            if( getFinishedCount() == getChildren().size() )
            {
                // yep. deactivate this node, too
                deactivate();
            }
        }

#if defined(VERBOSE) && defined(DBG_UTIL)
        const char* ParallelTimeContainer::getDescription() const
        {
            return "ParallelTimeContainer";
        }
#endif

    }
}
