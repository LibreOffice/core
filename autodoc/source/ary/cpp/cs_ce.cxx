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
#include "cs_ce.hxx"

// NOT FULLY DEFINED SERVICES
#include <ary/cpp/c_namesp.hxx>



namespace
{
const uintt
    C_nReservedElements = ::ary::cpp::predefined::ce_MAX;    // Skipping "0" and the GlobalNamespace
}




namespace ary
{
namespace cpp
{

Ce_Storage *    Ce_Storage::pInstance_ = 0;



Ce_Storage::Ce_Storage()
    :   stg::Storage<CodeEntity>(C_nReservedElements),
        aTypes(),
        aOperations(),
        aData()

{
    Set_Reserved( predefined::ce_GlobalNamespace,
                  *new Namespace );

    csv_assert(pInstance_ == 0);
    pInstance_ = this;
}

Ce_Storage::~Ce_Storage()
{
    csv_assert(pInstance_ != 0);
    pInstance_ = 0;
}

Ce_id
Ce_Storage::Store_Type(DYN CodeEntity & pass_ce)
{
    Ce_id
        ret = Store_Entity(pass_ce);
    aTypes.Add(ret);
    return ret;
}

Ce_id
Ce_Storage::Store_Operation(DYN CodeEntity & pass_ce)
{
    Ce_id
        ret = Store_Entity(pass_ce);
    aOperations.Add(ret);
    return ret;
}

Ce_id
Ce_Storage::Store_Datum(DYN CodeEntity & pass_ce)
{
    Ce_id
        ret = Store_Entity(pass_ce);
    aData.Add(ret);
    return ret;
}



}   // namespace cpp
}   // namespace ary
