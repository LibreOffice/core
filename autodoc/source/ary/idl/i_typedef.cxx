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
#include <ary/idl/i_typedef.hxx>
#include <ary/idl/ik_typedef.hxx>


// NOT FULLY DECLARED SERVICES
#include <cosv/tpl/processor.hxx>
#include <sci_impl.hxx>
#include "i2s_calculator.hxx"


namespace ary
{
namespace idl
{


Typedef::Typedef( const String &      i_sName,
                  Ce_id               i_nOwner,
                  Type_id             i_nDefiningType )
    :   sName(i_sName),
        nOwner(i_nOwner),
        nDefiningType(i_nDefiningType)
{
}

Typedef::~Typedef()
{
}

void
Typedef::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

ClassId
Typedef::get_AryClass() const
{
    return class_id;
}

const String &
Typedef::inq_LocalName() const
{
    return sName;
}

Ce_id
Typedef::inq_NameRoom() const
{
    return nOwner;
}

Ce_id
Typedef::inq_Owner() const
{
    return nOwner;
}

E_SightLevel
Typedef::inq_SightLevel() const
{
    return sl_File;
}


namespace ifc_typedef
{

inline const Typedef &
typedef_cast( const CodeEntity & i_ce )
{
    csv_assert( i_ce.AryClass() == Typedef::class_id );
    return static_cast< const Typedef& >(i_ce);
}

Type_id
attr::DefiningType( const CodeEntity & i_ce )
{
    return typedef_cast(i_ce).nDefiningType;
}


void
xref::Get_SynonymTypedefs( Dyn_CeIterator &    o_result,
                           const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(typedef_2s_SynonymTypedefs));
}

void
xref::Get_AsReturns( Dyn_CeIterator &    o_result,
                     const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(typedef_2s_AsReturns));
}

void
xref::Get_AsParameters( Dyn_CeIterator &    o_result,
                        const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(typedef_2s_AsParameters));
}

void
xref::Get_AsDataTypes( Dyn_CeIterator &    o_result,
                       const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(typedef_2s_AsDataTypes));
}

} // namespace ifc_typedef



}   //  namespace   idl
}   //  namespace   ary
