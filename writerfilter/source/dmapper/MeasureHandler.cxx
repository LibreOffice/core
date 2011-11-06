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


#include <MeasureHandler.hxx>
#include <PropertyMap.hxx>
#include <doctok/resourceids.hxx>
#include <ConversionHelper.hxx>
#include <ooxml/resourceids.hxx>
#include <com/sun/star/text/SizeType.hpp>
#include "dmapperLoggers.hxx"

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;

/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
MeasureHandler::MeasureHandler() :
LoggedProperties(dmapper_logger, "MeasureHandler"),
m_nMeasureValue( 0 ),
m_nUnit( -1 ),
m_nRowHeightSizeType( text::SizeType::MIN )
{
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
MeasureHandler::~MeasureHandler()
{
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void MeasureHandler::lcl_attribute(Id rName, Value & rVal)
{
    sal_Int32 nIntValue = rVal.getInt();
    (void)rName;
    /* WRITERFILTERSTATUS: table: MeasureHandler_attributedata */
    switch( rName )
    {
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_rtf::LN_unit:
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_TblWidth_type:// = 90668;
            //can be: NS_ooxml::LN_Value_ST_TblWidth_nil, NS_ooxml::LN_Value_ST_TblWidth_pct,
            //        NS_ooxml::LN_Value_ST_TblWidth_dxa, NS_ooxml::LN_Value_ST_TblWidth_auto;
            m_nUnit = nIntValue;
        break;
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Height_hRule: // 90666;
        {
            ::rtl::OUString sHeightType = rVal.getString();
            if( sHeightType.equalsAscii( "exact" ) )
                m_nRowHeightSizeType = text::SizeType::FIX;
        }
        break;
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_rtf::LN_trleft:
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_rtf::LN_preferredWidth:
        case NS_ooxml::LN_CT_TblWidth_w:// = 90667;
            m_nMeasureValue = nIntValue;
        break;
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Height_val: // 90665 -- a string value
        {
            m_nUnit = NS_ooxml::LN_Value_ST_TblWidth_dxa;
            ::rtl::OUString sHeight = rVal.getString();
            m_nMeasureValue = sHeight.toInt32();
        }
        break;
        default:
            OSL_ENSURE( false, "unknown attribute");
    }
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void MeasureHandler::lcl_sprm(Sprm & rSprm)
{
    (void)rSprm;
}
/*-- 24.04.2007 09:09:01---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 MeasureHandler::getMeasureValue() const
{
    sal_Int32 nRet = 0;
    if( m_nMeasureValue != 0 && m_nUnit >= 0 )
    {
        // TODO m_nUnit 3 - twip, other values unknown :-(
        if( m_nUnit == 3 || sal::static_int_cast<Id>(m_nUnit) == NS_ooxml::LN_Value_ST_TblWidth_dxa)
            nRet = ConversionHelper::convertTwipToMM100( m_nMeasureValue );
        //todo: handle additional width types:
        //NS_ooxml::LN_Value_ST_TblWidth_nil, NS_ooxml::LN_Value_ST_TblWidth_pct,
        //NS_ooxml::LN_Value_ST_TblWidth_dxa, NS_ooxml::LN_Value_ST_TblWidth_auto;
    }
    return nRet;
}
/*-- 18.06.2007 10:24:26---------------------------------------------------

  -----------------------------------------------------------------------*/
bool MeasureHandler::isAutoWidth() const
{
    return sal::static_int_cast<Id>(m_nUnit) == NS_ooxml::LN_Value_ST_TblWidth_auto;
}

} //namespace dmapper
} //namespace writerfilter
