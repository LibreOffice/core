/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
m_nLineWidth(0),
m_nLineType(0),
m_nLineColor(0),
m_nLineDistance(0),
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
            m_nLineWidth = ConversionHelper::convertTwipToMM100( nIntValue * 5 / 2 );
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
            //if 1 then line has shadow - unsupported
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
    switch( rSprm.getId())
    {
        case NS_ooxml::LN_CT_TblBorders_top:
        case NS_ooxml::LN_CT_TblBorders_left:
        case NS_ooxml::LN_CT_TblBorders_bottom:
        case NS_ooxml::LN_CT_TblBorders_right:
        case NS_ooxml::LN_CT_TblBorders_insideH:
        case NS_ooxml::LN_CT_TblBorders_insideV:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties.get())
                pProperties->resolve(*this);
            ConversionHelper::MakeBorderLine( m_nLineWidth,   m_nLineType, m_nLineColor,
                                   m_aBorderLines[rSprm.getId() - NS_ooxml::LN_CT_TblBorders_top], m_bOOXML );

            m_aFilledLines[ rSprm.getId( ) - NS_ooxml::LN_CT_TblBorders_top] = true;
        }
        break;
        default:;
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

} //namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
