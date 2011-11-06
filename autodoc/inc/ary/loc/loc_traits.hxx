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



#ifndef ARY_LOC_TRAITS_HXX
#define ARY_LOC_TRAITS_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <ary/loc/loc_types4loc.hxx>


namespace ary
{
namespace symtree
{
    template <class> class Node;
}
}




namespace ary
{
namespace loc
{



/** Basic traits for derived ones of ->LocationEntity.
*/
struct Le_Traits
{
    typedef LocationEntity      entity_base_type;
    typedef Le_id               id_type;

    static entity_base_type &
                        EntityOf_(
                            id_type             i_id );
};


/** An instance of SYMBOL_TRAITS for ->::ary::symtree::Node.

    @see ::ary::symtree::Node
*/
struct LeNode_Traits : public Le_Traits
{
    static symtree::Node<LeNode_Traits> *
                        NodeOf_(
                            entity_base_type &  i_entity );

    static entity_base_type *
                        ParentOf_(
                            const entity_base_type &
                                                i_entity );
    template <class KEY>
    static id_type      Search_(
                            const entity_base_type &
                                                i_entity,
                            const KEY &         i_localKey );
};



/** An instance of COMPARE for ->::ary::SortedIds<>.

    @see ::ary::SortedIds<>
*/
struct Le_Compare : public Le_Traits
{
    typedef  String             key_type;

    static const key_type &
                        KeyOf_(
                            const entity_base_type &
                                                i_entity );
    static bool         Lesser_(
                            const key_type &    i_1,
                            const key_type &    i_2 );
};





} // namespace loc
} // namespace ary
#endif
