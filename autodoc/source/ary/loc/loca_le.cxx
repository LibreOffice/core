/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: loca_le.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:17:17 $
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
