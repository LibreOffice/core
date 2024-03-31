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

#include <drawingml/textcharacterpropertiescontext.hxx>

#include <oox/helper/attributelist.hxx>
#include <drawingml/colorchoicecontext.hxx>
#include <drawingml/linepropertiescontext.hxx>
#include <drawingml/misccontexts.hxx>
#include <drawingml/textcharacterproperties.hxx>
#include <drawingml/texteffectscontext.hxx>
#include "hyperlinkcontext.hxx"
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <sax/fastattribs.hxx>

#include <sal/log.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star::uno;

namespace oox::drawingml {

// CT_TextCharacterProperties
TextCharacterPropertiesContext::TextCharacterPropertiesContext(
        ContextHandler2Helper const & rParent,
        const AttributeList& rAttribs,
        TextCharacterProperties& rTextCharacterProperties )
: ContextHandler2( rParent )
, mrTextCharacterProperties( rTextCharacterProperties )
{
    int nVisualTokenAmount = sax_fastparser::castToFastAttributeList(
                rAttribs.getFastAttributeList() ).getFastAttributeTokens().size();

    if (nVisualTokenAmount == 0)
        mrTextCharacterProperties.mbHasEmptyParaProperties = true;

    if ( rAttribs.hasAttribute( XML_lang ) )
    {
        mrTextCharacterProperties.moLang = rAttribs.getString( XML_lang );
        --nVisualTokenAmount; // Not a visual attribute
    }
    if ( rAttribs.hasAttribute( XML_altLang ))
    {
        --nVisualTokenAmount; // Not a visual attribute
    }
    if ( rAttribs.hasAttribute( XML_sz ) )
        mrTextCharacterProperties.moHeight = rAttribs.getInteger( XML_sz );
    if ( rAttribs.hasAttribute( XML_spc ) )
        mrTextCharacterProperties.moSpacing = rAttribs.getInteger( XML_spc );
    if ( rAttribs.hasAttribute( XML_u ) )
        mrTextCharacterProperties.moUnderline = rAttribs.getToken( XML_u );
    if ( rAttribs.hasAttribute( XML_strike ) )
        mrTextCharacterProperties.moStrikeout = rAttribs.getToken( XML_strike );
    if ( rAttribs.hasAttribute( XML_baseline ) && rAttribs.getInteger( XML_baseline, 0 ) != 0 )
        mrTextCharacterProperties.moBaseline = rAttribs.getInteger( XML_baseline );

    if ( rAttribs.hasAttribute( XML_b ) )
        mrTextCharacterProperties.moBold = rAttribs.getBool( XML_b );
    if ( rAttribs.hasAttribute( XML_i ) )
        mrTextCharacterProperties.moItalic = rAttribs.getBool( XML_i );
    if( rAttribs.hasAttribute( XML_cap ) )
        mrTextCharacterProperties.moCaseMap = rAttribs.getToken( XML_cap );
    if ( rAttribs.hasAttribute( XML_dirty ) )
    {
        --nVisualTokenAmount; // Not a visual attribute
    }
    if ( rAttribs.hasAttribute( XML_smtClean ) )
    {
        --nVisualTokenAmount; // Not a visual attribute
    }

    if ( nVisualTokenAmount > 0 )
        mrTextCharacterProperties.mbHasVisualRunProperties = true;

    /* TODO / unhandled so far:
       A_TOKEN( kern )
       XML_altLang
       A_TOKEN( kumimoji )
       A_TOKEN( spc )
       A_TOKEN( normalizeH )
       A_TOKEN( noProof )
       A_TOKEN( dirty )
       A_TOKEN( err )
       A_TOKEN( smtClean )
       A_TOKEN( smtId )
    */
}

TextCharacterPropertiesContext::~TextCharacterPropertiesContext()
{
}

ContextHandlerRef TextCharacterPropertiesContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    if( aElementToken != A_TOKEN(lang) )
        mrTextCharacterProperties.mbHasVisualRunProperties = true;

