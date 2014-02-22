/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
    if ( rAttribs.hasAttribute( XML_baseline ) )
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




        case A_TOKEN( solidFill ):  
            return new ColorContext( *this, mrTextCharacterProperties.maCharColor );

        
        case A_TOKEN( effectDag ):  
        case A_TOKEN( effectLst ):  
        break;

        case A_TOKEN( highlight ):  
            return new ColorContext( *this, mrTextCharacterProperties.maHighlightColor );

        
        case A_TOKEN( uLnTx ):      
            mrTextCharacterProperties.moUnderlineLineFollowText = true;
        break;




        
        case A_TOKEN( uFillTx ):    
            mrTextCharacterProperties.moUnderlineFillFollowText = true;
        break;
        case A_TOKEN( uFill ):      
            return new SimpleFillPropertiesContext( *this, mrTextCharacterProperties.maUnderlineColor );

        
        case A_TOKEN( latin ):      
            mrTextCharacterProperties.maLatinFont.setAttributes( rAttribs );
        break;
        case A_TOKEN( ea ):         
            mrTextCharacterProperties.maAsianFont.setAttributes( rAttribs );
        break;
        case A_TOKEN( cs ):         
            mrTextCharacterProperties.maComplexFont.setAttributes( rAttribs );
        break;
        case A_TOKEN( sym ):        
            mrTextCharacterProperties.maSymbolFont.setAttributes( rAttribs );
        break;

        case A_TOKEN( hlinkClick ):     
        case A_TOKEN( hlinkMouseOver ): 
            return new HyperLinkContext( *this, rAttribs,  mrTextCharacterProperties.maHyperlinkPropertyMap );
        case OOX_TOKEN( doc, rFonts ):
            if( rAttribs.hasAttribute(OOX_TOKEN(doc, ascii)) )
            {
                mrTextCharacterProperties.maLatinFont.setAttributes(rAttribs.getString(OOX_TOKEN(doc, ascii), OUString()));
            }
            if( rAttribs.hasAttribute(OOX_TOKEN(doc, cs)) )
            {
                mrTextCharacterProperties.maComplexFont.setAttributes(rAttribs.getString(OOX_TOKEN(doc, cs), OUString()));
            }
            if( rAttribs.hasAttribute(OOX_TOKEN(doc, eastAsia)) )
            {
                mrTextCharacterProperties.maAsianFont.setAttributes(rAttribs.getString(OOX_TOKEN(doc, eastAsia), OUString()));
            }
            break;
        case OOX_TOKEN( doc, b ):
            mrTextCharacterProperties.moBold = rAttribs.getBool(OOX_TOKEN( doc, val ), true);
            break;
        case OOX_TOKEN( doc, bCs ):
            break;
        case OOX_TOKEN( doc, color ):
            if (rAttribs.getInteger(OOX_TOKEN(doc, val)).has())
            {
                mrTextCharacterProperties.maCharColor.setSrgbClr(rAttribs.getIntegerHex(OOX_TOKEN(doc, val)).get());
            }
            break;
        case OOX_TOKEN( doc, sz ):
            if (rAttribs.getInteger(OOX_TOKEN(doc, val)).has())
            {
                sal_Int32 nVal = rAttribs.getInteger(OOX_TOKEN(doc, val)).get();
                
                mrTextCharacterProperties.moHeight = nVal * 50;
            }
            break;
        case OOX_TOKEN( doc, szCs ):
            break;
        case OOX_TOKEN( doc, caps ):
            {
                if( rAttribs.getBool(OOX_TOKEN( doc, val ), true) )
                    mrTextCharacterProperties.moCaseMap = XML_all;
                else
                    mrTextCharacterProperties.moCaseMap = XML_none;
            }
            break;
        case OOX_TOKEN( doc, smallCaps ):
            {
                if( rAttribs.getBool(OOX_TOKEN( doc, val ), true) )
                    mrTextCharacterProperties.moCaseMap = XML_small;
                else
                    mrTextCharacterProperties.moCaseMap = XML_none;
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
