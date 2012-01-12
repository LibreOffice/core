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
#include <adoc/a_rdocu.hxx>


// NOT FULLY DECLARED SERVICES
#include <adoc/docu_pe.hxx>
#include <adoc/adoc_tok.hxx>
#include <ary/doc/d_oldcppdocu.hxx>
#include <doc_deal.hxx>



namespace adoc
{


DocuExplorer::DocuExplorer()
    :   pDocuDistributor(0),
        pPE(new Adoc_PE),
        bIsPassedFirstDocu(false)
{
}

DocuExplorer::~DocuExplorer()
{
}

void
DocuExplorer::StartNewFile( DocuDealer & o_rDocuDistributor )
{
    pDocuDistributor = &o_rDocuDistributor;
    bIsPassedFirstDocu = false;
}


void
DocuExplorer::Process_Token( DYN adoc::Token & let_drToken )
{
    csv_assert(pDocuDistributor != 0);

    let_drToken.Trigger(*pPE);
    if ( pPE->IsComplete() )
    {
        ary::doc::OldCppDocu *
            pDocu = pPE->ReleaseJustParsedDocu();
        if ( pDocu != 0 )
        {
            if (bIsPassedFirstDocu)
                pDocuDistributor->TakeDocu( *pDocu );
            else
            {
                delete pDocu;
                bIsPassedFirstDocu = true;
            }
        }
    }

    delete &let_drToken;
}


}   // namespace adoc

