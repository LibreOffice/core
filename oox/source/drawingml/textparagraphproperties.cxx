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

#include "oox/drawingml/textparagraphproperties.hxx"

#include <com/sun/star/text/XNumberingRulesSupplier.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include "oox/helper/helper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "properties.hxx"
#include "tokens.hxx"

using rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using ::com::sun::star::awt::FontDescriptor;

namespace oox { namespace drawingml {

BulletList::BulletList( )
: maBulletColorPtr( new Color() )
{
}

bool BulletList::is() const
{
    return mnNumberingType.hasValue();
}

void BulletList::setBulletChar( const ::rtl::OUString & sChar )
{
    mnNumberingType <<= NumberingType::CHAR_SPECIAL;
    msBulletChar <<= sChar;
}

void BulletList::setGraphic( ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& rXGraphic )
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
    msNumberingSuffix <<= CREATE_OUSTRING( ")" );
    msNumberingPrefix <<= CREATE_OUSTRING( "(" );
}

void BulletList::setSuffixParenRight()
{
    msNumberingSuffix <<= CREATE_OUSTRING( ")" );
    msNumberingPrefix <<= OUString();
}

void BulletList::setSuffixPeriod()
{
    msNumberingSuffix <<= CREATE_OUSTRING( "." );
    msNumberingPrefix <<= OUString();
}

void BulletList::setSuffixNone()
{
    msNumberingSuffix <<= OUString();
    msNumberingPrefix <<= OUString();
}

void BulletList::setSuffixMinusRight()
{
    msNumberingSuffix <<= CREATE_OUSTRING( "-" );
    msNumberingPrefix <<= OUString();
}

void BulletList::setType( sal_Int32 nType )
{
//	OSL_TRACE( "OOX: set list numbering type %d", nType);
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
    if ( rSource.mnFontSize.hasValue() )
        mnFontSize = rSource.mnFontSize;
    if ( rSource.maStyleName.hasValue() )
        maStyleName = rSource.maStyleName;
    if ( rSource.maGraphic.hasValue() )
        maGraphic = rSource.maGraphic;
}

void BulletList::pushToPropMap( const ::oox::core::XmlFilterBase& rFilterBase, PropertyMap& rPropMap ) const
{
    if( msNumberingPrefix.hasValue() )
        rPropMap[ PROP_Prefix ] = msNumberingPrefix;
    if( msNumberingSuffix.hasValue() )
        rPropMap[ PROP_Suffix ] = msNumberingSuffix;
    if( mnStartAt.hasValue() )
        rPropMap[ PROP_StartWith ] = mnStartAt;
    rPropMap[ PROP_Adjust ] <<= HoriOrientation::LEFT;

    if( mnNumberingType.hasValue() )
        rPropMap[ PROP_NumberingType ] = mnNumberingType;

    OUString aBulletFontName;
    sal_Int16 nBulletFontPitch = 0;
    sal_Int16 nBulletFontFamily = 0;
    if( maBulletFont.getFontData( aBulletFontName, nBulletFontPitch, nBulletFontFamily, rFilterBase ) )
    {
        FontDescriptor aFontDesc;
        sal_Int16 nFontSize = 0;
        if( mnFontSize >>= nFontSize )
            aFontDesc.Height = nFontSize;

        // TODO move the to the TextFont struct.
        aFontDesc.Name = aBulletFontName;
        aFontDesc.Pitch = nBulletFontPitch;
        aFontDesc.Family = nBulletFontFamily;
        rPropMap[ PROP_BulletFont ] <<= aFontDesc;
        rPropMap[ PROP_BulletFontName ] <<= aBulletFontName;
    }
    if ( msBulletChar.hasValue() )
        rPropMap[ PROP_BulletChar ] = msBulletChar;
    if ( maGraphic.hasValue() )
    {
        Reference< com::sun::star::awt::XBitmap > xBitmap( maGraphic, UNO_QUERY );
        if ( xBitmap.is() )
            rPropMap[ PROP_Graphic ] <<= xBitmap;
    }
    if( mnSize.hasValue() )
        rPropMap[ PROP_BulletRelSize ] = mnSize;
    if ( maStyleName.hasValue() )
        rPropMap[ PROP_CharStyleName ] <<= maStyleName;
    if ( maBulletColorPtr->isUsed() )
        rPropMap[ PROP_BulletColor ] <<= maBulletColorPtr->getColor( rFilterBase.getGraphicHelper() );
}

TextParagraphProperties::TextParagraphProperties()
: mnLevel( 0 )
{
}

TextParagraphProperties::~TextParagraphProperties()
{
}

void TextParagraphProperties::apply( const TextParagraphProperties& rSourceProps )
{
    maTextParagraphPropertyMap.insert( rSourceProps.maTextParagraphPropertyMap.begin(), rSourceProps.maTextParagraphPropertyMap.end() );
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
}

void TextParagraphProperties::pushToPropSet( const ::oox::core::XmlFilterBase& rFilterBase,
    const Reference < XPropertySet >& xPropSet, PropertyMap& rioBulletMap, const BulletList* pMasterBuList, sal_Bool bApplyBulletMap, float fCharacterSize ) const
{
    PropertySet aPropSet( xPropSet );
    aPropSet.setProperties( maTextParagraphPropertyMap );

    sal_Int32 nNumberingType = NumberingType::NUMBER_NONE;
    if ( maBulletList.mnNumberingType.hasValue() )
        maBulletList.mnNumberingType >>= nNumberingType;
    else if ( pMasterBuList && pMasterBuList->mnNumberingType.hasValue() )
        pMasterBuList->mnNumberingType >>= nNumberingType;
    if ( nNumberingType == NumberingType::NUMBER_NONE )
        aPropSet.setProperty< sal_Int16 >( PROP_NumberingLevel, -1 );

    maBulletList.pushToPropMap( rFilterBase, rioBulletMap );

    if ( maParaTopMargin.bHasValue )
        aPropSet.setProperty( PROP_ParaTopMargin, maParaTopMargin.toMargin( getCharHeightPoints( 18.0 ) ) );
    if ( maParaBottomMargin.bHasValue )
        aPropSet.setProperty( PROP_ParaBottomMargin, maParaBottomMargin.toMargin( getCharHeightPoints( 18.0 ) ) );
    if ( nNumberingType == NumberingType::BITMAP )
    {
        fCharacterSize = getCharHeightPoints( fCharacterSize );

        com::sun::star::awt::Size aBulletSize;
        aBulletSize.Width = aBulletSize.Height = static_cast< sal_Int32 >( ( fCharacterSize * ( 2540.0 / 72.0 ) * 0.8 ) );
        rioBulletMap[ PROP_GraphicSize ] <<= aBulletSize;
    }

    boost::optional< sal_Int32 > noParaLeftMargin( moParaLeftMargin );
    boost::optional< sal_Int32 > noFirstLineIndentation( moFirstLineIndentation );

    if ( nNumberingType != NumberingType::NUMBER_NONE )
    {
        if ( noParaLeftMargin )
        {
            rioBulletMap[ PROP_LeftMargin ] <<= static_cast< sal_Int32 >( *noParaLeftMargin );
            noParaLeftMargin = boost::optional< sal_Int32 >( 0 );
        }
        if ( noFirstLineIndentation )
        {
            rioBulletMap[ PROP_FirstLineOffset ] <<= static_cast< sal_Int32 >( *noFirstLineIndentation );
            noFirstLineIndentation = boost::optional< sal_Int32 >( 0 );
        }
    }

    if ( bApplyBulletMap )
    {
        Reference< XIndexReplace > xNumRule;
        aPropSet.getProperty( xNumRule, PROP_NumberingRules );
        OSL_ENSURE( xNumRule.is(), "can't get Numbering rules");

        if( xNumRule.is() )
        {
            if( !rioBulletMap.empty() )
            {
                Sequence< PropertyValue > aBulletPropSeq = rioBulletMap.makePropertyValueSequence();
                xNumRule->replaceByIndex( getLevel(), makeAny( aBulletPropSeq ) );
            }

            aPropSet.setProperty( PROP_NumberingRules, xNumRule );
        }
    }
    if ( noParaLeftMargin )
        aPropSet.setProperty( PROP_ParaLeftMargin, *noParaLeftMargin );
    if ( noFirstLineIndentation )
        aPropSet.setProperty( PROP_ParaFirstLineIndent, *noFirstLineIndentation );
}

float TextParagraphProperties::getCharHeightPoints( float fDefault ) const
{
    return maTextCharacterProperties.getCharHeightPoints( fDefault );
}

} }
