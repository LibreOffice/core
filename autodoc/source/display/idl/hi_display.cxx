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
#include <idl/hi_display.hxx>


// NOT FULLY DEFINED SERVICES
#include <cosv/file.hxx>
#include <ary/idl/i_ce.hxx>
#include <ary/idl/i_module.hxx>
#include <ary/getncast.hxx>
#include <toolkit/out_tree.hxx>
#include <cfrstd.hxx>
#include "hi_ary.hxx"
#include "hi_env.hxx"
#include "hi_main.hxx"


extern const String C_sCssFilename_Idl;


inline bool
HtmlDisplay_Idl::IsModule( const ary::idl::CodeEntity & i_ce ) const
{
     return ary::is_type<ary::idl::Module>(i_ce);
}

inline const ary::idl::Module &
HtmlDisplay_Idl::Module_Cast( const ary::idl::CodeEntity & i_ce ) const
{
     return ary::ary_cast<ary::idl::Module>(i_ce);
}




HtmlDisplay_Idl::HtmlDisplay_Idl()
    :   pCurPageEnv(),
        pMainDisplay()
{
}

HtmlDisplay_Idl::~HtmlDisplay_Idl()
{
}

void
HtmlDisplay_Idl::do_Run( const char *                       i_sOutputDirectory,
                         const ary::idl::Gate &             i_rAryGate,
                         const display::CorporateFrame &    i_rLayout )
{
    SetRunData( i_sOutputDirectory, i_rAryGate, i_rLayout );

    Create_StartFile();
    Create_CssFile();
    Create_FilesInNameTree();
    Create_IndexFiles();
    Create_FilesInProjectTree();
    Create_PackageList();
    Create_HelpFile();
}

void
HtmlDisplay_Idl::SetRunData( const char *                       i_sOutputDirectory,
                             const ary::idl::Gate &             i_rAryGate,
                             const display::CorporateFrame &    i_rLayout  )
{
    csv::ploc::Path aOutputDir( i_sOutputDirectory, true );
    pCurPageEnv = new HtmlEnvironment_Idl( aOutputDir, i_rAryGate, i_rLayout );
    pMainDisplay = new MainDisplay_Idl(*pCurPageEnv);
}

void
HtmlDisplay_Idl::Create_StartFile()
{
}

void
HtmlDisplay_Idl::Create_FilesInNameTree()
{
    Cout() << "\nCreate files in subtree namespaces ..." << Endl();

    const ary::idl::Module &
        rGlobalNamespace = pCurPageEnv->Data().GlobalNamespace();
    pCurPageEnv->Goto_Directory( pCurPageEnv->OutputTree().NamesRoot(), true );

    RecursiveDisplay_Module(rGlobalNamespace);

    Cout() << "... done." << Endl();
}

void
HtmlDisplay_Idl::Create_IndexFiles()
{
    Cout() << "\nCreate files in subtree index ..." << Endl();
    pCurPageEnv->Goto_Directory( pCurPageEnv->OutputTree().IndexRoot(), true );
    pMainDisplay->WriteGlobalIndices();
    Cout() << "... done.\n" << Endl();
}

typedef ary::Dyn_StdConstIterator<ary::idl::Ce_id>  Dyn_CeIterator;
typedef ary::StdConstIterator<ary::idl::Ce_id>      CeIterator;

void
HtmlDisplay_Idl::RecursiveDisplay_Module( const ary::idl::Module & i_module )
{
    i_module.Accept(*pMainDisplay);

    Dyn_CeIterator
        aMembers;
    i_module.Get_Names(aMembers);

    for ( CeIterator & iter = *aMembers;
          iter;
          ++iter )
    {
        const ary::idl::CodeEntity &
            rCe = pCurPageEnv->Data().Find_Ce(*iter);

        if ( NOT IsModule(rCe) )
            rCe.Accept(*pMainDisplay);
        else
        {
            pCurPageEnv->Goto_DirectoryLevelDown( rCe.LocalName(), true );
            RecursiveDisplay_Module( Module_Cast(rCe) );
            pCurPageEnv->Goto_DirectoryLevelUp();
        }
    }   // end for
}

void
HtmlDisplay_Idl::Create_FilesInProjectTree()
{
}

void
HtmlDisplay_Idl::Create_PackageList()
{
}

void
HtmlDisplay_Idl::Create_HelpFile()
{
}

void
HtmlDisplay_Idl::Create_CssFile()
{
    Cout() << "\nCreate css file ..." << Endl();

    pCurPageEnv->Goto_Directory( pCurPageEnv->OutputTree().Root(), true );
    pCurPageEnv->Set_CurFile( C_sCssFilename_Idl );

    StreamLock
        slCurFilePath(700);
    pCurPageEnv->Get_CurFilePath(slCurFilePath());

    csv::File
        aCssFile(slCurFilePath().c_str(), csv::CFM_CREATE);
    csv::OpenCloseGuard
        aOpenGuard(aCssFile);
    if (NOT aOpenGuard)
    {
        Cerr() << "Can't create file " << slCurFilePath().c_str() << "." << Endl();
        return;
    }

    aCssFile.write("/*      Autodoc css file for IDL documentation      */\n\n\n");
    aCssFile.write(pCurPageEnv->Layout().CssStyle());
    aCssFile.write("\n\n\n");
    aCssFile.write(pCurPageEnv->Layout().CssStylesExplanation());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
