/*************************************************************************
 *
 *  $RCSfile: c_enum.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <precomp.h>
#include <ary/cpp/c_enum.hxx>


// NOT FULLY DECLARED SERVICES
#include "rcids.hxx"
#include <ary/cpp/cpp_disp.hxx>
#include <slots.hxx>
#include "c_slots.hxx"




namespace ary
{
namespace cpp
{


Enum::Enum()
    :   // aEssentials,
        // aValues,
        eProtection(PROTECT_global)
{
}

Enum::Enum( Cid                 i_nId,
            const udmstri &     i_sLocalName,
            Cid                 i_nOwner,
            E_Protection        i_eProtection,
            Lid                 i_nFile )
    :   aEssentials( i_nId,
                     i_sLocalName,
                     i_nOwner,
                     i_nFile ),
        // aValues
        eProtection(i_eProtection)
{

}

Enum::~Enum()
{
}

void
Enum::Add_Value( Cid i_nId )
{
    aValues.push_back( i_nId );
}

Cid
Enum::inq_Id() const
{
    return aEssentials.Id();
}

const udmstri &
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
Enum::do_StoreAt( ary::Display & o_rOut ) const
{
    ary::cpp::Display *  pD = dynamic_cast< ary::cpp::Display* >(&o_rOut);
    if (pD != 0)
    {
         pD->Display_Enum(*this);
    }
}

RCid
Enum::inq_RC() const
{
    return RC_();
}


const ary::Documentation &
Enum::inq_Info() const
{
    return aEssentials.Info();
}

void
Enum::do_Add_Documentation( DYN ary::Documentation & let_drInfo )
{
    aEssentials.SetInfo(let_drInfo);
}

Gid
Enum::inq_Id_Group() const
{
    return static_cast<Gid>(Id());
}

const RepositoryEntity &
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
        case SLOT_Values:               return new Slot_RidList(aValues);
        default:
                                        return new Slot_Null;
    }   // end switch
}



}   // namespace cpp
}   // namespace ary



