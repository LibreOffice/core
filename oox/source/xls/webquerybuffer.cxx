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

#include "oox/xls/webquerybuffer.hxx"
#include "oox/helper/attributelist.hxx"

#define DEBUG_OOX_WEBQUERY_BUFFER 1

#if DEBUG_OOX_WEBQUERY_BUFFER
#include <stdio.h>
#endif

using ::rtl::OUString;

namespace oox {
namespace xls {

const sal_Int32 Connection::CONNECTION_ODBC_SOURCE    = 1;
const sal_Int32 Connection::CONNECTION_DAO_SOURCE     = 2;
const sal_Int32 Connection::CONNECTION_FILE_SOURCE    = 3;
const sal_Int32 Connection::CONNECTION_WEBQUERY       = 4;
const sal_Int32 Connection::CONNECTION_OLEDB_SOURCE   = 5;
const sal_Int32 Connection::CONNECTION_TEXT_SOURCE    = 6;
const sal_Int32 Connection::CONNECTION_ADO_RECORD_SET = 7;
const sal_Int32 Connection::CONNECTION_DSP            = 8;

// ============================================================================

WebQueryBuffer::WebQueryBuffer( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
    maQueryTableMap.clear();
}

void WebQueryBuffer::importQueryTable( const AttributeList& rAttribs )
{
    OUString aName = rAttribs.getString( XML_name, OUString() );
    if ( !aName.getLength() )
        return;

    QueryTable aQTable;
    aQTable.mnConnectionId = rAttribs.getInteger( XML_connectionId, 0 );

    maQueryTableMap.insert( QueryTableHashMap::value_type( aName, aQTable ) );

    // All documented attributes of queryTable:
    //   adjustColumnWidth (bool)
    //   applyAlignmentFormats (bool)
    //   applyBorderFormats (bool)
    //   applyFontFormats (bool)
    //   applyNumberFormats (bool)
    //   applyPatternFormats (bool)
    //   applyWidthHeightFormats (bool)
    //   autoFormatId (unsigned int)
    //   backgroundRefresh (bool)
    //   connectionId (unsigned int)
    //   disableEdit (bool)
    //   disableRefresh (bool)
    //   fillFormulas (bool)
    //   firstBackgroundRefresh (bool)
    //   growShrinkType (insertClear, insertDelete, overwriteClear)
    //   headers (bool)
    //   intermediate (bool)
    //   name (string)
    //   preserveFormatting(bool)
    //   refreshOnLoad (bool)
    //   removeDataOnSave (bool)
    //   rowNumbers (bool)
}

void WebQueryBuffer::importConnection( const AttributeList& rAttribs )
{
    if ( !rAttribs.hasAttribute( XML_id ) || !rAttribs.hasAttribute( XML_name ) )
    {
        mnCurConnId = -1;
        return;
    }

    sal_uInt32 nId = rAttribs.getUnsigned( XML_id, 0 );
    if ( maConnections.size() < (nId + 1) )
        maConnections.resize(nId + 1);

    Connection aConn;
    aConn.maName = rAttribs.getString( XML_name, OUString() );
    aConn.mnType = rAttribs.getInteger( XML_type, 0 );
    maConnections[nId] = aConn;
    mnCurConnId = nId;

    // All documented attributes of connection.
    //   background (bool)
    //   credentials (integrated, none, prompt, stored)
    //   deleted (bool)
    //   description (string)
    //   id (unsigned int)
    //   interval (unsigned int)
    //   keepAlive (bool)
    //   minRefreshableVersion (unsigned byte)
    //   name (string)
    //   new (bool)
    //   odcFile (string)
    //   onlyUseConnectionFile (bool)
    //   reconnectionMethod (unsigned int)
    //   refreshedVersion (unsigned byte)
    //   refreshOnLoad (bool)
    //   saveData (bool)
    //   savePassword (bool)
    //   singleSignOnId (string)
    //   sourceFile (string)
    //   type (unsigned int)
}

void WebQueryBuffer::importWebPr( const AttributeList& rAttribs )
{
    if ( 0 > mnCurConnId )
        return;

    Connection& rConn = maConnections[mnCurConnId];
    rConn.mpProperties.reset( new WebProperties );
    WebProperties* pWebPr = static_cast< WebProperties* >( rConn.mpProperties.get() );
    pWebPr->maURL = rAttribs.getString( XML_url, OUString() );

    // All available attributes:
    //   consecutive (bool)
    //   editPage (string)
    //   firstRow (bool)
    //   htmlFormat (all, none, rtf)
    //   htmlTables (bool)
    //   parsePre (bool)
    //   post (string)
    //   sourceData (bool)
    //   textDates (bool)
    //   url (string)
    //   xl2000 (bool)
    //   xl97 (bool)
    //   xml (bool)
}

void WebQueryBuffer::dump() const
{
#if DEBUG_OOX_WEBQUERY_BUFFER
    fprintf(stdout, "----------------------------------------\n");
    {
        using ::std::vector;
        vector< Connection >::const_iterator itr = maConnections.begin(), itrEnd = maConnections.end();
        sal_Int32 nId = 0;
        for (; itr != itrEnd; ++itr, ++nId)
        {
            if ( itr->mnType == Connection::CONNECTION_WEBQUERY )
            {
                WebProperties* pWebPr = static_cast< WebProperties* >( itr->mpProperties.get() );
                fprintf(stdout, "WebQueryBuffer::dump: id = %d  url = %s\n",
                        (int)nId,
                        OUStringToOString(pWebPr->maURL, RTL_TEXTENCODING_UTF8).getStr());
            }
        }
    }

    QueryTableHashMap::const_iterator itr = maQueryTableMap.begin(), itrEnd = maQueryTableMap.end();
    for (; itr != itrEnd; ++itr)
    {
        fprintf(stdout, "WebQueryBuffer::dump: name = %s  connection ID = %d\n",
                OUStringToOString(itr->first, RTL_TEXTENCODING_UTF8).getStr(),
                (int)(itr->second.mnConnectionId));
    }

    fprintf(stdout, "----------------------------------------\n");
    fflush(stdout);
#endif
}

// ============================================================================

} // namespace xls
} // namespace oox

