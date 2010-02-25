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
#include <ary/cpp/c_namesp.hxx>


// NOT FULLY DECLARED SERVICES
#include <algorithm>
#include <cosv/tpl/tpltools.hxx>
#include <ary/cpp/c_funct.hxx>
#include <ary/cpp/c_gate.hxx>
#include <ary/getncast.hxx>
#include <slots.hxx>
#include "c_slots.hxx"


namespace ary
{
namespace cpp
{

typedef std::multimap<String, Ce_id>::const_iterator      operations_citer;

Namespace::Namespace()
    :   aEssentials(),
        aAssignedNode(),
        // aLocalNamespaces,
        // aLocalClasses,
        // aLocalEnums,
        // aLocalTypedefs,
        // aLocalOperations,
        // aLocalVariables,
        // aLocalConstants,
        pParent(0),
        nDepth(0)
{
    aAssignedNode.Assign_Entity(*this);
}

Namespace::Namespace( const String  &     i_sLocalName,
                      Namespace &         i_rParent )
    :   aEssentials( i_sLocalName,
                     i_rParent.CeId(),
                     Lid(0) ),
        aAssignedNode(),
        // aLocalNamespaces,
        // aLocalClasses,
        // aLocalEnums,
        // aLocalTypedefs,
        // aLocalOperations,
        // aLocalVariables,
        // aLocalConstants,
        pParent(&i_rParent),
        nDepth(i_rParent.Depth()+1)
{
    aAssignedNode.Assign_Entity(*this);
}

Namespace::~Namespace()
{
}

void
Namespace::Add_LocalNamespace( DYN Namespace & io_rLocalNamespace )
{
    aLocalNamespaces[io_rLocalNamespace.LocalName()] = &io_rLocalNamespace;
}

void
Namespace::Add_LocalClass( const String  &     i_sLocalName,
                           Cid                 i_nId )
{
    aLocalClasses[i_sLocalName] = i_nId;
}

void
Namespace::Add_LocalEnum( const String  &     i_sLocalName,
                          Cid                 i_nId )
{
    aLocalEnums[i_sLocalName] = i_nId;
}

void
Namespace::Add_LocalTypedef( const String  &     i_sLocalName,
                             Cid                 i_nId )
{
    aLocalTypedefs[i_sLocalName] = i_nId;
}

void
Namespace::Add_LocalOperation( const String  &     i_sLocalName,
                               Cid                 i_nId )
{
    aLocalOperations.insert( Map_Operations::value_type(i_sLocalName, i_nId) );
}


void
Namespace::Add_LocalVariable( const String  &     i_sLocalName,
                              Cid                 i_nId )
{
    aLocalVariables[i_sLocalName] = i_nId;
}

void
Namespace::Add_LocalConstant( const String  &     i_sLocalName,
                              Cid                 i_nId )
{
    aLocalConstants[i_sLocalName] = i_nId;
}

uintt
Namespace::Depth() const
{
    return nDepth;
}

Namespace *
Namespace::Parent() const
{
    return pParent;
}

Ce_id
Namespace::Search_Child(const String & i_key) const
{
    Namespace *
        ret_nsp = Search_LocalNamespace(i_key);
    if (ret_nsp != 0)
        return ret_nsp->CeId();

    Ce_id
        ret = Search_LocalClass(i_key);
    if (ret.IsValid())
        return ret;

    ret = csv::value_from_map(aLocalEnums, i_key, Ce_id(0));
    if (ret.IsValid())
        return ret;
    ret = csv::value_from_map(aLocalTypedefs, i_key, Ce_id(0));
    if (ret.IsValid())
        return ret;
    ret = csv::value_from_map(aLocalVariables, i_key, Ce_id(0));
    if (ret.IsValid())
        return ret;
    return csv::value_from_map(aLocalConstants, i_key, Ce_id(0));
}

Namespace *
Namespace::Search_LocalNamespace( const String  & i_sLocalName ) const
{
    return csv::value_from_map(aLocalNamespaces, i_sLocalName, (Namespace*)(0));
}

uintt
Namespace::Get_SubNamespaces( std::vector< const Namespace* > & o_rResultList ) const
{
    for ( Map_NamespacePtr::const_iterator it = aLocalNamespaces.begin();
          it != aLocalNamespaces.end();
          ++it )
    {
        o_rResultList.push_back( (*it).second );
    }
    return o_rResultList.size();
}

Ce_id
Namespace::Search_LocalClass( const String  & i_sName ) const
{
    return csv::value_from_map(aLocalClasses, i_sName, Ce_id(0));
}

void
Namespace::Search_LocalOperations( std::vector<Ce_id> & o_result,
                                   const String &       i_sName ) const
{
    operations_citer
        itLower = aLocalOperations.lower_bound(i_sName);
    if (itLower == aLocalOperations.end())
        return;
    if ( (*itLower).first != i_sName )
        return;

    operations_citer
        itEnd = aLocalOperations.end();
    for ( operations_citer it = itLower;
          it != aLocalOperations.end() ? (*itLower).first == i_sName : false;
          ++it )
    {
        o_result.push_back((*it).second);
    }
}


const String  &
Namespace::inq_LocalName() const
{
    return aEssentials.LocalName();
}

Cid
Namespace::inq_Owner() const
{
    return aEssentials.Owner();
}

Lid
Namespace::inq_Location() const
{
    return Lid(0);
}

void
Namespace::do_Accept(csv::ProcessorIfc & io_processor) const
{
    csv::CheckedCall(io_processor,*this);
}

ClassId
Namespace::get_AryClass() const
{
    return class_id;
}

Gid
Namespace::inq_Id_Group() const
{
    return static_cast<Gid>(Id());
}

const ary::cpp::CppEntity &
Namespace::inq_RE_Group() const
{
    return *this;
}

const ary::group::SlotList &
Namespace::inq_Slots() const
{
    static const SlotAccessId aProjectSlotData[]
            = { SLOT_SubNamespaces, SLOT_Classes, SLOT_Enums, SLOT_Typedefs, SLOT_Operations,
                SLOT_Variables, SLOT_Constants };
    static const std::vector< SlotAccessId >
            aSlots( &aProjectSlotData[0],
                      &aProjectSlotData[0]
                        + sizeof aProjectSlotData / sizeof (SlotAccessId) );
    return aSlots;
}

DYN Slot *
Namespace::inq_Create_Slot( SlotAccessId i_nSlot ) const
{
    switch ( i_nSlot )
    {
        case SLOT_SubNamespaces:    return new Slot_SubNamespaces(aLocalNamespaces);
        case SLOT_Classes:          return new Slot_MapLocalCe(aLocalClasses);
        case SLOT_Enums:            return new Slot_MapLocalCe(aLocalEnums);
        case SLOT_Typedefs:         return new Slot_MapLocalCe(aLocalTypedefs);
        case SLOT_Operations:       return new Slot_MapOperations(aLocalOperations);
        case SLOT_Variables:        return new Slot_MapLocalCe(aLocalVariables);
        case SLOT_Constants:        return new Slot_MapLocalCe(aLocalConstants);
        default:
                                    return new Slot_Null;
    }   // end switch
}


}   // namespace cpp
}   // namespace ary
