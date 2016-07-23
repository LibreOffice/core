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
#include <PropertyMap.hxx>
#include <ooxml/resourceids.hxx>
#include <DomainMapper_Impl.hxx>
#include <ConversionHelper.hxx>
#include <i18nutil/paper.hxx>
#include <osl/diagnose.h>
#include <rtl/ustring.hxx>
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
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/TextGridMode.hpp>
#include <com/sun/star/text/XTextCopy.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include "PropertyMapHelper.hxx"

using namespace ::com::sun::star;

namespace writerfilter {
namespace dmapper{


PropertyMap::PropertyMap()
{
}


PropertyMap::~PropertyMap()
{
}


uno::Sequence< beans::PropertyValue > PropertyMap::GetPropertyValues(bool bCharGrabBag)
{
    using comphelper::makePropertyValue;
    if(m_aValues.empty() && !m_vMap.empty())
    {
        size_t nCharGrabBag = 0;
        size_t nParaGrabBag = 0;
        size_t nCellGrabBag = 0;
        size_t nRowGrabBag = 0;
        for (MapIterator i = m_vMap.begin(); i != m_vMap.end(); ++i)
        {
            if ( i->second.getGrabBagType() == CHAR_GRAB_BAG )
                nCharGrabBag++;
            else if ( i->second.getGrabBagType() == PARA_GRAB_BAG )
                nParaGrabBag++;
            else if ( i->second.getGrabBagType() == CELL_GRAB_BAG )
                nCellGrabBag++;
            else if ( i->first == PROP_CELL_INTEROP_GRAB_BAG)
            {
                uno::Sequence<beans::PropertyValue> aSeq;
                i->second.getValue() >>= aSeq;
                nCellGrabBag += aSeq.getLength();
            }
            else if ( i->second.getGrabBagType() == ROW_GRAB_BAG )
                nRowGrabBag++;
        }

        //style names have to be the first elements within the property sequence
        //otherwise they will overwrite 'hard' attributes
        MapIterator aParaStyleIter = m_vMap.find(PROP_PARA_STYLE_NAME);
        if( aParaStyleIter != m_vMap.end())
        {
            m_aValues.push_back( makePropertyValue(
                        getPropertyName( aParaStyleIter->first ),
                        aParaStyleIter->second.getValue()
                        ) );
        }

        MapIterator aCharStyleIter = m_vMap.find(PROP_CHAR_STYLE_NAME);
        if( aCharStyleIter != m_vMap.end())
        {
            m_aValues.push_back( makePropertyValue(
                        getPropertyName( aCharStyleIter->first ),
                        aCharStyleIter->second.getValue()
                        ) );
        }
        MapIterator aNumRuleIter = m_vMap.find(PROP_NUMBERING_RULES);
        if( aNumRuleIter != m_vMap.end())
        {
            m_aValues.push_back( makePropertyValue(
                        getPropertyName( aNumRuleIter->first ),
                        aNumRuleIter->second.getValue()
                        ) );
        }

        // If there are any grab bag properties, we need one slot for them.
        uno::Sequence<beans::PropertyValue> aCharGrabBagValues(nCharGrabBag);
        uno::Sequence<beans::PropertyValue> aParaGrabBagValues(nParaGrabBag);
        uno::Sequence<beans::PropertyValue> aCellGrabBagValues(nCellGrabBag);
        uno::Sequence<beans::PropertyValue> aRowGrabBagValues(nRowGrabBag);
        beans::PropertyValue* pCharGrabBagValues = aCharGrabBagValues.getArray();
        beans::PropertyValue* pParaGrabBagValues = aParaGrabBagValues.getArray();
        beans::PropertyValue* pCellGrabBagValues = aCellGrabBagValues.getArray();
        beans::PropertyValue* pRowGrabBagValues = aRowGrabBagValues.getArray();
        // Record index for the next property to be added in each grab bag.
        sal_Int32 nRowGrabBagValue = 0;
        sal_Int32 nCellGrabBagValue = 0;
        sal_Int32 nParaGrabBagValue = 0;
        sal_Int32 nCharGrabBagValue = 0;

        MapIterator aMapIter = m_vMap.begin();
        for( ; aMapIter != m_vMap.end(); ++aMapIter )
        {
            if( aMapIter != aParaStyleIter && aMapIter != aCharStyleIter && aMapIter != aNumRuleIter )
            {
                if ( aMapIter->second.getGrabBagType() == CHAR_GRAB_BAG )
                {
                    if (bCharGrabBag)
                    {
                        pCharGrabBagValues[nCharGrabBagValue].Name = getPropertyName( aMapIter->first );
                        pCharGrabBagValues[nCharGrabBagValue].Value = aMapIter->second.getValue();
                        ++nCharGrabBagValue;
                    }
                }
                else if ( aMapIter->second.getGrabBagType() == PARA_GRAB_BAG )
                {
                    pParaGrabBagValues[nParaGrabBagValue].Name = getPropertyName( aMapIter->first );
                    pParaGrabBagValues[nParaGrabBagValue].Value = aMapIter->second.getValue();
                    ++nParaGrabBagValue;
                }
                else if ( aMapIter->second.getGrabBagType() == CELL_GRAB_BAG )
                {
                    pCellGrabBagValues[nCellGrabBagValue].Name = getPropertyName( aMapIter->first );
                    pCellGrabBagValues[nCellGrabBagValue].Value = aMapIter->second.getValue();
                    ++nCellGrabBagValue;
                }
                else if ( aMapIter->second.getGrabBagType() == ROW_GRAB_BAG )
                {
                    pRowGrabBagValues[nRowGrabBagValue].Name = getPropertyName( aMapIter->first );
                    pRowGrabBagValues[nRowGrabBagValue].Value = aMapIter->second.getValue();
                    ++nRowGrabBagValue;
                }
                else if (aMapIter->first == PROP_CELL_INTEROP_GRAB_BAG)
                {
                    uno::Sequence<beans::PropertyValue> aSeq;
                    aMapIter->second.getValue() >>= aSeq;
                    for (sal_Int32 i = 0; i < aSeq.getLength(); ++i)
                    {
                        pCellGrabBagValues[nCellGrabBagValue] = aSeq[i];
                        ++nCellGrabBagValue;
                    }
                }
                else
                {
                    m_aValues.push_back( makePropertyValue(
                                getPropertyName( aMapIter->first ),
                                aMapIter->second.getValue()
                                ) );
                }
            }
        }
        if (nCharGrabBag && bCharGrabBag)
        {
            m_aValues.push_back( makePropertyValue( "CharInteropGrabBag",
                        uno::makeAny(aCharGrabBagValues))
                    );
        }
        if (nParaGrabBag)
        {
            m_aValues.push_back( makePropertyValue( "ParaInteropGrabBag",
                        uno::makeAny(aParaGrabBagValues))
                    );
        }
        if (nCellGrabBag)
        {
            m_aValues.push_back( makePropertyValue( "CellInteropGrabBag",
                        uno::makeAny(aCellGrabBagValues))
                    );
        }
        if (nRowGrabBag)
        {
            m_aValues.push_back( makePropertyValue( "RowInteropGrabBag",
                        uno::makeAny(aRowGrabBagValues))
                    );
        }
    }
    return comphelper::containerToSequence(m_aValues);
}

#ifdef DEBUG_WRITERFILTER
static void lcl_AnyToTag(const uno::Any & rAny)
{
    try {
        sal_Int32 aInt = 0;
        if (rAny >>= aInt) {
            TagLogger::getInstance().attribute("value", rAny);
        } else {
            TagLogger::getInstance().attribute("unsignedValue", 0);
        }

        sal_uInt32 auInt = 0;
        rAny >>= auInt;
        TagLogger::getInstance().attribute("unsignedValue", auInt);

        float aFloat = 0.0f;
        if (rAny >>= aFloat) {
            TagLogger::getInstance().attribute("floatValue", rAny);
        } else {
            TagLogger::getInstance().attribute("unsignedValue", 0);
        }

        OUString aStr;
        rAny >>= aStr;
        TagLogger::getInstance().attribute("stringValue", aStr);
    }
    catch (...) {
    }
}
#endif

void PropertyMap::Insert( PropertyIds eId, const uno::Any& rAny, bool bOverwrite, GrabBagType i_GrabBagType )
{
#ifdef DEBUG_WRITERFILTER
    const OUString& rInsert = getPropertyName(eId);

    TagLogger::getInstance().startElement("propertyMap.insert");
    TagLogger::getInstance().attribute("name", rInsert);
    lcl_AnyToTag(rAny);
    TagLogger::getInstance().endElement();
#endif

    if (!bOverwrite)
        m_vMap.insert(std::make_pair(eId, PropValue(rAny, i_GrabBagType)));
    else
        m_vMap[eId] = PropValue(rAny, i_GrabBagType);

    Invalidate();
}

void PropertyMap::Erase( PropertyIds eId )
{
    //Safe call to erase, it throws no exceptions, even if eId is not in m_vMap
    m_vMap.erase(eId);

    Invalidate();
}

boost::optional<PropertyMap::Property> PropertyMap::getProperty( PropertyIds eId ) const
{
    MapIterator aIter = m_vMap.find(eId);
    if (aIter==m_vMap.end())
        return boost::optional<Property>();
    else
        return std::make_pair( eId, aIter->second.getValue() ) ;
}

bool PropertyMap::isSet( PropertyIds eId) const
{
    return m_vMap.find(eId)!=m_vMap.end();
}

#ifdef DEBUG_WRITERFILTER
void PropertyMap::dumpXml() const
{
    TagLogger::getInstance().startElement("PropertyMap");

    MapIterator aMapIter = m_vMap.begin();
    while (aMapIter != m_vMap.end())
    {
        TagLogger::getInstance().startElement("property");

        TagLogger::getInstance().attribute("name", getPropertyName( aMapIter->first ));

        switch (aMapIter->first)
        {
            case PROP_TABLE_COLUMN_SEPARATORS:
               lcl_DumpTableColumnSeparators(aMapIter->second.getValue());
                break;
            default:
            {
                try {
                    sal_Int32 aInt = 0;
                    aMapIter->second.getValue() >>= aInt;
                    TagLogger::getInstance().attribute("value", aInt);

                    sal_uInt32 auInt = 0;
                    aMapIter->second.getValue() >>= auInt;
                    TagLogger::getInstance().attribute("unsignedValue", auInt);

                    float aFloat = 0.0;
                    aMapIter->second.getValue() >>= aFloat;
                    TagLogger::getInstance().attribute("floatValue", aFloat);

                    OUString aStr;
                    aMapIter->second.getValue() >>= auInt;
                    TagLogger::getInstance().attribute("stringValue", aStr);
                }
                catch (...) {
                }
            }
                break;
        }

        TagLogger::getInstance().endElement();

        ++aMapIter;
    }

    TagLogger::getInstance().endElement();
}
#endif

void PropertyMap::InsertProps(const PropertyMapPtr& rMap)
{
    if (rMap)
    {
        MapIterator pEnd = rMap->m_vMap.end();
        for ( MapIterator iter = rMap->m_vMap.begin(); iter!=pEnd; ++iter )
            m_vMap[iter->first] = iter->second;

        insertTableProperties(rMap.get());

        Invalidate();
    }
}

void PropertyMap::insertTableProperties( const PropertyMap* )
{
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().element("PropertyMap.insertTableProperties");
#endif
}

void PropertyMap::printProperties()
{
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().startElement("properties");

    MapIterator aMapIter = m_vMap.begin();
    MapIterator aEndIter = m_vMap.end();
    for( ; aMapIter != aEndIter; ++aMapIter )
    {
        SAL_INFO("writerfilter", getPropertyName(aMapIter->first));

        table::BorderLine2 aLine;
        sal_Int32 nColor;
        if ( aMapIter->second.getValue() >>= aLine )
        {
            TagLogger::getInstance().startElement("borderline");
            TagLogger::getInstance().attribute("color", aLine.Color);
            TagLogger::getInstance().attribute("inner", aLine.InnerLineWidth);
            TagLogger::getInstance().attribute("outer", aLine.OuterLineWidth);
            TagLogger::getInstance().endElement();
        }
        else if ( aMapIter->second.getValue() >>= nColor )
        {
            TagLogger::getInstance().startElement("color");
            TagLogger::getInstance().attribute("number", nColor);
            TagLogger::getInstance().endElement();
        }
    }

    TagLogger::getInstance().endElement();
#else
    (void) this; // avoid loplugin:staticmethods
#endif
}

SectionPropertyMap::SectionPropertyMap(bool bIsFirstSection) :
    m_bIsFirstSection( bIsFirstSection )
    ,m_nBorderParams( 0 )
    ,m_bTitlePage( false )
    ,m_nColumnCount( 0 )
    ,m_nColumnDistance( 1249 )
    ,m_bSeparatorLineIsOn( false )
    ,m_bEvenlySpaced( false )
    ,m_bIsLandscape( false )
    ,m_bPageNoRestart( false )
    ,m_nPageNumber( -1 )
    ,m_nPageNumberType(-1)
    ,m_nBreakType( -1 )
    ,m_nPaperBin( -1 )
    ,m_nFirstPaperBin( -1 )
    ,m_nLeftMargin( 3175 ) //page left margin, default 0x708 (1800) twip -> 3175 1/100 mm
    ,m_nRightMargin( 3175 )//page right margin, default 0x708 (1800) twip -> 3175 1/100 mm
    ,m_nTopMargin( 2540 )
    ,m_nBottomMargin( 2540 )
    ,m_nHeaderTop( 1270 ) //720 twip
    ,m_nHeaderBottom( 1270 )//720 twip
    ,m_nDzaGutter( 0 )
    ,m_nGridType(0)
    ,m_nGridLinePitch( 1 )
    ,m_nDxtCharSpace( 0 )
    ,m_bGridSnapToChars(true)
    ,m_nLnnMod( 0 )
    ,m_nLnc( 0 )
    ,m_ndxaLnn( 0 )
    ,m_nLnnMin( 0 )
    ,m_bDefaultHeaderLinkToPrevious(true)
    ,m_bEvenPageHeaderLinkToPrevious(true)
    ,m_bFirstPageHeaderLinkToPrevious(true)
    ,m_bDefaultFooterLinkToPrevious(true)
    ,m_bEvenPageFooterLinkToPrevious(true)
    ,m_bFirstPageFooterLinkToPrevious(true)
{
    static sal_Int32 nNumber = 0;
    nSectionNumber = nNumber++;
    for( sal_Int32 nBorder = 0; nBorder < 4; ++nBorder )
    {
        m_nBorderDistances[ nBorder ] = -1;
        m_bBorderShadows[nBorder] = false;
    }
    //todo: set defaults in ApplyPropertiesToPageStyles
    //initialize defaults
    PaperInfo aLetter(PAPER_LETTER);
    //page height, 1/100mm
    Insert( PROP_HEIGHT, uno::makeAny( (sal_Int32) aLetter.getHeight() ) );
    //page width, 1/100mm
    Insert( PROP_WIDTH, uno::makeAny( (sal_Int32) aLetter.getWidth() ) );
    //page left margin, default 0x708 (1800) twip -> 3175 1/100 mm
    Insert( PROP_LEFT_MARGIN, uno::makeAny( (sal_Int32) 3175 ) );
    //page right margin, default 0x708 (1800) twip -> 3175 1/100 mm
    Insert( PROP_RIGHT_MARGIN, uno::makeAny( (sal_Int32) 3175 ) );
    //page top margin, default 0x5a0 (1440) twip -> 2540 1/100 mm
    Insert( PROP_TOP_MARGIN, uno::makeAny( (sal_Int32)2540 ) );
    //page bottom margin, default 0x5a0 (1440) twip -> 2540 1/100 mm
    Insert( PROP_BOTTOM_MARGIN, uno::makeAny( (sal_Int32) 2540 ) );
    //page style layout
    Insert(PROP_PAGE_STYLE_LAYOUT, uno::makeAny(style::PageStyleLayout_ALL));
    uno::Any aFalse( ::uno::makeAny( false ) );
    Insert( PROP_GRID_DISPLAY, aFalse);
    Insert( PROP_GRID_PRINT, aFalse);
    Insert( PROP_GRID_MODE, uno::makeAny(text::TextGridMode::NONE));


    if( m_bIsFirstSection )
    {
        m_sFirstPageStyleName = getPropertyName( PROP_FIRST_PAGE );
        m_sFollowPageStyleName = getPropertyName( PROP_STANDARD );
    }
}


SectionPropertyMap::~SectionPropertyMap()
{
}


OUString lcl_FindUnusedPageStyleName(const uno::Sequence< OUString >& rPageStyleNames)
{
    static const char DEFAULT_STYLE[] = "Converted";
    //find the highest number x in each style with the name "DEFAULT_STYLE+x" and
    //return an incremented name
    sal_Int32 nMaxIndex = 0;

    const OUString* pStyleNames = rPageStyleNames.getConstArray();
    for( sal_Int32 nStyle = 0; nStyle < rPageStyleNames.getLength(); ++nStyle)
    {
        if( pStyleNames[nStyle].startsWith(DEFAULT_STYLE) )
        {
            sal_Int32 nIndex = pStyleNames[nStyle].copy( strlen(DEFAULT_STYLE) ).toInt32();
            if( nIndex > nMaxIndex)
                nMaxIndex = nIndex;
        }
    }
    return DEFAULT_STYLE + OUString::number( nMaxIndex + 1);
}


uno::Reference< beans::XPropertySet > SectionPropertyMap::GetPageStyle(
        const uno::Reference< container::XNameContainer >& xPageStyles,
        const uno::Reference < lang::XMultiServiceFactory >& xTextFactory,
        bool bFirst )
{
    uno::Reference< beans::XPropertySet > xRet;
    try
    {
        if( bFirst )
        {
            if( m_sFirstPageStyleName.isEmpty() && xPageStyles.is() )
            {
                uno::Sequence< OUString > aPageStyleNames = xPageStyles->getElementNames();
                m_sFirstPageStyleName = lcl_FindUnusedPageStyleName(aPageStyleNames);
                m_aFirstPageStyle.set( xTextFactory->createInstance("com.sun.star.style.PageStyle"),
                                       uno::UNO_QUERY);

                // Call insertByName() before GetPageStyle(), otherwise the
                // first and the follow page style will have the same name, and
                // insertByName() will fail.
                if (xPageStyles.is())
                    xPageStyles->insertByName( m_sFirstPageStyleName, uno::makeAny(m_aFirstPageStyle) );

                // Ensure that m_aFollowPageStyle has been created
                GetPageStyle( xPageStyles, xTextFactory, false );
                // Chain m_aFollowPageStyle to be after m_aFirstPageStyle
                m_aFirstPageStyle->setPropertyValue("FollowStyle",
                    uno::makeAny(m_sFollowPageStyleName));
            }
            else if( !m_aFirstPageStyle.is() && xPageStyles.is() )
            {
                xPageStyles->getByName(m_sFirstPageStyleName) >>= m_aFirstPageStyle;
            }
            xRet = m_aFirstPageStyle;
        }
        else
        {
            if( m_sFollowPageStyleName.isEmpty() && xPageStyles.is() )
            {
                uno::Sequence< OUString > aPageStyleNames = xPageStyles->getElementNames();
                m_sFollowPageStyleName = lcl_FindUnusedPageStyleName(aPageStyleNames);
                m_aFollowPageStyle.set( xTextFactory->createInstance("com.sun.star.style.PageStyle"),
                                        uno::UNO_QUERY);
                xPageStyles->insertByName( m_sFollowPageStyleName, uno::makeAny(m_aFollowPageStyle) );
            }
            else if(!m_aFollowPageStyle.is() && xPageStyles.is() )
            {
                xPageStyles->getByName(m_sFollowPageStyleName) >>= m_aFollowPageStyle;
            }
            xRet = m_aFollowPageStyle;
        }

    }
    catch( const uno::Exception& rException )
    {
        SAL_WARN("writerfilter", "SectionPropertyMap::GetPageStyle() failed: " << rException.Message);
    }

    return xRet;
}


void SectionPropertyMap::SetBorder( BorderPosition ePos, sal_Int32 nLineDistance, const table::BorderLine2& rBorderLine, bool bShadow )
{
    m_oBorderLines[ePos] = rBorderLine;
    m_nBorderDistances[ePos] = nLineDistance;
    m_bBorderShadows[ePos] = bShadow;
}


void SectionPropertyMap::ApplyBorderToPageStyles(
            const uno::Reference< container::XNameContainer >& xPageStyles,
            const uno::Reference < lang::XMultiServiceFactory >& xTextFactory,
        sal_Int32 nValue )
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
    uno::Reference< beans::XPropertySet >  xFirst;
    uno::Reference< beans::XPropertySet >  xSecond;
    sal_Int32 nOffsetFrom = (nValue & 0x00E0) >> 5;
    //todo: negative spacing (from ww8par6.cxx)
    switch( nValue & 0x07)
    {
        case 0: /*all styles*/
            if ( !m_sFollowPageStyleName.isEmpty() )
                xFirst = GetPageStyle( xPageStyles, xTextFactory, false );
            if ( !m_sFirstPageStyleName.isEmpty() )
                xSecond = GetPageStyle( xPageStyles, xTextFactory, true );
        break;
        case 1: /*first page*/
            if ( !m_sFirstPageStyleName.isEmpty() )
                xFirst = GetPageStyle( xPageStyles, xTextFactory, true );
        break;
        case 2: /*left and right*/
            if ( !m_sFollowPageStyleName.isEmpty() )
                xFirst  = GetPageStyle( xPageStyles, xTextFactory, false );
        break;
        case 3: //whole document?
            //todo: how to apply a border to the whole document - find all sections or access all page styles?
        default:
            return;
    }
    //has to be sorted like enum BorderPosition: l-r-t-b
    static const PropertyIds aBorderIds[4] =
    {
        PROP_LEFT_BORDER,
        PROP_RIGHT_BORDER,
        PROP_TOP_BORDER,
        PROP_BOTTOM_BORDER
    };
    static const PropertyIds aBorderDistanceIds[4] =
    {
        PROP_LEFT_BORDER_DISTANCE,
        PROP_RIGHT_BORDER_DISTANCE,
        PROP_TOP_BORDER_DISTANCE,
        PROP_BOTTOM_BORDER_DISTANCE
    };
    static const PropertyIds aMarginIds[4] =
    {
        PROP_LEFT_MARGIN,
        PROP_RIGHT_MARGIN,
        PROP_TOP_MARGIN,
        PROP_BOTTOM_MARGIN
    };

