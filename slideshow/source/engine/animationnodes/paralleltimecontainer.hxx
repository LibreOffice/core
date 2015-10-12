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
#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_ANIMATIONNODES_PARALLELTIMECONTAINER_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_ANIMATIONNODES_PARALLELTIMECONTAINER_HXX

#include "basecontainernode.hxx"

namespace slideshow {
namespace internal {

/** This class implements parallel node containers

    All children of this node are played in parallel
*/
class ParallelTimeContainer : public BaseContainerNode
{
public:
    ParallelTimeContainer(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::animations::XAnimationNode >& xNode,
        const BaseContainerNodeSharedPtr&               rParent,
        const NodeContext&                              rContext )
        : BaseContainerNode( xNode, rParent, rContext ) {}

#if defined(DBG_UTIL)
    virtual const char* getDescription() const override
        { return "ParallelTimeContainer"; }
#endif

private:
    virtual void activate_st() override;
    virtual void notifyDeactivating( AnimationNodeSharedPtr const& pChildNode ) override;
};

} // namespace internal
} // namespace slideshow

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_ANIMATIONNODES_PARALLELTIMECONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
