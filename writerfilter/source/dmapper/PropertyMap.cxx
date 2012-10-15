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
#include <rtl/ustring.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/TextGridMode.hpp>
#include <com/sun/star/text/XTextCopy.hpp>
#include "dmapperLoggers.hxx"
#include "PropertyMapHelper.hxx"

using namespace ::com::sun::star;

namespace writerfilter {
namespace dmapper{



PropertyMap::PropertyMap() :
    m_cFootnoteSymbol( 0 ),
    m_nFootnoteFontId( -1 )
{
}


PropertyMap::~PropertyMap()
{
}


uno::Sequence< beans::PropertyValue > PropertyMap::GetPropertyValues()
{
    if(!m_aValues.getLength() && size())
    {
        m_aValues.realloc( size() );
        ::com::sun::star::beans::PropertyValue* pValues = m_aValues.getArray();
        //style names have to be the first elements within the property sequence
        //otherwise they will overwrite 'hard' attributes
        sal_Int32 nValue = 0;
        PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
        PropertyMap::iterator aParaStyleIter = find(PropertyDefinition( PROP_PARA_STYLE_NAME, false ) );
        if( aParaStyleIter != end())
        {
            pValues[nValue].Name = rPropNameSupplier.GetName( aParaStyleIter->first.eId );
            pValues[nValue].Value = aParaStyleIter->second;
            ++nValue;
        }

        PropertyMap::iterator aCharStyleIter = find(PropertyDefinition( PROP_CHAR_STYLE_NAME, false ));
        if( aCharStyleIter != end())
        {
            pValues[nValue].Name = rPropNameSupplier.GetName( aCharStyleIter->first.eId );
            pValues[nValue].Value = aCharStyleIter->second;
            ++nValue;
        }
        PropertyMap::iterator aNumRuleIter = find(PropertyDefinition( PROP_NUMBERING_RULES, false ) );
        if( aNumRuleIter != end())
        {
            pValues[nValue].Name = rPropNameSupplier.GetName( aNumRuleIter->first.eId );
            pValues[nValue].Value = aNumRuleIter->second;
            ++nValue;
        }
        PropertyMap::iterator aMapIter = begin();
        for( ; nValue < m_aValues.getLength(); ++aMapIter )
        {
            if( aMapIter != aParaStyleIter && aMapIter != aCharStyleIter && aMapIter != aNumRuleIter )
            {
                pValues[nValue].Name = rPropNameSupplier.GetName( aMapIter->first.eId );
                pValues[nValue].Value = aMapIter->second;
                ++nValue;
            }
        }
    }
    return m_aValues;
}

#ifdef DEBUG_DMAPPER_PROPERTY_MAP
static void lcl_AnyToTag(const uno::Any & rAny)
{
    try {
        sal_Int32 aInt = 0;
        rAny >>= aInt;
        dmapper_logger->attribute("value", aInt);

        sal_uInt32 auInt = 0;
        rAny >>= auInt;
        dmapper_logger->attribute("unsignedValue", auInt);

        float aFloat = 0.0f;
        rAny >>= aFloat;
        dmapper_logger->attribute("floatValue", aFloat);

        OUString aStr;
        rAny >>= aStr;
        dmapper_logger->attribute("stringValue", aStr);
    }
    catch (...) {
    }
}
#endif

void PropertyMap::Insert( PropertyIds eId, bool bIsTextProperty, const uno::Any& rAny, bool bOverwrite )
{
#ifdef DEBUG_DMAPPER_PROPERTY_MAP
    const OUString& rInsert = PropertyNameSupplier::
        GetPropertyNameSupplier().GetName(eId);

    dmapper_logger->startElement("propertyMap.insert");
    dmapper_logger->attribute("name", rInsert);
    lcl_AnyToTag(rAny);
    dmapper_logger->endElement();
#endif

    PropertyMap::iterator aElement = find(PropertyDefinition( eId, bIsTextProperty ) );
    if( aElement != end())
    {
        if(!bOverwrite)
            return;
        erase( aElement );
    }
    _PropertyMap::insert( PropertyMap::value_type
                          (PropertyDefinition( eId, bIsTextProperty),
                           rAny ));
    Invalidate();
}

#if OSL_DEBUG_LEVEL > 1
void PropertyMap::dumpXml( const TagLogger::Pointer_t pLogger ) const
{
    pLogger->startElement("PropertyMap");

    PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
    PropertyMap::const_iterator aMapIter = begin();
    while (aMapIter != end())
    {
        pLogger->startElement("property");

        pLogger->attribute("name", rPropNameSupplier.GetName( aMapIter->first.eId ));

        switch (aMapIter->first.eId)
        {
            case PROP_TABLE_COLUMN_SEPARATORS:
               lcl_DumpTableColumnSeparators(pLogger, aMapIter->second);
                break;
            default:
            {
                try {
                    sal_Int32 aInt = 0;
                    aMapIter->second >>= aInt;
                    pLogger->attribute("value", aInt);

                    sal_uInt32 auInt = 0;
                    aMapIter->second >>= auInt;
                    pLogger->attribute("unsignedValue", auInt);

                    float aFloat = 0.0;
                    aMapIter->second >>= aFloat;
                    pLogger->attribute("floatValue", aFloat);

                    OUString aStr;
                    aMapIter->second >>= auInt;
                    pLogger->attribute("stringValue", aStr);
                }
                catch (...) {
                }
            }
                break;
        }

        pLogger->endElement();

        ++aMapIter;
    }

    pLogger->endElement();
}
#endif



template<class T>
    struct removeExistingElements : public ::std::unary_function<T, void>
{
  PropertyMap& rMap;

