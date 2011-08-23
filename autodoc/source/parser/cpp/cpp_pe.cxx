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
#include "cpp_pe.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/doc/d_oldcppdocu.hxx>
#include "cpp_tok.hxx"




namespace cpp {

void
Cpp_PE::SetTokenResult( E_TokenDone			i_eDone,
                        E_EnvStackAction	i_eWhat2DoWithEnvStack,
                        ParseEnvironment *	i_pParseEnv2Push )
{
    rMyEnv.SetTokenResult(		i_eDone,
                                i_eWhat2DoWithEnvStack,
                                i_pParseEnv2Push );
}

Cpp_PE::Cpp_PE( Cpp_PE * io_pParent )
    :	ParseEnvironment( io_pParent ),
        rMyEnv( io_pParent->Env() )
{
    csv_assert(io_pParent != 0);
}

Cpp_PE::Cpp_PE( EnvData & i_rEnv )
    :	ParseEnvironment(0),
        rMyEnv(i_rEnv)
{
}

void
Cpp_PE::StdHandlingOfSyntaxError( const char * )
{
    SetTokenResult(not_done, pop_failure);
}


Cpp_PE *
Cpp_PE::Handle_ChildFailure()
{
     return 0;
}

}   // namespace cpp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
