/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: webquerybuffer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:06:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    OUString aName = rAttribs.getString( XML_name );
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

    sal_uInt32 nId = rAttribs.getUnsignedInteger( XML_id, 0 );
    if ( maConnections.size() < (nId + 1) )
        maConnections.resize(nId + 1);

    Connection aConn;
    aConn.maName = rAttribs.getString( XML_name );
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
    pWebPr->maURL = rAttribs.getString( XML_url );

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

