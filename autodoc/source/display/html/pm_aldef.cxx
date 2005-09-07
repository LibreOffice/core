/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pm_aldef.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:33:57 $
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
#include "pm_aldef.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/template/tpltools.hxx>
#include <ary/cpp/c_disply.hxx>
#include <ary/cpp/c_define.hxx>
#include <ary/cpp/c_macro.hxx>
#include <ary/loc/l_rogate.hxx>
#include <ary/loc/loc_file.hxx>
#include "hd_docu.hxx"
#include "html_kit.hxx"
#include "navibar.hxx"
#include "opageenv.hxx"
#include "pagemake.hxx"
#include "strconst.hxx"


using namespace csi;
using csi::html::HorizontalLine;
using csi::html::Link;
using csi::html::Label;
using csi::html::AlignAttr;



PageMaker_AllDefs::PageMaker_AllDefs( PageDisplay & io_rPage )
    :   SpecializedPageMaker(io_rPage),
        pDocuDisplay( new Docu_Display(io_rPage.Env()) ),
        pNavi(0)
{
}

PageMaker_AllDefs::~PageMaker_AllDefs()
{
}

void
PageMaker_AllDefs::MakePage()
{
    pNavi = new NavigationBar( Env(), NavigationBar::LOC_AllDefs );
    Write_NavBar();

    Write_TopArea();

    Write_DefinesList();
    Write_MacrosList();

    pNavi->Write_SubRows();
}

void
PageMaker_AllDefs::Write_NavBar()
{
    pNavi->MakeSubRow( "" );
    pNavi->AddItem( "Defines", "defines", true );
    pNavi->AddItem( "Macros", "macros", true );
    pNavi->Write( CurOut() );
    CurOut() << new HorizontalLine;
}

void
PageMaker_AllDefs::Write_TopArea()
{
    adcdisp::PageTitle_Std fTitle;
    fTitle( CurOut(), "Defines and ", "Macros" );

    CurOut() << new HorizontalLine;
}

void
PageMaker_AllDefs::Write_DocuArea()
{
    // Not needed.
}

void
PageMaker_AllDefs::Write_DefinesList()
{
    CurOut()
        << new html::LineBreak
        << new html::LineBreak
        >> *new xml::AnElement("div")
            << new html::ClassAttr("define")
            >> *new html::Label("defines")
                >> *new html::Headline(3)
                    << "Defines";

    List_Ids aAllDefines;
    uintt nCount = Env().Gate().RoDefines().Get_AllDefines(aAllDefines);

    if (nCount > 0)
    {
        csv::call_for_each( aAllDefines,
                            this,
                            &PageMaker_AllDefs::Write_Define );
    }
    else
    {
        CurOut() << "None.";
    }

    CurOut() << new HorizontalLine;
}

void
PageMaker_AllDefs::Write_MacrosList()

{
    CurOut()
        << new html::LineBreak
        << new html::LineBreak
        >> *new xml::AnElement("div")
            << new html::ClassAttr("define")
            >> *new html::Label("macros")
                >> *new html::Headline(3)
                    << "Macros";

    List_Ids aAllMacros;
    uintt nCount = Env().Gate().RoDefines().Get_AllMacros(aAllMacros);

    if (nCount > 0)
    {
        csv::call_for_each(  aAllMacros,
                             this,
                             &PageMaker_AllDefs::Write_Macro );
    }
    else
    {
        CurOut() << "None.";
    }

    CurOut() << new HorizontalLine;
}

void
PageMaker_AllDefs::Write_Define( const ary::Rid & i_nId )
{
    const ary::cpp::Define *
        pDef = Env().Gate().RoDefines().Find_Define(i_nId);
    if (pDef == 0)
        return;

    CurOut() << new html::HorizontalLine;

    adcdisp::ExplanationList aDocu( CurOut(), true );
    aDocu.AddEntry();

    aDocu.Term()
        >> *new html::Label( pDef->DefinedName() )
            << " ";
    aDocu.Term()
        << pDef->DefinedName();

    Write_DefsDocu( aDocu.Def(), *pDef );
}

void
PageMaker_AllDefs::Write_Macro( const ary::Rid & i_nId )
{
    const ary::cpp::Macro *
        pDef = Env().Gate().RoDefines().Find_Macro(i_nId);
    if (pDef == 0)
        return;

    CurOut() << new html::HorizontalLine;

    adcdisp::ExplanationList aDocu( CurOut(), true );
    aDocu.AddEntry();

    aDocu.Term()
        >> *new html::Label( pDef->DefinedName() )
            << " ";
    aDocu.Term()
        << pDef->DefinedName()
        << "(";
    WriteOut_TokenList( aDocu.Term(), pDef->Params(), ", " );
    aDocu.Term()
        << ")";

    Write_DefsDocu( aDocu.Def(), *pDef );
}


void
PageMaker_AllDefs::Write_DefsDocu( csi::xml::Element &              o_rOut,
                                   const ary::cpp::CppDefinition &  i_rTextReplacement )
{
    if ( i_rTextReplacement.DefinitionText().size() > 0 )
    {
        EraseLeadingSpace( *const_cast< udmstri* >(
                                &(*i_rTextReplacement.DefinitionText().begin())
                         ) );
    }

    adcdisp::ExplanationTable rList( o_rOut );

    rList.AddEntry( "Defined As" );
    WriteOut_TokenList( rList.Def(), i_rTextReplacement.DefinitionText(), " " );

    ary::loc::SourceCodeFile *
        pFile = Env().Gate().RoLocations().Find_File( i_rTextReplacement.Location() );
    if (pFile != 0 )
    {
        rList.AddEntry( "In File" );
        rList.Def() << pFile->Name();
    }

    ShowDocu_On( o_rOut, *pDocuDisplay, i_rTextReplacement );
}


