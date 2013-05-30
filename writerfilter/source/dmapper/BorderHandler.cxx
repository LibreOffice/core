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
#include <BorderHandler.hxx>
#include <PropertyMap.hxx>
#include <resourcemodel/QNameToString.hxx>
#include <doctok/resourceids.hxx>
#include <ConversionHelper.hxx>
#include <com/sun/star/table/BorderLine2.hpp>
#include <ooxml/resourceids.hxx>
#include <dmapperLoggers.hxx>

namespace writerfilter {

namespace dmapper {

using namespace ::com::sun::star;


BorderHandler::BorderHandler( bool bOOXML ) :
LoggedProperties(dmapper_logger, "BorderHandler"),
m_nCurrentBorderPosition( BORDER_TOP ),
m_nLineWidth(15), // Word default, in twips
m_nLineType(0),
m_nLineColor(0),
m_nLineDistance(0),
m_bShadow(false),
m_bOOXML( bOOXML )
{
    const int nBorderCount(BORDER_COUNT);
    std::fill_n(m_aFilledLines, nBorderCount, false);
    std::fill_n(m_aBorderLines, nBorderCount, table::BorderLine2());
}

BorderHandler::~BorderHandler()
{
}

void BorderHandler::lcl_attribute(Id rName, Value & rVal)
{
    sal_Int32 nIntValue = rVal.getInt();
    switch( rName )
    {
        case NS_rtf::LN_rgbrc:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rVal.getProperties();
            if( pProperties.get())
            {
                pProperties->resolve(*this);
                ConversionHelper::MakeBorderLine( m_nLineWidth,   m_nLineType, m_nLineColor,
                                                                                m_aBorderLines[m_nCurrentBorderPosition], m_bOOXML );
                OSL_ENSURE(m_nCurrentBorderPosition < BORDER_COUNT, "too many border values");
                ++m_nCurrentBorderPosition;
            }
        }
        break;
        case NS_rtf::LN_DPTLINEWIDTH: // 0x2871
            //  width of a single line in 1/8 pt, max of 32 pt -> twip * 5 / 2.
            m_nLineWidth = nIntValue * 5 / 2;
        break;
        case NS_rtf::LN_BRCTYPE:    // 0x2872
            m_nLineType = nIntValue;
        break;
        case NS_ooxml::LN_CT_Border_color:
        case NS_rtf::LN_ICO:        // 0x2873
            m_nLineColor = nIntValue;
        break;
        case NS_rtf::LN_DPTSPACE:   // border distance in points
            m_nLineDistance = ConversionHelper::convertTwipToMM100( nIntValue * 20 );
        break;
        case NS_rtf::LN_FSHADOW:    // 0x2875
            m_bShadow = nIntValue;
        break;
        case NS_rtf::LN_FFRAME:     // 0x2876
        case NS_rtf::LN_UNUSED2_15: // 0x2877
            // ignored
        break;
        case NS_ooxml::LN_CT_Border_themeTint: break;
        case NS_ooxml::LN_CT_Border_themeColor: break;
        default:
            OSL_FAIL( "unknown attribute");
    }
}

void BorderHandler::lcl_sprm(Sprm & rSprm)
{
    BorderPosition pos = BORDER_COUNT; // invalid pos
    bool rtl = false; // TODO detect
    switch( rSprm.getId())
    {
        case NS_ooxml::LN_CT_TblBorders_top:
            pos = BORDER_TOP;
            break;
        case NS_ooxml::LN_CT_TblBorders_start:
            pos = rtl ? BORDER_RIGHT : BORDER_LEFT;
            break;
        case NS_ooxml::LN_CT_TblBorders_left:
            pos = BORDER_LEFT;
            break;
        case NS_ooxml::LN_CT_TblBorders_bottom:
            pos = BORDER_BOTTOM;
            break;
        case NS_ooxml::LN_CT_TblBorders_end:
            pos = rtl ? BORDER_LEFT : BORDER_RIGHT;
            break;
        case NS_ooxml::LN_CT_TblBorders_right:
            pos = BORDER_RIGHT;
            break;
        case NS_ooxml::LN_CT_TblBorders_insideH:
            pos = BORDER_HORIZONTAL;
            break;
        case NS_ooxml::LN_CT_TblBorders_insideV:
            pos = BORDER_VERTICAL;
            break;
        default:
            break;
    }
    if( pos != BORDER_COUNT )
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if( pProperties.get())
            pProperties->resolve(*this);
        ConversionHelper::MakeBorderLine( m_nLineWidth,   m_nLineType, m_nLineColor,
                               m_aBorderLines[ pos ], m_bOOXML );
        m_aFilledLines[ pos ] = true;
    }
}

PropertyMapPtr  BorderHandler::getProperties()
{
    static const PropertyIds aPropNames[BORDER_COUNT] =
    {
        PROP_TOP_BORDER,
        PROP_LEFT_BORDER,
        PROP_BOTTOM_BORDER,
        PROP_RIGHT_BORDER,
        META_PROP_HORIZONTAL_BORDER,
        META_PROP_VERTICAL_BORDER
    };
    PropertyMapPtr pPropertyMap(new PropertyMap);
    // don't fill in default properties
    if( m_bOOXML || m_nCurrentBorderPosition )
    {
        for( sal_Int32 nProp = 0; nProp < BORDER_COUNT; ++nProp)
        {
            if ( m_aFilledLines[nProp] ) {
                pPropertyMap->Insert( aPropNames[nProp], false, uno::makeAny( m_aBorderLines[nProp] ) );
            }
        }
    }
    return pPropertyMap;
}
/*-------------------------------------------------------------------------
    used only in OOXML import
  -----------------------------------------------------------------------*/
table::BorderLine2 BorderHandler::getBorderLine()
{
    table::BorderLine2 aBorderLine;
    ConversionHelper::MakeBorderLine( m_nLineWidth, m_nLineType, m_nLineColor, aBorderLine, m_bOOXML );
    return aBorderLine;
}

bool BorderHandler::getShadow()
{
    return m_bShadow;
}

} //namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
