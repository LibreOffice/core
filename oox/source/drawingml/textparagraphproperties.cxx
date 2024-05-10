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

#include <drawingml/textparagraphproperties.hxx>

#include <com/sun/star/text/XNumberingRulesSupplier.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>

#include <osl/diagnose.h>

#include <oox/helper/propertyset.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>

#if OSL_DEBUG_LEVEL > 0
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <oox/ppt/pptimport.hxx>
#include <oox/ppt/slidepersist.hxx>
#endif

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using ::com::sun::star::awt::FontDescriptor;

namespace oox::drawingml {

BulletList::BulletList( )
: maBulletColorPtr( std::make_shared<Color>() ),
  mbBulletColorFollowText ( false ),
  mbBulletFontFollowText ( false ),
  mbBulletSizeFollowText ( false )
{
}

bool BulletList::is() const
{
    return mnNumberingType.hasValue();
}

void BulletList::setBulletChar( const OUString & sChar )
{
    mnNumberingType <<= NumberingType::CHAR_SPECIAL;
    msBulletChar <<= sChar;
}

void BulletList::setGraphic( css::uno::Reference< css::graphic::XGraphic > const & rXGraphic )
{
    mnNumberingType <<= NumberingType::BITMAP;
    maGraphic <<= rXGraphic;
}

void BulletList::setNone( )
{
    mnNumberingType <<= NumberingType::NUMBER_NONE;
}

void BulletList::setSuffixParenBoth()
{
    msNumberingSuffix <<= u")"_ustr;
    msNumberingPrefix <<= u"("_ustr;
}

void BulletList::setSuffixParenRight()
{
    msNumberingSuffix <<= u")"_ustr;
    msNumberingPrefix <<= OUString();
}

void BulletList::setSuffixPeriod()
{
    msNumberingSuffix <<= u"."_ustr;
    msNumberingPrefix <<= OUString();
}

void BulletList::setSuffixNone()
{
    msNumberingSuffix <<= OUString();
    msNumberingPrefix <<= OUString();
}

void BulletList::setSuffixMinusRight()
{
    msNumberingSuffix <<= u"-"_ustr;
    msNumberingPrefix <<= OUString();
}

void BulletList::setType( sal_Int32 nType )
{
    OSL_ASSERT((nType & sal_Int32(0xFFFF0000))==0);
    switch( nType )
    {
    case XML_alphaLcParenBoth:
        mnNumberingType <<= NumberingType::CHARS_LOWER_LETTER;
        setSuffixParenBoth();
        break;
    case XML_alphaLcParenR:
        mnNumberingType <<= NumberingType::CHARS_LOWER_LETTER;
        setSuffixParenRight();
        break;
    case XML_alphaLcPeriod:
        mnNumberingType <<= NumberingType::CHARS_LOWER_LETTER;
        setSuffixPeriod();
        break;
    case XML_alphaUcParenBoth:
        mnNumberingType <<= NumberingType::CHARS_UPPER_LETTER;
        setSuffixParenBoth();
        break;
    case XML_alphaUcParenR:
        mnNumberingType <<= NumberingType::CHARS_UPPER_LETTER;
        setSuffixParenRight();
        break;
    case XML_alphaUcPeriod:
        mnNumberingType <<= NumberingType::CHARS_UPPER_LETTER;
        setSuffixPeriod();
        break;
    case XML_arabic1Minus:
    case XML_arabic2Minus:
    case XML_arabicDbPeriod:
    case XML_arabicDbPlain:
        // TODO
        break;
    case XML_arabicParenBoth:
         mnNumberingType <<= NumberingType::ARABIC;
        setSuffixParenBoth();
        break;
    case XML_arabicParenR:
         mnNumberingType <<= NumberingType::ARABIC;
        setSuffixParenRight();
        break;
    case XML_arabicPeriod:
         mnNumberingType <<= NumberingType::ARABIC;
        setSuffixPeriod();
        break;
    case XML_arabicPlain:
         mnNumberingType <<= NumberingType::ARABIC;
        setSuffixNone();
        break;
    case XML_circleNumDbPlain:
    case XML_circleNumWdBlackPlain:
    case XML_circleNumWdWhitePlain:
        mnNumberingType <<= NumberingType::CIRCLE_NUMBER;
        break;
    case XML_ea1ChsPeriod:
        mnNumberingType <<= NumberingType::NUMBER_UPPER_ZH;
         setSuffixPeriod();
        break;
    case XML_ea1ChsPlain:
        mnNumberingType <<= NumberingType::NUMBER_UPPER_ZH;
        setSuffixNone();
        break;
    case XML_ea1ChtPeriod:
        mnNumberingType <<= NumberingType::NUMBER_UPPER_ZH_TW;
         setSuffixPeriod();
        break;
    case XML_ea1ChtPlain:
        mnNumberingType <<= NumberingType::NUMBER_UPPER_ZH_TW;
        setSuffixNone();
        break;
    case XML_ea1JpnChsDbPeriod:
    case XML_ea1JpnKorPeriod:
    case XML_ea1JpnKorPlain:
        break;
    case XML_hebrew2Minus:
        mnNumberingType <<= NumberingType::CHARS_HEBREW;
        setSuffixMinusRight();
        break;
    case XML_hindiAlpha1Period:
    case XML_hindiAlphaPeriod:
    case XML_hindiNumParenR:
    case XML_hindiNumPeriod:
        // TODO
        break;
    case XML_romanLcParenBoth:
        mnNumberingType <<= NumberingType::ROMAN_LOWER;
        setSuffixParenBoth();
        break;
    case XML_romanLcParenR:
        mnNumberingType <<= NumberingType::ROMAN_LOWER;
        setSuffixParenRight();
        break;
    case XML_romanLcPeriod:
        mnNumberingType <<= NumberingType::ROMAN_LOWER;
         setSuffixPeriod();
        break;
    case XML_romanUcParenBoth:
        mnNumberingType <<= NumberingType::ROMAN_UPPER;
        setSuffixParenBoth();
        break;
    case XML_romanUcParenR:
        mnNumberingType <<= NumberingType::ROMAN_UPPER;
        setSuffixParenRight();
        break;
    case XML_romanUcPeriod:
        mnNumberingType <<= NumberingType::ROMAN_UPPER;
         setSuffixPeriod();
        break;
    case XML_thaiAlphaParenBoth:
    case XML_thaiNumParenBoth:
        mnNumberingType <<= NumberingType::CHARS_THAI;
        setSuffixParenBoth();
        break;
    case XML_thaiAlphaParenR:
    case XML_thaiNumParenR:
        mnNumberingType <<= NumberingType::CHARS_THAI;
        setSuffixParenRight();
        break;
    case XML_thaiAlphaPeriod:
    case XML_thaiNumPeriod:
        mnNumberingType <<= NumberingType::CHARS_THAI;
         setSuffixPeriod();
        break;
    }
}

void BulletList::setBulletSize(sal_Int16 nSize)
{
    mnSize <<= nSize;
}

void BulletList::setBulletAspectRatio(double nAspectRatio)
{
    mnAspectRatio <<= nAspectRatio;
}

void BulletList::setFontSize(sal_Int16 nSize)
{
    mnFontSize <<= nSize;
}

void BulletList::apply( const BulletList& rSource )
{
    if ( rSource.maBulletColorPtr->isUsed() )
        maBulletColorPtr = rSource.maBulletColorPtr;
    if ( rSource.mbBulletColorFollowText.hasValue() )
        mbBulletColorFollowText = rSource.mbBulletColorFollowText;
    if ( rSource.mbBulletFontFollowText.hasValue() )
        mbBulletFontFollowText = rSource.mbBulletFontFollowText;
    if ( rSource.mbBulletSizeFollowText.hasValue() )
        mbBulletSizeFollowText = rSource.mbBulletSizeFollowText;
    maBulletFont.assignIfUsed( rSource.maBulletFont );
    if ( rSource.msBulletChar.hasValue() )
        msBulletChar = rSource.msBulletChar;
    if ( rSource.mnStartAt.hasValue() )
        mnStartAt = rSource.mnStartAt;
    if ( rSource.mnNumberingType.hasValue() )
        mnNumberingType = rSource.mnNumberingType;
    if ( rSource.msNumberingPrefix.hasValue() )
        msNumberingPrefix = rSource.msNumberingPrefix;
    if ( rSource.msNumberingSuffix.hasValue() )
        msNumberingSuffix = rSource.msNumberingSuffix;
    if ( rSource.mnSize.hasValue() )
        mnSize = rSource.mnSize;
    if ( rSource.mnAspectRatio.hasValue() )
        mnAspectRatio = rSource.mnAspectRatio;
    if ( rSource.mnFontSize.hasValue() )
        mnFontSize = rSource.mnFontSize;
    if ( rSource.maStyleName.hasValue() )
        maStyleName = rSource.maStyleName;
    if ( rSource.maGraphic.hasValue() )
        maGraphic = rSource.maGraphic;
}

void BulletList::pushToPropMap( const ::oox::core::XmlFilterBase* pFilterBase, PropertyMap& rPropMap ) const
{
    if( msNumberingPrefix.hasValue() )
        rPropMap.setAnyProperty( PROP_Prefix, msNumberingPrefix);
    if( msNumberingSuffix.hasValue() )
        rPropMap.setAnyProperty( PROP_Suffix, msNumberingSuffix);
    if( mnStartAt.hasValue() )
        rPropMap.setAnyProperty( PROP_StartWith, mnStartAt);
    rPropMap.setProperty( PROP_Adjust, HoriOrientation::LEFT);

    if( mnNumberingType.hasValue() )
        rPropMap.setAnyProperty( PROP_NumberingType, mnNumberingType);

    OUString aBulletFontName;
    sal_Int16 nBulletFontPitch = 0;
    sal_Int16 nBulletFontFamily = 0;
    float nBulletFontWeight = css::awt::FontWeight::NORMAL;
    bool bSymbolFont = false;
    if( pFilterBase) {
        bool bFollowTextFont = false;
        mbBulletFontFollowText >>= bFollowTextFont;
        if (!bFollowTextFont && maBulletFont.getFontData( aBulletFontName, nBulletFontPitch, nBulletFontFamily, &bSymbolFont, *pFilterBase ) )
        {
            FontDescriptor aFontDesc;
            sal_Int16 nFontSize = 0;
            if( mnFontSize >>= nFontSize )
                aFontDesc.Height = nFontSize;

            // TODO It is likely that bSymbolFont from getFontData is sufficient to check here
            // and looking at the font name is not necessary, if it is necessary then moving
            // the name lookup into getFontData is likely the best fix
            aFontDesc.Name = aBulletFontName;
            aFontDesc.Pitch = nBulletFontPitch;
            aFontDesc.Family = nBulletFontFamily;
            aFontDesc.Weight = nBulletFontWeight;
            if ( bSymbolFont ||
                 aBulletFontName.equalsIgnoreAsciiCase("Wingdings") ||
                 aBulletFontName.equalsIgnoreAsciiCase("Wingdings 2") ||
                 aBulletFontName.equalsIgnoreAsciiCase("Wingdings 3") ||
                 aBulletFontName.equalsIgnoreAsciiCase("Monotype Sorts") ||
                 aBulletFontName.equalsIgnoreAsciiCase("Monotype Sorts 2") ||
                 aBulletFontName.equalsIgnoreAsciiCase("Webdings") ||
                 aBulletFontName.equalsIgnoreAsciiCase("StarBats") ||
                 aBulletFontName.equalsIgnoreAsciiCase("StarMath") ||
                 aBulletFontName.equalsIgnoreAsciiCase("ZapfDingbats") ) {
                aFontDesc.CharSet = RTL_TEXTENCODING_SYMBOL;
                bSymbolFont = true;
            }
            rPropMap.setProperty( PROP_BulletFont, aFontDesc);
            rPropMap.setProperty( PROP_BulletFontName, aBulletFontName);
        }
    }
    if ( msBulletChar.hasValue() ) {
        OUString sBuChar;

        msBulletChar >>= sBuChar;

        if( pFilterBase && sBuChar.getLength() == 1 && maBulletFont.getFontData( aBulletFontName, nBulletFontPitch, nBulletFontFamily, nullptr, *pFilterBase ) && bSymbolFont )
        {
            sal_Unicode nBuChar = sBuChar.toChar();
            nBuChar &= 0x00ff;
            nBuChar |= 0xf000;
            sBuChar = OUString( &nBuChar, 1 );
        }

        rPropMap.setProperty( PROP_BulletChar, sBuChar);
    }
    if ( maGraphic.hasValue() )
    {
        Reference<css::awt::XBitmap> xBitmap(maGraphic, UNO_QUERY);
        if (xBitmap.is())
            rPropMap.setProperty(PROP_GraphicBitmap, xBitmap);
    }
    bool bFollowTextSize = false;
    mbBulletSizeFollowText >>= bFollowTextSize;
    if( !bFollowTextSize && mnSize.hasValue() )
        rPropMap.setAnyProperty( PROP_BulletRelSize, mnSize);
    if ( maStyleName.hasValue() )
        rPropMap.setAnyProperty( PROP_CharStyleName, maStyleName);
    if (pFilterBase ) {
        bool bFollowTextColor = false;
        mbBulletColorFollowText >>= bFollowTextColor;
        if ( maBulletColorPtr->isUsed() && !bFollowTextColor )
            rPropMap.setProperty( PROP_BulletColor, maBulletColorPtr->getColor( pFilterBase->getGraphicHelper() ));
    }
}

TextParagraphProperties::TextParagraphProperties()
: mnLevel( 0 )
{
}

void TextParagraphProperties::apply( const TextParagraphProperties& rSourceProps )
{
    maTextParagraphPropertyMap.assignAll( rSourceProps.maTextParagraphPropertyMap );
    maBulletList.apply( rSourceProps.maBulletList );
    maTextCharacterProperties.assignUsed( rSourceProps.maTextCharacterProperties );
    if ( rSourceProps.maParaTopMargin.bHasValue )
        maParaTopMargin = rSourceProps.maParaTopMargin;
    if ( rSourceProps.maParaBottomMargin.bHasValue )
        maParaBottomMargin = rSourceProps.maParaBottomMargin;
    if ( rSourceProps.moParaLeftMargin )
        moParaLeftMargin = rSourceProps.moParaLeftMargin;
    if ( rSourceProps.moFirstLineIndentation )
        moFirstLineIndentation = rSourceProps.moFirstLineIndentation;
    if ( rSourceProps.moDefaultTabSize )
        moDefaultTabSize = rSourceProps.moDefaultTabSize;
    if( rSourceProps.mnLevel )
        mnLevel = rSourceProps.mnLevel;
    if( rSourceProps.moParaAdjust )
        moParaAdjust = rSourceProps.moParaAdjust;
    if( rSourceProps.maLineSpacing.bHasValue )
        maLineSpacing = rSourceProps.maLineSpacing;
}

void TextParagraphProperties::pushToPropSet( const ::oox::core::XmlFilterBase* pFilterBase,
    const Reference < XPropertySet >& xPropSet, PropertyMap& rioBulletMap, const BulletList* pMasterBuList, bool bApplyBulletMap, float fCharacterSize,
    bool bPushDefaultValues ) const
{
    PropertySet aPropSet( xPropSet );
    aPropSet.setProperties( maTextParagraphPropertyMap );

    sal_Int32 nNumberingType = NumberingType::NUMBER_NONE;
    if ( maBulletList.mnNumberingType.hasValue() )
    {
        maBulletList.mnNumberingType >>= nNumberingType;
        aPropSet.setProperty< sal_Int16 >( PROP_NumberingLevel, getLevel() );
    }
    else if ( pMasterBuList && pMasterBuList->mnNumberingType.hasValue() )
        pMasterBuList->mnNumberingType >>= nNumberingType;
    if ( nNumberingType == NumberingType::NUMBER_NONE
        && aPropSet.hasProperty(PROP_NumberingLevel) )
        aPropSet.setProperty< sal_Int16 >( PROP_NumberingLevel, -1 );

    maBulletList.pushToPropMap( pFilterBase, rioBulletMap );

    if ( maParaTopMargin.bHasValue || bPushDefaultValues )
        aPropSet.setProperty( PROP_ParaTopMargin, maParaTopMargin.toMargin( fCharacterSize != 0.0 ? fCharacterSize : getCharHeightPoints ( 12.0 ) ) );
    if ( maParaBottomMargin.bHasValue || bPushDefaultValues )
        aPropSet.setProperty( PROP_ParaBottomMargin, maParaBottomMargin.toMargin( fCharacterSize != 0.0 ? fCharacterSize : getCharHeightPoints ( 12.0 ) ) );

    std::optional< sal_Int32 > noParaLeftMargin( moParaLeftMargin );
    std::optional< sal_Int32 > noFirstLineIndentation( moFirstLineIndentation );

    if ( nNumberingType != NumberingType::NUMBER_NONE )
    {
        if ( noParaLeftMargin )
        {
            aPropSet.setProperty<sal_Int32>( PROP_ParaLeftMargin, 0);
            rioBulletMap.setProperty( PROP_LeftMargin, *noParaLeftMargin);
            noParaLeftMargin.reset();
        }
        if ( noFirstLineIndentation )
        {
            // Force Paragraph property as zero - impress seems to use the value from previous
            // (non) bullet line if not set to zero explicitly :(
            aPropSet.setProperty<sal_Int32>( PROP_ParaFirstLineIndent, 0);
            rioBulletMap.setProperty( PROP_FirstLineOffset, *noFirstLineIndentation);
            noFirstLineIndentation.reset();
        }
        if ( nNumberingType != NumberingType::BITMAP && !rioBulletMap.hasProperty( PROP_BulletColor ) && pFilterBase )
            rioBulletMap.setProperty( PROP_BulletColor, maTextCharacterProperties.maFillProperties.getBestSolidColor().getColor( pFilterBase->getGraphicHelper()));
    }

    if ( bApplyBulletMap )
    {
        Reference< XIndexReplace > xNumRule;
        aPropSet.getProperty( xNumRule, PROP_NumberingRules );
        OSL_ENSURE( xNumRule.is(), "can't get Numbering rules");

        try
        {
            if( xNumRule.is() )
            {
                if( !rioBulletMap.empty() )
                {
                    // fix default bullet size to be 100%
                    if( !rioBulletMap.hasProperty( PROP_BulletRelSize ) )
                        rioBulletMap.setProperty<sal_Int16>( PROP_BulletRelSize, 100);
                    Sequence< PropertyValue > aBulletPropSeq = rioBulletMap.makePropertyValueSequence();
                    xNumRule->replaceByIndex( getLevel(), Any( aBulletPropSeq ) );
                }

                aPropSet.setProperty( PROP_NumberingRules, xNumRule );
            }
        }
        catch (const Exception &)
        {
            // Don't warn for now, expected to fail for Writer.
        }
    }
    if ( noParaLeftMargin )
        aPropSet.setProperty( PROP_ParaLeftMargin, *noParaLeftMargin);
    if ( noFirstLineIndentation )
    {
        aPropSet.setProperty( PROP_ParaFirstLineIndent, *noFirstLineIndentation );
        if( bPushDefaultValues )
        {
            // Reset TabStops - these would be auto calculated by Impress
            TabStop aTabStop;
            aTabStop.Position = 0;
            Sequence< TabStop > aSeq { aTabStop };
            aPropSet.setProperty( PROP_ParaTabStops, aSeq );
        }
    }
    else
        aPropSet.setProperty<sal_Int32>( PROP_ParaFirstLineIndent, 0);

    if ( moDefaultTabSize )
    {
        aPropSet.setProperty( PROP_ParaTabStopDefaultDistance, *moDefaultTabSize );
    }

    if ( moParaAdjust )
    {
        aPropSet.setProperty( PROP_ParaAdjust, *moParaAdjust);
    }
    else
    {
        aPropSet.setProperty( PROP_ParaAdjust, css::style::ParagraphAdjust_LEFT);
    }

    if ( maLineSpacing.bHasValue )
    {
        aPropSet.setProperty( PROP_ParaLineSpacing, maLineSpacing.toLineSpacing());
    }
    else
    {
        aPropSet.setProperty( PROP_ParaLineSpacing, css::style::LineSpacing( css::style::LineSpacingMode::PROP, 100 ));
    }
}

float TextParagraphProperties::getCharHeightPoints( float fDefault ) const
{
    return maTextCharacterProperties.getCharHeightPoints( fDefault );
}

#ifdef DBG_UTIL
// Note: Please don't remove this function. This is required for
// debugging pptx import problems.
void TextParagraphProperties::dump() const
{
    Reference< css::drawing::XShape > xShape( oox::ppt::PowerPointImport::mpDebugFilterBase->getModelFactory()->createInstance( u"com.sun.star.presentation.TitleTextShape"_ustr ), UNO_QUERY );
    Reference< css::text::XText > xText( xShape, UNO_QUERY );

    Reference< css::drawing::XDrawPage > xDebugPage(ppt::SlidePersist::mxDebugPage.get(), UNO_QUERY);
    if (xDebugPage.is())
        xDebugPage->add( xShape );

    PropertyMap emptyMap;

    xText->setString( u"debug"_ustr );
    Reference< css::text::XTextCursor > xStart = xText->createTextCursor();
    xStart->gotoEnd( true );
    Reference< XPropertySet > xPropSet( xStart, UNO_QUERY );
    pushToPropSet( nullptr, xPropSet, emptyMap, nullptr, false, 0 );
    PropertySet aPropSet( xPropSet );
    aPropSet.dump();
}
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
