/*************************************************************************
 *
 *  $RCSfile: c_namesp.cxx,v $
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
#include <ary/cpp/c_namesp.hxx>


// NOT FULLY DECLARED SERVICES
#include <cosv/template/tpltools.hxx>
#include "rcids.hxx"
#include <ary/cpp/cpp_disp.hxx>
#include <slots.hxx>
#include "c_slots.hxx"


namespace ary
{
namespace cpp
{



Namespace::Namespace()
    :   // aEssentials,
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
}

Namespace::Namespace( Rid i_nId )
    :   aEssentials( i_nId, udmstri::Null_(), 0, 0 ),
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
}

Namespace::Namespace( Cid                 i_nId,
                      const udmstri &     i_sLocalName,
                      Namespace &         i_rParent )
    :   aEssentials( i_nId,
                     i_sLocalName,
                     i_rParent.Id(),
                     0 ),
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
Namespace::Add_LocalClass( const udmstri &     i_sLocalName,
                           Cid                 i_nId )
{
    aLocalClasses[i_sLocalName] = i_nId;
}

void
Namespace::Add_LocalEnum( const udmstri &     i_sLocalName,
                          Cid                 i_nId )
{
    aLocalEnums[i_sLocalName] = i_nId;
}

void
Namespace::Add_LocalTypedef( const udmstri &     i_sLocalName,
                             Cid                 i_nId )
{
    aLocalTypedefs[i_sLocalName] = i_nId;
}

void
Namespace::Add_LocalOperation( const udmstri &     i_sLocalName,
                               OSid                i_nOS,
                               Cid                 i_nId )
{
    aLocalOperations.insert(
            S_LocalOperation(i_sLocalName, i_nOS, i_nId) );
}


void
Namespace::Add_LocalVariable( const udmstri &     i_sLocalName,
                              Cid                 i_nId )
{
    aLocalVariables[i_sLocalName] = i_nId;
}

void
Namespace::Add_LocalConstant( const udmstri &     i_sLocalName,
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

Namespace *
Namespace::Search_LocalNamespace( const udmstri & i_sLocalName ) const
{
    return csv::value_from_map(aLocalNamespaces, i_sLocalName);
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

Rid
Namespace::Search_LocalClass( const udmstri & i_sName ) const
{
    return csv::value_from_map(aLocalClasses, i_sName);
}

Cid
Namespace::inq_Id() const
{
    return aEssentials.Id();
}

const udmstri &
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
    return 0;
}

void
Namespace::do_StoreAt( ary::Display & o_rOut ) const
{
    ary::cpp::Display *  pD = dynamic_cast< ary::cpp::Display* >(&o_rOut);
    if (pD != 0)
    {
        pD->Display_Namespace(*this);
    }
}

RCid
Namespace::inq_RC() const
{
    return RC_();
}


const ary::Documentation &
Namespace::inq_Info() const
{
    return aEssentials.Info();
}

void
Namespace::do_Add_Documentation( DYN ary::Documentation & let_drInfo )
{
    aEssentials.SetInfo(let_drInfo);
}

Gid
Namespace::inq_Id_Group() const
{
    return static_cast<Gid>(aEssentials.Id());
}

const RepositoryEntity &
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
        case SLOT_Operations:       return new Slot_OperationSet(aLocalOperations);
        case SLOT_Variables:        return new Slot_MapLocalCe(aLocalVariables);
        case SLOT_Constants:        return new Slot_MapLocalCe(aLocalConstants);
        default:
                                    return new Slot_Null;
    }   // end switch
}


}   // namespace cpp
}   // namespace ary



#if 0
/*
uintt
Namespace::Get_LocalNamespaces( ary::List_Cid& o_rResultList ) const
{
    for ( Map_NamespacePtr::const_iterator it = aLocalNamespaces.begin();
          it != aLocalNamespaces.end();
          ++it )
    {
        o_rResultList.push_back((*it).second->Id());
    }
    return o_rResultList.size();
}

uintt
Namespace::Get_LocalClasses( ary::List_Cid& o_rResultList ) const
{
    for ( Map_LocalCe::const_iterator it = aLocalClasses.begin();
          it != aLocalClasses.end();
          ++it )
    {
        o_rResultList.push_back((*it).second);
    }
    return o_rResultList.size();
}

uintt
Namespace::Get_LocalFunctions( ary::List_Cid & o_rResultList ) const
{
    for ( Set_LocalOperation::const_iterator it = aLocalOperations.begin();
          it != aLocalOperations.end();
          ++it )
    {
        o_rResultList.push_back((*it).nId);
    }
    return o_rResultList.size();
}
*/
#endif // 0


