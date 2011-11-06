/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#include <BorderHandler.hxx>
#include <PropertyMap.hxx>
#include <resourcemodel/QNameToString.hxx>
#include <doctok/resourceids.hxx>
#include <ConversionHelper.hxx>
#include <com/sun/star/table/BorderLine.hpp>
#include <ooxml/resourceids.hxx>
#include <dmapperLoggers.hxx>

namespace writerfilter {

namespace dmapper {

using namespace ::com::sun::star;
//using namespace ::std;

/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
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
    std::fill_n(m_aBorderLines, nBorderCount, table::BorderLine());
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
BorderHandler::~BorderHandler()
{
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void BorderHandler::lcl_attribute(Id rName, Value & rVal)
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
void BorderHandler::lcl_sprm(Sprm & rSprm)
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

            m_aFilledLines[ rSprm.getId( ) - NS_ooxml::LN_CT_TblBorders_top] = true;
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
