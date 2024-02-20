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

#include <string_view>
#include <unordered_set>

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
#include <comphelper/diagnose_ex.hxx>
#include "PropertyMapHelper.hxx"
#include <o3tl/sorted_vector.hxx>
#include <o3tl/unit_conversion.hxx>
#include <utility>

using namespace com::sun::star;

namespace writerfilter::dmapper {

uno::Sequence< beans::PropertyValue > PropertyMap::GetPropertyValues( bool bCharGrabBag )
{
    using comphelper::makePropertyValue;

    if ( !m_aValues.empty() || m_vMap.empty() )
        return comphelper::containerToSequence( m_aValues );

    size_t nCharGrabBag = 0;
    size_t nParaGrabBag = 0;
    size_t nCellGrabBag = 0;
    size_t nRowGrabBag  = 0;

    const PropValue* pParaStyleProp = nullptr;
    const PropValue* pCharStyleProp = nullptr;
    const PropValue* pNumRuleProp   = nullptr;

    m_aValues.reserve( m_vMap.size() );
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
                std::copy(std::cbegin(aSeq), std::cend(aSeq), pCellGrabBagValues + nCellGrabBagValue);
                nCellGrabBagValue += aSeq.getLength();
            }
            else
            {
                m_aValues.push_back( makePropertyValue( getPropertyName( rPropPair.first ), rPropPair.second.getValue() ) );
            }
        }
    }

    if ( nCharGrabBag && bCharGrabBag )
        m_aValues.push_back( makePropertyValue( "CharInteropGrabBag", uno::Any( aCharGrabBagValues ) ) );

    if ( nParaGrabBag )
        m_aValues.push_back( makePropertyValue( "ParaInteropGrabBag", uno::Any( aParaGrabBagValues ) ) );

    if ( nCellGrabBag )
        m_aValues.push_back( makePropertyValue( "CellInteropGrabBag", uno::Any( aCellGrabBagValues ) ) );

    if ( nRowGrabBag )
        m_aValues.push_back( makePropertyValue( "RowInteropGrabBag", uno::Any( aRowGrabBagValues ) ) );

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
                    TagLogger::getInstance().attribute( "stringValue", std::u16string_view() );
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
    , m_nLeftMargin( o3tl::convert(1, o3tl::Length::in, o3tl::Length::mm100) )
    , m_nRightMargin( o3tl::convert(1, o3tl::Length::in, o3tl::Length::mm100) )
    , m_nGutterMargin(0)
    , m_nTopMargin( o3tl::convert(1, o3tl::Length::in, o3tl::Length::mm100) )
    , m_nBottomMargin( o3tl::convert(1, o3tl::Length::in, o3tl::Length::mm100) )
    , m_nHeaderTop( o3tl::convert(0.5, o3tl::Length::in, o3tl::Length::mm100) )
    , m_nHeaderBottom( o3tl::convert(0.5, o3tl::Length::in, o3tl::Length::mm100) )
    , m_nGridType( 0 )
    , m_nGridLinePitch( 1 )
    , m_nDxtCharSpace( 0 )
    , m_bGridSnapToChars( true )
    , m_nLnnMod( 0 )
    , m_nLnc(NS_ooxml::LN_Value_ST_LineNumberRestart_newPage)
    , m_ndxaLnn( 0 )
    , m_nLnnMin( 0 )
    , m_nPaperSourceFirst( 0 )
    , m_nPaperSourceOther( 0 )
    , m_bDynamicHeightTop( true )
    , m_bDynamicHeightBottom( true )
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
    Insert( PROP_HEIGHT, uno::Any( static_cast<sal_Int32>(aLetter.getHeight()) ) );
    // page width, 1/100mm
    Insert( PROP_WIDTH, uno::Any( static_cast<sal_Int32>(aLetter.getWidth()) ) );
    // page left margin, 1/100 mm
    Insert( PROP_LEFT_MARGIN, uno::Any( sal_Int32(o3tl::convert(1, o3tl::Length::in, o3tl::Length::mm100)) ) );
    // page right margin, 1/100 mm
    Insert( PROP_RIGHT_MARGIN, uno::Any( sal_Int32(o3tl::convert(1, o3tl::Length::in, o3tl::Length::mm100)) ) );
    // page top margin, 1/100 mm
    Insert( PROP_TOP_MARGIN, uno::Any( sal_Int32(o3tl::convert(1, o3tl::Length::in, o3tl::Length::mm100)) ) );
    // page bottom margin, 1/100 mm
    Insert( PROP_BOTTOM_MARGIN, uno::Any( sal_Int32(o3tl::convert(1, o3tl::Length::in, o3tl::Length::mm100)) ) );
    // page style layout
    Insert( PROP_PAGE_STYLE_LAYOUT, uno::Any( style::PageStyleLayout_ALL ) );
    uno::Any aFalse( uno::Any( false ) );
    Insert( PROP_GRID_DISPLAY, aFalse );
    Insert( PROP_GRID_PRINT, aFalse );
    Insert( PROP_GRID_MODE, uno::Any( text::TextGridMode::NONE ) );

    if ( m_bIsFirstSection )
    {
        m_sPageStyleName = getPropertyName(PROP_STANDARD);
    }
}

uno::Reference<beans::XPropertySet> SectionPropertyMap::GetPageStyle(DomainMapper_Impl& rDM_Impl)
{
    const uno::Reference< container::XNameContainer >& xPageStyles = rDM_Impl.GetPageStyles();
    const uno::Reference < lang::XMultiServiceFactory >& xTextFactory = rDM_Impl.GetTextFactory();
    uno::Reference<beans::XPropertySet> xReturnPageStyle;
    try
    {
        if (m_sPageStyleName.isEmpty() && xPageStyles.is())
        {
            assert( !rDM_Impl.IsInFootOrEndnote() && "Don't create useless page styles" );

            m_sPageStyleName = rDM_Impl.GetUnusedPageStyleName();

            m_aPageStyle.set(xTextFactory->createInstance("com.sun.star.style.PageStyle"), uno::UNO_QUERY );
            xPageStyles->insertByName(m_sPageStyleName, uno::Any(m_aPageStyle));
        }
        else if (!m_aPageStyle.is() && xPageStyles.is())
        {
            xPageStyles->getByName(m_sPageStyleName) >>= m_aPageStyle;
        }
        xReturnPageStyle = m_aPageStyle;
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION( "writerfilter" );
    }

    return xReturnPageStyle;
}

