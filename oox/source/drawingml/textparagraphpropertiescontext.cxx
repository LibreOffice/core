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

#include <drawingml/textparagraphpropertiescontext.hxx>

#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <svx/unopage.hxx>
#include <sal/log.hxx>
#include <tools/diagnose_ex.h>

#include <drawingml/colorchoicecontext.hxx>
#include <drawingml/misccontexts.hxx>
#include <drawingml/textcharacterpropertiescontext.hxx>
#include <drawingml/fillproperties.hxx>
#include <oox/helper/attributelist.hxx>
#include "textspacingcontext.hxx"
#include "texttabstoplistcontext.hxx"
#include <oox/token/namespaces.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <tools/helpers.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::graphic;

namespace oox::drawingml {
namespace {

double  lclGetGraphicAspectRatio( const Reference< XGraphic >& rxGraphic )
{
    double fRatio = 1.0;
    Reference< com::sun::star::beans::XPropertySet > xGraphicPropertySet( rxGraphic, UNO_QUERY_THROW );
    css::awt::Size aSizeHmm( 0, 0 );
    xGraphicPropertySet->getPropertyValue( "Size100thMM" ) >>= aSizeHmm;

    if( aSizeHmm.Width > 0 && aSizeHmm.Height > 0)
        return double(aSizeHmm.Width)/double(aSizeHmm.Height);
    else
    {
        css::awt::Size aSourceSizePixel( 0, 0 );
        xGraphicPropertySet->getPropertyValue( "SizePixel" ) >>= aSourceSizePixel;

        if( aSourceSizePixel.Width > 0 && aSourceSizePixel.Height > 0 )
            return double(aSourceSizePixel.Width)/double(aSourceSizePixel.Height);
    }
    return fRatio;
}

} //namespace

// CT_TextParagraphProperties
TextParagraphPropertiesContext::TextParagraphPropertiesContext( ContextHandler2Helper const & rParent,
                                                                const AttributeList& rAttribs,
                                                                TextParagraphProperties& rTextParagraphProperties )
: ContextHandler2( rParent )
, mrTextParagraphProperties( rTextParagraphProperties )
, mrBulletList( rTextParagraphProperties.getBulletList() )
{
    OUString sValue;

    PropertyMap& rPropertyMap( mrTextParagraphProperties.getTextParagraphPropertyMap() );

    // ST_TextAlignType
    if ( rAttribs.hasAttribute( XML_algn ) )
    {
        mrTextParagraphProperties.getParaAdjust() = GetParaAdjust( rAttribs.getToken( XML_algn, XML_l ) );
    }
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
        rPropertyMap.setProperty( PROP_ParaIsHyphenation, bLatinLineBrk);
    }
    // TODO see what to do with Asian hyphenation

    // ST_TextFontAlignType
    // TODO
//  sal_Int32 nFontAlign = rAttribs.getToken( XML_fontAlgn, XML_base );

    if ( rAttribs.hasAttribute( XML_hangingPunct ) )
    {
        bool bHangingPunct = rAttribs.getBool( XML_hangingPunct, false );
        rPropertyMap.setProperty( PROP_ParaIsHangingPunctuation, bHangingPunct);
    }

  // ST_Coordinate
    if ( rAttribs.hasAttribute( XML_indent ) )
    {
        sValue = rAttribs.getString( XML_indent ).get();
        mrTextParagraphProperties.getFirstLineIndentation() = std::optional< sal_Int32 >( sValue.isEmpty() ? 0 : GetCoordinate( sValue ) );
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
        mrTextParagraphProperties.getParaLeftMargin() = std::optional< sal_Int32 >( sValue.isEmpty() ? 0 : GetCoordinate( sValue ) );
    }

    // ParaRightMargin
    if ( rAttribs.hasAttribute( XML_marR ) )
    {
        sValue = rAttribs.getString( XML_marR ).get();
        sal_Int32 nMarR  = sValue.isEmpty() ? 0 : GetCoordinate( sValue ) ;
        rPropertyMap.setProperty( PROP_ParaRightMargin, nMarR);
    }

