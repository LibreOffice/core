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
#include "tkp_cpp.hxx"

// NOT FULLY DECLARED SERVICES
#include "cx_c_std.hxx"
#include "c_dealer.hxx"


namespace cpp {




TokenParser_Cpp::TokenParser_Cpp( DYN autodoc::TkpDocuContext & let_drDocuContext )
    :	pBaseContext( new Context_CppStd( let_drDocuContext ) ),
        pCurContext(0),
        pDealer(0),
        pCharacterSource(0)
{
    SetStartContext();
}

TokenParser_Cpp::~TokenParser_Cpp()
{
}

void
TokenParser_Cpp::AssignPartners( CharacterSource &   io_rCharacterSource,
                                 cpp::Distributor &  o_rDealer )
{
    pDealer = &o_rDealer;
    pBaseContext->AssignDealer(o_rDealer);
    pCharacterSource = &io_rCharacterSource;
}

void
TokenParser_Cpp::StartNewFile( const csv::ploc::Path & i_file )
{
    csv_assert(pDealer != 0);
    pDealer->StartNewFile(i_file);

    csv_assert(pCharacterSource != 0);
    Start(*pCharacterSource);
}

void
TokenParser_Cpp::SetStartContext()
{
    pCurContext = pBaseContext.Ptr();
}

void
TokenParser_Cpp::SetCurrentContext( TkpContext & io_rContext )
{
    pCurContext = &io_rContext;
}

TkpContext &
TokenParser_Cpp::CurrentContext()
{
    return *pCurContext;
}

}   // namespace cpp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
