/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tablestyletextstylecontext.cxx,v $
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

#include "oox/drawingml/table/tablestyletextstylecontext.hxx"
#include "oox/drawingml/colorchoicecontext.hxx"
#include "oox/drawingml/stylematrixreferencecontext.hxx"
#include "oox/drawingml/textfontcontext.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"
using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using ::rtl::OUString;

namespace oox { namespace drawingml { namespace table {

TableStyleTextStyleContext::TableStyleTextStyleContext( ContextHandler& rParent,
    const Reference< XFastAttributeList >& xAttribs, TableStylePart& rTableStylePart )
: ContextHandler( rParent )
, mrTableStylePart( rTableStylePart )
{
    sal_Int32 nB = xAttribs->getOptionalValueToken( XML_b, XML_def );
    if ( nB == XML_on )
        mrTableStylePart.getTextBoldStyle() = ::boost::optional< sal_Bool >( sal_True );
    else if ( nB == XML_off )
        mrTableStylePart.getTextBoldStyle() = ::boost::optional< sal_Bool >( sal_False );

    sal_Int32 nI = xAttribs->getOptionalValueToken( XML_i, XML_def );
    if ( nI == XML_on )
        mrTableStylePart.getTextItalicStyle() = ::boost::optional< sal_Bool >( sal_True );
    else if ( nI == XML_off )
        mrTableStylePart.getTextItalicStyle() = ::boost::optional< sal_Bool >( sal_False );
}

TableStyleTextStyleContext::~TableStyleTextStyleContext()
{
}

// CT_TableStyleTextStyle
uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
TableStyleTextStyleContext::createFastChildContext( ::sal_Int32 aElementToken, const uno::Reference< xml::sax::XFastAttributeList >& xAttribs )
    throw ( xml::sax::SAXException, uno::RuntimeException)
{
    uno::Reference< xml::sax::XFastContextHandler > xRet;

    switch( aElementToken )
    {
        // EG_ThemeableFontStyles (choice)
        case NMSP_DRAWINGML|XML_font:       // CT_FontCollection
            xRet.set( this );
            break;
        case NMSP_DRAWINGML|XML_ea:             // CT_TextFont
            xRet.set( new TextFontContext( *this, aElementToken,  xAttribs, mrTableStylePart.getAsianFont() ) );
            break;
        case NMSP_DRAWINGML|XML_cs:             // CT_TextFont
            xRet.set( new TextFontContext( *this, aElementToken,  xAttribs, mrTableStylePart.getComplexFont() ) );
            break;
        case NMSP_DRAWINGML|XML_sym:            // CT_TextFont
            xRet.set( new TextFontContext( *this, aElementToken,  xAttribs, mrTableStylePart.getSymbolFont() ) );
            break;
        case NMSP_DRAWINGML|XML_latin:          // CT_TextFont
            xRet.set( new TextFontContext( *this, aElementToken,  xAttribs, mrTableStylePart.getLatinFont() ) );
            break;

        case NMSP_DRAWINGML|XML_fontRef:    // CT_FontReference
            xRet.set( new StyleMatrixReferenceContext( *this, xAttribs,
                mrTableStylePart.getThemeableFontRef(), mrTableStylePart.getThemeableFontColor() ) );
            break;

        case NMSP_DRAWINGML|XML_extLst:     // CT_OfficeArtExtensionList
            break;
    }
    if( !xRet.is() )
        xRet.set( new colorChoiceContext( *this, mrTableStylePart.getTextColor() ) );

    return xRet;
}

} } }
