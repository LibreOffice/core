/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_class.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:29:57 $
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
#include <ary/cpp/c_class.hxx>


// NOT FULLY DECLARED SERVICES
#include "rcids.hxx"
#include <ary/cpp/cpp_disp.hxx>
#include <slots.hxx>
#include "c_slots.hxx"




namespace ary
{
namespace cpp
{

Class::Class()
    :   // aEssentials,
           // aBaseClasses,
        // aClasses,
        // aOperations,
        // aData,
        eClassKey(CK_class),
        eProtection(PROTECT_global),
        eVirtuality(VIRTUAL_none)
{
}

Class::Class( Cid                 i_nId,
              const udmstri &     i_sLocalName,
              Cid                 i_nOwner,
              E_Protection        i_eProtection,
              Lid                 i_nFile,
              E_ClassKey          i_eClassKey )
    :   aEssentials( i_nId,
                     i_sLocalName,
                     i_nOwner,
                     i_nFile ),
           // aBaseClasses,
        // aClasses,
        // aOperations,
        // aData,
        eClassKey(i_eClassKey),
        eProtection(i_eProtection),
        eVirtuality(VIRTUAL_none)
{
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
Class::Add_TemplateParameterType( const udmstri &     i_sLocalName,
                                  Tid                 i_nIdAsType )
{
    aTemplateParameterTypes.push_back(
            List_TplParam::value_type(i_sLocalName,i_nIdAsType) );
}

void
Class::Add_LocalClass( const udmstri &     i_sLocalName,
                       Cid                 i_nId )
{
    aClasses.push_back( S_LocalCe(i_sLocalName, i_nId) );
}

void
Class::Add_LocalEnum( const udmstri &     i_sLocalName,
                      Cid                 i_nId )
{
    aEnums.push_back( S_LocalCe(i_sLocalName, i_nId) );
}

void
Class::Add_LocalTypedef( const udmstri &     i_sLocalName,
                         Cid                 i_nId )
{
    aTypedefs.push_back( S_LocalCe(i_sLocalName, i_nId) );
}

void
Class::Add_LocalOperation( const udmstri &          i_sLocalName,
                           OSid                     i_nOS,
                           Cid                      i_nId )
{
    aOperations.push_back( S_LocalOperation(i_sLocalName, i_nOS, i_nId) );
}

void
Class::Add_LocalStaticOperation( const udmstri &     i_sLocalName,
                                 OSid                i_nOS,
                                 Cid                 i_nId )
{
    aStaticOperations.push_back( S_LocalOperation(i_sLocalName, i_nOS, i_nId) );
}

void
Class::Add_LocalData( const udmstri &     i_sLocalName,
                      Cid                 i_nId )
{
    aData.push_back( S_LocalCe(i_sLocalName, i_nId) );
}

void
Class::Add_LocalStaticData( const udmstri &     i_sLocalName,
                            Cid                 i_nId )
{
    aStaticData.push_back( S_LocalCe(i_sLocalName, i_nId) );
}

Rid
Class::Search_LocalClass( const udmstri & i_sName ) const
{
     CIterator_Locals itFound = PosOfName(aClasses, i_sName);
    if (itFound != aClasses.end())
        return (*itFound).nId;
    return 0;
}

Cid
Class::inq_Id() const
{
    return aEssentials.Id();
}

const udmstri &
Class::inq_LocalName() const
{
    return aEssentials.LocalName();
}

Cid
Class::inq_Owner() const
{
    return aEssentials.Owner();
}

Lid
Class::inq_Location() const
{
    return aEssentials.Location();
}

void
Class::do_StoreAt( ary::Display & o_rOut ) const
{
    ary::cpp::Display *  pD = dynamic_cast< ary::cpp::Display* >(&o_rOut);
    if (pD != 0)
    {
         pD->Display_Class(*this);
    }
}

RCid
Class::inq_RC() const
{
    return RC_();
}


const ary::Documentation &
Class::inq_Info() const
{
    return aEssentials.Info();
}

void
Class::do_Add_Documentation( DYN ary::Documentation & let_drInfo )
{
    aEssentials.SetInfo(let_drInfo);
}

Gid
Class::inq_Id_Group() const
{
     return static_cast<Gid>(aEssentials.Id());
}

const RepositoryEntity &
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
        case SLOT_Operations:           return new Slot_OperationList(aOperations);
        case SLOT_StaticOperations:     return new Slot_OperationList(aStaticOperations);
        case SLOT_Data:                 return new Slot_ListLocalCe(aData);
        case SLOT_StaticData:           return new Slot_ListLocalCe(aStaticData);
        case SLOT_FriendClasses:        return new Slot_RidList(aFriendClasses);
        case SLOT_FriendOperations:     return new Slot_RidList(aFriendOperations);
        default:
                                        return new Slot_Null;
    }   // end switch
}


#if 0
uintt
Class::Get_LocalClasses( ary::List_Rid & o_rResultList ) const
{
    for ( CIterator_Locals it = aClasses.begin();
          it != aClasses.end();
          ++it )
    {
        o_rResultList.push_back((*it).nId);
    }
    return o_rResultList.size();
}

uintt
Class::Get_LocalFunctions( ary::List_Rid & o_rResultList ) const
{
    for ( List_LocalOperation::const_iterator its = aStaticOperations.begin();
          its != aStaticOperations.end();
          ++its )
    {
        o_rResultList.push_back((*its).nId);
    }
    for ( List_LocalOperation::const_iterator it = aOperations.begin();
          it != aOperations.end();
          ++it )
    {
        o_rResultList.push_back((*it).nId);
    }
    return o_rResultList.size();
}

const udmstri &
Class::LocalNameOfOwner() const
{
     return LocalName();
}
#endif // 0

#if 0
Cid
Class::Find_LocalClass( const udmstri & i_sLocalName ) const
{
    CIterator_Locals ret = PosOfName(aClasses, i_sLocalName);
    if (ret != aTypes.end())
        return static_cast<Cid>( (*ret).nId );
    return 0;
}

Tid
Class::Find_LocalType( const udmstri & i_sLocalName ) const
{
    CIterator_Locals ret = PosOfName(aEnums, i_sLocalName);
    if (ret != aEnums.end())
        return (*ret).nId;
    ret = PosOfName(aTypedefs, i_sLocalName);
    if (ret != aTypedefs.end())
        return (*ret).nId;
    ret = PosOfName(aClasses, i_sLocalName);
    if (ret != aClasses.end())
        return (*ret).nId;
    return 0;
}
#endif // 0


Class::CIterator_Locals
Class::PosOfName( const List_LocalCe &  i_rList,
                  const udmstri &       i_sName ) const
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

Class::Iterator_Locals
Class::PosOfName( List_LocalCe &    i_rList,
                  const udmstri &   i_sName )
{
    for ( Iterator_Locals ret = i_rList.begin();
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



