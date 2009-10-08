/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tablefragment.cxx,v $
 * $Revision: 1.4 $
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

#include "oox/xls/scenariocontext.hxx"
#include "oox/xls/scenariobuffer.hxx"

using ::oox::core::ContextHandlerRef;

namespace oox {
namespace xls {

// ============================================================================

OoxScenarioContext::OoxScenarioContext( OoxWorksheetContextBase& rParent, SheetScenarios& rSheetScenarios ) :
    OoxWorksheetContextBase( rParent ),
    mrScenario( rSheetScenarios.createScenario() )
{
}

ContextHandlerRef OoxScenarioContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( scenario ):
            if( nElement == XLS_TOKEN( inputCells ) ) mrScenario.importInputCells( rAttribs );
        break;
    }
    return 0;
}

void OoxScenarioContext::onStartElement( const AttributeList& rAttribs )
{
    if( isRootElement() )
        mrScenario.importScenario( rAttribs );
}

ContextHandlerRef OoxScenarioContext::onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case OOBIN_ID_SCENARIO:
            if( nRecId == OOBIN_ID_INPUTCELLS ) mrScenario.importInputCells( rStrm );
        break;
    }
    return 0;
}

void OoxScenarioContext::onStartRecord( RecordInputStream& rStrm )
{
    if( isRootElement() )
        mrScenario.importScenario( rStrm );
}

// ============================================================================

OoxScenariosContext::OoxScenariosContext( OoxWorksheetFragmentBase& rFragment ) :
    OoxWorksheetContextBase( rFragment ),
    mrSheetScenarios( getScenarios().createSheetScenarios( getSheetIndex() ) )
{
}

ContextHandlerRef OoxScenariosContext::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( scenarios ):
            if( nElement == XLS_TOKEN( scenario ) ) return new OoxScenarioContext( *this, mrSheetScenarios );
        break;
    }
    return 0;
}

void OoxScenariosContext::onStartElement( const AttributeList& rAttribs )
{
    if( isRootElement() )
        mrSheetScenarios.importScenarios( rAttribs );
}

ContextHandlerRef OoxScenariosContext::onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& )
{
    switch( getCurrentElement() )
    {
        case OOBIN_ID_SCENARIOS:
            if( nRecId == OOBIN_ID_SCENARIO ) return new OoxScenarioContext( *this, mrSheetScenarios );
        break;
    }
    return 0;
}

void OoxScenariosContext::onStartRecord( RecordInputStream& rStrm )
{
    if( isRootElement() )
        mrSheetScenarios.importScenarios( rStrm );
}

// ============================================================================

} // namespace xls
} // namespace oox

