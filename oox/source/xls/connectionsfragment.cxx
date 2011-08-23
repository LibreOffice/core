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
#include "oox/xls/webquerybuffer.hxx"

using ::rtl::OUString;
using ::oox::core::ContextHandlerRef;

namespace oox {
namespace xls {

OoxConnectionsFragment::OoxConnectionsFragment( const WorkbookHelper& rHelper, const OUString& rFragmentPath ) :
    OoxWorkbookFragmentBase( rHelper, rFragmentPath )
{
}

ContextHandlerRef OoxConnectionsFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nElement == XLS_TOKEN( connections ) ) return this;
        break;

        case XLS_TOKEN( connections ):
            switch( nElement )
            {
                case XLS_TOKEN( connection ):   importConnection( rAttribs );   return this;
            }
        break;

        case XLS_TOKEN( connection ):
            switch( nElement )
            {
                case XLS_TOKEN( webPr ):        importWebPr( rAttribs );        return this;
            }
        break;

        case XLS_TOKEN( webPr ):
            switch( nElement )
            {
                case XLS_TOKEN( tables ):       importTables( rAttribs );       return this;
            }
        break;

        case XLS_TOKEN( tables ):
            switch( nElement )
            {
                case XLS_TOKEN( s ):            importS( rAttribs );            break;
                case XLS_TOKEN( x ):            importX( rAttribs );            break;
            }
        break;
    }
    return 0;
}

void OoxConnectionsFragment::importConnection( const AttributeList& rAttribs )
{
    if ( rAttribs.getInteger( XML_type, 0 ) == Connection::CONNECTION_WEBQUERY )
    {
        getWebQueries().importConnection( rAttribs );
    }
}

void OoxConnectionsFragment::importWebPr( const AttributeList& rAttribs )
{
    getWebQueries().importWebPr( rAttribs );
}

void OoxConnectionsFragment::importTables( const AttributeList& /*rAttribs*/ )
{
//     sal_Int32 nCount = rAttribs.getInteger( XML_count, 0 );
}

void OoxConnectionsFragment::importS( const AttributeList& /*rAttribs*/ )
{
//     OUString aName = rAttribs.getString( XML_v );
}

void OoxConnectionsFragment::importX( const AttributeList& /*rAttribs*/ )
{
//     sal_Int32 nSharedId = rAttribs.getInteger( XML_v, 0 );
}

} // namespace xls
} // namespace oox

