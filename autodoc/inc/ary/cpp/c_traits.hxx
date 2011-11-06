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



#ifndef ARY_CPP_C_TRAITS_HXX
#define ARY_CPP_C_TRAITS_HXX



// USED SERVICES
    // BASE CLASSES
    // OTHER
#include <ary/cpp/c_types4cpp.hxx>



namespace ary
{
namespace symtree
{
    template <class X> class Node;
}
}




namespace ary
{
namespace cpp
{


/** Basic traits for derived ones of ->CodeEntity.
*/
struct Ce_Traits
{
    typedef CodeEntity          entity_base_type;
    typedef Ce_id               id_type;

    static entity_base_type &
                        EntityOf_(
                            id_type             i_id );
};


/** An instance of SYMBOL_TRAITS for ->::ary::SortedIds<>.

    @see ::ary::SortedIds<>
*/
struct CeNode_Traits : public Ce_Traits
{
    static const symtree::Node<CeNode_Traits> *
                        NodeOf_(
                            const entity_base_type &
                                                i_entity );
    static symtree::Node<CeNode_Traits> *
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
struct Ce_Compare : public Ce_Traits
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

/** An instance of COMPARE for ->::ary::SortedIds<>.

    @see ::ary::SortedIds<>
*/
struct Ce_GlobalCompare : public Ce_Traits
{
    typedef  entity_base_type   key_type;

    static const key_type &
                        KeyOf_(
                            const entity_base_type &
                                                i_entity )
                            { return i_entity; }
    static bool         Lesser_(
                            const key_type &    i_1,
                            const key_type &    i_2 );
};


/** Basic traits for derivd ones of ->DefineEntity.
*/
struct Def_Traits
{
    typedef DefineEntity        entity_base_type;
    typedef De_id               id_type;

    static entity_base_type &
                        EntityOf_(
                            id_type             i_id );
};


/** An instance of COMPARE for ->::ary::SortedIds<>.

    @see ::ary::SortedIds<>
*/
struct Def_Compare : public Def_Traits
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

/** Basic traits for derivd ones of ->Type.
*/
struct Type_Traits
{
    typedef Type                entity_base_type;
    typedef Type_id             id_type;

    static entity_base_type &
                        EntityOf_(
                            id_type             i_id );
};

/** An instance of COMPARE for ->::ary::SortedIds<>.

    @see ::ary::SortedIds<>
*/
struct UsedType_Compare : public Type_Traits
{
    typedef  UsedType           key_type;

    static const key_type &
                        KeyOf_(
                            const entity_base_type &
                                                i_entity );
    static bool         Lesser_(
                            const key_type &    i_1,
                            const key_type &    i_2 );
};






// IMPLEMENTATION

/// Implementation helper for ->CeNode_Traits::Search_ .
Ce_id                   CeNode_Search(
                            const CodeEntity &  i_entity,
                            const String &      i_localKey );


template <class KEY>
Ce_Traits::id_type
CeNode_Traits::Search_( const entity_base_type & i_entity,
                        const KEY &              i_localKey )
{
    return CeNode_Search(i_entity, i_localKey);
}




}   //  namespace cpp
}   //  namespace ary
#endif