// removes the content - all the paragraphs of an input XText
void SectionPropertyMap::removeXTextContent(uno::Reference<text::XText> const& rxText)
{
    uno::Reference<text::XText> xText(rxText);
    if (!xText.is())
        return;
    xText->setString(OUString());
    uno::Reference<text::XParagraphAppend> const xAppend(xText, uno::UNO_QUERY_THROW);
    uno::Reference<lang::XComponent> const xParagraph(xAppend->finishParagraph(uno::Sequence<beans::PropertyValue>()), uno::UNO_QUERY_THROW);
    xParagraph->dispose();
}

/** Set the header/footer sharing as defined by titlePage and evenAndOdd flags
 *  in the document and clear the content of anything not written during the import.
 */
void SectionPropertyMap::setHeaderFooterProperties(DomainMapper_Impl& rDM_Impl)
{
    if (!m_aPageStyle.is())
        return;

    bool bHasHeader = false;
    bool bHasFooter = false;

    const OUString& sHeaderIsOn = getPropertyName(PROP_HEADER_IS_ON);
    const OUString& sFooterIsOn = getPropertyName(PROP_FOOTER_IS_ON);

    m_aPageStyle->getPropertyValue(sHeaderIsOn) >>= bHasHeader;
    m_aPageStyle->getPropertyValue(sFooterIsOn) >>= bHasFooter;

    bool bEvenAndOdd = rDM_Impl.GetSettingsTable()->GetEvenAndOddHeaders();

    if (bHasHeader && !m_bLeftHeader && !bEvenAndOdd)
    {
        auto aAny = m_aPageStyle->getPropertyValue(getPropertyName(PROP_HEADER_TEXT_LEFT));
        uno::Reference<text::XText> xText(aAny, uno::UNO_QUERY);
        if (xText.is())
            SectionPropertyMap::removeXTextContent(xText);
    }

    if (bHasFooter && !m_bLeftFooter && !bEvenAndOdd)
    {
        auto aAny = m_aPageStyle->getPropertyValue(getPropertyName(PROP_FOOTER_TEXT_LEFT));
        uno::Reference<text::XText> xText(aAny, uno::UNO_QUERY);
        if (xText.is())
            SectionPropertyMap::removeXTextContent(xText);
    }

    if (bHasHeader && !m_bFirstHeader && !m_bTitlePage)
    {
        auto aAny = m_aPageStyle->getPropertyValue(getPropertyName(PROP_HEADER_TEXT_FIRST));
        uno::Reference<text::XText> xText(aAny, uno::UNO_QUERY);
        if (xText.is())
            SectionPropertyMap::removeXTextContent(xText);
    }

    if (bHasFooter && !m_bFirstFooter && !m_bTitlePage)
    {
        auto aAny = m_aPageStyle->getPropertyValue(getPropertyName(PROP_FOOTER_TEXT_FIRST));
        uno::Reference<text::XText> xText(aAny, uno::UNO_QUERY);
        if (xText.is())
            SectionPropertyMap::removeXTextContent(xText);
    }

    m_aPageStyle->setPropertyValue(getPropertyName(PROP_HEADER_IS_SHARED), uno::Any(!bEvenAndOdd));
    m_aPageStyle->setPropertyValue(getPropertyName(PROP_FOOTER_IS_SHARED), uno::Any(!bEvenAndOdd));
    m_aPageStyle->setPropertyValue(getPropertyName(PROP_FIRST_IS_SHARED), uno::Any(!m_bTitlePage));

    bool bHadFirstHeader = m_bHadFirstHeader && m_bTitlePage;
    if (bHasHeader && !bHadFirstHeader && !m_bHadLeftHeader && !m_bHadRightHeader)
    {
        m_aPageStyle->setPropertyValue(sHeaderIsOn, uno::Any(false));
    }
}

void SectionPropertyMap::SetBorder( BorderPosition ePos, sal_Int32 nLineDistance, const table::BorderLine2& rBorderLine, bool bShadow )
{
    m_oBorderLines[ePos]     = rBorderLine;
    m_nBorderDistances[ePos] = nLineDistance;
    m_bBorderShadows[ePos]   = bShadow;
}

void SectionPropertyMap::ApplyPaperSource(DomainMapper_Impl& rDM_Impl)
{
    uno::Reference<beans::XPropertySet> xFirst;
    // todo: negative spacing (from ww8par6.cxx)
    if (!m_sPageStyleName.isEmpty())
    {
        xFirst = GetPageStyle(rDM_Impl);
        if ( xFirst.is() )
            try
            {
                //TODO: which of the two tray values needs to be set? first/other - the interfaces requires the name of the tray!
                xFirst->setPropertyValue(getPropertyName(PROP_PAPER_TRAY),
                                         uno::Any(m_nPaperSourceFirst));
            }
            catch (const uno::Exception&)
            {
                TOOLS_WARN_EXCEPTION("writerfilter", "Paper source not found");
            }
    }
}

