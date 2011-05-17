/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "oox/drawingml/textcharacterpropertiescontext.hxx"

#include "oox/helper/attributelist.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/colorchoicecontext.hxx"
#include "oox/drawingml/lineproperties.hxx"
#include "oox/drawingml/textparagraphproperties.hxx"
#include "oox/core/relations.hxx"
#include "hyperlinkcontext.hxx"

using ::rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::awt;

namespace oox { namespace drawingml {

// --------------------------------------------------------------------

// CT_TextCharacterProperties
TextCharacterPropertiesContext::TextCharacterPropertiesContext(
        ContextHandler& rParent,
        const Reference< XFastAttributeList >& rXAttributes,
        TextCharacterProperties& rTextCharacterProperties )
: ContextHandler( rParent )
, mrTextCharacterProperties( rTextCharacterProperties )
{
    AttributeList aAttribs( rXAttributes );
    if ( aAttribs.hasAttribute( XML_lang ) )
        mrTextCharacterProperties.moLang = aAttribs.getString( XML_lang );
    if ( aAttribs.hasAttribute( XML_sz ) )
        mrTextCharacterProperties.moHeight = aAttribs.getInteger( XML_sz );
    if ( aAttribs.hasAttribute( XML_spc ) )
        mrTextCharacterProperties.moSpacing = aAttribs.getInteger( XML_spc );
    if ( aAttribs.hasAttribute( XML_u ) )
        mrTextCharacterProperties.moUnderline = aAttribs.getToken( XML_u );
    if ( aAttribs.hasAttribute( XML_strike ) )
        mrTextCharacterProperties.moStrikeout = aAttribs.getToken( XML_strike );

//  mrTextCharacterProperties.moCaseMap     = aAttribs.getToken( XML_cap );
    if ( aAttribs.hasAttribute( XML_b ) )
        mrTextCharacterProperties.moBold = aAttribs.getBool( XML_b );
    if ( aAttribs.hasAttribute( XML_i ) )
        mrTextCharacterProperties.moItalic = aAttribs.getBool( XML_i );

// TODO
/*   todo: we need to be able to iterate over the XFastAttributes

  // ST_TextNonNegativePoint
    const OUString sCharKerning( CREATE_OUSTRING( "CharKerning" ) );
    //case A_TOKEN( kern ):

  // ST_TextLanguageID
    OUString sAltLang = rXAttributes->getOptionalValue( XML_altLang );

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

// --------------------------------------------------------------------

void TextCharacterPropertiesContext::endFastElement( sal_Int32 ) throw (SAXException, RuntimeException)
{
}

// --------------------------------------------------------------------

Reference< XFastContextHandler > TextCharacterPropertiesContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttributes ) throw (SAXException, RuntimeException)
{
    AttributeList aAttribs( xAttributes );
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
// TODO unsupported yet
//        case A_TOKEN( ln ):         // CT_LineProperties
//            xRet.set( new LinePropertiesContext( getHandler(), xAttributes, maTextOutlineProperties ) );
//        break;

        case A_TOKEN( solidFill ):  // EG_FillProperties
            xRet.set( new ColorContext( *this, mrTextCharacterProperties.maCharColor ) );
        break;

        // EG_EffectProperties
        case A_TOKEN( effectDag ):  // CT_EffectContainer 5.1.10.25
        case A_TOKEN( effectLst ):  // CT_EffectList 5.1.10.26
        break;

        case A_TOKEN( highlight ):  // CT_Color
            xRet.set( new ColorContext( *this, mrTextCharacterProperties.maHighlightColor ) );
        break;

        // EG_TextUnderlineLine
        case A_TOKEN( uLnTx ):      // CT_TextUnderlineLineFollowText
            mrTextCharacterProperties.moUnderlineLineFollowText = true;
        break;
// TODO unsupported yet
//        case A_TOKEN( uLn ):        // CT_LineProperties
//            xRet.set( new LinePropertiesContext( getHandler(), xAttributes, maUnderlineProperties ) );
//        break;

        // EG_TextUnderlineFill
        case A_TOKEN( uFillTx ):    // CT_TextUnderlineFillFollowText
            mrTextCharacterProperties.moUnderlineFillFollowText = true;
        break;
        case A_TOKEN( uFill ):      // CT_TextUnderlineFillGroupWrapper->EG_FillProperties (not supported)
            xRet.set( new SimpleFillPropertiesContext( *this, mrTextCharacterProperties.maUnderlineColor ) );
        break;

        // CT_FontCollection
        case A_TOKEN( latin ):      // CT_TextFont
            mrTextCharacterProperties.maLatinFont.setAttributes( aAttribs );
        break;
        case A_TOKEN( ea ):         // CT_TextFont
            mrTextCharacterProperties.maAsianFont.setAttributes( aAttribs );
        break;
        case A_TOKEN( cs ):         // CT_TextFont
            mrTextCharacterProperties.maComplexFont.setAttributes( aAttribs );
        break;
        case A_TOKEN( sym ):        // CT_TextFont
            mrTextCharacterProperties.maSymbolFont.setAttributes( aAttribs );
        break;

        case A_TOKEN( hlinkClick ):     // CT_Hyperlink
        case A_TOKEN( hlinkMouseOver ): // CT_Hyperlink
            xRet.set( new HyperLinkContext( *this, xAttributes,  mrTextCharacterProperties.maHyperlinkPropertyMap ) );
        break;
    }
    if( !xRet.is() )
        xRet.set( this );
    return xRet;
}

// --------------------------------------------------------------------

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