  removeExistingElements(PropertyMap& _rMap ) : rMap(_rMap) {}
  void operator() (T x)
  {
    PropertyMap::iterator aElement = rMap.find(x.first);
    if( aElement != rMap.end())
        rMap.erase( aElement );
  }
};


void PropertyMap::insert( const PropertyMapPtr pMap, bool bOverwrite )
{
    if( pMap.get() )
    {
        if( bOverwrite )
            ::std::for_each( pMap->begin(), pMap->end(), removeExistingElements<PropertyMap::value_type>(*this) );
        _PropertyMap::insert(pMap->begin(), pMap->end());
        insertTableProperties(pMap.get());

        Invalidate();
    }
}


const uno::Reference< text::XFootnote>&  PropertyMap::GetFootnote() const
{
    return m_xFootnote;
}


void PropertyMap::insertTableProperties( const PropertyMap* )
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->element("PropertyMap.insertTableProperties");
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
    ,m_bGutterRTL( false )
    ,m_bSFBiDi( false )
    ,m_nGridType(0)
    ,m_nGridLinePitch( 1 )
    ,m_nDxtCharSpace( 0 )
    ,m_nLnnMod( 0 )
    ,m_nLnc( 0 )
    ,m_ndxaLnn( 0 )
    ,m_nLnnMin( 0 )
{
    static sal_Int32 nNumber = 0;
    nSectionNumber = nNumber++;
    memset(&m_pBorderLines, 0x00, sizeof(m_pBorderLines));
    for( sal_Int32 nBorder = 0; nBorder < 4; ++nBorder )
        m_nBorderDistances[ nBorder ] = -1;
    //todo: set defaults in ApplyPropertiesToPageStyles
    //initialize defaults
    PaperInfo aLetter(PAPER_LETTER);
    //page height, 1/100mm
    Insert( PROP_HEIGHT, false, uno::makeAny( (sal_Int32) aLetter.getHeight() ) );
    //page width, 1/100mm
    Insert( PROP_WIDTH, false, uno::makeAny( (sal_Int32) aLetter.getWidth() ) );
    //page left margin, default 0x708 (1800) twip -> 3175 1/100 mm
    Insert( PROP_LEFT_MARGIN, false, uno::makeAny( (sal_Int32) 3175 ) );
    //page right margin, default 0x708 (1800) twip -> 3175 1/100 mm
    Insert( PROP_RIGHT_MARGIN, false, uno::makeAny( (sal_Int32) 3175 ) );
    //page top margin, default 0x5a0 (1440) twip -> 2540 1/100 mm
    Insert( PROP_TOP_MARGIN, false, uno::makeAny( (sal_Int32)2540 ) );
    //page bottom margin, default 0x5a0 (1440) twip -> 2540 1/100 mm
    Insert( PROP_BOTTOM_MARGIN, false, uno::makeAny( (sal_Int32) 2540 ) );
    //page style layout
    Insert(PROP_PAGE_STYLE_LAYOUT, false, uno::makeAny(style::PageStyleLayout_ALL));
    uno::Any aFalse( ::uno::makeAny( false ) );
    Insert( PROP_GRID_DISPLAY, false, aFalse);
    Insert( PROP_GRID_PRINT, false, aFalse);
    Insert( PROP_GRID_MODE, false, uno::makeAny(text::TextGridMode::NONE));


    if( m_bIsFirstSection )
    {
        PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
        m_sFirstPageStyleName = rPropNameSupplier.GetName( PROP_FIRST_PAGE );
        m_sFollowPageStyleName = rPropNameSupplier.GetName( PROP_STANDARD );
    }
}


SectionPropertyMap::~SectionPropertyMap()
{
    for( sal_Int16 ePos = BORDER_LEFT; ePos <= BORDER_BOTTOM; ++ePos)
        delete m_pBorderLines[ePos];
}


OUString lcl_FindUnusedPageStyleName(const uno::Sequence< OUString >& rPageStyleNames)
{
    static const sal_Char cDefaultStyle[] = "Converted";
    //find the hightest number x in each style with the name "cDefaultStyle+x" and
    //return an incremented name
    sal_Int32 nMaxIndex = 0;
    const sal_Int32 nDefaultLength = sizeof(cDefaultStyle)/sizeof(sal_Char) - 1;
    const OUString sDefaultStyle( cDefaultStyle, nDefaultLength, RTL_TEXTENCODING_ASCII_US );

    const OUString* pStyleNames = rPageStyleNames.getConstArray();
    for( sal_Int32 nStyle = 0; nStyle < rPageStyleNames.getLength(); ++nStyle)
    {
        if( pStyleNames[nStyle].getLength() > nDefaultLength &&
                !rtl_ustr_compare_WithLength( sDefaultStyle.getStr(), nDefaultLength, pStyleNames[nStyle].getStr(), nDefaultLength))
        {
            sal_Int32 nIndex = pStyleNames[nStyle].copy( nDefaultLength ).toInt32();
            if( nIndex > nMaxIndex)
                nMaxIndex = nIndex;
        }
    }
    OUString sRet( sDefaultStyle );
    sRet += OUString::valueOf( nMaxIndex + 1);
    return sRet;
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
                m_aFirstPageStyle = uno::Reference< beans::XPropertySet > (
                        xTextFactory->createInstance("com.sun.star.style.PageStyle"),
                        uno::UNO_QUERY);
                if (xPageStyles.is())
                    xPageStyles->insertByName( m_sFirstPageStyleName, uno::makeAny(m_aFirstPageStyle) );
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
                m_aFollowPageStyle = uno::Reference< beans::XPropertySet > (
                        xTextFactory->createInstance("com.sun.star.style.PageStyle"),
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
    catch( const uno::Exception& )
    {
    }

    return xRet;
}


void SectionPropertyMap::SetBorder( BorderPosition ePos, sal_Int32 nLineDistance, const table::BorderLine2& rBorderLine )
{
    delete m_pBorderLines[ePos];
    m_pBorderLines[ePos] = new table::BorderLine2( rBorderLine );
    m_nBorderDistances[ePos] = nLineDistance;
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

    PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
    for( sal_Int32 nBorder = 0; nBorder < 4; ++nBorder)
    {
        if( m_pBorderLines[nBorder] )
        {
            const OUString sBorderName = rPropNameSupplier.GetName( aBorderIds[nBorder] );
            if (xFirst.is())
                xFirst->setPropertyValue( sBorderName, uno::makeAny( *m_pBorderLines[nBorder] ));
            if(xSecond.is())
                xSecond->setPropertyValue( sBorderName, uno::makeAny( *m_pBorderLines[nBorder] ));
        }
        if( m_nBorderDistances[nBorder] >= 0 )
        {
            sal_uInt32 nLineWidth = 0;
            if (m_pBorderLines[nBorder])
                nLineWidth = m_pBorderLines[nBorder]->LineWidth;
            SetBorderDistance( xFirst, aMarginIds[nBorder], aBorderDistanceIds[nBorder],
                  m_nBorderDistances[nBorder], nOffsetFrom, nLineWidth );
            if(xSecond.is())
                SetBorderDistance( xSecond, aMarginIds[nBorder], aBorderDistanceIds[nBorder],
                      m_nBorderDistances[nBorder], nOffsetFrom, nLineWidth );
        }
    }
}

void SectionPropertyMap::SetBorderDistance( uno::Reference< beans::XPropertySet > xStyle,
        PropertyIds eMarginId, PropertyIds eDistId, sal_Int32 nDistance, sal_Int32 nOffsetFrom, sal_uInt32 nLineWidth )
{
    PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();

    sal_Int32 nDist = nDistance;
    if( nOffsetFrom == 1 )
    {
        const OUString sMarginName = rPropNameSupplier.GetName( eMarginId );
        uno::Any aMargin = xStyle->getPropertyValue( sMarginName );
        sal_Int32 nMargin = 0;
        aMargin >>= nMargin;

        // Change the margins with the ( border distance - line width )
        xStyle->setPropertyValue( sMarginName, uno::makeAny( nDistance - nLineWidth ) );

        // Set the distance to ( Margin - distance )
        nDist = nMargin - nDistance;
    }
    const OUString sBorderDistanceName = rPropNameSupplier.GetName( eDistId );
    if (xStyle.is())
        xStyle->setPropertyValue( sBorderDistanceName, uno::makeAny( nDist ));
}



uno::Reference< text::XTextColumns > SectionPropertyMap::ApplyColumnProperties(
                            uno::Reference< beans::XPropertySet > xColumnContainer )
{
    uno::Reference< text::XTextColumns > xColumns;
    try
    {
        PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
        const OUString sTextColumns = rPropNameSupplier.GetName( PROP_TEXT_COLUMNS );
        if (xColumnContainer.is())
            xColumnContainer->getPropertyValue(sTextColumns) >>= xColumns;
        uno::Reference< beans::XPropertySet > xColumnPropSet( xColumns, uno::UNO_QUERY_THROW );
        if( !m_bEvenlySpaced &&
                (sal_Int32(m_aColWidth.size()) == (m_nColumnCount + 1 )) &&
                (sal_Int32(m_aColDistance.size()) == m_nColumnCount))
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
            double fRel = double( nRefValue ) / double( nColSum );
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
            xColumnPropSet->setPropertyValue( rPropNameSupplier.GetName( PROP_AUTOMATIC_DISTANCE ), uno::makeAny( m_nColumnDistance ));
        }

        if(m_bSeparatorLineIsOn)
            xColumnPropSet->setPropertyValue(
                rPropNameSupplier.GetName( PROP_SEPARATOR_LINE_IS_ON ),
                uno::makeAny( m_bSeparatorLineIsOn ));
        xColumnContainer->setPropertyValue( sTextColumns, uno::makeAny( xColumns ) );
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
                    PropertyNameSupplier::GetPropertyNameSupplier().GetName(PROP_HEADER_IS_ON) ) >>= bRet;
        else
            m_aFollowPageStyle->getPropertyValue(
                    PropertyNameSupplier::GetPropertyNameSupplier().GetName(PROP_HEADER_IS_ON) ) >>= bRet;
    }
    return bRet;
}


bool SectionPropertyMap::HasFooter(bool bFirstPage) const
{
    bool bRet = false;
    if( (bFirstPage && m_aFirstPageStyle.is()) ||( !bFirstPage && m_aFollowPageStyle.is()) )
    {
        if( bFirstPage )
            m_aFirstPageStyle->getPropertyValue(
                    PropertyNameSupplier::GetPropertyNameSupplier().GetName(PROP_FOOTER_IS_ON) ) >>= bRet;
        else
            m_aFollowPageStyle->getPropertyValue(
                    PropertyNameSupplier::GetPropertyNameSupplier().GetName(PROP_FOOTER_IS_ON) ) >>= bRet;
    }
    return bRet;
}


#define MIN_HEAD_FOOT_HEIGHT 100 //minimum header/footer height

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

        PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();

        try {
            // Loop over the Header and Footer properties to copy them
            static PropertyIds aProperties[] =
            {
                PROP_HEADER_TEXT,
                PROP_FOOTER_TEXT,
            };

            bool bHasPrevHeader = false;
            bool bHasHeader = false;

            OUString sHeaderIsOn = rPropNameSupplier.GetName( PROP_HEADER_IS_ON );
            if (xPrevStyle.is())
                xPrevStyle->getPropertyValue( sHeaderIsOn ) >>= bHasPrevHeader;
            if (xStyle.is())
                xStyle->getPropertyValue( sHeaderIsOn ) >>= bHasHeader;
            bool bCopyHeader = bHasPrevHeader && !bHasHeader;

            if ( bCopyHeader )
                xStyle->setPropertyValue( sHeaderIsOn, uno::makeAny( sal_True ) );

            bool bHasPrevFooter = false;
            bool bHasFooter = false;

            OUString sFooterIsOn = rPropNameSupplier.GetName( PROP_FOOTER_IS_ON );
            if (xPrevStyle.is())
                xPrevStyle->getPropertyValue( sFooterIsOn ) >>= bHasPrevFooter;
            if (xStyle.is())
                xStyle->getPropertyValue( sFooterIsOn ) >>= bHasFooter;
            bool bCopyFooter = bHasPrevFooter && !bHasFooter;

            if ( bCopyFooter && xStyle.is() )
                xStyle->setPropertyValue( sFooterIsOn, uno::makeAny( sal_True ) );

            // Copying the text properties
            for ( int i = 0, nNbProps = 2; i < nNbProps; i++ )
            {
                bool bIsHeader = ( i < nNbProps / 2 );
                PropertyIds aPropId = aProperties[i];
                OUString sName = rPropNameSupplier.GetName( aPropId );

                if ( ( bIsHeader && bCopyHeader ) || ( !bIsHeader && bCopyFooter ) )
                {
                    SAL_INFO("writerfilter", "Copying " << sName);
                    // TODO has to be copied
                    uno::Reference< text::XTextCopy > xTxt;
                    if (xStyle.is())
                        xTxt.set(xStyle->getPropertyValue( sName ), uno::UNO_QUERY_THROW );

                    uno::Reference< text::XTextCopy > xPrevTxt;
                    if (xPrevStyle.is())
                        xPrevTxt.set(xPrevStyle->getPropertyValue( sName ), uno::UNO_QUERY_THROW );

                    xTxt->copyText( xPrevTxt );
                }
            }
        }
        catch ( const uno::Exception& e )
        {
            SAL_INFO("writerfilter", "An exception occurred in SectionPropertyMap::CopyLastHeaderFooter( ) - " << e.Message);
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
        m_nTopMargin = m_nHeaderTop;
        if( nTopMargin > 0 && nTopMargin > m_nHeaderTop )
            m_nHeaderTop = nTopMargin - m_nHeaderTop;
        else
            m_nHeaderTop = 0;

        //minimum header height 1mm
        if( m_nHeaderTop < MIN_HEAD_FOOT_HEIGHT )
            m_nHeaderTop = MIN_HEAD_FOOT_HEIGHT;
    }


