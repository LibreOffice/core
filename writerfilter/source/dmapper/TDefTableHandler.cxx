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
#include <TDefTableHandler.hxx>
#include <PropertyMap.hxx>
#include <ConversionHelper.hxx>
#include <ooxml/resourceids.hxx>
#include <doctok/resourceids.hxx>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/VertOrientation.hpp>

#include "dmapperLoggers.hxx"

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;



TDefTableHandler::TDefTableHandler(bool bOOXML) :
LoggedProperties(dmapper_logger, "TDefTableHandler"),
m_nLineWidth(0),
m_nLineType(0),
m_nLineColor(0),
m_nLineDistance(0),
m_bOOXML( bOOXML )
{
}


TDefTableHandler::~TDefTableHandler()
{
}


void TDefTableHandler::lcl_attribute(Id rName, Value & rVal)
{
    sal_Int32 nIntValue = rVal.getInt();
    (void)nIntValue;
    (void)rName;
    switch( rName )
    {
        case NS_rtf::LN_cellx:
            // nIntValue contains the vert. line position
            //TODO: nIntValue is wrong for negative values!
            if( nIntValue > 0x7fff )
                nIntValue -= 0xffff;
            m_aCellBorderPositions.push_back( ConversionHelper::convertTwipToMM100( nIntValue ) );
        break;
        case NS_rtf::LN_tc:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rVal.getProperties();
            if( pProperties.get())
            {
                pProperties->resolve( *this );
            }
        }
        break;
        //from LN_tc
        case NS_rtf::LN_FFIRSTMERGED:
        case NS_rtf::LN_FMERGED:
        case NS_rtf::LN_FVERTICAL:
        case NS_rtf::LN_FBACKWARD:
        case NS_rtf::LN_FROTATEFONT:
        case NS_rtf::LN_FVERTMERGE:
        case NS_rtf::LN_FVERTRESTART:
        break;
        case NS_rtf::LN_VERTALIGN:
            //TODO: m_aCellVertAlign is just a temporary solution! 0 - top 1 - center 2 - bottom
            m_aCellVertAlign.push_back( nIntValue );
        break;
        case NS_rtf::LN_FUNUSED:
        case NS_rtf::LN_CellPrefferedSize:
        break;
        case NS_rtf::LN_BRCTOP:
        case NS_rtf::LN_BRCLEFT:
        case NS_rtf::LN_BRCBOTTOM:
        case NS_rtf::LN_BRCRIGHT:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rVal.getProperties();
            localResolve( rName, pProperties );
        }
        break;
        //from LN_BRCXXXX - handled within the BorderHandler
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
        case NS_rtf::LN_DPTSPACE:   // 0x2874
            m_nLineDistance = nIntValue;
        break;
        case NS_rtf::LN_FSHADOW:    // 0x2875
            //if 1 then line has shadow - unsupported
        case NS_rtf::LN_FFRAME:     // 0x2876
        case NS_rtf::LN_UNUSED2_15: // 0x2877
            // ignored
        break;
        case NS_ooxml::LN_CT_Border_themeColor:
        case NS_ooxml::LN_CT_Border_themeTint:
        case NS_ooxml::LN_CT_Border_themeShade:
            // ignored
        break;
        default:
            OSL_FAIL("unknown attribute");
    }
}


void TDefTableHandler::localResolve(Id rName, writerfilter::Reference<Properties>::Pointer_t pProperties)
{
    if( pProperties.get())
    {
        m_nLineWidth = m_nLineType = m_nLineColor = m_nLineDistance = 0;
        pProperties->resolve( *this );
        table::BorderLine2 aBorderLine;
        ConversionHelper::MakeBorderLine( m_nLineWidth,   m_nLineType, m_nLineColor,
                                                                        aBorderLine, m_bOOXML );
        bool rtl = false; // TODO
        switch( rName )
        {
            case NS_ooxml::LN_CT_TcBorders_top:
            case NS_rtf::LN_BRCTOP:
                m_aTopBorderLines.push_back(aBorderLine);
            break;
            case NS_ooxml::LN_CT_TcBorders_start:
                if( rtl )
                    m_aRightBorderLines.push_back(aBorderLine);
                else
                    m_aLeftBorderLines.push_back(aBorderLine);
            break;
            case NS_ooxml::LN_CT_TcBorders_left:
            case NS_rtf::LN_BRCLEFT:
                m_aLeftBorderLines.push_back(aBorderLine);
            break;
            case NS_ooxml::LN_CT_TcBorders_bottom:
            case NS_rtf::LN_BRCBOTTOM:
                m_aBottomBorderLines.push_back(aBorderLine);
            break;
            case NS_ooxml::LN_CT_TcBorders_end:
                if( rtl )
                    m_aLeftBorderLines.push_back(aBorderLine);
                else
                    m_aRightBorderLines.push_back(aBorderLine);
            break;
            case NS_ooxml::LN_CT_TcBorders_right:
            case NS_rtf::LN_BRCRIGHT:
                m_aRightBorderLines.push_back(aBorderLine);
            break;
            case NS_ooxml::LN_CT_TcBorders_insideH:
                m_aInsideHBorderLines.push_back(aBorderLine);
            break;
            case NS_ooxml::LN_CT_TcBorders_insideV:
                m_aInsideVBorderLines.push_back(aBorderLine);
            break;
            default:;
        }
    }
}


