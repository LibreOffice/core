/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_enum.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:24:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


Enum::Enum()
    :   aEssentials(),
        aValues(),
        eProtection(PROTECT_global)
{
}

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
