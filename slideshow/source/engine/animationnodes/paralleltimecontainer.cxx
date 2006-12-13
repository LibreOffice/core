/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: paralleltimecontainer.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 15:34:00 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

#include "paralleltimecontainer.hxx"
#include "delayevent.hxx"
#include "boost/bind.hpp"
#include "boost/mem_fn.hpp"
#include <algorithm>

namespace slideshow {
namespace internal {

void ParallelTimeContainer::activate_st()
{
    // resolve all children:
    std::size_t const nResolvedNodes =
        static_cast<std::size_t>(std::count_if(
                                     maChildren.begin(), maChildren.end(),
                                     boost::mem_fn(&AnimationNode::resolve) ));
    (void) nResolvedNodes; // avoid warning
    OSL_ENSURE( nResolvedNodes == maChildren.size(),
                "### resolving all children failed!" );

    if (isDurationIndefinite() && maChildren.empty()) {
        // deactivate ASAP:
        scheduleDeactivationEvent(
            makeEvent( boost::bind( &AnimationNode::deactivate, getSelf() ) ) );
    }
    else { // use default
        scheduleDeactivationEvent();
    }
}

void ParallelTimeContainer::notifyDeactivating(
    AnimationNodeSharedPtr const& pChildNode )
{
    notifyDeactivatedChild( pChildNode );
}

} // namespace internal
} // namespace slideshow

