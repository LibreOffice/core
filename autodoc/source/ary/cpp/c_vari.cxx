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
#include <ary/cpp/c_vari.hxx>


// NOT FULLY DECLARED SERVICES



namespace ary
{
namespace cpp
{

Variable::Variable( const String  &     i_sLocalName,
                    Cid                 i_nOwner,
                    E_Protection        i_eProtection,
                    Lid                 i_nFile,
                    Tid                 i_nType,
                    VariableFlags       i_aFlags,
                    const String  &     i_sArraySize,
                    const String  &     i_sInitValue )
    :   aEssentials( i_sLocalName,
                     i_nOwner,
                     i_nFile ),
           nType(i_nType),
        eProtection(i_eProtection),
        aFlags(i_aFlags),
        sArraySize(i_sArraySize),
        sInitialisation(i_sInitValue)
{
}

Variable::~Variable()
{
}

const String  &
Variable::inq_LocalName() const
{
    return aEssentials.LocalName();
}

Cid
Variable::inq_Owner() const
{
    return aEssentials.Owner();
}

Lid
Variable::inq_Location() const
{
    return aEssentials.Location();
}

void
Variable::do_Accept(csv::ProcessorIfc & io_processor) const
{
    csv::CheckedCall(io_processor,*this);
}

ClassId
Variable::get_AryClass() const
{
    return class_id;
}


}   // namespace cpp
}   // namespace ary
