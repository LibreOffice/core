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
#include <ary/idl/i_ce.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/doc/d_oldidldocu.hxx>
#include <ary/getncast.hxx>


namespace ary
{
namespace idl
{



CodeEntity::CodeEntity()
    :   aDocu(),
        p2s(0)
{
}

CodeEntity::~CodeEntity()
{
}

const Ce_2s &
CodeEntity::Secondaries() const
{
    return const_cast<CodeEntity*>(this)->Secondaries();
}

Ce_2s &
CodeEntity::Secondaries()
{
    if (p2s)
        return *p2s;
    p2s = Ce_2s::Create_(AryClass());
    return *p2s;
}




}   // namespace idl
}   // namespace ary