void SectionPropertyMap::ApplyBorderToPageStyles( DomainMapper_Impl& rDM_Impl,
                                                  BorderApply /*eBorderApply*/, BorderOffsetFrom eOffsetFrom )
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

    uno::Reference<beans::XPropertySet> xFirst;
    // todo: negative spacing (from ww8par6.cxx)
    if (!m_sPageStyleName.isEmpty())
        xFirst = GetPageStyle(rDM_Impl);

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
            const OUString & sBorderName = getPropertyName( aBorderIds[nBorder] );
            if ( xFirst.is() )
                xFirst->setPropertyValue( sBorderName, uno::Any( *m_oBorderLines[nBorder] ) );
        }
        if ( m_nBorderDistances[nBorder] >= 0 )
        {
            sal_uInt32 nLineWidth = 0;
            if ( m_oBorderLines[nBorder] )
                nLineWidth = m_oBorderLines[nBorder]->LineWidth;
            if ( xFirst.is() )
                SetBorderDistance( xFirst, aMarginIds[nBorder], aBorderDistanceIds[nBorder],
                    m_nBorderDistances[nBorder], eOffsetFrom, nLineWidth, rDM_Impl );
        }
    }

    if ( m_bBorderShadows[BORDER_RIGHT] )
    {
        table::ShadowFormat aFormat = getShadowFromBorder( *m_oBorderLines[BORDER_RIGHT] );
        if ( xFirst.is() )
            xFirst->setPropertyValue( getPropertyName( PROP_SHADOW_FORMAT ), uno::Any( aFormat ) );
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
                                            sal_uInt32 nLineWidth,
                                            DomainMapper_Impl& rDM_Impl )
{
    if (!xStyle.is())
        return;
    const OUString & sMarginName = getPropertyName( eMarginId );
    const OUString & sBorderDistanceName = getPropertyName( eDistId );
    uno::Any aMargin = xStyle->getPropertyValue( sMarginName );
    sal_Int32 nMargin = 0;
    aMargin >>= nMargin;
    editeng::BorderDistanceFromWord(eOffsetFrom == BorderOffsetFrom::Edge, nMargin, nDistance,
                                    nLineWidth);

    if (eOffsetFrom == BorderOffsetFrom::Edge)
    {
        uno::Any aGutterMargin = xStyle->getPropertyValue( "GutterMargin" );
        sal_Int32 nGutterMargin = 0;
        aGutterMargin >>= nGutterMargin;

        if (eMarginId == PROP_LEFT_MARGIN && !rDM_Impl.GetSettingsTable()->GetGutterAtTop())
        {
            nMargin -= nGutterMargin;
            nDistance += nGutterMargin;
        }

        if (eMarginId == PROP_TOP_MARGIN && rDM_Impl.GetSettingsTable()->GetGutterAtTop())
        {
            nMargin -= nGutterMargin;
            nDistance += nGutterMargin;
        }
    }

    // Change the margins with the border distance
    uno::Reference< beans::XMultiPropertySet > xMultiSet( xStyle, uno::UNO_QUERY_THROW );
    uno::Sequence<OUString> aProperties { sMarginName, sBorderDistanceName };
    uno::Sequence<uno::Any> aValues { uno::Any( nMargin ), uno::Any( nDistance ) };
    xMultiSet->setPropertyValues( aProperties, aValues );
}

void SectionPropertyMap::DontBalanceTextColumns()
{
    try
    {
        if ( m_xColumnContainer.is() )
            m_xColumnContainer->setPropertyValue( "DontBalanceTextColumns", uno::Any( true ) );
    }
    catch ( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION( "writerfilter", "SectionPropertyMap::DontBalanceTextColumns" );
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
                xSection->setPropertyValue( getPropertyName(PROP_IS_PROTECTED), uno::Any(bIsProtected) );
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
        const OUString & sTextColumns = getPropertyName( PROP_TEXT_COLUMNS );
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
            xColumnPropSet->setPropertyValue( getPropertyName( PROP_AUTOMATIC_DISTANCE ), uno::Any( m_nColumnDistance ) );
        }

        if ( m_bSeparatorLineIsOn )
        {
            xColumnPropSet->setPropertyValue( "SeparatorLineIsOn", uno::Any( true ) );
            xColumnPropSet->setPropertyValue( "SeparatorLineVerticalAlignment", uno::Any( style::VerticalAlignment_TOP ) );
            xColumnPropSet->setPropertyValue( "SeparatorLineRelativeHeight", uno::Any( static_cast<sal_Int8>(100) ) );
            xColumnPropSet->setPropertyValue( "SeparatorLineColor", uno::Any( static_cast<sal_Int32>(COL_BLACK) ) );
            // 1 twip -> 2 mm100.
            xColumnPropSet->setPropertyValue( "SeparatorLineWidth", uno::Any( static_cast<sal_Int32>(2) ) );
        }
        xColumnContainer->setPropertyValue( sTextColumns, uno::Any( xColumns ) );
        // Set the columns to be unbalanced if that compatibility option is set or this is the last section.
        m_xColumnContainer = xColumnContainer;
        if ( rDM_Impl.GetSettingsTable()->GetNoColumnBalance() || rDM_Impl.GetIsLastSectionGroup() )
            DontBalanceTextColumns();
    }
    catch ( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION( "writerfilter", "SectionPropertyMap::ApplyColumnProperties" );
    }
    return xColumns;
}

bool SectionPropertyMap::HasHeader() const
{
    bool bRet = false;
    if (m_aPageStyle.is())
        m_aPageStyle->getPropertyValue(getPropertyName(PROP_HEADER_IS_ON)) >>= bRet;
    return bRet;
}

bool SectionPropertyMap::HasFooter() const
{
    bool bRet = false;
    if (m_aPageStyle.is())
        m_aPageStyle->getPropertyValue(getPropertyName(PROP_FOOTER_IS_ON)) >>= bRet;
    return bRet;
}

#define MIN_HEAD_FOOT_HEIGHT 100 // minimum header/footer height

