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

#include <precomp.h>
#include <ary/cpp/c_class.hxx>


// NOT FULLY DECLARED SERVICES
#include <slots.hxx>
#include "c_slots.hxx"



namespace ary
{
namespace cpp
{

Class::Class( const String  &     i_sLocalName,
              Ce_id               i_nOwner,
              E_Protection        i_eProtection,
              loc::Le_id          i_nFile,
              E_ClassKey          i_eClassKey )
    :   aEssentials( i_sLocalName,
                     i_nOwner,
                     i_nFile ),
        aAssignedNode(),
           aBaseClasses(),
           aTemplateParameterTypes(),
        aClasses(),
        aEnums(),
        aTypedefs(),
        aOperations(),
        aStaticOperations(),
        aData(),
        aStaticData(),
        aFriendClasses(),
        aFriendOperations(),
        aKnownDerivatives(),
        eClassKey(i_eClassKey),
        eProtection(i_eProtection),
        eVirtuality(VIRTUAL_none)
{
    aAssignedNode.Assign_Entity(*this);
}

Class::~Class()
{
}

void
Class::Add_BaseClass( const S_Classes_Base & i_rBaseClass )
{
    aBaseClasses.push_back(i_rBaseClass);
}

void
Class::Add_TemplateParameterType( const String  &     i_sLocalName,
                                  Type_id             i_nIdAsType )
{
    aTemplateParameterTypes.push_back(
            List_TplParam::value_type(i_sLocalName,i_nIdAsType) );
}

void
Class::Add_LocalClass( const String  &     i_sLocalName,
                       Cid                 i_nId )
{
    aClasses.push_back( S_LocalCe(i_sLocalName, i_nId) );
}

void
Class::Add_LocalEnum( const String  &     i_sLocalName,
                      Cid                 i_nId )
{
    aEnums.push_back( S_LocalCe(i_sLocalName, i_nId) );
}

void
Class::Add_LocalTypedef( const String  &     i_sLocalName,
                         Cid                 i_nId )
{
    aTypedefs.push_back( S_LocalCe(i_sLocalName, i_nId) );
}

void
Class::Add_LocalOperation( const String  &          i_sLocalName,
                           Cid                      i_nId )
{
    aOperations.push_back( S_LocalCe(i_sLocalName, i_nId) );
}

void
Class::Add_LocalStaticOperation( const String  &     i_sLocalName,
                                 Cid                 i_nId )
{
    aStaticOperations.push_back( S_LocalCe(i_sLocalName, i_nId) );
}

void
Class::Add_LocalData( const String  &     i_sLocalName,
                      Cid                 i_nId )
{
    aData.push_back( S_LocalCe(i_sLocalName, i_nId) );
}

void
Class::Add_LocalStaticData( const String  &     i_sLocalName,
                            Cid                 i_nId )
{
    aStaticData.push_back( S_LocalCe(i_sLocalName, i_nId) );
}


struct find_name
{
                        find_name(
                            const String &      i_name )
                            :   sName(i_name) {}

    bool                operator()(
                            const S_LocalCe &   i_lce ) const
                            { return i_lce.sLocalName == sName; }
  private:
    String              sName;
};

Ce_id
Class::Search_Child(const String & i_key) const
{
    Ce_id
        ret = Ce_id(Search_LocalClass(i_key));
    if (ret.IsValid())
        return ret;

    CIterator_Locals
        itret = std::find_if(aEnums.begin(), aEnums.end(), find_name(i_key));
    if (itret != aEnums.end())
        return (*itret).nId;
    itret = std::find_if(aTypedefs.begin(), aTypedefs.end(), find_name(i_key));
    if (itret != aTypedefs.end())
        return (*itret).nId;
    itret = std::find_if(aData.begin(), aData.end(), find_name(i_key));
    if (itret != aData.end())
        return (*itret).nId;
    itret = std::find_if(aStaticData.begin(), aStaticData.end(), find_name(i_key));
    if (itret != aStaticData.end())
        return (*itret).nId;
    return Ce_id(0);
}

Rid
Class::Search_LocalClass( const String  & i_sName ) const
{
     CIterator_Locals itFound = PosOfName(aClasses, i_sName);
    if (itFound != aClasses.end())
        return (*itFound).nId.Value();
    return 0;
}

const String  &
Class::inq_LocalName() const
{
    return aEssentials.LocalName();
}

Cid
Class::inq_Owner() const
{
    return aEssentials.Owner();
}

loc::Le_id
Class::inq_Location() const
{
    return aEssentials.Location();
}

void
Class::do_Accept(csv::ProcessorIfc & io_processor) const
{
    csv::CheckedCall(io_processor,*this);
}

ClassId
Class::get_AryClass() const
{
    return class_id;
}

Gid
Class::inq_Id_Group() const
{
     return static_cast<Gid>(Id());
}

const ary::cpp::CppEntity &
Class::inq_RE_Group() const
{
     return *this;
}

const group::SlotList &
Class::inq_Slots() const
{
    static const SlotAccessId aProjectSlotData[]
            = { SLOT_Bases,
                SLOT_NestedClasses,
                SLOT_Enums,
                SLOT_Typedefs,
                SLOT_Operations,
                SLOT_StaticOperations,
                SLOT_Data,
                SLOT_StaticData,
                SLOT_FriendClasses,
                SLOT_FriendOperations };
    static const std::vector< SlotAccessId >
            aSlots( &aProjectSlotData[0],
                      &aProjectSlotData[0]
                        + sizeof aProjectSlotData / sizeof (SlotAccessId) );
    return aSlots;
}


DYN Slot *
Class::inq_Create_Slot( SlotAccessId i_nSlot ) const
{
    switch ( i_nSlot )
    {
        case SLOT_Bases:                return new Slot_BaseClass(aBaseClasses);
        case SLOT_NestedClasses:        return new Slot_ListLocalCe(aClasses);
        case SLOT_Enums:                return new Slot_ListLocalCe(aEnums);
        case SLOT_Typedefs:             return new Slot_ListLocalCe(aTypedefs);
        case SLOT_Operations:           return new Slot_ListLocalCe(aOperations);
        case SLOT_StaticOperations:     return new Slot_ListLocalCe(aStaticOperations);
        case SLOT_Data:                 return new Slot_ListLocalCe(aData);
        case SLOT_StaticData:           return new Slot_ListLocalCe(aStaticData);
        case SLOT_FriendClasses:        return new Slot_SequentialIds<Ce_id>(aFriendClasses);
        case SLOT_FriendOperations:     return new Slot_SequentialIds<Ce_id>(aFriendOperations);
        default:
                                        return new Slot_Null;
    }   // end switch
}

Class::CIterator_Locals
Class::PosOfName( const List_LocalCe &  i_rList,
                  const String  &       i_sName ) const
{
    for ( CIterator_Locals ret = i_rList.begin();
          ret != i_rList.end();
          ++ret )
    {
         if ( (*ret).sLocalName == i_sName )
            return ret;
    }
    return i_rList.end();
}

}   //  namespace   cpp
}   //  namespace   ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
