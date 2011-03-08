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
#include "c_rcode.hxx"

// NOT FULLY DECLARED SERVICES
#include <ary/cpp/c_gate.hxx>
#include <ary/cpp/c_namesp.hxx>
#include <ary/loc/locp_le.hxx>
#include "cpp_pe.hxx"
#include <adc_cl.hxx>
#include <x_parse.hxx>
#include "pe_file.hxx"

const uintt C_nNO_TRY = uintt(-1);


namespace cpp {


CodeExplorer::CodeExplorer( ary::cpp::Gate & io_rAryGate )
    :   aGlobalParseContext(io_rAryGate),
        // aEnvironments,
        pPE_File(0),
        pGate(&io_rAryGate),
        dpCurToken(0)
{
    pPE_File = new PE_File( aGlobalParseContext );
}

CodeExplorer::~CodeExplorer()
{
}

void
CodeExplorer::StartNewFile()
{
    csv::erase_container(aEnvironments);

    aEnvironments.push_back( pPE_File.MutablePtr() );
    pPE_File->Enter(push);
}

void
CodeExplorer::Process_Token( DYN cpp::Token & let_drToken )
{
if (DEBUG_ShowTokens())
{
    Cout() << let_drToken.Text() << Endl();
}
    dpCurToken = &let_drToken;
    aGlobalParseContext.ResetResult();

    do {
        CurToken().Trigger( CurEnv() );
        AcknowledgeResult();
    } while ( dpCurToken );
}

void
CodeExplorer::AcknowledgeResult()
{
    if (CurResult().eDone == done)
        dpCurToken = 0;

    switch ( CurResult().eStackAction )
    {
        case stay:
                break;
        case push:
                CurEnv().Leave(push);
                aEnvironments.push_back( &PushEnv() );
                PushEnv().Enter(push);
                break;
        case pop_success:
                CurEnv().Leave(pop_success);
                aEnvironments.pop_back();
                CurEnv().Enter(pop_success);
                break;
        case pop_failure:
        {
                Cpp_PE * pRecover = 0;
                do {
                    CurEnv().Leave(pop_failure);
                    aEnvironments.pop_back();
                    if ( aEnvironments.empty() )
                        break;
                    pRecover = CurEnv().Handle_ChildFailure();
                } while ( pRecover == 0 );
                if ( pRecover != 0 )
                {
                    aEnvironments.push_back(pRecover);
                    pRecover->Enter(push);
                }
                else
                {
                    throw X_Parser( X_Parser::x_UnexpectedToken, CurToken().Text(), aGlobalParseContext.CurFileName(), aGlobalParseContext.LineCount() );
                }
        }       break;
        default:
            csv_assert(false);
    }   // end switch(CurResult().eStackAction)
}

const Token &
CodeExplorer::CurToken() const
{
    csv_assert(dpCurToken);

    return *dpCurToken;
}

Cpp_PE &
CodeExplorer::CurEnv() const
{
    csv_assert(aEnvironments.size() > 0);
    csv_assert(aEnvironments.back() != 0);

    return *aEnvironments.back();
}

Cpp_PE &
CodeExplorer::PushEnv() const
{
    TokenProcessing_Result & rCurResult = const_cast< TokenProcessing_Result& >(aGlobalParseContext.CurResult());
    Cpp_PE * ret = dynamic_cast< Cpp_PE* >(rCurResult.pEnv2Push);
    csv_assert( ret != 0 );
    return *ret;
}



}   // namespace cpp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
