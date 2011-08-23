/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
    virtual			   	~AryGroup() {}

    // INQUIRY
    Gid 		        Id_Group() const;
    const cpp::CppEntity &
                        RE_Group() const;
    const group::SlotList &
                        Slots() const;
    DYN Slot *          Create_Slot(
                            SlotAccessId        i_nSlot ) const;


  private:
    virtual Gid 		inq_Id_Group() const = 0;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
