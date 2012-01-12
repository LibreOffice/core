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
#include "ca_type.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/cpp/c_builtintype.hxx>
#include <ary/cpp/c_class.hxx>
#include <ary/cpp/c_namesp.hxx>
#include <ary/cpp/cp_ce.hxx>
#include <ary/cpp/inpcontx.hxx>
#include <ary/cpp/usedtype.hxx>
#include <ary/getncast.hxx>
#include "c_reposypart.hxx"
#include "cs_type.hxx"




namespace ary
{
namespace cpp
{


TypeAdmin::TypeAdmin(RepositoryPartition & io_myReposyPartition)
    :   aStorage(),
        pCppRepositoryPartition(&io_myReposyPartition)
{
}

TypeAdmin::~TypeAdmin()
{
}


// KORR_FUTURE
//  Remove unused parameter.

const Type &
TypeAdmin::CheckIn_UsedType( const InputContext &   ,
                             DYN UsedType &         pass_type )
{
    Dyn<UsedType>
        pNewType(&pass_type);  // Ensure clean up of heap object.

    Type_id
        tid(0);
    if (pass_type.IsBuiltInType())
    {
        tid = aStorage.Search_BuiltInType(
                        BuiltInType::SpecializedName_( pass_type.LocalName().c_str(),
                                                       pass_type.TypeSpecialisation() ));
        csv_assert(tid.IsValid());
        return aStorage[tid];
    }

    tid = aStorage.UsedTypeIndex().Search(pass_type);
    if (tid.IsValid())
    {
        return aStorage[tid];
    }

    // Type does not yet exist:
        // Transfer ownership from pNewTypeand assign id:
    aStorage.Store_Entity(*pNewType.Release());

    aStorage.UsedTypeIndex().Add(pass_type.TypeId());
    return pass_type;
}

const Type &
TypeAdmin::Find_Type(Type_id i_type) const
{
    return aStorage[i_type];
}

bool
TypeAdmin::Get_TypeText( StreamStr &         o_result,
                         Type_id             i_type ) const
{
    if (NOT i_type.IsValid())
        return false;
    aStorage[i_type].Get_Text(o_result, *pCppRepositoryPartition);
    return true;
}

bool
TypeAdmin::Get_TypeText( StreamStr &         o_preName,
                         StreamStr &         o_name,
                         StreamStr &         o_postName,
                         Type_id             i_type ) const
{
    if (NOT i_type.IsValid())
        return false;
    aStorage[i_type].Get_Text(o_preName, o_name, o_postName, *pCppRepositoryPartition);
    return true;
}

Type_id
TypeAdmin::Tid_Ellipse() const
{
    return Type_id(predefined::t_ellipse);
}




}   // namespace cpp
}   // namespace ary
