/*************************************************************************
 *
 *  $RCSfile: distrib.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:39:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <precomp.h>
#include <s2_luidl/distrib.hxx>


// NOT FULLY DECLARED SERVICES
#include <ary_i/codeinf2.hxx>
#include <s2_luidl/tkp_uidl.hxx>
#include <s2_luidl/parsenv2.hxx>
#include <s2_luidl/tk_punct.hxx>
#include <s2_dsapi/docu_pe2.hxx>
#include <adc_cl.hxx>
#include <x_parse2.hxx>



const uintt C_nNO_TRY = uintt(-1);


namespace csi
{
namespace uidl
{

TokenDistributor::TokenDistributor( ary::n22::Repository & io_rRepository )
    :   pTokenSource(0),
        aProcessingData( io_rRepository, aDocumentation ),
        aDocumentation()
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

TokenDistributor::ProcessingData::ProcessingData( ary::n22::Repository &    io_rRepository,
                                                  Documentation &       i_rDocuProcessor )
    :   // aEnvironments
        // aTokenQueue
        // itCurToken
        // aCurResult
        nTryCount(0),
        bFinished(false),
        rRepository(io_rRepository),
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
        }       break;
        default:
            csv_assert(false);
    }   // end switch(aCurResult.eStackAction)
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
Documentation::Documentation()
    :   pDocuParseEnv(new csi::dsapi::SapiDocu_PE),
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
            pMostRecentDocu = 0;            // Deletes the most recent docu.
            bIsPassedFirstDocu = true;
        }
    }
}


}   // namespace uidl
}   // namespace csi

