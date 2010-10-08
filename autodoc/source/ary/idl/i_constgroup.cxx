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
#include <ary/idl/i_constgroup.hxx>
#include <ary/idl/ik_constgroup.hxx>


// NOT FULLY DECLARED SERVICES
#include <cosv/tpl/processor.hxx>
#include <sci_impl.hxx>


namespace ary
{
namespace idl
{


ConstantsGroup::ConstantsGroup( const String &      i_sName,
                                Ce_id               i_nModule )
    :   sName(i_sName),
        nModule(i_nModule),
        aConstants()
{
}

ConstantsGroup::~ConstantsGroup()
{
}

void
ConstantsGroup::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

ClassId
ConstantsGroup::get_AryClass() const
{
    return class_id;
}

const String &
ConstantsGroup::inq_LocalName() const
{
    return sName;
}

Ce_id
ConstantsGroup::inq_NameRoom() const
{
    return nModule;
}

Ce_id
ConstantsGroup::inq_Owner() const
{
    return nModule;
}

E_SightLevel
ConstantsGroup::inq_SightLevel() const
{
    return sl_File;
}


namespace ifc_constgroup
{

inline const ConstantsGroup &
constgroup_cast( const CodeEntity & i_ce )
{
    csv_assert( i_ce.AryClass() == ConstantsGroup::class_id );
    return static_cast< const ConstantsGroup& >(i_ce);
}

void
attr::Get_Constants( Dyn_CeIterator &    o_result,
                     const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(constgroup_cast(i_ce).aConstants);
}

} // namespace ifc_constgroup


}   //  namespace   idl
}   //  namespace   ary
