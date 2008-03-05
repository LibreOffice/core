/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BorderHandler.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:48:49 $
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
#include <BorderHandler.hxx>
#include <PropertyMap.hxx>
#include <doctok/resourceids.hxx>
#include <ConversionHelper.hxx>
#include <com/sun/star/table/BorderLine.hpp>
#include <ooxml/resourceids.hxx>

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;
//using namespace ::std;

/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
BorderHandler::BorderHandler( bool bOOXML ) :
    m_nCurrentBorderPosition( BORDER_TOP ),
    m_nLineWidth(0),
    m_nLineType(0),
    m_nLineColor(0),
    m_nLineDistance(0),
    m_bOOXML( bOOXML )
{
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
BorderHandler::~BorderHandler()
{
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void BorderHandler::attribute(Id rName, Value & rVal)
{
    sal_Int32 nIntValue = rVal.getInt();
    /* WRITERFILTERSTATUS: table: BorderHandler_attributedata */
    switch( rName )
    {
        /* WRITERFILTERSTATUS: done: 75, planned: 0, spent: 0 */
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
        /* WRITERFILTERSTATUS: done: 75, planned: 0, spent: 0 */
        case NS_rtf::LN_DPTLINEWIDTH: // 0x2871
            //  width of a single line in 1/8 pt, max of 32 pt -> twip * 5 / 2.
            m_nLineWidth = ConversionHelper::convertTwipToMM100( nIntValue * 5 / 2 );
        break;
        /* WRITERFILTERSTATUS: done: 75, planned: 0, spent: 0 */
        case NS_rtf::LN_BRCTYPE:    // 0x2872
            m_nLineType = nIntValue;
        break;
        /* WRITERFILTERSTATUS: done: 75, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Border_color:
        case NS_rtf::LN_ICO:        // 0x2873
            m_nLineColor = nIntValue;
        break;
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
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
            OSL_ENSURE( false, "unknown attribute");
    }
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void BorderHandler::sprm(Sprm & rSprm)
{
    /* WRITERFILTERSTATUS: table: BorderHandler_sprm */
    switch( rSprm.getId())
    {
        /* WRITERFILTERSTATUS: done: 75, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_TblBorders_top:
        /* WRITERFILTERSTATUS: done: 75, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_TblBorders_left:
        /* WRITERFILTERSTATUS: done: 75, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_TblBorders_bottom:
        /* WRITERFILTERSTATUS: done: 75, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_TblBorders_right:
        /* WRITERFILTERSTATUS: done: 75, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_TblBorders_insideH:
        /* WRITERFILTERSTATUS: done: 75, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_TblBorders_insideV:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties.get())
                pProperties->resolve(*this);
            ConversionHelper::MakeBorderLine( m_nLineWidth,   m_nLineType, m_nLineColor,
                                   m_aBorderLines[rSprm.getId() - NS_ooxml::LN_CT_TblBorders_top], m_bOOXML );
        }
        break;
        default:;
    }
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
    // don't fill in default properties
    if( m_nCurrentBorderPosition )
    {
        for( sal_Int32 nProp = 0; nProp < BORDER_COUNT; ++nProp)
            pPropertyMap->Insert( aPropNames[nProp], false, uno::makeAny( m_aBorderLines[nProp] ) );
    }
    return pPropertyMap;
}
/*-- 14.11.2007 12:42:52---------------------------------------------------
    used only in OOXML import
  -----------------------------------------------------------------------*/
table::BorderLine BorderHandler::getBorderLine()
{
    table::BorderLine aBorderLine;
    ConversionHelper::MakeBorderLine( m_nLineWidth, m_nLineType, m_nLineColor, aBorderLine, m_bOOXML );
    return aBorderLine;
}
} //namespace dmapper
} //namespace writerfilter
