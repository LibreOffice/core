/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_class.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:57:00 $
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

#ifndef ARY_CPP_C_CLASS_HXX
#define ARY_CPP_C_CLASS_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/ce.hxx>
#include <ary/cpp/ca_type.hxx>
#include <ary/arygroup.hxx>
#include <ary/re.hxx>
    // COMPONENTS
#include <ary/cessentl.hxx>
#include <ary/cpp/c_idlist.hxx>
#include <ary/opertype.hxx>
#include <ary/cpp/c_etypes.hxx>
    // PARAMETERS
#include <ary/idlists.hxx>



namespace ary
{
namespace cpp
{
    class Enum;
    class Typedef;
    class Function;
    class Variable;



class Class : public CodeEntity,
              public AryGroup
{
  public:
    enum E_Slots
    {
        SLOT_Bases = 1,
        SLOT_NestedClasses,
        SLOT_Enums,
        SLOT_Typedefs,
        SLOT_Operations,
        SLOT_StaticOperations,
        SLOT_Data,
        SLOT_StaticData,
        SLOT_FriendClasses,
        SLOT_FriendOperations
    };

    // LIFECYCLE
                        Class();
                        Class(
                            Cid                 i_nId,
                            const udmstri &     i_sLocalName,
                            Cid                 i_nOwner,
                            E_Protection        i_eProtection,
                            Lid                 i_nFile,
                            E_ClassKey          i_eClassKey );
                        ~Class();

    // OPERATIONS
    void                Add_BaseClass(
                            const S_Classes_Base &
                                                i_rBaseClass );
    void                Add_TemplateParameterType(
                            const udmstri &     i_sLocalName,
                            Tid                 i_nIdAsType );
    void                Add_KnownDerivative(
                            Rid                 i_nId )
                                                { aKnownDerivatives.push_back(i_nId); }

    void                Add_LocalClass(
                            const udmstri &     i_sLocalName,
                            Cid                 i_nId );
    void                Add_LocalEnum(
                            const udmstri &     i_sLocalName,
                            Cid                 i_nId );
    void                Add_LocalTypedef(
                            const udmstri &     i_sLocalName,
                            Cid                 i_nId );
    void                Add_LocalOperation(
                            const udmstri &     i_sLocalName,
                            OSid                i_nOS,
                            Cid                 i_nId );
    void                Add_LocalStaticOperation(
                            const udmstri &     i_sLocalName,
                            OSid                i_nOS,
                            Cid                 i_nId );
    void                Add_LocalData(
                            const udmstri &     i_sLocalName,
                            Cid                 i_nId );
    void                Add_LocalStaticData(
                            const udmstri &     i_sLocalName,
                            Cid                 i_nId );

    void                UpdateVirtuality(
                            E_Virtuality        i_eVirtuality )
                                                { if ( int(i_eVirtuality) > int(eVirtuality) )
                                                     eVirtuality = i_eVirtuality; }
    const List_TplParam &
                        TemplateParameters() const
                                                { return aTemplateParameterTypes; }
    const List_Bases &  BaseClasses() const     { return aBaseClasses; }
    const List_Rid &    KnownDerivatives() const
                                                { return aKnownDerivatives; }

    // INQUIRY
    static RCid         RC_()                   { return 0x1002; }

    E_ClassKey          ClassKey() const;
    E_Protection        Protection() const;
    E_Virtuality        Virtuality() const      { return eVirtuality; }

    Rid                 Search_LocalClass(
                            const udmstri &     i_sName ) const;

  private:
    // Interface ary::CodeEntity
    virtual Cid         inq_Id() const;
    virtual const udmstri &
                        inq_LocalName() const;
    virtual Cid         inq_Owner() const;
    virtual Lid         inq_Location() const;

    // Interface ary::RepositoryEntity
    virtual void        do_StoreAt(
                            ary::Display &      o_rOut ) const;
    virtual RCid        inq_RC() const;
    virtual const ary::Documentation &
                        inq_Info() const;
    virtual void        do_Add_Documentation(
                            DYN ary::Documentation &
                                                let_drInfo );
    // Interface ary::AryGroup
    virtual Gid         inq_Id_Group() const;
    virtual const RepositoryEntity &
                        inq_RE_Group() const;
    virtual const group::SlotList &
                        inq_Slots() const;
    virtual DYN Slot *  inq_Create_Slot(
                            SlotAccessId        i_nSlot ) const;
    // Local
     typedef List_LocalCe::const_iterator        CIterator_Locals;
     typedef List_LocalCe::iterator              Iterator_Locals;

    CIterator_Locals    PosOfName(
                            const List_LocalCe& i_rList,
                            const udmstri &     i_sName ) const;
    Iterator_Locals     PosOfName(
                            List_LocalCe &      io_rList,
                            const udmstri &     i_sName );
    // DATA
    CeEssentials        aEssentials;

    List_Bases          aBaseClasses;
    List_TplParam       aTemplateParameterTypes;

    List_LocalCe        aClasses;
    List_LocalCe        aEnums;
    List_LocalCe        aTypedefs;
    List_LocalOperation aOperations;
    List_LocalOperation aStaticOperations;
    List_LocalCe        aData;
    List_LocalCe        aStaticData;
    List_Rid            aFriendClasses;
    List_Rid            aFriendOperations;
    List_Rid            aKnownDerivatives;

    E_ClassKey          eClassKey;
    E_Protection        eProtection;
    E_Virtuality        eVirtuality;
};



// IMPLEMENTATION

inline E_ClassKey
Class::ClassKey() const
    { return eClassKey; }
inline E_Protection
Class::Protection() const
    { return eProtection; }


}   //  namespace cpp
}   //  namespace ary


#endif

