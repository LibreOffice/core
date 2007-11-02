/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: traits_impl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:03:41 $
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
