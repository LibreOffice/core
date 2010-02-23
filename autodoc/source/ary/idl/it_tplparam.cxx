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
#include "it_tplparam.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/tpl/processor.hxx>



namespace ary
{
namespace idl
{



TemplateParamType::TemplateParamType( const char * i_sName )
    :   Named_Type(i_sName)
{
}

TemplateParamType::~TemplateParamType()
{
}

ClassId
TemplateParamType::get_AryClass() const
{
    return class_id;
}

void
TemplateParamType::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

void
TemplateParamType::inq_Get_Text( StringVector &      ,          // o_module
                                 String &            o_name,
                                 Ce_id &             ,          // o_nRelatedCe
                                 int &               ,          // o_nSequenceCount
                                 const Gate &        ) const    // i_rGate
{
    o_name = Name();
}


//*************    Implemented default function for idl::Type ********//

const std::vector<Type_id> *
Type::inq_TemplateParameters() const
{
    return 0;
}

const Type &
Type::inq_FirstEnclosedNonSequenceType(const Gate & ) const
{
    return *this;
}


}   // namespace idl
}   // namespace ary
