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
