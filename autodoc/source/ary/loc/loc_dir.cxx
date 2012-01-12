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
