/*************************************************************************
 *
 *  $RCSfile: dsply_op.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:23 $
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
#include "dsply_op.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/template/tpltools.hxx>
#include <ary/cpp/c_disply.hxx>
#include <ary/cpp/crog_grp.hxx>
#include <ary/cpp/c_funct.hxx>
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
OperationsDisplay::Display_Function( const ary::cpp::Function & i_rData )
{
    if ( Ce_IsInternal(i_rData) )
        return;

    PageDisplay & rPage = FindPage_for( i_rData );

    csi::xml::Element & rOut = rPage.CurOut();
    Display_SglOperation( rOut, i_rData );
}

void
OperationsDisplay::PrepareForGlobals()
{
    if (pClassMembersDisplay)
        pClassMembersDisplay = 0;

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

const ary::DisplayGate *
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
            pFound = csv::value_from_map( aMap_GlobalFunctionsDisplay, nSourceFile );
    if ( pFound == 0 )
    {
         pFound = new PageDisplay( *pEnv );
        const ary::cpp::FileGroup *
                pFgr = pEnv->Gate().RoGroups().Search_FileGroup( nSourceFile );
        csv_assert( pFgr != 0 );
        pFound->Setup_OperationsFile_for( *pFgr );
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
    fTitle( aDocu.Term(), i_rData.LocalName(), i_rData.Signature() );

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
    pDocuShow->Assign_Out( aDocu.Def() );
    i_rData.Info().StoreAt( *pDocuShow );
    pDocuShow->Unassign_Out();

    rOut << new html::HorizontalLine;
}


