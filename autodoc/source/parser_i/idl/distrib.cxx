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
#include <s2_luidl/distrib.hxx>


// NOT FULLY DECLARED SERVICES
#include <ary/doc/d_oldidldocu.hxx>
#include <parser/parserinfo.hxx>
#include <s2_luidl/tkp_uidl.hxx>
#include <s2_luidl/parsenv2.hxx>
#include <s2_luidl/tk_punct.hxx>
#include <s2_dsapi/docu_pe2.hxx>
#include <adc_cl.hxx>
#include <x_parse2.hxx>



const uintt	C_nNO_TRY = uintt(-1);


namespace csi
{
namespace uidl
{

TokenDistributor::TokenDistributor( ary::Repository & io_rRepository,
                                    ParserInfo &      io_rParserInfo )
    :	pTokenSource(0),
        aDocumentation(io_rParserInfo),
        aProcessingData( io_rRepository, aDocumentation, io_rParserInfo )
{
}

TokenDistributor::~TokenDistributor()
{
}

void
TokenDistributor::TradeToken()
{
    bool bGoon = true;
    while (bGoon AND NOT aProcessingData.NextTokenExists())
    {
        bGoon = pTokenSource->GetNextToken();
    }
    if (bGoon)
        aProcessingData.ProcessCurToken();
}

TokenDistributor::ProcessingData::ProcessingData(
                                        ary::Repository & 	io_rRepository,
                                        Documentation &	        i_rDocuProcessor,
                                        ParserInfo &            io_rParserInfo )
    :	// aEnvironments
        // aTokenQueue
        // itCurToken
        // aCurResult
        nTryCount(0),
        bFinished(false),
        rRepository(io_rRepository),
        rParserInfo(io_rParserInfo),
        pDocuProcessor(&i_rDocuProcessor),
        bPublishedRecentlyOn(false)
{
    itCurToken = aTokenQueue.end();
}

TokenDistributor::ProcessingData::~ProcessingData()
{
}

void
TokenDistributor::ProcessingData::SetTopParseEnvironment( UnoIDL_PE & io_pTopParseEnvironment )
{
    csv::erase_container(aEnvironments);
    aEnvironments.push_back( EnvironmentInfo( &io_pTopParseEnvironment, 0 ) );
    io_pTopParseEnvironment.EstablishContacts(0,rRepository,aCurResult);
}

void
TokenDistributor::ProcessingData::Receive( DYN csi::uidl::Token & let_drToken )
{
    aTokenQueue.push_back( &let_drToken );
    itCurToken = aTokenQueue.end()-1;
}

void
TokenDistributor::ProcessingData::Increment_CurLine()
{
    rParserInfo.Increment_CurLine();
}

void
TokenDistributor::ProcessingData::ProcessCurToken()
{

if (DEBUG_ShowTokens())
{
    Cout() << (*itCurToken)->Text() << Endl();
}

    aCurResult.reset();

    CurEnvironment().ProcessToken( CurToken() );
    AcknowledgeResult();
}


UnoIDL_PE &
TokenDistributor::ProcessingData::CurEnvironment() const
{
    csv_assert(aEnvironments.size() > 0);
    csv_assert(aEnvironments.back().first != 0);

    return *aEnvironments.back().first;
}

bool
TokenDistributor::ProcessingData::NextTokenExists() const
{
    return itCurToken != aTokenQueue.end();
}

void
TokenDistributor::ProcessingData::AcknowledgeResult()
{
    if (aCurResult.eDone == done)
        ++itCurToken;

    switch ( aCurResult.eStackAction )
    {
        case stay:
                if (aCurResult.eDone != done)
                {
                    csv_assert(false);
                }
                break;
        case push_sure:
                CurEnv().Leave(push_sure);
                aEnvironments.push_back( EnvironmentInfo(&PushEnv(), C_nNO_TRY) );
                PushEnv().Enter(push_sure);
                PushEnv().SetDocu(pDocuProcessor->ReleaseLastParsedDocu());
                if (bPublishedRecentlyOn)
                {
                    PushEnv().SetPublished();
                    bPublishedRecentlyOn = false;
                }

                break;
        case push_try:
                Cout() << "TestInfo: Environment tried." << Endl();
                CurEnv().Leave(push_try);
                aEnvironments.push_back( EnvironmentInfo(&PushEnv(), CurTokenPosition()) );
                nTryCount++;
                PushEnv().Enter(push_try);
                break;
        case pop_success:
                CurEnv().Leave(pop_success);
                if ( CurEnv_TriedTokenPosition() > 0 )
                    DecrementTryCount();
                aEnvironments.pop_back();
                CurEnv().Enter(pop_success);
                break;
        case pop_failure:
        {
                CurEnv().Leave(pop_failure);
                if (aCurResult.eDone == done)
                {
                    csv_assert(false);
                }

                if ( CurEnv_TriedTokenPosition() == C_nNO_TRY )
                    throw X_AutodocParser( X_AutodocParser::x_UnexpectedToken, (*itCurToken)->Text() );

                itCurToken = aTokenQueue.begin() + CurEnv_TriedTokenPosition();
                DecrementTryCount();
                aEnvironments.pop_back();
                CurEnv().Enter(pop_failure);
        }		break;
        default:
            csv_assert(false);
    }	// end switch(aCurResult.eStackAction)
}

void
TokenDistributor::ProcessingData::DecrementTryCount()
{
    nTryCount--;
    if (nTryCount == 0)
    {
        aTokenQueue.erase(aTokenQueue.begin(), itCurToken);
        itCurToken = aTokenQueue.begin();
    }
}

TokenDistributor::
Documentation::Documentation(ParserInfo & io_rParserInfo)
    :	pDocuParseEnv(new csi::dsapi::SapiDocu_PE(io_rParserInfo)),
        rParserInfo(io_rParserInfo),
        pMostRecentDocu(0),
        bIsPassedFirstDocu(false)
{
}

TokenDistributor::
Documentation::~Documentation()
{
}

void
TokenDistributor::
Documentation::Receive( DYN csi::dsapi::Token & let_drToken )
{
    csv_assert(pDocuParseEnv);

    pDocuParseEnv->ProcessToken(let_drToken);
    if ( pDocuParseEnv->IsComplete() )
    {
        pMostRecentDocu = pDocuParseEnv->ReleaseJustParsedDocu();
        if (NOT bIsPassedFirstDocu)
        {
            pMostRecentDocu = 0;			// Deletes the most recent docu.
            bIsPassedFirstDocu = true;
        }
    }
}

void
TokenDistributor::
Documentation::Increment_CurLine()
{
    rParserInfo.Increment_CurLine();
}


}   // namespace uidl
}   // namespace csi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
