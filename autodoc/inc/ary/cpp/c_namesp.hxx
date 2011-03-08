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

#ifndef ARY_CPP_C_NAMESP_HXX
#define ARY_CPP_C_NAMESP_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/c_ce.hxx>
#include <ary/arygroup.hxx>
    // OTHER
#include <ary/symtreenode.hxx>
#include <ary/cessentl.hxx>
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/cpp/c_slntry.hxx>

namespace ary
{
namespace cpp
{
    class Gate;
    class OperationSignature;
}
}



namespace ary
{
namespace cpp
{



/** A C++ namespace.
*/
class Namespace : public CodeEntity,
                  public AryGroup
{
  public:
    enum E_ClassId { class_id = 1000 };

    enum E_Slots
    {
        SLOT_SubNamespaces = 1,
        SLOT_Classes,
        SLOT_Enums,
        SLOT_Typedefs,
        SLOT_Operations,
        SLOT_Variables,
        SLOT_Constants
    };

    typedef ::ary::symtree::Node<CeNode_Traits>    node_t;

                        Namespace();
                        Namespace(
                            const String  &     i_sName,
                            Namespace &         i_rParent );
                        ~Namespace();
    // OPERATIONS
    void                Add_LocalNamespace(
                            Namespace &         io_rLocalNamespace );
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
    void                Add_LocalVariable(
                            const String  &     i_sLocalName,
                            Cid                 i_nId );
    void                Add_LocalConstant(
                            const String  &     i_sLocalName,
                            Cid                 i_nId );

    // INQUIRY
    virtual uintt       Depth() const;
    Namespace *         Parent() const;

    Ce_id               Search_Child(
                            const String &      i_key ) const;
    Namespace *         Search_LocalNamespace(
                            const String  &     i_sLocalName ) const;
    uintt               Get_SubNamespaces(
                            std::vector< const Namespace* > &
                                                o_rResultList ) const;
    Ce_id               Search_LocalClass(
                            const String &      i_sName ) const;
    void                Search_LocalOperations(
                            std::vector<Ce_id> &
                                                o_result,
                            const String &      i_sName ) const;
    const node_t &      AsNode() const;

    // ACCESS
    node_t &            AsNode();

  private:
    NON_COPYABLE(Namespace);

    // Interface csv::ConstProcessorClient
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;

    // Interface CodeEntity
    virtual const String  &
                        inq_LocalName() const;
    virtual Cid         inq_Owner() const;
    virtual Lid         inq_Location() const;

    // Interface ary::cpp::CppEntity
    virtual ClassId     get_AryClass() const;

        // Interface AryGroup
    virtual Gid         inq_Id_Group() const;
    virtual const cpp::CppEntity &
                        inq_RE_Group() const;
    virtual const ary::group::SlotList &
                        inq_Slots() const;
    virtual DYN Slot *  inq_Create_Slot(
                            SlotAccessId        i_nSlot ) const;
    // Local
    typedef std::multimap<String, Ce_id>  Map_Operations;

    // DATA
    CeEssentials        aEssentials;
    node_t              aAssignedNode;

    Map_NamespacePtr    aLocalNamespaces;
    Map_LocalCe         aLocalClasses;
    Map_LocalCe         aLocalEnums;
    Map_LocalCe         aLocalTypedefs;
    Map_Operations      aLocalOperations;
    Map_LocalCe         aLocalVariables;
    Map_LocalCe         aLocalConstants;

    Namespace *         pParent;
    uintt               nDepth;
};



// IMPLEMENTATION
inline const Namespace::node_t &
Namespace::AsNode() const
{
    return aAssignedNode;
}

inline Namespace::node_t &
Namespace::AsNode()
{
    return aAssignedNode;
}




}   // namespace cpp
}   // ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
