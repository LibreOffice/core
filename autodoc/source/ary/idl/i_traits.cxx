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
#include <ary/idl/i_traits.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/namesort.hxx>
#include "is_ce.hxx"



namespace ary
{
namespace idl
{



//********************      Ce_Traits      ********************//
Ce_Traits::entity_base_type &
Ce_Traits::EntityOf_(id_type i_id)
{
    csv_assert(i_id.IsValid());
    return Ce_Storage::Instance_()[i_id];
}

//********************      Ce_Compare      ********************//
const Ce_Compare::key_type &
Ce_Compare::KeyOf_(const entity_base_type & i_entity)
{
    return i_entity.LocalName();
}

bool
Ce_Compare::Lesser_( const key_type &    i_1,
                     const key_type &    i_2 )
{
    static ::ary::LesserName    less_;
    return less_(i_1,i_2);
}




}   // namespace idl
}   // namespace ary
