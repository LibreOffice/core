/*************************************************************************
 *
 *  $RCSfile: hi_display.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-11-01 17:14:52 $
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
#include <idl/hi_display.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_ce.hxx>
#include <ary/idl/i_module.hxx>
#include <toolkit/out_tree.hxx>
#include "hi_ary.hxx"
#include "hi_env.hxx"
#include "hi_main.hxx"


inline bool
HtmlDisplay_Idl::IsModule( const ary::idl::CodeEntity & i_ce ) const
{
     return i_ce.ClassId() == ary::idl::Module::class_id;
}

inline const ary::idl::Module &
HtmlDisplay_Idl::Module_Cast( const ary::idl::CodeEntity & i_ce ) const
{
     return static_cast< const ary::idl::Module& >(i_ce);
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
    i_module.Visit(*pMainDisplay);

    Dyn_CeIterator  aMembers;
    i_module.Get_Names(aMembers);

    for ( CeIterator & iter = *aMembers;
          iter;
          ++iter )
    {
        const ary::idl::CodeEntity &
            rCe = pCurPageEnv->Data().Find_Ce(*iter);

        if ( NOT IsModule(rCe) )
            rCe.Visit(*pMainDisplay);
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
#if 0
    Cout() << "\nCreate package list ..." << std::flush;

    pCurPageEnv->CurPosition() = pCurPageEnv->OutputTree().Root();

    // KORR
    // ...

    Cout() << " done." << Endl();
#endif // 0
}

void
HtmlDisplay_Idl::Create_HelpFile()
{
}


