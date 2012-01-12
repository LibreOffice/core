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
#include "cs_type.hxx"

// NOT FULLY DEFINED SERVICES
#include <ary/cpp/c_builtintype.hxx>


namespace
{
    const uintt
        C_nReservedElements = ary::cpp::predefined::t_MAX;    // Skipping "0" and the builtin types
}


namespace ary
{
namespace cpp
{



Type_Storage *    Type_Storage::pInstance_ = 0;




Type_Storage::Type_Storage()
    :   stg::Storage<Type>(C_nReservedElements),
        aBuiltInTypes()
{
    Setup_BuiltInTypes();

    csv_assert(pInstance_ == 0);
    pInstance_ = this;
}

Type_Storage::~Type_Storage()
{
    csv_assert(pInstance_ != 0);
    pInstance_ = 0;
}

Type_id
Type_Storage::Search_BuiltInType( const String & i_specializedName ) const
{
    return csv::value_from_map(aBuiltInTypes, i_specializedName, Tid(0));
}

void
Type_Storage::Setup_BuiltInTypes()
{
    Set_BuiltInType( predefined::t_void, "void" );
    Set_BuiltInType( predefined::t_bool, "bool" );
    Set_BuiltInType( predefined::t_char, "char" );
    Set_BuiltInType( predefined::t_signed_char, "char", TYSP_signed );
    Set_BuiltInType( predefined::t_unsigned_char, "char", TYSP_unsigned );
    Set_BuiltInType( predefined::t_short, "short" );
    Set_BuiltInType( predefined::t_unsigned_short, "short", TYSP_unsigned );
    Set_BuiltInType( predefined::t_int, "int" );
    Set_BuiltInType( predefined::t_unsigned_int, "int", TYSP_unsigned );
    Set_BuiltInType( predefined::t_long, "long" );
    Set_BuiltInType( predefined::t_unsigned_long, "long", TYSP_unsigned );
    Set_BuiltInType( predefined::t_float, "float" );
    Set_BuiltInType( predefined::t_double, "double" );
    Set_BuiltInType( predefined::t_size_t, "size_t" );
    Set_BuiltInType( predefined::t_wchar_t, "wchar_t" );
    Set_BuiltInType( predefined::t_ptrdiff_t, "ptrdiff_t" );
    Set_BuiltInType( predefined::t_ellipse, "..." );
}

void
Type_Storage::Set_BuiltInType( Rid                              i_id,
                               const char *                     i_sName,
                               ary::cpp::E_TypeSpecialisation   i_eSpecialisation )
{
    DYN BuiltInType &
        rNew = *new BuiltInType(i_sName, i_eSpecialisation);
    Set_Reserved( i_id, rNew);  // Here goes the ownership for rNew.
    aBuiltInTypes[rNew.SpecializedName()] = rNew.TypeId();
}



}   // namespace cpp
}   // namespace ary
