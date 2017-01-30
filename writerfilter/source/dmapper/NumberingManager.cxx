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

#include <sal/config.h>

#include "ConversionHelper.hxx"
#include "NumberingManager.hxx"
#include "StyleSheetTable.hxx"
#include "PropertyIds.hxx"

#include <ooxml/resourceids.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/PositionAndSpaceMode.hpp>
#include <com/sun/star/text/XChapterNumberingSupplier.hpp>

#include <osl/diagnose.h>
#include <rtl/ustring.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/propertyvalue.hxx>

using namespace com::sun::star;

#define NUMBERING_MAX_LEVELS    10


namespace writerfilter {
namespace dmapper {

//---------------------------------------------------  Utility functions
template <typename T>
beans::PropertyValue lcl_makePropVal(PropertyIds nNameID, T const & aValue)
{
    return {getPropertyName(nNameID), 0, uno::makeAny(aValue), beans::PropertyState_DIRECT_VALUE};
}

sal_Int32 lcl_findProperty( const uno::Sequence< beans::PropertyValue >& aProps, const OUString& sName )
{
    sal_Int32 i = 0;
    sal_Int32 nLen = aProps.getLength( );
    sal_Int32 nPos = -1;

    while ( nPos == -1 && i < nLen )
    {
        if ( aProps[i].Name.equals( sName ) )
            nPos = i;
        else
            i++;
    }

    return nPos;
}

void lcl_mergeProperties( uno::Sequence< beans::PropertyValue >& aSrc,
        uno::Sequence< beans::PropertyValue >& aDst )
{
    for ( sal_Int32 i = 0, nSrcLen = aSrc.getLength( ); i < nSrcLen; i++ )
    {
        // Look for the same property in aDst
        sal_Int32 nPos = lcl_findProperty( aDst, aSrc[i].Name );
        if ( nPos >= 0 )
        {
            // Replace the property value by the one in aSrc
            aDst[nPos] = aSrc[i];
        }
        else
        {
            // Simply add the new value
            aDst.realloc( aDst.getLength( ) + 1 );
            aDst[ aDst.getLength( ) - 1 ] = aSrc[i];
        }
    }
}

//--------------------------------------------  ListLevel implementation
void ListLevel::SetValue( Id nId, sal_Int32 nValue )
{
    switch( nId )
    {
        case NS_ooxml::LN_CT_Lvl_start:
            m_nIStartAt = nValue;
        break;
        case NS_ooxml::LN_CT_Lvl_numFmt:
            m_nNFC = nValue;
        break;
        case NS_ooxml::LN_CT_Lvl_isLgl:
            m_nFLegal = nValue;
        break;
        case NS_ooxml::LN_CT_Lvl_legacy:
            m_nFPrevSpace = nValue;
        break;
        case NS_ooxml::LN_CT_Lvl_suff:
            m_nXChFollow = nValue;
        break;
        case NS_ooxml::LN_CT_TabStop_pos:
            if (nValue < 0)
            {
                SAL_INFO("writerfilter",
                        "unsupported list tab stop position " << nValue);
            }
            else
                m_nTabstop = nValue;
        break;
        default:
            OSL_FAIL( "this line should never be reached");
    }
}

void ListLevel::SetParaStyle( const std::shared_ptr< StyleSheetEntry >& pStyle )
{
    if (!pStyle)
        return;
    m_pParaStyle = pStyle;
    // AFAICT .docx spec does not identify which numberings or paragraph
    // styles are actually the ones to be used for outlines (chapter numbering),
    // it only kind of says somewhere that they should be named Heading1 to Heading9.
    const OUString styleId= pStyle->sConvertedStyleName;
    m_outline = ( styleId.getLength() == RTL_CONSTASCII_LENGTH( "Heading 1" )
        && styleId.match( "Heading ", 0 )
        && styleId[ RTL_CONSTASCII_LENGTH( "Heading " ) ] >= '1'
        && styleId[ RTL_CONSTASCII_LENGTH( "Heading " ) ] <= '9' );
}

sal_Int16 ListLevel::GetParentNumbering( const OUString& sText, sal_Int16 nLevel,
        OUString& rPrefix, OUString& rSuffix )
{
    sal_Int16 nParentNumbering = 1;

    //now parse the text to find %n from %1 to %nLevel+1
    //everything before the first % and the last %x is prefix and suffix
    OUString sLevelText( sText );
    sal_Int32 nCurrentIndex = 0;
    sal_Int32 nFound = sLevelText.indexOf( '%', nCurrentIndex );
    if( nFound > 0 )
    {
        rPrefix = sLevelText.copy( 0, nFound );
        sLevelText = sLevelText.copy( nFound );
    }
    sal_Int32 nMinLevel = nLevel;
    //now the text should either be empty or start with %
    nFound = sLevelText.getLength( ) > 1 ? 0 : -1;
    while( nFound >= 0 )
    {
        if( sLevelText.getLength() > 1 )
        {
            sal_Unicode cLevel = sLevelText[1];
            if( cLevel >= '1' && cLevel <= '9' )
            {
                if( cLevel - '1' < nMinLevel )
                    nMinLevel = cLevel - '1';
                //remove first char - next char is removed later
                sLevelText = sLevelText.copy( 1 );
            }
        }
        //remove old '%' or number
        sLevelText = sLevelText.copy( 1 );
        nCurrentIndex = 0;
        nFound = sLevelText.indexOf( '%', nCurrentIndex );
        //remove the text before the next %
        if(nFound > 0)
            sLevelText = sLevelText.copy( nFound -1 );
    }
    if( nMinLevel < nLevel )
    {
        nParentNumbering = sal_Int16( nLevel - nMinLevel + 1);
    }

    rSuffix = sLevelText;

    return nParentNumbering;
}

uno::Sequence< beans::PropertyValue > ListLevel::GetProperties( )
{
    uno::Sequence< beans::PropertyValue > aLevelProps = GetLevelProperties( );
    if ( m_pParaStyle.get( ) )
        AddParaProperties( &aLevelProps );
    return aLevelProps;
}

static bool IgnoreForCharStyle(const OUString& aStr)
{
    //Names found in PropertyIds.cxx, Lines 56-396
    return (aStr=="Adjust" || aStr=="IndentAt" || aStr=="FirstLineIndent"
            || aStr=="FirstLineOffset" || aStr=="LeftMargin" || aStr=="CharFontName"
        );
}
uno::Sequence< beans::PropertyValue > ListLevel::GetCharStyleProperties( )
{
    PropertyValueVector_t rProperties;

    uno::Sequence< beans::PropertyValue > vPropVals = PropertyMap::GetPropertyValues();
    beans::PropertyValue* aValIter = vPropVals.begin();
    beans::PropertyValue* aEndIter = vPropVals.end();
    for( ; aValIter != aEndIter; ++aValIter )
    {
        if (IgnoreForCharStyle(aValIter->Name))
            continue;
        else if(aValIter->Name=="CharInteropGrabBag" || aValIter->Name=="ParaInteropGrabBag") {
            uno::Sequence<beans::PropertyValue> vGrabVals;
            aValIter->Value >>= vGrabVals;
            beans::PropertyValue* aGrabIter = vGrabVals.begin();
            for(; aGrabIter!=vGrabVals.end(); ++aGrabIter) {
                if(!IgnoreForCharStyle(aGrabIter->Name))
                    rProperties.push_back(beans::PropertyValue(aGrabIter->Name, 0, aGrabIter->Value, beans::PropertyState_DIRECT_VALUE));
            }
        }
        else
            rProperties.push_back(beans::PropertyValue(aValIter->Name, 0, aValIter->Value, beans::PropertyState_DIRECT_VALUE));
    }

    return comphelper::containerToSequence(rProperties);
}

uno::Sequence< beans::PropertyValue > ListLevel::GetLevelProperties( )
{
    const sal_Int16 aWWToUnoAdjust[] =
    {
        text::HoriOrientation::LEFT,
        text::HoriOrientation::CENTER,
        text::HoriOrientation::RIGHT,
    };

    std::vector<beans::PropertyValue> aNumberingProperties;

    if( m_nIStartAt >= 0)
        aNumberingProperties.push_back(lcl_makePropVal<sal_Int16>(PROP_START_WITH, m_nIStartAt) );

    sal_Int16 nNumberFormat = ConversionHelper::ConvertNumberingType(m_nNFC);
    if( m_nNFC >= 0)
    {
        if (!m_sGraphicURL.isEmpty() || m_sGraphicBitmap.is())
            nNumberFormat = style::NumberingType::BITMAP;
        else if (m_sBulletChar.isEmpty() && nNumberFormat != style::NumberingType::CHAR_SPECIAL)
            // w:lvlText is empty, that means no numbering in Word.
            // CHAR_SPECIAL is handled separately below.
            nNumberFormat = style::NumberingType::NUMBER_NONE;
        aNumberingProperties.push_back(lcl_makePropVal(PROP_NUMBERING_TYPE, nNumberFormat));
    }

    if( m_nJC >= 0 && m_nJC <= sal::static_int_cast<sal_Int32>(sizeof(aWWToUnoAdjust) / sizeof(sal_Int16)) )
        aNumberingProperties.push_back(lcl_makePropVal(PROP_ADJUST, aWWToUnoAdjust[m_nJC]));

    if( !isOutlineNumbering())
    {
        // todo: this is not the bullet char
        if( nNumberFormat == style::NumberingType::CHAR_SPECIAL )
        {
            if (!m_sBulletChar.isEmpty())
            {
                aNumberingProperties.push_back(lcl_makePropVal(PROP_BULLET_CHAR, m_sBulletChar.copy(0, 1)));
            }
            else
            {
                // If w:lvlText's value is null - set bullet char to zero.
                aNumberingProperties.push_back(lcl_makePropVal<sal_Unicode>(PROP_BULLET_CHAR, 0));
            }
        }
        if (!m_sGraphicURL.isEmpty())
            aNumberingProperties.push_back(lcl_makePropVal(PROP_GRAPHIC_URL, m_sGraphicURL));
        if (m_sGraphicBitmap.is())
            aNumberingProperties.push_back(lcl_makePropVal(PROP_GRAPHIC_BITMAP, m_sGraphicBitmap));
    }

    aNumberingProperties.push_back(lcl_makePropVal(PROP_LISTTAB_STOP_POSITION, m_nTabstop));

    //TODO: handling of nFLegal?
    //TODO: nFNoRestart lower levels do not restart when higher levels are incremented, like:
    //1.
    //1.1
    //2.2
    //2.3
    //3.4


    if( m_nFWord6 > 0) //Word 6 compatibility
    {
        if( m_nFPrev == 1)
            aNumberingProperties.push_back(lcl_makePropVal<sal_Int16>(PROP_PARENT_NUMBERING, NUMBERING_MAX_LEVELS));
        //TODO: prefixing space     nFPrevSpace;     - has not been used in WW8 filter
    }

//    TODO: sRGBXchNums;     array of inherited numbers

//  nXChFollow; following character 0 - tab, 1 - space, 2 - nothing
    aNumberingProperties.push_back(lcl_makePropVal(PROP_LEVEL_FOLLOW, m_nXChFollow));

    const int nIds = 5;
    PropertyIds aReadIds[nIds] =
    {
        PROP_ADJUST, PROP_INDENT_AT, PROP_FIRST_LINE_INDENT,
            PROP_FIRST_LINE_OFFSET, PROP_LEFT_MARGIN
    };
    for(PropertyIds & rReadId : aReadIds) {
        boost::optional<PropertyMap::Property> aProp = getProperty(rReadId);
        if (aProp)
            aNumberingProperties.push_back(
                    beans::PropertyValue( getPropertyName(aProp->first), 0, aProp->second, beans::PropertyState_DIRECT_VALUE )
                    );
    }

    boost::optional<PropertyMap::Property> aPropFont = getProperty(PROP_CHAR_FONT_NAME);
    if(aPropFont && !isOutlineNumbering())
        aNumberingProperties.push_back(
                beans::PropertyValue( getPropertyName(PROP_BULLET_FONT_NAME), 0, aPropFont->second, beans::PropertyState_DIRECT_VALUE )
                );

    return comphelper::containerToSequence(aNumberingProperties);
}

// Add the properties only if they do not already exist in the sequence.
void ListLevel::AddParaProperties( uno::Sequence< beans::PropertyValue >* props )
{
    uno::Sequence< beans::PropertyValue >& aProps = *props;

    OUString sFirstLineIndent = getPropertyName(
            PROP_FIRST_LINE_INDENT );
    OUString sIndentAt = getPropertyName(
            PROP_INDENT_AT );

    bool hasFirstLineIndent = lcl_findProperty( aProps, sFirstLineIndent );
    bool hasIndentAt = lcl_findProperty( aProps, sIndentAt );

    if( hasFirstLineIndent && hasIndentAt )
        return; // has them all, nothing to add

    uno::Sequence< beans::PropertyValue > aParaProps = m_pParaStyle->pProperties->GetPropertyValues( );

    // ParaFirstLineIndent -> FirstLineIndent
    // ParaLeftMargin -> IndentAt

    OUString sParaIndent = getPropertyName(
            PROP_PARA_FIRST_LINE_INDENT );
    OUString sParaLeftMargin = getPropertyName(
            PROP_PARA_LEFT_MARGIN );

    sal_Int32 nLen = aParaProps.getLength( );
    for ( sal_Int32 i = 0; i < nLen; i++ )
    {
        if ( !hasFirstLineIndent && aParaProps[i].Name.equals( sParaIndent ) )
        {
            aProps.realloc( aProps.getLength() + 1 );
            aProps[aProps.getLength( ) - 1] = aParaProps[i];
            aProps[aProps.getLength( ) - 1].Name = sFirstLineIndent;
        }
        else if ( !hasIndentAt && aParaProps[i].Name.equals( sParaLeftMargin ) )
        {
            aProps.realloc( aProps.getLength() + 1 );
            aProps[aProps.getLength( ) - 1] = aParaProps[i];
            aProps[aProps.getLength( ) - 1].Name = sIndentAt;
        }

    }
}

NumPicBullet::NumPicBullet()
    : m_nId(0)
{
}

NumPicBullet::~NumPicBullet()
{
}

void NumPicBullet::SetId(sal_Int32 nId)
{
    m_nId = nId;
}

void NumPicBullet::SetShape(uno::Reference<drawing::XShape> const& xShape)
{
    m_xShape = xShape;
}


//--------------------------------------- AbstractListDef implementation

AbstractListDef::AbstractListDef( ) :
    m_nTmpl( -1 )
    ,m_nId( -1 )
{
}

AbstractListDef::~AbstractListDef( )
{
}

void AbstractListDef::SetValue( sal_uInt32 nSprmId, sal_Int32 nValue )
{
    switch( nSprmId )
    {
        case NS_ooxml::LN_CT_AbstractNum_tmpl:
            m_nTmpl = nValue;
        break;
        default:
            OSL_FAIL( "this line should never be reached");
    }
}

ListLevel::Pointer AbstractListDef::GetLevel( sal_uInt16 nLvl )
{
    ListLevel::Pointer pLevel;
    if ( m_aLevels.size( ) > nLvl )
        pLevel = m_aLevels[ nLvl ];
    return pLevel;
}

void AbstractListDef::AddLevel( )
{
    ListLevel::Pointer pLevel( new ListLevel );
    m_pCurrentLevel = pLevel;
    m_aLevels.push_back( pLevel );
}

uno::Sequence< uno::Sequence< beans::PropertyValue > > AbstractListDef::GetPropertyValues( )
{
    uno::Sequence< uno::Sequence< beans::PropertyValue > > result( sal_Int32( m_aLevels.size( ) ) );
    uno::Sequence< beans::PropertyValue >* aResult = result.getArray( );

    int nLevels = m_aLevels.size( );
    for ( int i = 0; i < nLevels; i++ )
    {
        aResult[i] = m_aLevels[i]->GetProperties( );
    }

    return result;
}

//----------------------------------------------  ListDef implementation

ListDef::ListDef( ) : AbstractListDef( )
{
}

ListDef::~ListDef( )
{
}

OUString ListDef::GetStyleName( sal_Int32 nId )
{
    OUString sStyleName( "WWNum" );
    sStyleName += OUString::number( nId );

    return sStyleName;
}

uno::Sequence< uno::Sequence< beans::PropertyValue > > ListDef::GetPropertyValues( )
{
    if (!m_pAbstractDef)
        return uno::Sequence< uno::Sequence< beans::PropertyValue > >();

    // [1] Call the same method on the abstract list
    uno::Sequence< uno::Sequence< beans::PropertyValue > > aAbstract = m_pAbstractDef->GetPropertyValues( );

    // [2] Call the upper class method
    uno::Sequence< uno::Sequence< beans::PropertyValue > > aThis = AbstractListDef::GetPropertyValues( );

    // Merge the results of [2] in [1]
    sal_Int32 nThisCount = aThis.getLength( );
    sal_Int32 nAbstractCount = aAbstract.getLength( );
    for ( sal_Int32 i = 0; i < nThisCount && i < nAbstractCount; i++ )
    {
        uno::Sequence< beans::PropertyValue > level = aThis[i];
        if ( level.hasElements() )
        {
            // If the element contains something, merge it
            lcl_mergeProperties( level, aAbstract[i] );
        }
    }

    return aAbstract;
}

uno::Reference< container::XNameContainer > lcl_getUnoNumberingStyles(
       uno::Reference<lang::XMultiServiceFactory> const& xFactory)
{
    uno::Reference< container::XNameContainer > xStyles;

    try
    {
        uno::Reference< style::XStyleFamiliesSupplier > xFamilies( xFactory, uno::UNO_QUERY_THROW );
        uno::Any oFamily = xFamilies->getStyleFamilies( )->getByName("NumberingStyles");

        oFamily >>= xStyles;
    }
    catch ( const uno::Exception & )
    {
    }

    return xStyles;
}

void ListDef::CreateNumberingRules( DomainMapper& rDMapper,
        uno::Reference<lang::XMultiServiceFactory> const& xFactory)
{
    // Get the UNO Numbering styles
    uno::Reference< container::XNameContainer > xStyles = lcl_getUnoNumberingStyles( xFactory );

    // Do the whole thing
    if( !m_xNumRules.is() && xFactory.is() && xStyles.is( ) )
    {
        try
        {
            // Create the numbering style
            uno::Reference< beans::XPropertySet > xStyle (
                xFactory->createInstance("com.sun.star.style.NumberingStyle"),
                uno::UNO_QUERY_THROW );

            OUString sStyleName = GetStyleName( GetId( ) );

            xStyles->insertByName( sStyleName, makeAny( xStyle ) );

            uno::Any oStyle = xStyles->getByName( sStyleName );
            xStyle.set( oStyle, uno::UNO_QUERY_THROW );

            // Get the default OOo Numbering style rules
            uno::Any aRules = xStyle->getPropertyValue( getPropertyName( PROP_NUMBERING_RULES ) );
            aRules >>= m_xNumRules;

            uno::Sequence< uno::Sequence< beans::PropertyValue > > aProps = GetPropertyValues( );

            sal_Int32 nAbstLevels = m_pAbstractDef ? m_pAbstractDef->Size() : 0;
            sal_Int16 nLevel = 0;
            while ( nLevel < nAbstLevels )
            {
                ListLevel::Pointer pAbsLevel = m_pAbstractDef->GetLevel( nLevel );
                ListLevel::Pointer pLevel = GetLevel( nLevel );

                // Get the merged level properties
                auto aLvlProps = comphelper::sequenceToContainer< std::vector<beans::PropertyValue> >(aProps[sal_Int32(nLevel)]);

                // Get the char style
                uno::Sequence< beans::PropertyValue > aAbsCharStyleProps = pAbsLevel->GetCharStyleProperties( );
                uno::Sequence< beans::PropertyValue >& rAbsCharStyleProps = aAbsCharStyleProps;
                if ( pLevel.get( ) )
                {
                    uno::Sequence< beans::PropertyValue > aCharStyleProps =
                        pLevel->GetCharStyleProperties( );
                    uno::Sequence< beans::PropertyValue >& rCharStyleProps = aCharStyleProps;
                    lcl_mergeProperties( rAbsCharStyleProps, rCharStyleProps );
                }

                if( aAbsCharStyleProps.getLength() )
                {
                    // Change the sequence into a vector
                    PropertyValueVector_t aStyleProps;
                    for ( sal_Int32 i = 0, nLen = aAbsCharStyleProps.getLength() ; i < nLen; i++ )
                    {
                        aStyleProps.push_back( aAbsCharStyleProps[i] );
                    }

                    //create (or find) a character style containing the character
                    // attributes of the symbol and apply it to the numbering level
                    OUString sStyle = rDMapper.getOrCreateCharStyle( aStyleProps, /*bAlwaysCreate=*/true );
                    aLvlProps.push_back(comphelper::makePropertyValue(getPropertyName(PROP_CHAR_STYLE_NAME), sStyle));
                }

                // Get the prefix / suffix / Parent numbering
                // and add them to the level properties
                OUString sText = pAbsLevel->GetBulletChar( );
                // Inherit <w:lvlText> from the abstract level in case the override would be empty.
                if (pLevel.get() && !pLevel->GetBulletChar().isEmpty())
                    sText = pLevel->GetBulletChar( );

                OUString sPrefix;
                OUString sSuffix;
                OUString& rPrefix = sPrefix;
                OUString& rSuffix = sSuffix;
                sal_Int16 nParentNum = ListLevel::GetParentNumbering(
                       sText, nLevel, rPrefix, rSuffix );

                aLvlProps.push_back(comphelper::makePropertyValue(getPropertyName(PROP_PREFIX), rPrefix));

                if (sText.isEmpty())
                {
                    // Empty <w:lvlText>? Then put a Unicode "zero width space" as a suffix, so LabelFollowedBy is still shown, as in Word.
                    // With empty suffix, Writer does not show LabelFollowedBy, either.
                    auto it = std::find_if(aLvlProps.begin(), aLvlProps.end(), [](const beans::PropertyValue& rValue) { return rValue.Name == "NumberingType"; });
                    if (it != aLvlProps.end())
                    {
                        sal_Int16 nNumberFormat = it->Value.get<sal_Int16>();

                        // No need for a zero width space without a real LabelFollowedBy.
                        bool bLabelFollowedBy = true;
                        it = std::find_if(aLvlProps.begin(), aLvlProps.end(), [](const beans::PropertyValue& rValue) { return rValue.Name == "LabelFollowedBy"; });
                        if (it != aLvlProps.end())
                        {
                            sal_Int16 nValue;
                            if (it->Value >>= nValue)
                                bLabelFollowedBy = nValue != SvxNumberFormat::NOTHING;
                        }

                        if (bLabelFollowedBy && nNumberFormat == style::NumberingType::NUMBER_NONE)
                            rSuffix = OUString(static_cast<sal_Unicode>(0x200B));
                    }
                }

                aLvlProps.push_back(comphelper::makePropertyValue(getPropertyName(PROP_SUFFIX), rSuffix));
                aLvlProps.push_back(comphelper::makePropertyValue(getPropertyName(PROP_PARENT_NUMBERING), nParentNum));

                aLvlProps.push_back(comphelper::makePropertyValue(getPropertyName(PROP_POSITION_AND_SPACE_MODE), sal_Int16(text::PositionAndSpaceMode::LABEL_ALIGNMENT)));


                // Replace the numbering rules for the level
                m_xNumRules->replaceByIndex(nLevel, uno::makeAny(comphelper::containerToSequence(aLvlProps)));

                // Handle the outline level here
                if ( pAbsLevel->isOutlineNumbering())
                {
                    uno::Reference< text::XChapterNumberingSupplier > xOutlines (
                        xFactory, uno::UNO_QUERY_THROW );
                    uno::Reference< container::XIndexReplace > xOutlineRules =
                        xOutlines->getChapterNumberingRules( );

                    StyleSheetEntryPtr pParaStyle = pAbsLevel->GetParaStyle( );
                    aLvlProps.push_back(comphelper::makePropertyValue(getPropertyName(PROP_HEADING_STYLE_NAME), pParaStyle->sConvertedStyleName));

                    xOutlineRules->replaceByIndex(nLevel, uno::makeAny(comphelper::containerToSequence(aLvlProps)));
                }

                nLevel++;
            }

            // Create the numbering style for these rules
            OUString sNumRulesName = getPropertyName( PROP_NUMBERING_RULES );
            xStyle->setPropertyValue( sNumRulesName, uno::makeAny( m_xNumRules ) );
        }
        catch( const lang::IllegalArgumentException& e )
        {
            SAL_WARN( "writerfilter", "Exception: " << e.Message );
             assert( !"Incorrect argument to UNO call" );
        }
        catch( const uno::RuntimeException& e )
        {
            SAL_WARN( "writerfilter", "Exception: " << e.Message );
             assert( !"Incorrect argument to UNO call" );
        }
        catch( const uno::Exception& e )
        {
            SAL_WARN( "writerfilter", "Exception: " << e.Message );
        }
    }

}

//-------------------------------------  NumberingManager implementation


ListsManager::ListsManager(DomainMapper& rDMapper,
    const uno::Reference<lang::XMultiServiceFactory> & xFactory)
    : LoggedProperties("ListsManager")
    , LoggedTable("ListsManager")
    , m_rDMapper(rDMapper)
    , m_xFactory(xFactory)
{
}

ListsManager::~ListsManager( )
{
    DisposeNumPicBullets();
}

void ListsManager::DisposeNumPicBullets( )
{
    uno::Reference<drawing::XShape> xShape;
    for (std::vector<NumPicBullet::Pointer>::iterator it = m_aNumPicBullets.begin(); it != m_aNumPicBullets.end(); ++it)
    {
        xShape = (*it)->GetShape();
        if (xShape.is())
        {
            uno::Reference<lang::XComponent> xShapeComponent(xShape, uno::UNO_QUERY);
            xShapeComponent->dispose();
        }
    }
}

void ListsManager::lcl_attribute( Id nName, Value& rVal )
{
    ListLevel::Pointer pCurrentLvl;

    if (nName != NS_ooxml::LN_CT_NumPicBullet_numPicBulletId)
    {
        OSL_ENSURE( m_pCurrentDefinition.get(), "current entry has to be set here");
        if(!m_pCurrentDefinition.get())
            return ;
        pCurrentLvl = m_pCurrentDefinition->GetCurrentLevel( );
    }
    else
    {
        SAL_WARN_IF(!m_pCurrentNumPicBullet.get(), "writerfilter", "current entry has to be set here");
        if (!m_pCurrentNumPicBullet.get())
            return;
    }
    int nIntValue = rVal.getInt();


    switch(nName)
    {
        case NS_ooxml::LN_CT_LevelText_val:
        {
            //this strings contains the definition of the level
            //the level number is marked as %n
            //these numbers can be mixed randomly together with separators pre- and suffixes
            //the Writer supports only a number of upper levels to show, separators is always a dot
            //and each level can have a prefix and a suffix
            if(pCurrentLvl.get())
            {
                //if the BulletChar is a soft-hyphen (0xad)
                //replace it with a hard-hyphen (0x2d)
                //-> this fixes missing hyphen export in PDF etc.
                // see tdf#101626
                pCurrentLvl->SetBulletChar( rVal.getString().replace( 0xad, 0x2d ) );
            }
        }
        break;
        case NS_ooxml::LN_CT_Lvl_start:
        case NS_ooxml::LN_CT_Lvl_numFmt:
        case NS_ooxml::LN_CT_Lvl_isLgl:
        case NS_ooxml::LN_CT_Lvl_legacy:
            if ( pCurrentLvl.get( ) )
                pCurrentLvl->SetValue( nName, sal_Int32( nIntValue ) );
        break;
        case NS_ooxml::LN_CT_Num_numId:
            m_pCurrentDefinition->SetId( rVal.getString().toInt32( ) );
        break;
        case NS_ooxml::LN_CT_AbstractNum_nsid:
            m_pCurrentDefinition->SetId( nIntValue );
        break;
        case NS_ooxml::LN_CT_AbstractNum_tmpl:
            m_pCurrentDefinition->SetValue( nName, nIntValue );
        break;
        case NS_ooxml::LN_CT_NumLvl_ilvl:
        {
            //add a new level to the level vector and make it the current one
            m_pCurrentDefinition->AddLevel();

            writerfilter::Reference<Properties>::Pointer_t pProperties;
            if((pProperties = rVal.getProperties()).get())
                pProperties->resolve(*this);
        }
        break;
        case NS_ooxml::LN_CT_AbstractNum_abstractNumId:
        {
            // This one corresponds to the AbstractNum Id definition
            // The reference to the abstract num is in the sprm method
            sal_Int32 nVal = rVal.getString().toInt32();
            m_pCurrentDefinition->SetId( nVal );
        }
        break;
        case NS_ooxml::LN_CT_Ind_left:
            pCurrentLvl->Insert(
                PROP_INDENT_AT, uno::makeAny( ConversionHelper::convertTwipToMM100( nIntValue ) ));
            break;
        case NS_ooxml::LN_CT_Ind_hanging:
            pCurrentLvl->Insert(
                PROP_FIRST_LINE_INDENT, uno::makeAny( - ConversionHelper::convertTwipToMM100( nIntValue ) ));
        break;
        case NS_ooxml::LN_CT_Ind_firstLine:
            pCurrentLvl->Insert(
                PROP_FIRST_LINE_INDENT, uno::makeAny( ConversionHelper::convertTwipToMM100( nIntValue ) ));
        break;
        case NS_ooxml::LN_CT_Lvl_ilvl: //overrides previous level - unsupported
        case NS_ooxml::LN_CT_Lvl_tplc: //template code - unsupported
        case NS_ooxml::LN_CT_Lvl_tentative: //marks level as unused in the document - unsupported
        break;
        case NS_ooxml::LN_CT_TabStop_pos:
        {
            //no paragraph attributes in ListTable char style sheets
            if ( pCurrentLvl.get( ) )
                pCurrentLvl->SetValue( nName,
                    ConversionHelper::convertTwipToMM100( nIntValue ) );
        }
        break;
        case NS_ooxml::LN_CT_TabStop_val:
        {
            // TODO Do something of that
        }
        break;
        case NS_ooxml::LN_CT_NumPicBullet_numPicBulletId:
            m_pCurrentNumPicBullet->SetId(rVal.getString().toInt32());
        break;
        default:
            SAL_WARN("writerfilter", "ListsManager::lcl_attribute: unhandled token: " << nName);
    }
}

void ListsManager::lcl_sprm( Sprm& rSprm )
{
    //fill the attributes of the style sheet
    sal_uInt32 nSprmId = rSprm.getId();
    if( m_pCurrentDefinition.get() ||
        nSprmId == NS_ooxml::LN_CT_Numbering_abstractNum ||
        nSprmId == NS_ooxml::LN_CT_Numbering_num ||
        (nSprmId == NS_ooxml::LN_CT_NumPicBullet_pict && m_pCurrentNumPicBullet.get()) ||
        nSprmId == NS_ooxml::LN_CT_Numbering_numPicBullet)
    {
        static bool bIsStartVisited = false;
        sal_Int32 nIntValue = rSprm.getValue()->getInt();
        switch( nSprmId )
        {
            case NS_ooxml::LN_CT_Numbering_abstractNum:
            {
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if(pProperties.get())
                {
                    //create a new Abstract list entry
                    OSL_ENSURE( !m_pCurrentDefinition.get(), "current entry has to be NULL here");
                    m_pCurrentDefinition.reset( new AbstractListDef );
                    pProperties->resolve( *this );
                    //append it to the table
                    m_aAbstractLists.push_back( m_pCurrentDefinition );
                    m_pCurrentDefinition = AbstractListDef::Pointer();
                }
            }
            break;
            case NS_ooxml::LN_CT_Numbering_num:
            {
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if(pProperties.get())
                {
                    // Create a new list entry
                    OSL_ENSURE( !m_pCurrentDefinition.get(), "current entry has to be NULL here");
                    ListDef::Pointer listDef( new ListDef );
                    m_pCurrentDefinition = listDef;
                    pProperties->resolve( *this );
                    //append it to the table
                    m_aLists.push_back( listDef );

                    m_pCurrentDefinition = AbstractListDef::Pointer();
                }
            }
            break;
            case NS_ooxml::LN_CT_Numbering_numPicBullet:
            {
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if (pProperties.get())
                {
                    NumPicBullet::Pointer numPicBullet(new NumPicBullet());
                    m_pCurrentNumPicBullet = numPicBullet;
                    pProperties->resolve(*this);
                    m_aNumPicBullets.push_back(numPicBullet);
                    m_pCurrentNumPicBullet = NumPicBullet::Pointer();
                }
            }
            break;
            case NS_ooxml::LN_CT_NumPicBullet_pict:
            {
                uno::Reference<drawing::XShape> xShape = m_rDMapper.PopPendingShape();

                // Respect only the aspect ratio of the picture, not its size.
                awt::Size aPrefSize = xShape->getSize();
                // See SwDefBulletConfig::InitFont(), default height is 14.
                const int nFontHeight = 14;
                // Point -> mm100.
                const int nHeight = nFontHeight * 35;
                if (aPrefSize.Height * aPrefSize.Width != 0)
                {
                    int nWidth = (nHeight * aPrefSize.Width) / aPrefSize.Height;
                    awt::Size aSize(nWidth, nHeight);
                    xShape->setSize(aSize);
                }

                m_pCurrentNumPicBullet->SetShape(xShape);
            }
            break;
            case NS_ooxml::LN_CT_Lvl_lvlPicBulletId:
            {
                uno::Reference<drawing::XShape> xShape;
                for (std::vector<NumPicBullet::Pointer>::iterator it = m_aNumPicBullets.begin(); it != m_aNumPicBullets.end(); ++it)
                {
                    if ((*it)->GetId() == nIntValue)
                    {
                        xShape = (*it)->GetShape();
                        break;
                    }
                }
                if (xShape.is())
                {
                    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
                    try
                    {
                        uno::Any aAny = xPropertySet->getPropertyValue("GraphicURL");
                        if (aAny.has<OUString>())
                            m_pCurrentDefinition->GetCurrentLevel()->SetGraphicURL(aAny.get<OUString>());
                    } catch(const beans::UnknownPropertyException&)
                    {}
                    try
                    {
                        uno::Reference< graphic::XGraphic > gr;
                        xPropertySet->getPropertyValue("Bitmap") >>= gr;
                        m_pCurrentDefinition->GetCurrentLevel()->SetGraphicBitmap( gr );
                    } catch(const beans::UnknownPropertyException&)
                    {}

                    // Now that we saved the URL of the graphic, remove it from the document.
                    uno::Reference<lang::XComponent> xShapeComponent(xShape, uno::UNO_QUERY);
                    xShapeComponent->dispose();
                }
            }
            break;
            case NS_ooxml::LN_CT_Num_abstractNumId:
            {
                sal_Int32 nAbstractNumId = rSprm.getValue()->getInt();
                ListDef* pListDef = dynamic_cast< ListDef* >( m_pCurrentDefinition.get( ) );
                if ( pListDef != nullptr )
                {
                    // The current def should be a ListDef
                    pListDef->SetAbstractDefinition(
                           GetAbstractList( nAbstractNumId ) );
                }
            }
            break;
            case NS_ooxml::LN_CT_AbstractNum_multiLevelType:
            break;
            case NS_ooxml::LN_CT_AbstractNum_tmpl:
                m_pCurrentDefinition->SetValue( nSprmId, nIntValue );
            break;
            case NS_ooxml::LN_CT_AbstractNum_lvl:
            {
                m_pCurrentDefinition->AddLevel();
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if(pProperties.get())
                    pProperties->resolve(*this);
            }
            break;
            case NS_ooxml::LN_CT_Lvl_start:
                if (m_pCurrentDefinition->GetCurrentLevel().get())
                    m_pCurrentDefinition->GetCurrentLevel( )->SetValue( nSprmId, nIntValue );
                bIsStartVisited = true;
            break;
            case NS_ooxml::LN_CT_Lvl_numFmt:
            case NS_ooxml::LN_CT_Lvl_isLgl:
            case NS_ooxml::LN_CT_Lvl_legacy:
                if (m_pCurrentDefinition->GetCurrentLevel().get())
                {
                    m_pCurrentDefinition->GetCurrentLevel( )->SetValue( nSprmId, nIntValue );
                    if( !bIsStartVisited )
                    {
                        m_pCurrentDefinition->GetCurrentLevel( )->SetValue( NS_ooxml::LN_CT_Lvl_start, 0 );
                        bIsStartVisited = true;
                    }
                }
            break;
            case NS_ooxml::LN_CT_Lvl_suff:
            {
                if (m_pCurrentDefinition->GetCurrentLevel().get())
                {
                    SvxNumberFormat::LabelFollowedBy value = SvxNumberFormat::LISTTAB;
                    if( rSprm.getValue()->getString() == "tab" )
                        value = SvxNumberFormat::LISTTAB;
                    else if( rSprm.getValue()->getString() == "space" )
                        value = SvxNumberFormat::SPACE;
                    else if( rSprm.getValue()->getString() == "nothing" )
                        value = SvxNumberFormat::NOTHING;
                    else
                        SAL_WARN( "writerfilter", "Unknown ST_LevelSuffix value "
                            << rSprm.getValue()->getString());
                    m_pCurrentDefinition->GetCurrentLevel()->SetValue( nSprmId, value );
                }
            }
            break;
            case NS_ooxml::LN_CT_Lvl_lvlText:
            case NS_ooxml::LN_CT_Lvl_rPr : //contains LN_EG_RPrBase_rFonts
            {
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if(pProperties.get())
                    pProperties->resolve(*this);
            }
            break;
            case NS_ooxml::LN_CT_NumLvl_lvl:
            {
                // overwrite level
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if(pProperties.get())
                    pProperties->resolve(*this);
            }
            break;
            case NS_ooxml::LN_CT_Lvl_lvlJc:
            {
                sal_Int16 nValue = text::HoriOrientation::NONE;
                switch (nIntValue)
                {
                case NS_ooxml::LN_Value_ST_Jc_left:
                case NS_ooxml::LN_Value_ST_Jc_start:
                    nValue = text::HoriOrientation::LEFT;
                    break;
                case NS_ooxml::LN_Value_ST_Jc_center:
                    nValue = text::HoriOrientation::CENTER;
                    break;
                case NS_ooxml::LN_Value_ST_Jc_right:
                case NS_ooxml::LN_Value_ST_Jc_end:
                    nValue = text::HoriOrientation::RIGHT;
                    break;
                }
                if (nValue != text::HoriOrientation::NONE)
                {
                    m_pCurrentDefinition->GetCurrentLevel( )->Insert(
                        PROP_ADJUST, uno::makeAny( nValue ) );
                        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                }
            }
            break;
            case NS_ooxml::LN_CT_Lvl_pPr:
            case NS_ooxml::LN_CT_PPrBase_ind:
            {
                //todo: how to handle paragraph properties within numbering levels (except LeftIndent and FirstLineIndent)?
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if(pProperties.get())
                    pProperties->resolve(*this);
            }
            break;
            case NS_ooxml::LN_CT_PPrBase_tabs:
            case NS_ooxml::LN_CT_Tabs_tab:
            {
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if(pProperties.get())
                    pProperties->resolve(*this);
            }
            break;
            case NS_ooxml::LN_CT_Lvl_pStyle:
            {
                OUString sStyleName = rSprm.getValue( )->getString( );
                ListLevel::Pointer pLevel = m_pCurrentDefinition->GetCurrentLevel( );
                StyleSheetTablePtr pStylesTable = m_rDMapper.GetStyleSheetTable( );
                const StyleSheetEntryPtr pStyle = pStylesTable->FindStyleSheetByISTD( sStyleName );
                pLevel->SetParaStyle( pStyle );
            }
            break;
            case NS_ooxml::LN_CT_Num_lvlOverride:
            {
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if (pProperties.get())
                    pProperties->resolve(*this);
            }
            break;
            case NS_ooxml::LN_CT_NumLvl_startOverride:
            {
                if(m_pCurrentDefinition)
                {
                    if (ListLevel::Pointer pCurrentLevel = m_pCurrentDefinition->GetCurrentLevel())
                        // <w:num> -> <w:lvlOverride> -> <w:startOverride> is the non-abstract equivalent of
                        // <w:abstractNum> -> <w:lvl> -> <w:start>
                        pCurrentLevel->SetValue(NS_ooxml::LN_CT_Lvl_start, nIntValue);
                }
            }
            break;
            case NS_ooxml::LN_CT_AbstractNum_numStyleLink:
            {
                OUString sStyleName = rSprm.getValue( )->getString( );
                AbstractListDef* pAbstractListDef = dynamic_cast< AbstractListDef* >( m_pCurrentDefinition.get( ) );
                pAbstractListDef->SetNumStyleLink(sStyleName);
            }
            break;
            case NS_ooxml::LN_EG_RPrBase_rFonts: //contains font properties
            case NS_ooxml::LN_EG_RPrBase_color:
            case NS_ooxml::LN_EG_RPrBase_u:
            case NS_ooxml::LN_EG_RPrBase_sz:
            case NS_ooxml::LN_EG_RPrBase_lang:
            case NS_ooxml::LN_EG_RPrBase_eastAsianLayout:
                //no break!
            default:
                if( m_pCurrentDefinition->GetCurrentLevel( ).get())
                {
                    m_rDMapper.PushListProperties( m_pCurrentDefinition->GetCurrentLevel( ) );
                    m_rDMapper.sprm( rSprm );
                    m_rDMapper.PopListProperties();
                }
        }
    }
}

void ListsManager::lcl_entry( int /* pos */,
                          writerfilter::Reference<Properties>::Pointer_t ref )
{
    if( m_rDMapper.IsOOXMLImport() || m_rDMapper.IsRTFImport() )
    {
        ref->resolve(*this);
    }
    else
    {
        // Create AbstractListDef's
        OSL_ENSURE( !m_pCurrentDefinition.get(), "current entry has to be NULL here");
        m_pCurrentDefinition.reset( new AbstractListDef( ) );
        ref->resolve(*this);
        //append it to the table
        m_aAbstractLists.push_back( m_pCurrentDefinition );
        m_pCurrentDefinition = AbstractListDef::Pointer();
    }
}

AbstractListDef::Pointer ListsManager::GetAbstractList( sal_Int32 nId )
{
    AbstractListDef::Pointer pAbstractList;

    int nLen = m_aAbstractLists.size( );
    int i = 0;
    while ( !pAbstractList.get( ) && i < nLen )
    {
        if ( m_aAbstractLists[i]->GetId( ) == nId )
        {
            if ( m_aAbstractLists[i]->GetNumStyleLink().getLength() > 0 )
            {
                // If the abstract num has a style linked, check the linked style's number id.
                StyleSheetTablePtr pStylesTable = m_rDMapper.GetStyleSheetTable( );

                const StyleSheetEntryPtr pStyleSheetEntry =
                    pStylesTable->FindStyleSheetByISTD( m_aAbstractLists[i]->GetNumStyleLink() );

                const StyleSheetPropertyMap* pStyleSheetProperties =
                    dynamic_cast<const StyleSheetPropertyMap*>(pStyleSheetEntry ? pStyleSheetEntry->pProperties.get() : nullptr);

                if( pStyleSheetProperties && pStyleSheetProperties->GetNumId() >= 0 )
                {
                    ListDef::Pointer pList = GetList( pStyleSheetProperties->GetNumId() );
                    if ( pList!=nullptr )
                        return pList->GetAbstractDefinition();
                    else
                        pAbstractList = m_aAbstractLists[i];
                }

            }
            else
            {
                pAbstractList = m_aAbstractLists[i];
            }
        }
        i++;
    }

    return pAbstractList;
}

ListDef::Pointer ListsManager::GetList( sal_Int32 nId )
{
    ListDef::Pointer pList;

    int nLen = m_aLists.size( );
    int i = 0;
    while ( !pList.get( ) && i < nLen )
    {
        if ( m_aLists[i]->GetId( ) == nId )
            pList = m_aLists[i];
        i++;
    }

    return pList;
}

void ListsManager::CreateNumberingRules( )
{
    // Loop over the definitions
    std::vector< ListDef::Pointer >::iterator listIt = m_aLists.begin( );
    for ( ; listIt != m_aLists.end( ); ++listIt )
    {
        (*listIt)->CreateNumberingRules( m_rDMapper, m_xFactory );
    }
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
