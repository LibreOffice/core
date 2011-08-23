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

#include <osl/diagnose.h>

#include "oox/drawingml/table/tablerowcontext.hxx"
#include "oox/drawingml/table/tablecellcontext.hxx"
#include "oox/drawingml/table/tablerow.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star;
using ::rtl::OUString;

namespace oox { namespace drawingml { namespace table {

TableRowContext::TableRowContext( ContextHandler& rParent, const uno::Reference< xml::sax::XFastAttributeList >& xAttribs, TableRow& rTableRow )
: ContextHandler( rParent )
, mrTableRow( rTableRow )
{
    rTableRow.setHeight( xAttribs->getOptionalValue( XML_h ).toInt32() );
}

TableRowContext::~TableRowContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
TableRowContext::createFastChildContext( ::sal_Int32 aElementToken, const uno::Reference< xml::sax::XFastAttributeList >& xAttribs )
    throw ( xml::sax::SAXException, uno::RuntimeException)
{
    uno::Reference< xml::sax::XFastContextHandler > xRet;

    switch( aElementToken )
    {
    case NMSP_DRAWINGML|XML_tc:			// CT_TableCell
        {
            std::vector< TableCell >& rvTableCells = mrTableRow.getTableCells();
            rvTableCells.resize( rvTableCells.size() + 1 );
            xRet.set( new TableCellContext( *this, xAttribs, rvTableCells.back() ) );		
        }
        break;
    case NMSP_DRAWINGML|XML_extLst:		// CT_OfficeArtExtensionList
    default:
        break;
    }
    if( !xRet.is() )
    {
        uno::Reference< XFastContextHandler > xTmp( this );
        xRet.set( xTmp );
    }
    return xRet;
}

} } }
