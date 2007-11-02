/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_traits.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:51:40 $
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
    static id_type      IdOf_(
                            const entity_base_type &
                                                i_entity );
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
    static const String &
                        LocalNameOf_(
                            const entity_base_type &
                                                i_entity );
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
    static id_type      IdOf_(
                            const entity_base_type &
                                                i_entity );
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
    static id_type      IdOf_(
                            const entity_base_type &
                                                i_entity );
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
