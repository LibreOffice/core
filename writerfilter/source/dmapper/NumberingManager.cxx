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
#include "ConversionHelper.hxx"
#include "NumberingManager.hxx"
#include "StyleSheetTable.hxx"
#include "PropertyIds.hxx"

#include <doctok/resourceids.hxx>
#include <doctok/sprmids.hxx>
#include <ooxml/resourceids.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/PositionAndSpaceMode.hpp>
#include <com/sun/star/text/XChapterNumberingSupplier.hpp>

#include <rtl/ustring.hxx>

#include "dmapperLoggers.hxx"

using namespace com::sun::star;

#define MAKE_PROPVAL(NameId, Value) \
    beans::PropertyValue(aPropNameSupplier.GetName(NameId), 0, uno::makeAny(Value), beans::PropertyState_DIRECT_VALUE )

#define NUMBERING_MAX_LEVELS    10


namespace writerfilter {
namespace dmapper {

//---------------------------------------------------  Utility functions

void lcl_printProperties( uno::Sequence< beans::PropertyValue > aProps )
{
    sal_Int32 nLen = aProps.getLength( );
    for ( sal_Int32 i = 0; i < nLen; i++ )
    {
        uno::Any aValue = aProps[i].Value;
        sal_Int32 nValue = 0;
        OUString sValue;

        if ( !( aValue >>= sValue ) && ( aValue >>= nValue ) )
            sValue = OUString::valueOf( nValue );

        SAL_INFO("writerfilter", "Property " << aProps[i].Name << ": " << sValue);
    }
}

sal_Int32 lcl_findProperty( uno::Sequence< beans::PropertyValue > aProps, OUString sName )
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
        case NS_rtf::LN_ISTARTAT:
            m_nIStartAt = nValue;
        break;
        case NS_rtf::LN_NFC:
            m_nNFC = nValue;
        break;
        case NS_rtf::LN_JC:
            m_nJC = nValue;
        break;
        case NS_rtf::LN_FLEGAL:
            m_nFLegal = nValue;
        break;
        case NS_rtf::LN_FNORESTART:
            m_nFNoRestart = nValue;
        break;
        case NS_rtf::LN_FIDENTSAV:
            m_nFPrev = nValue;
        break;
        case NS_rtf::LN_FCONVERTED:
            m_nFPrevSpace = nValue;
        break;
        case NS_rtf::LN_IXCHFOLLOW:
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

void ListLevel::SetParaStyle( boost::shared_ptr< StyleSheetEntry > pStyle )
{
    if (!pStyle)
        return;
    m_pParaStyle = pStyle;
    // AFAICT .docx spec does not identify which numberings or paragraph
    // styles are actually the ones to be used for outlines (chapter numbering),
    // it only kind of says somewhere that they should be named Heading1 to Heading9.
    const OUString styleId = pStyle->sStyleIdentifierD;
    m_outline = ( styleId.getLength() == RTL_CONSTASCII_LENGTH( "Heading1" )
        && styleId.match( "Heading", 0 )
        && styleId[ RTL_CONSTASCII_LENGTH( "Heading" ) ] >= '1'
        && styleId[ RTL_CONSTASCII_LENGTH( "Heading" ) ] <= '9' );
}

sal_Int16 ListLevel::GetParentNumbering( OUString sText, sal_Int16 nLevel,
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
            sal_Unicode cLevel = sLevelText.getStr()[1];
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

uno::Sequence< beans::PropertyValue > ListLevel::GetCharStyleProperties( )
{
    PropertyValueVector_t rProperties;
    PropertyNameSupplier& aPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();

    _PropertyMap::const_iterator aMapIter = begin();
    _PropertyMap::const_iterator aEndIter = end();
    for( ; aMapIter != aEndIter; ++aMapIter )
    {
        switch( aMapIter->first.eId )
        {
            case PROP_ADJUST:
            case PROP_INDENT_AT:
            case PROP_FIRST_LINE_INDENT:
            case PROP_FIRST_LINE_OFFSET:
            case PROP_LEFT_MARGIN:
            case PROP_CHAR_FONT_NAME:
                // Do nothing: handled in the GetPropertyValues method
            break;
            default:
            {
                rProperties.push_back(
                        beans::PropertyValue(
                            aPropNameSupplier.GetName( aMapIter->first.eId ), 0,
                            aMapIter->second, beans::PropertyState_DIRECT_VALUE ));
            }
        }
    }

    uno::Sequence< beans::PropertyValue > aRet( rProperties.size() );
    beans::PropertyValue* pValues = aRet.getArray();
    PropertyValueVector_t::const_iterator aIt = rProperties.begin();
    PropertyValueVector_t::const_iterator aEndIt = rProperties.end();
    for(sal_uInt32 nIndex = 0; aIt != aEndIt; ++aIt,++nIndex)
    {
        pValues[nIndex] = *aIt;
    }
    return aRet;
}

uno::Sequence< beans::PropertyValue > ListLevel::GetLevelProperties( )
{
    const sal_Int16 aWWToUnoAdjust[] =
    {
        text::HoriOrientation::LEFT,
        text::HoriOrientation::CENTER,
        text::HoriOrientation::RIGHT,
    };

    PropertyNameSupplier& aPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
    PropertyValueVector_t aNumberingProperties;

    if( m_nIStartAt >= 0)
        aNumberingProperties.push_back( MAKE_PROPVAL(PROP_START_WITH, (sal_Int16)m_nIStartAt) );

    sal_Int16 nNumberFormat = ConversionHelper::ConvertNumberingType(m_nNFC);
    if( m_nNFC >= 0)
    {
        if (!m_sGraphicURL.isEmpty() || m_sGraphicBitmap.is())
            nNumberFormat = style::NumberingType::BITMAP;
        aNumberingProperties.push_back( MAKE_PROPVAL(PROP_NUMBERING_TYPE, nNumberFormat ));
    }

    if( m_nJC >= 0 && m_nJC <= sal::static_int_cast<sal_Int32>(sizeof(aWWToUnoAdjust) / sizeof(sal_Int16)) )
        aNumberingProperties.push_back( MAKE_PROPVAL(PROP_ADJUST, aWWToUnoAdjust[m_nJC]));

    if( !isOutlineNumbering())
    {
        // todo: this is not the bullet char
        if( nNumberFormat == style::NumberingType::CHAR_SPECIAL )
        {
            if (!m_sBulletChar.isEmpty())
            {
                aNumberingProperties.push_back( MAKE_PROPVAL(PROP_BULLET_CHAR, m_sBulletChar.copy(0,1)));
            }
            else
            {
                // If w:lvlText's value is null - set bullet char to zero.
                aNumberingProperties.push_back( MAKE_PROPVAL(PROP_BULLET_CHAR, sal_Unicode(0x0)));
            }
        }
        if (!m_sGraphicURL.isEmpty())
            aNumberingProperties.push_back(MAKE_PROPVAL(PROP_GRAPHIC_URL, m_sGraphicURL));
        if (m_sGraphicBitmap.is())
            aNumberingProperties.push_back(MAKE_PROPVAL(PROP_GRAPHIC_BITMAP, m_sGraphicBitmap));
    }

    aNumberingProperties.push_back( MAKE_PROPVAL( PROP_LISTTAB_STOP_POSITION, m_nTabstop ) );

    //TODO: handling of nFLegal?
    //TODO: nFNoRestart lower levels do not restart when higher levels are incremented, like:
    //1.
    //1.1
    //2.2
    //2.3
    //3.4
    //

    if( m_nFWord6 > 0) //Word 6 compatibility
    {
        if( m_nFPrev == 1)
            aNumberingProperties.push_back( MAKE_PROPVAL( PROP_PARENT_NUMBERING, (sal_Int16) NUMBERING_MAX_LEVELS ));
        //TODO: prefixing space     nFPrevSpace;     - has not been used in WW8 filter
    }

//    TODO: sRGBXchNums;     array of inherited numbers

//  nXChFollow; following character 0 - tab, 1 - space, 2 - nothing
    aNumberingProperties.push_back( MAKE_PROPVAL( PROP_LEVEL_FOLLOW, m_nXChFollow ));


    _PropertyMap::const_iterator aMapIter = begin();
    _PropertyMap::const_iterator aEndIter = end();
    for( ; aMapIter != aEndIter; ++aMapIter )
    {
        switch( aMapIter->first.eId )
        {
            case PROP_ADJUST:
            case PROP_INDENT_AT:
            case PROP_FIRST_LINE_INDENT:
            case PROP_FIRST_LINE_OFFSET:
            case PROP_LEFT_MARGIN:
                aNumberingProperties.push_back(
                    beans::PropertyValue( aPropNameSupplier.GetName( aMapIter->first.eId ), 0, aMapIter->second, beans::PropertyState_DIRECT_VALUE ));
            break;
            case PROP_CHAR_FONT_NAME:
                if( !isOutlineNumbering())
                {
                    aNumberingProperties.push_back(
                        beans::PropertyValue( aPropNameSupplier.GetName( PROP_BULLET_FONT_NAME ), 0, aMapIter->second, beans::PropertyState_DIRECT_VALUE ));
                }
            break;
            default:
            {
                // Handled in GetCharStyleProperties method
            }

        }
    }

    uno::Sequence< beans::PropertyValue > aRet(aNumberingProperties.size());
    beans::PropertyValue* pValues = aRet.getArray();
    PropertyValueVector_t::const_iterator aIt = aNumberingProperties.begin();
    PropertyValueVector_t::const_iterator aEndIt = aNumberingProperties.end();
    for(sal_uInt32 nIndex = 0; aIt != aEndIt; ++aIt,++nIndex)
    {
        pValues[nIndex] = *aIt;
    }
    return aRet;
}

// Add the properties only if they do not already exist in the sequence.
void ListLevel::AddParaProperties( uno::Sequence< beans::PropertyValue >* props )
{
    uno::Sequence< beans::PropertyValue >& aProps = *props;
    PropertyNameSupplier& aPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();

    OUString sFirstLineIndent = aPropNameSupplier.GetName(
            PROP_FIRST_LINE_INDENT );
    OUString sIndentAt = aPropNameSupplier.GetName(
            PROP_INDENT_AT );

    bool hasFirstLineIndent = lcl_findProperty( aProps, sFirstLineIndent );
    bool hasIndentAt = lcl_findProperty( aProps, sIndentAt );

    if( hasFirstLineIndent && hasIndentAt )
        return; // has them all, nothing to add

    uno::Sequence< beans::PropertyValue > aParaProps = m_pParaStyle->pProperties->GetPropertyValues( );

    // ParaFirstLineIndent -> FirstLineIndent
    // ParaLeftMargin -> IndentAt

    OUString sParaIndent = aPropNameSupplier.GetName(
            PROP_PARA_FIRST_LINE_INDENT );
    OUString sParaLeftMargin = aPropNameSupplier.GetName(
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

void NumPicBullet::SetShape(uno::Reference<drawing::XShape> xShape)
{
    m_xShape = xShape;
}

sal_Int32 NumPicBullet::GetId()
{
    return m_nId;
}

uno::Reference<drawing::XShape> NumPicBullet::GetShape()
{
    return m_xShape;
}

//--------------------------------------- AbstractListDef implementation

AbstractListDef::AbstractListDef( ) :
    m_nTPLC( -1 )
    ,m_nSimpleList( -1 )
    ,m_nRestart( -1 )
    ,m_nUnsigned( -1 )
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
        case NS_rtf::LN_TPLC:
            m_nTPLC = nValue;
        break;
        case NS_rtf::LN_FSIMPLELIST:
            m_nSimpleList = nValue;
        break;
        case NS_rtf::LN_fAutoNum:
            m_nRestart = nValue;
        break;
        case NS_rtf::LN_fHybrid:
            m_nUnsigned = nValue;
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
    sStyleName += OUString::valueOf( nId );

    return sStyleName;
}

uno::Sequence< uno::Sequence< beans::PropertyValue > > ListDef::GetPropertyValues( )
{
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
       uno::Reference< lang::XMultiServiceFactory > xFactory )
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
        uno::Reference< lang::XMultiServiceFactory> xFactory )
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

            PropertyNameSupplier& aPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();

            // Get the default OOo Numbering style rules
            uno::Any aRules = xStyle->getPropertyValue( aPropNameSupplier.GetName( PROP_NUMBERING_RULES ) );
            aRules >>= m_xNumRules;

            uno::Sequence< uno::Sequence< beans::PropertyValue > > aProps = GetPropertyValues( );

            sal_Int32 nAbstLevels = m_pAbstractDef->Size( );
            sal_Int16 nLevel = 0;
            while ( nLevel < nAbstLevels )
            {
                ListLevel::Pointer pAbsLevel = m_pAbstractDef->GetLevel( nLevel );
                ListLevel::Pointer pLevel = GetLevel( nLevel );

                // Get the merged level properties
                uno::Sequence< beans::PropertyValue > aLvlProps = aProps[sal_Int32( nLevel )];

#if OSL_DEBUG_LEVEL > 1
                lcl_printProperties( aLvlProps );
#endif

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
                    OUString sStyle = rDMapper.getOrCreateCharStyle( aStyleProps );
                    aLvlProps.realloc( aLvlProps.getLength() + 1);
                    aLvlProps[sal::static_int_cast<sal_uInt32>(aLvlProps.getLength()) - 1].Name = aPropNameSupplier.GetName( PROP_CHAR_STYLE_NAME );
                    aLvlProps[sal::static_int_cast<sal_uInt32>(aLvlProps.getLength()) - 1].Value <<= sStyle;
                }

                // Get the prefix / suffix / Parent numbering
                // and add them to the level properties
                OUString sText = pAbsLevel->GetBulletChar( );
                if ( pLevel.get( ) )
                    sText = pLevel->GetBulletChar( );

                OUString sPrefix;
                OUString sSuffix;
                OUString& rPrefix = sPrefix;
                OUString& rSuffix = sSuffix;
                sal_Int16 nParentNum = ListLevel::GetParentNumbering(
                       sText, nLevel, rPrefix, rSuffix );

                aLvlProps.realloc( aLvlProps.getLength( ) + 4 );
                aLvlProps[sal::static_int_cast<sal_uInt32>(aLvlProps.getLength()) - 4] = MAKE_PROPVAL( PROP_PREFIX, rPrefix );
                aLvlProps[sal::static_int_cast<sal_uInt32>(aLvlProps.getLength()) - 3] = MAKE_PROPVAL( PROP_SUFFIX, rSuffix );
                aLvlProps[sal::static_int_cast<sal_uInt32>(aLvlProps.getLength()) - 2] = MAKE_PROPVAL( PROP_PARENT_NUMBERING, nParentNum );

                aLvlProps[sal::static_int_cast<sal_uInt32>(aLvlProps.getLength()) - 1] = MAKE_PROPVAL( PROP_POSITION_AND_SPACE_MODE,
                            sal_Int16( text::PositionAndSpaceMode::LABEL_ALIGNMENT ) );

                StyleSheetEntryPtr pParaStyle = pAbsLevel->GetParaStyle( );
                if( pParaStyle.get())
                {
                    aLvlProps.realloc( aLvlProps.getLength() + 1 );
                    aLvlProps[sal::static_int_cast<sal_uInt32>(aLvlProps.getLength()) - 1] = MAKE_PROPVAL( PROP_PARAGRAPH_STYLE_NAME,
                        pParaStyle->sConvertedStyleName );
                }

                // Replace the numbering rules for the level
                m_xNumRules->replaceByIndex( nLevel, uno::makeAny( aLvlProps ) );

                // Handle the outline level here
                if ( pAbsLevel->isOutlineNumbering())
                {
                    uno::Reference< text::XChapterNumberingSupplier > xOutlines (
                        xFactory, uno::UNO_QUERY_THROW );
                    uno::Reference< container::XIndexReplace > xOutlineRules =
                        xOutlines->getChapterNumberingRules( );

                    aLvlProps.realloc( aLvlProps.getLength() + 1 );
                    aLvlProps[sal::static_int_cast<sal_uInt32>(aLvlProps.getLength()) - 1] = MAKE_PROPVAL( PROP_HEADING_STYLE_NAME,
                        pParaStyle->sConvertedStyleName );

                    xOutlineRules->replaceByIndex( nLevel, uno::makeAny( aLvlProps ) );
                }

                nLevel++;
            }

