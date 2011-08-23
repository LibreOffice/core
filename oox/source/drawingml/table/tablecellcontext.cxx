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

#include "oox/drawingml/table/tablecellcontext.hxx"
#include "oox/drawingml/textbodycontext.hxx"
#include "oox/drawingml/linepropertiescontext.hxx"
#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/helper/attributelist.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star;
using ::rtl::OUString;

namespace oox { namespace drawingml { namespace table {

TableCellContext::TableCellContext( ContextHandler& rParent, const uno::Reference< xml::sax::XFastAttributeList >& xAttribs, TableCell& rTableCell )
: ContextHandler( rParent )
, mrTableCell( rTableCell )
{
    if ( xAttribs->hasAttribute( XML_rowSpan ) )
        mrTableCell.setRowSpan( xAttribs->getOptionalValue( XML_rowSpan ).toInt32() );
    if ( xAttribs->hasAttribute( XML_gridSpan ) )
        mrTableCell.setGridSpan( xAttribs->getOptionalValue( XML_gridSpan ).toInt32() );

    AttributeList aAttribs( xAttribs );
    mrTableCell.sethMerge( aAttribs.getBool( XML_hMerge, sal_False ) );
    mrTableCell.setvMerge( aAttribs.getBool( XML_vMerge, sal_False ) );
}

TableCellContext::~TableCellContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
TableCellContext::createFastChildContext( ::sal_Int32 aElementToken, const uno::Reference< xml::sax::XFastAttributeList >& xAttribs )
    throw ( xml::sax::SAXException, uno::RuntimeException)
{
    uno::Reference< xml::sax::XFastContextHandler > xRet;

    switch( aElementToken )
    {
    case NMSP_DRAWINGML|XML_txBody:		// CT_TextBody
        {
            oox::drawingml::TextBodyPtr xTextBody( new oox::drawingml::TextBody );
            mrTableCell.setTextBody( xTextBody );
            xRet = new oox::drawingml::TextBodyContext( *this, *xTextBody );
        }
        break;

    case NMSP_DRAWINGML|XML_tcPr:		// CT_TableCellProperties
        {
            AttributeList aAttribs( xAttribs );
            mrTableCell.setLeftMargin( aAttribs.getInteger( XML_marL, 91440 ) );
            mrTableCell.setRightMargin( aAttribs.getInteger( XML_marR, 91440 ) );
            mrTableCell.setTopMargin( aAttribs.getInteger( XML_marT, 45720 ) );
            mrTableCell.setBottomMargin( aAttribs.getInteger( XML_marB, 45720 ) );
            mrTableCell.setVertToken( xAttribs->getOptionalValueToken( XML_vert, XML_horz ) );					// ST_TextVerticalType
            mrTableCell.setAnchorToken( xAttribs->getOptionalValueToken( XML_anchor, XML_t ) );					// ST_TextAnchoringType
            mrTableCell.setAnchorCtr( aAttribs.getBool( XML_anchorCtr, sal_False ) );
            mrTableCell.setHorzOverflowToken( xAttribs->getOptionalValueToken( XML_horzOverflow, XML_clip ) );	// ST_TextHorzOverflowType
        }
        break;
        case NMSP_DRAWINGML|XML_lnL:
                xRet.set( new oox::drawingml::LinePropertiesContext( *this, xAttribs, mrTableCell.maLinePropertiesLeft ) );
            break;
        case NMSP_DRAWINGML|XML_lnR:
                xRet.set( new oox::drawingml::LinePropertiesContext( *this, xAttribs, mrTableCell.maLinePropertiesRight ) );
            break;
        case NMSP_DRAWINGML|XML_lnT:
                xRet.set( new oox::drawingml::LinePropertiesContext( *this, xAttribs, mrTableCell.maLinePropertiesTop ) );
            break;
        case NMSP_DRAWINGML|XML_lnB:
                xRet.set( new oox::drawingml::LinePropertiesContext( *this, xAttribs, mrTableCell.maLinePropertiesBottom ) );
            break;
        case NMSP_DRAWINGML|XML_lnTlToBr:
                xRet.set( new oox::drawingml::LinePropertiesContext( *this, xAttribs, mrTableCell.maLinePropertiesTopLeftToBottomRight ) );
            break;
        case NMSP_DRAWINGML|XML_lnBlToTr:
                xRet.set( new oox::drawingml::LinePropertiesContext( *this, xAttribs, mrTableCell.maLinePropertiesBottomLeftToTopRight ) );
            break;
        case NMSP_DRAWINGML|XML_cell3D:	// CT_Cell3D
        break;

    case NMSP_DRAWINGML|XML_extLst:		// CT_OfficeArtExtensionList
    break;

    default:
        xRet.set( FillPropertiesContext::createFillContext( *this, aElementToken, xAttribs, mrTableCell.maFillProperties ) );
    break;

    }
    if ( !xRet.is() )
    {
        uno::Reference< XFastContextHandler > xTmp( this );
        xRet.set( xTmp );
    }
    return xRet;
}

} } }
