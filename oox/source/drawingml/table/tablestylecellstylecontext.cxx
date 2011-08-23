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

#include "oox/drawingml/table/tablestylecellstylecontext.hxx"
#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/drawingml/linepropertiescontext.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/helper/attributelist.hxx"
#include "tokens.hxx"
using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using ::rtl::OUString;

namespace oox { namespace drawingml { namespace table {

TableStyleCellStyleContext::TableStyleCellStyleContext( ContextHandler& rParent, TableStylePart& rTableStylePart )
: ContextHandler( rParent )
, mrTableStylePart( rTableStylePart )
, mnLineType( XML_none )
{
}

TableStyleCellStyleContext::~TableStyleCellStyleContext()
{
}

// CT_TableStyleCellStyle
uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
TableStyleCellStyleContext::createFastChildContext( ::sal_Int32 aElementToken, const uno::Reference< xml::sax::XFastAttributeList >& xAttribs )
    throw ( xml::sax::SAXException, uno::RuntimeException)
{
    uno::Reference< xml::sax::XFastContextHandler > xRet;
    AttributeList aAttribs( xAttribs );
    switch( aElementToken )
    {
        case NMSP_DRAWINGML|XML_tcBdr:		// CT_TableCellBorderStyle
            break;
        case NMSP_DRAWINGML|XML_left:		// CT_ThemeableLineStyle
        case NMSP_DRAWINGML|XML_right:
        case NMSP_DRAWINGML|XML_top:
        case NMSP_DRAWINGML|XML_bottom:
        case NMSP_DRAWINGML|XML_insideH:
        case NMSP_DRAWINGML|XML_insideV:
        case NMSP_DRAWINGML|XML_tl2br:
        case NMSP_DRAWINGML|XML_tr2bl:
            mnLineType = getToken( aElementToken );
            break;

        case NMSP_DRAWINGML|XML_ln:
            {
                if ( mnLineType != XML_none )
                {
                    std::map < sal_Int32, ::oox::drawingml::LinePropertiesPtr >& rLineBorders = mrTableStylePart.getLineBorders();
                    ::oox::drawingml::LinePropertiesPtr mpLineProperties( new oox::drawingml::LineProperties );
                    rLineBorders[ mnLineType ] = mpLineProperties;
                    xRet = new LinePropertiesContext( *this, xAttribs, *mpLineProperties );
                }
            }
            break;
        case NMSP_DRAWINGML|XML_lnRef:
            {
                if ( mnLineType != XML_none )
                {
                    ShapeStyleRef& rLineStyleRef = mrTableStylePart.getStyleRefs()[ mnLineType ];
                    rLineStyleRef.mnThemedIdx = aAttribs.getInteger( XML_idx, 0 );
                    xRet.set( new ColorContext( *this, rLineStyleRef.maPhClr ) );
                }
            }
            break;

        // EG_ThemeableFillStyle (choice)
        case NMSP_DRAWINGML|XML_fill:		// CT_FillProperties
            {
                FillPropertiesPtr& rxFillProperties = mrTableStylePart.getFillProperties();
                rxFillProperties.reset( new FillProperties );
                xRet.set( new FillPropertiesContext( *this, *rxFillProperties ) );
            }
            break;
        case NMSP_DRAWINGML|XML_fillRef:	// CT_StyleMatrixReference
            {
                ShapeStyleRef& rStyleRef = mrTableStylePart.getStyleRefs()[ XML_fillRef ];
                rStyleRef.mnThemedIdx = aAttribs.getInteger( XML_idx, 0 );
                xRet.set( new ColorContext( *this, rStyleRef.maPhClr ) );
            }
            break;

        case NMSP_DRAWINGML|XML_cell3D:		// CT_Cell3D
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
