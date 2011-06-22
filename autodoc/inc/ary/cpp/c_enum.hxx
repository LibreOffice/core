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

#ifndef ARY_CPP_C_ENUM_HXX
#define ARY_CPP_C_ENUM_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/c_ce.hxx>
#include <ary/arygroup.hxx>
    // OTHER
#include <ary/cessentl.hxx>
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/sequentialids.hxx>





namespace ary
{
namespace cpp
{


/** A C++ enum declaration.
*/
class Enum : public CodeEntity,
             public AryGroup
{
  public:
    enum E_ClassId { class_id = 1002 };

    enum E_Slots
    {
        SLOT_Values = 1
    };

    // LIFECYCLE
                        Enum(
                            const String  &     i_sLocalName,
                            Ce_id               i_nOwner,
                            E_Protection        i_eProtection,
                            Lid                 i_nFile );
                        ~Enum();

    // OPERATIONS
    void                Add_Value(
                            Ce_id               i_nId );

    // INQUIRY
    E_Protection        Protection() const      { return eProtection; }

  private:
    // Interface csv::ConstProcessorClient
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;

    // Interface ary::cpp::CodeEntity
    virtual const String  &
                        inq_LocalName() const;
    virtual Cid         inq_Owner() const;
    virtual Lid         inq_Location() const;

    // Interface ary::cpp::CppEntity
    virtual ClassId     get_AryClass() const;

    // Interface ary::AryGroup
    virtual Gid         inq_Id_Group() const;
    virtual const cpp::CppEntity &
                        inq_RE_Group() const;
    virtual const group::SlotList &
                        inq_Slots() const;
    virtual DYN Slot *  inq_Create_Slot(
                            SlotAccessId        i_nSlot ) const;

    // DATA
       CeEssentials     aEssentials;
    SequentialIds<Ce_id>
                        aValues;
    E_Protection        eProtection;
};





}   // namespace cpp
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
