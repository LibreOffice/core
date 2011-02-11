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

#include "oox/xls/connectionsfragment.hxx"

#include "oox/helper/attributelist.hxx"
#include "oox/xls/biffhelper.hxx"
#include "oox/xls/connectionsbuffer.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::oox::core;

using ::rtl::OUString;

// ============================================================================

ConnectionContext::ConnectionContext( WorkbookFragmentBase& rParent, Connection& rConnection ) :
    WorkbookContextBase( rParent ),
    mrConnection( rConnection )
{
}

ContextHandlerRef ConnectionContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( connection ):
            if( nElement == XLS_TOKEN( webPr ) )
            {
                mrConnection.importWebPr( rAttribs );
                return this;
            }
        break;

        case XLS_TOKEN( webPr ):
            if( nElement == XLS_TOKEN( tables ) )
            {
                mrConnection.importTables( rAttribs );
                return this;
            }
        break;

        case XLS_TOKEN( tables ):
            mrConnection.importTable( rAttribs, nElement );
        break;
    }
    return 0;
}

void ConnectionContext::onStartElement( const AttributeList& rAttribs )
{
    if( getCurrentElement() == XLS_TOKEN( connection ) )
        mrConnection.importConnection( rAttribs );
}

ContextHandlerRef ConnectionContext::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case BIFF12_ID_CONNECTION:
            if( nRecId == BIFF12_ID_WEBPR )
            {
                mrConnection.importWebPr( rStrm );
                return this;
            }
        break;

        case BIFF12_ID_WEBPR:
            if( nRecId == BIFF12_ID_WEBPRTABLES )
            {
                mrConnection.importWebPrTables( rStrm );
                return this;
            }
        break;

        case BIFF12_ID_WEBPRTABLES:
            mrConnection.importWebPrTable( rStrm, nRecId );
        break;
    }
    return 0;
}

void ConnectionContext::onStartRecord( SequenceInputStream& rStrm )
{
    if( getCurrentElement() == BIFF12_ID_CONNECTION )
        mrConnection.importConnection( rStrm );
}

// ============================================================================

ConnectionsFragment::ConnectionsFragment( const WorkbookHelper& rHelper, const OUString& rFragmentPath ) :
    WorkbookFragmentBase( rHelper, rFragmentPath )
{
}

ContextHandlerRef ConnectionsFragment::onCreateContext( sal_Int32 nElement, const AttributeList& /*rAttribs*/ )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nElement == XLS_TOKEN( connections ) )
                return this;
        break;

        case XLS_TOKEN( connections ):
            if( nElement == XLS_TOKEN( connection ) )
                return new ConnectionContext( *this, getConnections().createConnection() );
        break;
    }
    return 0;
}

ContextHandlerRef ConnectionsFragment::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& /*rStrm*/ )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nRecId == BIFF12_ID_CONNECTIONS )
                return this;
        break;

        case BIFF12_ID_CONNECTIONS:
            if( nRecId == BIFF12_ID_CONNECTION )
                return new ConnectionContext( *this, getConnections().createConnection() );
        break;
    }
    return 0;
}

const RecordInfo* ConnectionsFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { BIFF12_ID_CONNECTIONS,    BIFF12_ID_CONNECTIONS + 1   },
        { BIFF12_ID_CONNECTION,     BIFF12_ID_CONNECTION + 1    },
        { BIFF12_ID_WEBPR,          BIFF12_ID_WEBPR + 1         },
        { BIFF12_ID_WEBPRTABLES,    BIFF12_ID_WEBPRTABLES + 1   },
        { -1,                       -1                          }
    };
    return spRecInfos;
}

void ConnectionsFragment::finalizeImport()
{
    getConnections().finalizeImport();
}

// ============================================================================

} // namespace xls
} // namespace oox
