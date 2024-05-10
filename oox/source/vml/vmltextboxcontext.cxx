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

#include <oox/helper/attributelist.hxx>
#include <oox/vml/vmlformatting.hxx>
#include <oox/vml/vmltextboxcontext.hxx>
#include <oox/vml/vmlshape.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <o3tl/string_view.hxx>
#include <utility>

namespace oox::vml {

using ::oox::core::ContextHandler2;
using ::oox::core::ContextHandler2Helper;
using ::oox::core::ContextHandlerRef;

TextPortionContext::TextPortionContext( ContextHandler2Helper const & rParent,
        TextBox& rTextBox, TextParagraphModel aParagraph, TextFontModel  aParentFont,
        sal_Int32 nElement, const AttributeList& rAttribs ) :
    ContextHandler2( rParent ),
    mrTextBox( rTextBox ),
    maParagraph(std::move( aParagraph )),
    maFont(std::move( aParentFont )),
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
            OSL_ENSURE( !maFont.mobBold.has_value(), "TextPortionContext::TextPortionContext - nested <b> elements" );
            maFont.mobBold = true;
        break;
        case XML_i:
            OSL_ENSURE( !maFont.mobItalic.has_value(), "TextPortionContext::TextPortionContext - nested <i> elements" );
            maFont.mobItalic = true;
        break;
        case XML_s:
            OSL_ENSURE( !maFont.mobStrikeout.has_value(), "TextPortionContext::TextPortionContext - nested <s> elements" );
            maFont.mobStrikeout = true;
        break;
        case OOX_TOKEN(dml, blip):
            {
                std::optional<OUString> oRelId = rAttribs.getString(R_TOKEN(embed));
                if (oRelId.has_value())
                    mrTextBox.mrTypeModel.moGraphicPath = getFragmentPathFromRelId(oRelId.value());
            }
        break;
        case VML_TOKEN(imagedata):
            {
                std::optional<OUString> oRelId = rAttribs.getString(R_TOKEN(id));
                if (oRelId.has_value())
                    mrTextBox.mrTypeModel.moGraphicPath = getFragmentPathFromRelId(oRelId.value());
            }
        break;
        case XML_span:
        case W_TOKEN(r):
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
    if (getNamespace(getCurrentElement()) == NMSP_doc && getCurrentElement() != W_TOKEN(t))
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
        case W_TOKEN(b):
            maFont.mobBold = true;
        break;
        case W_TOKEN(sz):
            maFont.monSize = rAttribs.getInteger( W_TOKEN(val) );
        break;
        case W_TOKEN(br):
            mrTextBox.appendPortion( maParagraph, maFont, u"\n"_ustr );
        break;
        case W_TOKEN(color):
            maFont.moColor = rAttribs.getString( W_TOKEN(val) );
        break;
        case W_TOKEN(spacing):
            maFont.monSpacing = rAttribs.getInteger(W_TOKEN(val));
        break;
        case W_TOKEN(r):
        case W_TOKEN(rPr):
        case W_TOKEN(t):
        break;
        case W_TOKEN(rFonts):
            // See https://msdn.microsoft.com/en-us/library/documentformat.openxml.wordprocessing.runfonts(v=office.14).aspx
            maFont.moName = rAttribs.getString(W_TOKEN(ascii));
            maFont.moNameAsian = rAttribs.getString(W_TOKEN(eastAsia));
            maFont.moNameComplex = rAttribs.getString(W_TOKEN(cs));
        break;
        default:
            SAL_INFO("oox", "unhandled: 0x" << std::hex<< getCurrentElement());
        break;
    }
}

void TextPortionContext::onEndElement()
{
    if (getNamespace(getCurrentElement()) == NMSP_doc && getCurrentElement() != W_TOKEN(t))
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
        mrTextBox.appendPortion( maParagraph, maFont, OUString( ' ' ) );
}