namespace
{

// Copy the content of the header/footer property to the target style
void copyHeaderFooterTextProperty(const uno::Reference<beans::XPropertySet>& xSource,
                                  const uno::Reference<beans::XPropertySet>& xTarget,
                                  PropertyIds ePropId)
{
    if (!xSource.is() || !xTarget.is())
        return;

    try {
        const OUString& sName = getPropertyName(ePropId);
        SAL_INFO( "writerfilter", "Copying " << sName );
        uno::Reference<text::XText> xTextTarget(xTarget->getPropertyValue(sName), uno::UNO_QUERY_THROW);
        // remove any content already present or else it can become a mess
        SectionPropertyMap::removeXTextContent(xTextTarget);
        uno::Reference<text::XTextCopy> xTextCopyTarget(xTextTarget, uno::UNO_QUERY_THROW);
        if (!xTextCopyTarget.is())
            return;
        uno::Reference<text::XTextCopy> xTextCopySource(xSource->getPropertyValue(sName), uno::UNO_QUERY_THROW);
        if (!xTextCopySource.is())
            return;
        xTextCopyTarget->copyText(xTextCopySource);
    }
    catch (const uno::Exception&)
    {
        TOOLS_INFO_EXCEPTION( "writerfilter", "An exception occurred in SectionPropertyMap::CopyHeaderFooterTextProperty( )" );
    }
}

// Copies all the header and footer content and relevant flags from the source style to the target.
void completeCopyHeaderFooter(const uno::Reference<beans::XPropertySet>& xSourceStyle,
        const uno::Reference<beans::XPropertySet>& xTargetStyle,
        bool const bMissingHeader, bool const bMissingFooter)
{
    if (!xSourceStyle.is() || !xTargetStyle.is())
        return;

    bool bSourceHasHeader = false;
    bool bSourceHasFooter = false;
    bool bSourceHeaderIsShared = true;
    bool bSourceFooterIsShared = true;
    bool bSourceFirstIsShared = true;

    const OUString& sHeaderIsOn = getPropertyName(PROP_HEADER_IS_ON);
    const OUString& sFooterIsOn = getPropertyName(PROP_FOOTER_IS_ON);
    const OUString& sHeaderIsShared = getPropertyName(PROP_HEADER_IS_SHARED);
    const OUString& sFooterIsShared = getPropertyName(PROP_FOOTER_IS_SHARED);
    const OUString& sFirstIsShared = getPropertyName(PROP_FIRST_IS_SHARED);

    xSourceStyle->getPropertyValue(sHeaderIsOn) >>= bSourceHasHeader;
    xSourceStyle->getPropertyValue(sFooterIsOn) >>= bSourceHasFooter;
    xSourceStyle->getPropertyValue(sHeaderIsShared) >>= bSourceHeaderIsShared;
    xSourceStyle->getPropertyValue(sFooterIsShared) >>= bSourceFooterIsShared;
    xSourceStyle->getPropertyValue(sFirstIsShared) >>= bSourceFirstIsShared;

    xTargetStyle->setPropertyValue(sHeaderIsOn, uno::Any(bSourceHasHeader));
    xTargetStyle->setPropertyValue(sFooterIsOn, uno::Any(bSourceHasFooter));
    xTargetStyle->setPropertyValue(sHeaderIsShared, uno::Any(bSourceHeaderIsShared));
    xTargetStyle->setPropertyValue(sFooterIsShared, uno::Any(bSourceFooterIsShared));
    xTargetStyle->setPropertyValue(sFirstIsShared, uno::Any(bSourceFirstIsShared));

    if (bSourceHasHeader)
    {
        if (!bSourceFirstIsShared)
            copyHeaderFooterTextProperty(xSourceStyle, xTargetStyle, PROP_HEADER_TEXT_FIRST);
        if (!bSourceHeaderIsShared)
            copyHeaderFooterTextProperty(xSourceStyle, xTargetStyle, PROP_HEADER_TEXT_LEFT);
        copyHeaderFooterTextProperty(xSourceStyle, xTargetStyle, PROP_HEADER_TEXT);
    }

    if (bSourceHasFooter)
    {
        if (!bSourceFirstIsShared)
            copyHeaderFooterTextProperty(xSourceStyle, xTargetStyle, PROP_FOOTER_TEXT_FIRST);
        if (!bSourceFooterIsShared)
            copyHeaderFooterTextProperty(xSourceStyle, xTargetStyle, PROP_FOOTER_TEXT_LEFT);
        copyHeaderFooterTextProperty(xSourceStyle, xTargetStyle, PROP_FOOTER_TEXT);
    }
    // tdf#153196 the copy is used for the first page, the source will be used
    // on subsequent pages, so clear source's first page header/footer
    if (!bSourceFirstIsShared)
    {
        xSourceStyle->setPropertyValue(sFirstIsShared, uno::Any(true));
    }
    // if there is *only* a first-footer, and no previous section from which
    // to inherit a footer, the import process has created an empty footer
    // that didn't exist in the file; remove it
    if (bSourceHasHeader && bMissingHeader)
    {
        xSourceStyle->setPropertyValue(sHeaderIsOn, uno::Any(false));
    }
    if (bSourceHasFooter && bMissingFooter)
    {
        // setting "FooterIsShared" to true here does nothing, because it causes
        // left footer to be stashed, which means it will be exported anyway
        xSourceStyle->setPropertyValue(sFooterIsOn, uno::Any(false));
    }
}

// Copy headers and footers from the previous page style.
void copyHeaderFooter(const DomainMapper_Impl& rDM_Impl,
                                          const uno::Reference< beans::XPropertySet >& xPreviousStyle,
                                          const uno::Reference< beans::XPropertySet >& xStyle,
                                          bool bCopyRightHeader, bool bCopyLeftHeader, bool bCopyFirstHeader,
                                          bool bCopyRightFooter, bool bCopyLeftFooter, bool bCopyFirstFooter,
                                          bool bEvenAndOdd, bool bTitlePage)
{
    if (!rDM_Impl.IsNewDoc())
    {   // see also DomainMapper_Impl::PushPageHeaderFooter()
        return; // tdf#139737 SwUndoInserts cannot deal with new header/footer
    }

    if (!xPreviousStyle.is())
        return;

    bool bCopyHeader = bCopyRightHeader || bCopyLeftHeader || bCopyFirstHeader;
    bool bCopyFooter = bCopyRightFooter || bCopyLeftFooter || bCopyFirstFooter;

    if (!bCopyHeader && !bCopyFooter)
        return;

    bool bPreviousHasHeader = false;
    bool bPreviousHasFooter = false;

    bool bHasHeader = false;
    bool bHasFooter = false;

    const OUString& sHeaderIsOn = getPropertyName(PROP_HEADER_IS_ON);
    const OUString& sFooterIsOn = getPropertyName(PROP_FOOTER_IS_ON);
    const OUString& sHeaderIsShared = getPropertyName(PROP_HEADER_IS_SHARED);
    const OUString& sFooterIsShared = getPropertyName(PROP_FOOTER_IS_SHARED);
    const OUString& sFirstIsShared = getPropertyName(PROP_FIRST_IS_SHARED);

    xPreviousStyle->getPropertyValue(sHeaderIsOn) >>= bPreviousHasHeader;
    xPreviousStyle->getPropertyValue(sFooterIsOn) >>= bPreviousHasFooter;

    xStyle->getPropertyValue(sHeaderIsOn) >>= bHasHeader;
    xStyle->getPropertyValue(sFooterIsOn) >>= bHasFooter;

    xStyle->setPropertyValue(sHeaderIsOn, uno::Any(bPreviousHasHeader || bHasHeader));
    xStyle->setPropertyValue(sFooterIsOn, uno::Any(bPreviousHasFooter || bHasFooter));
    xStyle->setPropertyValue(sHeaderIsShared, uno::Any(false));
    xStyle->setPropertyValue(sFooterIsShared, uno::Any(false));
    xStyle->setPropertyValue(sFirstIsShared, uno::Any(false));

    if (bPreviousHasHeader && bCopyHeader)
    {
        if (bCopyRightHeader)
            copyHeaderFooterTextProperty(xPreviousStyle, xStyle, PROP_HEADER_TEXT);
        if (bCopyLeftHeader && bEvenAndOdd)
            copyHeaderFooterTextProperty(xPreviousStyle, xStyle, PROP_HEADER_TEXT_LEFT);
        if (bCopyFirstHeader && bTitlePage)
            copyHeaderFooterTextProperty(xPreviousStyle, xStyle, PROP_HEADER_TEXT_FIRST);
    }

    if (bPreviousHasFooter && bCopyFooter)
    {
        if (bCopyRightFooter)
            copyHeaderFooterTextProperty(xPreviousStyle, xStyle, PROP_FOOTER_TEXT);
        if (bCopyLeftFooter && bEvenAndOdd)
            copyHeaderFooterTextProperty(xPreviousStyle, xStyle, PROP_FOOTER_TEXT_LEFT);
        if (bCopyFirstFooter && bTitlePage)
            copyHeaderFooterTextProperty(xPreviousStyle, xStyle, PROP_FOOTER_TEXT_FIRST);
    }

    xStyle->setPropertyValue(sHeaderIsOn, uno::Any(bPreviousHasHeader || bHasHeader));
    xStyle->setPropertyValue(sFooterIsOn, uno::Any(bPreviousHasFooter || bHasFooter));

    xStyle->setPropertyValue(sHeaderIsShared, uno::Any(!bEvenAndOdd));
    xStyle->setPropertyValue(sFooterIsShared, uno::Any(!bEvenAndOdd));
    xStyle->setPropertyValue(sFirstIsShared, uno::Any(!bTitlePage));
}

} // end anonymous namespace


