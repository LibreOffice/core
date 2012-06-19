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
#include "it_builtin.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/tpl/processor.hxx>



namespace ary
{
namespace idl
{



BuiltInType::BuiltInType( const char * i_sName )
    :   Named_Type(i_sName)
{
}

BuiltInType::~BuiltInType()
{
}

ClassId
BuiltInType::get_AryClass() const
{
    return class_id;
}

void
BuiltInType::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

void
BuiltInType::inq_Get_Text( StringVector &      ,            // o_module
                           String &            o_name,
                           Ce_id &             ,            // o_nRelatedCe
                           int &               ,            // o_nSequenceCount
                           const Gate &        ) const      // i_rGate
{
    o_name = Name();
}




}   // namespace idl
}   // namespace ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
