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
#include <html/chd_udk2.hxx>


// NOT FULLY DEFINED SERVICES
#include <cosv/tpl/tpltools.hxx>
#include <ary/ary_disp.hxx>
#include <ary/ceslot.hxx>
#include <ary/cpp/c_gate.hxx>
#include <ary/cpp/c_namesp.hxx>
#include <ary/cpp/cp_ce.hxx>

#include "dsply_cl.hxx"
#include "dsply_da.hxx"
#include "dsply_op.hxx"
#include "opageenv.hxx"
#include "outfile.hxx"
#include "pagemake.hxx"



//********************      CppHtmlDisplay_Udk2     ********************//


CppHtmlDisplay_Udk2::CppHtmlDisplay_Udk2()
    :   pCurPageEnv(0)
{
}

CppHtmlDisplay_Udk2::~CppHtmlDisplay_Udk2()
{
}

void
CppHtmlDisplay_Udk2::do_Run( const char *                      i_sOutputDirectory,
                             const ary::cpp::Gate &            i_rAryGate,
                             const display::CorporateFrame &   i_rLayout )
{
    SetRunData( i_sOutputDirectory, i_rAryGate, i_rLayout );

    Create_Css_File();
    Create_Overview_File();
    Create_Help_File();
    Create_AllDefs_File();

    CreateFiles_InSubTree_Namespaces();
    CreateFiles_InSubTree_Index();
}

void
CppHtmlDisplay_Udk2::SetRunData( const char *                       i_sOutputDirectory,
                                 const ary::cpp::Gate &             i_rAryGate,
                                 const display::CorporateFrame &    i_rLayout )
{
    csv::ploc::Path aOutputDir( i_sOutputDirectory, true );
    pCurPageEnv = new OuputPage_Environment( aOutputDir, i_rAryGate, i_rLayout );
}

void
CppHtmlDisplay_Udk2::Create_Css_File()
{
    pCurPageEnv->MoveDir_2Root();
    pCurPageEnv->SetFile_Css();
    HtmlDocuFile::WriteCssFile(pCurPageEnv->CurPath());
}

void
CppHtmlDisplay_Udk2::Create_Overview_File()
{
    pCurPageEnv->MoveDir_2Root();
    PageDisplay aPageMaker( *pCurPageEnv );
    aPageMaker.Create_OverviewFile();
}

void
CppHtmlDisplay_Udk2::Create_Help_File()
{
    PageDisplay aPageMaker( *pCurPageEnv );
    aPageMaker.Create_HelpFile();
}

void
CppHtmlDisplay_Udk2::Create_AllDefs_File()
{
    PageDisplay aPageMaker( *pCurPageEnv );
    aPageMaker.Create_AllDefsFile();
}

void
CppHtmlDisplay_Udk2::CreateFiles_InSubTree_Namespaces()
{
    Cout() << "\nCreate files in subtree namespaces" << Endl();

    const ary::cpp::Namespace &
        rGlobalNsp = Gate().Ces().GlobalNamespace();

    RecursiveDisplay_Namespace(rGlobalNsp);
    Cout() << Endl();
}

void
CppHtmlDisplay_Udk2::CreateFiles_InSubTree_Index()
{
    Cout() << "\nCreate files in subtree index" << Endl();
    Cout() << Endl();

    PageDisplay aPageMaker( *pCurPageEnv );
    aPageMaker.Create_IndexFiles();
}

void
CppHtmlDisplay_Udk2::RecursiveDisplay_Namespace( const ary::cpp::Namespace & i_rNsp )
{
    if (i_rNsp.Owner().IsValid())
        pCurPageEnv->MoveDir_Down2( i_rNsp );
    else
        pCurPageEnv->MoveDir_2Names();
    DisplayFiles_InNamespace( i_rNsp );

    typedef std::vector< const ary::cpp::Namespace* > NspList;
    NspList     aSubNspList;
    i_rNsp.Get_SubNamespaces( aSubNspList );
    for ( NspList::const_iterator it = aSubNspList.begin();
          it != aSubNspList.end();
          ++it )
    {
        RecursiveDisplay_Namespace( *(*it) );
    }   // end for

    pCurPageEnv->MoveDir_Up();
}

void
CppHtmlDisplay_Udk2::DisplayFiles_InNamespace( const ary::cpp::Namespace & i_rNsp )
{
    PageDisplay aPageMaker( *pCurPageEnv );

    ary::Slot_AutoPtr pSlot;

    // Namespace
    aPageMaker.Create_NamespaceFile();

    // Classes
    ClassDisplayer aClassDisplayer( *pCurPageEnv );
    DisplaySlot( aClassDisplayer, i_rNsp, ary::cpp::Namespace::SLOT_Classes );

    // Enums
    DisplaySlot( aPageMaker, i_rNsp, ary::cpp::Namespace::SLOT_Enums );

    // Typedefs
    DisplaySlot( aPageMaker, i_rNsp, ary::cpp::Namespace::SLOT_Typedefs );

    // Operations
    OperationsDisplay aOperationsDisplayer( *pCurPageEnv );
    DisplaySlot( aOperationsDisplayer, i_rNsp, ary::cpp::Namespace::SLOT_Operations );
    aOperationsDisplayer.Create_Files();

    // Data
    DataDisplay aDataDisplayer( *pCurPageEnv );

    aDataDisplayer.PrepareForConstants();
    DisplaySlot( aDataDisplayer, i_rNsp, ary::cpp::Namespace::SLOT_Constants );

    aDataDisplayer.PrepareForVariables();
    DisplaySlot( aDataDisplayer, i_rNsp, ary::cpp::Namespace::SLOT_Variables );

    aDataDisplayer.Create_Files();
}

const ary::cpp::Gate &
CppHtmlDisplay_Udk2::Gate() const
{
    return pCurPageEnv->Gate();
}
