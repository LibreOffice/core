/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: loc_dir.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:17:04 $
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

    Le_id               Search_Child(
                            const String &      i_name ) const;
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
