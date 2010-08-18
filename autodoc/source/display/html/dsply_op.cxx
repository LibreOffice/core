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
#include "dsply_op.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/tpl/tpltools.hxx>
#include <ary/cpp/c_funct.hxx>
#include <ary/cpp/c_gate.hxx>
#include <ary/loc/locp_le.hxx>
#include <udm/html/htmlitem.hxx>
#include "hd_docu.hxx"
#include "hdimpl.hxx"
#include "html_kit.hxx"
#include "opageenv.hxx"
#include "pagemake.hxx"

using namespace csi;
using namespace adcdisp;




OperationsDisplay::OperationsDisplay( OuputPage_Environment & io_rEnv )
    :   // aMap_GlobalFunctionsDisplay,
        // pClassMembersDisplay,
        pEnv( &io_rEnv ),
        pDocuShow( new Docu_Display(io_rEnv) )
{
}

OperationsDisplay::~OperationsDisplay()
{
    csv::erase_map_of_heap_ptrs( aMap_GlobalFunctionsDisplay );
}

void
OperationsDisplay::PrepareForStdMembers()
{
    csv::erase_map_of_heap_ptrs( aMap_GlobalFunctionsDisplay );

    pClassMembersDisplay = new PageDisplay(*pEnv);
    const ary::cpp::Class * pClass = pEnv->CurClass();
    csv_assert( pClass != 0 );
    pClassMembersDisplay->Setup_OperationsFile_for(*pClass);
}

void
OperationsDisplay::PrepareForStaticMembers()
{
    // Doesn't need to do anything yet.
}

void
OperationsDisplay::Create_Files()
{
    if (pClassMembersDisplay)
        pClassMembersDisplay->Create_File();
    else
    {
         for ( Map_FileId2PagePtr::const_iterator it = aMap_GlobalFunctionsDisplay.begin();
              it != aMap_GlobalFunctionsDisplay.end();
              ++it )
        {
             (*it).second->Create_File();
        }
    }
}

void
OperationsDisplay::do_Process( const ary::cpp::Function & i_rData )
{
    if ( Ce_IsInternal(i_rData) )
        return;

    PageDisplay & rPage = FindPage_for( i_rData );

    csi::xml::Element & rOut = rPage.CurOut();
    Display_SglOperation( rOut, i_rData );
}

const ary::cpp::Gate *
OperationsDisplay::inq_Get_ReFinder() const
{
    return & pEnv->Gate();
}

PageDisplay &
OperationsDisplay::FindPage_for( const ary::cpp::Function & i_rData )
{
    if ( pClassMembersDisplay )
        return *pClassMembersDisplay;

    SourceFileId
            nSourceFile = i_rData.Location();
    PageDisplay *
            pFound = csv::value_from_map( aMap_GlobalFunctionsDisplay, nSourceFile, (PageDisplay*)0 );
    if ( pFound == 0 )
    {
         pFound = new PageDisplay( *pEnv );
        const ary::loc::File &
                rFile = pEnv->Gate().Locations().Find_File( nSourceFile );
        pFound->Setup_OperationsFile_for(rFile);
        aMap_GlobalFunctionsDisplay[nSourceFile] = pFound;
    }

    return *pFound;
}

void
OperationsDisplay::Display_SglOperation( csi::xml::Element &        rOut,
                                         const ary::cpp::Function & i_rData )
{
    adcdisp::ExplanationList aDocu(rOut, true);
    aDocu.AddEntry( 0 );


    adcdisp::OperationTitle fTitle;
    fTitle( aDocu.Term(),
            i_rData.LocalName(),
            i_rData.CeId(),
            pEnv->Gate() );

    // Syntax
    adcdisp::ExplanationList aSyntaxHeader(aDocu.Def());
    aSyntaxHeader.AddEntry( 0, "simple" );
    csi::xml::Element & rHeader = aSyntaxHeader.Term();

    adcdisp::ParameterTable
        aParams( aSyntaxHeader.Def() );

    if (i_rData.TemplateParameters().size() > 0)
    {
        TemplateClause fTemplateClause;
        fTemplateClause( rHeader, i_rData.TemplateParameters() );
        rHeader << new html::LineBreak;
    }
    if ( i_rData.Flags().IsExternC() )
    {
         rHeader
                << "extern \"C\""
                << new html::LineBreak;
    }

    bool bConst = false;
    bool bVolatile = false;
    WriteOut_LinkedFunctionText( rHeader, aParams, i_rData, *pEnv,
                                 &bConst, &bVolatile );
    aDocu.Def() << new html::LineBreak;

    // Flags
    aDocu.AddEntry_NoTerm();
    adcdisp::FlagTable
        aFlags( aDocu.Def(), 8 );

    const ary::cpp::FunctionFlags &
        rFFlags = i_rData.Flags();
    aFlags.SetColumn( 0, "virtual",
                      i_rData.Virtuality() != ary::cpp::VIRTUAL_none );
    aFlags.SetColumn( 1, "abstract",
                      i_rData.Virtuality() == ary::cpp::VIRTUAL_abstract );
    aFlags.SetColumn( 2, "const", bConst );
    aFlags.SetColumn( 3, "volatile", bVolatile );
    aFlags.SetColumn( 4, "template",
                      i_rData.TemplateParameters().size() > 0 );
    aFlags.SetColumn( 5, "static",
                      rFFlags.IsStaticLocal() OR rFFlags.IsStaticMember() );
    aFlags.SetColumn( 6, "inline",
                      rFFlags.IsInline() );
    aFlags.SetColumn( 7, "C-linkage",
                      rFFlags.IsExternC() );
    aDocu.Def() << new html::LineBreak;

    // Docu
    aDocu.AddEntry_NoTerm();
    pDocuShow->Assign_Out(aDocu.Def());
    pDocuShow->Process(i_rData.Docu());
    pDocuShow->Unassign_Out();

    rOut << new html::HorizontalLine;
}
