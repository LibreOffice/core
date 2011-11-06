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
