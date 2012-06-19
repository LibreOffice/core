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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
