/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textliststylecontext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:29:36 $
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
        case NMSP_DRAWINGML|XML_defPPr:     // CT_TextParagraphProperties
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

