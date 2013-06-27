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

#include "oox/drawingml/textcharacterpropertiescontext.hxx"

#include "oox/helper/attributelist.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/colorchoicecontext.hxx"
#include "oox/drawingml/lineproperties.hxx"
#include "oox/drawingml/textparagraphproperties.hxx"
#include "oox/core/relations.hxx"
#include "hyperlinkcontext.hxx"

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

//  mrTextCharacterProperties.moCaseMap     = rAttribs.getToken( XML_cap );
    if ( rAttribs.hasAttribute( XML_b ) )
        mrTextCharacterProperties.moBold = rAttribs.getBool( XML_b );
    if ( rAttribs.hasAttribute( XML_i ) )
        mrTextCharacterProperties.moItalic = rAttribs.getBool( XML_i );

// TODO
/*   todo: we need to be able to iterate over the XFastAttributes

  // ST_TextNonNegativePoint
    const OUString sCharKerning( "CharKerning" );
    //case A_TOKEN( kern ):

  // ST_TextLanguageID
    OUString sAltLang = rAttribs.getString( XML_altLang ).get();

        case A_TOKEN( kumimoji ):       // xsd:boolean
            break;
        case A_TOKEN( spc ):            // ST_TextPoint
        case A_TOKEN( normalizeH ):     // xsd:boolean
        case A_TOKEN( baseline ):       // ST_Percentage
        case A_TOKEN( noProof ):        // xsd:boolean
        case A_TOKEN( dirty ):          // xsd:boolean
        case A_TOKEN( err ):            // xsd:boolean
        case A_TOKEN( smtClean ):       // xsd:boolean
        case A_TOKEN( smtId ):          // xsd:unsignedInt
            break;
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

        case A_TOKEN( solidFill ):  // EG_FillProperties
            return new ColorContext( *this, mrTextCharacterProperties.maCharColor );

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
    }

    return this;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
