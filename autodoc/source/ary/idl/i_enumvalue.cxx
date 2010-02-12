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
#include <ary/idl/i_enumvalue.hxx>


// NOT FULLY DECLARED SERVICES
#include <cosv/tpl/processor.hxx>
#include <ary/idl/ik_enumvalue.hxx>


namespace ary
{
namespace idl
{

EnumValue::EnumValue( const String &      i_sName,
                      Ce_id               i_nOwner,
                      Ce_id               i_nNameRoom,
                      const String &      i_sInitValue )
    :   sName(i_sName),
        nOwner(i_nOwner),
        nNameRoom(i_nNameRoom),
        sValue(i_sInitValue)
{
}

EnumValue::~EnumValue()
{
}

void
EnumValue::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

ClassId
EnumValue::get_AryClass() const
{
    return class_id;
}

const String &
EnumValue::inq_LocalName() const
{
    return sName;
}

Ce_id
EnumValue::inq_NameRoom() const
{
    return nNameRoom;
}

Ce_id
EnumValue::inq_Owner() const
{
    return nOwner;
}

E_SightLevel
EnumValue::inq_SightLevel() const
{
    return sl_Member;
}



namespace ifc_enumvalue
{

inline const EnumValue &
enumvalue_cast( const CodeEntity &  i_ce )
{
    csv_assert( i_ce.AryClass() == EnumValue::class_id );
    return static_cast< const EnumValue& >(i_ce);
}

const String &
attr::Value( const CodeEntity & i_ce )
{
    return enumvalue_cast(i_ce).sValue;
}


} // namespace ifc_enumvalue


}   //  namespace   idl
}   //  namespace   ary
