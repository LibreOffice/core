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

#include "oox/drawingml/table/tablestylecontext.hxx"
#include "oox/drawingml/table/tablebackgroundstylecontext.hxx"
#include "oox/drawingml/table/tablepartstylecontext.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"
using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using ::rtl::OUString;

namespace oox { namespace drawingml { namespace table {

TableStyleContext::TableStyleContext( ContextHandler& rParent,
    const Reference< XFastAttributeList >& xAttribs, TableStyle& rTableStyle )
: ContextHandler( rParent )
, mrTableStyle( rTableStyle )
{
    mrTableStyle.getStyleId() = xAttribs->getOptionalValue( XML_styleId );
    mrTableStyle.getStyleName() = xAttribs->getOptionalValue( XML_styleName );
}

TableStyleContext::~TableStyleContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
TableStyleContext::createFastChildContext( ::sal_Int32 aElementToken, const uno::Reference< xml::sax::XFastAttributeList >& /* xAttribs */ )
    throw ( xml::sax::SAXException, uno::RuntimeException)
{
    uno::Reference< xml::sax::XFastContextHandler > xRet;

    switch( aElementToken )
    {
        case NMSP_DRAWINGML|XML_tblBg:		// CT_TableBackgroundStyle
            xRet = new TableBackgroundStyleContext( *this, mrTableStyle );
            break;
        case NMSP_DRAWINGML|XML_wholeTbl:	// CT_TablePartStyle
            xRet = new TablePartStyleContext( *this, mrTableStyle.getWholeTbl() );
            break;
        case NMSP_DRAWINGML|XML_band1H:		// CT_TablePartStyle
            xRet = new TablePartStyleContext( *this, mrTableStyle.getBand1H() );
            break;
        case NMSP_DRAWINGML|XML_band2H:		// CT_TablePartStyle
            xRet = new TablePartStyleContext( *this, mrTableStyle.getBand2H() );
            break;
        case NMSP_DRAWINGML|XML_band1V:		// CT_TablePartStyle
            xRet = new TablePartStyleContext( *this, mrTableStyle.getBand1V() );
            break;
        case NMSP_DRAWINGML|XML_band2V:		// CT_TablePartStyle
            xRet = new TablePartStyleContext( *this, mrTableStyle.getBand2V() );
            break;
        case NMSP_DRAWINGML|XML_lastCol:	// CT_TablePartStyle
            xRet = new TablePartStyleContext( *this, mrTableStyle.getLastCol() );
            break;
        case NMSP_DRAWINGML|XML_firstCol:	// CT_TablePartStyle
            xRet = new TablePartStyleContext( *this, mrTableStyle.getFirstCol() );
            break;
        case NMSP_DRAWINGML|XML_lastRow:	// CT_TablePartStyle
            xRet = new TablePartStyleContext( *this, mrTableStyle.getLastRow() );
            break;
        case NMSP_DRAWINGML|XML_seCell:		// CT_TablePartStyle
            xRet = new TablePartStyleContext( *this, mrTableStyle.getSeCell() );
            break;
        case NMSP_DRAWINGML|XML_swCell:		// CT_TablePartStyle
            xRet = new TablePartStyleContext( *this, mrTableStyle.getSwCell() );
            break;
        case NMSP_DRAWINGML|XML_firstRow:	// CT_TablePartStyle
            xRet = new TablePartStyleContext( *this, mrTableStyle.getFirstRow() );
            break;
        case NMSP_DRAWINGML|XML_neCell:		// CT_TablePartStyle
            xRet = new TablePartStyleContext( *this, mrTableStyle.getNeCell() );
            break;
        case NMSP_DRAWINGML|XML_nwCell:		// CT_TablePartStyle
            xRet = new TablePartStyleContext( *this, mrTableStyle.getNwCell() );
            break;
        case NMSP_DRAWINGML|XML_extLst:		// CT_OfficeArtExtensionList
            break;
    }
    if( !xRet.is() )
    {
        uno::Reference<XFastContextHandler> xTmp(this);
        xRet.set( xTmp );
    }
    return xRet;
}

} } }