    if( nTopMargin >= 0 ) //fixed height header -> see WW8Par6.hxx
    {
        operator[]( PropertyDefinition( PROP_HEADER_IS_DYNAMIC_HEIGHT, false )) = uno::makeAny( true );
        operator[]( PropertyDefinition( PROP_HEADER_DYNAMIC_SPACING, false )) = uno::makeAny( true );
        operator[]( PropertyDefinition( PROP_HEADER_BODY_DISTANCE, false )) = uno::makeAny( m_nHeaderTop - MIN_HEAD_FOOT_HEIGHT );// ULSpace.Top()
        operator[]( PropertyDefinition( PROP_HEADER_HEIGHT, false )) =  uno::makeAny( m_nHeaderTop );

    }
    else
    {
        //todo: old filter fakes a frame into the header/footer to support overlapping
        //current setting is completely wrong!
        operator[]( PropertyDefinition( PROP_HEADER_HEIGHT, false )) =  uno::makeAny( m_nHeaderTop );
        operator[]( PropertyDefinition( PROP_HEADER_BODY_DISTANCE, false )) = uno::makeAny( nTopMargin - m_nHeaderTop );
        operator[]( PropertyDefinition( PROP_HEADER_IS_DYNAMIC_HEIGHT, false)) = uno::makeAny( false );
        operator[]( PropertyDefinition( PROP_HEADER_DYNAMIC_SPACING, false)) = uno::makeAny( false );
    }

