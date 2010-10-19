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

namespace oox {
namespace xls {

// ============================================================================

using ::rtl::OUString;
using ::oox::core::ContextHandlerRef;
using ::oox::core::RecordInfo;

// ============================================================================

ConnectionContext::ConnectionContext( WorkbookFragmentBase& rParent, const ConnectionRef& rxConnection ) :
    WorkbookContextBase( rParent ),
    mxConnection( rxConnection )
{
    OSL_ENSURE( mxConnection.get(), "ConnectionContext::ConnectionContext - missing connection" );
}

ContextHandlerRef ConnectionContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( connection ):
            if( nElement == XLS_TOKEN( webPr ) )
            {
                mxConnection->importWebPr( rAttribs );
                return this;
            }
        break;

        case XLS_TOKEN( webPr ):
            if( nElement == XLS_TOKEN( tables ) )
            {
                mxConnection->importTables( rAttribs );
                return this;
            }
        break;

        case XLS_TOKEN( tables ):
            mxConnection->importTable( rAttribs, nElement );
        break;
    }
    return 0;
}

ContextHandlerRef ConnectionContext::onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case BIFF12_ID_CONNECTION:
            if( nRecId == BIFF12_ID_WEBPR )
            {
                mxConnection->importWebPr( rStrm );
                return this;
            }
        break;

        case BIFF12_ID_WEBPR:
            if( nRecId == BIFF12_ID_WEBPRTABLES )
            {
                mxConnection->importWebPrTables( rStrm );
                return this;
            }
        break;

        case BIFF12_ID_WEBPRTABLES:
            mxConnection->importWebPrTable( rStrm, nRecId );
        break;
    }
    return 0;
}

// ============================================================================

ConnectionsFragment::ConnectionsFragment( const WorkbookHelper& rHelper, const OUString& rFragmentPath ) :
    WorkbookFragmentBase( rHelper, rFragmentPath )
{
}

ContextHandlerRef ConnectionsFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nElement == XLS_TOKEN( connections ) )
                return this;
        break;

        case XLS_TOKEN( connections ):
            if( nElement == XLS_TOKEN( connection ) )
            {
                ConnectionRef xConnection = getConnections().importConnection( rAttribs );
                if( xConnection.get() )
                    return new ConnectionContext( *this, xConnection );
            }
        break;
    }
    return 0;
}

ContextHandlerRef ConnectionsFragment::onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nRecId == BIFF12_ID_CONNECTIONS )
                return this;
        break;

        case BIFF12_ID_CONNECTIONS:
            if( nRecId == BIFF12_ID_CONNECTION )
            {
                ConnectionRef xConnection = getConnections().importConnection( rStrm );
                if( xConnection.get() )
                    return new ConnectionContext( *this, xConnection );
            }
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

// ============================================================================

} // namespace xls
} // namespace oox
