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

#include <connectionsfragment.hxx>
#include <documentimport.hxx>

#include <oox/token/namespaces.hxx>
#include <biffhelper.hxx>
#include <connectionsbuffer.hxx>
#include <sal/log.hxx>

namespace oox::xls {

using namespace ::oox::core;

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
            if (nElement == XLS_TOKEN(dbPr))
            {
                mrConnection.importDbPr(rAttribs);
                return this;
            }
            if (nElement == XLS_TOKEN(olapPr))
            {
                mrConnection.importOlapPr(rAttribs);
                return this;
            }
            if( nElement == XLS_TOKEN( webPr ) )
            {
                mrConnection.importWebPr( rAttribs );
                return this;
            }
            if (nElement == XLS_TOKEN(textPr))
            {
                mrConnection.importTextPr(rAttribs);
                return this;
            }
            if (nElement == XLS_TOKEN(parameters))
            {
                mrConnection.importParameters(rAttribs);
                return this;
            }
            if (nElement == XLS_TOKEN(extLst))
            {
                mrConnection.importExtensionList();
                return this;
            }
        break;

        case XLS_TOKEN( webPr ):
            if( nElement == XLS_TOKEN( tables ) )
            {
                mrConnection.importTables(rAttribs);
                return this;
            }
        break;

        case XLS_TOKEN( tables ):
            mrConnection.importTable( rAttribs, nElement );
        break;

        case XLS_TOKEN(textPr):
            if (nElement == XLS_TOKEN(textFields))
            {
                mrConnection.importTextFields(rAttribs);
                return this;
            }
            break;

        case XLS_TOKEN(textFields):
            if (nElement == XLS_TOKEN(textField))
            {
                mrConnection.importTextField(rAttribs);
                return this;
            }
            break;

        case XLS_TOKEN(parameters):
            if (nElement == XLS_TOKEN(parameter))
            {
                mrConnection.importParameter(rAttribs);
                return this;
            }
            break;

        case XLS_TOKEN(extLst):
            if (nElement == XLS_TOKEN(ext))
            {
                mrConnection.importExtension(rAttribs);
                return this;
            }
            break;

        case XLS_TOKEN(ext):
            if (nElement == X15_TOKEN(connection))
            {
                // imports <x15:connection> element
                mrConnection.importXFifteenConnection(rAttribs);
                return this;
            }
            break;
        case X15_TOKEN(connection):
            if (nElement == X15_TOKEN(rangePr))
            {
                // imports <x15:rangePr> element
                mrConnection.importXFifteenRangePr(rAttribs);
                return this;
            }
            break;
    }
    return nullptr;
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
    return nullptr;
}

void ConnectionContext::onStartRecord( SequenceInputStream& rStrm )
{
    if( getCurrentElement() == BIFF12_ID_CONNECTION )
        mrConnection.importConnection( rStrm );
}

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
    return nullptr;
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
    return nullptr;
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

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
