/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: it_ce.cxx,v $
 * $Revision: 1.8 $
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
#include "it_ce.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/tpl/processor.hxx>
#include <ary/idl/i_ce.hxx>
#include <ary/idl/i_gate.hxx>
#include <ary/idl/i_module.hxx>
#include <ary/idl/ip_ce.hxx>



namespace ary
{
namespace idl
{


Ce_Type::Ce_Type( Ce_id     i_nRelatedCe,
                  Type_id   i_nTemplateType )
    :   nRelatedCe(i_nRelatedCe),
        nTemplateType(i_nTemplateType)
{
}

Ce_Type::~Ce_Type()
{
}

ClassId
Ce_Type::get_AryClass() const
{
    return class_id;
}

void
Ce_Type::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

void
Ce_Type::inq_Get_Text( StringVector &      o_module,
                       String &            o_name,
                       Ce_id &             o_nRelatedCe,
                       int &               ,                // o_nSequenceCount
                       const Gate &        i_rGate ) const
{
    String sDummyMember;

    const CodeEntity &
        rCe = i_rGate.Ces().Find_Ce(nRelatedCe);
    i_rGate.Ces().Get_Text( o_module,
                            o_name,
                            sDummyMember,
                            rCe );
    o_nRelatedCe = nRelatedCe;
}

Type_id
Ce_Type::inq_TemplateParameterType() const
{
    return nTemplateType;
}




}   // namespace idl
}   // namespace ary