    sal_Int32 nBottomMargin = m_nBottomMargin;
    sal_Int32 nHeaderBottom = m_nHeaderBottom;
    if( HasFooter( bFirstPage ) )
    {
        m_nBottomMargin = m_nHeaderBottom;
        if( nBottomMargin > 0 && nBottomMargin > m_nHeaderBottom )
            m_nHeaderBottom = nBottomMargin - m_nHeaderBottom;
        else
            m_nHeaderBottom = 0;
        if( m_nHeaderBottom < MIN_HEAD_FOOT_HEIGHT )
            m_nHeaderBottom = MIN_HEAD_FOOT_HEIGHT;
    }

    if( nBottomMargin >= 0 ) //fixed height footer -> see WW8Par6.hxx
    {
        operator[]( PropertyDefinition( PROP_FOOTER_IS_DYNAMIC_HEIGHT, false )) = uno::makeAny( true );
        operator[]( PropertyDefinition( PROP_FOOTER_DYNAMIC_SPACING, false )) = uno::makeAny( true );
        operator[]( PropertyDefinition( PROP_FOOTER_BODY_DISTANCE, false )) = uno::makeAny( m_nHeaderBottom - MIN_HEAD_FOOT_HEIGHT);
        operator[]( PropertyDefinition( PROP_FOOTER_HEIGHT, false )) =  uno::makeAny( m_nHeaderBottom );
    }
    else
    {
        //todo: old filter fakes a frame into the header/footer to support overlapping
        //current setting is completely wrong!
        operator[]( PropertyDefinition( PROP_FOOTER_IS_DYNAMIC_HEIGHT, false)) = uno::makeAny( false );
        operator[]( PropertyDefinition( PROP_FOOTER_DYNAMIC_SPACING, false)) = uno::makeAny( false );
        operator[]( PropertyDefinition( PROP_FOOTER_HEIGHT, false )) =  uno::makeAny( nBottomMargin - m_nHeaderBottom );
        operator[]( PropertyDefinition( PROP_FOOTER_BODY_DISTANCE, false )) = uno::makeAny( m_nHeaderBottom );
    }

    //now set the top/bottom margin for the follow page style
    operator[]( PropertyDefinition( PROP_TOP_MARGIN, false )) = uno::makeAny( m_nTopMargin );
    operator[]( PropertyDefinition( PROP_BOTTOM_MARGIN, false )) = uno::makeAny( m_nBottomMargin );

    // Restore original top margin, so we don't end up with a smaller margin in case we have to produce two page styles from one Word section.
    m_nTopMargin = nTopMargin;
    m_nHeaderTop = nHeaderTop;
    m_nHeaderBottom = nHeaderBottom;
}

uno::Reference<beans::XPropertySet> lcl_GetRangeProperties(bool bIsFirstSection, DomainMapper_Impl& rDM_Impl, uno::Reference<text::XTextRange> xStartingRange)
{
    uno::Reference< beans::XPropertySet > xRangeProperties;
    if (bIsFirstSection)
    {
        uno::Reference<container::XEnumerationAccess> xEnumAccess(rDM_Impl.GetBodyText(), uno::UNO_QUERY_THROW);
        uno::Reference<container::XEnumeration> xEnum = xEnumAccess->createEnumeration();
        xRangeProperties = uno::Reference<beans::XPropertySet>(xEnum->nextElement(), uno::UNO_QUERY_THROW);
    }
    else
        xRangeProperties = uno::Reference<beans::XPropertySet>(xStartingRange, uno::UNO_QUERY_THROW);
    return xRangeProperties;
}

