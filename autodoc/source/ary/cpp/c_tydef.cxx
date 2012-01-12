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
#include <ary/cpp/c_tydef.hxx>


// NOT FULLY DECLARED SERVICES
#include <slots.hxx>
#include "c_slots.hxx"




namespace ary
{
namespace cpp
{

Typedef::Typedef( const String  &     i_sLocalName,
                  Cid                 i_nOwner,
                  E_Protection        i_eProtection,
                  Lid                 i_nFile,
                  Tid                 i_nDescribingType )
    :   aEssentials( i_sLocalName,
                     i_nOwner,
                     i_nFile ),
        nDescribingType(i_nDescribingType),
        eProtection(i_eProtection)
{
}

Typedef::~Typedef()
{

}

const String  &
Typedef::inq_LocalName() const
{
    return aEssentials.LocalName();
}

Cid
Typedef::inq_Owner() const
{
    return aEssentials.Owner();
}

Lid
Typedef::inq_Location() const
{
    return aEssentials.Location();
}

void
Typedef::do_Accept(csv::ProcessorIfc & io_processor) const
{
    csv::CheckedCall(io_processor,*this);
}

ClassId
Typedef::get_AryClass() const
{
    return class_id;
}


}   // namespace cpp
}   // namespace ary
