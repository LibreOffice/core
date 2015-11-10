/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "scenariocontext.hxx"

#include "scenariobuffer.hxx"

namespace oox {
namespace xls {

using ::oox::core::ContextHandlerRef;

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
    return nullptr;
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
    return nullptr;
}

void ScenarioContext::onStartRecord( SequenceInputStream& rStrm )
{
    if( isRootElement() )
        mrScenario.importScenario( rStrm );
}

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
    return nullptr;
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
    return nullptr;
}

void ScenariosContext::onStartRecord( SequenceInputStream& rStrm )
{
    if( isRootElement() )
        mrSheetScenarios.importScenarios( rStrm );
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
