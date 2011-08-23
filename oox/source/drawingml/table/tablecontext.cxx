/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "oox/helper/attributelist.hxx"
#include "oox/drawingml/guidcontext.hxx"
#include "oox/drawingml/table/tablecontext.hxx"
#include "oox/drawingml/table/tableproperties.hxx"
#include "oox/drawingml/table/tablestylecontext.hxx"
#include "oox/drawingml/table/tablerowcontext.hxx"
#include "oox/core/namespaces.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star;
using ::rtl::OUString;

namespace oox { namespace drawingml { namespace table {

TableContext::TableContext( ContextHandler& rParent, ShapePtr pShapePtr )
: ShapeContext( rParent, ShapePtr(), pShapePtr )
, mrTableProperties( *pShapePtr->getTableProperties().get() )
{
    pShapePtr->setServiceName( "com.sun.star.drawing.TableShape" );
    pShapePtr->setSubType( 0 );
}

TableContext::~TableContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
TableContext::createFastChildContext( ::sal_Int32 aElementToken, const uno::Reference< xml::sax::XFastAttributeList >& xAttribs )
    throw ( xml::sax::SAXException, uno::RuntimeException)
{
    uno::Reference< xml::sax::XFastContextHandler > xRet;

    switch( aElementToken )
    {
    case NMSP_DRAWINGML|XML_tblPr:				// CT_TableProperties
        {
            AttributeList aAttribs( xAttribs );
            mrTableProperties.isRtl() = aAttribs.getBool( XML_rtl, sal_False );
            mrTableProperties.isFirstRow() = aAttribs.getBool( XML_firstRow, sal_False );
            mrTableProperties.isFirstCol() = aAttribs.getBool( XML_firstCol, sal_False );
            mrTableProperties.isLastRow() = aAttribs.getBool( XML_lastRow, sal_False );
            mrTableProperties.isLastCol() = aAttribs.getBool( XML_lastCol, sal_False );
            mrTableProperties.isBandRow() = aAttribs.getBool( XML_bandRow, sal_False );
            mrTableProperties.isBandCol() = aAttribs.getBool( XML_bandCol, sal_False );
        }
        break;
    case NMSP_DRAWINGML|XML_tableStyle:			// CT_TableStyle
        {
            boost::shared_ptr< TableStyle >& rTableStyle = mrTableProperties.getTableStyle();
            rTableStyle.reset( new TableStyle() );
            xRet = new TableStyleContext( *this, xAttribs, *rTableStyle );
        }
        break;
    case NMSP_DRAWINGML|XML_tableStyleId:		// ST_Guid
        xRet.set( new oox::drawingml::GuidContext( *this, mrTableProperties.getStyleId() ) );
        break;

    case NMSP_DRAWINGML|XML_tblGrid:			// CT_TableGrid
        break;
    case NMSP_DRAWINGML|XML_gridCol:			// CT_TableCol
        {
            std::vector< sal_Int32 >& rvTableGrid( mrTableProperties.getTableGrid() );
            rvTableGrid.push_back( xAttribs->getOptionalValue( XML_w ).toInt32() );
        }
        break;
    case NMSP_DRAWINGML|XML_tr:					// CT_TableRow
        {
            std::vector< TableRow >& rvTableRows( mrTableProperties.getTableRows() );
            rvTableRows.resize( rvTableRows.size() + 1 );
            xRet.set( new TableRowContext( *this, xAttribs, rvTableRows.back() ) );
        }
        break;
    }

    if( !xRet.is() )
        xRet.set( this );

    return xRet;
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