    for( sal_Int32 nBorder = 0; nBorder < 4; ++nBorder)
    {
        if( m_oBorderLines[nBorder] )
        {
            const OUString sBorderName = getPropertyName( aBorderIds[nBorder] );
            if (xFirst.is())
                xFirst->setPropertyValue( sBorderName, uno::makeAny( *m_oBorderLines[nBorder] ));
            if(xSecond.is())
                xSecond->setPropertyValue( sBorderName, uno::makeAny( *m_oBorderLines[nBorder] ));
        }
        if( m_nBorderDistances[nBorder] >= 0 )
        {
            sal_uInt32 nLineWidth = 0;
            if (m_oBorderLines[nBorder])
                nLineWidth = m_oBorderLines[nBorder]->LineWidth;
            if(xFirst.is())
                SetBorderDistance( xFirst, aMarginIds[nBorder], aBorderDistanceIds[nBorder],
                  m_nBorderDistances[nBorder], nOffsetFrom, nLineWidth );
            if(xSecond.is())
                SetBorderDistance( xSecond, aMarginIds[nBorder], aBorderDistanceIds[nBorder],
                      m_nBorderDistances[nBorder], nOffsetFrom, nLineWidth );
        }
    }

    if (m_bBorderShadows[BORDER_RIGHT])
    {
        table::ShadowFormat aFormat = getShadowFromBorder(*m_oBorderLines[BORDER_RIGHT]);
        if (xFirst.is())
            xFirst->setPropertyValue(getPropertyName(PROP_SHADOW_FORMAT), uno::makeAny(aFormat));
        if (xSecond.is())
            xSecond->setPropertyValue(getPropertyName(PROP_SHADOW_FORMAT), uno::makeAny(aFormat));
    }
}

table::ShadowFormat PropertyMap::getShadowFromBorder(const table::BorderLine2& rBorder)
{
    // In Word UI, shadow is a boolean property, in OOXML, it's a boolean
    // property of each 4 border type, finally in Writer the border is a
    // property of the page style, with shadow location, distance and
    // color. See SwWW8ImplReader::SetShadow().
    table::ShadowFormat aFormat;
    aFormat.Color = COL_BLACK;
    aFormat.Location = table::ShadowLocation_BOTTOM_RIGHT;
    aFormat.ShadowWidth = rBorder.LineWidth;
    return aFormat;
}

void SectionPropertyMap::SetBorderDistance( uno::Reference< beans::XPropertySet > const& xStyle,
        PropertyIds eMarginId, PropertyIds eDistId, sal_Int32 nDistance, sal_Int32 nOffsetFrom, sal_uInt32 nLineWidth )
{
    sal_Int32 nDist = nDistance;
    if( nOffsetFrom == 1 ) // From page
    {
        const OUString sMarginName = getPropertyName( eMarginId );
        uno::Any aMargin = xStyle->getPropertyValue( sMarginName );
        sal_Int32 nMargin = 0;
        aMargin >>= nMargin;

        // Change the margins with the border distance
        xStyle->setPropertyValue( sMarginName, uno::makeAny( nDistance ) );

        // Set the distance to ( Margin - distance - nLineWidth )
        nDist = nMargin - nDistance - nLineWidth;
    }
    const OUString sBorderDistanceName = getPropertyName( eDistId );
    if (xStyle.is())
        xStyle->setPropertyValue( sBorderDistanceName, uno::makeAny( nDist ));
}


uno::Reference< text::XTextColumns > SectionPropertyMap::ApplyColumnProperties(
                            uno::Reference< beans::XPropertySet > const& xColumnContainer, DomainMapper_Impl& rDM_Impl )
{
    uno::Reference< text::XTextColumns > xColumns;
    try
    {
        const OUString sTextColumns = getPropertyName( PROP_TEXT_COLUMNS );
        if (xColumnContainer.is())
            xColumnContainer->getPropertyValue(sTextColumns) >>= xColumns;
        uno::Reference< beans::XPropertySet > xColumnPropSet( xColumns, uno::UNO_QUERY_THROW );
        if( !m_bEvenlySpaced &&
                (sal_Int32(m_aColWidth.size()) == (m_nColumnCount + 1 )) &&
                ((sal_Int32(m_aColDistance.size()) == m_nColumnCount) || (sal_Int32(m_aColDistance.size()) == m_nColumnCount + 1)) )
        {
            //the column width in word is an absolute value, in OOo it's relative
            //the distances are both absolute
            sal_Int32 nColSum = 0;
            for( sal_Int32 nCol = 0; nCol <= m_nColumnCount; ++nCol)
            {
                nColSum += m_aColWidth[nCol];
                if(nCol)
                    nColSum += m_aColDistance[nCol -1];
            }

            sal_Int32 nRefValue = xColumns->getReferenceValue();
            double fRel = nColSum ? double( nRefValue ) / double( nColSum ) : 0.0;
            uno::Sequence< text::TextColumn > aColumns( m_nColumnCount + 1 );
            text::TextColumn* pColumn = aColumns.getArray();

            nColSum = 0;
            for( sal_Int32 nCol = 0; nCol <= m_nColumnCount; ++nCol)
            {
                pColumn[nCol].LeftMargin = nCol ? m_aColDistance[nCol - 1 ] / 2 : 0;
                pColumn[nCol].RightMargin = nCol == m_nColumnCount ? 0 : m_aColDistance[nCol] / 2;
                pColumn[nCol].Width = sal_Int32((double( m_aColWidth[nCol] + pColumn[nCol].RightMargin + pColumn[nCol].LeftMargin ) + 0.5 ) * fRel );
                nColSum += pColumn[nCol].Width;
            }
            if( nColSum != nRefValue )
                pColumn[m_nColumnCount].Width -= ( nColSum - nRefValue );
            xColumns->setColumns( aColumns );
        }
        else
        {
            xColumns->setColumnCount( m_nColumnCount + 1 );
            xColumnPropSet->setPropertyValue( getPropertyName( PROP_AUTOMATIC_DISTANCE ), uno::makeAny( m_nColumnDistance ));
        }

        if(m_bSeparatorLineIsOn)
        {
            xColumnPropSet->setPropertyValue("SeparatorLineIsOn", uno::makeAny(true));
            xColumnPropSet->setPropertyValue("SeparatorLineVerticalAlignment", uno::makeAny(style::VerticalAlignment_TOP));
            xColumnPropSet->setPropertyValue("SeparatorLineRelativeHeight", uno::makeAny(static_cast<sal_Int8>(100)));
            xColumnPropSet->setPropertyValue("SeparatorLineColor", uno::makeAny(static_cast<sal_Int32>(COL_BLACK)));
            // 1 twip -> 2 mm100.
            xColumnPropSet->setPropertyValue("SeparatorLineWidth", uno::makeAny(static_cast<sal_Int32>(2)));
        }
        xColumnContainer->setPropertyValue( sTextColumns, uno::makeAny( xColumns ) );
        // Set the columns to be unbalanced if that compatibility option is set or this is the last section.
        if (rDM_Impl.GetSettingsTable()->GetNoColumnBalance() || rDM_Impl.GetIsLastSectionGroup())
            xColumnContainer->setPropertyValue("DontBalanceTextColumns", uno::makeAny(true));
    }
    catch( const uno::Exception& )
    {
        OSL_FAIL( "Exception in SectionPropertyMap::ApplyColumnProperties");
    }
    return xColumns;
}


bool SectionPropertyMap::HasHeader(bool bFirstPage) const
{
    bool bRet = false;
    if( (bFirstPage && m_aFirstPageStyle.is()) ||( !bFirstPage && m_aFollowPageStyle.is()) )
    {
        if( bFirstPage )
            m_aFirstPageStyle->getPropertyValue(
                    getPropertyName(PROP_HEADER_IS_ON) ) >>= bRet;
        else
            m_aFollowPageStyle->getPropertyValue(
                    getPropertyName(PROP_HEADER_IS_ON) ) >>= bRet;
    }
    return bRet;
}


bool SectionPropertyMap::HasFooter(bool bFirstPage) const
{
    bool bRet = false;
    if( (bFirstPage && m_aFirstPageStyle.is()) ||( !bFirstPage && m_aFollowPageStyle.is()) )
    {
        if( bFirstPage )
            m_aFirstPageStyle->getPropertyValue( getPropertyName(PROP_FOOTER_IS_ON) ) >>= bRet;
        else
            m_aFollowPageStyle->getPropertyValue( getPropertyName(PROP_FOOTER_IS_ON) ) >>= bRet;
    }
    return bRet;
}


#define MIN_HEAD_FOOT_HEIGHT 100 //minimum header/footer height

void SectionPropertyMap::CopyHeaderFooterTextProperty (
    const uno::Reference< beans::XPropertySet >& xPrevStyle,
    const uno::Reference< beans::XPropertySet >& xStyle,
    PropertyIds ePropId )
{
    try {
        OUString sName = getPropertyName( ePropId );

        SAL_INFO("writerfilter", "Copying " << sName);
        uno::Reference< text::XTextCopy > xTxt;
        if (xStyle.is())
            xTxt.set(xStyle->getPropertyValue( sName ), uno::UNO_QUERY_THROW );

        uno::Reference< text::XTextCopy > xPrevTxt;
        if (xPrevStyle.is())
            xPrevTxt.set(xPrevStyle->getPropertyValue( sName ), uno::UNO_QUERY_THROW );

        xTxt->copyText( xPrevTxt );
    }
    catch ( const uno::Exception& e )
    {
        SAL_INFO("writerfilter", "An exception occurred in SectionPropertyMap::CopyHeaderFooterTextProperty( ) - " << e.Message);
    }
}

// Copy headers and footers from the previous page style.
void SectionPropertyMap::CopyHeaderFooter(
    const uno::Reference< beans::XPropertySet >& xPrevStyle,
    const uno::Reference< beans::XPropertySet >& xStyle,
    bool bOmitRightHeader, bool bOmitLeftHeader,
    bool bOmitRightFooter, bool bOmitLeftFooter)
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
        xStyle->setPropertyValue( sHeaderIsOn, uno::makeAny( true ) );
        xStyle->setPropertyValue( sHeaderIsShared, uno::makeAny(bHeaderIsShared));
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

