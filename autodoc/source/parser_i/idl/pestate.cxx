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
#include <s2_luidl/pestate.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/doc/d_oldidldocu.hxx>
#include <s2_luidl/parsenv2.hxx>




namespace csi
{
namespace uidl
{

void
ParseEnvState::Process_Identifier( const TokIdentifier & )
{
    Process_Default();
}

void
ParseEnvState::Process_NameSeparator()
{
    Process_Default();
}

void
ParseEnvState::Process_Punctuation( const TokPunctuation & )
{
    Process_Default();
}

void
ParseEnvState::Process_BuiltInType( const TokBuiltInType & )
{
    Process_Default();
}

void
ParseEnvState::Process_TypeModifier( const TokTypeModifier & )
{
    Process_Default();
}

void
ParseEnvState::Process_MetaType( const TokMetaType &    )
{
    Process_Default();
}

void
ParseEnvState::Process_Stereotype( const TokStereotype & )
{
    Process_Default();
}

void
ParseEnvState::Process_ParameterHandling( const TokParameterHandling & )
{
    Process_Default();
}

void
ParseEnvState::Process_Raises()
{
    Process_Default();
}

void
ParseEnvState::Process_Needs()
{
    Process_Default();
}

void
ParseEnvState::Process_Observes()
{
    Process_Default();
}

void
ParseEnvState::Process_Assignment( const TokAssignment & )
{
    Process_Default();
}

void
ParseEnvState::Process_EOL()
{
    MyPE().SetResult(done,stay);
}


void
ParseEnvState::On_SubPE_Left()
{
}

void
ParseEnvState::Process_Default()
{
    if (bDefaultIsError)
        MyPE().SetResult(not_done, pop_failure);
    else    // ignore:
        MyPE().SetResult(done, stay);
}


}   // namespace uidl
}   // namespace csi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
