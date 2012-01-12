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
#include "loca_le.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/loc/loc_dir.hxx>
#include <ary/loc/loc_file.hxx>
#include <ary/loc/loc_root.hxx>
#include <loc_internalgate.hxx>
#include "locs_le.hxx"




namespace ary
{
namespace loc
{

DYN LocationPilot &
InternalGate::Create_Locations_()
{
    return *new LocationAdmin;
}




inline Le_Storage &
LocationAdmin::Storage() const
{
    csv_assert(pStorage);
    return *pStorage.MutablePtr();
}


LocationAdmin::LocationAdmin()
    :   pStorage(new Le_Storage)
{
}

LocationAdmin::~LocationAdmin()
{
}

Root &
LocationAdmin::CheckIn_Root(const csv::ploc::Path & i_path)
{
    Dyn<Root>
        p_new( new Root(i_path) );

    Le_id
        id = Storage().RootIndex().Search(p_new->LocalName());
    if ( id.IsValid() )
    {
        return ary_cast<Root>(Storage()[id]);
    }

    Root *
        ret = p_new.Ptr();
    Storage().Store_Entity(*p_new.Release());
    Storage().RootIndex().Add(ret->LeId());

    Directory *
        p_rootdir = new Directory(ret->LeId());
    Storage().Store_Entity(*p_rootdir);
    ret->Assign_Directory(p_rootdir->LeId());

    return *ret;
}

File &
LocationAdmin::CheckIn_File( const String  &                   i_name,
                                  const csv::ploc::DirectoryChain & i_subPath,
                                  Le_id                             i_root )
{
    Root &
        root = Find_Root(i_root);
    Directory &
        parent_dir = CheckIn_Directories(
                        Find_Directory(root.MyDir()),
                        i_subPath.Begin(),
                        i_subPath.End() );
    Le_id
        fid = parent_dir.Search_File(i_name);
    if (NOT fid.IsValid())
    {
        File *
            ret = new File(i_name, parent_dir.LeId());
        Storage().Store_Entity(*ret);
        parent_dir.Add_File(*ret);
        return *ret;
    }
    else
    {
        return Find_File(fid);
    }
}

Root &
LocationAdmin::Find_Root(Le_id  i_id) const
{
    return ary_cast<Root>(Storage()[i_id]);
}

Directory &
LocationAdmin::Find_Directory(Le_id i_id) const
{
    return ary_cast<Directory>(Storage()[i_id]);
}

File &
LocationAdmin::Find_File(Le_id i_id) const
{
    return ary_cast<File>(Storage()[i_id]);
}

Directory &
LocationAdmin::CheckIn_Directory( Directory &         io_parent,
                                       const String &      i_name )
{
    Le_id
        did = io_parent.Search_Dir(i_name);
    if (NOT did.IsValid())
    {
        Directory *
            ret = new Directory(i_name, io_parent.LeId());
        Storage().Store_Entity(*ret);
        io_parent.Add_Dir(*ret);
        return *ret;
    }
    else
    {
        return Find_Directory(did);
    }
}

Directory &
LocationAdmin::CheckIn_Directories(
                            Directory &                     io_root,
                            StringVector::const_iterator    i_beginSubPath,
                            StringVector::const_iterator    i_endSubPath )
{
    if (i_beginSubPath == i_endSubPath)
        return io_root;

    Directory &
        next = CheckIn_Directory(io_root, *i_beginSubPath);
    return CheckIn_Directories(next, i_beginSubPath+1, i_endSubPath);
}


}   // namespace loc
}   // namespace ary
