/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: chd_udk2.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 13:50:29 $
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
#include <html/chd_udk2.hxx>


// NOT FULLY DEFINED SERVICES
#include <cosv/template/tpltools.hxx>

#include <ary/ary_disp.hxx>
#include <ary/project.hxx>
#include <ary/ceslot.hxx>
#include <ary/cpp/c_disply.hxx>
#include <ary/cpp/crog_grp.hxx>
#include <ary/cpp/cg_proj.hxx>
#include <ary/cpp/c_namesp.hxx>

#include "dsply_cl.hxx"
#include "dsply_da.hxx"
#include "dsply_op.hxx"
#include "opageenv.hxx"
#include "outfile.hxx"
#include "pagemake.hxx"





namespace
{


//********************      S_ProjectListFiller     ********************//
struct  S_ProjectListFiller : public ary::Display
{
  public:
                        S_ProjectListFiller(
                            std::vector< const ary::cpp::ProjectGroup * > &
                                                o_rList,
                            const ary::cpp::DisplayGate &
                                                i_rAryGate );
                        ~S_ProjectListFiller();

    virtual void        DisplaySlot_CppProject(
                            ary::Gid            i_nId );
    void                Fill_FromNameList(
                            const StringVector &
                                                i_sNameList );
  private:
    std::vector< const ary::cpp::ProjectGroup * > *
                        pList;
    const ary::cpp::RoGate_Groups *
                        pGroupGate;
};


S_ProjectListFiller::S_ProjectListFiller(
                    std::vector< const ary::cpp::ProjectGroup * > & o_rList,
                    const ary::cpp::DisplayGate &                   i_rAryGate )
    :   pList(&o_rList),
        pGroupGate(&i_rAryGate.RoGroups())
{
}

S_ProjectListFiller::~S_ProjectListFiller()
{
}

void
S_ProjectListFiller::DisplaySlot_CppProject( ary::Gid i_nId )
{
    const ary::cpp::ProjectGroup *
            p = pGroupGate->Find_ProjectGroup( i_nId );
    if ( p != 0 )
        pList->push_back( p );
}

void
S_ProjectListFiller::Fill_FromNameList( const StringVector & i_sNameList )
{
    for ( StringVector::const_iterator it = i_sNameList.begin();
          it != i_sNameList.end();
          ++it )
    {
        const ary::cpp::ProjectGroup * p = pGroupGate->Search_ProjectGroup( *it );
        if ( p != 0 )
            pList->push_back( p );
    }    // end for
}

}   // anonymus namespace


//********************      CppHtmlDisplay_Udk2     ********************//


CppHtmlDisplay_Udk2::CppHtmlDisplay_Udk2()
    :   pCurPageEnv(0),
        // aProjectList,
        bFilterByProjectList(false)
{
}

CppHtmlDisplay_Udk2::~CppHtmlDisplay_Udk2()
{
}

void
CppHtmlDisplay_Udk2::do_Run( const char *                      i_sOutputDirectory,
                             const ary::cpp::DisplayGate &     i_rAryGate,
                             const display::CorporateFrame &   i_rLayout,
                             const StringVector *    i_pProjectList )
{
    SetRunData( i_sOutputDirectory, i_rAryGate, i_rLayout, i_pProjectList );

    Create_Css_File();
    Create_Overview_File();
    Create_Help_File();
    Create_AllDefs_File();

    CreateFiles_InSubTree_Projects();
    CreateFiles_InSubTree_Namespaces();
    CreateFiles_InSubTree_Index();
}

void
CppHtmlDisplay_Udk2::SetRunData( const char *                       i_sOutputDirectory,
                                 const ary::cpp::DisplayGate &      i_rAryGate,
                                 const display::CorporateFrame &    i_rLayout,
                                 const StringVector *     i_pProjectList )
{
    csv::ploc::Path aOutputDir( i_sOutputDirectory, true );

    pCurPageEnv = new OuputPage_Environment( aOutputDir, i_rAryGate, i_rLayout );

    csv::erase_container( aProjectList );
    S_ProjectListFiller aPrjGetter( aProjectList, i_rAryGate );

    if ( i_pProjectList != 0 )
    {
        aPrjGetter.Fill_FromNameList( *i_pProjectList );

        bFilterByProjectList = true;
    }
    else
    {
        const ary::TopProject &
                rTop            = i_rAryGate.RoGroups().Get_TopProject();
        ary::Slot_AutoPtr
                pSlot_Projects( rTop.Create_Slot(ary::TopProject::SLOT_CppLocationBases) );
        pSlot_Projects->StoreAt( aPrjGetter );

        bFilterByProjectList = false;
    }
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
CppHtmlDisplay_Udk2::CreateFiles_InSubTree_Projects()
{
    Cout() << "\nCreate files in subtree projects" << Endl();
    Cout() << Endl();
}

void
CppHtmlDisplay_Udk2::CreateFiles_InSubTree_Namespaces()
{
    Cout() << "\nCreate files in subtree namespaces" << Endl();

    const ary::cpp::Namespace & rGlobalNsp = Gate().GlobalNamespace();

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
    if ( NOT CheckFilters(i_rNsp) )
        return;

    pCurPageEnv->MoveDir_Down2( i_rNsp );
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

const ary::cpp::DisplayGate &
CppHtmlDisplay_Udk2::Gate() const
{
    return pCurPageEnv->Gate();
}