void TDefTableHandler::lcl_sprm(Sprm & rSprm)
{
    switch( rSprm.getId() )
    {
        case NS_ooxml::LN_CT_TcBorders_top:
        case NS_ooxml::LN_CT_TcBorders_left:
        case NS_ooxml::LN_CT_TcBorders_start:
        case NS_ooxml::LN_CT_TcBorders_bottom:
        case NS_ooxml::LN_CT_TcBorders_right:
        case NS_ooxml::LN_CT_TcBorders_end:
        case NS_ooxml::LN_CT_TcBorders_insideH:
        case NS_ooxml::LN_CT_TcBorders_insideV:
        case NS_ooxml::LN_CT_TcBorders_tl2br:
        case NS_ooxml::LN_CT_TcBorders_tr2bl:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            localResolve( rSprm.getId(), pProperties );
        }
        break;
        default:;
    }
}


PropertyMapPtr  TDefTableHandler::getRowProperties() const
{
    PropertyMapPtr pPropertyMap(new PropertyMap);

    // Writer only wants the separators, Word provides also the outer border positions
    if( m_aCellBorderPositions.size() > 2 )
    {
        //determine table width
        double nFullWidth = m_aCellBorderPositions[m_aCellBorderPositions.size() - 1] - m_aCellBorderPositions[0];
        //the positions have to be distibuted in a range of 10000
        const double nFullWidthRelative = 10000.;
        uno::Sequence< text::TableColumnSeparator > aSeparators( m_aCellBorderPositions.size() - 2 );
        text::TableColumnSeparator* pSeparators = aSeparators.getArray();
        for( sal_uInt32 nBorder = 1; nBorder < m_aCellBorderPositions.size() - 1; ++nBorder )
        {
            sal_Int16 nRelPos =
                sal::static_int_cast< sal_Int16 >(double(m_aCellBorderPositions[nBorder]) * nFullWidthRelative / nFullWidth );

            pSeparators[nBorder - 1].Position =  nRelPos;
            pSeparators[nBorder - 1].IsVisible = sal_True;
        }
        pPropertyMap->Insert( PROP_TABLE_COLUMN_SEPARATORS, false, uno::makeAny( aSeparators ) );
    }

    return pPropertyMap;
}


void TDefTableHandler::fillCellProperties(
            size_t nCell, ::boost::shared_ptr< TablePropertyMap > pCellProperties ) const
{
    if( m_aCellBorderPositions.size() > nCell )
    {
        sal_Int16 nVertOrient = text::VertOrientation::NONE;
        switch( m_aCellVertAlign[nCell] ) //0 - top 1 - center 2 - bottom
        {
            case 1: nVertOrient = text::VertOrientation::CENTER; break;
            case 2: nVertOrient = text::VertOrientation::BOTTOM; break;
            default:;
        }
        pCellProperties->Insert( PROP_VERT_ORIENT, false, uno::makeAny( nVertOrient ) );
    }
    if( m_aTopBorderLines.size() > nCell )
        pCellProperties->Insert( PROP_TOP_BORDER, false, uno::makeAny( m_aTopBorderLines[nCell] ) );
    if( m_aLeftBorderLines.size() > nCell )
        pCellProperties->Insert( PROP_LEFT_BORDER, false, uno::makeAny( m_aLeftBorderLines[nCell] ) );
    if( m_aBottomBorderLines.size() > nCell )
        pCellProperties->Insert( PROP_BOTTOM_BORDER, false, uno::makeAny( m_aBottomBorderLines[nCell] ) );
    if( m_aRightBorderLines.size() > nCell )
        pCellProperties->Insert( PROP_RIGHT_BORDER, false, uno::makeAny( m_aRightBorderLines[nCell] ) );
    if( m_aInsideHBorderLines.size() > nCell )
        pCellProperties->Insert( META_PROP_HORIZONTAL_BORDER, false, uno::makeAny( m_aInsideHBorderLines[nCell] ) );
    if( m_aInsideVBorderLines.size() > nCell )
        pCellProperties->Insert( META_PROP_VERTICAL_BORDER, false, uno::makeAny( m_aInsideVBorderLines[nCell] ) );
}


sal_Int32 TDefTableHandler::getTableWidth() const
{
    sal_Int32 nWidth = 0;
    if( m_aCellBorderPositions.size() > 1 )
    {
        //determine table width
        nWidth = m_aCellBorderPositions[m_aCellBorderPositions.size() - 1] - m_aCellBorderPositions[0];
    }
    return nWidth;
}


size_t TDefTableHandler::getCellCount() const
{
    return m_aCellVertAlign.size();
}

} //namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
