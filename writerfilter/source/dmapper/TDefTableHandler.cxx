/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TDefTableHandler.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2007-05-24 11:32:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef INCLUDED_TDEFTABLEHANDLER_HXX
#include <TDefTableHandler.hxx>
#endif
#ifndef INCLUDED_DMAPPER_PROPERTYMAP_HXX
#include <PropertyMap.hxx>
#endif
#ifndef INCLUDED_DMAPPER_CONVERSIONHELPER_HXX
#include <ConversionHelper.hxx>
#endif
#ifndef INCLUDED_QNAME_HXX
#include <odiapi/qname.hxx>
#endif
#ifndef INCLUDED_RESOURCESIDS
#include <doctok/resourceids.hxx>
#endif
#ifndef _COM_SUN_STAR_TABLE_BORDERLINE_HPP_
#include <com/sun/star/table/BorderLine.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TABLECOLUMNSEPARATOR_HPP_
#include <com/sun/star/text/TableColumnSeparator.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_VERTORIENTATION_HDL_
#include <com/sun/star/text/VertOrientation.hpp>
#endif

namespace dmapper {

using namespace ::com::sun::star;
using namespace ::writerfilter;

/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
TDefTableHandler::TDefTableHandler() :
    m_nLineWidth(0),
    m_nLineType(0),
    m_nLineColor(0),
    m_nLineDistance(0)
{
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
TDefTableHandler::~TDefTableHandler()
{
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void TDefTableHandler::attribute(doctok::Id rName, doctok::Value & rVal)
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
            m_aCellBorderPositions.push_back( ConversionHelper::convertToMM100( nIntValue ) );
        break;
        case NS_rtf::LN_tc:
        {
            doctok::Reference<doctok::Properties>::Pointer_t pProperties = rVal.getProperties();
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
            doctok::Reference<doctok::Properties>::Pointer_t pProperties = rVal.getProperties();
            if( pProperties.get())
            {
                m_nLineWidth = m_nLineType = m_nLineColor = m_nLineDistance = 0;
                pProperties->resolve( *this );
                table::BorderLine aBorderLine;
                ConversionHelper::MakeBorderLine( m_nLineWidth,   m_nLineType, m_nLineColor,
                                                                                aBorderLine );

                switch( rName )
                {
                    case NS_rtf::LN_BRCTOP:
                        m_aTopBorderLines.push_back(aBorderLine);
                    break;
                    case NS_rtf::LN_BRCLEFT:
                        m_aLeftBorderLines.push_back(aBorderLine);
                    break;
                    case NS_rtf::LN_BRCBOTTOM:
                        m_aBottomBorderLines.push_back(aBorderLine);
                    break;
                    case NS_rtf::LN_BRCRIGHT:
                        m_aRightBorderLines.push_back(aBorderLine);
                    break;
                    default:;
                }
            }
        }
        break;
        //from LN_BRCXXXX - handled within the BorderHandler
        case NS_rtf::LN_DPTLINEWIDTH: // 0x2871
            //  width of a single line in 1/8 pt, max of 32 pt -> twip * 5 / 2.
            m_nLineWidth = ConversionHelper::convertToMM100( nIntValue * 5 / 2 );
        break;
        case NS_rtf::LN_BRCTYPE:    // 0x2872
            m_nLineType = nIntValue;
        break;
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
        default:
            OSL_ASSERT("unknown attribute");
    }
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void TDefTableHandler::sprm(doctok::Sprm & rSprm)
{
    (void)rSprm;
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
        pPropertyMap->Insert( PROP_TABLE_COLUMN_SEPARATORS, uno::makeAny( aSeparators ) );
    }

    return pPropertyMap;
}
/*-- 10.05.2007 16:10:33---------------------------------------------------

  -----------------------------------------------------------------------*/
::boost::shared_ptr<PropertyMap> TDefTableHandler::getCellProperties( size_t nCell ) const
{
    PropertyMapPtr pPropertyMap(new PropertyMap);
    if( m_aCellBorderPositions.size() > nCell )
    {
        sal_Int16 nVertOrient = text::VertOrientation::NONE;
        switch( m_aCellVertAlign[nCell] ) //0 - top 1 - center 2 - bottom
        {
            case 1: nVertOrient = text::VertOrientation::CENTER; break;
            case 2: nVertOrient = text::VertOrientation::BOTTOM; break;
            default:;
        }
        pPropertyMap->Insert( PROP_VERT_ORIENT, uno::makeAny( nVertOrient ) );
    }
    if( m_aTopBorderLines.size() > nCell )
        pPropertyMap->Insert( PROP_TOP_BORDER, uno::makeAny( m_aTopBorderLines[nCell] ) );
    if( m_aLeftBorderLines.size() > nCell )
        pPropertyMap->Insert( PROP_LEFT_BORDER, uno::makeAny( m_aLeftBorderLines[nCell] ) );
    if( m_aBottomBorderLines.size() > nCell )
        pPropertyMap->Insert( PROP_BOTTOM_BORDER, uno::makeAny( m_aBottomBorderLines[nCell] ) );
    if( m_aRightBorderLines.size() > nCell )
        pPropertyMap->Insert( PROP_RIGHT_BORDER, uno::makeAny( m_aRightBorderLines[nCell] ) );
    return pPropertyMap;
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
