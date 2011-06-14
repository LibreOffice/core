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

#include "oox/xls/scenariocontext.hxx"

#include "oox/xls/scenariobuffer.hxx"

namespace oox {
namespace xls {

// ============================================================================

using ::oox::core::ContextHandlerRef;

// ============================================================================

ScenarioContext::ScenarioContext( WorksheetContextBase& rParent, SheetScenarios& rSheetScenarios ) :
    WorksheetContextBase( rParent ),
    mrScenario( rSheetScenarios.createScenario() )
{
}

ContextHandlerRef ScenarioContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( scenario ):
            if( nElement == XLS_TOKEN( inputCells ) ) mrScenario.importInputCells( rAttribs );
        break;
    }
    return 0;
}

void ScenarioContext::onStartElement( const AttributeList& rAttribs )
{
    if( isRootElement() )
        mrScenario.importScenario( rAttribs );
}

ContextHandlerRef ScenarioContext::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case BIFF12_ID_SCENARIO:
            if( nRecId == BIFF12_ID_INPUTCELLS ) mrScenario.importInputCells( rStrm );
        break;
    }
    return 0;
}

void ScenarioContext::onStartRecord( SequenceInputStream& rStrm )
{
    if( isRootElement() )
        mrScenario.importScenario( rStrm );
}

// ============================================================================

ScenariosContext::ScenariosContext( WorksheetFragmentBase& rFragment ) :
    WorksheetContextBase( rFragment ),
    mrSheetScenarios( getScenarios().createSheetScenarios( getSheetIndex() ) )
{
}

ContextHandlerRef ScenariosContext::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( scenarios ):
            if( nElement == XLS_TOKEN( scenario ) ) return new ScenarioContext( *this, mrSheetScenarios );
        break;
    }
    return 0;
}

void ScenariosContext::onStartElement( const AttributeList& rAttribs )
{
    if( isRootElement() )
        mrSheetScenarios.importScenarios( rAttribs );
}

ContextHandlerRef ScenariosContext::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& )
{
    switch( getCurrentElement() )
    {
        case BIFF12_ID_SCENARIOS:
            if( nRecId == BIFF12_ID_SCENARIO ) return new ScenarioContext( *this, mrSheetScenarios );
        break;
    }
    return 0;
}

void ScenariosContext::onStartRecord( SequenceInputStream& rStrm )
{
    if( isRootElement() )
        mrSheetScenarios.importScenarios( rStrm );
}

// ============================================================================

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */