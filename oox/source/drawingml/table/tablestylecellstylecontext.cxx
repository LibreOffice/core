/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tablestylecellstylecontext.cxx,v $
 * $Revision: 1.2 $
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
#include "oox/drawingml/stylematrixreferencecontext.hxx"
#include "oox/core/namespaces.hxx"
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
, meLineType( TableStylePart::NONE )
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
    switch( aElementToken )
    {
        case NMSP_DRAWINGML|XML_tcBdr:      // CT_TableCellBorderStyle
            break;
        case NMSP_DRAWINGML|XML_left:       // CT_ThemeableLineStyle
            meLineType = TableStylePart::LEFT;
            break;
        case NMSP_DRAWINGML|XML_right:
            meLineType = TableStylePart::RIGHT;
            break;
        case NMSP_DRAWINGML|XML_top:
            meLineType = TableStylePart::TOP;
            break;
        case NMSP_DRAWINGML|XML_bottom:
            meLineType = TableStylePart::BOTTOM;
            break;
        case NMSP_DRAWINGML|XML_insideH:
            meLineType = TableStylePart::INSIDEH;
            break;
        case NMSP_DRAWINGML|XML_insideV:
            meLineType = TableStylePart::INSIDEV;
            break;
        case NMSP_DRAWINGML|XML_tl2br:
            meLineType = TableStylePart::TL2BR;
            break;
        case NMSP_DRAWINGML|XML_tr2bl:
            meLineType = TableStylePart::TR2BL;
            break;

        case NMSP_DRAWINGML|XML_ln:
            {
                if ( meLineType != TableStylePart::NONE )
                {
                    std::map < TableStylePart::LineType, boost::shared_ptr< ::oox::drawingml::LineProperties > >& rLineBorders = mrTableStylePart.getLineBorders();
                    boost::shared_ptr< ::oox::drawingml::LineProperties > mpLineProperties( new oox::drawingml::LineProperties );
                    rLineBorders[ meLineType ] = mpLineProperties;
                    xRet = new LinePropertiesContext( *this, xAttribs, *mpLineProperties );
                }
            }
            break;
        case NMSP_DRAWINGML|XML_lnRef:
            {
                if ( meLineType != TableStylePart::NONE )
                {
                    std::map < TableStylePart::LineType, rtl::OUString >& rLineBordersStyleRef = mrTableStylePart.getLineBordersStyleRef();
                    std::map < TableStylePart::LineType, oox::drawingml::Color >& rLineBordersStyleColor = mrTableStylePart.getLineBordersStyleColor();
                    xRet.set( new StyleMatrixReferenceContext( *this, xAttribs, rLineBordersStyleRef[ meLineType ], rLineBordersStyleColor[ meLineType ] ) );
                }
            }
            break;

        // EG_ThemeableFillStyle (choice)
        case NMSP_DRAWINGML|XML_fill:       // CT_FillProperties
            {
                boost::shared_ptr< oox::drawingml::FillProperties >& rFillProperties( mrTableStylePart.getFillProperties() );
                rFillProperties = boost::shared_ptr< oox::drawingml::FillProperties > ( new oox::drawingml::FillProperties() );
                xRet.set( new oox::drawingml::FillPropertiesContext( *this, *rFillProperties.get() ) );
            }
            break;
        case NMSP_DRAWINGML|XML_fillRef:    // CT_StyleMatrixReference
            xRet.set( new StyleMatrixReferenceContext( *this, xAttribs, mrTableStylePart.getFillStyleRef(), mrTableStylePart.getFillStyleColor() ) );
            break;

        case NMSP_DRAWINGML|XML_cell3D:     // CT_Cell3D
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
