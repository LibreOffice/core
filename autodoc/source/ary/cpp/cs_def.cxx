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
#include "cs_def.hxx"

// NOT FULLY DEFINED SERVICES


namespace
{
const uintt
    C_nReservedElements = ::ary::cpp::predefined::de_MAX;    // Skipping "0"
}



namespace ary
{
namespace cpp
{

Def_Storage *    Def_Storage::pInstance_ = 0;




Def_Storage::Def_Storage()
    :   stg::Storage<DefineEntity>(C_nReservedElements)
{
    csv_assert(pInstance_ == 0);
    pInstance_ = this;
}

Def_Storage::~Def_Storage()
{
    csv_assert(pInstance_ != 0);
    pInstance_ = 0;
}

De_id
Def_Storage::Store_Define(DYN DefineEntity &  pass_de)
{
    De_id
        ret = Store_Entity(pass_de);
    aDefines.Add(ret);
    return ret;
}

De_id
Def_Storage::Store_Macro(DYN DefineEntity &  pass_de)
{
    De_id
        ret = Store_Entity(pass_de);
    aMacros.Add(ret);
    return ret;
}



}   // namespace cpp
}   // namespace ary
