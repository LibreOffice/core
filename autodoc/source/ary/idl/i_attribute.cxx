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
