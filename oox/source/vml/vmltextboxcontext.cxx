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
#include "oox/vml/vmlshape.hxx"
#include <com/sun/star/drawing/XShape.hpp>

namespace oox {
namespace vml {

// ============================================================================

using ::oox::core::ContextHandler2;
using ::oox::core::ContextHandler2Helper;
using ::oox::core::ContextHandlerRef;

// ============================================================================

TextPortionContext::TextPortionContext( ContextHandler2Helper& rParent,
        TextBox& rTextBox, TextParagraphModel& rParagraph, const TextFontModel& rParentFont,
        sal_Int32 nElement, const AttributeList& rAttribs ) :
    ContextHandler2( rParent ),
    mrTextBox( rTextBox ),
    maParagraph( rParagraph ),
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
        case OOX_TOKEN(dml, blip):
            {
                OptValue<OUString> oRelId = rAttribs.getString(R_TOKEN(embed));
                if (oRelId.has())
                    mrTextBox.mrTypeModel.moGraphicPath = getFragmentPathFromRelId(oRelId.get());
            }
        break;
        case VML_TOKEN(imagedata):
            {
                OptValue<OUString> oRelId = rAttribs.getString(R_TOKEN(id));
                if (oRelId.has())
                    mrTextBox.mrTypeModel.moGraphicPath = getFragmentPathFromRelId(oRelId.get());
            }
        break;
        case XML_span:
        case OOX_TOKEN(doc, r):
        break;
        default:
            OSL_ENSURE( false, "TextPortionContext::TextPortionContext - unknown element" );
    }
}

ContextHandlerRef TextPortionContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    OSL_ENSURE( nElement != XML_font, "TextPortionContext::onCreateContext - nested <font> elements" );
    if (getNamespace(getCurrentElement()) == NMSP_doc)
        return this;
    return new TextPortionContext( *this, mrTextBox, maParagraph, maFont, nElement, rAttribs );
}

void TextPortionContext::onCharacters( const OUString& rChars )
{
    if (getNamespace(getCurrentElement()) == NMSP_doc && getCurrentElement() != OOX_TOKEN(doc, t))
        return;

    switch( getCurrentElement() )
    {
        case XML_span:
            // replace all NBSP characters with SP
            mrTextBox.appendPortion( maParagraph, maFont, rChars.replace( 0xA0, ' ' ) );
        break;
        default:
            mrTextBox.appendPortion( maParagraph, maFont, rChars );
    }
}

void TextPortionContext::onStartElement(const AttributeList& rAttribs)
{
    switch (getCurrentElement())
    {
        case OOX_TOKEN(doc, b):
            maFont.mobBold = true;
        break;
        case OOX_TOKEN(doc, sz):
            maFont.monSize = rAttribs.getInteger( OOX_TOKEN(doc, val) );
        break;
        case OOX_TOKEN(doc, br):
            mrTextBox.appendPortion( maParagraph, maFont, "\n" );
        break;
        case OOX_TOKEN(doc, color):
            maFont.moColor = rAttribs.getString( OOX_TOKEN(doc, val) );
        break;
    }
}

void TextPortionContext::onEndElement()
{
    if (getNamespace(getCurrentElement()) == NMSP_doc && getCurrentElement() != OOX_TOKEN(doc, t))
        return;

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
        mrTextBox.appendPortion( maParagraph, maFont, OUString( sal_Unicode( ' ' ) ) );
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
        OUString remainingStr;

        ConversionHelper::separatePair( value, remainingStr, inset, ',' );
        rTextBox.borderDistanceLeft = ConversionHelper::decodeMeasureToHmm( graphicHelper,
            value.isEmpty() ? "0.1in" : value, 0, false, false );

        inset = remainingStr;
        ConversionHelper::separatePair( value, remainingStr, inset, ',' );
        rTextBox.borderDistanceTop = ConversionHelper::decodeMeasureToHmm( graphicHelper,
            value.isEmpty() ? "0.05in" : value, 0, false, false );

        inset = remainingStr;
        ConversionHelper::separatePair( value, remainingStr, inset, ',' );
        rTextBox.borderDistanceRight = ConversionHelper::decodeMeasureToHmm( graphicHelper,
            value.isEmpty() ? "0.1in" : value, 0, false, false );

        inset = remainingStr;
        ConversionHelper::separatePair( value, remainingStr, inset, ',' );
        rTextBox.borderDistanceBottom = ConversionHelper::decodeMeasureToHmm( graphicHelper,
            value.isEmpty() ? "0.05in" : value, 0, false, false );

        rTextBox.borderDistanceSet = true;
    }

    OUString sStyle = rAttribs.getString( XML_style, OUString() );
    sal_Int32 nIndex = 0;
    while( nIndex >= 0 )
    {
        OUString aName, aValue;
        if( ConversionHelper::separatePair( aName, aValue, sStyle.getToken( 0, ';', nIndex ), ':' ) )
        {
            if( aName == "layout-flow" )      rTextBox.maLayoutFlow = aValue;
            else if (aName == "mso-fit-shape-to-text")
                rTextBox.mrTypeModel.mbAutoHeight = true;
            else
                SAL_WARN("oox", "unhandled style property: " << aName);
        }
    }
}

ContextHandlerRef TextBoxContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case VML_TOKEN( textbox ):
            if( nElement == XML_div ) return this;
            else if (nElement == OOX_TOKEN(doc, txbxContent)) return this;
        break;
        case XML_div:
            if( nElement == XML_font ) return new TextPortionContext( *this, mrTextBox, maParagraph, TextFontModel(), nElement, rAttribs );
        break;
        case OOX_TOKEN(doc, txbxContent):
            if (nElement == OOX_TOKEN(doc, p)) return this;
        break;
        case OOX_TOKEN(doc, p):
            if (nElement == OOX_TOKEN(doc, r))
                return new TextPortionContext( *this, mrTextBox, maParagraph, TextFontModel(), nElement, rAttribs );
            else
                return this;
        break;
        case OOX_TOKEN(doc, pPr):
            return this;
        break;
    }
    return 0;
}

void TextBoxContext::onStartElement(const AttributeList& rAttribs)
{
    switch (getCurrentElement())
    {
        case OOX_TOKEN(doc, jc):
            maParagraph.moParaAdjust = rAttribs.getString( OOX_TOKEN(doc, val) );
        break;
    }
}

void TextBoxContext::onEndElement()
{
    if (getCurrentElement() == OOX_TOKEN(doc, p))
    {
        mrTextBox.appendPortion( maParagraph, TextFontModel(), "\n" );
        maParagraph = TextParagraphModel();
    }
}

// ============================================================================

} // namespace vml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
