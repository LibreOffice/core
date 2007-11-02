/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: is_type.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:54:25 $
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
#include "is_type.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/tpl/tpltools.hxx>

namespace
{

const uintt
    C_nReservedElements = ary::idl::predefined::type_MAX;    // Skipping "0" and the built in types.
}


namespace ary
{
namespace idl
{

Type_Storage *          Type_Storage::pInstance_ = 0;



Type_Storage::Type_Storage()
    :   stg::Storage<Type>(C_nReservedElements),
        aSequenceIndex()
{
    csv_assert(pInstance_ == 0);
    pInstance_ = this;
}

Type_Storage::~Type_Storage()
{
    csv_assert(pInstance_ != 0);
    pInstance_ = 0;
}

void
Type_Storage::Add_Sequence( Type_id             i_nRelatedType,
                            Type_id             i_nSequence )
{
    aSequenceIndex[i_nRelatedType] = i_nSequence;
}

Type_id
Type_Storage::Search_SequenceOf( Type_id i_nRelatedType )
{
    return csv::value_from_map(aSequenceIndex, i_nRelatedType);
}




}   // namespace idl
}   // namespace ary
