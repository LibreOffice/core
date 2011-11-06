/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
