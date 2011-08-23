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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
