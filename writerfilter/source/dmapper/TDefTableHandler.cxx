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
#include <TDefTableHandler.hxx>
#include <PropertyMap.hxx>
#include <ConversionHelper.hxx>
#include <ooxml/resourceids.hxx>
#include <doctok/resourceids.hxx>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/VertOrientation.hpp>

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;

/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
TDefTableHandler::TDefTableHandler(bool bOOXML) :
    m_nLineWidth(0),
    m_nLineType(0),
    m_nLineColor(0),
    m_nLineDistance(0),
    m_bOOXML( bOOXML )
{
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
TDefTableHandler::~TDefTableHandler()
{
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void TDefTableHandler::attribute(Id rName, Value & rVal)
{
    sal_Int32 nIntValue = rVal.getInt();
    (void)nIntValue;
    (void)rName;
    /* WRITERFILTERSTATUS: table: TDefTable_attributedata */
    switch( rName )
    {
        /* WRITERFILTERSTATUS: done: 50, planned: 0, spent: 0 */
        case NS_rtf::LN_cellx:
            // nIntValue contains the vert. line position
            //TODO: nIntValue is wrong for negative values!
            if( nIntValue > 0x7fff )
                nIntValue -= 0xffff;
            m_aCellBorderPositions.push_back( ConversionHelper::convertTwipToMM100( nIntValue ) );
        break;
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
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
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FFIRSTMERGED: 
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FMERGED:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FVERTICAL:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FBACKWARD:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FROTATEFONT:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FVERTMERGE:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FVERTRESTART:
        break;
        /* WRITERFILTERSTATUS: done: 50, planned: 0, spent: 0 */
        case NS_rtf::LN_VERTALIGN:
            //TODO: m_aCellVertAlign is just a temporary solution! 0 - top 1 - center 2 - bottom
            m_aCellVertAlign.push_back( nIntValue );
        break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FUNUSED:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_CellPrefferedSize:
        break;
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_rtf::LN_BRCTOP:
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_rtf::LN_BRCLEFT:
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_rtf::LN_BRCBOTTOM:
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_rtf::LN_BRCRIGHT:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rVal.getProperties();
            localResolve( rName, pProperties );
        }
        break;
        //from LN_BRCXXXX - handled within the BorderHandler
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_rtf::LN_DPTLINEWIDTH: // 0x2871
            //  width of a single line in 1/8 pt, max of 32 pt -> twip * 5 / 2.
            m_nLineWidth = ConversionHelper::convertTwipToMM100( nIntValue * 5 / 2 );
        break;
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_rtf::LN_BRCTYPE:    // 0x2872
            m_nLineType = nIntValue;
        break;
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Border_color:
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_rtf::LN_ICO:        // 0x2873
            m_nLineColor = nIntValue;
        break;
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_rtf::LN_DPTSPACE:   // 0x2874
            m_nLineDistance = nIntValue;
        break;
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FSHADOW:    // 0x2875
            //if 1 then line has shadow - unsupported
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_FFRAME:     // 0x2876
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_UNUSED2_15: // 0x2877
            // ignored
        break;
        case NS_ooxml::LN_CT_Border_themeColor:
        case NS_ooxml::LN_CT_Border_themeTint:
        case NS_ooxml::LN_CT_Border_themeShade:
            // ignored
        break;
        default:
            OSL_ENSURE(false, "unknown attribute");
    }
}
/*-- 08.10.2007 12:55:32---------------------------------------------------

  -----------------------------------------------------------------------*/
void TDefTableHandler::localResolve(Id rName, writerfilter::Reference<Properties>::Pointer_t pProperties)
{
    if( pProperties.get())
    {
        m_nLineWidth = m_nLineType = m_nLineColor = m_nLineDistance = 0;
        pProperties->resolve( *this );
        table::BorderLine aBorderLine;
        ConversionHelper::MakeBorderLine( m_nLineWidth,   m_nLineType, m_nLineColor,  
                                                                        aBorderLine, m_bOOXML );
                
        /* WRITERFILTERSTATUS: table: TDefTable_localresolve */
        switch( rName )
        {
            /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
            case NS_ooxml::LN_CT_TcBorders_top:
            /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
            case NS_rtf::LN_BRCTOP:
                m_aTopBorderLines.push_back(aBorderLine);
            break;
            /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
            case NS_ooxml::LN_CT_TcBorders_left:
            /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
            case NS_rtf::LN_BRCLEFT:
                m_aLeftBorderLines.push_back(aBorderLine);
            break;
            /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
            case NS_ooxml::LN_CT_TcBorders_bottom: 
            /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
            case NS_rtf::LN_BRCBOTTOM:
                m_aBottomBorderLines.push_back(aBorderLine);
            break;
            /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
            case NS_ooxml::LN_CT_TcBorders_right:
            /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
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
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void TDefTableHandler::sprm(Sprm & rSprm)
{
    /* WRITERFILTERSTATUS: table: TDefTable_sprm */
    switch( rSprm.getId() )
    {
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_TcBorders_top:
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_TcBorders_left:
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_TcBorders_bottom: 
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_TcBorders_right:
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
/*-- 24.04.2007 09:09:01---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 10.05.2007 16:10:33---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 09.05.2007 13:14:17---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 10.05.2007 16:09:10---------------------------------------------------
            
  -----------------------------------------------------------------------*/
size_t TDefTableHandler::getCellCount() const
{
    return m_aCellVertAlign.size();
}

} //namespace dmapper
} //namespace writerfilter
