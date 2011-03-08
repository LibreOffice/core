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

#ifndef ARY_LOC_DIR_HXX
#define ARY_LOC_DIR_HXX

// BASE CLASSES
#include <ary/loc/loc_le.hxx>

// USED SERVICES
#include <ary/loc/loc_traits.hxx>
#include <ary/symtreenode.hxx>

namespace ary
{
namespace loc
{
    class File;
}
}




namespace ary
{
namespace loc
{


/** Represents a directory for source code files.
*/
class Directory : public LocationEntity
{
  public:
    enum E_ClassId { class_id = 7030 };

    typedef ::ary::symtree::Node<LeNode_Traits>    node_t;

    /// Used for root directories.
    explicit            Directory(
                            Le_id               i_assignedRoot );

    /// Used for subdirectories which have a parent directory.
                        Directory(
                            const String  &     i_localName,
                            Le_id               i_parentDirectory );
    virtual             ~Directory();

    void                Add_Dir(
                            const Directory  &  i_dir );
    void                Add_File(
                            const File &        i_file );

    Le_id               Parent() const;
    Le_id               AssignedRoot() const;

    Le_id               Search_Dir(
                            const String &      i_name ) const;
    Le_id               Search_File(
                            const String &      i_name ) const;

    const node_t &      AsNode() const;
    node_t &            AsNode();

  private:
    struct Container;

    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // Interface ary::Object:
    virtual ClassId     get_AryClass() const;

    // Interface LocationEntity:
    virtual const String &
                        inq_LocalName() const;
    virtual Le_id       inq_ParentDirectory() const;

    // DATA
    String              sLocalName;
    Le_id               nParentDirectory;
    Le_id               nAssignedRoot;
    node_t              aAssignedNode;
    Dyn<Container>      pChildren;
};




// IMPLEMENTATION
inline Le_id
Directory::Parent() const
{
    return nParentDirectory;
}

inline Le_id
Directory::AssignedRoot() const
{
    return nAssignedRoot;
}

inline const Directory::node_t &
Directory::AsNode() const
{
    return aAssignedNode;
}

inline Directory::node_t &
Directory::AsNode()
{
    return aAssignedNode;
}




} // namespace loc
} // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