    if ( rAttribs.hasAttribute( XML_rtl ) )
    {
        bool bRtl = rAttribs.getBool( XML_rtl, false );
        rPropertyMap.setProperty( PROP_WritingMode, ( bRtl ? WritingMode2::RL_TB : WritingMode2::LR_TB ));
    }
}

TextParagraphPropertiesContext::~TextParagraphPropertiesContext()
{
    PropertyMap& rPropertyMap( mrTextParagraphProperties.getTextParagraphPropertyMap() );
    if ( mrTextParagraphProperties.getLineSpacing().bHasValue )
        rPropertyMap.setProperty( PROP_ParaLineSpacing, mrTextParagraphProperties.getLineSpacing().toLineSpacing());
    else
        rPropertyMap.setProperty( PROP_ParaLineSpacing, css::style::LineSpacing( css::style::LineSpacingMode::PROP, 100 ));

    ::std::vector< TabStop >::size_type nTabCount = maTabList.size();
    if( nTabCount != 0 )
    {
        Sequence< TabStop > aSeq( nTabCount );
        TabStop * aArray = aSeq.getArray();
        OSL_ENSURE( aArray != nullptr, "sequence array is NULL" );
        ::std::copy( maTabList.begin(), maTabList.end(), aArray );
        rPropertyMap.setProperty( PROP_ParaTabStops, aSeq);
    }

    if (mxBlipProps && mxBlipProps->mxFillGraphic.is())
    {
        mrBulletList.setGraphic( mxBlipProps->mxFillGraphic );
        mrBulletList.setBulletAspectRatio( lclGetGraphicAspectRatio(mxBlipProps->mxFillGraphic) );
    }

    if( mrBulletList.is() )
        rPropertyMap.setProperty( PROP_IsNumbering, true);
    sal_Int16 nLevel = mrTextParagraphProperties.getLevel();
    rPropertyMap.setProperty( PROP_NumberingLevel, nLevel);
    rPropertyMap.setProperty( PROP_NumberingIsNumber, true);

    if( mrTextParagraphProperties.getParaAdjust() )
        rPropertyMap.setProperty( PROP_ParaAdjust, *mrTextParagraphProperties.getParaAdjust());
}

