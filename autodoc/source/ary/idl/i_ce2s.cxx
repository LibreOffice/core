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
#include <ary/idl/i_ce.hxx>


// NOT FULLY DEFINED SERVICES
#include <cosv/tpl/tpltools.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <ary/getncast.hxx>


namespace ary
{
namespace idl
{

namespace
{
const std::vector<Ce_id> C_sNullVector_Ce_ids;
}


Ce_2s::~Ce_2s()
{
    csv::erase_container_of_heap_ptrs(aXrefLists);
}

DYN Ce_2s *
Ce_2s::Create_( ClassId )
{
    return new Ce_2s;
}


std::vector<Ce_id> &
Ce_2s::Access_List( int i_indexOfList )
{
    csv_assert(i_indexOfList >= 0 AND i_indexOfList < 1000);

    while (i_indexOfList >= (int) aXrefLists.size())
    {
        aXrefLists.push_back(new std::vector<Ce_id>);
    }
    return *aXrefLists[i_indexOfList];
}

const std::vector<Ce_id> &
Ce_2s::List( int i_indexOfList ) const
{
    if (uintt(i_indexOfList) < aXrefLists.size())
        return *aXrefLists[i_indexOfList];
    else
        return C_sNullVector_Ce_ids;
}


}   // namespace idl
}   // namespace ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