// Copy header and footer content from the previous docx section as needed.
//
// Any headers and footers which were not defined in this docx section
// should be "linked" with the corresponding header or footer from the
// previous section.  LO does not support linking of header/footer content
// across page styles so we just copy the content from the previous section.
void SectionPropertyMap::CopyLastHeaderFooter(DomainMapper_Impl& rDM_Impl )
{
    SAL_INFO( "writerfilter", "START>>> SectionPropertyMap::CopyLastHeaderFooter()" );
    SectionPropertyMap* pLastContext = rDM_Impl.GetLastSectionContext();
    if (pLastContext)
    {
        uno::Reference<beans::XPropertySet> xPreviousStyle = pLastContext->GetPageStyle(rDM_Impl);
        uno::Reference<beans::XPropertySet> xStyle = GetPageStyle(rDM_Impl);

        bool bEvenAndOdd = rDM_Impl.GetSettingsTable()->GetEvenAndOddHeaders();

        copyHeaderFooter(rDM_Impl, xPreviousStyle, xStyle,
                         m_bDefaultHeaderLinkToPrevious,
                         m_bEvenPageHeaderLinkToPrevious,
                         m_bFirstPageHeaderLinkToPrevious,
                         m_bDefaultFooterLinkToPrevious,
                         m_bEvenPageFooterLinkToPrevious,
                         m_bFirstPageFooterLinkToPrevious,
                         bEvenAndOdd, m_bTitlePage);
    }
    SAL_INFO( "writerfilter", "END>>> SectionPropertyMap::CopyLastHeaderFooter()" );
}

void SectionPropertyMap::PrepareHeaderFooterProperties()
{
    sal_Int32 nTopMargin = m_nTopMargin;
    sal_Int32 nHeaderHeight = m_nHeaderTop;
    if (HasHeader())
    {
        nTopMargin = m_nHeaderTop;
        nHeaderHeight = m_nTopMargin - m_nHeaderTop;

        // minimum header height 1mm
        if ( nHeaderHeight < MIN_HEAD_FOOT_HEIGHT )
            nHeaderHeight = MIN_HEAD_FOOT_HEIGHT;
    }

    Insert(PROP_HEADER_IS_DYNAMIC_HEIGHT, uno::Any(m_bDynamicHeightTop));
    Insert(PROP_HEADER_DYNAMIC_SPACING, uno::Any(m_bDynamicHeightTop));
    Insert(PROP_HEADER_BODY_DISTANCE, uno::Any(nHeaderHeight - MIN_HEAD_FOOT_HEIGHT));
    Insert(PROP_HEADER_HEIGHT, uno::Any(nHeaderHeight));
    // looks like PROP_HEADER_HEIGHT = height of the header + space between the header, and the body

    sal_Int32 nBottomMargin = m_nBottomMargin;
    sal_Int32 nFooterHeight = m_nHeaderBottom;
    if (HasFooter())
    {
        nBottomMargin = m_nHeaderBottom;
        nFooterHeight = m_nBottomMargin - m_nHeaderBottom;

        // minimum footer height 1mm
        if ( nFooterHeight < MIN_HEAD_FOOT_HEIGHT )
            nFooterHeight = MIN_HEAD_FOOT_HEIGHT;
    }

    Insert(PROP_FOOTER_IS_DYNAMIC_HEIGHT, uno::Any(m_bDynamicHeightBottom));
    Insert(PROP_FOOTER_DYNAMIC_SPACING, uno::Any(m_bDynamicHeightBottom));
    Insert(PROP_FOOTER_BODY_DISTANCE, uno::Any(nFooterHeight - MIN_HEAD_FOOT_HEIGHT));
    Insert(PROP_FOOTER_HEIGHT, uno::Any(nFooterHeight));

    //now set the top/bottom margin for the follow page style
    Insert( PROP_TOP_MARGIN, uno::Any( std::max<sal_Int32>(nTopMargin, 0) ) );
    Insert( PROP_BOTTOM_MARGIN, uno::Any( std::max<sal_Int32>(nBottomMargin, 0) ) );
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

void SectionPropertyMap::HandleMarginsHeaderFooter(DomainMapper_Impl& rDM_Impl)
{
    Insert( PROP_LEFT_MARGIN, uno::Any( m_nLeftMargin ) );
    Insert( PROP_RIGHT_MARGIN, uno::Any( m_nRightMargin ) );
    Insert(PROP_GUTTER_MARGIN, uno::Any(m_nGutterMargin));

    if ( rDM_Impl.m_oBackgroundColor )
        Insert( PROP_BACK_COLOR, uno::Any( *rDM_Impl.m_oBackgroundColor ) );

    // Check for missing footnote separator only in case there is at least
    // one footnote.
    if (rDM_Impl.m_bHasFtn && !rDM_Impl.m_bHasFtnSep)
    {
        // Set footnote line width to zero, document has no footnote separator.
        Insert(PROP_FOOTNOTE_LINE_RELATIVE_WIDTH, uno::Any(sal_Int32(0)));
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
                        Insert( PROP_FOOTNOTE_LINE_ADJUST, uno::Any( sal_Int16(text::HorizontalAdjust_RIGHT) ), false );
                    else
                        Insert( PROP_FOOTNOTE_LINE_ADJUST, uno::Any( sal_Int16(text::HorizontalAdjust_LEFT) ), false );
                }
            }
        }
        catch ( const uno::Exception& ) {}
    }

    /*** if headers/footers are available then the top/bottom margins of the
    header/footer are copied to the top/bottom margin of the page
    */
    CopyLastHeaderFooter(rDM_Impl);
    PrepareHeaderFooterProperties();

    // tdf#119952: If top/bottom margin was negative during docx import,
    // then the header/footer and the body could be on top of each other
    // writer is unable to display both of them in the same position, but can be simulated
    // by moving the header/footer text into a flyframe anchored to the header/footer,
    // leaving an empty dummy header/footer.
    rDM_Impl.ConvertHeaderFooterToTextFrame(m_bDynamicHeightTop, m_bDynamicHeightBottom);
}