ContextHandlerRef TextParagraphPropertiesContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
        case A_TOKEN( lnSpc ):          // CT_TextSpacing
            return new TextSpacingContext( *this, mrTextParagraphProperties.getLineSpacing() );
        case A_TOKEN( spcBef ):         // CT_TextSpacing
            return new TextSpacingContext( *this, mrTextParagraphProperties.getParaTopMargin() );
        case A_TOKEN( spcAft ):         // CT_TextSpacing
            return new TextSpacingContext( *this, mrTextParagraphProperties.getParaBottomMargin() );
        // EG_TextBulletColor
        case A_TOKEN( buClrTx ):        // CT_TextBulletColorFollowText ???
            mrBulletList.mbBulletColorFollowText <<= true;
            break;
        case A_TOKEN( buClr ):          // CT_Color
            return new ColorContext( *this, *mrBulletList.maBulletColorPtr );
        // EG_TextBulletSize
        case A_TOKEN( buSzTx ):         // CT_TextBulletSizeFollowText
            mrBulletList.setBulletSize(100);
            break;
        case A_TOKEN( buSzPct ):        // CT_TextBulletSizePercent
            mrBulletList.setBulletSize( std::lround( GetPercent( rAttribs.getString( XML_val ).get() ) / 1000.f ) );
            break;
        case A_TOKEN( buSzPts ):        // CT_TextBulletSizePoint
            mrBulletList.setBulletSize(0);
            mrBulletList.setFontSize( static_cast<sal_Int16>(GetTextSize( rAttribs.getString( XML_val ).get() ) ) );
            break;

        // EG_TextBulletTypeface
        case A_TOKEN( buFontTx ):       // CT_TextBulletTypefaceFollowText
            mrBulletList.mbBulletFontFollowText <<= true;
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
                TOOLS_WARN_EXCEPTION("oox", "OOX: SAXException in XML_buAutoNum");
            }
            break;
        }
        case A_TOKEN( buChar ):         // CT_TextCharBullet
            try {

                mrBulletList.setBulletChar( rAttribs.getString( XML_char ).get() );
                mrBulletList.setSuffixNone();
            }
            catch(SAXException& /* e */)
            {
                TOOLS_WARN_EXCEPTION("oox", "OOX: SAXException in XML_buChar");
            }
            break;
        case A_TOKEN( buBlip ):         // CT_TextBlipBullet
            {
                mxBlipProps = std::make_shared<BlipFillProperties>();
                return new BlipFillContext( *this, rAttribs, *mxBlipProps );
            }
        case A_TOKEN( tabLst ):         // CT_TextTabStopList
            return new TextTabStopListContext( *this, maTabList );
        case A_TOKEN( defRPr ):         // CT_TextCharacterProperties
            return new TextCharacterPropertiesContext( *this, rAttribs, mrTextParagraphProperties.getTextCharacterProperties() );
        case W_TOKEN( jc ):
            {
                OptValue< OUString > oParaAdjust = rAttribs.getString( W_TOKEN(val) );
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
        case W_TOKEN( spacing ):
            {
                // Spacing before
                if( !rAttribs.getBool(W_TOKEN(beforeAutospacing), false) )
                {
                    OptValue<sal_Int32> oBefore = rAttribs.getInteger(W_TOKEN(before));
                    if (oBefore.has())
                    {
                        TextSpacing& rSpacing = mrTextParagraphProperties.getParaTopMargin();
                        rSpacing.nUnit = TextSpacing::Unit::Points;
                        rSpacing.nValue = TwipsToHMM(oBefore.get());
                        rSpacing.bHasValue = true;
                    }
                    else
                    {
                        OptValue<sal_Int32> oBeforeLines = rAttribs.getInteger(W_TOKEN(beforeLines));
                        if (oBeforeLines.has())
                        {
                            TextSpacing& rSpacing = mrTextParagraphProperties.getParaTopMargin();
                            rSpacing.nUnit = TextSpacing::Unit::Percent;
                            rSpacing.nValue = oBeforeLines.get() * MAX_PERCENT / 100;
                            rSpacing.bHasValue = true;
                        }
                    }
                }

                // Spacing after
                if( !rAttribs.getBool(W_TOKEN(afterAutospacing), false) )
                {
                    OptValue<sal_Int32> oAfter = rAttribs.getInteger(W_TOKEN(after));
                    if (oAfter.has())
                    {
                        TextSpacing& rSpacing = mrTextParagraphProperties.getParaBottomMargin();
                        rSpacing.nUnit = TextSpacing::Unit::Points;
                        rSpacing.nValue = TwipsToHMM(oAfter.get());
                        rSpacing.bHasValue = true;
                    }
                    else
                    {
                        OptValue<sal_Int32> oAfterLines = rAttribs.getInteger(W_TOKEN(afterLines));
                        if (oAfterLines.has())
                        {
                            TextSpacing& rSpacing = mrTextParagraphProperties.getParaBottomMargin();
                            rSpacing.nUnit = TextSpacing::Unit::Percent;
                            rSpacing.nValue = oAfterLines.get() * MAX_PERCENT / 100;
                            rSpacing.bHasValue = true;
                        }
                    }
                }

                // Line spacing
                OptValue<OUString> oLineRule = rAttribs.getString(W_TOKEN(lineRule));
                OptValue<sal_Int32> oLineSpacing = rAttribs.getInteger(W_TOKEN(line));
                if (oLineSpacing.has())
                {
                    TextSpacing& rLineSpacing = mrTextParagraphProperties.getLineSpacing();
                    if( !oLineRule.has() || oLineRule.get() == "auto" )
                    {
                        rLineSpacing.nUnit = TextSpacing::Unit::Percent;
                        rLineSpacing.nValue = oLineSpacing.get() * MAX_PERCENT / 240;
                    }
                    else
                    {
                        rLineSpacing.nUnit = TextSpacing::Unit::Points;
                        rLineSpacing.nValue = TwipsToHMM(oLineSpacing.get());
                    }
                    rLineSpacing.bHasValue = true;
                }
            }
            break;
        default:
            SAL_WARN("oox", "TextParagraphPropertiesContext::onCreateContext: unhandled element: " << getBaseToken(aElementToken));
    }
    return this;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
