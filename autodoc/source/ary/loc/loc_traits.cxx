/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: loc_traits.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:17:05 $
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

Le_Traits::id_type
Le_Traits::IdOf_(const entity_base_type & i_entity)
{
    return i_entity.LeId();
}




//********************      LeNode_Traits       ************************//
symtree::Node<LeNode_Traits> *
LeNode_Traits::NodeOf_(entity_base_type & io_entity)
{
    if (is_type<Directory>(io_entity))
        return & ary_cast<Directory>(io_entity).AsNode();
    return 0;
}

const String &
LeNode_Traits::LocalNameOf_(const entity_base_type & i_entity)
{
    return i_entity.LocalName();
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

template <class KEY>
Le_Traits::id_type
LeNode_Traits::Search_( const entity_base_type & i_entity,
                        const KEY &              i_localKey )
{
    if (is_type<Directory>(i_entity))
        return ary_cast<Directory>(i_entity).Search_Child(i_localKey);
    return id_type(0);
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