void SectionPropertyMap::CloseSectionGroup( DomainMapper_Impl& rDM_Impl )
{
    PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
    if( m_nLnnMod )
    {
        bool bFirst = rDM_Impl.IsLineNumberingSet();
        rDM_Impl.SetLineNumbering( m_nLnnMod, m_nLnc, m_ndxaLnn );
        if( m_nLnnMin > 0 || (bFirst && m_nLnc == 1))
        {
            //set the starting value at the beginning of the section
            try
            {
                uno::Reference< beans::XPropertySet > xRangeProperties;
                if( m_xStartingRange.is() )
                {
                    xRangeProperties = uno::Reference< beans::XPropertySet >( m_xStartingRange, uno::UNO_QUERY_THROW );
                }
                else
                {
                    //set the start value at the beginning of the document
                    xRangeProperties = uno::Reference< beans::XPropertySet >( rDM_Impl.GetTextDocument()->getText()->getStart(), uno::UNO_QUERY_THROW );
                }
                xRangeProperties->setPropertyValue( rPropNameSupplier.GetName( PROP_PARA_LINE_NUMBER_START_VALUE ), uno::makeAny( m_nLnnMin + 1 ));
            }
            catch( const uno::Exception& )
            {
                OSL_FAIL( "Exception in SectionPropertyMap::CloseSectionGroup");
            }
        }
    }

    //depending on the break type no page styles should be created
    if(m_nBreakType == 0)
    {
        //todo: insert a section or access the already inserted section
        uno::Reference< beans::XPropertySet > xSection =
                                    rDM_Impl.appendTextSectionAfter( m_xStartingRange );
        if( m_nColumnCount > 0 && xSection.is())
            ApplyColumnProperties( xSection );
        uno::Reference<beans::XPropertySet> xRangeProperties(lcl_GetRangeProperties(m_bIsFirstSection, rDM_Impl, m_xStartingRange));
        xRangeProperties->setPropertyValue(rPropNameSupplier.GetName(PROP_PAGE_DESC_NAME), uno::makeAny(m_bTitlePage ? m_sFirstPageStyleName : m_sFollowPageStyleName));
    }
    // If the section is of type "New column" (0x01), then simply insert a column break.
    // But only if there actually are columns on the page, otherwise a column break
    // seems to be handled like a page break by MSO.
    else if(m_nBreakType == 1 && m_nColumnCount > 0 )
    {
        uno::Reference< beans::XPropertySet > xRangeProperties;
        if( m_xStartingRange.is() )
        {
            xRangeProperties = uno::Reference< beans::XPropertySet >( m_xStartingRange, uno::UNO_QUERY_THROW );
        }
        else
        {
            //set the start value at the beginning of the document
            xRangeProperties = uno::Reference< beans::XPropertySet >( rDM_Impl.GetTextDocument()->getText()->getStart(), uno::UNO_QUERY_THROW );
        }
        xRangeProperties->setPropertyValue( rPropNameSupplier.GetName( PROP_BREAK_TYPE ),
            uno::makeAny( com::sun::star::style::BreakType_COLUMN_BEFORE));
    }
    else
    {
        //get the properties and create appropriate page styles
        uno::Reference< beans::XPropertySet > xFollowPageStyle = GetPageStyle( rDM_Impl.GetPageStyles(), rDM_Impl.GetTextFactory(), false );

        if( m_nDzaGutter > 0 )
        {
            //todo: iGutterPos from DocProperties are missing
            if( m_bGutterRTL )
                m_nRightMargin += m_nDzaGutter;
            else
                m_nLeftMargin += m_nDzaGutter;
        }
        operator[]( PropertyDefinition( PROP_LEFT_MARGIN, false )) =  uno::makeAny( m_nLeftMargin  );
        operator[]( PropertyDefinition( PROP_RIGHT_MARGIN, false )) = uno::makeAny( m_nRightMargin );

        /*** if headers/footers are available then the top/bottom margins of the
            header/footer are copied to the top/bottom margin of the page
          */
        CopyLastHeaderFooter( false, rDM_Impl );
        PrepareHeaderFooterProperties( false );

        const OUString sTrayIndex = rPropNameSupplier.GetName( PROP_PRINTER_PAPER_TRAY_INDEX );
        if( m_nPaperBin >= 0 )
            xFollowPageStyle->setPropertyValue( sTrayIndex, uno::makeAny( m_nPaperBin ) );
        uno::Reference< text::XTextColumns > xColumns;
        if( m_nColumnCount > 0 )
            xColumns = ApplyColumnProperties( xFollowPageStyle );

        //prepare text grid properties
        sal_Int32 nHeight = 1;
        PropertyMap::iterator aElement = find(PropertyDefinition( PROP_HEIGHT, false ));
        if( aElement != end())
            aElement->second >>= nHeight;

        sal_Int32 nWidth = 1;
        aElement = find(PropertyDefinition( PROP_WIDTH, false ));
        if( aElement != end())
            aElement->second >>= nWidth;

        text::WritingMode eWritingMode = text::WritingMode_LR_TB;
        aElement = find(PropertyDefinition( PROP_WRITING_MODE, false ));
        if( aElement != end())
            aElement->second >>= eWritingMode;

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

        operator[]( PropertyDefinition( PROP_GRID_LINES, false )) =
                uno::makeAny( static_cast<sal_Int16>(nTextAreaHeight/nGridLinePitch));

        sal_Int32 nCharWidth = 423; //240 twip/ 12 pt
        //todo: is '0' the right index here?
        const StyleSheetEntryPtr pEntry = rDM_Impl.GetStyleSheetTable()->FindStyleSheetByISTD(OUString::valueOf(static_cast<sal_Int32>(0), 16));
        if( pEntry.get( ) )
        {
            PropertyMap::iterator aElement_ = pEntry->pProperties->find(PropertyDefinition( PROP_CHAR_HEIGHT_ASIAN, false ));
            if( aElement_ != pEntry->pProperties->end())
            {
                double fHeight = 0;
                if( aElement_->second >>= fHeight )
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
        operator[]( PropertyDefinition( PROP_GRID_BASE_HEIGHT, false )) = uno::makeAny( nCharWidth );
        sal_Int32 nRubyHeight = nGridLinePitch - nCharWidth;
        if(nRubyHeight < 0 )
            nRubyHeight = 0;
        operator[]( PropertyDefinition( PROP_GRID_RUBY_HEIGHT, false )) = uno::makeAny( nRubyHeight );

        sal_Int16 nGridMode = text::TextGridMode::NONE;

        switch (m_nGridType)
        {
            case NS_ooxml::LN_Value_wordprocessingml_ST_DocGrid_lines:
                nGridMode = text::TextGridMode::LINES;
                break;
            case NS_ooxml::LN_Value_wordprocessingml_ST_DocGrid_linesAndChars:
                nGridMode = text::TextGridMode::LINES_AND_CHARS;
                break;
            default:
                break;
        }

        operator[](PropertyDefinition(PROP_GRID_MODE, false)) = uno::makeAny(nGridMode);

        if (rDM_Impl.IsNewDoc())
            _ApplyProperties( xFollowPageStyle );

        //todo: creating a "First Page" style depends on HasTitlePage und _fFacingPage_
        if( m_bTitlePage )
        {
            CopyLastHeaderFooter( true, rDM_Impl );
            PrepareHeaderFooterProperties( true );
            uno::Reference< beans::XPropertySet > xFirstPageStyle = GetPageStyle(
                                rDM_Impl.GetPageStyles(), rDM_Impl.GetTextFactory(), true );
            if (rDM_Impl.IsNewDoc())
                _ApplyProperties( xFirstPageStyle );

            sal_Int32 nPaperBin = m_nFirstPaperBin >= 0 ? m_nFirstPaperBin : m_nPaperBin >= 0 ? m_nPaperBin : 0;
            if( nPaperBin )
                xFollowPageStyle->setPropertyValue( sTrayIndex, uno::makeAny( nPaperBin ) );
            if( xColumns.is() )
                xFollowPageStyle->setPropertyValue(
                    rPropNameSupplier.GetName( PROP_TEXT_COLUMNS ), uno::makeAny( xColumns ));
        }

        ApplyBorderToPageStyles( rDM_Impl.GetPageStyles( ), rDM_Impl.GetTextFactory( ), m_nBorderParams );

        try
        {
            {
                //now apply this break at the first paragraph of this section
                uno::Reference<beans::XPropertySet> xRangeProperties(lcl_GetRangeProperties(m_bIsFirstSection, rDM_Impl, m_xStartingRange));
            /* break type
            0 - No break 1 - New Colunn 2 - New page 3 - Even page 4 - odd page */
                if ((m_bTitlePage && m_bIsFirstSection) || !m_bTitlePage)
                    xRangeProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_PAGE_DESC_NAME ),
                            uno::makeAny( m_bTitlePage ? m_sFirstPageStyleName : m_sFollowPageStyleName ));
                else
                {
                    // In this miserable situation (second or later section on a title page), make sure that the header / footer is not lost.
                    uno::Reference< container::XNameAccess > xPageStyles(rDM_Impl.GetPageStyles(), uno::UNO_QUERY);
                    if (xPageStyles->hasByName(m_sFollowPageStyleName))
                    {
                        uno::Reference<beans::XPropertySet> xCurrent(xPageStyles->getByName(rPropNameSupplier.GetName(PROP_STANDARD)), uno::UNO_QUERY);
                        uno::Reference<beans::XPropertySet> xFollow(xPageStyles->getByName(m_sFollowPageStyleName), uno::UNO_QUERY);

                        if (xFollow->getPropertyValue(rPropNameSupplier.GetName(PROP_HEADER_IS_ON)).get<sal_Bool>())
                        {
                            xCurrent->setPropertyValue(rPropNameSupplier.GetName(PROP_HEADER_IS_ON), uno::makeAny(sal_True));
                            uno::Reference<text::XTextRange> xCurrentRange(xCurrent->getPropertyValue(rPropNameSupplier.GetName(PROP_HEADER_TEXT)), uno::UNO_QUERY_THROW);
                            xCurrentRange->setString("");
                            uno::Reference<text::XTextCopy> xCurrentTxt(xCurrentRange, uno::UNO_QUERY_THROW);
                            uno::Reference<text::XTextCopy> xFollowTxt(xFollow->getPropertyValue(rPropNameSupplier.GetName(PROP_HEADER_TEXT)), uno::UNO_QUERY_THROW);
                            xCurrentTxt->copyText(xFollowTxt);
                        }
                        if (xFollow->getPropertyValue(rPropNameSupplier.GetName(PROP_FOOTER_IS_ON)).get<sal_Bool>())
                        {
                            xCurrent->setPropertyValue(rPropNameSupplier.GetName(PROP_FOOTER_IS_ON), uno::makeAny(sal_True));
                            uno::Reference<text::XTextRange> xCurrentRange(xCurrent->getPropertyValue(rPropNameSupplier.GetName(PROP_FOOTER_TEXT)), uno::UNO_QUERY_THROW);
                            xCurrentRange->setString("");
                            uno::Reference<text::XTextCopy> xCurrentTxt(xCurrentRange, uno::UNO_QUERY_THROW);
                            uno::Reference<text::XTextCopy> xFollowTxt(xFollow->getPropertyValue(rPropNameSupplier.GetName(PROP_FOOTER_TEXT)), uno::UNO_QUERY_THROW);
                            xCurrentTxt->copyText(xFollowTxt);
                        }
                    }
                }
                // handle page breaks with odd/even page numbering
                style::PageStyleLayout nPageStyleLayout(style::PageStyleLayout_ALL);
                if (m_nBreakType == 3)
                    nPageStyleLayout = style::PageStyleLayout_LEFT;
                else if (m_nBreakType == 4)
                    nPageStyleLayout = style::PageStyleLayout_RIGHT;
                if (nPageStyleLayout)
                    xFollowPageStyle->setPropertyValue(rPropNameSupplier.GetName(PROP_PAGE_STYLE_LAYOUT), uno::makeAny(nPageStyleLayout));
                if(m_bPageNoRestart || m_nPageNumber >= 0)
                {
                    sal_Int16 nPageNumber = m_nPageNumber >= 0 ? static_cast< sal_Int16 >(m_nPageNumber) : 1;
                    xRangeProperties->setPropertyValue(rPropNameSupplier.GetName( PROP_PAGE_NUMBER_OFFSET ),
                        uno::makeAny( nPageNumber ));
                }
            }
        }
        catch (const uno::Exception&)
        {
            OSL_FAIL( "Exception in SectionPropertyMap::CloseSectionGroup");
        }
    }
    rDM_Impl.SetIsFirstParagraphInSection(true);
}