    if ( bHasPrevFooter )
    {
        xStyle->setPropertyValue( sFooterIsOn, uno::makeAny( true ) );
        xStyle->setPropertyValue( sFooterIsShared, uno::makeAny(bFooterIsShared) );
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
}

// Copy header and footer content from the previous docx section as needed.
//
// Any headers and footers which were not defined in this docx section
// should be "linked" with the corresponding header or footer from the
// previous section.  LO does not support linking of header/footer content
// across page styles so we just copy the content from the previous section.
void SectionPropertyMap::CopyLastHeaderFooter( bool bFirstPage, DomainMapper_Impl& rDM_Impl )
{
    SAL_INFO("writerfilter", "START>>> SectionPropertyMap::CopyLastHeaderFooter()");
    SectionPropertyMap* pLastContext = rDM_Impl.GetLastSectionContext( );
    if ( pLastContext )
    {
        uno::Reference< beans::XPropertySet > xPrevStyle = pLastContext->GetPageStyle(
                rDM_Impl.GetPageStyles(),
                rDM_Impl.GetTextFactory(),
                bFirstPage );
        uno::Reference< beans::XPropertySet > xStyle = GetPageStyle(
                rDM_Impl.GetPageStyles(),
                rDM_Impl.GetTextFactory(),
                bFirstPage );

        if (bFirstPage)
        {
            CopyHeaderFooter( xPrevStyle, xStyle,
                    !m_bFirstPageHeaderLinkToPrevious, true,
                    !m_bFirstPageFooterLinkToPrevious, true);
        }
        else
        {
            CopyHeaderFooter( xPrevStyle, xStyle,
                    !m_bDefaultHeaderLinkToPrevious,
                    !m_bEvenPageHeaderLinkToPrevious,
                    !m_bDefaultFooterLinkToPrevious,
                    !m_bEvenPageFooterLinkToPrevious);
        }
    }
    SAL_INFO("writerfilter", "END>>> SectionPropertyMap::CopyLastHeaderFooter()");
}

void SectionPropertyMap::PrepareHeaderFooterProperties( bool bFirstPage )
{
    sal_Int32 nTopMargin = m_nTopMargin;
    sal_Int32 nHeaderTop = m_nHeaderTop;
    if(HasHeader(bFirstPage))
    {
        nTopMargin = nHeaderTop;
        if( m_nTopMargin > 0 && m_nTopMargin > nHeaderTop )
            nHeaderTop = m_nTopMargin - nHeaderTop;
        else
            nHeaderTop = 0;

        //minimum header height 1mm
        if( nHeaderTop < MIN_HEAD_FOOT_HEIGHT )
            nHeaderTop = MIN_HEAD_FOOT_HEIGHT;
    }


    if( m_nTopMargin >= 0 ) //fixed height header -> see WW8Par6.hxx
    {
        Insert(PROP_HEADER_IS_DYNAMIC_HEIGHT, uno::makeAny( true ));
        Insert(PROP_HEADER_DYNAMIC_SPACING, uno::makeAny( true ));
        Insert(PROP_HEADER_BODY_DISTANCE, uno::makeAny( nHeaderTop - MIN_HEAD_FOOT_HEIGHT ));// ULSpace.Top()
        Insert(PROP_HEADER_HEIGHT, uno::makeAny( nHeaderTop ));

    }
    else
    {
        //todo: old filter fakes a frame into the header/footer to support overlapping
        //current setting is completely wrong!
        Insert(PROP_HEADER_HEIGHT, uno::makeAny( nHeaderTop ));
        Insert(PROP_HEADER_BODY_DISTANCE, uno::makeAny( m_nTopMargin - nHeaderTop ));
        Insert(PROP_HEADER_IS_DYNAMIC_HEIGHT, uno::makeAny( false ));
        Insert(PROP_HEADER_DYNAMIC_SPACING, uno::makeAny( false ));
    }

    sal_Int32 nBottomMargin = m_nBottomMargin;
    sal_Int32 nHeaderBottom = m_nHeaderBottom;
    if( HasFooter( bFirstPage ) )
    {
        nBottomMargin = nHeaderBottom;
        if( m_nBottomMargin > 0 && m_nBottomMargin > nHeaderBottom )
            nHeaderBottom = m_nBottomMargin - nHeaderBottom;
        else
            nHeaderBottom = 0;
        if( nHeaderBottom < MIN_HEAD_FOOT_HEIGHT )
            nHeaderBottom = MIN_HEAD_FOOT_HEIGHT;
    }

    if( m_nBottomMargin >= 0 ) //fixed height footer -> see WW8Par6.hxx
    {
        Insert(PROP_FOOTER_IS_DYNAMIC_HEIGHT, uno::makeAny( true ));
        Insert(PROP_FOOTER_DYNAMIC_SPACING, uno::makeAny( true ));
        Insert(PROP_FOOTER_BODY_DISTANCE, uno::makeAny( nHeaderBottom - MIN_HEAD_FOOT_HEIGHT));
        Insert(PROP_FOOTER_HEIGHT, uno::makeAny( nHeaderBottom ));
    }
    else
    {
        //todo: old filter fakes a frame into the header/footer to support overlapping
        //current setting is completely wrong!
        Insert(PROP_FOOTER_IS_DYNAMIC_HEIGHT, uno::makeAny( false ));
        Insert(PROP_FOOTER_DYNAMIC_SPACING, uno::makeAny( false ));
        Insert(PROP_FOOTER_HEIGHT, uno::makeAny( m_nBottomMargin - nHeaderBottom ));
        Insert(PROP_FOOTER_BODY_DISTANCE, uno::makeAny( nHeaderBottom ));
    }

    //now set the top/bottom margin for the follow page style
    Insert(PROP_TOP_MARGIN, uno::makeAny( nTopMargin ));
    Insert(PROP_BOTTOM_MARGIN, uno::makeAny( nBottomMargin ));
}

uno::Reference<beans::XPropertySet> lcl_GetRangeProperties(bool bIsFirstSection,
        DomainMapper_Impl& rDM_Impl,
        uno::Reference<text::XTextRange> const& xStartingRange)
{
    uno::Reference< beans::XPropertySet > xRangeProperties;
    if (bIsFirstSection && rDM_Impl.GetBodyText().is())
    {
        uno::Reference<container::XEnumerationAccess> xEnumAccess(rDM_Impl.GetBodyText(), uno::UNO_QUERY_THROW);
        uno::Reference<container::XEnumeration> xEnum = xEnumAccess->createEnumeration();
        xRangeProperties.set(xEnum->nextElement(), uno::UNO_QUERY_THROW);
    }
    else if (xStartingRange.is())
        xRangeProperties.set(xStartingRange, uno::UNO_QUERY_THROW);
    return xRangeProperties;
}

void SectionPropertyMap::HandleMarginsHeaderFooter(bool bFirstPage, DomainMapper_Impl& rDM_Impl)
{
    if( m_nDzaGutter > 0 )
    {
        //todo: iGutterPos from DocProperties are missing
        m_nLeftMargin += m_nDzaGutter;
    }
    Insert(PROP_LEFT_MARGIN, uno::makeAny( m_nLeftMargin  ));
    Insert(PROP_RIGHT_MARGIN, uno::makeAny( m_nRightMargin ));

    if (rDM_Impl.m_oBackgroundColor)
        Insert(PROP_BACK_COLOR, uno::makeAny(*rDM_Impl.m_oBackgroundColor));
    if (!rDM_Impl.m_bHasFtnSep)
        // Set footnote line width to zero, document has no footnote separator.
        Insert(PROP_FOOTNOTE_LINE_RELATIVE_WIDTH, uno::makeAny(sal_Int32(0)));

    /*** if headers/footers are available then the top/bottom margins of the
      header/footer are copied to the top/bottom margin of the page
      */
    CopyLastHeaderFooter( bFirstPage, rDM_Impl );
    PrepareHeaderFooterProperties( bFirstPage );
}

bool SectionPropertyMap::FloatingTableConversion(FloatingTableInfo& rInfo)
{
    // Note that this is just a list of heuristics till sw core can have a
    // table that is floating and can span over multiple pages at the same
    // time.

    sal_Int32 nPageWidth = GetPageWidth();
    sal_Int32 nTextAreaWidth = nPageWidth - GetLeftMargin() - GetRightMargin();
    // Count the layout width of the table.
    sal_Int32 nTableWidth = rInfo.m_nTableWidth;
    sal_Int32 nLeftMargin = 0;
    if (rInfo.getPropertyValue("LeftMargin") >>= nLeftMargin)
        nTableWidth += nLeftMargin;
    sal_Int32 nRightMargin = 0;
    if (rInfo.getPropertyValue("RightMargin") >>= nRightMargin)
        nTableWidth += nRightMargin;

    sal_Int16 nHoriOrientRelation = rInfo.getPropertyValue("HoriOrientRelation").get<sal_Int16>();
    sal_Int16 nVertOrientRelation = rInfo.getPropertyValue("VertOrientRelation").get<sal_Int16>();
    if (nHoriOrientRelation == text::RelOrientation::PAGE_FRAME && nVertOrientRelation == text::RelOrientation::PAGE_FRAME)
    {
        sal_Int16 nHoriOrient = rInfo.getPropertyValue("HoriOrient").get<sal_Int16>();
        sal_Int16 nVertOrient = rInfo.getPropertyValue("VertOrient").get<sal_Int16>();
        if (nHoriOrient == text::HoriOrientation::NONE && nVertOrient == text::VertOrientation::NONE)
        {
            // Anchor position is relative to the page horizontally and vertically as well and is an absolute position.
            // The more close we are to the left edge, the less likely there will be any wrapping.
            // The more close we are to the bottom, the more likely the table will span over to the next page
            // So if we're in the bottom left quarter, don't do any conversion.
            sal_Int32 nHoriOrientPosition = rInfo.getPropertyValue("HoriOrientPosition").get<sal_Int32>();
            sal_Int32 nVertOrientPosition = rInfo.getPropertyValue("VertOrientPosition").get<sal_Int32>();
            sal_Int32 nPageHeight = getProperty(PROP_HEIGHT)->second.get<sal_Int32>();
            if (nHoriOrientPosition < (nPageWidth / 2) && nVertOrientPosition > (nPageHeight / 2))
                return false;
        }
    }

    // If the table is wider than the text area, then don't create a fly
    // for the table: no wrapping will be performed anyway, but multi-page
    // tables will be broken.
    if (nTableWidth < nTextAreaWidth)
        return true;

    // If the position is relative to the edge of the page, then we always
    // create the fly.
    if (rInfo.getPropertyValue("HoriOrientRelation") == text::RelOrientation::PAGE_FRAME)
        return true;

    // If there are columns, always create the fly, otherwise the columns would
    // restrict geometry of the table.
    if (ColumnCount() + 1 >= 2)
        return true;

    return false;
}

void SectionPropertyMap::InheritOrFinalizePageStyles( DomainMapper_Impl& rDM_Impl )
throw ( css::beans::UnknownPropertyException,
        css::beans::PropertyVetoException,
        css::lang::IllegalArgumentException,
        css::lang::WrappedTargetException,
        css::uno::RuntimeException, std::exception )
{
    // don't mess with even/odd headers.
    // just replicate what was done before to avoid causing any regressions
    if( rDM_Impl.GetSettingsTable()->GetEvenAndOddHeaders() )
    {
        if( m_nBreakType != static_cast<sal_Int32>(NS_ooxml::LN_Value_ST_SectionMark_nextColumn) )
        {
            OUString aName = m_bTitlePage ? m_sFirstPageStyleName : m_sFollowPageStyleName;
            if( !aName.isEmpty() )
            {
                HandleMarginsHeaderFooter( /*bFirstPage=*/false, rDM_Impl );
                if( m_bTitlePage )
                    HandleMarginsHeaderFooter( /*bFirstPage=*/true, rDM_Impl );
                uno::Reference<beans::XPropertySet> xPageStyle( rDM_Impl.GetPageStyles()->getByName(aName), uno::UNO_QUERY_THROW );
                if( rDM_Impl.IsNewDoc() )
                {
                    ApplyProperties_( xPageStyle );
                    if( m_bTitlePage && m_aFollowPageStyle.is() )
                        ApplyProperties_( m_aFollowPageStyle );
                }
            }
        }
        return;
    }

    const uno::Reference< container::XNameContainer >& xPageStyles = rDM_Impl.GetPageStyles();
    const uno::Reference < lang::XMultiServiceFactory >& xTextFactory = rDM_Impl.GetTextFactory();

    // if no new styles have been created for this section, inherit from the previous section,
    // otherwise apply this section's settings to the new style.
    SectionPropertyMap* pLastContext = rDM_Impl.GetLastSectionContext();
    if( pLastContext && m_sFirstPageStyleName.isEmpty() )
        m_sFirstPageStyleName =  pLastContext->GetPageStyleName( /*bFirst=*/true );
    else
    {
        HandleMarginsHeaderFooter( /*bFirst=*/true, rDM_Impl );
        GetPageStyle( xPageStyles, xTextFactory, /*bFirst=*/true );
        if( rDM_Impl.IsNewDoc() && m_aFirstPageStyle.is() )
            ApplyProperties_( m_aFirstPageStyle );
    }

    if( pLastContext && m_sFollowPageStyleName.isEmpty() )
        m_sFollowPageStyleName = pLastContext->GetPageStyleName();
    else
    {
        HandleMarginsHeaderFooter( /*bFirst=*/false, rDM_Impl );
        GetPageStyle( xPageStyles, xTextFactory, /*bFirst=*/false );
        if( rDM_Impl.IsNewDoc() && m_aFollowPageStyle.is() )
            ApplyProperties_( m_aFollowPageStyle );
    }

    GetPageStyle( xPageStyles, xTextFactory, /*bFirst=*/true );
    // Chain m_aFollowPageStyle to be after m_aFirstPageStyle
    m_aFirstPageStyle->setPropertyValue( "FollowStyle", uno::makeAny(m_sFollowPageStyleName) );
}

void SectionPropertyMap::CloseSectionGroup( DomainMapper_Impl& rDM_Impl )
{
    // Text area width is known at the end of a section: decide if tables should be converted or not.
    std::vector<FloatingTableInfo>& rPendingFloatingTables = rDM_Impl.m_aPendingFloatingTables;
    uno::Reference<text::XTextAppendAndConvert> xBodyText( rDM_Impl.GetBodyText(), uno::UNO_QUERY );
    for (FloatingTableInfo & rInfo : rPendingFloatingTables)
    {
        if (FloatingTableConversion(rInfo))
            xBodyText->convertToTextFrame(rInfo.m_xStart, rInfo.m_xEnd, rInfo.m_aFrameProperties);
    }
    rPendingFloatingTables.clear();

    if( m_nLnnMod )
    {
        bool bFirst = rDM_Impl.IsLineNumberingSet();
        rDM_Impl.SetLineNumbering( m_nLnnMod, m_nLnc, m_ndxaLnn );
        if( m_nLnnMin > 0 || (bFirst && m_nLnc == NS_ooxml::LN_Value_ST_LineNumberRestart_newSection))
        {
            //set the starting value at the beginning of the section
            try
            {
                uno::Reference< beans::XPropertySet > xRangeProperties;
                if( m_xStartingRange.is() )
                {
                    xRangeProperties.set( m_xStartingRange, uno::UNO_QUERY_THROW );
                }
                else
                {
                    //set the start value at the beginning of the document
                    xRangeProperties.set( rDM_Impl.GetTextDocument()->getText()->getStart(), uno::UNO_QUERY_THROW );
                }
                xRangeProperties->setPropertyValue( getPropertyName( PROP_PARA_LINE_NUMBER_START_VALUE ), uno::makeAny( m_nLnnMin ));
            }
            catch( const uno::Exception& )
            {
                OSL_FAIL( "Exception in SectionPropertyMap::CloseSectionGroup");
            }
        }
    }

    // depending on the break type no page styles should be created
    // If the section type is missing, but we have columns, then this should be
    // handled as a continuous section break.
    if(m_nBreakType == static_cast<sal_Int32>(NS_ooxml::LN_Value_ST_SectionMark_continuous) || (m_nBreakType == -1 && m_nColumnCount > 0))
    {
        //todo: insert a section or access the already inserted section
        uno::Reference< beans::XPropertySet > xSection =
                                    rDM_Impl.appendTextSectionAfter( m_xStartingRange );
        if( m_nColumnCount > 0 && xSection.is() )
            ApplyColumnProperties( xSection, rDM_Impl );

        try
        {
            InheritOrFinalizePageStyles( rDM_Impl );
            OUString aName = m_bTitlePage ? m_sFirstPageStyleName : m_sFollowPageStyleName;
            uno::Reference<beans::XPropertySet> xRangeProperties( lcl_GetRangeProperties(m_bIsFirstSection, rDM_Impl, m_xStartingRange) );
            if ( m_bIsFirstSection && !aName.isEmpty() && xRangeProperties.is() )
                xRangeProperties->setPropertyValue( getPropertyName(PROP_PAGE_DESC_NAME), uno::makeAny(aName) );
        }
        catch( const uno::Exception& )
        {
            SAL_WARN("writerfilter", "failed to set PageDescName!");
        }
    }
    // If the section is of type "New column" (0x01), then simply insert a column break.
    // But only if there actually are columns on the page, otherwise a column break
    // seems to be handled like a page break by MSO.
    else if(m_nBreakType == static_cast<sal_Int32>(NS_ooxml::LN_Value_ST_SectionMark_nextColumn) && m_nColumnCount > 0 )
    {
        try
        {
            InheritOrFinalizePageStyles( rDM_Impl );
            uno::Reference< beans::XPropertySet > xRangeProperties;
            if( m_xStartingRange.is() )
            {
                xRangeProperties.set( m_xStartingRange, uno::UNO_QUERY_THROW );
            }
            else
            {
                //set the start value at the beginning of the document
                xRangeProperties.set( rDM_Impl.GetTextDocument()->getText()->getStart(), uno::UNO_QUERY_THROW );
            }
            xRangeProperties->setPropertyValue( getPropertyName(PROP_BREAK_TYPE), uno::makeAny(style::BreakType_COLUMN_BEFORE) );
        }
        catch( const uno::Exception& ) {}
    }
    else
    {
        //get the properties and create appropriate page styles
        uno::Reference< beans::XPropertySet > xFollowPageStyle = GetPageStyle( rDM_Impl.GetPageStyles(), rDM_Impl.GetTextFactory(), false );

        HandleMarginsHeaderFooter(/*bFirstPage=*/false, rDM_Impl);

        const OUString sTrayIndex = getPropertyName( PROP_PRINTER_PAPER_TRAY_INDEX );
        if( m_nPaperBin >= 0 )
            xFollowPageStyle->setPropertyValue( sTrayIndex, uno::makeAny( m_nPaperBin ) );
        if ( rDM_Impl.GetSettingsTable()->GetMirrorMarginSettings() )
        {
            Insert(PROP_PAGE_STYLE_LAYOUT, uno::makeAny(style::PageStyleLayout_MIRRORED));
        }
        uno::Reference< text::XTextColumns > xColumns;
        if( m_nColumnCount > 0 )
            xColumns = ApplyColumnProperties( xFollowPageStyle, rDM_Impl );

        //prepare text grid properties
        sal_Int32 nHeight = 1;
        boost::optional<PropertyMap::Property> pProp = getProperty(PROP_HEIGHT);
        if(pProp)
            pProp->second >>= nHeight;

        sal_Int32 nWidth = 1;
        pProp = getProperty(PROP_WIDTH);
        if(pProp)
            pProp->second >>= nWidth;

        text::WritingMode eWritingMode = text::WritingMode_LR_TB;
        pProp = getProperty(PROP_WRITING_MODE);
        if(pProp)
            pProp->second >>= eWritingMode;

        sal_Int32 nTextAreaHeight = eWritingMode == text::WritingMode_LR_TB ?
            nHeight - m_nTopMargin - m_nBottomMargin :
            nWidth - m_nLeftMargin - m_nRightMargin;

        sal_Int32 nGridLinePitch = m_nGridLinePitch;
        //sep.dyaLinePitch
        if (nGridLinePitch < 1 || nGridLinePitch > 31680)
        {
            SAL_WARN("writerfilter", "sep.dyaLinePitch outside legal range: " << nGridLinePitch);
            nGridLinePitch = 1;
        }

        Insert(PROP_GRID_LINES, uno::makeAny( static_cast<sal_Int16>(nTextAreaHeight/nGridLinePitch)));

        // PROP_GRID_MODE
        Insert( PROP_GRID_MODE, uno::makeAny( static_cast<sal_Int16> (m_nGridType) ));
        if (m_nGridType == text::TextGridMode::LINES_AND_CHARS)
        {
            Insert( PROP_GRID_SNAP_TO_CHARS, uno::makeAny(m_bGridSnapToChars));
        }

        sal_Int32 nCharWidth = 423; //240 twip/ 12 pt
        //todo: is '0' the right index here?
        const StyleSheetEntryPtr pEntry = rDM_Impl.GetStyleSheetTable()->FindStyleSheetByISTD(OUString::number(0, 16));
        if( pEntry.get( ) )
        {
            boost::optional<PropertyMap::Property> pPropHeight = pEntry->pProperties->getProperty(PROP_CHAR_HEIGHT_ASIAN);
            if(pProp)
            {
                double fHeight = 0;
                if( pPropHeight->second >>= fHeight )
                    nCharWidth = ConversionHelper::convertTwipToMM100( (long)( fHeight * 20.0 + 0.5 ));
            }
        }

        //dxtCharSpace
        if(m_nDxtCharSpace)
        {
            sal_Int32 nCharSpace = m_nDxtCharSpace;
            //main lives in top 20 bits, and is signed.
            sal_Int32 nMain = (nCharSpace & 0xFFFFF000);
            nMain /= 0x1000;
            nCharWidth += ConversionHelper::convertTwipToMM100( nMain * 20 );

            sal_Int32 nFraction = (nCharSpace & 0x00000FFF);
            nFraction = (nFraction * 20)/0xFFF;
            nCharWidth += ConversionHelper::convertTwipToMM100( nFraction );
        }
        Insert(PROP_GRID_BASE_HEIGHT, uno::makeAny( nCharWidth ));
        sal_Int32 nRubyHeight = nGridLinePitch - nCharWidth;
        if(nRubyHeight < 0 )
            nRubyHeight = 0;
        Insert(PROP_GRID_RUBY_HEIGHT, uno::makeAny( nRubyHeight ));

        if (m_nPageNumberType >= 0)
            Insert(PROP_NUMBERING_TYPE, uno::makeAny(m_nPageNumberType));

        // #i119558#, force to set document as standard page mode,
        // refer to ww8 import process function "SwWW8ImplReader::SetDocumentGrid"
        try
        {
            uno::Reference< beans::XPropertySet > xDocProperties;
            xDocProperties.set( rDM_Impl.GetTextDocument(), uno::UNO_QUERY_THROW );
            bool bSquaredPageMode = false;
            Insert(PROP_GRID_STANDARD_MODE, uno::makeAny( !bSquaredPageMode ));
            xDocProperties->setPropertyValue("DefaultPageMode", uno::makeAny( bSquaredPageMode ));
        }
        catch (const uno::Exception& rEx)
        {
            OSL_ENSURE( false, "Exception in SectionPropertyMap::CloseSectionGroup");
            (void)rEx;
        }

        if (rDM_Impl.IsNewDoc())
            ApplyProperties_( xFollowPageStyle );

        //todo: creating a "First Page" style depends on HasTitlePage und _fFacingPage_
        if( m_bTitlePage )
        {
            CopyLastHeaderFooter( true, rDM_Impl );
            PrepareHeaderFooterProperties( true );
            uno::Reference< beans::XPropertySet > xFirstPageStyle = GetPageStyle(
                                rDM_Impl.GetPageStyles(), rDM_Impl.GetTextFactory(), true );
            if (rDM_Impl.IsNewDoc())
                ApplyProperties_( xFirstPageStyle );

            sal_Int32 nPaperBin = m_nFirstPaperBin >= 0 ? m_nFirstPaperBin : m_nPaperBin >= 0 ? m_nPaperBin : 0;
            if( nPaperBin )
                xFirstPageStyle->setPropertyValue( sTrayIndex, uno::makeAny( nPaperBin ) );
            if( xColumns.is() )
                xFirstPageStyle->setPropertyValue(
                    getPropertyName( PROP_TEXT_COLUMNS ), uno::makeAny( xColumns ));
        }

        ApplyBorderToPageStyles( rDM_Impl.GetPageStyles( ), rDM_Impl.GetTextFactory( ), m_nBorderParams );

        try
        {
            {
                //now apply this break at the first paragraph of this section
                uno::Reference<beans::XPropertySet> xRangeProperties(lcl_GetRangeProperties(m_bIsFirstSection, rDM_Impl, m_xStartingRange));

                // Handle page breaks with odd/even page numbering. We need to use an extra page style for setting the page style
                // to left/right, because if we set it to the normal style, we'd set it to "First Page"/"Default Style", which would
                // break them (all default pages would be only left or right).
                if (m_nBreakType == static_cast<sal_Int32>(NS_ooxml::LN_Value_ST_SectionMark_evenPage) || m_nBreakType == static_cast<sal_Int32>(NS_ooxml::LN_Value_ST_SectionMark_oddPage))
                {
                    OUString* pageStyle = m_bTitlePage ? &m_sFirstPageStyleName : &m_sFollowPageStyleName;
                    OUString evenOddStyleName = lcl_FindUnusedPageStyleName(rDM_Impl.GetPageStyles()->getElementNames());
                    uno::Reference< beans::XPropertySet > evenOddStyle(
                            rDM_Impl.GetTextFactory()->createInstance("com.sun.star.style.PageStyle"),
                            uno::UNO_QUERY);
                    // Unfortunately using setParent() does not work for page styles, so make a deep copy of the page style.
                    uno::Reference< beans::XPropertySet > pageProperties( m_bTitlePage ? m_aFirstPageStyle : m_aFollowPageStyle );
                    uno::Reference< beans::XPropertySetInfo > pagePropertiesInfo( pageProperties->getPropertySetInfo());
                    uno::Sequence< beans::Property > propertyList( pagePropertiesInfo->getProperties());
                    for( int i = 0; i < propertyList.getLength(); ++i )
                    {
                        if(( propertyList[i].Attributes & beans::PropertyAttribute::READONLY ) == 0 )
                            evenOddStyle->setPropertyValue( propertyList[ i ].Name, pageProperties->getPropertyValue( propertyList[ i ].Name ));
                    }
                    evenOddStyle->setPropertyValue("FollowStyle", uno::makeAny(*pageStyle));
                    rDM_Impl.GetPageStyles()->insertByName( evenOddStyleName, uno::makeAny( evenOddStyle ) );
                    evenOddStyle->setPropertyValue("HeaderIsOn", uno::makeAny(false));
                    evenOddStyle->setPropertyValue("FooterIsOn", uno::makeAny(false));
                    CopyHeaderFooter( pageProperties, evenOddStyle );
                    *pageStyle = evenOddStyleName; // And use it instead of the original one (which is set as follow of this one).
                    if (m_nBreakType == static_cast<sal_Int32>(NS_ooxml::LN_Value_ST_SectionMark_evenPage))
                        evenOddStyle->setPropertyValue(getPropertyName(PROP_PAGE_STYLE_LAYOUT), uno::makeAny(style::PageStyleLayout_LEFT));
                    else if (m_nBreakType == static_cast<sal_Int32>(NS_ooxml::LN_Value_ST_SectionMark_oddPage))
                        evenOddStyle->setPropertyValue(getPropertyName(PROP_PAGE_STYLE_LAYOUT), uno::makeAny(style::PageStyleLayout_RIGHT));
                }

                if (xRangeProperties.is() && rDM_Impl.IsNewDoc())
                    xRangeProperties->setPropertyValue(
                        getPropertyName( PROP_PAGE_DESC_NAME ),
                        uno::makeAny( m_bTitlePage ?  m_sFirstPageStyleName
                                      : m_sFollowPageStyleName ));

                if(m_bPageNoRestart || m_nPageNumber >= 0)
                {
                    sal_Int16 nPageNumber = m_nPageNumber >= 0 ? static_cast< sal_Int16 >(m_nPageNumber) : 1;
                    xRangeProperties->setPropertyValue(getPropertyName( PROP_PAGE_NUMBER_OFFSET ),
                        uno::makeAny( nPageNumber ));
                }
            }
        }
        catch (const uno::Exception&)
        {
            OSL_FAIL( "Exception in SectionPropertyMap::CloseSectionGroup");
        }
    }
    rDM_Impl.SetIsLastSectionGroup(false);
    rDM_Impl.SetIsFirstParagraphInSection(true);
}

// Clear the flag that says we should take the header/footer content from
// the previous section.  This should be called when we encounter a header
// or footer definition for this section.
void SectionPropertyMap::ClearHeaderFooterLinkToPrevious(
        bool bHeader, PageType eType )
{
    if( bHeader ) {
        switch( eType ) {
            case PAGE_FIRST: m_bFirstPageHeaderLinkToPrevious = false; break;
            case PAGE_LEFT:  m_bEvenPageHeaderLinkToPrevious = false; break;
            case PAGE_RIGHT: m_bDefaultHeaderLinkToPrevious = false; break;
            // no default case as all enumeration values have been covered
        }
    }
    else
    {
        switch( eType ) {
            case PAGE_FIRST: m_bFirstPageFooterLinkToPrevious = false; break;
            case PAGE_LEFT:  m_bEvenPageFooterLinkToPrevious = false; break;
            case PAGE_RIGHT: m_bDefaultFooterLinkToPrevious = false; break;
        }
    }
}

class NamedPropertyValue {
    OUString m_aName;
public:
    explicit NamedPropertyValue(const OUString& i_aStr)
        : m_aName(i_aStr)   { }
    bool operator() (beans::PropertyValue& aVal)
    {   return aVal.Name == m_aName;    }
};
void SectionPropertyMap::ApplyProperties_(
        uno::Reference< beans::XPropertySet > const& xStyle)
{
    uno::Reference<beans::XMultiPropertySet> const xMultiSet(xStyle,
            uno::UNO_QUERY);

    std::vector<OUString> vNames;
    std::vector<uno::Any> vValues;
    { //Convert GetPropertyValues() value into something useful
        uno::Sequence<beans::PropertyValue> vPropVals = GetPropertyValues();

        //Temporarily store the items that are in grab bags
        uno::Sequence<beans::PropertyValue> vCharVals;
        uno::Sequence<beans::PropertyValue> vParaVals;
        beans::PropertyValue* pCharGrabBag = std::find_if(vPropVals.begin(),vPropVals.end(),NamedPropertyValue("CharInteropGrabBag") );
        if (pCharGrabBag!=vPropVals.end())
            (pCharGrabBag->Value)>>=vCharVals;
        beans::PropertyValue* pParaGrabBag = std::find_if(vPropVals.begin(),vPropVals.end(),NamedPropertyValue("ParaInteropGrabBag") );
        if (pParaGrabBag!=vPropVals.end())
            (pParaGrabBag->Value)>>=vParaVals;

        for (beans::PropertyValue* pIter = vPropVals.begin(); pIter!=vPropVals.end(); ++pIter)
        {
            if(pIter!=pCharGrabBag && pIter!=pParaGrabBag)
            {
                vNames.push_back(pIter->Name);
                vValues.push_back(pIter->Value);
            }
        }
        for (beans::PropertyValue & v : vCharVals)
        {
            vNames.push_back(v.Name);
            vValues.push_back(v.Value);
        }
        for (beans::PropertyValue & v : vParaVals)
        {
            vNames.push_back(v.Name);
            vValues.push_back(v.Value);
        }
    }
    if (xMultiSet.is())
    {
        try
        {
            xMultiSet->setPropertyValues(comphelper::containerToSequence(vNames), comphelper::containerToSequence(vValues));
        }
        catch( const uno::Exception& )
        {
            OSL_FAIL( "Exception in SectionPropertyMap::ApplyProperties_");
        }
        return;
    }
    for (size_t i = 0; i < vNames.size(); ++i)
    {
        try
        {
            if (xStyle.is())
                xStyle->setPropertyValue(vNames[i], vValues[i]);
        }
        catch( const uno::Exception& )
        {
            OSL_FAIL( "Exception in SectionPropertyMap::ApplyProperties_");
        }
    }
}

sal_Int32 SectionPropertyMap::GetPageWidth()
{
    return getProperty(PROP_WIDTH)->second.get<sal_Int32>();
}

StyleSheetPropertyMap::StyleSheetPropertyMap() :
    mnCT_TrPrBase_jc( 0 ),
    mnCT_TblWidth_w( 0 ),
    mnCT_TblWidth_type( 0 ),
    mbCT_TrPrBase_jcSet( false ),
    mbCT_TblWidth_wSet( false ),
    mbCT_TblWidth_typeSet( false ),
    mnListId( -1 ),
    mnListLevel( -1 ),
    mnOutlineLevel( -1 ),
    mnNumId( -1 )
{
}


StyleSheetPropertyMap::~StyleSheetPropertyMap()
{
}


ParagraphProperties::ParagraphProperties() :
    m_bFrameMode( false ),
    m_nDropCap(NS_ooxml::LN_Value_doc_ST_DropCap_none),
    m_nLines(0),
    m_w(-1),
    m_h(-1),
    m_nWrap(-1),
    m_hAnchor(-1),
    m_vAnchor(-1),
    m_x(-1),
    m_bxValid( false ),
    m_y(-1),
    m_byValid( false ),
    m_hSpace(-1),
    m_vSpace(-1),
    m_hRule(-1),
    m_xAlign(-1),
    m_yAlign(-1),
    m_bAnchorLock(false),
    m_nDropCapLength(0)
{
}


ParagraphProperties::ParagraphProperties(const ParagraphProperties& rCopy) :
    m_bFrameMode ( rCopy.m_bFrameMode),
    m_nDropCap   ( rCopy.m_nDropCap),
    m_nLines     ( rCopy.m_nLines),
    m_w          ( rCopy.m_w),
    m_h          ( rCopy.m_h),
    m_nWrap      ( rCopy.m_nWrap),
    m_hAnchor    ( rCopy.m_hAnchor),
    m_vAnchor    ( rCopy.m_vAnchor),
    m_x          ( rCopy.m_x),
    m_bxValid    ( rCopy.m_bxValid),
    m_y          ( rCopy.m_y),
    m_byValid    ( rCopy.m_byValid),
    m_hSpace     ( rCopy.m_hSpace),
    m_vSpace     ( rCopy.m_vSpace),
    m_hRule      ( rCopy.m_hRule),
    m_xAlign     ( rCopy.m_xAlign),
    m_yAlign     ( rCopy.m_yAlign),
    m_bAnchorLock( rCopy.m_bAnchorLock),
    m_nDropCapLength( rCopy.m_nDropCapLength ),
    m_sParaStyleName( rCopy.m_sParaStyleName),
    m_xStartingRange( rCopy.m_xStartingRange ),
    m_xEndingRange( rCopy.m_xEndingRange)
{
}


ParagraphProperties::~ParagraphProperties()
{
}


bool ParagraphProperties::operator==(const ParagraphProperties& rCompare)
{
    return
        m_bFrameMode == rCompare.m_bFrameMode &&
        m_nDropCap   == rCompare.m_nDropCap &&
        m_nLines     == rCompare.m_nLines &&
        m_w          == rCompare.m_w &&
        m_h          == rCompare.m_h &&
        m_nWrap      == rCompare.m_nWrap &&
        m_hAnchor    == rCompare.m_hAnchor &&
        m_vAnchor    == rCompare.m_vAnchor &&
        m_x          == rCompare.m_x &&
        m_bxValid    == rCompare.m_bxValid &&
        m_y          == rCompare.m_y &&
        m_byValid    == rCompare.m_byValid &&
        m_hSpace     == rCompare.m_hSpace &&
        m_vSpace     == rCompare.m_vSpace &&
        m_hRule      == rCompare.m_hRule &&
        m_xAlign     == rCompare.m_xAlign &&
        m_yAlign     == rCompare.m_yAlign &&
        m_bAnchorLock== rCompare.m_bAnchorLock;
}

void ParagraphProperties::ResetFrameProperties()
{
    m_bFrameMode = false;
    m_nDropCap = NS_ooxml::LN_Value_doc_ST_DropCap_none;
    m_nLines = 0;
    m_w = -1;
    m_h = -1;
    m_nWrap = -1;
    m_hAnchor = -1;
    m_vAnchor = -1;
    m_x = -1;
    m_bxValid = false;
    m_y = -1;
    m_byValid = false;
    m_hSpace = -1;
    m_vSpace = -1;
    m_hRule = -1;
    m_xAlign = -1;
    m_yAlign = -1;
    m_bAnchorLock = false;
    m_nDropCapLength = 0;
}


ParagraphPropertyMap::ParagraphPropertyMap()
{
}


ParagraphPropertyMap::~ParagraphPropertyMap()
{
}


TablePropertyMap::TablePropertyMap()
{
}


TablePropertyMap::~TablePropertyMap()
{
}


bool TablePropertyMap::getValue( TablePropertyMapTarget eWhich, sal_Int32& nFill )
{
    if( eWhich < TablePropertyMapTarget_MAX )
    {
        if(m_aValidValues[eWhich].bValid)
            nFill = m_aValidValues[eWhich].nValue;
        return m_aValidValues[eWhich].bValid;
    }
    else
    {
        OSL_FAIL( "invalid TablePropertyMapTarget");
        return false;
    }
}


void TablePropertyMap::setValue( TablePropertyMapTarget eWhich, sal_Int32 nSet )
{
    if( eWhich < TablePropertyMapTarget_MAX )
    {
        m_aValidValues[eWhich].bValid = true;
        m_aValidValues[eWhich].nValue = nSet;
    }
    else
        OSL_FAIL( "invalid TablePropertyMapTarget");
}


void TablePropertyMap::insertTableProperties( const PropertyMap* pMap )
{
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().startElement("TablePropertyMap.insertTableProperties");
    pMap->dumpXml();
#endif

    const TablePropertyMap* pSource = dynamic_cast< const TablePropertyMap* >(pMap);
    if( pSource )
    {
        for( sal_Int32 eTarget = TablePropertyMapTarget_START;
            eTarget < TablePropertyMapTarget_MAX; ++eTarget )
        {
            if( pSource->m_aValidValues[eTarget].bValid )
            {
                m_aValidValues[eTarget].bValid = true;
                m_aValidValues[eTarget].nValue = pSource->m_aValidValues[eTarget].nValue;
            }
        }
    }
#ifdef DEBUG_WRITERFILTER
    dumpXml();
    TagLogger::getInstance().endElement();
#endif
}


}//namespace dmapper
}//namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
