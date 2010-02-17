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
#include <ary/idl/i_structelem.hxx>
#include <ary/idl/ik_structelem.hxx>


// NOT FULLY DECLARED SERVICES
#include <cosv/tpl/processor.hxx>
#include <sci_impl.hxx>


namespace ary
{
namespace idl
{


StructElement::StructElement( const String &      i_sName,
                              Ce_id               i_nOwner,
                              Ce_id               i_nNameRoom,
                              Type_id             i_nType )
    :   sName(i_sName),
        nOwner(i_nOwner),
        nNameRoom(i_nNameRoom),
        nType(i_nType)
{
}

StructElement::~StructElement()
{
}

void
StructElement::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

ClassId
StructElement::get_AryClass() const
{
    return class_id;
}

const String &
StructElement::inq_LocalName() const
{
    return sName;
}

Ce_id
StructElement::inq_NameRoom() const
{
    return nNameRoom;
}

Ce_id
StructElement::inq_Owner() const
{
    return nOwner;
}

E_SightLevel
StructElement::inq_SightLevel() const
{
    return sl_Member;
}


namespace ifc_structelement
{

inline const StructElement &
selem_cast( const CodeEntity &  i_ce )
{
    csv_assert( i_ce.AryClass() == StructElement::class_id );
    return static_cast< const StructElement& >(i_ce);
}

Type_id
attr::Type( const CodeEntity & i_ce )
{
    return selem_cast(i_ce).nType;
}

} // namespace ifc_structelement



}   //  namespace   idl
}   //  namespace   ary