void SectionPropertyMap::_ApplyProperties( uno::Reference< beans::XPropertySet > xStyle )
{
    PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
    uno::Reference<beans::XMultiPropertySet> const xMultiSet(xStyle,
            uno::UNO_QUERY);
    if (xMultiSet.is())
    {   // FIXME why is "this" a STL container???
        uno::Sequence<OUString> names(this->size());
        uno::Sequence<uno::Any> values(this->size());
        PropertyMap::iterator it = this->begin();
        for (size_t i = 0; it != this->end(); ++it, ++i)
        {
            names[i] = rPropNameSupplier.GetName(it->first.eId);
            values[i] = it->second;
        }
        try
        {
            xMultiSet->setPropertyValues(names, values);
        }
        catch( const uno::Exception& )
        {
            OSL_FAIL( "Exception in <PageStyle>::setPropertyValue");
        }
        return;
    }
    PropertyMap::iterator aMapIter = begin();
    while( aMapIter != end())
    {
        try
        {
            if (xStyle.is())
                xStyle->setPropertyValue( rPropNameSupplier.GetName( aMapIter->first.eId ), aMapIter->second );
        }
        catch( const uno::Exception& )
        {
            OSL_FAIL( "Exception in <PageStyle>::setPropertyValue");
        }
        ++aMapIter;
    }
}
sal_Int32 lcl_AlignPaperBin( sal_Int32 nSet )
{
    //default tray numbers are above 0xff
    if( nSet > 0xff )
        nSet = nSet >> 8;
    //there are some special numbers which can't be handled easily
    //1, 4, 15, manual tray, upper tray, auto select? see ww8atr.cxx
    //todo: find out appropriate conversion
    return nSet;
}


