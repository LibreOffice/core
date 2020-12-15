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
#include "PropertyMap.hxx"
#include "TagLogger.hxx"
#include <ooxml/resourceids.hxx>
#include "DomainMapper_Impl.hxx"
#include "ConversionHelper.hxx"
#include <editeng/boxitem.hxx>
#include <i18nutil/paper.hxx>
#include <osl/diagnose.h>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/HorizontalAdjust.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XRedline.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/TextGridMode.hpp>
#include <com/sun/star/text/XTextCopy.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <tools/diagnose_ex.h>
#include "PropertyMapHelper.hxx"
#include <o3tl/sorted_vector.hxx>

using namespace com::sun::star;

namespace writerfilter::dmapper {

uno::Sequence< beans::PropertyValue > PropertyMap::GetPropertyValues( bool bCharGrabBag )
{
    using comphelper::makePropertyValue;

    if ( m_aValues.empty() && !m_vMap.empty() )
    {
        size_t nCharGrabBag = 0;
        size_t nParaGrabBag = 0;
        size_t nCellGrabBag = 0;
        size_t nRowGrabBag  = 0;

        const PropValue* pParaStyleProp = nullptr;
        const PropValue* pCharStyleProp = nullptr;
        const PropValue* pNumRuleProp   = nullptr;

        for ( const auto& rPropPair : m_vMap )
        {
            if ( rPropPair.second.getGrabBagType() == CHAR_GRAB_BAG )
                nCharGrabBag++;
            else if ( rPropPair.second.getGrabBagType() == PARA_GRAB_BAG )
                nParaGrabBag++;
            else if ( rPropPair.second.getGrabBagType() == CELL_GRAB_BAG )
                nCellGrabBag++;
            else if ( rPropPair.first == PROP_CELL_INTEROP_GRAB_BAG )
            {
                uno::Sequence< beans::PropertyValue > aSeq;
                rPropPair.second.getValue() >>= aSeq;
                nCellGrabBag += aSeq.getLength();
            }
            else if ( rPropPair.second.getGrabBagType() == ROW_GRAB_BAG )
                nRowGrabBag++;

            if ( rPropPair.first == PROP_PARA_STYLE_NAME ) pParaStyleProp = &rPropPair.second;
            if ( rPropPair.first == PROP_CHAR_STYLE_NAME ) pCharStyleProp = &rPropPair.second;
            if ( rPropPair.first == PROP_NUMBERING_RULES ) pNumRuleProp   = &rPropPair.second;
        }

        // Style names have to be the first elements within the property sequence
        // otherwise they will overwrite 'hard' attributes
        if ( pParaStyleProp != nullptr )
            m_aValues.push_back( makePropertyValue( getPropertyName( PROP_PARA_STYLE_NAME ), pParaStyleProp->getValue() ) );
        if ( pCharStyleProp != nullptr )
            m_aValues.push_back( makePropertyValue( getPropertyName( PROP_CHAR_STYLE_NAME ), pCharStyleProp->getValue() ) );
        if ( pNumRuleProp != nullptr )
            m_aValues.push_back( makePropertyValue(getPropertyName( PROP_NUMBERING_RULES ), pNumRuleProp->getValue() ) );

        // If there are any grab bag properties, we need one slot for them.
        uno::Sequence< beans::PropertyValue > aCharGrabBagValues( nCharGrabBag );
        uno::Sequence< beans::PropertyValue > aParaGrabBagValues( nParaGrabBag );
        uno::Sequence< beans::PropertyValue > aCellGrabBagValues( nCellGrabBag );
        uno::Sequence< beans::PropertyValue > aRowGrabBagValues ( nRowGrabBag );
        beans::PropertyValue* pCharGrabBagValues = aCharGrabBagValues.getArray();
        beans::PropertyValue* pParaGrabBagValues = aParaGrabBagValues.getArray();
        beans::PropertyValue* pCellGrabBagValues = aCellGrabBagValues.getArray();
        beans::PropertyValue* pRowGrabBagValues  = aRowGrabBagValues.getArray();
        // Record index for the next property to be added in each grab bag.
        sal_Int32 nRowGrabBagValue  = 0;
        sal_Int32 nCellGrabBagValue = 0;
        sal_Int32 nParaGrabBagValue = 0;
        sal_Int32 nCharGrabBagValue = 0;

        for ( const auto& rPropPair : m_vMap )
        {
            if ( rPropPair.first != PROP_PARA_STYLE_NAME &&
                 rPropPair.first != PROP_CHAR_STYLE_NAME &&
                 rPropPair.first != PROP_NUMBERING_RULES )
            {
                if ( rPropPair.second.getGrabBagType() == CHAR_GRAB_BAG )
                {
                    if ( bCharGrabBag )
                    {
                        pCharGrabBagValues[nCharGrabBagValue].Name  = getPropertyName( rPropPair.first );
                        pCharGrabBagValues[nCharGrabBagValue].Value = rPropPair.second.getValue();
                        ++nCharGrabBagValue;
                    }
                }
                else if ( rPropPair.second.getGrabBagType() == PARA_GRAB_BAG )
                {
                    pParaGrabBagValues[nParaGrabBagValue].Name  = getPropertyName( rPropPair.first );
                    pParaGrabBagValues[nParaGrabBagValue].Value = rPropPair.second.getValue();
                    ++nParaGrabBagValue;
                }
                else if ( rPropPair.second.getGrabBagType() == CELL_GRAB_BAG )
                {
                    pCellGrabBagValues[nCellGrabBagValue].Name  = getPropertyName( rPropPair.first );
                    pCellGrabBagValues[nCellGrabBagValue].Value = rPropPair.second.getValue();
                    ++nCellGrabBagValue;
                }
                else if ( rPropPair.second.getGrabBagType() == ROW_GRAB_BAG )
                {
                    pRowGrabBagValues[nRowGrabBagValue].Name  = getPropertyName( rPropPair.first );
                    pRowGrabBagValues[nRowGrabBagValue].Value = rPropPair.second.getValue();
                    ++nRowGrabBagValue;
                }
                else if ( rPropPair.first == PROP_CELL_INTEROP_GRAB_BAG )
                {
                    uno::Sequence< beans::PropertyValue > aSeq;
                    rPropPair.second.getValue() >>= aSeq;
                    std::copy(aSeq.begin(), aSeq.end(), pCellGrabBagValues + nCellGrabBagValue);
                    nCellGrabBagValue += aSeq.getLength();
                }
                else
                {
                    m_aValues.push_back( makePropertyValue( getPropertyName( rPropPair.first ), rPropPair.second.getValue() ) );
                }
            }
        }

        if ( nCharGrabBag && bCharGrabBag )
            m_aValues.push_back( makePropertyValue( "CharInteropGrabBag", uno::makeAny( aCharGrabBagValues ) ) );

        if ( nParaGrabBag )
            m_aValues.push_back( makePropertyValue( "ParaInteropGrabBag", uno::makeAny( aParaGrabBagValues ) ) );

        if ( nCellGrabBag )
            m_aValues.push_back( makePropertyValue( "CellInteropGrabBag", uno::makeAny( aCellGrabBagValues ) ) );

        if ( nRowGrabBag )
            m_aValues.push_back( makePropertyValue( "RowInteropGrabBag", uno::makeAny( aRowGrabBagValues ) ) );
    }

    return comphelper::containerToSequence( m_aValues );
}

std::vector< PropertyIds > PropertyMap::GetPropertyIds()
{
    std::vector< PropertyIds > aRet;
    for ( const auto& rPropPair : m_vMap )
        aRet.push_back( rPropPair.first );
    return aRet;
}

#ifdef DBG_UTIL
static void lcl_AnyToTag( const uno::Any& rAny )
{
    try {
        sal_Int32 aInt = 0;
        if ( rAny >>= aInt )
        {
            TagLogger::getInstance().attribute( "value", rAny );
        }
        else
        {
            TagLogger::getInstance().attribute( "unsignedValue", 0 );
        }

        sal_uInt32 auInt = 0;
        rAny >>= auInt;
        TagLogger::getInstance().attribute( "unsignedValue", auInt );

        float aFloat = 0.0f;
        if ( rAny >>= aFloat )
        {
            TagLogger::getInstance().attribute( "floatValue", rAny );
        }
        else
        {
            TagLogger::getInstance().attribute( "unsignedValue", 0 );
        }

        OUString aStr;
        rAny >>= aStr;
        TagLogger::getInstance().attribute( "stringValue", aStr );
    }
    catch ( ... )
    {
    }
}
#endif

void PropertyMap::Insert( PropertyIds eId, const uno::Any& rAny, bool bOverwrite, GrabBagType i_GrabBagType, bool bDocDefault )
{
#ifdef DBG_UTIL
    const OUString& rInsert = getPropertyName(eId);

    TagLogger::getInstance().startElement("propertyMap.insert");
    TagLogger::getInstance().attribute("name", rInsert);
    lcl_AnyToTag(rAny);
    TagLogger::getInstance().endElement();
#endif

    if ( !bOverwrite )
        m_vMap.insert(std::make_pair(eId, PropValue(rAny, i_GrabBagType, bDocDefault)));
    else
        m_vMap[eId] = PropValue(rAny, i_GrabBagType);

    Invalidate();
}

void PropertyMap::Erase( PropertyIds eId )
{
    // Safe call to erase, it throws no exceptions, even if eId is not in m_vMap
    m_vMap.erase(eId);

    Invalidate();
}

std::optional< PropertyMap::Property > PropertyMap::getProperty( PropertyIds eId ) const
{
    std::map< PropertyIds, PropValue >::const_iterator aIter = m_vMap.find( eId );
    if ( aIter == m_vMap.end() )
        return std::optional<Property>();
    else
        return std::make_pair( eId, aIter->second.getValue() );
}

bool PropertyMap::isSet( PropertyIds eId) const
{
    return m_vMap.find( eId ) != m_vMap.end();
}

bool PropertyMap::isDocDefault( PropertyIds eId ) const
{
    std::map< PropertyIds, PropValue >::const_iterator aIter = m_vMap.find( eId );
    if ( aIter == m_vMap.end() )
        return false;
    else
        return aIter->second.getIsDocDefault();
}

#ifdef DBG_UTIL
void PropertyMap::dumpXml() const
{
    TagLogger::getInstance().startElement( "PropertyMap" );

    for ( const auto& rPropPair : m_vMap )
    {
        TagLogger::getInstance().startElement( "property" );

        TagLogger::getInstance().attribute( "name", getPropertyName( rPropPair.first ) );

        switch ( rPropPair.first )
        {
            case PROP_TABLE_COLUMN_SEPARATORS:
                lcl_DumpTableColumnSeparators( rPropPair.second.getValue() );
                break;
            default:
            {
                try
                {
                    sal_Int32 aInt = 0;
                    rPropPair.second.getValue() >>= aInt;
                    TagLogger::getInstance().attribute( "value", aInt );

                    sal_uInt32 auInt = 0;
                    rPropPair.second.getValue() >>= auInt;
                    TagLogger::getInstance().attribute( "unsignedValue", auInt );

                    float aFloat = 0.0;
                    rPropPair.second.getValue() >>= aFloat;
                    TagLogger::getInstance().attribute( "floatValue", aFloat );

                    rPropPair.second.getValue() >>= auInt;
                    TagLogger::getInstance().attribute( "stringValue", OUString() );
                }
                catch ( ... )
                {
                }
            }
            break;
        }

        TagLogger::getInstance().endElement();
    }

    TagLogger::getInstance().endElement();
}
#endif

void PropertyMap::InsertProps( const PropertyMapPtr& rMap, const bool bOverwrite )
{
    if ( !rMap )
        return;

    for ( const auto& rPropPair : rMap->m_vMap )
    {
        if ( bOverwrite || !m_vMap.count(rPropPair.first) )
        {
            if ( !bOverwrite && !rPropPair.second.getIsDocDefault() )
                m_vMap.insert(std::make_pair(rPropPair.first, PropValue(rPropPair.second.getValue(), rPropPair.second.getGrabBagType(), true)));
            else
                m_vMap[rPropPair.first] = rPropPair.second;
        }
    }

    insertTableProperties( rMap.get(), bOverwrite );

    Invalidate();
}

void PropertyMap::insertTableProperties( const PropertyMap*, const bool )
{
#ifdef DBG_UTIL
    TagLogger::getInstance().element( "PropertyMap.insertTableProperties" );
#endif
}

void PropertyMap::printProperties()
{
#ifdef DBG_UTIL
    TagLogger::getInstance().startElement( "properties" );

    for ( const auto& rPropPair : m_vMap )
    {
        SAL_INFO( "writerfilter", getPropertyName( rPropPair.first ) );

        table::BorderLine2 aLine;
        sal_Int32 nColor;
        if ( rPropPair.second.getValue() >>= aLine )
        {
            TagLogger::getInstance().startElement( "borderline" );
            TagLogger::getInstance().attribute( "color", aLine.Color );
            TagLogger::getInstance().attribute( "inner", aLine.InnerLineWidth );
            TagLogger::getInstance().attribute( "outer", aLine.OuterLineWidth );
            TagLogger::getInstance().endElement();
        }
        else if ( rPropPair.second.getValue() >>= nColor )
        {
            TagLogger::getInstance().startElement( "color" );
            TagLogger::getInstance().attribute( "number", nColor );
            TagLogger::getInstance().endElement();
        }
    }

    TagLogger::getInstance().endElement();
#else
    (void) this; // avoid loplugin:staticmethods
#endif
}

SectionPropertyMap::SectionPropertyMap( bool bIsFirstSection )
    : m_bIsFirstSection( bIsFirstSection )
    , m_eBorderApply( BorderApply::ToAllInSection )
    , m_eBorderOffsetFrom( BorderOffsetFrom::Text )
    , m_bTitlePage( false )
    , m_nColumnCount( 0 )
    , m_nColumnDistance( 1249 )
    , m_bSeparatorLineIsOn( false )
    , m_bEvenlySpaced( false )
    , m_nPageNumber( -1 )
    , m_nPageNumberType( -1 )
    , m_nBreakType( -1 )
    , m_nLeftMargin( 2540 )  // page left margin,  default 1 inch = 1440 twip -> 2540 1/100 mm
    , m_nRightMargin( 2540 ) // page right margin,  default 1 inch = 1440 twip -> 2540 1/100 mm
    , m_nTopMargin( 2540 )
    , m_nBottomMargin( 2540 )
    , m_nHeaderTop( 1270 )    // 720 twip
    , m_nHeaderBottom( 1270 ) // 720 twip
    , m_nGridType( 0 )
    , m_nGridLinePitch( 1 )
    , m_nDxtCharSpace( 0 )
    , m_bGridSnapToChars( true )
    , m_nLnnMod( 0 )
    , m_nLnc(NS_ooxml::LN_Value_ST_LineNumberRestart_newPage)
    , m_ndxaLnn( 0 )
    , m_nLnnMin( 0 )
    , m_bDefaultHeaderLinkToPrevious( true )
    , m_bEvenPageHeaderLinkToPrevious( true )
    , m_bFirstPageHeaderLinkToPrevious( true )
    , m_bDefaultFooterLinkToPrevious( true )
    , m_bEvenPageFooterLinkToPrevious( true )
    , m_bFirstPageFooterLinkToPrevious( true )
{
#ifdef DBG_UTIL
    static sal_Int32 nNumber = 0;
    m_nDebugSectionNumber = nNumber++;
#endif

    for ( sal_Int32 nBorder = 0; nBorder < 4; ++nBorder )
    {
        m_nBorderDistances[nBorder] = -1;
        m_bBorderShadows[nBorder] = false;
    }
    // todo: set defaults in ApplyPropertiesToPageStyles
    // initialize defaults
    PaperInfo aLetter( PAPER_LETTER );
    // page height, 1/100mm
    Insert( PROP_HEIGHT, uno::makeAny( static_cast<sal_Int32>(aLetter.getHeight()) ) );
    // page width, 1/100mm
    Insert( PROP_WIDTH, uno::makeAny( static_cast<sal_Int32>(aLetter.getWidth()) ) );
    // page left margin, default 0x5a0 (1440) twip -> 2540 1/100 mm
    Insert( PROP_LEFT_MARGIN, uno::makeAny( sal_Int32(2540) ) );
    // page right margin, default 0x5a0 (1440) twip -> 2540 1/100 mm
    Insert( PROP_RIGHT_MARGIN, uno::makeAny( sal_Int32(2540) ) );
    // page top margin, default 0x5a0 (1440) twip -> 2540 1/100 mm
    Insert( PROP_TOP_MARGIN, uno::makeAny( sal_Int32(2540) ) );
    // page bottom margin, default 0x5a0 (1440) twip -> 2540 1/100 mm
    Insert( PROP_BOTTOM_MARGIN, uno::makeAny( sal_Int32(2540) ) );
    // page style layout
    Insert( PROP_PAGE_STYLE_LAYOUT, uno::makeAny( style::PageStyleLayout_ALL ) );
    uno::Any aFalse( uno::makeAny( false ) );
    Insert( PROP_GRID_DISPLAY, aFalse );
    Insert( PROP_GRID_PRINT, aFalse );
    Insert( PROP_GRID_MODE, uno::makeAny( text::TextGridMode::NONE ) );

    if ( m_bIsFirstSection )
    {
        m_sFirstPageStyleName = getPropertyName( PROP_FIRST_PAGE );
        m_sFollowPageStyleName = getPropertyName( PROP_STANDARD );
    }
}

uno::Reference< beans::XPropertySet > SectionPropertyMap::GetPageStyle( DomainMapper_Impl& rDM_Impl,
                                                                        bool bFirst )
{
    const uno::Reference< container::XNameContainer >& xPageStyles = rDM_Impl.GetPageStyles();
    const uno::Reference < lang::XMultiServiceFactory >& xTextFactory = rDM_Impl.GetTextFactory();
    uno::Reference< beans::XPropertySet > xRet;
    try
    {
        if ( bFirst )
        {
            if ( m_sFirstPageStyleName.isEmpty() && xPageStyles.is() )
            {
                assert( !rDM_Impl.IsInFootOrEndnote() && "Don't create useless page styles" );
                m_sFirstPageStyleName = rDM_Impl.GetUnusedPageStyleName();
                m_aFirstPageStyle.set( xTextFactory->createInstance( "com.sun.star.style.PageStyle" ),
                    uno::UNO_QUERY );

                // Call insertByName() before GetPageStyle(), otherwise the
                // first and the follow page style will have the same name, and
                // insertByName() will fail.
                if ( xPageStyles.is() )
                    xPageStyles->insertByName( m_sFirstPageStyleName, uno::makeAny( m_aFirstPageStyle ) );

                // Ensure that m_aFollowPageStyle has been created
                GetPageStyle( rDM_Impl, false );
                // Chain m_aFollowPageStyle to be after m_aFirstPageStyle
                m_aFirstPageStyle->setPropertyValue( "FollowStyle",
                    uno::makeAny( m_sFollowPageStyleName ) );
            }
            else if ( !m_aFirstPageStyle.is() && xPageStyles.is() )
            {
                xPageStyles->getByName( m_sFirstPageStyleName ) >>= m_aFirstPageStyle;
            }
            xRet = m_aFirstPageStyle;
        }
        else
        {
            if ( m_sFollowPageStyleName.isEmpty() && xPageStyles.is() )
            {
                assert( !rDM_Impl.IsInFootOrEndnote() && "Don't create useless page styles" );
                m_sFollowPageStyleName = rDM_Impl.GetUnusedPageStyleName();
                m_aFollowPageStyle.set( xTextFactory->createInstance( "com.sun.star.style.PageStyle" ),
                    uno::UNO_QUERY );
                xPageStyles->insertByName( m_sFollowPageStyleName, uno::makeAny( m_aFollowPageStyle ) );
            }
            else if ( !m_aFollowPageStyle.is() && xPageStyles.is() )
            {
                xPageStyles->getByName( m_sFollowPageStyleName ) >>= m_aFollowPageStyle;
            }
            xRet = m_aFollowPageStyle;
        }

    }
    catch ( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION( "writerfilter" );
    }

    return xRet;
}

void SectionPropertyMap::SetBorder( BorderPosition ePos, sal_Int32 nLineDistance, const table::BorderLine2& rBorderLine, bool bShadow )
{
    m_oBorderLines[ePos]     = rBorderLine;
    m_nBorderDistances[ePos] = nLineDistance;
    m_bBorderShadows[ePos]   = bShadow;
}

void SectionPropertyMap::ApplyBorderToPageStyles( DomainMapper_Impl& rDM_Impl,
                                                  BorderApply eBorderApply, BorderOffsetFrom eOffsetFrom )
{
    /*
    page border applies to:
    nIntValue & 0x07 ->
    0 all pages in this section
    1 first page in this section
    2 all pages in this section but first
    3 whole document (all sections)
    nIntValue & 0x18 -> page border depth 0 - in front 1- in back
    nIntValue & 0xe0 ->
    page border offset from:
    0 offset from text
    1 offset from edge of page
    */
    uno::Reference< beans::XPropertySet > xFirst;
    uno::Reference< beans::XPropertySet > xSecond;
    // todo: negative spacing (from ww8par6.cxx)
    switch ( eBorderApply )
    {
        case BorderApply::ToAllInSection: // all styles
            if ( !m_sFollowPageStyleName.isEmpty() )
                xFirst = GetPageStyle( rDM_Impl, false );
            if ( !m_sFirstPageStyleName.isEmpty() )
                xSecond = GetPageStyle( rDM_Impl, true );
            break;
        case BorderApply::ToFirstPageInSection: // first page
            if ( !m_sFirstPageStyleName.isEmpty() )
                xFirst = GetPageStyle( rDM_Impl, true );
            break;
        case BorderApply::ToAllButFirstInSection: // left and right
            if ( !m_sFollowPageStyleName.isEmpty() )
                xFirst = GetPageStyle( rDM_Impl, false );
            break;
        default:
            return;
    }

    // has to be sorted like enum BorderPosition: l-r-t-b
    const PropertyIds aBorderIds[4] =
    {
        PROP_LEFT_BORDER,
        PROP_RIGHT_BORDER,
        PROP_TOP_BORDER,
        PROP_BOTTOM_BORDER
    };

    const PropertyIds aBorderDistanceIds[4] =
    {
        PROP_LEFT_BORDER_DISTANCE,
        PROP_RIGHT_BORDER_DISTANCE,
        PROP_TOP_BORDER_DISTANCE,
        PROP_BOTTOM_BORDER_DISTANCE
    };

    const PropertyIds aMarginIds[4] =
    {
        PROP_LEFT_MARGIN,
        PROP_RIGHT_MARGIN,
        PROP_TOP_MARGIN,
        PROP_BOTTOM_MARGIN
    };

    for ( sal_Int32 nBorder = 0; nBorder < 4; ++nBorder )
    {
        if ( m_oBorderLines[nBorder] )
        {
            const OUString sBorderName = getPropertyName( aBorderIds[nBorder] );
            if ( xFirst.is() )
                xFirst->setPropertyValue( sBorderName, uno::makeAny( *m_oBorderLines[nBorder] ) );
            if ( xSecond.is() )
                xSecond->setPropertyValue( sBorderName, uno::makeAny( *m_oBorderLines[nBorder] ) );
        }
        if ( m_nBorderDistances[nBorder] >= 0 )
        {
            sal_uInt32 nLineWidth = 0;
            if ( m_oBorderLines[nBorder] )
                nLineWidth = m_oBorderLines[nBorder]->LineWidth;
            if ( xFirst.is() )
                SetBorderDistance( xFirst, aMarginIds[nBorder], aBorderDistanceIds[nBorder],
                    m_nBorderDistances[nBorder], eOffsetFrom, nLineWidth );
            if ( xSecond.is() )
                SetBorderDistance( xSecond, aMarginIds[nBorder], aBorderDistanceIds[nBorder],
                    m_nBorderDistances[nBorder], eOffsetFrom, nLineWidth );
        }
    }

    if ( m_bBorderShadows[BORDER_RIGHT] )
    {
        table::ShadowFormat aFormat = getShadowFromBorder( *m_oBorderLines[BORDER_RIGHT] );
        if ( xFirst.is() )
            xFirst->setPropertyValue( getPropertyName( PROP_SHADOW_FORMAT ), uno::makeAny( aFormat ) );
        if ( xSecond.is() )
            xSecond->setPropertyValue( getPropertyName( PROP_SHADOW_FORMAT ), uno::makeAny( aFormat ) );
    }
}

table::ShadowFormat PropertyMap::getShadowFromBorder( const table::BorderLine2& rBorder )
{
    // In Word UI, shadow is a boolean property, in OOXML, it's a boolean
    // property of each 4 border type, finally in Writer the border is a
    // property of the page style, with shadow location, distance and
    // color. See SwWW8ImplReader::SetShadow().
    table::ShadowFormat aFormat;
    aFormat.Color       = sal_Int32(COL_BLACK);
    aFormat.Location    = table::ShadowLocation_BOTTOM_RIGHT;
    aFormat.ShadowWidth = rBorder.LineWidth;
    return aFormat;
}

void SectionPropertyMap::SetBorderDistance( const uno::Reference< beans::XPropertySet >& xStyle,
                                            PropertyIds eMarginId,
                                            PropertyIds eDistId,
                                            sal_Int32 nDistance,
                                            BorderOffsetFrom eOffsetFrom,
                                            sal_uInt32 nLineWidth )
{
    if (!xStyle.is())
        return;
    const OUString sMarginName = getPropertyName( eMarginId );
    const OUString sBorderDistanceName = getPropertyName( eDistId );
    uno::Any aMargin = xStyle->getPropertyValue( sMarginName );
    sal_Int32 nMargin = 0;
    aMargin >>= nMargin;
    editeng::BorderDistanceFromWord(eOffsetFrom == BorderOffsetFrom::Edge, nMargin, nDistance,
                                    nLineWidth);

    // Change the margins with the border distance
    uno::Reference< beans::XMultiPropertySet > xMultiSet( xStyle, uno::UNO_QUERY_THROW );
    uno::Sequence<OUString> aProperties { sMarginName, sBorderDistanceName };
    uno::Sequence<uno::Any> aValues { uno::makeAny( nMargin ), uno::makeAny( nDistance ) };
    xMultiSet->setPropertyValues( aProperties, aValues );
}

void SectionPropertyMap::DontBalanceTextColumns()
{
    try
    {
        if ( m_xColumnContainer.is() )
            m_xColumnContainer->setPropertyValue( "DontBalanceTextColumns", uno::makeAny( true ) );
    }
    catch ( const uno::Exception& )
    {
        OSL_FAIL( "Exception in SectionPropertyMap::DontBalanceTextColumns" );
    }
}

void SectionPropertyMap::ApplySectionProperties( const uno::Reference< beans::XPropertySet >& xSection, DomainMapper_Impl& /*rDM_Impl*/ )
{
    try
    {
        if ( xSection.is() )
        {
            std::optional< PropertyMap::Property > pProp = getProperty( PROP_WRITING_MODE );
            if ( pProp )
                xSection->setPropertyValue( "WritingMode", pProp->second );
        }
    }
    catch ( uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("writerfilter", "Exception in SectionPropertyMap::ApplySectionProperties");
    }
}

void SectionPropertyMap::ApplyProtectionProperties( uno::Reference< beans::XPropertySet >& xSection, DomainMapper_Impl& rDM_Impl )
{
    try
    {
        // Word implements section protection differently than LO.
        // PROP_IS_PROTECTED only applies if global setting GetProtectForm is enabled.
        bool bIsProtected = rDM_Impl.GetSettingsTable()->GetProtectForm();
        if ( bIsProtected )
        {
            // If form protection is enabled then section protection is enabled, unless explicitly disabled
            if ( isSet(PROP_IS_PROTECTED) )
                getProperty(PROP_IS_PROTECTED)->second >>= bIsProtected;
            if ( !xSection.is() )
                xSection = rDM_Impl.appendTextSectionAfter( m_xStartingRange );
            if ( xSection.is() )
                xSection->setPropertyValue( getPropertyName(PROP_IS_PROTECTED), uno::makeAny(bIsProtected) );
        }
    }
    catch ( uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("writerfilter", "ApplyProtectionProperties failed setting PROP_IS_PROTECTED");
    }
}

uno::Reference< text::XTextColumns > SectionPropertyMap::ApplyColumnProperties( const uno::Reference< beans::XPropertySet >& xColumnContainer,
                                                                                DomainMapper_Impl& rDM_Impl )
{
    uno::Reference< text::XTextColumns > xColumns;
    assert( m_nColumnCount > 1 && "ApplyColumnProperties called without any columns" );
    try
    {
        const OUString sTextColumns = getPropertyName( PROP_TEXT_COLUMNS );
        if ( xColumnContainer.is() )
            xColumnContainer->getPropertyValue( sTextColumns ) >>= xColumns;
        uno::Reference< beans::XPropertySet > xColumnPropSet( xColumns, uno::UNO_QUERY_THROW );
        if ( !m_bEvenlySpaced &&
             ( sal_Int32(m_aColWidth.size()) == m_nColumnCount ) &&
             ( (sal_Int32(m_aColDistance.size()) == m_nColumnCount - 1) || (sal_Int32(m_aColDistance.size()) == m_nColumnCount) ) )
        {
            // the column width in word is an absolute value, in OOo it's relative
            // the distances are both absolute
            sal_Int32 nColSum = 0;
            for ( sal_Int32 nCol = 0; nCol < m_nColumnCount; ++nCol )
            {
                nColSum += m_aColWidth[nCol];
                if ( nCol )
                    nColSum += m_aColDistance[nCol - 1];
            }

            sal_Int32 nRefValue = xColumns->getReferenceValue();
            double fRel = nColSum ? double( nRefValue ) / double( nColSum ) : 0.0;
            uno::Sequence< text::TextColumn > aColumns( m_nColumnCount );
            text::TextColumn* pColumn = aColumns.getArray();

            nColSum = 0;
            for ( sal_Int32 nCol = 0; nCol < m_nColumnCount; ++nCol )
            {
                const double fLeft = nCol ? m_aColDistance[nCol - 1] / 2 : 0;
                pColumn[nCol].LeftMargin = fLeft;
                const double fRight = (nCol == m_nColumnCount - 1) ? 0 : m_aColDistance[nCol] / 2;
                pColumn[nCol].RightMargin = fRight;
                const double fWidth = m_aColWidth[nCol];
                pColumn[nCol].Width = (fWidth + fLeft + fRight) * fRel;
                nColSum += pColumn[nCol].Width;
            }
            if ( nColSum != nRefValue )
                pColumn[m_nColumnCount - 1].Width += (nRefValue - nColSum);
            assert( pColumn[m_nColumnCount - 1].Width >= 0 );

            xColumns->setColumns( aColumns );
        }
        else
        {
            xColumns->setColumnCount( m_nColumnCount );
            xColumnPropSet->setPropertyValue( getPropertyName( PROP_AUTOMATIC_DISTANCE ), uno::makeAny( m_nColumnDistance ) );
        }

        if ( m_bSeparatorLineIsOn )
        {
            xColumnPropSet->setPropertyValue( "SeparatorLineIsOn", uno::makeAny( true ) );
            xColumnPropSet->setPropertyValue( "SeparatorLineVerticalAlignment", uno::makeAny( style::VerticalAlignment_TOP ) );
            xColumnPropSet->setPropertyValue( "SeparatorLineRelativeHeight", uno::makeAny( static_cast<sal_Int8>(100) ) );
            xColumnPropSet->setPropertyValue( "SeparatorLineColor", uno::makeAny( static_cast<sal_Int32>(COL_BLACK) ) );
            // 1 twip -> 2 mm100.
            xColumnPropSet->setPropertyValue( "SeparatorLineWidth", uno::makeAny( static_cast<sal_Int32>(2) ) );
        }
        xColumnContainer->setPropertyValue( sTextColumns, uno::makeAny( xColumns ) );
        // Set the columns to be unbalanced if that compatibility option is set or this is the last section.
        m_xColumnContainer = xColumnContainer;
        if ( rDM_Impl.GetSettingsTable()->GetNoColumnBalance() || rDM_Impl.GetIsLastSectionGroup() )
            DontBalanceTextColumns();
    }
    catch ( const uno::Exception& )
    {
        OSL_FAIL( "Exception in SectionPropertyMap::ApplyColumnProperties" );
    }
    return xColumns;
}

bool SectionPropertyMap::HasHeader( bool bFirstPage ) const
{
    bool bRet = false;
    if ( (bFirstPage && m_aFirstPageStyle.is()) || (!bFirstPage && m_aFollowPageStyle.is()) )
    {
        if ( bFirstPage )
            m_aFirstPageStyle->getPropertyValue(
                getPropertyName( PROP_HEADER_IS_ON ) ) >>= bRet;
        else
            m_aFollowPageStyle->getPropertyValue(
                getPropertyName( PROP_HEADER_IS_ON ) ) >>= bRet;
    }
    return bRet;
}

bool SectionPropertyMap::HasFooter( bool bFirstPage ) const
{
    bool bRet = false;
    if ( (bFirstPage && m_aFirstPageStyle.is()) || (!bFirstPage && m_aFollowPageStyle.is()) )
    {
        if ( bFirstPage )
            m_aFirstPageStyle->getPropertyValue( getPropertyName( PROP_FOOTER_IS_ON ) ) >>= bRet;
        else
            m_aFollowPageStyle->getPropertyValue( getPropertyName( PROP_FOOTER_IS_ON ) ) >>= bRet;
    }
    return bRet;
}

#define MIN_HEAD_FOOT_HEIGHT 100 // minimum header/footer height

void SectionPropertyMap::CopyHeaderFooterTextProperty( const uno::Reference< beans::XPropertySet >& xPrevStyle,
                                                       const uno::Reference< beans::XPropertySet >& xStyle,
                                                       PropertyIds ePropId )
{
    try {
        OUString sName = getPropertyName( ePropId );

        SAL_INFO( "writerfilter", "Copying " << sName );
        uno::Reference< text::XTextCopy > xTxt;
        if ( xStyle.is() )
            xTxt.set( xStyle->getPropertyValue( sName ), uno::UNO_QUERY_THROW );

        uno::Reference< text::XTextCopy > xPrevTxt;
        if ( xPrevStyle.is() )
            xPrevTxt.set( xPrevStyle->getPropertyValue( sName ), uno::UNO_QUERY_THROW );

        xTxt->copyText( xPrevTxt );
    }
    catch ( const uno::Exception& )
    {
        TOOLS_INFO_EXCEPTION( "writerfilter", "An exception occurred in SectionPropertyMap::CopyHeaderFooterTextProperty( )" );
    }
}

// Copy headers and footers from the previous page style.
void SectionPropertyMap::CopyHeaderFooter( const uno::Reference< beans::XPropertySet >& xPrevStyle,
                                           const uno::Reference< beans::XPropertySet >& xStyle,
                                           bool bOmitRightHeader,
                                           bool bOmitLeftHeader,
                                           bool bOmitRightFooter,
                                           bool bOmitLeftFooter )
{
    bool bHasPrevHeader = false;
    bool bHeaderIsShared = true;
    OUString sHeaderIsOn = getPropertyName( PROP_HEADER_IS_ON );
    OUString sHeaderIsShared = getPropertyName( PROP_HEADER_IS_SHARED );
    if ( xPrevStyle.is() )
    {
        xPrevStyle->getPropertyValue( sHeaderIsOn ) >>= bHasPrevHeader;
        xPrevStyle->getPropertyValue( sHeaderIsShared ) >>= bHeaderIsShared;
    }

    if ( bHasPrevHeader )
    {
        uno::Reference< beans::XMultiPropertySet > xMultiSet( xStyle, uno::UNO_QUERY_THROW );
        uno::Sequence<OUString> aProperties { sHeaderIsOn, sHeaderIsShared };
        uno::Sequence<uno::Any> aValues { uno::makeAny( true ), uno::makeAny( bHeaderIsShared ) };
        xMultiSet->setPropertyValues( aProperties, aValues );
        if ( !bOmitRightHeader )
        {
            CopyHeaderFooterTextProperty( xPrevStyle, xStyle,
                PROP_HEADER_TEXT );
        }
        if ( !bHeaderIsShared && !bOmitLeftHeader )
        {
            CopyHeaderFooterTextProperty( xPrevStyle, xStyle,
                PROP_HEADER_TEXT_LEFT );
        }
    }

    bool bHasPrevFooter = false;
    bool bFooterIsShared = true;
    OUString sFooterIsOn = getPropertyName( PROP_FOOTER_IS_ON );
    OUString sFooterIsShared = getPropertyName( PROP_FOOTER_IS_SHARED );
    if ( xPrevStyle.is() )
    {
        xPrevStyle->getPropertyValue( sFooterIsOn ) >>= bHasPrevFooter;
        xPrevStyle->getPropertyValue( sFooterIsShared ) >>= bFooterIsShared;
    }

    if ( !bHasPrevFooter )
        return;

    uno::Reference< beans::XMultiPropertySet > xMultiSet( xStyle, uno::UNO_QUERY_THROW );
    uno::Sequence<OUString> aProperties { sFooterIsOn, sFooterIsShared };
    uno::Sequence<uno::Any> aValues { uno::makeAny( true ), uno::makeAny( bFooterIsShared ) };
    xMultiSet->setPropertyValues( aProperties, aValues );
    if ( !bOmitRightFooter )
    {
        CopyHeaderFooterTextProperty( xPrevStyle, xStyle,
            PROP_FOOTER_TEXT );
    }
    if ( !bFooterIsShared && !bOmitLeftFooter )
    {
        CopyHeaderFooterTextProperty( xPrevStyle, xStyle,
            PROP_FOOTER_TEXT_LEFT );
    }
}

// Copy header and footer content from the previous docx section as needed.
//
// Any headers and footers which were not defined in this docx section
// should be "linked" with the corresponding header or footer from the
// previous section.  LO does not support linking of header/footer content
// across page styles so we just copy the content from the previous section.
void SectionPropertyMap::CopyLastHeaderFooter( bool bFirstPage, DomainMapper_Impl& rDM_Impl )
{
    SAL_INFO( "writerfilter", "START>>> SectionPropertyMap::CopyLastHeaderFooter()" );
    SectionPropertyMap* pLastContext = rDM_Impl.GetLastSectionContext();
    if ( pLastContext )
    {
        uno::Reference< beans::XPropertySet > xPrevStyle = pLastContext->GetPageStyle( rDM_Impl,
            bFirstPage );
        uno::Reference< beans::XPropertySet > xStyle = GetPageStyle( rDM_Impl,
            bFirstPage );

        if ( bFirstPage )
        {
            CopyHeaderFooter( xPrevStyle, xStyle,
                !m_bFirstPageHeaderLinkToPrevious, true,
                !m_bFirstPageFooterLinkToPrevious, true );
        }
        else
        {
            CopyHeaderFooter( xPrevStyle, xStyle,
                !m_bDefaultHeaderLinkToPrevious,
                !m_bEvenPageHeaderLinkToPrevious,
                !m_bDefaultFooterLinkToPrevious,
                !m_bEvenPageFooterLinkToPrevious );
        }
    }
    SAL_INFO( "writerfilter", "END>>> SectionPropertyMap::CopyLastHeaderFooter()" );
}

void SectionPropertyMap::PrepareHeaderFooterProperties( bool bFirstPage )
{
    bool bCopyFirstToFollow = bFirstPage && m_bTitlePage && m_aFollowPageStyle.is();

    sal_Int32 nTopMargin = m_nTopMargin;
    sal_Int32 nHeaderTop = m_nHeaderTop;
    if ( HasHeader( bFirstPage ) )
    {
        nTopMargin = nHeaderTop;
        if ( m_nTopMargin > 0 && m_nTopMargin > nHeaderTop )
            nHeaderTop = m_nTopMargin - nHeaderTop;
        else
            nHeaderTop = 0;

        // minimum header height 1mm
        if ( nHeaderTop < MIN_HEAD_FOOT_HEIGHT )
            nHeaderTop = MIN_HEAD_FOOT_HEIGHT;
    }


    if ( m_nTopMargin >= 0 ) //fixed height header -> see WW8Par6.hxx
    {
        Insert( PROP_HEADER_IS_DYNAMIC_HEIGHT, uno::makeAny( true ) );
        Insert( PROP_HEADER_DYNAMIC_SPACING, uno::makeAny( true ) );
        Insert( PROP_HEADER_BODY_DISTANCE, uno::makeAny( nHeaderTop - MIN_HEAD_FOOT_HEIGHT ) );// ULSpace.Top()
        Insert( PROP_HEADER_HEIGHT, uno::makeAny( nHeaderTop ) );

        if (bCopyFirstToFollow && HasHeader(/*bFirstPage=*/true))
        {
            m_aFollowPageStyle->setPropertyValue("HeaderDynamicSpacing",
                                                 getProperty(PROP_HEADER_DYNAMIC_SPACING)->second);
            m_aFollowPageStyle->setPropertyValue("HeaderHeight",
                                                 getProperty(PROP_HEADER_HEIGHT)->second);
        }
    }
    else
    {
        //todo: old filter fakes a frame into the header/footer to support overlapping
        //current setting is completely wrong!
        Insert( PROP_HEADER_HEIGHT, uno::makeAny( nHeaderTop ) );
        Insert( PROP_HEADER_BODY_DISTANCE, uno::makeAny( m_nTopMargin - nHeaderTop ) );
        Insert( PROP_HEADER_IS_DYNAMIC_HEIGHT, uno::makeAny( false ) );
        Insert( PROP_HEADER_DYNAMIC_SPACING, uno::makeAny( false ) );
    }

    sal_Int32 nBottomMargin = m_nBottomMargin;
    sal_Int32 nHeaderBottom = m_nHeaderBottom;
    if ( HasFooter( bFirstPage ) )
    {
        nBottomMargin = nHeaderBottom;
        if ( m_nBottomMargin > 0 && m_nBottomMargin > nHeaderBottom )
            nHeaderBottom = m_nBottomMargin - nHeaderBottom;
        else
            nHeaderBottom = 0;
        if ( nHeaderBottom < MIN_HEAD_FOOT_HEIGHT )
            nHeaderBottom = MIN_HEAD_FOOT_HEIGHT;
    }

    if ( m_nBottomMargin >= 0 ) //fixed height footer -> see WW8Par6.hxx
    {
        Insert( PROP_FOOTER_IS_DYNAMIC_HEIGHT, uno::makeAny( true ) );
        Insert( PROP_FOOTER_DYNAMIC_SPACING, uno::makeAny( true ) );
        Insert( PROP_FOOTER_BODY_DISTANCE, uno::makeAny( nHeaderBottom - MIN_HEAD_FOOT_HEIGHT ) );
        Insert( PROP_FOOTER_HEIGHT, uno::makeAny( nHeaderBottom ) );

        if (bCopyFirstToFollow && HasFooter(/*bFirstPage=*/true))
        {
            m_aFollowPageStyle->setPropertyValue("FooterDynamicSpacing",
                                                 getProperty(PROP_FOOTER_DYNAMIC_SPACING)->second);
            m_aFollowPageStyle->setPropertyValue("FooterHeight",
                                                 getProperty(PROP_FOOTER_HEIGHT)->second);
        }
    }
    else
    {
        //todo: old filter fakes a frame into the header/footer to support overlapping
        //current setting is completely wrong!
        Insert( PROP_FOOTER_IS_DYNAMIC_HEIGHT, uno::makeAny( false ) );
        Insert( PROP_FOOTER_DYNAMIC_SPACING, uno::makeAny( false ) );
        Insert( PROP_FOOTER_HEIGHT, uno::makeAny( m_nBottomMargin - nHeaderBottom ) );
        Insert( PROP_FOOTER_BODY_DISTANCE, uno::makeAny( nHeaderBottom ) );
    }

    //now set the top/bottom margin for the follow page style
    Insert( PROP_TOP_MARGIN, uno::makeAny( std::max<sal_Int32>(nTopMargin, 0) ) );
    Insert( PROP_BOTTOM_MARGIN, uno::makeAny( std::max<sal_Int32>(nBottomMargin, 0) ) );
}

static uno::Reference< beans::XPropertySet > lcl_GetRangeProperties( bool bIsFirstSection,
                                                              DomainMapper_Impl& rDM_Impl,
                                                              const uno::Reference< text::XTextRange >& xStartingRange )
{
    uno::Reference< beans::XPropertySet > xRangeProperties;
    if ( bIsFirstSection && rDM_Impl.GetBodyText().is() )
    {
        uno::Reference< container::XEnumerationAccess > xEnumAccess( rDM_Impl.GetBodyText(), uno::UNO_QUERY_THROW );
        uno::Reference< container::XEnumeration > xEnum = xEnumAccess->createEnumeration();
        xRangeProperties.set( xEnum->nextElement(), uno::UNO_QUERY_THROW );
        if ( rDM_Impl.GetIsDummyParaAddedForTableInSection() && xEnum->hasMoreElements() )
            xRangeProperties.set( xEnum->nextElement(), uno::UNO_QUERY_THROW );
    }
    else if ( xStartingRange.is() )
        xRangeProperties.set( xStartingRange, uno::UNO_QUERY_THROW );
    return xRangeProperties;
}

void SectionPropertyMap::HandleMarginsHeaderFooter( bool bFirstPage, DomainMapper_Impl& rDM_Impl )
{
    Insert( PROP_LEFT_MARGIN, uno::makeAny( m_nLeftMargin ) );
    Insert( PROP_RIGHT_MARGIN, uno::makeAny( m_nRightMargin ) );

    if ( rDM_Impl.m_oBackgroundColor )
        Insert( PROP_BACK_COLOR, uno::makeAny( *rDM_Impl.m_oBackgroundColor ) );

    // Check for missing footnote separator only in case there is at least
    // one footnote.
    if (rDM_Impl.m_bHasFtn && !rDM_Impl.m_bHasFtnSep)
    {
        // Set footnote line width to zero, document has no footnote separator.
        Insert(PROP_FOOTNOTE_LINE_RELATIVE_WIDTH, uno::makeAny(sal_Int32(0)));
    }
    if ( rDM_Impl.m_bHasFtnSep )
    {
        //If default paragraph style is RTL, footnote separator should be right aligned
        //and for RTL locales, LTR default paragraph style should present a left aligned footnote separator
        try
        {
            uno::Reference<style::XStyleFamiliesSupplier> xStylesSupplier(rDM_Impl.GetTextDocument(), uno::UNO_QUERY);
            if ( xStylesSupplier.is() )
            {
                uno::Reference<container::XNameAccess> xStyleFamilies = xStylesSupplier->getStyleFamilies();
                uno::Reference<container::XNameAccess> xParagraphStyles;
                if ( xStyleFamilies.is() )
                    xStyleFamilies->getByName("ParagraphStyles") >>= xParagraphStyles;
                uno::Reference<beans::XPropertySet> xStandard;
                if ( xParagraphStyles.is() )
                    xParagraphStyles->getByName("Standard") >>= xStandard;
                if ( xStandard.is() )
                {
                    sal_Int16 aWritingMode(0);
                    xStandard->getPropertyValue( getPropertyName(PROP_WRITING_MODE) ) >>= aWritingMode;
                    if( aWritingMode == text::WritingMode2::RL_TB )
                        Insert( PROP_FOOTNOTE_LINE_ADJUST, uno::makeAny( sal_Int16(text::HorizontalAdjust_RIGHT) ), false );
                    else
                        Insert( PROP_FOOTNOTE_LINE_ADJUST, uno::makeAny( sal_Int16(text::HorizontalAdjust_LEFT) ), false );
                }
            }
        }
        catch ( const uno::Exception& ) {}
    }

    /*** if headers/footers are available then the top/bottom margins of the
    header/footer are copied to the top/bottom margin of the page
    */
    CopyLastHeaderFooter( bFirstPage, rDM_Impl );
    PrepareHeaderFooterProperties( bFirstPage );
}

bool SectionPropertyMap::FloatingTableConversion( const DomainMapper_Impl& rDM_Impl, FloatingTableInfo& rInfo )
{
    // This is OOXML version of the code deciding if the table needs to be
    // in a floating frame.
    // For ww8 code, see SwWW8ImplReader::FloatingTableConversion in
    // sw/source/filter/ww8/ww8par.cxx
    // The two should do the same, so if you make changes here, please check
    // that the other is in sync.

    // Note that this is just a list of heuristics till sw core can have a
    // table that is floating and can span over multiple pages at the same
    // time.

    // If there is an explicit section break right after a table, then there
    // will be no wrapping anyway.
    if (rDM_Impl.m_bConvertedTable && !rDM_Impl.GetIsLastSectionGroup() && rInfo.m_nBreakType == NS_ooxml::LN_Value_ST_SectionMark_nextPage)
        return false;

    sal_Int32 nPageWidth = GetPageWidth();
    sal_Int32 nTextAreaWidth = nPageWidth - GetLeftMargin() - GetRightMargin();
    // Count the layout width of the table.
    sal_Int32 nTableWidth = rInfo.m_nTableWidth;
    if (rInfo.m_nTableWidthType == text::SizeType::VARIABLE)
    {
        nTableWidth *= nTextAreaWidth / 100.0;
    }
    sal_Int32 nLeftMargin = 0;
    if ( rInfo.getPropertyValue( "LeftMargin" ) >>= nLeftMargin )
        nTableWidth += nLeftMargin;
    sal_Int32 nRightMargin = 0;
    if ( rInfo.getPropertyValue( "RightMargin" ) >>= nRightMargin )
        nTableWidth += nRightMargin;

    sal_Int16 nHoriOrientRelation = rInfo.getPropertyValue( "HoriOrientRelation" ).get<sal_Int16>();
    sal_Int16 nVertOrientRelation = rInfo.getPropertyValue( "VertOrientRelation" ).get<sal_Int16>();
    if ( nHoriOrientRelation == text::RelOrientation::PAGE_FRAME && nVertOrientRelation == text::RelOrientation::PAGE_FRAME )
    {
        sal_Int16 nHoriOrient = rInfo.getPropertyValue( "HoriOrient" ).get<sal_Int16>();
        sal_Int16 nVertOrient = rInfo.getPropertyValue( "VertOrient" ).get<sal_Int16>();
        if ( nHoriOrient == text::HoriOrientation::NONE && nVertOrient == text::VertOrientation::NONE )
        {
            // Anchor position is relative to the page horizontally and vertically as well and is an absolute position.
            // The more close we are to the left edge, the less likely there will be any wrapping.
            // The more close we are to the bottom, the more likely the table will span over to the next page
            // So if we're in the bottom left quarter, don't do any conversion.
            sal_Int32 nHoriOrientPosition = rInfo.getPropertyValue( "HoriOrientPosition" ).get<sal_Int32>();
            sal_Int32 nVertOrientPosition = rInfo.getPropertyValue( "VertOrientPosition" ).get<sal_Int32>();
            sal_Int32 nPageHeight = getProperty( PROP_HEIGHT )->second.get<sal_Int32>();
            if ( nHoriOrientPosition < (nPageWidth / 2) && nVertOrientPosition >( nPageHeight / 2 ) )
                return false;
        }
    }

    // It seems Word has a limit here, so that in case the table width is quite
    // close to the text area width, then it won't perform a wrapping, even in
    // case the content (e.g. an empty paragraph) would fit. The magic constant
    // here represents this limit.
    const sal_Int32 nMagicNumber = 469;

    // If the table's width is smaller than the text area width, text might
    // be next to the table and so it should behave as a floating table.
    if ( (nTableWidth + nMagicNumber) < nTextAreaWidth )
        return true;

    // If the position is relative to the edge of the page, then we need to check the whole
    // page width to see whether text can fit next to the table.
    if ( nHoriOrientRelation == text::RelOrientation::PAGE_FRAME )
    {
        // If the table is wide enough so that no text fits next to it, then don't create a fly
        // for the table: no wrapping will be performed anyway, but multi-page
        // tables will be broken.
        if ((nTableWidth + nMagicNumber) < (nPageWidth - std::min(GetLeftMargin(), GetRightMargin())))
            return true;
    }

    // If there are columns, always create the fly, otherwise the columns would
    // restrict geometry of the table.
    if ( ColumnCount() > 1 )
        return true;

    return false;
}

void SectionPropertyMap::InheritOrFinalizePageStyles( DomainMapper_Impl& rDM_Impl )
{
    // if no new styles have been created for this section, inherit from the previous section,
    // otherwise apply this section's settings to the new style.
    // Ensure that FollowPage is inherited first - otherwise GetPageStyle may auto-create a follow when checking FirstPage.
    SectionPropertyMap* pLastContext = rDM_Impl.GetLastSectionContext();
    //tdf124637 TODO: identify and skip special sections (like footnotes/endnotes)
    if ( pLastContext && m_sFollowPageStyleName.isEmpty() )
        m_sFollowPageStyleName = pLastContext->GetPageStyleName();
    else
    {
        HandleMarginsHeaderFooter( /*bFirst=*/false, rDM_Impl );
        GetPageStyle( rDM_Impl, /*bFirst=*/false );
        if ( rDM_Impl.IsNewDoc() && m_aFollowPageStyle.is() )
            ApplyProperties_( m_aFollowPageStyle );
    }

    // FirstPageStyle may only be inherited if it will not be used or re-linked to a different follow
    if ( !m_bTitlePage && pLastContext && m_sFirstPageStyleName.isEmpty() )
        m_sFirstPageStyleName = pLastContext->GetPageStyleName( /*bFirst=*/true );
    else
    {
        HandleMarginsHeaderFooter( /*bFirst=*/true, rDM_Impl );
        GetPageStyle( rDM_Impl, /*bFirst=*/true );
        if ( rDM_Impl.IsNewDoc() && m_aFirstPageStyle.is() )
            ApplyProperties_( m_aFirstPageStyle );

        // Chain m_aFollowPageStyle to be after m_aFirstPageStyle
        m_aFirstPageStyle->setPropertyValue( "FollowStyle", uno::makeAny( m_sFollowPageStyleName ) );
    }
}

void SectionPropertyMap::HandleIncreasedAnchoredObjectSpacing(DomainMapper_Impl& rDM_Impl)
{
    // Ignore Word 2010 and older.
    if (rDM_Impl.GetSettingsTable()->GetWordCompatibilityMode() < 15)
        return;

    sal_Int32 nPageWidth = GetPageWidth();
    sal_Int32 nTextAreaWidth = nPageWidth - GetLeftMargin() - GetRightMargin();

    std::vector<AnchoredObjectsInfo>& rAnchoredObjectAnchors = rDM_Impl.m_aAnchoredObjectAnchors;
    for (const auto& rAnchor : rAnchoredObjectAnchors)
    {
        // Ignore this paragraph when there are not enough shapes to trigger the Word bug we
        // emulate.
        if (rAnchor.m_aAnchoredObjects.size() < 4)
            continue;

        // Ignore this paragraph if none of the objects are wrapped in the background.
        sal_Int32 nOpaqueCount = 0;
        for (const auto& rAnchored : rAnchor.m_aAnchoredObjects)
        {
            uno::Reference<beans::XPropertySet> xShape(rAnchored.m_xAnchoredObject, uno::UNO_QUERY);
            if (!xShape.is())
            {
                continue;
            }

            bool bOpaque = true;
            xShape->getPropertyValue("Opaque") >>= bOpaque;
            if (!bOpaque)
            {
                ++nOpaqueCount;
            }
        }
        if (nOpaqueCount < 1)
        {
            continue;
        }

        // Analyze the anchored objects of this paragraph, now that we know the
        // page width.
        sal_Int32 nShapesWidth = 0;
        for (const auto& rAnchored : rAnchor.m_aAnchoredObjects)
        {
            uno::Reference<drawing::XShape> xShape(rAnchored.m_xAnchoredObject, uno::UNO_QUERY);
            if (!xShape.is())
                continue;

            uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
            if (!xPropertySet.is())
                continue;

            // Ignore objects with no wrapping.
            text::WrapTextMode eWrap = text::WrapTextMode_THROUGH;
            xPropertySet->getPropertyValue("Surround") >>= eWrap;
            if (eWrap == text::WrapTextMode_THROUGH)
                continue;

            // Use the original left margin, in case GraphicImport::lcl_sprm() reduced the doc model
            // one to 0.
            sal_Int32 nLeftMargin = rAnchored.m_nLeftMargin;
            sal_Int32 nRightMargin = 0;
            xPropertySet->getPropertyValue("RightMargin") >>= nRightMargin;
            nShapesWidth += xShape->getSize().Width + nLeftMargin + nRightMargin;
        }

        // Ignore cases when we have enough horizontal space for the shapes.
        if (nTextAreaWidth > nShapesWidth)
            continue;

        sal_Int32 nHeight = 0;
        for (const auto& rAnchored : rAnchor.m_aAnchoredObjects)
        {
            uno::Reference<drawing::XShape> xShape(rAnchored.m_xAnchoredObject, uno::UNO_QUERY);
            if (!xShape.is())
                continue;

            nHeight += xShape->getSize().Height;
        }

        uno::Reference<beans::XPropertySet> xParagraph(rAnchor.m_xParagraph, uno::UNO_QUERY);
        if (xParagraph.is())
        {
            sal_Int32 nTopMargin = 0;
            xParagraph->getPropertyValue("ParaTopMargin") >>= nTopMargin;
            // Increase top spacing of the paragraph to match Word layout
            // behavior.
            nTopMargin = std::max(nTopMargin, nHeight);
            xParagraph->setPropertyValue("ParaTopMargin", uno::makeAny(nTopMargin));
        }
    }
    rAnchoredObjectAnchors.clear();
}

void SectionPropertyMap::CloseSectionGroup( DomainMapper_Impl& rDM_Impl )
{
    SectionPropertyMap* pPrevSection = rDM_Impl.GetLastSectionContext();

    // The default section type is nextPage.
    if ( m_nBreakType == -1 )
        m_nBreakType = NS_ooxml::LN_Value_ST_SectionMark_nextPage;
    else if ( m_nBreakType == NS_ooxml::LN_Value_ST_SectionMark_nextColumn )
    {
        // Word 2013+ seems to treat a section column break as a page break all the time.
        // It always acts like a page break if there are no columns, or a different number of columns.
        // Also, if this is the first section, the break type is basically irrelevant - works best as nextPage.
        if ( rDM_Impl.GetSettingsTable()->GetWordCompatibilityMode() > 14
             || !pPrevSection
             || m_nColumnCount < 2
             || m_nColumnCount != pPrevSection->ColumnCount()
           )
        {
            m_nBreakType = NS_ooxml::LN_Value_ST_SectionMark_nextPage;
        }
    }
    else if ( m_nBreakType == NS_ooxml::LN_Value_ST_SectionMark_continuous )
    {
        // if page orientation differs from previous section, it can't be treated as continuous
        if ( pPrevSection )
        {
            bool bIsLandscape = false;
            std::optional< PropertyMap::Property > pProp = getProperty( PROP_IS_LANDSCAPE );
            if ( pProp )
                pProp->second >>= bIsLandscape;

            bool bPrevIsLandscape = false;
            pProp = pPrevSection->getProperty( PROP_IS_LANDSCAPE );
            if ( pProp )
                pProp->second >>= bPrevIsLandscape;

            if ( bIsLandscape != bPrevIsLandscape )
                m_nBreakType = NS_ooxml::LN_Value_ST_SectionMark_nextPage;
        }
    }

    // Text area width is known at the end of a section: decide if tables should be converted or not.
    std::vector<FloatingTableInfo>& rPendingFloatingTables = rDM_Impl.m_aPendingFloatingTables;
    uno::Reference<text::XTextAppendAndConvert> xBodyText( rDM_Impl.GetBodyText(), uno::UNO_QUERY );
    for ( FloatingTableInfo & rInfo : rPendingFloatingTables )
    {
        rInfo.m_nBreakType = m_nBreakType;
        if ( FloatingTableConversion( rDM_Impl, rInfo ) )
        {
            std::vector<css::uno::Any> aFramedRedlines = rDM_Impl.aFramedRedlines;
            try
            {
                // convert redline ranges to cursor movement and character length
                std::vector<sal_Int32> redPos, redLen;
                std::vector<OUString> redCell;
                std::vector<OUString> redTable;
                for( size_t i = 0; i < aFramedRedlines.size(); i+=3)
                {
                    uno::Reference<text::XText> xCell;
                    uno::Reference< text::XTextRange > xRange;
                    aFramedRedlines[i] >>= xRange;
                    uno::Reference< beans::XPropertySet > xRangeProperties;
                    if ( xRange.is() )
                    {
                        OUString sTableName;
                        OUString sCellName;
                        xRangeProperties.set( xRange, uno::UNO_QUERY_THROW );
                        if (xRangeProperties->getPropertySetInfo()->hasPropertyByName("TextTable"))
                        {
                            uno::Any aTable = xRangeProperties->getPropertyValue("TextTable");
                            if ( aTable != uno::Any() )
                            {
                                uno::Reference<text::XTextTable> xTable;
                                aTable >>= xTable;
                                uno::Reference<beans::XPropertySet> xTableProperties(xTable, uno::UNO_QUERY);
                                xTableProperties->getPropertyValue("TableName") >>= sTableName;
                            }
                            if (xRangeProperties->getPropertySetInfo()->hasPropertyByName("Cell"))
                            {
                                uno::Any aCell = xRangeProperties->getPropertyValue("Cell");
                                if ( aCell != uno::Any() )
                                {
                                    aCell >>= xCell;
                                    uno::Reference<beans::XPropertySet> xCellProperties(xCell, uno::UNO_QUERY);
                                    xCellProperties->getPropertyValue("CellName") >>= sCellName;
                                }
                            }
                        }
                        redTable.push_back(sTableName);
                        redCell.push_back(sCellName);
                        bool bOk = false;
                        if (!sTableName.isEmpty() && !sCellName.isEmpty())
                        {
                            uno::Reference<text::XTextCursor> xRangeCursor = xCell->createTextCursorByRange( xRange );
                            if ( xRangeCursor.is() )
                            {
                                bOk = true;
                                sal_Int32 nLen = xRange->getString().getLength();
                                redLen.push_back(nLen);
                                xRangeCursor->gotoStart(true);
                                redPos.push_back(xRangeCursor->getString().getLength() - nLen);
                            }
                        }
                        if (!bOk)
                        {
                            // missing cell or failed createTextCursorByRange()
                            redLen.push_back(-1);
                            redPos.push_back(-1);
                        }
                    }
                }

                xBodyText->convertToTextFrame(rInfo.m_xStart, rInfo.m_xEnd,
                                              rInfo.m_aFrameProperties);

                uno::Reference<text::XTextTablesSupplier> xTextDocument(rDM_Impl.GetTextDocument(), uno::UNO_QUERY);
                uno::Reference<container::XNameAccess> xTables = xTextDocument->getTextTables();
                for( size_t i = 0; i < aFramedRedlines.size(); i+=3)
                {
                    OUString sType;
                    beans::PropertyValues aRedlineProperties( 3 );
                    // skip failed createTextCursorByRange()
                    if (redPos[i/3] == -1)
                        continue;
                    aFramedRedlines[i+1] >>= sType;
                    aFramedRedlines[i+2] >>= aRedlineProperties;
                    uno::Reference<text::XTextTable> xTable(xTables->getByName(redTable[i/3]), uno::UNO_QUERY);
                    uno::Reference<text::XText> xCell(xTable->getCellByName(redCell[i/3]), uno::UNO_QUERY);
                    uno::Reference<text::XTextCursor> xCrsr = xCell->createTextCursor();
                    xCrsr->goRight(redPos[i/3], false);
                    xCrsr->goRight(redLen[i/3], true);
                    uno::Reference < text::XRedline > xRedline( xCrsr, uno::UNO_QUERY_THROW );
                    xRedline->makeRedline( sType, aRedlineProperties );
                }
            }
            catch (const uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION("writerfilter", "convertToTextFrame() failed");
            }

            aFramedRedlines.clear();
        }
    }
    rPendingFloatingTables.clear();

    try
    {
        HandleIncreasedAnchoredObjectSpacing(rDM_Impl);
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("writerfilter", "HandleIncreasedAnchoredObjectSpacing() failed");
    }

    if ( m_nLnnMod )
    {
        bool bFirst = rDM_Impl.IsLineNumberingSet();
        rDM_Impl.SetLineNumbering( m_nLnnMod, m_nLnc, m_ndxaLnn );
        if ( m_nLnnMin > 0 || (bFirst && m_nLnc == NS_ooxml::LN_Value_ST_LineNumberRestart_newSection) )
        {
            //set the starting value at the beginning of the section
            try
            {
                uno::Reference< beans::XPropertySet > xRangeProperties;
                if ( m_xStartingRange.is() )
                {
                    xRangeProperties.set( m_xStartingRange, uno::UNO_QUERY_THROW );
                }
                else
                {
                    //set the start value at the beginning of the document
                    xRangeProperties.set( rDM_Impl.GetTextDocument()->getText()->getStart(), uno::UNO_QUERY_THROW );
                }
                // Writer is 1-based, Word is 0-based.
                xRangeProperties->setPropertyValue(
                    getPropertyName(PROP_PARA_LINE_NUMBER_START_VALUE),
                    uno::makeAny(m_nLnnMin + 1));
            }
            catch ( const uno::Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("writerfilter.dmapper", "Exception in SectionPropertyMap::CloseSectionGroup");
            }
        }
    }

    if (m_nBreakType == static_cast<sal_Int32>(NS_ooxml::LN_Value_ST_SectionMark_continuous)
        && !rDM_Impl.IsInComments())
    {
        //todo: insert a section or access the already inserted section
        uno::Reference< beans::XPropertySet > xSection =
            rDM_Impl.appendTextSectionAfter( m_xStartingRange );
        if ( xSection.is() )
        {
            if ( m_nColumnCount > 1 )
                ApplyColumnProperties( xSection, rDM_Impl );

            ApplyProtectionProperties( xSection, rDM_Impl );
        }

        try
        {
            InheritOrFinalizePageStyles( rDM_Impl );
            ApplySectionProperties( xSection, rDM_Impl );  //depends on InheritOrFinalizePageStyles
            OUString aName = m_bTitlePage ? m_sFirstPageStyleName : m_sFollowPageStyleName;
            uno::Reference< beans::XPropertySet > xRangeProperties( lcl_GetRangeProperties( m_bIsFirstSection, rDM_Impl, m_xStartingRange ) );
            if ( m_bIsFirstSection && !aName.isEmpty() && xRangeProperties.is() )
            {
                xRangeProperties->setPropertyValue( getPropertyName( PROP_PAGE_DESC_NAME ), uno::makeAny( aName ) );
            }
            else if ((!m_bFirstPageHeaderLinkToPrevious ||
                      !m_bFirstPageFooterLinkToPrevious ||
                      !m_bDefaultHeaderLinkToPrevious ||
                      !m_bDefaultFooterLinkToPrevious ||
                      !m_bEvenPageHeaderLinkToPrevious ||
                      !m_bEvenPageFooterLinkToPrevious)
                    && rDM_Impl.GetCurrentXText())
            {   // find a node in the section that has a page break and change
                // it to apply the page style; see "nightmare scenario" in
                // wwSectionManager::InsertSegments()
                auto xTextAppend = rDM_Impl.GetCurrentXText();
                uno::Reference<container::XEnumerationAccess> const xCursor(
                    xTextAppend->createTextCursorByRange(
                        uno::Reference<text::XTextContent>(xSection, uno::UNO_QUERY_THROW)->getAnchor()),
                    uno::UNO_QUERY_THROW);
                uno::Reference<container::XEnumeration> const xEnum(
                        xCursor->createEnumeration());
                bool isFound = false;
                while (xEnum->hasMoreElements())
                {
                    uno::Reference<beans::XPropertySet> xElem;
                    xEnum->nextElement() >>= xElem;
                    if (xElem->getPropertySetInfo()->hasPropertyByName("BreakType"))
                    {
                        style::BreakType bt;
                        if ((xElem->getPropertyValue("BreakType") >>= bt)
                            && bt == style::BreakType_PAGE_BEFORE)
                        {
                            // tdf#112201: do *not* use m_sFirstPageStyleName here!
                            xElem->setPropertyValue(getPropertyName(PROP_PAGE_DESC_NAME),
                                    uno::makeAny(m_sFollowPageStyleName));
                            isFound = true;
                            break;
                        }
                    }
                }
                uno::Reference<text::XParagraphCursor> const xPCursor(xCursor,
                                                                      uno::UNO_QUERY_THROW);
                float fCharHeight = 0;
                if (!isFound)
                {   // HACK: try the last paragraph of the previous section
                    xPCursor->gotoPreviousParagraph(false);
                    uno::Reference<beans::XPropertySet> const xPSCursor(xCursor, uno::UNO_QUERY_THROW);
                    style::BreakType bt;
                    if ((xPSCursor->getPropertyValue("BreakType") >>= bt)
                        && bt == style::BreakType_PAGE_BEFORE)
                    {
                        xPSCursor->setPropertyValue(getPropertyName(PROP_PAGE_DESC_NAME),
                                uno::makeAny(m_sFollowPageStyleName));
                        isFound = true;
                    }
                    else
                    {
                        xPSCursor->getPropertyValue("CharHeight") >>= fCharHeight;
                    }
                }
                if (!isFound && fCharHeight <= 1.0)
                {
                    // If still not found, see if the last paragraph is ~invisible, and work with
                    // the last-in-practice paragraph.
                    xPCursor->gotoPreviousParagraph(false);
                    uno::Reference<beans::XPropertySet> xPropertySet(xCursor, uno::UNO_QUERY_THROW);
                    OUString aPageDescName;
                    if ((xPropertySet->getPropertyValue("PageDescName") >>= aPageDescName)
                        && !aPageDescName.isEmpty())
                    {
                        uno::Reference<beans::XPropertySet> xPageStyle(
                            rDM_Impl.GetPageStyles()->getByName(aPageDescName), uno::UNO_QUERY);
                        xPageStyle->setPropertyValue("FollowStyle",
                                                     uno::makeAny(m_sFollowPageStyleName));
                    }
                }
            }
        }
        catch ( const uno::Exception& )
        {
            SAL_WARN( "writerfilter", "failed to set PageDescName!" );
        }
    }
    // If the section is of type "New column" (0x01), then simply insert a column break.
    // But only if there actually are columns on the page, otherwise a column break
    // seems to be handled like a page break by MSO.
    else if (m_nBreakType == static_cast<sal_Int32>(NS_ooxml::LN_Value_ST_SectionMark_nextColumn)
            && m_nColumnCount > 1 && !rDM_Impl.IsInComments())
    {
        try
        {
            InheritOrFinalizePageStyles( rDM_Impl );
            /*TODO tdf#135343: Just inserting a column break sounds like the right idea, but the implementation is wrong.
             * Somehow, the previous column section needs to be extended to cover this new text.
             * Currently, it is completely broken, producing a no-column section that starts on a new page.
             */
            uno::Reference< beans::XPropertySet > xRangeProperties;
            if ( m_xStartingRange.is() )
            {
                xRangeProperties.set( m_xStartingRange, uno::UNO_QUERY_THROW );
            }
            else
            {
                //set the start value at the beginning of the document
                xRangeProperties.set( rDM_Impl.GetTextDocument()->getText()->getStart(), uno::UNO_QUERY_THROW );
            }
            xRangeProperties->setPropertyValue( getPropertyName( PROP_BREAK_TYPE ), uno::makeAny( style::BreakType_COLUMN_BEFORE ) );
        }
        catch ( const uno::Exception& ) {}
    }
    else if (!rDM_Impl.IsInComments())
    {
        uno::Reference< beans::XPropertySet > xSection;
        ApplyProtectionProperties( xSection, rDM_Impl );

        //get the properties and create appropriate page styles
        uno::Reference< beans::XPropertySet > xFollowPageStyle;
        //This part certainly is not needed for footnotes, so don't create unused page styles.
        if ( !rDM_Impl.IsInFootOrEndnote() )
        {
            xFollowPageStyle.set( GetPageStyle( rDM_Impl, false ) );

            HandleMarginsHeaderFooter(/*bFirstPage=*/false, rDM_Impl );
        }

        if ( rDM_Impl.GetSettingsTable()->GetMirrorMarginSettings() )
        {
            Insert( PROP_PAGE_STYLE_LAYOUT, uno::makeAny( style::PageStyleLayout_MIRRORED ) );
        }
        uno::Reference< text::XTextColumns > xColumns;
        if ( m_nColumnCount > 1 )
        {
            // prefer setting column properties into a section, not a page style if at all possible.
            if ( !xSection.is() )
                xSection = rDM_Impl.appendTextSectionAfter( m_xStartingRange );
            if ( xSection.is() )
                ApplyColumnProperties( xSection, rDM_Impl );
            else if ( xFollowPageStyle.is() )
                xColumns = ApplyColumnProperties( xFollowPageStyle, rDM_Impl );
        }

        // these BreakTypes are effectively page-breaks: don't evenly distribute text in columns before a page break;
        if ( pPrevSection && pPrevSection->ColumnCount() )
            pPrevSection->DontBalanceTextColumns();

        //prepare text grid properties
        sal_Int32 nHeight = 1;
        std::optional< PropertyMap::Property > pProp = getProperty( PROP_HEIGHT );
        if ( pProp )
            pProp->second >>= nHeight;

        sal_Int32 nWidth = 1;
        pProp = getProperty( PROP_WIDTH );
        if ( pProp )
            pProp->second >>= nWidth;

        text::WritingMode eWritingMode = text::WritingMode_LR_TB;
        pProp = getProperty( PROP_WRITING_MODE );
        if ( pProp )
            pProp->second >>= eWritingMode;

        sal_Int32 nTextAreaHeight = eWritingMode == text::WritingMode_LR_TB ?
            nHeight - m_nTopMargin - m_nBottomMargin :
            nWidth - m_nLeftMargin - m_nRightMargin;

        sal_Int32 nGridLinePitch = m_nGridLinePitch;
        //sep.dyaLinePitch
        if ( nGridLinePitch < 1 || nGridLinePitch > 31680 )
        {
            SAL_WARN( "writerfilter", "sep.dyaLinePitch outside legal range: " << nGridLinePitch );
            nGridLinePitch = 1;
        }

        const sal_Int32 nGridLines = nTextAreaHeight / nGridLinePitch;
        if ( nGridLines >= 0 && nGridLines <= SAL_MAX_INT16 )
            Insert( PROP_GRID_LINES, uno::makeAny( sal_Int16(nGridLines) ) );

        // PROP_GRID_MODE
        Insert( PROP_GRID_MODE, uno::makeAny( static_cast<sal_Int16> (m_nGridType) ) );
        if ( m_nGridType == text::TextGridMode::LINES_AND_CHARS )
        {
            Insert( PROP_GRID_SNAP_TO_CHARS, uno::makeAny( m_bGridSnapToChars ) );
        }

        sal_Int32 nCharWidth = 423; //240 twip/ 12 pt
        const StyleSheetEntryPtr pEntry = rDM_Impl.GetStyleSheetTable()->FindStyleSheetByConvertedStyleName( "Standard" );
        if ( pEntry )
        {
            std::optional< PropertyMap::Property > pPropHeight = pEntry->pProperties->getProperty( PROP_CHAR_HEIGHT_ASIAN );
            if ( pPropHeight )
            {
                double fHeight = 0;
                if ( pPropHeight->second >>= fHeight )
                    nCharWidth = ConversionHelper::convertTwipToMM100( static_cast<tools::Long>(fHeight * 20.0 + 0.5) );
            }
        }

        //dxtCharSpace
        if ( m_nDxtCharSpace )
        {
            sal_Int32 nCharSpace = m_nDxtCharSpace;
            //main lives in top 20 bits, and is signed.
            sal_Int32 nMain = (nCharSpace & 0xFFFFF000);
            nMain /= 0x1000;
            nCharWidth += ConversionHelper::convertTwipToMM100( nMain * 20 );

            sal_Int32 nFraction = (nCharSpace & 0x00000FFF);
            nFraction = (nFraction * 20) / 0xFFF;
            nCharWidth += ConversionHelper::convertTwipToMM100( nFraction );
        }

        if ( m_nPageNumberType >= 0 )
            Insert( PROP_NUMBERING_TYPE, uno::makeAny( m_nPageNumberType ) );

        // #i119558#, force to set document as standard page mode,
        // refer to ww8 import process function "SwWW8ImplReader::SetDocumentGrid"
        try
        {
            uno::Reference< beans::XPropertySet > xDocProperties;
            xDocProperties.set( rDM_Impl.GetTextDocument(), uno::UNO_QUERY_THROW );
            Insert(PROP_GRID_STANDARD_MODE, uno::makeAny(true));
            xDocProperties->setPropertyValue("DefaultPageMode", uno::makeAny(false));
        }
        catch ( const uno::Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("writerfilter.dmapper", "Exception in SectionPropertyMap::CloseSectionGroup");
        }

        Insert( PROP_GRID_BASE_HEIGHT, uno::makeAny( nGridLinePitch ) );
        Insert( PROP_GRID_BASE_WIDTH, uno::makeAny( nCharWidth ) );
        Insert( PROP_GRID_RUBY_HEIGHT, uno::makeAny( sal_Int32( 0 ) ) );

        if ( rDM_Impl.IsNewDoc() && xFollowPageStyle.is() )
            ApplyProperties_( xFollowPageStyle );

        //todo: creating a "First Page" style depends on HasTitlePage and _fFacingPage_
        if ( m_bTitlePage )
        {
            CopyLastHeaderFooter( true, rDM_Impl );
            PrepareHeaderFooterProperties( true );
            uno::Reference< beans::XPropertySet > xFirstPageStyle = GetPageStyle(
                rDM_Impl, true );
            if ( rDM_Impl.IsNewDoc() )
                ApplyProperties_( xFirstPageStyle );

            if ( xColumns.is() )
                xFirstPageStyle->setPropertyValue(
                    getPropertyName( PROP_TEXT_COLUMNS ), uno::makeAny( xColumns ) );
        }

        ApplyBorderToPageStyles( rDM_Impl, m_eBorderApply, m_eBorderOffsetFrom );

        try
        {
            //now apply this break at the first paragraph of this section
            uno::Reference< beans::XPropertySet > xRangeProperties( lcl_GetRangeProperties( m_bIsFirstSection, rDM_Impl, m_xStartingRange ) );

            // Handle page breaks with odd/even page numbering. We need to use an extra page style for setting the page style
            // to left/right, because if we set it to the normal style, we'd set it to "First Page"/"Default Style", which would
            // break them (all default pages would be only left or right).
            if ( m_nBreakType == static_cast<sal_Int32>(NS_ooxml::LN_Value_ST_SectionMark_evenPage) || m_nBreakType == static_cast<sal_Int32>(NS_ooxml::LN_Value_ST_SectionMark_oddPage) )
            {
                OUString* pageStyle = m_bTitlePage ? &m_sFirstPageStyleName : &m_sFollowPageStyleName;
                OUString evenOddStyleName = rDM_Impl.GetUnusedPageStyleName();
                uno::Reference< beans::XPropertySet > evenOddStyle(
                    rDM_Impl.GetTextFactory()->createInstance( "com.sun.star.style.PageStyle" ),
                    uno::UNO_QUERY );
                // Unfortunately using setParent() does not work for page styles, so make a deep copy of the page style.
                uno::Reference< beans::XPropertySet > pageProperties( m_bTitlePage ? m_aFirstPageStyle : m_aFollowPageStyle );
                uno::Reference< beans::XPropertySetInfo > pagePropertiesInfo( pageProperties->getPropertySetInfo() );
                const uno::Sequence< beans::Property > propertyList( pagePropertiesInfo->getProperties() );
                // Ignore write-only properties.
                static const o3tl::sorted_vector<OUString> aDenylist
                    = { "FooterBackGraphicURL", "BackGraphicURL", "HeaderBackGraphicURL" };
                for ( const auto& rProperty : propertyList )
                {
                    if ( (rProperty.Attributes & beans::PropertyAttribute::READONLY) == 0 )
                    {
                        if (aDenylist.find(rProperty.Name) == aDenylist.end())
                            evenOddStyle->setPropertyValue(
                                rProperty.Name,
                                pageProperties->getPropertyValue(rProperty.Name));
                    }
                }
                evenOddStyle->setPropertyValue( "FollowStyle", uno::makeAny( *pageStyle ) );
                rDM_Impl.GetPageStyles()->insertByName( evenOddStyleName, uno::makeAny( evenOddStyle ) );
                evenOddStyle->setPropertyValue( "HeaderIsOn", uno::makeAny( false ) );
                evenOddStyle->setPropertyValue( "FooterIsOn", uno::makeAny( false ) );
                CopyHeaderFooter( pageProperties, evenOddStyle );
                *pageStyle = evenOddStyleName; // And use it instead of the original one (which is set as follow of this one).
                if ( m_nBreakType == static_cast<sal_Int32>(NS_ooxml::LN_Value_ST_SectionMark_evenPage) )
                    evenOddStyle->setPropertyValue( getPropertyName( PROP_PAGE_STYLE_LAYOUT ), uno::makeAny( style::PageStyleLayout_LEFT ) );
                else if ( m_nBreakType == static_cast<sal_Int32>(NS_ooxml::LN_Value_ST_SectionMark_oddPage) )
                    evenOddStyle->setPropertyValue( getPropertyName( PROP_PAGE_STYLE_LAYOUT ), uno::makeAny( style::PageStyleLayout_RIGHT ) );
            }

            if (rDM_Impl.m_xAltChunkStartingRange.is())
            {
                xRangeProperties.set(rDM_Impl.m_xAltChunkStartingRange, uno::UNO_QUERY);
            }
            if (xRangeProperties.is() && (rDM_Impl.IsNewDoc() || rDM_Impl.IsAltChunk()))
            {
                // Avoid setting page style in case of autotext: so inserting the autotext at the
                // end of the document does not introduce an unwanted page break.
                // Also avoid setting the page style at the very beginning if it still is the default page style.
                const OUString sPageStyle = m_bTitlePage ? m_sFirstPageStyleName : m_sFollowPageStyleName;
                if (!rDM_Impl.IsReadGlossaries()
                    && !rDM_Impl.IsInFootOrEndnote()
                    && !(m_bIsFirstSection && sPageStyle == getPropertyName( PROP_STANDARD ) && m_nPageNumber < 0)
                   )
                {
                    xRangeProperties->setPropertyValue(
                        getPropertyName( PROP_PAGE_DESC_NAME ),
                        uno::makeAny(sPageStyle) );
                }

                if (0 <= m_nPageNumber)
                {
                    sal_Int16 nPageNumber = static_cast< sal_Int16 >(m_nPageNumber);
                    xRangeProperties->setPropertyValue(getPropertyName(PROP_PAGE_NUMBER_OFFSET),
                        uno::makeAny(nPageNumber));
                }
            }
        }
        catch ( const uno::Exception& )
        {
            OSL_FAIL( "Exception in SectionPropertyMap::CloseSectionGroup" );
        }
    }

    // Now that the margins are known, resize relative width shapes because some shapes in LO do not support percentage-sizes
    sal_Int32 nParagraphWidth = GetPageWidth() - m_nLeftMargin - m_nRightMargin;
    if ( m_nColumnCount > 1 )
    {
        // skip custom-width columns since we don't know what column the shape is in.
        if ( !m_aColWidth.empty() )
            nParagraphWidth = 0;
        else
            nParagraphWidth = (nParagraphWidth - (m_nColumnDistance * (m_nColumnCount - 1))) / m_nColumnCount;
    }
    if ( nParagraphWidth > 0 )
    {
        const OUString sPropRelativeWidth = getPropertyName(PROP_RELATIVE_WIDTH);
        for ( const auto& xShape : m_xRelativeWidthShapes )
        {
            const uno::Reference<beans::XPropertySet> xShapePropertySet( xShape, uno::UNO_QUERY );
            if ( xShapePropertySet->getPropertySetInfo()->hasPropertyByName(sPropRelativeWidth) )
            {
                sal_uInt16 nPercent = 0;
                try
                {
                    xShapePropertySet->getPropertyValue(sPropRelativeWidth) >>= nPercent;
                }
                catch (const css::uno::RuntimeException& e)
                {
                    // May happen e.g. when text frame has no frame format
                    // See sw/qa/extras/ooxmlimport/data/n779627.docx
                    SAL_WARN("writerfilter", "Getting relative width failed. " << e.Message);
                }
                if ( nPercent )
                {
                    const sal_Int32 nWidth = nParagraphWidth * nPercent / 100;
                    xShape->setSize( awt::Size( nWidth, xShape->getSize().Height ) );
                }
            }
        }
    }
    m_xRelativeWidthShapes.clear();

    rDM_Impl.SetIsLastSectionGroup( false );
    rDM_Impl.SetIsFirstParagraphInSection( true );

    if ( !rDM_Impl.IsInFootOrEndnote() && !rDM_Impl.IsInComments() )
    {
        rDM_Impl.m_bHasFtn = false;
        rDM_Impl.m_bHasFtnSep = false;
    }
}

// Clear the flag that says we should take the header/footer content from
// the previous section.  This should be called when we encounter a header
// or footer definition for this section.
void SectionPropertyMap::ClearHeaderFooterLinkToPrevious( bool bHeader, PageType eType )
{
    if ( bHeader )
    {
        switch ( eType )
        {
            case PAGE_FIRST: m_bFirstPageHeaderLinkToPrevious = false; break;
            case PAGE_LEFT:  m_bEvenPageHeaderLinkToPrevious = false; break;
            case PAGE_RIGHT: m_bDefaultHeaderLinkToPrevious = false; break;
                // no default case as all enumeration values have been covered
        }
    }
    else
    {
        switch ( eType )
        {
            case PAGE_FIRST: m_bFirstPageFooterLinkToPrevious = false; break;
            case PAGE_LEFT:  m_bEvenPageFooterLinkToPrevious = false; break;
            case PAGE_RIGHT: m_bDefaultFooterLinkToPrevious = false; break;
        }
    }
}

namespace {

class NamedPropertyValue
{
private:
    OUString m_aName;

public:
    explicit NamedPropertyValue( const OUString& i_aStr )
        : m_aName( i_aStr )
    {
    }

