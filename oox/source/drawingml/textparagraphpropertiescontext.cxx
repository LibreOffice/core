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

#include "oox/drawingml/textparagraphpropertiescontext.hxx"

#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>

#include <svx/unopage.hxx>

#include "oox/drawingml/colorchoicecontext.hxx"
#include "oox/drawingml/textcharacterpropertiescontext.hxx"
#include "oox/drawingml/fillproperties.hxx"
#include "oox/helper/attributelist.hxx"
#include "textspacingcontext.hxx"
#include "texttabstoplistcontext.hxx"

using namespace ::oox::core;
using ::com::sun::star::awt::FontDescriptor;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;

namespace oox { namespace drawingml {

// CT_TextParagraphProperties
TextParagraphPropertiesContext::TextParagraphPropertiesContext( ContextHandler2Helper& rParent,
                                                                const AttributeList& rAttribs,
                                                                TextParagraphProperties& rTextParagraphProperties )
: ContextHandler2( rParent )
, mrTextParagraphProperties( rTextParagraphProperties )
, mrSpaceBefore( rTextParagraphProperties.getParaTopMargin() )
, mrSpaceAfter( rTextParagraphProperties.getParaBottomMargin() )
, mrBulletList( rTextParagraphProperties.getBulletList() )
{
    OUString sValue;

    PropertyMap& rPropertyMap( mrTextParagraphProperties.getTextParagraphPropertyMap() );

    // ST_TextAlignType
    rPropertyMap[ PROP_ParaAdjust ] <<= GetParaAdjust( rAttribs.getToken( XML_algn, XML_l ) );
    // TODO see to do the same with RubyAdjust

    // ST_Coordinate32
//  sValue = rAttribs.getString( XML_defTabSz ).get();    SJ: we need to be able to set the default tab size for each text object,
//                                                          this is possible at the moment only for the whole document.
//  sal_Int32 nDefTabSize = ( sValue.getLength() == 0 ? 0 : GetCoordinate(  sValue ) );
    // TODO

//  bool bEaLineBrk = rAttribs.getBool( XML_eaLnBrk, true );
    if ( rAttribs.hasAttribute( XML_latinLnBrk ) )
    {
        bool bLatinLineBrk = rAttribs.getBool( XML_latinLnBrk, true );
        rPropertyMap[ PROP_ParaIsHyphenation ] <<= bLatinLineBrk;
    }
    // TODO see what to do with Asian hyphenation

    // ST_TextFontAlignType
    // TODO
//  sal_Int32 nFontAlign = rAttribs.getToken( XML_fontAlgn, XML_base );

    if ( rAttribs.hasAttribute( XML_hangingPunct ) )
    {
        bool bHangingPunct = rAttribs.getBool( XML_hangingPunct, false );
        rPropertyMap[ PROP_ParaIsHangingPunctuation ] <<= bHangingPunct;
    }

  // ST_Coordinate
    if ( rAttribs.hasAttribute( XML_indent ) )
    {
        sValue = rAttribs.getString( XML_indent ).get();
        mrTextParagraphProperties.getFirstLineIndentation() = boost::optional< sal_Int32 >( sValue.isEmpty() ? 0 : GetCoordinate( sValue ) );
    }

  // ST_TextIndentLevelType
    // -1 is an invalid value and denote the lack of level
    sal_Int32 nLevel = rAttribs.getInteger( XML_lvl, 0 );
    if( nLevel > 8 || nLevel < 0 )
    {
        nLevel = 0;
    }

    mrTextParagraphProperties.setLevel( static_cast< sal_Int16 >( nLevel ) );

    char name[] = "Outline X";
    name[8] = static_cast<char>( '1' + nLevel );
    const OUString sStyleNameValue( OUString::createFromAscii( name ) );
    mrBulletList.setStyleName( sStyleNameValue );

    // ST_TextMargin
    // ParaLeftMargin
    if ( rAttribs.hasAttribute( XML_marL ) )
    {
        sValue = rAttribs.getString( XML_marL ).get();
        mrTextParagraphProperties.getParaLeftMargin() = boost::optional< sal_Int32 >( sValue.isEmpty() ? 0 : GetCoordinate( sValue ) );
    }

    // ParaRightMargin
    if ( rAttribs.hasAttribute( XML_marR ) )
    {
        sValue = rAttribs.getString( XML_marR ).get();
        sal_Int32 nMarR  = sValue.isEmpty() ? 0 : GetCoordinate( sValue ) ;
        rPropertyMap[ PROP_ParaRightMargin ] <<= nMarR;
    }

    if ( rAttribs.hasAttribute( XML_rtl ) )
    {
        bool bRtl = rAttribs.getBool( XML_rtl, false );
        rPropertyMap[ PROP_WritingMode ] <<= ( bRtl ? WritingMode2::RL_TB : WritingMode2::LR_TB );
    }
}


TextParagraphPropertiesContext::~TextParagraphPropertiesContext()
{
    PropertyMap& rPropertyMap( mrTextParagraphProperties.getTextParagraphPropertyMap() );
    if ( maLineSpacing.bHasValue )
        rPropertyMap[ PROP_ParaLineSpacing ] <<= maLineSpacing.toLineSpacing();
    else
        rPropertyMap[ PROP_ParaLineSpacing ] <<= ::com::sun::star::style::LineSpacing( ::com::sun::star::style::LineSpacingMode::PROP, 100 );


    ::std::list< TabStop >::size_type nTabCount = maTabList.size();
    if( nTabCount != 0 )
    {
        Sequence< TabStop > aSeq( nTabCount );
        TabStop * aArray = aSeq.getArray();
        OSL_ENSURE( aArray != NULL, "sequence array is NULL" );
        ::std::copy( maTabList.begin(), maTabList.end(), aArray );
        rPropertyMap[ PROP_ParaTabStops ] <<= aSeq;
    }

    if ( mxBlipProps.get() && mxBlipProps->mxGraphic.is() )
        mrBulletList.setGraphic( mxBlipProps->mxGraphic );

    if( mrBulletList.is() )
        rPropertyMap[ PROP_IsNumbering ] <<= sal_True;
    sal_Int16 nLevel = mrTextParagraphProperties.getLevel();
    rPropertyMap[ PROP_NumberingLevel ] <<= nLevel;
    rPropertyMap[ PROP_NumberingIsNumber ] <<= sal_True;

    if( mrTextParagraphProperties.getParaAdjust() )
        rPropertyMap[ PROP_ParaAdjust ] <<= mrTextParagraphProperties.getParaAdjust().get();
}

// --------------------------------------------------------------------

ContextHandlerRef TextParagraphPropertiesContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
        case A_TOKEN( lnSpc ):          // CT_TextSpacing
            return new TextSpacingContext( *this, maLineSpacing );
        case A_TOKEN( spcBef ):         // CT_TextSpacing
            return new TextSpacingContext( *this, mrSpaceBefore );
        case A_TOKEN( spcAft ):         // CT_TextSpacing
            return new TextSpacingContext( *this, mrSpaceAfter );
        // EG_TextBulletColor
        case A_TOKEN( buClrTx ):        // CT_TextBulletColorFollowText ???
            mrBulletList.mbBulletColorFollowText <<= sal_True;
            break;
        case A_TOKEN( buClr ):          // CT_Color
            return new ColorContext( *this, *mrBulletList.maBulletColorPtr );
        // EG_TextBulletSize
        case A_TOKEN( buSzTx ):         // CT_TextBulletSizeFollowText
            mrBulletList.setBulletSize(100);
            break;
        case A_TOKEN( buSzPct ):        // CT_TextBulletSizePercent
            mrBulletList.setBulletSize( static_cast<sal_Int16>( GetPercent( rAttribs.getString( XML_val ).get() ) / 1000 ) );
            break;
        case A_TOKEN( buSzPts ):        // CT_TextBulletSizePoint
            mrBulletList.setBulletSize(0);
            mrBulletList.setFontSize( static_cast<sal_Int16>(GetTextSize( rAttribs.getString( XML_val ).get() ) ) );
            break;

