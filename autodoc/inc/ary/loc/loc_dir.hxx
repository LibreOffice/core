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
