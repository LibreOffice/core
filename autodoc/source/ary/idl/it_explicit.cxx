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
#include "it_explicit.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/tpl/processor.hxx>
#include <ary/idl/i_module.hxx>
#include <ary/idl/i_gate.hxx>
#include <ary/idl/ip_ce.hxx>
#include <ary/idl/ip_type.hxx>
#include "it_xnameroom.hxx"



namespace ary
{
namespace idl
{


ExplicitType::ExplicitType( const String &      i_sName,
                            Type_id             i_nXNameRoom,
                            Ce_id               i_nModuleOfOccurrence,
                            const std::vector<Type_id> *
                                                i_templateParameters )
    :   Named_Type(i_sName),
        nXNameRoom(i_nXNameRoom),
        nModuleOfOccurrence(i_nModuleOfOccurrence),
        pTemplateParameters(0)
{
    if (i_templateParameters != 0)
        pTemplateParameters = new std::vector<Type_id>(*i_templateParameters);
}

ExplicitType::~ExplicitType()
{
}

ClassId
ExplicitType::get_AryClass() const
{
    return class_id;
}

void
ExplicitType::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

void
ExplicitType::inq_Get_Text( StringVector &      o_module,
                            String &            o_name,
                            Ce_id &             o_nRelatedCe,
                            int &               o_nSequenceCount,
                            const Gate &        i_rGate ) const
{
    const ExplicitNameRoom &
        rNameRoom = i_rGate.Types().Find_XNameRoom(nXNameRoom);
    rNameRoom.Get_Text(o_module,o_name,o_nRelatedCe,o_nSequenceCount,i_rGate);

    o_name = Name();
}

const std::vector<Type_id> *
ExplicitType::inq_TemplateParameters() const
{
    return pTemplateParameters.Ptr();
}


}   // namespace idl
}   // namespace ary
