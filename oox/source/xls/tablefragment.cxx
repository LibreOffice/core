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



#include "oox/xls/tablefragment.hxx"

#include "oox/xls/autofilterbuffer.hxx"
#include "oox/xls/autofiltercontext.hxx"
#include "oox/xls/tablebuffer.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::oox::core;

using ::rtl::OUString;

// ============================================================================

TableFragment::TableFragment( const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    WorksheetFragmentBase( rHelper, rFragmentPath ),
    mrTable( getTables().createTable() )
{
}

ContextHandlerRef TableFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nElement == XLS_TOKEN( table ) )
            {
                mrTable.importTable( rAttribs, getSheetIndex() );
                return this;
            }
        break;
        case XLS_TOKEN( table ):
            if( nElement == XLS_TOKEN( autoFilter ) )
                return new AutoFilterContext( *this, mrTable.createAutoFilter() );
        break;
    }
    return 0;
}

ContextHandlerRef TableFragment::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nRecId == BIFF12_ID_TABLE )
            {
                mrTable.importTable( rStrm, getSheetIndex() );
                return this;
            }
        break;
        case BIFF12_ID_TABLE:
            if( nRecId == BIFF12_ID_AUTOFILTER )
                return new AutoFilterContext( *this, mrTable.createAutoFilter() );
        break;
    }
    return 0;
}

const RecordInfo* TableFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { BIFF12_ID_AUTOFILTER,         BIFF12_ID_AUTOFILTER + 1        },
        { BIFF12_ID_CUSTOMFILTERS,      BIFF12_ID_CUSTOMFILTERS + 1     },
        { BIFF12_ID_DISCRETEFILTERS,    BIFF12_ID_DISCRETEFILTERS + 1   },
        { BIFF12_ID_FILTERCOLUMN,       BIFF12_ID_FILTERCOLUMN + 1      },
        { BIFF12_ID_TABLE,              BIFF12_ID_TABLE + 1             },
        { -1,                           -1                              }
    };
    return spRecInfos;
}

// ============================================================================

} // namespace xls
} // namespace oox
