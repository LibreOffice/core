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

#ifndef ARY_TRAITS_IMPL_HXX
#define ARY_TRAITS_IMPL_HXX


// USED SERVICES
#include <ary/getncast.hxx>


namespace ary
{
namespace traits
{


/** Finds the node assigned to an entity, if that entity has a specific
    actual type.

    @tpl NODE
    The assumed actual type of io_node.
*/
template<class NODE>
const typename NODE::node_t *
                    NodeOf(
                        const typename NODE::traits_t::entity_base_type &
                                                io_node );

/** Finds the node assigned to an entity, if that entity has a specific
    actual type.

    @tpl NODE
    The assumed actual type of io_node.
*/
template<class NODE>
typename NODE::node_t *
                    NodeOf(
                        typename NODE::traits_t::entity_base_type &
                                                io_node );

/** Finds a child to a node.
*/
template<class NODE, class KEY>
typename NODE::traits_t::id_type
                    Search_Child(
                        const typename NODE::traits_t::entity_base_type &
                                                i_node,
                        const KEY &             i_localKey );




// IMPLEMENTATION

template<class NODE>
const typename NODE::node_t *
NodeOf(const typename NODE::traits_t::entity_base_type & io_node)
{
    const NODE *
        pn = ary_cast<NODE>(&io_node);
    if (pn != 0)
        return & pn->AsNode();
    return 0;
}

template<class NODE>
typename NODE::node_t *
NodeOf(typename NODE::traits_t::entity_base_type & io_node)
{
    NODE *
        pn = ary_cast<NODE>(&io_node);
    if (pn != 0)
        return & pn->AsNode();
    return 0;
}

template<class NODE, class KEY>
typename NODE::traits_t::id_type
Search_Child( const typename NODE::traits_t::entity_base_type & i_node,
              const KEY &                                       i_localKey )
{
    const NODE *
        pn = ary_cast<NODE>(&i_node);
    if (pn != 0)
        return pn->Search_Child(i_localKey);
    return typename NODE::traits_t::id_type(0);
}




}   // namespace traits
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