void SectionPropertyMap::InheritOrFinalizePageStyles(DomainMapper_Impl& rDM_Impl)
{
    // if no new styles have been created for this section, inherit from the previous section,
    // otherwise apply this section's settings to the new style.
    SectionPropertyMap* pLastContext = rDM_Impl.GetLastSectionContext();
    //tdf124637 TODO: identify and skip special sections (like footnotes/endnotes)
    if (pLastContext && m_sPageStyleName.isEmpty())
        m_sPageStyleName = pLastContext->GetPageStyleName();
    else
    {
        HandleMarginsHeaderFooter(rDM_Impl);
        GetPageStyle(rDM_Impl);
        if (rDM_Impl.IsNewDoc() && m_aPageStyle.is())
            ApplyProperties_(m_aPageStyle);
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
            // Ignore inline objects stored only for redlining.
            if (rAnchored.m_xRedlineForInline)
                continue;

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
            xParagraph->setPropertyValue("ParaTopMargin", uno::Any(nTopMargin));
        }
    }
    rAnchoredObjectAnchors.clear();
}

void BeforeConvertToTextFrame(std::deque<css::uno::Any>& rFramedRedlines, std::vector<sal_Int32>& redPos, std::vector<sal_Int32>& redLen, std::vector<OUString>& redCell, std::vector<OUString>& redTable)
{
    // convert redline ranges to cursor movement and character length
    for( size_t i = 0; i < rFramedRedlines.size(); i+=3)
    {
        uno::Reference<text::XText> xCell;
        uno::Reference< text::XTextRange > xRange;
        rFramedRedlines[i] >>= xRange;
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
}

void AfterConvertToTextFrame(DomainMapper_Impl& rDM_Impl, std::deque<css::uno::Any>& aFramedRedlines, std::vector<sal_Int32>& redPos, std::vector<sal_Int32>& redLen, std::vector<OUString>& redCell, std::vector<OUString>& redTable)
{
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
        try
        {
            xRedline->makeRedline( sType, aRedlineProperties );
        }
        catch (const uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("writerfilter", "makeRedline() failed");
        }
    }
}

