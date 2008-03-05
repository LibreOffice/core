/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: connectionsfragment.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:58:07 $
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

#include "oox/xls/connectionsfragment.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/xls/webquerybuffer.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::sheet::XSpreadsheet;
using ::com::sun::star::xml::sax::SAXException;

namespace oox {
namespace xls {

OoxConnectionsFragment::OoxConnectionsFragment( const WorkbookHelper& rHelper, const OUString& rFragmentPath ) :
    OoxWorkbookFragmentBase( rHelper, rFragmentPath )
{
}

ContextWrapper OoxConnectionsFragment::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            return  (nElement == XLS_TOKEN( connections ));
        case XLS_TOKEN( connections ):
            return  (nElement == XLS_TOKEN( connection ));
        case XLS_TOKEN( connection ):
            return  (nElement == XLS_TOKEN( webPr )) ||
                    (nElement == XLS_TOKEN( textPr )) ||
                    (nElement == XLS_TOKEN( dbPr )) ||
                    (nElement == XLS_TOKEN( olapPr )) ||
                    (nElement == XLS_TOKEN( parameters ));
        case XLS_TOKEN( webPr ):
            return  (nElement == XLS_TOKEN( tables ));
        case XLS_TOKEN( tables ):
            return  (nElement == XLS_TOKEN( m )) ||
                    (nElement == XLS_TOKEN( s )) ||
                    (nElement == XLS_TOKEN( x ));
    }
    return false;
}

void OoxConnectionsFragment::onStartElement( const AttributeList& rAttribs )
{
    switch ( getCurrentElement() )
    {
        case XLS_TOKEN( connection ):
            importConnection( rAttribs );
        break;
        case XLS_TOKEN( webPr ):
            importWebPr( rAttribs );
        break;
        case XLS_TOKEN( tables ):
            importTables( rAttribs );
        break;
        case XLS_TOKEN( s ):
            importS( rAttribs );
        break;
        case XLS_TOKEN( x ):
            importX( rAttribs );
        break;
    }
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

