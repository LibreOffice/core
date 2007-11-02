/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: loc_dir.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:16:02 $
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
#include <ary/loc/loc_dir.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/loc/loc_file.hxx>
#include <sortedids.hxx>
#include "locs_le.hxx"


namespace ary
{
namespace loc
{

struct Directory::Container
{
    typedef SortedIds<Le_Compare>           SortedChildList;

    SortedChildList     aSubDirectories;
    SortedChildList     aFiles;

                        Container()
                            :   aSubDirectories(),
                                aFiles()
                            {}
};




Directory::Directory(Le_id i_assignedRoot)
    :   sLocalName(),
        nParentDirectory(0),
        nAssignedRoot(i_assignedRoot),
        aAssignedNode(),
        pChildren(new Container)
{
    aAssignedNode.Assign_Entity(*this);
}

Directory::Directory( const String  &   i_localName,
                      Le_id             i_parentDirectory )
    :   sLocalName(i_localName),
        nParentDirectory(i_parentDirectory),
        nAssignedRoot(0),
        aAssignedNode(),
        pChildren(new Container)
{
    aAssignedNode.Assign_Entity(*this);
}

Directory::~Directory()
{
}

void
Directory::Add_Dir(const Directory  &  i_dir)
{
    pChildren->aSubDirectories.Add(i_dir.LeId());
}

void
Directory::Add_File(const File & i_file)
{
    pChildren->aFiles.Add(i_file.LeId());
}

Le_id
Directory::Search_Child(const String & i_name) const
{
    Le_id
        ret = Search_Dir(i_name);
    if (ret.IsValid())
        return ret;
    return Search_File(i_name);
}

Le_id
Directory::Search_Dir(const String & i_name) const
{
    return pChildren->aSubDirectories.Search(i_name);
}

Le_id
Directory::Search_File(const String & i_name) const
{
    return pChildren->aFiles.Search(i_name);
}

void
Directory::do_Accept(csv::ProcessorIfc & io_processor) const
{
    csv::CheckedCall(io_processor,*this);
}

ClassId
Directory::get_AryClass() const
{
    return class_id;
}

const String &
Directory::inq_LocalName() const
{
    return sLocalName;
}

Le_id
Directory::inq_ParentDirectory() const
{
    return nParentDirectory;
}


} // namespace loc
} // namespace ary
