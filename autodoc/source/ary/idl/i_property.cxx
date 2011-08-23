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
#include <ary/idl/i_property.hxx>
#include <ary/idl/ik_property.hxx>


// NOT FULLY DECLARED SERVICES
#include <cosv/tpl/processor.hxx>


namespace ary
{
namespace idl
{


Property::Property( const String &      i_sName,
                    Ce_id               i_nService,
                    Ce_id               i_nModule,
                    Type_id             i_nType,
                    Stereotypes         i_stereotypes )
    :   sName(i_sName),
        nOwner(i_nService),
        nNameRoom(i_nModule),
        nType(i_nType),
        aStereotypes(i_stereotypes)
{
}

Property::~Property()
{
}


void
Property::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

ClassId
Property::get_AryClass() const
{
    return class_id;
}

const String &
Property::inq_LocalName() const
{
    return sName;
}

Ce_id
Property::inq_NameRoom() const
{
    return nNameRoom;
}

Ce_id
Property::inq_Owner() const
{
    return nOwner;
}

E_SightLevel
Property::inq_SightLevel() const
{
    return sl_Member;
}

namespace ifc_property
{

inline const Property &
property_cast( const CodeEntity &  i_ce )
{
    csv_assert( i_ce.AryClass() == Property::class_id );
    return static_cast< const Property& >(i_ce);
}

bool
attr::HasAnyStereotype( const CodeEntity &  i_ce )
{
    return property_cast(i_ce).aStereotypes.HasAny();
}

bool
attr::IsReadOnly( const CodeEntity & i_ce )
{
    return property_cast(i_ce).aStereotypes.IsReadOnly();
}

bool
attr::IsBound( const CodeEntity & i_ce )
{
    return property_cast(i_ce).aStereotypes.IsBound();
}

bool
attr::IsConstrained( const CodeEntity & i_ce )
{
    return property_cast(i_ce).aStereotypes.IsConstrained();
}

bool
attr::IsMayBeAmbiguous( const CodeEntity & i_ce )
{
    return property_cast(i_ce).aStereotypes.IsMayBeAmbiguous();
}

bool
attr::IsMayBeDefault( const CodeEntity & i_ce )
{
    return property_cast(i_ce).aStereotypes.IsMayBeDefault();
}

bool
attr::IsMayBeVoid( const CodeEntity & i_ce )
{
    return property_cast(i_ce).aStereotypes.IsMayBeVoid();
}

bool
attr::IsRemovable( const CodeEntity & i_ce )
{
    return property_cast(i_ce).aStereotypes.IsRemovable();
}

bool
attr::IsTransient( const CodeEntity & i_ce )
{
    return property_cast(i_ce).aStereotypes.IsTransient();
}

Type_id
attr::Type( const CodeEntity & i_ce )
{
    return property_cast(i_ce).nType;
}

} // namespace ifc_property


}   //  namespace   idl
}   //  namespace   ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