TextBoxContext::TextBoxContext( ContextHandler2Helper const & rParent, TextBox& rTextBox, const AttributeList& rAttribs,
    const GraphicHelper& graphicHelper ) :
    ContextHandler2( rParent ),
    mrTextBox( rTextBox )
{
    if( rAttribs.getStringDefaulted( XML_insetmode ) != "auto" )
    {
        OUString inset = rAttribs.getStringDefaulted( XML_inset );
        std::u16string_view value;
        std::u16string_view remainingStr;

        ConversionHelper::separatePair( value, remainingStr, inset, ',' );
        rTextBox.borderDistanceLeft = ConversionHelper::decodeMeasureToHmm( graphicHelper,
            value.empty() ? u"0.1in" : value, 0, false, false );

        inset = remainingStr;
        ConversionHelper::separatePair( value, remainingStr, inset, ',' );
        rTextBox.borderDistanceTop = ConversionHelper::decodeMeasureToHmm( graphicHelper,
            value.empty() ? u"0.05in" : value, 0, false, false );

        inset = remainingStr;
        ConversionHelper::separatePair( value, remainingStr, inset, ',' );
        rTextBox.borderDistanceRight = ConversionHelper::decodeMeasureToHmm( graphicHelper,
            value.empty() ? u"0.1in" : value, 0, false, false );

        inset = remainingStr;
        ConversionHelper::separatePair( value, remainingStr, inset, ',' );
        rTextBox.borderDistanceBottom = ConversionHelper::decodeMeasureToHmm( graphicHelper,
            value.empty() ? u"0.05in" : value, 0, false, false );

        rTextBox.borderDistanceSet = true;
    }

    OUString sStyle = rAttribs.getStringDefaulted( XML_style);
    sal_Int32 nIndex = 0;
    while( nIndex >= 0 )
    {
        std::u16string_view aName, aValue;
        if( ConversionHelper::separatePair( aName, aValue, o3tl::getToken(sStyle, 0, ';', nIndex ), ':' ) )
        {
            if( aName == u"layout-flow" )      rTextBox.maLayoutFlow = aValue;
            else if (aName == u"mso-fit-shape-to-text")
                rTextBox.mrTypeModel.mbAutoHeight = true;
            else if (aName == u"mso-layout-flow-alt")
                rTextBox.mrTypeModel.maLayoutFlowAlt = aValue;
            else if (aName == u"mso-next-textbox")
                rTextBox.msNextTextbox = aValue;
            else
                SAL_WARN("oox", "unhandled style property: " << OUString(aName));
        }
    }
}

ContextHandlerRef TextBoxContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case VML_TOKEN( textbox ):
            if( nElement == XML_div ) return this;
            else if (nElement == W_TOKEN(txbxContent)) return this;
        break;
        case XML_div:
            if( nElement == XML_font ) return new TextPortionContext( *this, mrTextBox, maParagraph, TextFontModel(), nElement, rAttribs );
        break;
        case W_TOKEN(txbxContent):
            if (nElement == W_TOKEN(p)) return this;
        break;
        case W_TOKEN(p):
        case W_TOKEN(sdtContent):
        case W_TOKEN(smartTag):
            if (nElement == W_TOKEN(r))
                return new TextPortionContext( *this, mrTextBox, maParagraph, TextFontModel(), nElement, rAttribs );
            else
                return this;
        case W_TOKEN(pPr):
        case W_TOKEN(sdt):
            return this;
        default:
            SAL_INFO("oox", "unhandled 0x" << std::hex << getCurrentElement());
        break;
    }
    return nullptr;
}

void TextBoxContext::onStartElement(const AttributeList& rAttribs)
{
    switch (getCurrentElement())
    {
        case W_TOKEN(jc):
            maParagraph.moParaAdjust = rAttribs.getString( W_TOKEN(val) );
        break;
        case W_TOKEN(pStyle):
            maParagraph.moParaStyleName = rAttribs.getString( W_TOKEN(val) );
        break;
    }
}

void TextBoxContext::onEndElement()
{
    if (getCurrentElement() == W_TOKEN(p))
    {
        mrTextBox.appendPortion( maParagraph, TextFontModel(), u"\n"_ustr );
        maParagraph = TextParagraphModel();
    }
}

} // namespace oox::vml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