            // Create the numbering style for these rules
            OUString sNumRulesName = aPropNameSupplier.GetName( PROP_NUMBERING_RULES );
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
                           const uno::Reference< lang::XMultiServiceFactory > xFactory) :
LoggedProperties(dmapper_logger, "ListsManager"),
LoggedTable(dmapper_logger, "ListsManager"),
m_rDMapper( rDMapper ),
m_xFactory( xFactory )
{
}

ListsManager::~ListsManager( )
{
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
        case NS_rtf::LN_RGBXCHNUMS:
            if(pCurrentLvl.get())
                pCurrentLvl->AddRGBXchNums( rVal.getString( ) );
        break;
        case NS_ooxml::LN_CT_LevelText_val:
        {
            //this strings contains the definition of the level
            //the level number is marked as %n
            //these numbers can be mixed randomly toghether with separators pre- and suffixes
            //the Writer supports only a number of upper levels to show, separators is always a dot
            //and each level can have a prefix and a suffix
            if(pCurrentLvl.get())
                pCurrentLvl->SetBulletChar( rVal.getString() );
        }
        break;
        case NS_rtf::LN_ISTARTAT:
        case NS_rtf::LN_NFC:
        case NS_rtf::LN_JC:
        case NS_rtf::LN_FLEGAL:
        case NS_rtf::LN_FNORESTART:
        case NS_rtf::LN_FIDENTSAV:
        case NS_rtf::LN_FCONVERTED:
        case NS_rtf::LN_IXCHFOLLOW:
            if ( pCurrentLvl.get( ) )
                pCurrentLvl->SetValue( nName, sal_Int32( nIntValue ) );
        break;
        case NS_rtf::LN_RGISTD:
            m_pCurrentDefinition->AddRGISTD( rVal.getString() );
        break;
        case NS_ooxml::LN_CT_Num_numId:
            m_pCurrentDefinition->SetId( rVal.getString().toInt32( ) );
        break;
        case NS_rtf::LN_LSID:
            m_pCurrentDefinition->SetId( nIntValue );
        break;
        case NS_rtf::LN_TPLC:
        case NS_rtf::LN_FSIMPLELIST:
        case NS_rtf::LN_fAutoNum:
        case NS_rtf::LN_fHybrid:
            m_pCurrentDefinition->SetValue( nName, nIntValue );
        break;
        case NS_ooxml::LN_CT_NumLvl_ilvl:
        case NS_rtf::LN_LISTLEVEL:
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
        {
#if OSL_DEBUG_LEVEL > 0
            OString sMessage( "ListTable::attribute() - Id: ");
            sMessage += OString::valueOf( sal_Int32( nName ), 10 );
            sMessage += " / 0x";
            sMessage += OString::valueOf( sal_Int32( nName ), 16 );
            sMessage += " value: ";
            sMessage += OString::valueOf( sal_Int32( nIntValue ), 10 );
            sMessage += " / 0x";
            sMessage += OString::valueOf( sal_Int32( nIntValue ), 16 );
            SAL_WARN("writerfilter", sMessage.getStr());
#endif
        }
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
                    uno::Reference<beans::XPropertySetInfo> info = xPropertySet->getPropertySetInfo();
                    uno::Sequence<beans::Property> properties = info->getProperties();
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
                if ( pListDef != NULL )
                {
                    // The current def should be a ListDef
                    pListDef->SetAbstractDefinition(
                           GetAbstractList( nAbstractNumId ) );
                }
            }
            break;
            case NS_ooxml::LN_CT_AbstractNum_multiLevelType:
            break;
            case NS_rtf::LN_TPLC:
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
            case NS_rtf::LN_RGBXCHNUMS: break;
            case NS_rtf::LN_ISTARTAT:
            case NS_rtf::LN_NFC:
            case NS_rtf::LN_JC:
            case NS_rtf::LN_FLEGAL:
            case NS_rtf::LN_FNORESTART:
            case NS_rtf::LN_FIDENTSAV:
            case NS_rtf::LN_FCONVERTED:
            case NS_rtf::LN_IXCHFOLLOW:
                if (m_pCurrentDefinition->GetCurrentLevel().get())
                    m_pCurrentDefinition->GetCurrentLevel( )->SetValue( nSprmId, nIntValue );
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
                static const sal_Int16 aWWAlignments[ ] =
                {
                    text::HoriOrientation::LEFT,
                    text::HoriOrientation::CENTER,
                    text::HoriOrientation::RIGHT
                };
                m_pCurrentDefinition->GetCurrentLevel( )->Insert(
                    PROP_ADJUST, uno::makeAny( aWWAlignments[ nIntValue ] ) );
                    writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
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
            case NS_sprm::LN_CHps:    // sprmCHps
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
        if ( m_bIsLFOImport )
        {
            // Create ListDef's
            OSL_ENSURE( !m_pCurrentDefinition.get(), "current entry has to be NULL here");
            ListDef::Pointer pList( new ListDef() );
            m_pCurrentDefinition = pList;
            ref->resolve(*this);
            //append it to the table
            m_aLists.push_back( pList );
            m_pCurrentDefinition = AbstractListDef::Pointer();
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
                    dynamic_cast<const StyleSheetPropertyMap*>(pStyleSheetEntry ? pStyleSheetEntry->pProperties.get() : 0);

                if( pStyleSheetProperties && pStyleSheetProperties->GetNumId() >= 0 )
                {
                    ListDef::Pointer pList = GetList( pStyleSheetProperties->GetNumId() );
                    if ( pList!=0 )
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
