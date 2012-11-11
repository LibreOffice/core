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

#include "oox/vml/vmlformatting.hxx"
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

TextBoxContext::TextBoxContext( ContextHandler2Helper& rParent, TextBox& rTextBox, const AttributeList& rAttribs,
    const GraphicHelper& graphicHelper ) :
    ContextHandler2( rParent ),
    mrTextBox( rTextBox )
{
    if( rAttribs.getString( XML_insetmode ).get() != "auto" )
    {
        OUString inset = rAttribs.getString( XML_inset ).get();
        OUString value;
        ConversionHelper::separatePair( value, inset, inset, ',' );
        rTextBox.borderDistanceLeft = ConversionHelper::decodeMeasureToEmu( graphicHelper,
            value.isEmpty() ? "0.1in" : value, 0, false, false );
        ConversionHelper::separatePair( value, inset, inset, ',' );
        rTextBox.borderDistanceTop = ConversionHelper::decodeMeasureToEmu( graphicHelper,
            value.isEmpty() ? "0.05in" : value, 0, false, false );
        ConversionHelper::separatePair( value, inset, inset, ',' );
        rTextBox.borderDistanceRight = ConversionHelper::decodeMeasureToEmu( graphicHelper,
            value.isEmpty() ? "0.1in" : value, 0, false, false );
        ConversionHelper::separatePair( value, inset, inset, ',' );
        rTextBox.borderDistanceBottom = ConversionHelper::decodeMeasureToEmu( graphicHelper,
            value.isEmpty() ? "0.05in" : value, 0, false, false );
        rTextBox.borderDistanceSet = true;
    }
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
