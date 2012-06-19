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


Ce_Type::Ce_Type( Ce_id                        i_relatedCe,
                  const std::vector<Type_id> * i_templateParameters )
    :   nRelatedCe(i_relatedCe),
        pTemplateParameters(0)
{
    if (i_templateParameters != 0)
        pTemplateParameters = new std::vector<Type_id>(*i_templateParameters);
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

const std::vector<Type_id> *
Ce_Type::inq_TemplateParameters() const
{
    return pTemplateParameters.Ptr();
}




}   // namespace idl
}   // namespace ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
