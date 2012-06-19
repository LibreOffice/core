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
#include <ary/idl/i_attribute.hxx>
#include <ary/idl/ik_attribute.hxx>


// NOT FULLY DECLARED SERVICES
#include <cosv/tpl/processor.hxx>
#include <ary/getncast.hxx>
#include <sci_impl.hxx>


namespace ary
{
namespace idl
{


Attribute::Attribute( const String &      i_sName,
                      Ce_id               i_nService,
                      Ce_id               i_nModule,
                      Type_id             i_nType,
                      bool                i_bReadonly,
                      bool                i_bBound )
    :   sName(i_sName),
        nOwner(i_nService),
        nNameRoom(i_nModule),
        nType(i_nType),
        aGetExceptions(),
        aSetExceptions(),
        bReadonly(i_bReadonly),
        bBound(i_bBound)
{
}

Attribute::~Attribute()
{
}


void
Attribute::do_Accept(csv::ProcessorIfc & io_processor) const
{
    csv::CheckedCall(io_processor,*this);
}

ClassId
Attribute::get_AryClass() const
{
    return class_id;
}

const String &
Attribute::inq_LocalName() const
{
    return sName;
}

Ce_id
Attribute::inq_NameRoom() const
{
    return nNameRoom;
}

Ce_id
Attribute::inq_Owner() const
{
    return nOwner;
}

E_SightLevel
Attribute::inq_SightLevel() const
{
    return sl_Member;
}

namespace ifc_attribute
{

inline const Attribute &
attribute_cast( const CodeEntity &  i_ce )
{
    csv_assert( is_type<Attribute>(i_ce) );
    return static_cast< const Attribute& >(i_ce);
}

bool
attr::HasAnyStereotype( const CodeEntity & i_ce )
{
    const Attribute & rAttr = attribute_cast(i_ce);
    return rAttr.bReadonly OR rAttr.bBound;
}

bool
attr::IsReadOnly( const CodeEntity & i_ce )
{
    return attribute_cast(i_ce).bReadonly;
}

bool
attr::IsBound( const CodeEntity & i_ce )
{
    return attribute_cast(i_ce).bBound;
}

Type_id
attr::Type( const CodeEntity & i_ce )
{
    return attribute_cast(i_ce).nType;
}

void
attr::Get_GetExceptions( Dyn_TypeIterator &  o_result,
                         const CodeEntity &  i_ce )
{
    o_result
        = new SCI_Vector<Type_id>( attribute_cast(i_ce).aGetExceptions );
}

void
attr::Get_SetExceptions( Dyn_TypeIterator &  o_result,
                         const CodeEntity &  i_ce )
{
    o_result
        = new SCI_Vector<Type_id>( attribute_cast(i_ce).aSetExceptions );
}


} // namespace ifc_attribute


}   //  namespace   idl
}   //  namespace   ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
