/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_dealer.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:47:59 $
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