void SectionPropertyMap::CreateEvenOddPageStyleCopy(DomainMapper_Impl& rDM_Impl, PageBreakType eBreakType)
{
    OUString evenOddStyleName = rDM_Impl.GetUnusedPageStyleName();
    uno::Reference<beans::XPropertySet> evenOddStyle(
        rDM_Impl.GetTextFactory()->createInstance("com.sun.star.style.PageStyle"),
        uno::UNO_QUERY);
    // Unfortunately using setParent() does not work for page styles, so make a deep copy of the page style.
    uno::Reference<beans::XPropertySet> pageProperties(m_aPageStyle);
    uno::Reference<beans::XPropertySetInfo> pagePropertiesInfo(pageProperties->getPropertySetInfo());
    const uno::Sequence<beans::Property> propertyList(pagePropertiesInfo->getProperties());

    // Ignore write-only properties.
    static const std::unordered_set<OUString> staticDenylist = {
        "FooterBackGraphicURL", "BackGraphicURL", "HeaderBackGraphicURL",
        "HeaderIsOn", "FooterIsOn",
        "HeaderIsShared", "FooterIsShared", "FirstIsShared",
        "HeaderText", "HeaderTextLeft", "HeaderTextFirst",
        "FooterText", "FooterTextLeft", "FooterTextFirst" };

    for (const auto& rProperty : propertyList)
    {
        if ((rProperty.Attributes & beans::PropertyAttribute::READONLY) == 0)
        {
            if (staticDenylist.find(rProperty.Name) == staticDenylist.end())
            {
                evenOddStyle->setPropertyValue(
                    rProperty.Name,
                    pageProperties->getPropertyValue(rProperty.Name));
            }
        }
    }
    evenOddStyle->setPropertyValue("FollowStyle", uno::Any(m_sPageStyleName));

    rDM_Impl.GetPageStyles()->insertByName(evenOddStyleName, uno::Any(evenOddStyle));

    if (rDM_Impl.IsNewDoc())
    {
        bool const bEvenAndOdd(rDM_Impl.GetSettingsTable()->GetEvenAndOddHeaders());
        completeCopyHeaderFooter(pageProperties, evenOddStyle,
            !rDM_Impl.SeenHeaderFooter(PagePartType::Header, PageType::RIGHT)
                && (!bEvenAndOdd || !rDM_Impl.SeenHeaderFooter(PagePartType::Header, PageType::LEFT)),
            !rDM_Impl.SeenHeaderFooter(PagePartType::Footer, PageType::RIGHT)
                && (!bEvenAndOdd || !rDM_Impl.SeenHeaderFooter(PagePartType::Footer, PageType::LEFT)));
    }

    if (eBreakType == PageBreakType::Even)
        evenOddStyle->setPropertyValue(getPropertyName(PROP_PAGE_STYLE_LAYOUT), uno::Any(style::PageStyleLayout_LEFT));
    else if (eBreakType == PageBreakType::Odd)
        evenOddStyle->setPropertyValue(getPropertyName(PROP_PAGE_STYLE_LAYOUT), uno::Any(style::PageStyleLayout_RIGHT));

    m_sPageStyleName = evenOddStyleName; // And use it instead of the original one (which is set as follow of this one).
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
                    uno::Any(m_nLnnMin + 1));
            }
            catch ( const uno::Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("writerfilter.dmapper", "Exception in SectionPropertyMap::CloseSectionGroup");
            }
        }
    }

    if (m_nBreakType == NS_ooxml::LN_Value_ST_SectionMark_continuous
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
            setHeaderFooterProperties(rDM_Impl);
            InheritOrFinalizePageStyles( rDM_Impl );
            ApplySectionProperties( xSection, rDM_Impl );  //depends on InheritOrFinalizePageStyles
            uno::Reference< beans::XPropertySet > xRangeProperties( lcl_GetRangeProperties( m_bIsFirstSection, rDM_Impl, m_xStartingRange ) );
            if ( m_bIsFirstSection && !m_sPageStyleName.isEmpty() && xRangeProperties.is() )
            {
                xRangeProperties->setPropertyValue(getPropertyName(PROP_PAGE_DESC_NAME), uno::Any(m_sPageStyleName));
            }
            else if ((!m_bFirstPageHeaderLinkToPrevious ||
                      !m_bFirstPageFooterLinkToPrevious ||
                      !m_bDefaultHeaderLinkToPrevious ||
                      !m_bDefaultFooterLinkToPrevious ||
                      !m_bEvenPageHeaderLinkToPrevious ||
                      !m_bEvenPageFooterLinkToPrevious)
                    && rDM_Impl.GetCurrentXText())
            {
                // find a node in the section that has a page break and change
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
                        style::BreakType eBreakType;
                        if ((xElem->getPropertyValue("BreakType") >>= eBreakType) && eBreakType == style::BreakType_PAGE_BEFORE)
                        {
                            // tdf#112201: do *not* use m_sFirstPageStyleName here!
                            xElem->setPropertyValue(getPropertyName(PROP_PAGE_DESC_NAME), uno::Any(m_sPageStyleName));
                            m_aPageStyle->setPropertyValue(getPropertyName(PROP_FIRST_IS_SHARED), uno::Any(true));
                            isFound = true;
                            break;
                        }
                    }
                }
                uno::Reference<text::XParagraphCursor> const xPCursor(xCursor, uno::UNO_QUERY_THROW);
                float fCharHeight = 0;
                if (!isFound)
                {   // HACK: try the last paragraph of the previous section
                    xPCursor->gotoPreviousParagraph(false);
                    uno::Reference<beans::XPropertySet> const xPSCursor(xCursor, uno::UNO_QUERY_THROW);
                    style::BreakType eBreakType;
                    if ((xPSCursor->getPropertyValue("BreakType") >>= eBreakType) && eBreakType == style::BreakType_PAGE_BEFORE)
                    {
                        xPSCursor->setPropertyValue(getPropertyName(PROP_PAGE_DESC_NAME), uno::Any(m_sPageStyleName));
                        m_aPageStyle->setPropertyValue(getPropertyName(PROP_FIRST_IS_SHARED), uno::Any(true));
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
                    if ((xPropertySet->getPropertyValue("PageDescName") >>= aPageDescName) && !aPageDescName.isEmpty())
                    {
                        uno::Reference<beans::XPropertySet> xPageStyle(rDM_Impl.GetPageStyles()->getByName(aPageDescName), uno::UNO_QUERY);
                        xPageStyle->setPropertyValue("FollowStyle", uno::Any(m_sPageStyleName));
                        m_aPageStyle->setPropertyValue(getPropertyName(PROP_FIRST_IS_SHARED), uno::Any(true));
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
    else if (m_nBreakType == NS_ooxml::LN_Value_ST_SectionMark_nextColumn
            && m_nColumnCount > 1 && !rDM_Impl.IsInComments())
    {
        try
        {
            setHeaderFooterProperties(rDM_Impl);
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
            xRangeProperties->setPropertyValue( getPropertyName( PROP_BREAK_TYPE ), uno::Any( style::BreakType_COLUMN_BEFORE ) );
        }
        catch ( const uno::Exception& ) {}
    }
    else if (!rDM_Impl.IsInComments())
    {
        uno::Reference< beans::XPropertySet > xSection;
        ApplyProtectionProperties( xSection, rDM_Impl );

        //get the properties and create appropriate page styles
        uno::Reference<beans::XPropertySet> xPageStyle;
        //This part certainly is not needed for footnotes, so don't create unused page styles.
        if ( !rDM_Impl.IsInFootOrEndnote() )
        {
            xPageStyle.set(GetPageStyle(rDM_Impl));
            setHeaderFooterProperties(rDM_Impl);
            HandleMarginsHeaderFooter(rDM_Impl);
        }

        if ( rDM_Impl.GetSettingsTable()->GetMirrorMarginSettings() )
        {
            Insert( PROP_PAGE_STYLE_LAYOUT, uno::Any( style::PageStyleLayout_MIRRORED ) );
        }
        uno::Reference< text::XTextColumns > xColumns;
        if ( m_nColumnCount > 1 )
        {
            // prefer setting column properties into a section, not a page style if at all possible.
            if ( !xSection.is() )
                xSection = rDM_Impl.appendTextSectionAfter( m_xStartingRange );
            if ( xSection.is() )
                ApplyColumnProperties(xSection, rDM_Impl);
            else if (xPageStyle.is())
                xColumns = ApplyColumnProperties(xPageStyle, rDM_Impl);
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

        sal_Int16 nWritingMode = text::WritingMode2::LR_TB;
        pProp = getProperty( PROP_WRITING_MODE );
        if ( pProp )
            pProp->second >>= nWritingMode;

        sal_Int32 nTextAreaHeight = nWritingMode == text::WritingMode2::LR_TB ?
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
        sal_Int16 nGridType = m_nGridType;
        if ( nGridLines >= 0 && nGridLines <= SAL_MAX_INT16 )
            Insert( PROP_GRID_LINES, uno::Any( sal_Int16(nGridLines) ) );
        else
            nGridType = text::TextGridMode::NONE;

        // PROP_GRID_MODE
        if ( nGridType == text::TextGridMode::LINES_AND_CHARS )
        {
            if (!m_nDxtCharSpace)
                nGridType = text::TextGridMode::LINES;
            else
                Insert( PROP_GRID_SNAP_TO_CHARS, uno::Any( m_bGridSnapToChars ) );
        }

        Insert( PROP_GRID_MODE, uno::Any( nGridType ) );

        sal_Int32 nCharWidth = 423; //240 twip/ 12 pt
        const StyleSheetEntryPtr pEntry = rDM_Impl.GetStyleSheetTable()->FindStyleSheetByConvertedStyleName( u"Standard" );
        if ( pEntry )
        {
            std::optional< PropertyMap::Property > pPropHeight = pEntry->m_pProperties->getProperty( PROP_CHAR_HEIGHT_ASIAN );
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
            Insert( PROP_NUMBERING_TYPE, uno::Any( m_nPageNumberType ) );

        // #i119558#, force to set document as standard page mode,
        // refer to ww8 import process function "SwWW8ImplReader::SetDocumentGrid"
        try
        {
            uno::Reference< beans::XPropertySet > xDocProperties;
            xDocProperties.set( rDM_Impl.GetTextDocument(), uno::UNO_QUERY_THROW );
            Insert(PROP_GRID_STANDARD_MODE, uno::Any(true));
            xDocProperties->setPropertyValue("DefaultPageMode", uno::Any(false));
        }
        catch ( const uno::Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("writerfilter.dmapper", "Exception in SectionPropertyMap::CloseSectionGroup");
        }

        Insert( PROP_GRID_BASE_HEIGHT, uno::Any( nGridLinePitch ) );
        Insert( PROP_GRID_BASE_WIDTH, uno::Any( nCharWidth ) );
        Insert( PROP_GRID_RUBY_HEIGHT, uno::Any( sal_Int32( 0 ) ) );

        if (rDM_Impl.IsNewDoc() && xPageStyle.is())
            ApplyProperties_(xPageStyle);

        ApplyBorderToPageStyles( rDM_Impl, m_eBorderApply, m_eBorderOffsetFrom );
        ApplyPaperSource(rDM_Impl);

        try
        {
            //now apply this break at the first paragraph of this section
            uno::Reference< beans::XPropertySet > xRangeProperties( lcl_GetRangeProperties( m_bIsFirstSection, rDM_Impl, m_xStartingRange ) );

            // Handle page breaks with odd/even page numbering. We need to use an extra page style for setting the page style
            // to left/right, because if we set it to the normal style, we'd set it to "First Page"/"Default Style", which would
            // break them (all default pages would be only left or right).
            if (m_nBreakType == NS_ooxml::LN_Value_ST_SectionMark_evenPage)
            {
                CreateEvenOddPageStyleCopy(rDM_Impl, PageBreakType::Even);
            }
            else if (m_nBreakType == NS_ooxml::LN_Value_ST_SectionMark_oddPage)
            {
                CreateEvenOddPageStyleCopy(rDM_Impl, PageBreakType::Odd);
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
                const OUString sPageStyle = m_sPageStyleName;
                if (!rDM_Impl.IsReadGlossaries()
                    && !rDM_Impl.IsInFootOrEndnote()
                    && !(m_bIsFirstSection && sPageStyle == getPropertyName( PROP_STANDARD ) && m_nPageNumber < 0)
                   )
                {
                    xRangeProperties->setPropertyValue(
                        getPropertyName( PROP_PAGE_DESC_NAME ),
                        uno::Any(sPageStyle) );
                }

                if (0 <= m_nPageNumber)
                {
                    sal_Int16 nPageNumber = static_cast< sal_Int16 >(m_nPageNumber);
                    xRangeProperties->setPropertyValue(getPropertyName(PROP_PAGE_NUMBER_OFFSET),
                        uno::Any(nPageNumber));
                }
            }
        }
        catch ( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION( "writerfilter", "SectionPropertyMap::CloseSectionGroup" );
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
        const OUString & sPropRelativeWidth = getPropertyName(PROP_RELATIVE_WIDTH);
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
void SectionPropertyMap::clearHeaderFooterLinkToPrevious(PagePartType ePartType, PageType eType)
{
    if (ePartType == PagePartType::Header)
    {
        switch (eType)
        {
            case PageType::FIRST: m_bFirstPageHeaderLinkToPrevious = false; break;
            case PageType::LEFT:  m_bEvenPageHeaderLinkToPrevious = false; break;
            case PageType::RIGHT: m_bDefaultHeaderLinkToPrevious = false; break;
        }
    }
    else if (ePartType == PagePartType::Footer)
    {
        switch (eType)
        {
            case PageType::FIRST: m_bFirstPageFooterLinkToPrevious = false; break;
            case PageType::LEFT:  m_bEvenPageFooterLinkToPrevious = false; break;
            case PageType::RIGHT: m_bDefaultFooterLinkToPrevious = false; break;
        }
    }
}

namespace {

class NamedPropertyValue
{
private:
    OUString m_aName;

public:
    explicit NamedPropertyValue( OUString i_aStr )
        : m_aName(std::move( i_aStr ))
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
        const uno::Sequence< beans::PropertyValue > vPropVals = GetPropertyValues();

        //Temporarily store the items that are in grab bags
        uno::Sequence< beans::PropertyValue > vCharVals;
        uno::Sequence< beans::PropertyValue > vParaVals;
        const beans::PropertyValue* pCharGrabBag = std::find_if( vPropVals.begin(), vPropVals.end(), NamedPropertyValue( "CharInteropGrabBag" ) );
        if ( pCharGrabBag != vPropVals.end() )
            (pCharGrabBag->Value) >>= vCharVals;
        const beans::PropertyValue* pParaGrabBag = std::find_if( vPropVals.begin(), vPropVals.end(), NamedPropertyValue( "ParaInteropGrabBag" ) );
        if ( pParaGrabBag != vPropVals.end() )
            (pParaGrabBag->Value) >>= vParaVals;

        for ( const beans::PropertyValue* pIter = vPropVals.begin(); pIter != vPropVals.end(); ++pIter )
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
            TOOLS_WARN_EXCEPTION( "writerfilter", "SectionPropertyMap::ApplyProperties_" );
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
            TOOLS_WARN_EXCEPTION( "writerfilter", "SectionPropertyMap::ApplyProperties_" );
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
