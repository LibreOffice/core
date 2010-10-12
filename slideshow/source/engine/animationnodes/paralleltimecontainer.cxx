/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

#include "paralleltimecontainer.hxx"
#include "delayevent.hxx"

#include <boost/bind.hpp>

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
            makeEvent( boost::bind( &AnimationNode::deactivate, getSelf() ),
                       "ParallelTimeContainer::deactivate") );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
