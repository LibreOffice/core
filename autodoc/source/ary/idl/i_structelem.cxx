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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