        // EG_TextBulletTypeface
        case A_TOKEN( buFontTx ):       // CT_TextBulletTypefaceFollowText
            mrBulletList.mbBulletFontFollowText <<= sal_True;
            break;
        case A_TOKEN( buFont ):         // CT_TextFont
            mrBulletList.maBulletFont.setAttributes( rAttribs );
            break;

        // EG_TextBullet
        case A_TOKEN( buNone ):         // CT_TextNoBullet
            mrBulletList.setNone();
            break;
        case A_TOKEN( buAutoNum ):      // CT_TextAutonumberBullet
        {
            try {
                sal_Int32 nType = rAttribs.getToken( XML_type, 0 );
                sal_Int32 nStartAt = rAttribs.getInteger( XML_startAt, 1 );
                if( nStartAt > 32767 )
                {
                    nStartAt = 32767;
                }
                else if( nStartAt < 1 )
                {
                    nStartAt = 1;
                }
                mrBulletList.setStartAt( nStartAt );
                mrBulletList.setType( nType );
            }
            catch(SAXException& /* e */ )
            {
                OSL_TRACE("OOX: SAXException in XML_buAutoNum");
            }
            break;
        }
        case A_TOKEN( buChar ):         // CT_TextCharBullet
            try {
                mrBulletList.setBulletChar( rAttribs.getString( XML_char ).get() );
            }
            catch(SAXException& /* e */)
            {
                OSL_TRACE("OOX: SAXException in XML_buChar");
            }
            break;
        case A_TOKEN( buBlip ):         // CT_TextBlipBullet
            {
                mxBlipProps.reset( new BlipFillProperties );
                return new BlipFillContext( *this, rAttribs, *mxBlipProps );
            }
        case A_TOKEN( tabLst ):         // CT_TextTabStopList
            return new TextTabStopListContext( *this, maTabList );
        case A_TOKEN( defRPr ):         // CT_TextCharacterProperties
            return new TextCharacterPropertiesContext( *this, rAttribs, mrTextParagraphProperties.getTextCharacterProperties() );
        case OOX_TOKEN( doc, jc ):
            {
                OptValue< OUString > oParaAdjust = rAttribs.getString( OOX_TOKEN(doc, val) );
                if( oParaAdjust.has() && !oParaAdjust.get().isEmpty() )
                {
                    const OUString& sParaAdjust = oParaAdjust.get();
                    if( sParaAdjust == "left" )
                        mrTextParagraphProperties.setParaAdjust(ParagraphAdjust_LEFT);
                    else if ( sParaAdjust == "right" )
                        mrTextParagraphProperties.setParaAdjust(ParagraphAdjust_RIGHT);
                    else if ( sParaAdjust == "center" )
                        mrTextParagraphProperties.setParaAdjust(ParagraphAdjust_CENTER);
                    else if ( sParaAdjust == "both" )
                        mrTextParagraphProperties.setParaAdjust(ParagraphAdjust_BLOCK);
                }
            }
            break;
        case OOX_TOKEN( doc, spacing ):
            {
                // Spacing before
                if( !rAttribs.getBool(OOX_TOKEN(doc, beforeAutospacing), false) )
                {
                    OptValue<sal_Int32> oBefore = rAttribs.getInteger(OOX_TOKEN(doc, before));
                    if (oBefore.has())
                    {
                        TextSpacing& rSpacing = mrTextParagraphProperties.getParaTopMargin();
                        rSpacing.nUnit = TextSpacing::POINTS;
                        rSpacing.nValue = TWIPS_TO_MM(oBefore.get());
                        rSpacing.bHasValue = true;
                    }
                    else
                    {
                        OptValue<sal_Int32> oBeforeLines = rAttribs.getInteger(OOX_TOKEN(doc, beforeLines));
                        if (oBeforeLines.has())
                        {
                            TextSpacing& rSpacing = mrTextParagraphProperties.getParaTopMargin();
                            rSpacing.nUnit = TextSpacing::PERCENT;
                            rSpacing.nValue = oBeforeLines.get() * MAX_PERCENT / 100;
                            rSpacing.bHasValue = true;
                        }
                    }
                }

                // Spacing after
                if( !rAttribs.getBool(OOX_TOKEN(doc, afterAutospacing), false) )
                {
                    OptValue<sal_Int32> oAfter = rAttribs.getInteger(OOX_TOKEN(doc, after));
                    if (oAfter.has())
                    {
                        TextSpacing& rSpacing = mrTextParagraphProperties.getParaBottomMargin();
                        rSpacing.nUnit = TextSpacing::POINTS;
                        rSpacing.nValue = TWIPS_TO_MM(oAfter.get());
                        rSpacing.bHasValue = true;
                    }
                    else
                    {
                        OptValue<sal_Int32> oAfterLines = rAttribs.getInteger(OOX_TOKEN(doc, afterLines));
                        if (oAfterLines.has())
                        {
                            TextSpacing& rSpacing = mrTextParagraphProperties.getParaBottomMargin();
                            rSpacing.nUnit = TextSpacing::PERCENT;
                            rSpacing.nValue = oAfterLines.get() * MAX_PERCENT / 100;
                            rSpacing.bHasValue = true;
                        }
                    }
                }

                // Line spacing
                OptValue<OUString> oLineRule = rAttribs.getString(OOX_TOKEN(doc, lineRule));
                OptValue<sal_Int32> oLineSpacing = rAttribs.getInteger(OOX_TOKEN(doc, line));
                if (oLineSpacing.has())
                {
                    if( !oLineRule.has() || oLineRule.get() == "auto" )
                    {
                        maLineSpacing.nUnit = TextSpacing::PERCENT;
                        maLineSpacing.nValue = oLineSpacing.get() * MAX_PERCENT / 240;
                    }
                    else
                    {
                        maLineSpacing.nUnit = TextSpacing::POINTS;
                        maLineSpacing.nValue = TWIPS_TO_MM(oLineSpacing.get());
                    }
                    maLineSpacing.bHasValue = true;
                }
            }
            break;
        default:
            SAL_WARN("oox", "TextParagraphPropertiesContext::onCreateContext: unhandled element: " << getBaseToken(aElementToken));
    }
    return this;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
