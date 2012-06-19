/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
