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

#include "oox/vml/vmltextboxcontext.hxx"

namespace oox {
namespace vml {

// ============================================================================

using ::oox::core::ContextHandler2;
using ::oox::core::ContextHandler2Helper;
using ::oox::core::ContextHandlerRef;
using ::rtl::OUString;

// ============================================================================

TextPortionContext::TextPortionContext( ContextHandler2Helper& rParent,
        TextBox& rTextBox, const TextFontModel& rParentFont,
        sal_Int32 nElement, const AttributeList& rAttribs ) :
    ContextHandler2( rParent ),
    mrTextBox( rTextBox ),
    maFont( rParentFont ),
    mnInitialPortions( rTextBox.getPortionCount() )
{
    switch( nElement )
    {
        case XML_font:
            maFont.moName = rAttribs.getXString( XML_face );
            maFont.moColor = rAttribs.getXString( XML_color );
            maFont.monSize = rAttribs.getInteger( XML_size );
        break;
        case XML_u:
            OSL_ENSURE( !maFont.monUnderline, "TextPortionContext::TextPortionContext - nested <u> elements" );
            maFont.monUnderline = (rAttribs.getToken( XML_class, XML_TOKEN_INVALID ) == XML_font4) ? XML_double : XML_single;
        break;
        case XML_sub:
        case XML_sup:
            OSL_ENSURE( !maFont.monEscapement, "TextPortionContext::TextPortionContext - nested <sub> or <sup> elements" );
            maFont.monEscapement = nElement;
        break;
        case XML_b:
            OSL_ENSURE( !maFont.mobBold, "TextPortionContext::TextPortionContext - nested <b> elements" );
            maFont.mobBold = true;
        break;
        case XML_i:
            OSL_ENSURE( !maFont.mobItalic, "TextPortionContext::TextPortionContext - nested <i> elements" );
            maFont.mobItalic = true;
        break;
        case XML_s:
            OSL_ENSURE( !maFont.mobStrikeout, "TextPortionContext::TextPortionContext - nested <s> elements" );
            maFont.mobStrikeout = true;
        break;
        case XML_span:
        break;
        default:
            OSL_ENSURE( false, "TextPortionContext::TextPortionContext - unknown element" );
    }
}

ContextHandlerRef TextPortionContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    OSL_ENSURE( nElement != XML_font, "TextPortionContext::onCreateContext - nested <font> elements" );
    return new TextPortionContext( *this, mrTextBox, maFont, nElement, rAttribs );
}

void TextPortionContext::onCharacters( const OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case XML_span:
            // replace all NBSP characters with SP
            mrTextBox.appendPortion( maFont, rChars.replace( 0xA0, ' ' ) );
        break;
        default:
            mrTextBox.appendPortion( maFont, rChars );
    }
}

void TextPortionContext::onEndElement()
{
    /*  A child element without own child elements may contain a single space
        character, for example:

          <div>
            <font><i>abc</i></font>
            <font> </font>
            <font><b>def</b></font>
          </div>

        represents the italic text 'abc', an unformatted space character, and
        the bold text 'def'. Unfortunately, the XML parser skips the space
        character without issuing a 'characters' event. The class member
        'mnInitialPortions' contains the number of text portions existing when
        this context has been constructed. If no text has been added in the
        meantime, the space character has to be added manually.
     */
    if( mrTextBox.getPortionCount() == mnInitialPortions )
        mrTextBox.appendPortion( maFont, OUString( sal_Unicode( ' ' ) ) );
}

// ============================================================================

TextBoxContext::TextBoxContext( ContextHandler2Helper& rParent, TextBox& rTextBox, const AttributeList& /*rAttribs*/ ) :
    ContextHandler2( rParent ),
    mrTextBox( rTextBox )
{
}

ContextHandlerRef TextBoxContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case VML_TOKEN( textbox ):
            if( nElement == XML_div ) return this;
        break;
        case XML_div:
            if( nElement == XML_font ) return new TextPortionContext( *this, mrTextBox, TextFontModel(), nElement, rAttribs );
        break;
    }
    return 0;
}

// ============================================================================

} // namespace vml
} // namespace oox
