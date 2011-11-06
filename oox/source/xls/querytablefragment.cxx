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



#include "oox/xls/querytablefragment.hxx"

#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/querytablebuffer.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::oox::core;

using ::rtl::OUString;

// ============================================================================

QueryTableFragment::QueryTableFragment( const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    WorksheetFragmentBase( rHelper, rFragmentPath ),
    mrQueryTable( getQueryTables().createQueryTable() )
{
}

ContextHandlerRef QueryTableFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nElement == XLS_TOKEN( queryTable ) )
                mrQueryTable.importQueryTable( rAttribs );
        break;
    }
    return 0;
}

ContextHandlerRef QueryTableFragment::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nRecId == BIFF12_ID_QUERYTABLE )
                mrQueryTable.importQueryTable( rStrm );
        break;
    }
    return 0;
}

const RecordInfo* QueryTableFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { BIFF12_ID_QUERYTABLE,         BIFF12_ID_QUERYTABLE + 1        },
        { BIFF12_ID_QUERYTABLEREFRESH,  BIFF12_ID_QUERYTABLEREFRESH + 1 },
        { -1,                           -1                              }
    };
    return spRecInfos;
}

// ============================================================================

BiffQueryTableContext::BiffQueryTableContext( const WorksheetHelper& rHelper ) :
    BiffWorksheetContextBase( rHelper ),
    mrQueryTable( getQueryTables().createQueryTable() )
{
}

void BiffQueryTableContext::importRecord( BiffInputStream& rStrm )
{
    switch( rStrm.getRecId() )
    {
        case BIFF_ID_QUERYTABLE:            mrQueryTable.importQueryTable( rStrm );         break;
        case BIFF_ID_QUERYTABLEREFRESH:     mrQueryTable.importQueryTableRefresh( rStrm );  break;
        case BIFF_ID_QUERYTABLESETTINGS:    mrQueryTable.importQueryTableSettings( rStrm ); break;
    }
}

// ============================================================================

} // namespace xls
} // namespace oox
