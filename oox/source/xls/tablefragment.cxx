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
