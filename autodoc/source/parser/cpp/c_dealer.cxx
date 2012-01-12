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
#include "c_dealer.hxx"


// NOT FULLY DECLARED SERVICES
#include <ary/cpp/c_gate.hxx>
#include <ary/loc/locp_le.hxx>
#include <ary/loc/loc_root.hxx>
#include <ary/loc/loc_file.hxx>
//#include <ary/docu.hxx>
#include <adoc/a_rdocu.hxx>
#include "all_toks.hxx"
#include "c_rcode.hxx"


namespace ary
{
namespace loc
{
    class Root;
}
}




namespace cpp
{

Distributor::Distributor( ary::cpp::Gate & io_rAryGate )
    :   aCppPreProcessor(),
        aCodeExplorer(io_rAryGate),
        aDocuExplorer(),
        pGate(&io_rAryGate),
        pFileEventHandler(0),
        pDocuDistributor(0)
{
    pFileEventHandler = & aCodeExplorer.FileEventHandler();
    pDocuDistributor = & aCodeExplorer.DocuDistributor();
}

void
Distributor::AssignPartners( CharacterSource &   io_rSourceText,
                             const MacroMap &    i_rValidMacros )
{
    aCppPreProcessor.AssignPartners(aCodeExplorer, io_rSourceText, i_rValidMacros);
}

Distributor::~Distributor()
{
}

void
Distributor::StartNewFile( const csv::ploc::Path & i_file )
{
    const csv::ploc::Root &
        root_dir = i_file.RootDir();
    StreamLock
        sl(700);
    root_dir.Get(sl());
    csv::ploc::Path
        root_path( sl().c_str(), true );
    ary::loc::Le_id
        root_id = pGate->Locations().CheckIn_Root(root_path).LeId();
    ary::loc::File &
        rFile = pGate->Locations().CheckIn_File(
                                        i_file.File(),
                                        i_file.DirChain(),
                                        root_id  );
    pFileEventHandler->SetCurFile(rFile);

    aCodeExplorer.StartNewFile();

    csv_assert( pDocuDistributor != 0 );
    aDocuExplorer.StartNewFile(*pDocuDistributor);
}


void
Distributor::Deal_Eol()
{
    pFileEventHandler->Event_IncrLineCount();
}

void
Distributor::Deal_Eof()
{
    // Do nothing yet.
}

void
Distributor::Deal_Cpp_UnblockMacro( Tok_UnblockMacro & let_drToken )
{
    aCppPreProcessor.UnblockMacro(let_drToken.Text());
    delete &let_drToken;
}

void
Distributor::Deal_CppCode( cpp::Token & let_drToken )
{
    aCppPreProcessor.Process_Token(let_drToken);
}

void
Distributor::Deal_AdcDocu( adoc::Token & let_drToken )
{
    aDocuExplorer.Process_Token(let_drToken);
}

Distributor *
Distributor::AsDistributor()
{
     return this;
}





}   // namespace cpp


