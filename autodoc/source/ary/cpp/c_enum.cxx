/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
