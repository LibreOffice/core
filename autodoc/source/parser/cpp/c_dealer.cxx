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
#include "c_dealer.hxx"

// NOT FULLY DECLARED SERVICES
#include <ary/cpp/c_gate.hxx>
#include <ary/loc/locp_le.hxx>
#include <ary/loc/loc_root.hxx>
#include <ary/loc/loc_file.hxx>
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
