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


#include "paralleltimecontainer.hxx"
#include <delayevent.hxx>

#include <functional>

namespace slideshow::internal {

void ParallelTimeContainer::activate_st()
{
    // resolve all children:
    std::size_t const nResolvedNodes =
        static_cast<std::size_t>(std::count_if(
                                     maChildren.begin(), maChildren.end(),
                                     std::mem_fn(&AnimationNode::resolve) ));
    OSL_ENSURE( nResolvedNodes == maChildren.size(),
                "### resolving all children failed!" );

    if (isDurationIndefinite() && maChildren.empty()) {
        // deactivate ASAP:
        auto self(getSelf());
        scheduleDeactivationEvent(
            makeEvent( [self=std::move(self)] () { self->deactivate(); },
                       u"ParallelTimeContainer::deactivate"_ustr) );
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

} // namespace slideshow::internal

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
