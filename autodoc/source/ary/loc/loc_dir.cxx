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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
