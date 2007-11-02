/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: arygroup.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:37:17 $
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

#ifndef ARY_ARYGROUP_HXX
#define ARY_ARYGROUP_HXX
//  KORR_DEPRECATED_3.0


// USED SERVICES
    // BASE CLASSES
    // OTHER
#include <ary/types.hxx>


namespace ary
{
namespace cpp
{
    class CppEntity;
}

class Slot;

namespace group
{
    typedef std::vector< SlotAccessId > SlotList;
}
}




namespace ary
{



class AryGroup
{
  public:
    // LIFECYCLE
    virtual             ~AryGroup() {}

    // INQUIRY
    Gid                 Id_Group() const;
    const cpp::CppEntity &
                        RE_Group() const;
    const group::SlotList &
                        Slots() const;
    DYN Slot *          Create_Slot(
                            SlotAccessId        i_nSlot ) const;


  private:
    virtual Gid         inq_Id_Group() const = 0;
    virtual const cpp::CppEntity &
                        inq_RE_Group() const = 0;
    virtual const group::SlotList &
                        inq_Slots() const = 0;
    virtual DYN Slot *  inq_Create_Slot(
                            SlotAccessId        i_nSlot ) const = 0;
};



// IMPLEMENTATION
inline Gid
AryGroup::Id_Group() const
    { return inq_Id_Group(); }
inline const cpp::CppEntity &
AryGroup::RE_Group() const
    { return inq_RE_Group(); }
inline const group::SlotList &
AryGroup::Slots() const
    { return inq_Slots(); }
inline DYN Slot *
AryGroup::Create_Slot( SlotAccessId i_nSlot ) const
    { return inq_Create_Slot(i_nSlot); }



}   // namespace ary
#endif
