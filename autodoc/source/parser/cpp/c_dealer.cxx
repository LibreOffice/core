/*************************************************************************
 *
 *  $RCSfile: c_dealer.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:29 $
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
#include "c_dealer.hxx"


// NOT FULLY DECLARED SERVICES
#include <ary/cpp/c_rwgate.hxx>
#include <ary/cpp/crwg_grp.hxx>
#include <ary/cpp/cg_proj.hxx>
#include <ary/loc/l_rwgate.hxx>
#include <ary/loc/loc_root.hxx>
#include <ary/loc/loc_file.hxx>
#include <ary/docu.hxx>
#include <adoc/a_rdocu.hxx>
#include "all_toks.hxx"
#include "c_rcode.hxx"



namespace ary
{
    namespace cpp
    {
        class ProjectGroup;
    }

    namespace loc
    {
        class LocationRoot;
    }

    class Documentation;
}




namespace cpp
{

Distributor::Distributor( ary::cpp::RwGate & io_rAryGate )
    :   // aCppPreProcessor,
        aCodeExplorer(io_rAryGate),
        // aDocuExplorer,
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
Distributor::StartNewProject( const udmstri &           i_sProjectName,
                              const csv::ploc::Path &   i_rProjectRootDirectory )
{
    ary::loc::LocationRoot & rRoot
            = pGate->Locations().CheckIn_Root( i_rProjectRootDirectory );
    ary::cpp::ProjectGroup & rProject
            = pGate->Groups().CheckIn_ProjectGroup( i_sProjectName, rRoot.Id() );
    pFileEventHandler->SetCurProject(rProject);
}

void
Distributor::StartNewFile( const udmstri &                      i_sFileName,
                           const csv::ploc::DirectoryChain &    i_rFileSubPath )
{
    ary::loc::SourceCodeFile & rFile
            = pGate->Locations().CheckIn_File(
                                        i_sFileName,
                                        i_rFileSubPath,
                                        pFileEventHandler->CurProject().RootDir() );
    ary::cpp::FileGroup & rCurFile
            = pGate->Groups().CheckIn_FileGroup(
                                    rFile.Id(),
                                    pFileEventHandler->CurProject().Id_Group(),
                                    i_sFileName );
    pFileEventHandler->SetCurFile(rCurFile, i_sFileName);

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






}   // namespace cpp


