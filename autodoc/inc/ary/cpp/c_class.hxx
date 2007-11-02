/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_class.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:46:14 $
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
#include <ary/cpp/c_ce.hxx>
#include <ary/arygroup.hxx>
    // OTHER
#include <ary/symtreenode.hxx>
#include <ary/cessentl.hxx>
#include <ary/sequentialids.hxx>
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/cpp/c_slntry.hxx>

namespace ary
{
namespace cpp
{
    class Enum;
    class Typedef;
    class Function;
    class Variable;
}
}



namespace ary
{
namespace cpp
{


/** A C++ class.
*/
class Class : public CodeEntity,
              public AryGroup
{
  public:
    enum E_ClassId { class_id = 1001 };

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

    typedef ::ary::symtree::Node<CeNode_Traits>    node_t;


    // LIFECYCLE
                        Class();
                        Class(
                            const String  &     i_sLocalName,
                            Ce_id               i_nOwner,
                            E_Protection        i_eProtection,
                            loc::Le_id          i_nFile,
                            E_ClassKey          i_eClassKey );
                        ~Class();

    // OPERATIONS
    void                Add_BaseClass(
                            const S_Classes_Base &
                                                i_rBaseClass );
    void                Add_TemplateParameterType(
                            const String  &     i_sLocalName,
                            Type_id             i_nIdAsType );
    void                Add_KnownDerivative(
                            Ce_id               i_nId )
                                                { aKnownDerivatives.Add(i_nId); }

    void                Add_LocalClass(
                            const String  &     i_sLocalName,
                            Cid                 i_nId );
    void                Add_LocalEnum(
                            const String  &     i_sLocalName,
                            Cid                 i_nId );
    void                Add_LocalTypedef(
                            const String  &     i_sLocalName,
                            Cid                 i_nId );
    void                Add_LocalOperation(
                            const String  &     i_sLocalName,
                            Cid                 i_nId );
    void                Add_LocalStaticOperation(
                            const String  &     i_sLocalName,
                            Cid                 i_nId );
    void                Add_LocalData(
                            const String  &     i_sLocalName,
                            Cid                 i_nId );
    void                Add_LocalStaticData(
                            const String  &     i_sLocalName,
                            Cid                 i_nId );

    void                UpdateVirtuality(
                            E_Virtuality        i_eVirtuality )
                                                { if ( int(i_eVirtuality) > int(eVirtuality) )
                                                     eVirtuality = i_eVirtuality; }
    const List_TplParam &
                        TemplateParameters() const
                                                { return aTemplateParameterTypes; }
    const List_Bases &  BaseClasses() const     { return aBaseClasses; }
    const SequentialIds<Ce_id> &
                        KnownDerivatives() const
                                                { return aKnownDerivatives; }

    // INQUIRY
    E_ClassKey          ClassKey() const;
    E_Protection        Protection() const;
    E_Virtuality        Virtuality() const      { return eVirtuality; }

    Ce_id               Search_Child(
                            const String &      i_key ) const;
    Rid                 Search_LocalClass(
                            const String  &     i_sName ) const;
    const node_t &      AsNode() const;

    // ACCESS
    node_t &            AsNode();

  private:
    NON_COPYABLE(Class);

    // Interface csv::ConstProcessorClient
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;

    // Interface ary::cpp::CodeEntity
    virtual const String  &
                        inq_LocalName() const;
    virtual Cid         inq_Owner() const;
    virtual loc::Le_id  inq_Location() const;

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
    // Local
     typedef List_LocalCe::const_iterator        CIterator_Locals;
     typedef List_LocalCe::iterator              Iterator_Locals;
    typedef SequentialIds<Ce_id>                IdSequence;

    CIterator_Locals    PosOfName(
                            const List_LocalCe& i_rList,
                            const String  &     i_sName ) const;
    Iterator_Locals     PosOfName(
                            List_LocalCe &      io_rList,
                            const String  &     i_sName );
    // DATA
    CeEssentials        aEssentials;
    node_t              aAssignedNode;

    List_Bases          aBaseClasses;
    List_TplParam       aTemplateParameterTypes;

    List_LocalCe        aClasses;
    List_LocalCe        aEnums;
    List_LocalCe        aTypedefs;
    List_LocalCe        aOperations;
    List_LocalCe        aStaticOperations;
    List_LocalCe        aData;
    List_LocalCe        aStaticData;

    IdSequence          aFriendClasses;
    IdSequence          aFriendOperations;
    IdSequence          aKnownDerivatives;

    E_ClassKey          eClassKey;
    E_Protection        eProtection;
    E_Virtuality        eVirtuality;
};




// IMPLEMENTATION
inline E_ClassKey
Class::ClassKey() const
{
    return eClassKey;
}

inline E_Protection
Class::Protection() const
{
    return eProtection;
}

inline const Class::node_t &
Class::AsNode() const
{
    return aAssignedNode;
}

inline Class::node_t &
Class::AsNode()
{
    return aAssignedNode;
}




}   //  namespace cpp
}   //  namespace ary
#endif
