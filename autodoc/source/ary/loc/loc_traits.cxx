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

#include <precomp.h>
#include <ary/loc/loc_traits.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/namesort.hxx>
#include <ary/getncast.hxx>
#include "locs_le.hxx"



namespace ary
{
namespace loc
{


//********************      Le_Traits       ************************//
Le_Traits::entity_base_type &
Le_Traits::EntityOf_(id_type i_id)
{
    csv_assert(i_id.IsValid());
    return Le_Storage::Instance_()[i_id];
}

//********************      LeNode_Traits       ************************//
symtree::Node<LeNode_Traits> *
LeNode_Traits::NodeOf_(entity_base_type & io_entity)
{
    if (is_type<Directory>(io_entity))
        return & ary_cast<Directory>(io_entity).AsNode();
    return 0;
}

Le_Traits::entity_base_type *
LeNode_Traits::ParentOf_(const entity_base_type & i_entity)
{
    Le_Traits::id_type
        ret = i_entity.ParentDirectory();
    if (ret.IsValid())
        return &EntityOf_(ret);
    return 0;
}

//********************      Le_Compare       ************************//
const Le_Compare::key_type &
Le_Compare::KeyOf_(const entity_base_type & i_entity)
{
    return i_entity.LocalName();
}

bool
Le_Compare::Lesser_( const key_type &    i_1,
                     const key_type &    i_2 )
{
    static ::ary::LesserName    less_;
    return less_(i_1,i_2);
}




}   // namespace loc
}   // namespace ary
