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
#include <ary/cpp/c_enum.hxx>


// NOT FULLY DECLARED SERVICES
#include <slots.hxx>
#include "c_slots.hxx"




namespace ary
{
namespace cpp
{

Enum::Enum( const String  &     i_sLocalName,
            Ce_id               i_nOwner,
            E_Protection        i_eProtection,
            Lid                 i_nFile )
    :   aEssentials( i_sLocalName,
                     i_nOwner,
                     i_nFile ),
        aValues(),
        eProtection(i_eProtection)
{
}

Enum::~Enum()
{
}

void
Enum::Add_Value( Ce_id i_nId )
{
    aValues.Add( i_nId );
}

const String  &
Enum::inq_LocalName() const
{
    return aEssentials.LocalName();
}

Cid
Enum::inq_Owner() const
{
    return aEssentials.Owner();
}

Lid
Enum::inq_Location() const
{
    return aEssentials.Location();
}

void
Enum::do_Accept(csv::ProcessorIfc & io_processor) const
{
    csv::CheckedCall(io_processor,*this);
}

ClassId
Enum::get_AryClass() const
{
    return class_id;
}

Gid
Enum::inq_Id_Group() const
{
    return static_cast<Gid>(Id());
}

const ary::cpp::CppEntity &
Enum::inq_RE_Group() const
{
    return *this;
}

const group::SlotList &
Enum::inq_Slots() const
{
    static const SlotAccessId aProjectSlotData[]
            = { SLOT_Values };
    static const std::vector< SlotAccessId >
            aSlots( &aProjectSlotData[0],
                      &aProjectSlotData[0]
                        + sizeof aProjectSlotData / sizeof (SlotAccessId) );
    return aSlots;
}

DYN Slot *
Enum::inq_Create_Slot( SlotAccessId  i_nSlot ) const
{
    switch ( i_nSlot )
    {
        case SLOT_Values:               return new Slot_SequentialIds<Ce_id>(aValues);
        default:
                                        return new Slot_Null;
    }   // end switch
}



}   // namespace cpp
}   // namespace ary
