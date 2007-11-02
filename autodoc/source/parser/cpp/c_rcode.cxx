/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_rcode.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:48:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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

#ifdef IRIX
// see i19418 incomplete type for cpp::PE_Namespace
#include "pe_namsp.hxx"
#include "pe_ignor.hxx"
#include "pe_defs.hxx"
#include "pe_vafu.hxx"
#include "pe_tydef.hxx"
#include "pe_tpltp.hxx"
#include "pe_type.hxx"
#include "pe_vari.hxx"
#include "pe_funct.hxx"
#include "pe_enum.hxx"
#include "pe_param.hxx"
#include "pe_class.hxx"
#include "pe_expression.hxx"
#include "pe_base.hxx"
#include "pe_enum.hxx"
#include "pe_enval.hxx"
#endif


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

ary::loc::Le_id
CodeExplorer::CurFile() const
{
    // KORR_FUTURE
    return ary::loc::Le_id(0);
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