    switch( aElementToken )
    {
        case A_TOKEN(ln): // CT_LineProperties
            // TODO still largely unsupported
            if (!mrTextCharacterProperties.moTextOutlineProperties.has_value())
                mrTextCharacterProperties.moTextOutlineProperties.emplace();
            return new LinePropertiesContext(*this, rAttribs, *mrTextCharacterProperties.moTextOutlineProperties);
        // EG_FillProperties
        case A_TOKEN( noFill ):
        case A_TOKEN( solidFill ):
        case A_TOKEN( gradFill ):
        case A_TOKEN( pattFill ):
        case A_TOKEN( blipFill ): // Fontwork uses blibFill.
            return FillPropertiesContext::createFillContext(*this, aElementToken, rAttribs, mrTextCharacterProperties.maFillProperties, nullptr);
        // EG_EffectProperties
        case A_TOKEN( effectDag ):  // CT_EffectContainer 5.1.10.25
        case A_TOKEN( effectLst ):  // CT_EffectList 5.1.10.26
        break;
        case A_TOKEN( highlight ):  // CT_Color
            return new ColorContext(*this, mrTextCharacterProperties.maHighlightColor);
        case W_TOKEN( highlight ):
            mrTextCharacterProperties.maHighlightColor = rAttribs.getHighlightColor(W_TOKEN(val));
            break;
        // EG_TextUnderlineLine
        case A_TOKEN( uLnTx ):      // CT_TextUnderlineLineFollowText
            mrTextCharacterProperties.moUnderlineLineFollowText = true;
        break;
// TODO unsupported yet
//        case A_TOKEN( uLn ):        // CT_LineProperties
//            return new LinePropertiesContext( getHandler(), rAttribs, maUnderlineProperties );

        // EG_TextUnderlineFill
        case A_TOKEN( uFillTx ):    // CT_TextUnderlineFillFollowText
            mrTextCharacterProperties.moUnderlineFillFollowText = true;
        break;
        case A_TOKEN( uFill ):      // CT_TextUnderlineFillGroupWrapper->EG_FillProperties (not supported)
            return new SimpleFillPropertiesContext( *this, mrTextCharacterProperties.maUnderlineColor );

        // CT_FontCollection
        case A_TOKEN( latin ):      // CT_TextFont
            mrTextCharacterProperties.maLatinFont.setAttributes( rAttribs );
        break;
        case A_TOKEN( ea ):         // CT_TextFont
            mrTextCharacterProperties.maAsianFont.setAttributes( rAttribs );
        break;
        case A_TOKEN( cs ):         // CT_TextFont
            mrTextCharacterProperties.maComplexFont.setAttributes( rAttribs );
        break;
        case A_TOKEN( sym ):        // CT_TextFont
            mrTextCharacterProperties.maSymbolFont.setAttributes( rAttribs );
        break;

        case A_TOKEN( hlinkClick ):     // CT_Hyperlink
        case A_TOKEN( hlinkMouseOver ): // CT_Hyperlink
            return new HyperLinkContext( *this, rAttribs,  mrTextCharacterProperties.maHyperlinkPropertyMap );
        case W_TOKEN( rFonts ):
            if( rAttribs.hasAttribute(W_TOKEN(ascii)) )
            {
                mrTextCharacterProperties.maLatinFont.setAttributes(rAttribs.getStringDefaulted(W_TOKEN(ascii)));
            }
            if (rAttribs.hasAttribute(W_TOKEN(asciiTheme)))
            {
                mrTextCharacterProperties.maLatinThemeFont.setAttributes(rAttribs.getStringDefaulted(W_TOKEN(asciiTheme)));
            }
            if( rAttribs.hasAttribute(W_TOKEN(cs)) )
            {
                mrTextCharacterProperties.maComplexFont.setAttributes(rAttribs.getStringDefaulted(W_TOKEN(cs)));
            }
            if (rAttribs.hasAttribute(W_TOKEN(cstheme)))
            {
                mrTextCharacterProperties.maComplexThemeFont.setAttributes(rAttribs.getStringDefaulted(W_TOKEN(cstheme)));
            }
            if( rAttribs.hasAttribute(W_TOKEN(eastAsia)) )
            {
                mrTextCharacterProperties.maAsianFont.setAttributes(rAttribs.getStringDefaulted(W_TOKEN(eastAsia)));
            }
            if (rAttribs.hasAttribute(W_TOKEN(eastAsiaTheme)))
            {
                mrTextCharacterProperties.maAsianThemeFont.setAttributes(rAttribs.getStringDefaulted(W_TOKEN(eastAsiaTheme)));
            }
            break;
        case W_TOKEN( u ):
        {
            // If you add here, check if it is in drawingmltypes.cxx 113.
            auto attrib = rAttribs.getStringDefaulted(W_TOKEN(val));
            if (attrib == "single" || attrib == "words") // TODO: implement words properly. Now it is a single line.
                mrTextCharacterProperties.moUnderline = XML_sng;
            else if (attrib == "wavyHeavy")
                mrTextCharacterProperties.moUnderline = XML_wavyHeavy;
            else if (attrib == "wavyDouble")
                mrTextCharacterProperties.moUnderline = XML_wavyDbl;
            else if (attrib == "wave")
                mrTextCharacterProperties.moUnderline = XML_wavy;
            else if (attrib == "thick")
                mrTextCharacterProperties.moUnderline = XML_heavy;
            else if (attrib == "dottedHeavy")
                mrTextCharacterProperties.moUnderline = XML_dottedHeavy;
            else if (attrib == "dotted")
                mrTextCharacterProperties.moUnderline = XML_dotted;
            else if (attrib == "dashDotDotHeavy")
                mrTextCharacterProperties.moUnderline = XML_dotDotDashHeavy;
            else if (attrib == "dotDotDash")
                mrTextCharacterProperties.moUnderline = XML_dotDotDash;
            else if (attrib == "dashDotHeavy")
                mrTextCharacterProperties.moUnderline = XML_dotDashHeavy;
            else if (attrib == "dotDash")
                mrTextCharacterProperties.moUnderline = XML_dotDash;
            else if (attrib == "double")
                mrTextCharacterProperties.moUnderline = XML_dbl;
            else if (attrib == "dashLongHeavy")
                mrTextCharacterProperties.moUnderline = XML_dashLongHeavy;
            else if (attrib == "dashLong")
                mrTextCharacterProperties.moUnderline = XML_dashLong;
            else if (attrib == "dashedHeavy")
                mrTextCharacterProperties.moUnderline = XML_dashHeavy;
            else if (attrib == "dash")
                mrTextCharacterProperties.moUnderline = XML_dash;
            else if (attrib == "none")
                mrTextCharacterProperties.moUnderline = XML_none;
            auto colorAttrib = rAttribs.getIntegerHex(W_TOKEN(color));
            if (colorAttrib.has_value())
            {
                oox::drawingml::Color theColor;
                theColor.setSrgbClr(colorAttrib.value());
                mrTextCharacterProperties.maUnderlineColor = theColor;
            }
            break;
        }
        case W_TOKEN( spacing ):
        {
            auto attrib = rAttribs.getInteger(W_TOKEN( val ), 0);
            mrTextCharacterProperties.moSpacing = attrib;
            break;
        }
        case W_TOKEN( b ):
            mrTextCharacterProperties.moBold = rAttribs.getBool(W_TOKEN( val ), true);
            break;
        case W_TOKEN( i ):
            mrTextCharacterProperties.moItalic = rAttribs.getBool(W_TOKEN( val ), true);
            break;
        case W_TOKEN( bCs ):
            break;
        case W_TOKEN( strike ):
            if (rAttribs.getBool(W_TOKEN(val), true))
                mrTextCharacterProperties.moStrikeout = XML_sngStrike;
            break;
        case W_TOKEN( dstrike ):
            if (rAttribs.getBool(W_TOKEN(val), true))
                mrTextCharacterProperties.moStrikeout = XML_dblStrike;
            break;
        case W_TOKEN( color ):
            if (rAttribs.getInteger(W_TOKEN(val)).has_value())
            {
                mrTextCharacterProperties.maFillProperties.maFillColor.setSrgbClr(rAttribs.getIntegerHex(W_TOKEN(val), 0));
                mrTextCharacterProperties.maFillProperties.moFillType = XML_solidFill;
            }
            break;
        case W_TOKEN(  sz ):
            if (rAttribs.getInteger(W_TOKEN(val)).has_value())
            {
                sal_Int32 nVal = rAttribs.getInteger(W_TOKEN(val), 0);
                // wml has half points, dml has hundred points
                mrTextCharacterProperties.moHeight = nVal * 50;
            }
            break;
        case W_TOKEN( szCs ):
            break;
        case W_TOKEN( caps ):
            {
                if( rAttribs.getBool(W_TOKEN( val ), true) )
                    mrTextCharacterProperties.moCaseMap = XML_all;
                else
                    mrTextCharacterProperties.moCaseMap = XML_none;
            }
            break;
        case W_TOKEN( smallCaps ):
            {
                if( rAttribs.getBool(W_TOKEN( val ), true) )
                    mrTextCharacterProperties.moCaseMap = XML_small;
                else
                    mrTextCharacterProperties.moCaseMap = XML_none;
            }
            break;
        case W_TOKEN(vertAlign):
        {
            // Map wordprocessingML <w:vertAlign w:val="..."/> to drawingML
            // <a:rPr baseline="...">.
            sal_Int32 nVal = rAttribs.getToken(W_TOKEN(val), 0);
            if (nVal == XML_superscript)
                mrTextCharacterProperties.moBaseline = 30000;
            else if (nVal == XML_subscript)
                mrTextCharacterProperties.moBaseline = -25000;
            break;
        }
        case W_TOKEN(lang):
            mrTextCharacterProperties.moLang = rAttribs.getStringDefaulted(W_TOKEN(val));
            break;
        case OOX_TOKEN(w14, glow):
        case OOX_TOKEN(w14, shadow):
        case OOX_TOKEN(w14, reflection):
        case OOX_TOKEN(w14, textOutline):
        case OOX_TOKEN(w14, textFill):
        case OOX_TOKEN(w14, scene3d):
        case OOX_TOKEN(w14, props3d):
        case OOX_TOKEN(w14, ligatures):
        case OOX_TOKEN(w14, numForm):
        case OOX_TOKEN(w14, numSpacing):
        case OOX_TOKEN(w14, stylisticSets):
        case OOX_TOKEN(w14, cntxtAlts):
            {
                return new TextEffectsContext( *this, aElementToken, mrTextCharacterProperties.maTextEffectsProperties );
            }
            break;
        default:
            SAL_WARN("oox", "TextCharacterPropertiesContext::onCreateContext: unhandled element: " << getBaseToken(aElementToken));
            break;
    }

    return this;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
