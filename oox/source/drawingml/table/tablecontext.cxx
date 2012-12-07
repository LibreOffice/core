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

#include <osl/diagnose.h>
#include "oox/helper/attributelist.hxx"
#include "oox/drawingml/guidcontext.hxx"
#include "oox/drawingml/table/tablecontext.hxx"
#include "oox/drawingml/table/tableproperties.hxx"
#include "oox/drawingml/table/tablestylecontext.hxx"
#include "oox/drawingml/table/tablerowcontext.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star;

namespace oox { namespace drawingml { namespace table {

TableContext::TableContext( ContextHandler& rParent, ShapePtr pShapePtr )
: ShapeContext( rParent, ShapePtr(), pShapePtr )
, mrTableProperties( *pShapePtr->getTableProperties().get() )
{
    pShapePtr->setTableType();
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
    case A_TOKEN( tblPr ):              // CT_TableProperties
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
    case A_TOKEN( tableStyle ):         // CT_TableStyle
        {
            boost::shared_ptr< TableStyle >& rTableStyle = mrTableProperties.getTableStyle();
            rTableStyle.reset( new TableStyle() );
            xRet = new TableStyleContext( *this, xAttribs, *rTableStyle );
        }
        break;
    case A_TOKEN( tableStyleId ):       // ST_Guid
        xRet.set( new oox::drawingml::GuidContext( *this, mrTableProperties.getStyleId() ) );
        break;

    case A_TOKEN( tblGrid ):            // CT_TableGrid
        break;
    case A_TOKEN( gridCol ):            // CT_TableCol
        {
            std::vector< sal_Int32 >& rvTableGrid( mrTableProperties.getTableGrid() );
            rvTableGrid.push_back( xAttribs->getOptionalValue( XML_w ).toInt32() );
        }
        break;
    case A_TOKEN( tr ):                 // CT_TableRow
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
