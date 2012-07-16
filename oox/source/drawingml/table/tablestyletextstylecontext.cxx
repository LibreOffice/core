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

#include "oox/drawingml/table/tablestyletextstylecontext.hxx"
#include "oox/drawingml/colorchoicecontext.hxx"
#include "oox/helper/attributelist.hxx"

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
    if( xAttribs->hasAttribute( XML_b ) ) {
        sal_Int32 nB = xAttribs->getOptionalValueToken( XML_b, XML_def );
        if ( nB == XML_on )
            mrTableStylePart.getTextBoldStyle() = ::boost::optional< sal_Bool >( sal_True );
        else if ( nB == XML_off )
            mrTableStylePart.getTextBoldStyle() = ::boost::optional< sal_Bool >( sal_False );
    }

    if( xAttribs->hasAttribute( XML_i ) ) {
        sal_Int32 nI = xAttribs->getOptionalValueToken( XML_i, XML_def );
        if ( nI == XML_on )
            mrTableStylePart.getTextItalicStyle() = ::boost::optional< sal_Bool >( sal_True );
        else if ( nI == XML_off )
            mrTableStylePart.getTextItalicStyle() = ::boost::optional< sal_Bool >( sal_False );
    }
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
    AttributeList aAttribs( xAttribs );

    switch( aElementToken )
    {
        // EG_ThemeableFontStyles (choice)
        case A_TOKEN( font ):       // CT_FontCollection
            xRet.set( this );
            break;
        case A_TOKEN( ea ):             // CT_TextFont
            mrTableStylePart.getAsianFont().setAttributes( aAttribs );
            return 0;
        case A_TOKEN( cs ):             // CT_TextFont
            mrTableStylePart.getComplexFont().setAttributes( aAttribs );
            return 0;
        case A_TOKEN( sym ):            // CT_TextFont
            mrTableStylePart.getSymbolFont().setAttributes( aAttribs );
            return 0;
        case A_TOKEN( latin ):          // CT_TextFont
            mrTableStylePart.getLatinFont().setAttributes( aAttribs );
            return 0;

        case A_TOKEN( fontRef ):    // CT_FontReference
            {
                ShapeStyleRef& rFontStyle = mrTableStylePart.getStyleRefs()[ XML_fontRef ];
                rFontStyle.mnThemedIdx = aAttribs.getToken( XML_idx, XML_none );
                xRet.set( new ColorContext( *this, rFontStyle.maPhClr ) );
            }
            break;

        case A_TOKEN( extLst ):     // CT_OfficeArtExtensionList
            break;
    }
    if( !xRet.is() )
        xRet.set( new ColorValueContext( *this, mrTableStylePart.getTextColor() ) );

    return xRet;
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
