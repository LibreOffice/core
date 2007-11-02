/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pm_aldef.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:31:08 $
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
#include <cosv/tpl/tpltools.hxx>
#include <ary/cpp/c_gate.hxx>
#include <ary/cpp/c_define.hxx>
#include <ary/cpp/c_macro.hxx>
#include <ary/cpp/cp_def.hxx>
#include <ary/loc/loc_file.hxx>
#include <ary/loc/locp_le.hxx>
#include <ary/getncast.hxx>
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

    ary::cpp::DefsResultList
        aAllDefines =  Env().Gate().Defs().AllDefines();
    ary::cpp::DefsConstIterator
        itEnd = aAllDefines.end();

    if (aAllDefines.begin() != itEnd)
    {
        for ( ary::cpp::DefsConstIterator it = aAllDefines.begin();
              it != itEnd;
              ++it )
        {
            Write_Define(*it);
        }
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

    ary::cpp::DefsResultList
        aAllMacros =  Env().Gate().Defs().AllMacros();
    ary::cpp::DefsConstIterator
        itEnd = aAllMacros.end();

    if (aAllMacros.begin() != itEnd)
    {
        for ( ary::cpp::DefsConstIterator it = aAllMacros.begin();
              it != itEnd;
              ++it )
        {
            Write_Macro(*it);
        }
    }
    else
    {
        CurOut() << "None.";
    }

    CurOut() << new HorizontalLine;
}

void
PageMaker_AllDefs::Write_Define(De_id  i_nId)
{
    csv_assert( ary::is_type<ary::cpp::Define>( Env().Gate().Defs().Find_Def(i_nId)) );
    const ary::cpp::Define &
        rDef = static_cast< const ary::cpp::Define& >( Env().Gate().Defs().Find_Def(i_nId) );

    CurOut() << new html::HorizontalLine;

    adcdisp::ExplanationList aDocu( CurOut(), true );
    aDocu.AddEntry();

    aDocu.Term()
        >> *new html::Label( rDef.LocalName() )
            << " ";
    aDocu.Term()
        << rDef.LocalName();

    Write_DefsDocu( aDocu.Def(), rDef );
}

void
PageMaker_AllDefs::Write_Macro(De_id  i_nId)
{
    csv_assert( Env().Gate().Defs().Find_Def(i_nId).AryClass() == ary::cpp::Macro::class_id );
    const ary::cpp::Macro &
        rDef = static_cast< const ary::cpp::Macro& >( Env().Gate().Defs().Find_Def(i_nId) );

    CurOut() << new html::HorizontalLine;

    adcdisp::ExplanationList aDocu( CurOut(), true );
    aDocu.AddEntry();

    aDocu.Term()
        >> *new html::Label( rDef.LocalName() )
            << " ";
    aDocu.Term()
        << rDef.LocalName()
        << "(";
    WriteOut_TokenList( aDocu.Term(), rDef.Params(), ", " );
    aDocu.Term()
        << ")";

    Write_DefsDocu( aDocu.Def(), rDef );
}


void
PageMaker_AllDefs::Write_DefsDocu( csi::xml::Element &              o_rOut,
                                   const ary::cpp::DefineEntity &  i_rTextReplacement )
{
    if ( i_rTextReplacement.DefinitionText().size() > 0 )
    {
        EraseLeadingSpace( *const_cast< String * >(
                                &(*i_rTextReplacement.DefinitionText().begin())
                         ) );
    }

    adcdisp::ExplanationTable rList( o_rOut );

    rList.AddEntry( "Defined As" );
    WriteOut_TokenList( rList.Def(), i_rTextReplacement.DefinitionText(), " " );

    const ary::loc::File &
        rFile = Env().Gate().Locations().Find_File( i_rTextReplacement.Location() );
    rList.AddEntry( "In File" );
    rList.Def() << rFile.LocalName();

    ShowDocu_On( o_rOut, *pDocuDisplay, i_rTextReplacement );
}