void SectionPropertyMap::SetPaperBin( sal_Int32 nSet )
{
    m_nPaperBin = lcl_AlignPaperBin( nSet );
}


void SectionPropertyMap::SetFirstPaperBin( sal_Int32 nSet )
{
    m_nFirstPaperBin = lcl_AlignPaperBin( nSet );
}


StyleSheetPropertyMap::StyleSheetPropertyMap() :
    mnCT_Spacing_line( 0 ),
    mnCT_Spacing_lineRule( 0 ),
    mbCT_TrPrBase_tblHeader( false ),
    mnCT_TrPrBase_jc( 0 ),
    mnCT_TcPrBase_vAlign( 0 ),
    mnCT_TblWidth_w( 0 ),
    mnCT_TblWidth_type( 0 ),
    mbCT_Spacing_lineSet( false ),
    mbCT_Spacing_lineRuleSet( false ),
    mbCT_TrPrBase_tblHeaderSet( false ),
    mbCT_TrPrBase_jcSet( false ),
    mbCT_TcPrBase_vAlignSet( false ),
    mbCT_TblWidth_wSet( false ),
    mbCT_TblWidth_typeSet( false ),
    mnListId( -1 ),
    mnListLevel( -1 ),
    mnOutlineLevel( -1 )
{
}


StyleSheetPropertyMap::~StyleSheetPropertyMap()
{
}


ParagraphProperties::ParagraphProperties() :
    m_bFrameMode( false ),
    m_nDropCap(NS_ooxml::LN_Value_wordprocessingml_ST_DropCap_none),
    m_nLines(0),
    m_w(-1),
    m_h(-1),
    m_nWrap(-1),
    m_hAnchor(-1),
    m_vAnchor(text::RelOrientation::FRAME),
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


int ParagraphProperties::operator==(const ParagraphProperties& rCompare)
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
    m_nDropCap = NS_ooxml::LN_Value_wordprocessingml_ST_DropCap_none;
    m_nLines = 0;
    m_w = -1;
    m_h = -1;
    m_nWrap = -1;
    m_hAnchor = -1;
    m_vAnchor = text::RelOrientation::FRAME;
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
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("TablePropertyMap.insertTableProperties");
    pMap->dumpXml(dmapper_logger);
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
#ifdef DEBUG_DOMAINMAPPER
    dumpXml( dmapper_logger );
    dmapper_logger->endElement();
#endif
}


}//namespace dmapper
}//namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