    bool operator() ( beans::PropertyValue const & aVal )
    {
        return aVal.Name == m_aName;
    }
};

}

void SectionPropertyMap::ApplyProperties_( const uno::Reference< beans::XPropertySet >& xStyle )
{
    uno::Reference< beans::XMultiPropertySet > const xMultiSet( xStyle, uno::UNO_QUERY );

    std::vector< OUString > vNames;
    std::vector< uno::Any > vValues;
    {
        // Convert GetPropertyValues() value into something useful
        uno::Sequence< beans::PropertyValue > vPropVals = GetPropertyValues();

        //Temporarily store the items that are in grab bags
        uno::Sequence< beans::PropertyValue > vCharVals;
        uno::Sequence< beans::PropertyValue > vParaVals;
        beans::PropertyValue* pCharGrabBag = std::find_if( vPropVals.begin(), vPropVals.end(), NamedPropertyValue( "CharInteropGrabBag" ) );
        if ( pCharGrabBag != vPropVals.end() )
            (pCharGrabBag->Value) >>= vCharVals;
        beans::PropertyValue* pParaGrabBag = std::find_if( vPropVals.begin(), vPropVals.end(), NamedPropertyValue( "ParaInteropGrabBag" ) );
        if ( pParaGrabBag != vPropVals.end() )
            (pParaGrabBag->Value) >>= vParaVals;

        for ( beans::PropertyValue* pIter = vPropVals.begin(); pIter != vPropVals.end(); ++pIter )
        {
            if ( pIter != pCharGrabBag && pIter != pParaGrabBag
                 && pIter->Name != "IsProtected" //section-only property
               )
            {
                vNames.push_back( pIter->Name );
                vValues.push_back( pIter->Value );
            }
        }
        for ( const beans::PropertyValue & v : std::as_const(vCharVals) )
        {
            vNames.push_back( v.Name );
            vValues.push_back( v.Value );
        }
        for ( const beans::PropertyValue & v : std::as_const(vParaVals) )
        {
            vNames.push_back( v.Name );
            vValues.push_back( v.Value );
        }
    }
    if ( xMultiSet.is() )
    {
        try
        {
            xMultiSet->setPropertyValues( comphelper::containerToSequence( vNames ), comphelper::containerToSequence( vValues ) );
            return;
        }
        catch ( const uno::Exception& )
        {
            OSL_FAIL( "Exception in SectionPropertyMap::ApplyProperties_" );
        }
    }
    for ( size_t i = 0; i < vNames.size(); ++i )
    {
        try
        {
            if ( xStyle.is() )
                xStyle->setPropertyValue( vNames[i], vValues[i] );
        }
        catch ( const uno::Exception& )
        {
            OSL_FAIL( "Exception in SectionPropertyMap::ApplyProperties_" );
        }
    }
}

sal_Int32 SectionPropertyMap::GetPageWidth() const
{
    return getProperty( PROP_WIDTH )->second.get<sal_Int32>();
}

StyleSheetPropertyMap::StyleSheetPropertyMap()
    : mnListLevel( -1 )
    , mnOutlineLevel( -1 )
{
}

ParagraphProperties::ParagraphProperties()
    : m_bFrameMode( false )
    , m_nDropCap( NS_ooxml::LN_Value_doc_ST_DropCap_none )
    , m_nLines( 0 )
    , m_w( -1 )
    , m_h( -1 )
    , m_nWrap( text::WrapTextMode::WrapTextMode_MAKE_FIXED_SIZE )
    , m_hAnchor( -1 )
    , m_vAnchor( -1 )
    , m_x( -1 )
    , m_bxValid( false )
    , m_y( -1 )
    , m_byValid( false )
    , m_hSpace( -1 )
    , m_vSpace( -1 )
    , m_hRule( -1 )
    , m_xAlign( -1 )
    , m_yAlign( -1 )
    , m_nDropCapLength( 0 )
{
}

bool ParagraphProperties::operator==( const ParagraphProperties& rCompare )
{
    return ( m_bFrameMode  == rCompare.m_bFrameMode &&
             m_nDropCap    == rCompare.m_nDropCap &&
             m_nLines      == rCompare.m_nLines &&
             m_w           == rCompare.m_w &&
             m_h           == rCompare.m_h &&
             m_nWrap       == rCompare.m_nWrap &&
             m_hAnchor     == rCompare.m_hAnchor &&
             m_vAnchor     == rCompare.m_vAnchor &&
             m_x           == rCompare.m_x &&
             m_bxValid     == rCompare.m_bxValid &&
             m_y           == rCompare.m_y &&
             m_byValid     == rCompare.m_byValid &&
             m_hSpace      == rCompare.m_hSpace &&
             m_vSpace      == rCompare.m_vSpace &&
             m_hRule       == rCompare.m_hRule &&
             m_xAlign      == rCompare.m_xAlign &&
             m_yAlign      == rCompare.m_yAlign );
}

void ParagraphProperties::ResetFrameProperties()
{
    m_bFrameMode     = false;
    m_nDropCap       = NS_ooxml::LN_Value_doc_ST_DropCap_none;
    m_nLines         = 0;
    m_w              = -1;
    m_h              = -1;
    m_nWrap          = text::WrapTextMode::WrapTextMode_MAKE_FIXED_SIZE;
    m_hAnchor        = -1;
    m_vAnchor        = -1;
    m_x              = -1;
    m_bxValid        = false;
    m_y              = -1;
    m_byValid        = false;
    m_hSpace         = -1;
    m_vSpace         = -1;
    m_hRule          = -1;
    m_xAlign         = -1;
    m_yAlign         = -1;
    m_nDropCapLength = 0;
}

bool TablePropertyMap::getValue( TablePropertyMapTarget eWhich, sal_Int32& nFill )
{
    if ( eWhich < TablePropertyMapTarget_MAX )
    {
        if ( m_aValidValues[eWhich].bValid )
            nFill = m_aValidValues[eWhich].nValue;
        return m_aValidValues[eWhich].bValid;
    }
    else
    {
        OSL_FAIL( "invalid TablePropertyMapTarget" );
        return false;
    }
}

void TablePropertyMap::setValue( TablePropertyMapTarget eWhich, sal_Int32 nSet )
{
    if ( eWhich < TablePropertyMapTarget_MAX )
    {
        m_aValidValues[eWhich].bValid = true;
        m_aValidValues[eWhich].nValue = nSet;
    }
    else
        OSL_FAIL( "invalid TablePropertyMapTarget" );
}

void TablePropertyMap::insertTableProperties( const PropertyMap* pMap, const bool bOverwrite )
{
#ifdef DBG_UTIL
    TagLogger::getInstance().startElement( "TablePropertyMap.insertTableProperties" );
    pMap->dumpXml();
#endif

    const TablePropertyMap* pSource = dynamic_cast< const TablePropertyMap* >(pMap);
    if ( pSource )
    {
        for ( sal_Int32 eTarget = TablePropertyMapTarget_START;
            eTarget < TablePropertyMapTarget_MAX; ++eTarget )
        {
            if ( pSource->m_aValidValues[eTarget].bValid && (bOverwrite || !m_aValidValues[eTarget].bValid) )
            {
                m_aValidValues[eTarget].bValid = true;
                m_aValidValues[eTarget].nValue = pSource->m_aValidValues[eTarget].nValue;
            }
        }
    }

#ifdef DBG_UTIL
    dumpXml();
    TagLogger::getInstance().endElement();
#endif
}

} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
