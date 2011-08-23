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

#include "oox/drawingml/textliststylecontext.hxx"
#include "oox/drawingml/textparagraphpropertiescontext.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/helper/attributelist.hxx"
#include "tokens.hxx"

using ::rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

// --------------------------------------------------------------------

// CT_TextListStyle
TextListStyleContext::TextListStyleContext( ContextHandler& rParent, TextListStyle& rTextListStyle )
: ContextHandler( rParent )
, mrTextListStyle( rTextListStyle )
{
}

TextListStyleContext::~TextListStyleContext()
{
}

// --------------------------------------------------------------------

void TextListStyleContext::endFastElement( sal_Int32 ) throw (SAXException, RuntimeException)
{
}

// --------------------------------------------------------------------

Reference< XFastContextHandler > TextListStyleContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& rxAttributes ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
        case NMSP_DRAWINGML|XML_defPPr:		// CT_TextParagraphProperties
            xRet.set( new TextParagraphPropertiesContext( *this, rxAttributes, *mrTextListStyle.getListStyle()[ 0 ] ) );
            break;
        case NMSP_DRAWINGML|XML_outline1pPr:
            xRet.set( new TextParagraphPropertiesContext( *this, rxAttributes, *mrTextListStyle.getAggregationListStyle()[ 0 ] ) );
            break;
        case NMSP_DRAWINGML|XML_outline2pPr:
            xRet.set( new TextParagraphPropertiesContext( *this, rxAttributes, *mrTextListStyle.getAggregationListStyle()[ 1 ] ) );
            break;
        case NMSP_DRAWINGML|XML_lvl1pPr:
            xRet.set( new TextParagraphPropertiesContext( *this, rxAttributes, *mrTextListStyle.getListStyle()[ 0 ] ) );
            break;
        case NMSP_DRAWINGML|XML_lvl2pPr:
            xRet.set( new TextParagraphPropertiesContext( *this, rxAttributes, *mrTextListStyle.getListStyle()[ 1 ] ) );
            break;
        case NMSP_DRAWINGML|XML_lvl3pPr:
            xRet.set( new TextParagraphPropertiesContext( *this, rxAttributes, *mrTextListStyle.getListStyle()[ 2 ] ) );
            break;
        case NMSP_DRAWINGML|XML_lvl4pPr:
            xRet.set( new TextParagraphPropertiesContext( *this, rxAttributes, *mrTextListStyle.getListStyle()[ 3 ] ) );
            break;
        case NMSP_DRAWINGML|XML_lvl5pPr:
            xRet.set( new TextParagraphPropertiesContext( *this, rxAttributes, *mrTextListStyle.getListStyle()[ 4 ] ) );
            break;
        case NMSP_DRAWINGML|XML_lvl6pPr:
            xRet.set( new TextParagraphPropertiesContext( *this, rxAttributes, *mrTextListStyle.getListStyle()[ 5 ] ) );
            break;
        case NMSP_DRAWINGML|XML_lvl7pPr:
            xRet.set( new TextParagraphPropertiesContext( *this, rxAttributes, *mrTextListStyle.getListStyle()[ 6 ] ) );
            break;
        case NMSP_DRAWINGML|XML_lvl8pPr:
            xRet.set( new TextParagraphPropertiesContext( *this, rxAttributes, *mrTextListStyle.getListStyle()[ 7 ] ) );
            break;
        case NMSP_DRAWINGML|XML_lvl9pPr:
            xRet.set( new TextParagraphPropertiesContext( *this, rxAttributes, *mrTextListStyle.getListStyle()[ 8 ] ) );
            break;
    }
    if ( !xRet.is() )
        xRet.set( this );
    return xRet;
}

// --------------------------------------------------------------------

} }

