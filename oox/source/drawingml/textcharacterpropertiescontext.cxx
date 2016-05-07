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

#include "drawingml/textcharacterpropertiescontext.hxx"

#include "oox/helper/attributelist.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "drawingml/colorchoicecontext.hxx"
#include "drawingml/texteffectscontext.hxx"
#include "oox/drawingml/lineproperties.hxx"
#include "drawingml/textparagraphproperties.hxx"
#include "oox/core/relations.hxx"
#include "hyperlinkcontext.hxx"
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::awt;

namespace oox { namespace drawingml {

// CT_TextCharacterProperties
TextCharacterPropertiesContext::TextCharacterPropertiesContext(
        ContextHandler2Helper& rParent,
        const AttributeList& rAttribs,
        TextCharacterProperties& rTextCharacterProperties )
: ContextHandler2( rParent )
, mrTextCharacterProperties( rTextCharacterProperties )
{
    if ( rAttribs.hasAttribute( XML_lang ) )
        mrTextCharacterProperties.moLang = rAttribs.getString( XML_lang );
    if ( rAttribs.hasAttribute( XML_sz ) )
        mrTextCharacterProperties.moHeight = rAttribs.getInteger( XML_sz );
    if ( rAttribs.hasAttribute( XML_spc ) )
        mrTextCharacterProperties.moSpacing = rAttribs.getInteger( XML_spc );
    if ( rAttribs.hasAttribute( XML_u ) )
        mrTextCharacterProperties.moUnderline = rAttribs.getToken( XML_u );
    if ( rAttribs.hasAttribute( XML_strike ) )
        mrTextCharacterProperties.moStrikeout = rAttribs.getToken( XML_strike );
    if ( rAttribs.hasAttribute( XML_baseline ) && rAttribs.getInteger( XML_baseline ).get() != 0 )
        mrTextCharacterProperties.moBaseline = rAttribs.getInteger( XML_baseline );

    if ( rAttribs.hasAttribute( XML_b ) )
        mrTextCharacterProperties.moBold = rAttribs.getBool( XML_b );
    if ( rAttribs.hasAttribute( XML_i ) )
        mrTextCharacterProperties.moItalic = rAttribs.getBool( XML_i );
    if( rAttribs.hasAttribute( XML_cap ) )
        mrTextCharacterProperties.moCaseMap = rAttribs.getToken( XML_cap );

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
    switch( aElementToken )
    {
// TODO unsupported yet
//        case A_TOKEN( ln ):         // CT_LineProperties
//            return new LinePropertiesContext( getHandler(), rAttribs, maTextOutlineProperties );
        // EG_FillProperties
        case A_TOKEN( noFill ):
        case A_TOKEN( solidFill ):
        case A_TOKEN( gradFill ):
        case A_TOKEN( pattFill ):
            return FillPropertiesContext::createFillContext( *this, aElementToken, rAttribs, mrTextCharacterProperties.maFillProperties );
        // EG_EffectProperties
        case A_TOKEN( effectDag ):  // CT_EffectContainer 5.1.10.25
        case A_TOKEN( effectLst ):  // CT_EffectList 5.1.10.26
        break;

        case A_TOKEN( highlight ):  // CT_Color
            return new ColorContext( *this, mrTextCharacterProperties.maHighlightColor );

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
                mrTextCharacterProperties.maLatinFont.setAttributes(rAttribs.getString(W_TOKEN(ascii), OUString()));
            }
            if (rAttribs.hasAttribute(W_TOKEN(asciiTheme)))
            {
                mrTextCharacterProperties.maLatinThemeFont.setAttributes(rAttribs.getString(W_TOKEN(asciiTheme), OUString()));
            }
            if( rAttribs.hasAttribute(W_TOKEN(cs)) )
            {
                mrTextCharacterProperties.maComplexFont.setAttributes(rAttribs.getString(W_TOKEN(cs), OUString()));
            }
            if (rAttribs.hasAttribute(W_TOKEN(cstheme)))
            {
                mrTextCharacterProperties.maComplexThemeFont.setAttributes(rAttribs.getString(W_TOKEN(cstheme), OUString()));
            }
            if( rAttribs.hasAttribute(W_TOKEN(eastAsia)) )
            {
                mrTextCharacterProperties.maAsianFont.setAttributes(rAttribs.getString(W_TOKEN(eastAsia), OUString()));
            }
            if (rAttribs.hasAttribute(W_TOKEN(eastAsiaTheme)))
            {
                mrTextCharacterProperties.maAsianThemeFont.setAttributes(rAttribs.getString(W_TOKEN(eastAsiaTheme), OUString()));
            }
            break;
        case W_TOKEN( b ):
            mrTextCharacterProperties.moBold = rAttribs.getBool(W_TOKEN( val ), true);
            break;
        case W_TOKEN( i ):
            mrTextCharacterProperties.moItalic = rAttribs.getBool(W_TOKEN( val ), true);
            break;
        case W_TOKEN( bCs ):
            break;
        case W_TOKEN( color ):
            if (rAttribs.getInteger(W_TOKEN(val)).has())
            {
                mrTextCharacterProperties.maFillProperties.maFillColor.setSrgbClr(rAttribs.getIntegerHex(W_TOKEN(val)).get());
                mrTextCharacterProperties.maFillProperties.moFillType.set(XML_solidFill);
            }
            break;
        case W_TOKEN(  sz ):
            if (rAttribs.getInteger(W_TOKEN(val)).has())
            {
                sal_Int32 nVal = rAttribs.getInteger(W_TOKEN(val)).get();
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

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
