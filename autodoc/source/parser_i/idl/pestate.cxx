/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
ParseEnvState::Process_MetaType( const TokMetaType &	)
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
    else	// ignore:
        MyPE().SetResult(done, stay);
}


}   // namespace uidl
}   // namespace csi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
