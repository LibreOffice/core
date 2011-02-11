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
