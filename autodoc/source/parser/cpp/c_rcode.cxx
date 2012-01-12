/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#include <precomp.h>
#include "c_rcode.hxx"


// NOT FULLY DECLARED SERVICES
#include <ary/cpp/c_gate.hxx>
#include <ary/cpp/c_namesp.hxx>
// #include <ary/cpp/c_groups.hxx>
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

