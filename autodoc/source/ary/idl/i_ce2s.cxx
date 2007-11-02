/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_ce2s.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:43:21 $
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
