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
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/awt/XBitmap.hpp>

#include <osl/diagnose.h>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/UnitConversion.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/string.hxx>
#include <unotxdoc.hxx>
#include <unoxstyle.hxx>
#include <regex>
#include <utility>

using namespace com::sun::star;

namespace writerfilter::dmapper {

//---------------------------------------------------  Utility functions
template <typename T>
static beans::PropertyValue lcl_makePropVal(PropertyIds nNameID, T const & aValue)
{
    return comphelper::makePropertyValue(getPropertyName(nNameID), aValue);
}

static sal_Int32 lcl_findProperty( const uno::Sequence< beans::PropertyValue >& aProps, std::u16string_view sName )
{
    sal_Int32 i = 0;
    sal_Int32 nLen = aProps.getLength( );
    sal_Int32 nPos = -1;

    while ( nPos == -1 && i < nLen )
    {
        if ( aProps[i].Name == sName )
            nPos = i;
        else
            i++;
    }

    return nPos;
}

static void lcl_mergeProperties( const uno::Sequence< beans::PropertyValue >& aSrc,
        uno::Sequence< beans::PropertyValue >& aDst )
{
    for ( const auto& rProp : aSrc )
    {
        // Look for the same property in aDst
        sal_Int32 nPos = lcl_findProperty( aDst, rProp.Name );
        if ( nPos >= 0 )
        {
            // Replace the property value by the one in aSrc
            aDst.getArray()[nPos] = rProp;
        }
        else
        {
            // Simply add the new value
            aDst.realloc( aDst.getLength( ) + 1 );
            aDst.getArray()[ aDst.getLength( ) - 1 ] = rProp;
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
        case NS_ooxml::LN_CT_NumLvl_startOverride:
            m_nStartOverride = nValue;
            break;
        case NS_ooxml::LN_CT_NumFmt_val:
            m_nNFC = nValue;
        break;
        case NS_ooxml::LN_CT_Lvl_isLgl:
            m_bIsLegal = true;
        break;
        case NS_ooxml::LN_CT_Lvl_legacy:
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
    m_bHasValues = true;
}

void ListLevel::SetCustomNumberFormat(const OUString& rValue) { m_aCustomNumberFormat = rValue; }

sal_Int16 ListLevel::GetNumberingType(sal_Int16 nDefault) const
{
    return ConversionHelper::ConvertNumberingType(m_nNFC, nDefault);
}

bool ListLevel::HasValues() const
{
    return m_bHasValues;
}

void ListLevel::SetParaStyle( const tools::SvRef< StyleSheetEntry >& pStyle )
{
    if (!pStyle)
        return;
    m_pParaStyle = pStyle;
}

uno::Sequence<beans::PropertyValue> ListLevel::GetProperties(bool bDefaults)
{
    uno::Sequence<beans::PropertyValue> aLevelProps = GetLevelProperties(bDefaults);
    if (m_pParaStyle)
        AddParaProperties( &aLevelProps );
    return aLevelProps;
}

static bool IgnoreForCharStyle(std::u16string_view aStr, const bool bIsSymbol)
{
    //Names found in PropertyIds.cxx, Lines 56-396
    return (aStr==u"Adjust" || aStr==u"IndentAt" || aStr==u"FirstLineIndent"
            || aStr==u"FirstLineOffset" || aStr==u"LeftMargin"
            // We need font names when they are different for the bullet and for the text.
            // But leave symbols alone, we only want to keep the font style for letters and numbers.
            || (bIsSymbol && aStr==u"CharFontName")
        );
}
uno::Sequence< beans::PropertyValue > ListLevel::GetCharStyleProperties( )
{
    PropertyValueVector_t rProperties;

    const uno::Sequence< beans::PropertyValue > vPropVals = PropertyMap::GetPropertyValues();
    const bool bIsSymbol(GetBulletChar().getLength() <= 1);
    for( const auto& rPropNal : vPropVals )
        if (! IgnoreForCharStyle(rPropNal.Name, bIsSymbol))
            rProperties.emplace_back(rPropNal.Name, 0, rPropNal.Value, beans::PropertyState_DIRECT_VALUE);

    return comphelper::containerToSequence(rProperties);
}

uno::Sequence<beans::PropertyValue> ListLevel::GetLevelProperties(bool bDefaults)
{
    std::vector<beans::PropertyValue> aNumberingProperties;

    if (m_nIStartAt >= 0)
        aNumberingProperties.push_back(lcl_makePropVal<sal_Int16>(PROP_START_WITH, m_nIStartAt) );
    else if (bDefaults)
        aNumberingProperties.push_back(lcl_makePropVal<sal_Int16>(PROP_START_WITH, 0));

    sal_Int16 nNumberFormat = -1;
    if (m_nNFC == NS_ooxml::LN_Value_ST_NumberFormat_custom)
    {
        nNumberFormat = ConversionHelper::ConvertCustomNumberFormat(m_aCustomNumberFormat);
    }
    else
    {
        nNumberFormat = ConversionHelper::ConvertNumberingType(m_nNFC);
    }
    if( m_nNFC >= 0)
    {
        if (m_xGraphicBitmap.is())
            nNumberFormat = style::NumberingType::BITMAP;
        aNumberingProperties.push_back(lcl_makePropVal(PROP_NUMBERING_TYPE, nNumberFormat));
    }

    // todo: this is not the bullet char
    if( nNumberFormat == style::NumberingType::CHAR_SPECIAL )
    {
        if (!GetBulletChar().isEmpty())
        {
            aNumberingProperties.push_back(lcl_makePropVal(PROP_BULLET_CHAR, m_sBulletChar->copy(0, 1)));
        }
        else
        {
            // If w:lvlText's value is null - set bullet char to zero.
            aNumberingProperties.push_back(lcl_makePropVal<sal_Unicode>(PROP_BULLET_CHAR, 0));
        }
    }
    if (m_xGraphicBitmap.is())
    {
        aNumberingProperties.push_back(lcl_makePropVal(PROP_GRAPHIC_BITMAP, m_xGraphicBitmap));
        aNumberingProperties.push_back(lcl_makePropVal(PROP_GRAPHIC_SIZE, m_aGraphicSize));
    }

    if (m_nTabstop.has_value())
        aNumberingProperties.push_back(lcl_makePropVal(PROP_LISTTAB_STOP_POSITION, *m_nTabstop));
    else if (bDefaults)
        aNumberingProperties.push_back(lcl_makePropVal<sal_Int16>(PROP_LISTTAB_STOP_POSITION, 0));

    //TODO: handling of nFLegal?
    //TODO: nFNoRestart lower levels do not restart when higher levels are incremented, like:
    //1.
    //1.1
    //2.2
    //2.3
    //3.4

//    TODO: sRGBXchNums;     array of inherited numbers

//  nXChFollow; following character 0 - tab, 1 - space, 2 - nothing
    if (bDefaults || m_nXChFollow != SvxNumberFormat::LISTTAB)
        aNumberingProperties.push_back(lcl_makePropVal(PROP_LEVEL_FOLLOW, m_nXChFollow));

    PropertyIds const aReadIds[] =
    {
        PROP_ADJUST, PROP_INDENT_AT, PROP_FIRST_LINE_INDENT,
        PROP_FIRST_LINE_OFFSET, PROP_LEFT_MARGIN
    };
    for(PropertyIds const & rReadId : aReadIds) {
        std::optional<PropertyMap::Property> aProp = getProperty(rReadId);
        if (aProp)
            aNumberingProperties.emplace_back( getPropertyName(aProp->first), 0, aProp->second, beans::PropertyState_DIRECT_VALUE );
        else if (rReadId == PROP_FIRST_LINE_INDENT && bDefaults)
            // Writer default is -360 twips, Word default seems to be 0.
            aNumberingProperties.emplace_back("FirstLineIndent", 0, uno::Any(static_cast<sal_Int32>(0)), beans::PropertyState_DIRECT_VALUE);
        else if (rReadId == PROP_INDENT_AT && bDefaults)
            // Writer default is 720 twips, Word default seems to be 0.
            aNumberingProperties.emplace_back("IndentAt", 0,
                                              uno::Any(static_cast<sal_Int32>(0)),
                                              beans::PropertyState_DIRECT_VALUE);
    }

    std::optional<PropertyMap::Property> aPropFont = getProperty(PROP_CHAR_FONT_NAME);
    if (aPropFont)
        aNumberingProperties.emplace_back( getPropertyName(PROP_BULLET_FONT_NAME), 0, aPropFont->second, beans::PropertyState_DIRECT_VALUE );

    if (m_bIsLegal)
        aNumberingProperties.push_back(lcl_makePropVal(PROP_LEVEL_IS_LEGAL, true));

    return comphelper::containerToSequence(aNumberingProperties);
}

// Add the properties only if they do not already exist in the sequence.
void ListLevel::AddParaProperties( uno::Sequence< beans::PropertyValue >* props )
{
    uno::Sequence< beans::PropertyValue >& aProps = *props;

    const OUString & sFirstLineIndent = getPropertyName(
            PROP_FIRST_LINE_INDENT );
    const OUString & sIndentAt = getPropertyName(
            PROP_INDENT_AT );

    bool hasFirstLineIndent = lcl_findProperty( aProps, sFirstLineIndent );
    bool hasIndentAt = lcl_findProperty( aProps, sIndentAt );

    if( hasFirstLineIndent && hasIndentAt )
        return; // has them all, nothing to add

    const uno::Sequence< beans::PropertyValue > aParaProps = m_pParaStyle->m_pProperties->GetPropertyValues( );

    // ParaFirstLineIndent -> FirstLineIndent
    // ParaLeftMargin -> IndentAt

    const OUString & sParaIndent = getPropertyName(
            PROP_PARA_FIRST_LINE_INDENT );
    const OUString & sParaLeftMargin = getPropertyName(
            PROP_PARA_LEFT_MARGIN );

    for ( const auto& rParaProp : aParaProps )
    {
        if ( !hasFirstLineIndent && rParaProp.Name == sParaIndent )
        {
            aProps.realloc( aProps.getLength() + 1 );
            auto pProps = aProps.getArray();
            pProps[aProps.getLength( ) - 1] = rParaProp;
            pProps[aProps.getLength( ) - 1].Name = sFirstLineIndent;
        }
        else if ( !hasIndentAt && rParaProp.Name == sParaLeftMargin )
        {
            aProps.realloc( aProps.getLength() + 1 );
            auto pProps = aProps.getArray();
            pProps[aProps.getLength( ) - 1] = rParaProp;
            pProps[aProps.getLength( ) - 1].Name = sIndentAt;
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
    m_nId( -1 )
{
}

AbstractListDef::~AbstractListDef( )
{
}

void AbstractListDef::SetValue( sal_uInt32 nSprmId )
{
    switch( nSprmId )
    {
        case NS_ooxml::LN_CT_AbstractNum_tmpl:
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

void AbstractListDef::AddLevel( sal_uInt16 nLvl )
{
    if ( nLvl >= m_aLevels.size() )
        m_aLevels.resize( nLvl+1 );

    if (!m_aLevels[nLvl])
    {
        m_aLevels[nLvl] = new ListLevel;
    }

    m_pCurrentLevel = m_aLevels[nLvl];
}

uno::Sequence<uno::Sequence<beans::PropertyValue>> AbstractListDef::GetPropertyValues(bool bDefaults)
{
    uno::Sequence< uno::Sequence< beans::PropertyValue > > result( sal_Int32( m_aLevels.size( ) ) );
    uno::Sequence< beans::PropertyValue >* aResult = result.getArray( );

    int nLevels = m_aLevels.size( );
    for ( int i = 0; i < nLevels; i++ )
    {
        if (m_aLevels[i])
            aResult[i] = m_aLevels[i]->GetProperties(bDefaults);
    }

    return result;
}

const OUString& AbstractListDef::MapListId(OUString const& rId)
{
    if (!m_oListId)
    {
        m_oListId = rId;
    }
    return *m_oListId;
}

//----------------------------------------------  ListDef implementation

ListDef::ListDef( )
{
}

ListDef::~ListDef( )
{
}

const OUString & ListDef::GetStyleName(sal_Int32 const nId,
    uno::Reference<container::XNameContainer> const& xStyles)
{
    if (xStyles.is())
    {
        OUString sStyleName = "WWNum" + OUString::number( nId );

        while (xStyles->hasByName(sStyleName)) // unique
        {
            sStyleName += "a";
        }

        m_StyleName = sStyleName;
    }
    else
    {
// fails in rtftok test        assert(!m_StyleName.isEmpty()); // must be inited first
    }

    return m_StyleName;
}

uno::Sequence<uno::Sequence<beans::PropertyValue>> ListDef::GetMergedPropertyValues()
{
    if (!m_pAbstractDef)
        return uno::Sequence< uno::Sequence< beans::PropertyValue > >();

    // [1] Call the same method on the abstract list
    uno::Sequence<uno::Sequence<beans::PropertyValue>> aAbstract
        = m_pAbstractDef->GetPropertyValues(/*bDefaults=*/true);
    auto aAbstractRange = asNonConstRange(aAbstract);

    // [2] Call the upper class method
    uno::Sequence<uno::Sequence<beans::PropertyValue>> aThis
        = AbstractListDef::GetPropertyValues(/*bDefaults=*/false);

    // Merge the results of [2] in [1]
    sal_Int32 nThisCount = aThis.getLength( );
    sal_Int32 nAbstractCount = aAbstract.getLength( );
    for ( sal_Int32 i = 0; i < nThisCount && i < nAbstractCount; i++ )
    {
        uno::Sequence< beans::PropertyValue > level = aThis[i];
        if (level.hasElements() && GetLevel(i)->HasValues())
        {
            // If the element contains something, merge it, but ignore stub overrides.
            lcl_mergeProperties( level, aAbstractRange[i] );
        }
    }

    return aAbstract;
}

static uno::Reference< container::XNameContainer > lcl_getUnoNumberingStyles(
       rtl::Reference<SwXTextDocument> const& xTextDoc)
{
    uno::Reference< container::XNameContainer > xStyles;
    if (!xTextDoc)
        return xStyles;

    try
    {
        uno::Any oFamily = xTextDoc->getStyleFamilies( )->getByName("NumberingStyles");

        oFamily >>= xStyles;
    }
    catch ( const uno::Exception & )
    {
    }

    return xStyles;
}

/// Rank the list in terms of suitability for becoming the Outline numbering rule in LO.
sal_uInt16 ListDef::GetChapterNumberingWeight() const
{
    sal_Int16 nWeight = 0;
    const sal_Int8 nAbstLevels = m_pAbstractDef ? m_pAbstractDef->Size() : 0;
    for (sal_Int8 nLevel = 0; nLevel < nAbstLevels; ++nLevel)
    {
        const ListLevel::Pointer pAbsLevel = m_pAbstractDef->GetLevel(nLevel);
        if (!pAbsLevel)
            continue;
        const StyleSheetEntryPtr pParaStyle = pAbsLevel->GetParaStyle();
        if (!pParaStyle)
            continue;
        const StyleSheetPropertyMap& rProps = *pParaStyle->m_pProperties;
        // In LO, the level's paraStyle outlineLevel always matches this listLevel.
        // An undefined listLevel is treated as the first level.
        sal_Int8 nListLevel = std::clamp<sal_Int8>(rProps.GetListLevel(), 0, 9);
        if (nListLevel != nLevel || rProps.GetOutlineLevel() != nLevel)
            return 0;
        else if (pAbsLevel->GetNumberingType(style::NumberingType::NUMBER_NONE)
                 != style::NumberingType::NUMBER_NONE)
        {
            // Arbitrarily chosen weighting factors - trying to round-trip LO choices if possible.
            // LibreOffice always saves Outline rule (usually containing heading styles) as numId 1.
            sal_uInt16 nWeightingFactor = GetId() == 1 ? 8 : 1;
            if (pParaStyle->m_sStyleIdentifierD.startsWith("Heading") )
                ++nWeightingFactor;
            nWeight += nWeightingFactor;
        }
    }
    return nWeight;
}

void ListDef::CreateNumberingRules( DomainMapper& rDMapper,
        rtl::Reference<SwXTextDocument> const& xTextDoc, sal_Int16 nOutline)
{
    // Get the UNO Numbering styles
    uno::Reference< container::XNameContainer > xStyles = lcl_getUnoNumberingStyles( xTextDoc );

    // Do the whole thing
    if( !(!m_xNumRules.is() && xTextDoc.is() && xStyles.is( )) )
        return;

    try
    {
        // Create the numbering style
        if (GetId() == nOutline)
            m_StyleName = "Outline"; //SwNumRule.GetOutlineRuleName()
        else
            xStyles->insertByName(
                GetStyleName(GetId(), xStyles),
                css::uno::Any(uno::Reference<css::style::XStyle>(xTextDoc->createNumberingStyle())));

        uno::Any oStyle = xStyles->getByName(GetStyleName());
        uno::Reference< beans::XPropertySet > xStyle( oStyle, uno::UNO_QUERY_THROW );

        // Get the default OOo Numbering style rules
        uno::Any aRules = xStyle->getPropertyValue( getPropertyName( PROP_NUMBERING_RULES ) );
        aRules >>= m_xNumRules;

        uno::Sequence<uno::Sequence<beans::PropertyValue>> aProps = GetMergedPropertyValues();

        sal_Int32 nAbstLevels = m_pAbstractDef ? m_pAbstractDef->Size() : 0;
        sal_Int32 nLevel = 0;
        while ( nLevel < nAbstLevels )
        {
            ListLevel::Pointer pAbsLevel = m_pAbstractDef->GetLevel( nLevel );
            ListLevel::Pointer pLevel = GetLevel( nLevel );

            // Get the merged level properties
            auto aLvlProps = comphelper::sequenceToContainer< std::vector<beans::PropertyValue> >(aProps[nLevel]);

            // Get the char style
            auto aAbsCharStyleProps = pAbsLevel
                                    ? pAbsLevel->GetCharStyleProperties()
                                    : uno::Sequence<beans::PropertyValue>();
            if ( pLevel )
            {
                uno::Sequence< beans::PropertyValue >& rAbsCharStyleProps = aAbsCharStyleProps;
                uno::Sequence< beans::PropertyValue > aCharStyleProps =
                    pLevel->GetCharStyleProperties( );
                uno::Sequence< beans::PropertyValue >& rCharStyleProps = aCharStyleProps;
                lcl_mergeProperties( rAbsCharStyleProps, rCharStyleProps );
            }

            // Change the sequence into a vector
            auto aStyleProps
                = comphelper::sequenceToContainer<PropertyValueVector_t>(aAbsCharStyleProps);

            //create (or find) a character style containing the character
            // attributes of the symbol and apply it to the numbering level
            OUString sStyle = rDMapper.getOrCreateCharStyle(aStyleProps, /*bAlwaysCreate=*/true);
            aLvlProps.push_back(
                comphelper::makePropertyValue(getPropertyName(PROP_CHAR_STYLE_NAME), sStyle));

            OUString sText = pAbsLevel
                           ? pAbsLevel->GetBulletChar()
                           : OUString();
            // Inherit <w:lvlText> from the abstract level in case the override would be empty.
            if (pLevel && pLevel->HasBulletChar())
                sText = pLevel->GetBulletChar( );

            aLvlProps.push_back(comphelper::makePropertyValue(getPropertyName(PROP_LIST_FORMAT), sText));

            aLvlProps.push_back(comphelper::makePropertyValue(getPropertyName(PROP_POSITION_AND_SPACE_MODE), sal_Int16(text::PositionAndSpaceMode::LABEL_ALIGNMENT)));

            // Replace the numbering rules for the level
            m_xNumRules->replaceByIndex(nLevel, uno::Any(comphelper::containerToSequence(aLvlProps)));

            // Handle the outline level here
            if (GetId() == nOutline && pAbsLevel && pAbsLevel->GetParaStyle())
            {
                uno::Reference< container::XIndexReplace > xOutlineRules =
                    xTextDoc->getChapterNumberingRules( );

                StyleSheetEntryPtr pParaStyle = pAbsLevel->GetParaStyle( );
                pParaStyle->m_bAssignedAsChapterNumbering = true;
                aLvlProps.push_back(comphelper::makePropertyValue(getPropertyName(PROP_HEADING_STYLE_NAME), pParaStyle->m_sConvertedStyleName));

                xOutlineRules->replaceByIndex(nLevel, uno::Any(comphelper::containerToSequence(aLvlProps)));
            }

            nLevel++;
        }

        // Create the numbering style for these rules
        const OUString & sNumRulesName = getPropertyName( PROP_NUMBERING_RULES );
        xStyle->setPropertyValue( sNumRulesName, uno::Any( m_xNumRules ) );
    }
    catch( const lang::IllegalArgumentException& )
    {
        TOOLS_WARN_EXCEPTION( "writerfilter", "" );
        assert( !"Incorrect argument to UNO call" );
    }
    catch( const uno::RuntimeException& )
    {
        TOOLS_WARN_EXCEPTION( "writerfilter", "" );
        assert( !"Incorrect argument to UNO call" );
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION( "writerfilter", "" );
    }

}

//-------------------------------------  NumberingManager implementation


ListsManager::ListsManager(DomainMapper& rDMapper,
    rtl::Reference<SwXTextDocument> xTextDoc)
    : LoggedProperties("ListsManager")
    , LoggedTable("ListsManager")
    , m_rDMapper(rDMapper)
    , m_xTextDoc(std::move(xTextDoc))
{
}

ListsManager::~ListsManager( )
{
    DisposeNumPicBullets();
}

void ListsManager::DisposeNumPicBullets( )
{
    uno::Reference<drawing::XShape> xShape;
    for (const auto& rNumPicBullet : m_aNumPicBullets)
    {
        xShape = rNumPicBullet->GetShape();
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
        OSL_ENSURE( m_pCurrentDefinition, "current entry has to be set here");
        if(!m_pCurrentDefinition)
            return ;
        pCurrentLvl = m_pCurrentDefinition->GetCurrentLevel( );
    }
    else
    {
        SAL_WARN_IF(!m_pCurrentNumPicBullet, "writerfilter", "current entry has to be set here");
        if (!m_pCurrentNumPicBullet)
            return;
    }
    int nIntValue = rVal.getInt();


    switch(nName)
    {
        case NS_ooxml::LN_CT_LevelText_val:
        {
            if(pCurrentLvl)
            {
                //if the BulletChar is a soft-hyphen (0xad)
                //replace it with a hard-hyphen (0x2d)
                //-> this fixes missing hyphen export in PDF etc.
                // see tdf#101626
                std::string sLevelText( rVal.getString().replace(0xad, 0x2d).toUtf8() );

                // DOCX level-text contains levels definition in format "%1.%2.%3"
                // we need to convert it to LO internal representation: "%1%.%2%.%3%"
                static const std::regex aTokenRegex("(%\\d)");
                sLevelText = std::regex_replace(sLevelText, aTokenRegex, "$1%");
                pCurrentLvl->SetBulletChar( OUString::fromUtf8(sLevelText) );
            }
        }
        break;
        case NS_ooxml::LN_CT_Lvl_start:
        case NS_ooxml::LN_CT_Lvl_numFmt:
        case NS_ooxml::LN_CT_NumFmt_format:
        case NS_ooxml::LN_CT_NumFmt_val:
        case NS_ooxml::LN_CT_Lvl_isLgl:
        case NS_ooxml::LN_CT_Lvl_legacy:
            if ( pCurrentLvl )
            {
                if (nName == NS_ooxml::LN_CT_NumFmt_format)
                {
                    pCurrentLvl->SetCustomNumberFormat(rVal.getString());
                }
                else
                {
                    pCurrentLvl->SetValue(nName, sal_Int32(nIntValue));
                }
            }
            break;
        case NS_ooxml::LN_CT_Num_numId:
            m_pCurrentDefinition->SetId( rVal.getString().toInt32( ) );
        break;
        case NS_ooxml::LN_CT_AbstractNum_nsid:
            m_pCurrentDefinition->SetId( nIntValue );
        break;
        case NS_ooxml::LN_CT_AbstractNum_tmpl:
            AbstractListDef::SetValue( nName );
        break;
        case NS_ooxml::LN_CT_NumLvl_ilvl:
            //add a new level to the level vector and make it the current one
            m_pCurrentDefinition->AddLevel(rVal.getString().toUInt32());
        break;
        case NS_ooxml::LN_CT_Lvl_ilvl:
            m_pCurrentDefinition->AddLevel(rVal.getString().toUInt32());
        break;
        case NS_ooxml::LN_CT_AbstractNum_abstractNumId:
        {
            // This one corresponds to the AbstractNum Id definition
            // The reference to the abstract num is in the sprm method
            sal_Int32 nVal = rVal.getString().toInt32();
            m_pCurrentDefinition->SetId( nVal );
        }
        break;
        case NS_ooxml::LN_CT_Ind_start:
        case NS_ooxml::LN_CT_Ind_left:
            if ( pCurrentLvl )
                pCurrentLvl->Insert(
                    PROP_INDENT_AT, uno::Any( ConversionHelper::convertTwipToMM100( nIntValue ) ));
            break;
        case NS_ooxml::LN_CT_Ind_hanging:
            if ( pCurrentLvl )
                pCurrentLvl->Insert(
                    PROP_FIRST_LINE_INDENT, uno::Any( - ConversionHelper::convertTwipToMM100( nIntValue ) ));
        break;
        case NS_ooxml::LN_CT_Ind_firstLine:
            if ( pCurrentLvl )
                pCurrentLvl->Insert(
                    PROP_FIRST_LINE_INDENT, uno::Any( ConversionHelper::convertTwipToMM100( nIntValue ) ));
        break;
        case NS_ooxml::LN_CT_Lvl_tplc: //template code - unsupported
        case NS_ooxml::LN_CT_Lvl_tentative: //marks level as unused in the document - unsupported
        break;
        case NS_ooxml::LN_CT_TabStop_pos:
        {
            //no paragraph attributes in ListTable char style sheets
            if ( pCurrentLvl )
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
    if( !(m_pCurrentDefinition ||
        nSprmId == NS_ooxml::LN_CT_Numbering_abstractNum ||
        nSprmId == NS_ooxml::LN_CT_Numbering_num ||
        (nSprmId == NS_ooxml::LN_CT_NumPicBullet_pict && m_pCurrentNumPicBullet) ||
        nSprmId == NS_ooxml::LN_CT_Numbering_numPicBullet))
        return;

    static bool bIsStartVisited = false;
    sal_Int32 nIntValue = rSprm.getValue()->getInt();
    switch( nSprmId )
    {
        case NS_ooxml::LN_CT_Numbering_abstractNum:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if(pProperties)
            {
                //create a new Abstract list entry
                OSL_ENSURE( !m_pCurrentDefinition, "current entry has to be NULL here");
                m_pCurrentDefinition = new AbstractListDef;
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
            if(pProperties)
            {
                // Create a new list entry
                OSL_ENSURE( !m_pCurrentDefinition, "current entry has to be NULL here");
                ListDef::Pointer listDef( new ListDef );
                m_pCurrentDefinition = listDef.get();
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
            if (pProperties)
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

            m_pCurrentNumPicBullet->SetShape(xShape);
        }
        break;
        case NS_ooxml::LN_CT_Lvl_lvlPicBulletId:
        if (ListLevel::Pointer pCurrentLevel = m_pCurrentDefinition->GetCurrentLevel())
        {
            uno::Reference<drawing::XShape> xShape;
            for (const auto& rNumPicBullet : m_aNumPicBullets)
            {
                if (rNumPicBullet->GetId() == nIntValue)
                {
                    xShape = rNumPicBullet->GetShape();
                    break;
                }
            }
            if (xShape.is())
            {
                uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
                try
                {
                    uno::Any aAny = xPropertySet->getPropertyValue("Graphic");
                    if (aAny.has<uno::Reference<graphic::XGraphic>>() && pCurrentLevel)
                    {
                        auto xGraphic = aAny.get<uno::Reference<graphic::XGraphic>>();
                        if (xGraphic.is())
                        {
                            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
                            pCurrentLevel->SetGraphicBitmap(xBitmap);
                        }
                    }
                }
                catch (const beans::UnknownPropertyException&)
                {}

                // Respect only the aspect ratio of the picture, not its size.
                awt::Size aPrefSize = xShape->getSize();
                if ( aPrefSize.Height * aPrefSize.Width != 0 )
                {
                    // See SwDefBulletConfig::InitFont(), default height is 14.
                    const int nFontHeight = 14;
                    // Point -> mm100.
                    const int nHeight = nFontHeight * 35;
                    int nWidth = (nHeight * aPrefSize.Width) / aPrefSize.Height;

                    awt::Size aSize( o3tl::toTwips(nWidth, o3tl::Length::mm100), o3tl::toTwips(nHeight, o3tl::Length::mm100) );
                    pCurrentLevel->SetGraphicSize( aSize );
                }
                else
                {
                    awt::Size aSize( o3tl::toTwips(aPrefSize.Width, o3tl::Length::mm100), o3tl::toTwips(aPrefSize.Height, o3tl::Length::mm100) );
                    pCurrentLevel->SetGraphicSize( aSize );
                }
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
            AbstractListDef::SetValue( nSprmId );
        break;
        case NS_ooxml::LN_CT_AbstractNum_lvl:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if(pProperties)
                pProperties->resolve(*this);
        }
        break;
        case NS_ooxml::LN_CT_Lvl_start:
            if (ListLevel::Pointer pCurrentLevel = m_pCurrentDefinition->GetCurrentLevel())
                pCurrentLevel->SetValue( nSprmId, nIntValue );
            bIsStartVisited = true;
        break;
        case NS_ooxml::LN_CT_Lvl_numFmt:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if (pProperties)
            {
                pProperties->resolve(*this);
            }
            if (ListLevel::Pointer pCurrentLevel = m_pCurrentDefinition->GetCurrentLevel())
            {
                if( !bIsStartVisited )
                {
                    pCurrentLevel->SetValue( NS_ooxml::LN_CT_Lvl_start, 0 );
                    bIsStartVisited = true;
                }
            }
        }
        break;
        case NS_ooxml::LN_CT_Lvl_isLgl:
        case NS_ooxml::LN_CT_Lvl_legacy:
            if (ListLevel::Pointer pCurrentLevel = m_pCurrentDefinition->GetCurrentLevel())
            {
                pCurrentLevel->SetValue(nSprmId, nIntValue);
            }
            break;
        case NS_ooxml::LN_CT_Lvl_suff:
        {
            if (ListLevel::Pointer pCurrentLevel = m_pCurrentDefinition->GetCurrentLevel())
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
                pCurrentLevel->SetValue( nSprmId, value );
            }
        }
        break;
        case NS_ooxml::LN_CT_Lvl_lvlText:
        case NS_ooxml::LN_CT_Lvl_rPr : //contains LN_EG_RPrBase_rFonts
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if(pProperties)
                pProperties->resolve(*this);
        }
        break;
        case NS_ooxml::LN_CT_NumLvl_lvl:
        {
            // overwrite level
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if(pProperties)
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
                if (ListLevel::Pointer pLevel = m_pCurrentDefinition->GetCurrentLevel())
                {
                    pLevel->Insert(
                        PROP_ADJUST, uno::Any( nValue ) );
                }
            }
        }
        break;
        case NS_ooxml::LN_CT_Lvl_pPr:
        case NS_ooxml::LN_CT_PPrBase_ind:
        {
            //todo: how to handle paragraph properties within numbering levels (except LeftIndent and FirstLineIndent)?
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if(pProperties)
                pProperties->resolve(*this);
        }
        break;
        case NS_ooxml::LN_CT_PPrBase_tabs:
        case NS_ooxml::LN_CT_Tabs_tab:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if(pProperties)
                pProperties->resolve(*this);
        }
        break;
        case NS_ooxml::LN_CT_Lvl_pStyle:
        {
            OUString sStyleName = rSprm.getValue( )->getString( );
            if (ListLevel::Pointer pLevel = m_pCurrentDefinition->GetCurrentLevel())
            {
                StyleSheetTablePtr pStylesTable = m_rDMapper.GetStyleSheetTable( );
                const StyleSheetEntryPtr pStyle = pStylesTable->FindStyleSheetByISTD( sStyleName );
                pLevel->SetParaStyle( pStyle );
            }
        }
        break;
        case NS_ooxml::LN_CT_Num_lvlOverride:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if (pProperties)
                pProperties->resolve(*this);
        }
        break;
        case NS_ooxml::LN_CT_NumLvl_startOverride:
        {
            if(m_pCurrentDefinition)
            {
                if (ListLevel::Pointer pCurrentLevel = m_pCurrentDefinition->GetCurrentLevel())
                {
                    pCurrentLevel->SetValue(NS_ooxml::LN_CT_NumLvl_startOverride, nIntValue);
                }
            }
        }
        break;
        case NS_ooxml::LN_CT_AbstractNum_numStyleLink:
        {
            OUString sStyleName = rSprm.getValue( )->getString( );
            m_pCurrentDefinition->SetNumStyleLink(sStyleName);
        }
        break;
        case NS_ooxml::LN_CT_AbstractNum_styleLink:
        {
            OUString sStyleName = rSprm.getValue()->getString();
            m_pCurrentDefinition->SetStyleLink(sStyleName);
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
            if (ListLevel::Pointer pCurrentLevel = m_pCurrentDefinition->GetCurrentLevel())
            {
                m_rDMapper.PushListProperties(pCurrentLevel.get());
                m_rDMapper.sprm( rSprm );
                m_rDMapper.PopListProperties();
            }
    }
}

void ListsManager::lcl_entry(const writerfilter::Reference<Properties>::Pointer_t& ref)
{
    if( m_rDMapper.IsOOXMLImport() || m_rDMapper.IsRTFImport() )
    {
        ref->resolve(*this);
    }
    else
    {
        // Create AbstractListDef's
        OSL_ENSURE( !m_pCurrentDefinition, "current entry has to be NULL here");
        m_pCurrentDefinition = new AbstractListDef( );
        ref->resolve(*this);
        //append it to the table
        m_aAbstractLists.push_back( m_pCurrentDefinition );
        m_pCurrentDefinition = AbstractListDef::Pointer();
    }
}

AbstractListDef::Pointer ListsManager::GetAbstractList( sal_Int32 nId )
{
    for (const auto& listDef : m_aAbstractLists)
    {
        if (listDef->GetId( ) == nId)
        {
            if (listDef->GetNumStyleLink().getLength() > 0)
            {
                // If the abstract num has a style linked, check the linked style's number id.
                StyleSheetTablePtr pStylesTable = m_rDMapper.GetStyleSheetTable( );

                const StyleSheetEntryPtr pStyleSheetEntry =
                    pStylesTable->FindStyleSheetByISTD(listDef->GetNumStyleLink() );

                const StyleSheetPropertyMap* pStyleSheetProperties =
                    pStyleSheetEntry ? pStyleSheetEntry->m_pProperties.get() : nullptr;

                if( pStyleSheetProperties && pStyleSheetProperties->props().GetListId() >= 0 )
                {
                    ListDef::Pointer pList = GetList( pStyleSheetProperties->props().GetListId() );
                    if ( pList!=nullptr )
                        return pList->GetAbstractDefinition();
                }

                // In stylesheet we did not found anything useful. Try to find base abstractnum having this stylelink
                for (const auto & baseListDef : m_aAbstractLists)
                {
                    if (baseListDef->GetStyleLink() == listDef->GetNumStyleLink())
                    {
                        return baseListDef;
                    }
                }
            }

            // Standalone abstract list
            return listDef;
        }
    }

    return nullptr;
}

ListDef::Pointer ListsManager::GetList( sal_Int32 nId )
{
    ListDef::Pointer pList;
    if (nId == -1)
        return pList;

    int nLen = m_aLists.size( );
    int i = 0;
    while ( !pList && i < nLen )
    {
        if ( m_aLists[i]->GetId( ) == nId )
            pList = m_aLists[i];
        i++;
    }

    // nId 0 is only valid for abstractNum, not numId (which has an abstract definition)
    assert(!pList || nId || !pList->GetAbstractDefinition() || m_rDMapper.IsRTFImport());

    return pList;
}

void ListsManager::CreateNumberingRules( )
{
    // Try to determine which numId would best work as LO's Chapter Numbering Outline rule.
    // (The best fix for many import bugs is just to prevent ANY assignment as chapter numbering.)
    sal_Int16 nChosenAsChapterNumberingId = -1;
    sal_uInt16 nHighestWeight = 5; // arbitrarily chosen minimum threshold
    for (const auto& rList : m_aLists)
    {
        sal_uInt16 nWeight = rList->GetChapterNumberingWeight();
        if (nWeight > nHighestWeight)
        {
            nHighestWeight = nWeight;
            nChosenAsChapterNumberingId = rList->GetId();
            //Optimization: if the weight cannot be beaten anymore, then quit early
            if (nHighestWeight > 17)
                break;
        }
    }

    // Loop over the definitions
    for ( const auto& rList : m_aLists )
    {
        rList->CreateNumberingRules(m_rDMapper, m_xTextDoc, nChosenAsChapterNumberingId);
    }
    m_rDMapper.GetStyleSheetTable()->ReApplyInheritedOutlineLevelFromChapterNumbering();
    m_rDMapper.GetStyleSheetTable()->ApplyNumberingStyleNameToParaStyles();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
