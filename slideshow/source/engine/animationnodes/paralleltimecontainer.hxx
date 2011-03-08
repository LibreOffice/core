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
#ifndef INCLUDED_SLIDESHOW_PARALLELTIMECONTAINER_HXX
#define INCLUDED_SLIDESHOW_PARALLELTIMECONTAINER_HXX

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

#if defined(VERBOSE) && defined(DBG_UTIL)
    virtual const char* getDescription() const
        { return "ParallelTimeContainer"; }
#endif

private:
    virtual void activate_st();
    virtual void notifyDeactivating( AnimationNodeSharedPtr const& pChildNode );
};

} // namespace internal
} // namespace slideshow

#endif /* INCLUDED_SLIDESHOW_PARALLELTIMECONTAINER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
