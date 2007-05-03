/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BorderHandler.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2007-05-03 06:25:37 $
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
#ifndef INCLUDED_BORDERHANDLER_HXX
#include <BorderHandler.hxx>
#endif
#ifndef INCLUDED_DMAPPER_PROPERTYMAP_HXX
#include <PropertyMap.hxx>
#endif
#ifndef INCLUDED_RESOURCESIDS
#include <doctok/resourceids.hxx>
#endif
#ifndef INCLUDED_DMAPPER_CONVERSIONHELPER_HXX
#include <ConversionHelper.hxx>
#endif
#ifndef _COM_SUN_STAR_TABLE_BORDERLINE_HPP_
#include <com/sun/star/table/BorderLine.hpp>
#endif

namespace dmapper {

using namespace ::com::sun::star;
using namespace writerfilter;
//using namespace ::std;

/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
BorderHandler::BorderHandler() :
    m_nCurrentBorderPosition( BORDER_TOP ),
    m_nLineWidth(0),
    m_nLineType(0),
    m_nLineColor(0),
    m_nLineDistance(0)
{
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
BorderHandler::~BorderHandler()
{
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void BorderHandler::attribute(doctok::Id rName, doctok::Value & rVal)
{
    sal_Int32 nIntValue = rVal.getInt();
    switch( rName )
    {
        case NS_rtf::LN_rgbrc:
        {
            doctok::Reference<Properties>::Pointer_t pProperties = rVal.getProperties();
            if( pProperties.get())
            {
                pProperties->resolve(*this);
                //
//                table::BorderLine* pToFill = 0;
//                switch(m_nCurrentBorderPosition)
//                {
//                    case BORDER_TOP:
//                        pToFill = &m_aTableBorder.TopLine;
//                        m_aTableBorder.IsTopLineValid = sal_True;
//                    break;
//                    case BORDER_LEFT:
//                        pToFill = &m_aTableBorder.LeftLine;
//                        m_aTableBorder.IsLeftLineValid = sal_True;
//                    break;
//                    case BORDER_BOTTOM:
//                        pToFill = &m_aTableBorder.BottomLine;
//                        m_aTableBorder.IsBottomLineValid = sal_True;;
//                    break;
//                    case BORDER_RIGHT:
//                        pToFill = &m_aTableBorder.RightLine;
//                        m_aTableBorder.IsRightLineValid = sal_True;
//                    break;
//                    case BORDER_HORIZONTAL:
//                        pToFill = &m_aTableBorder.HorizontalLine;
//                        m_aTableBorder.IsHorizontalLineValid = sal_True;;
//                    break;
//                    case BORDER_VERTICAL:
//                        m_aTableBorder.IsVerticalLineValid = sal_True;;
//                    default:
//                        pToFill = &m_aTableBorder.VerticalLine; break;
//                }
                ConversionHelper::MakeBorderLine( m_nLineWidth,   m_nLineType, m_nLineColor,
                                                                                m_aBorderLines[m_nCurrentBorderPosition] );
                OSL_ENSURE(m_nCurrentBorderPosition < BORDER_COUNT, "too many border values");
                ++m_nCurrentBorderPosition;
            }
        }
        break;
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
void BorderHandler::sprm(doctok::Sprm & rSprm)
{
    (void)rSprm;
}
/*-- 24.04.2007 09:09:01---------------------------------------------------

  -----------------------------------------------------------------------*/
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
    for( sal_Int32 nProp = 0; nProp < BORDER_COUNT; ++nProp)
        pPropertyMap->Insert( aPropNames[nProp], uno::makeAny( m_aBorderLines[nProp] ) );
    return pPropertyMap;
}
} //namespace dmapper
