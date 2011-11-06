/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
