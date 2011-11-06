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


#include <CellMarginHandler.hxx>
#include <PropertyMap.hxx>
#include <doctok/resourceids.hxx>
#include <ConversionHelper.hxx>
#include <ooxml/resourceids.hxx>
#include "dmapperLoggers.hxx"

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;
using namespace ::writerfilter;
//using namespace ::std;

/*-- 18.02.2008 12:36:51---------------------------------------------------

  -----------------------------------------------------------------------*/
CellMarginHandler::CellMarginHandler() :
LoggedProperties(dmapper_logger, "CellMarginHandler"),
m_nValue( 0 ),
m_nLeftMargin( 0 ),
m_bLeftMarginValid( false ),
m_nRightMargin( 0 ),
m_bRightMarginValid( false ),
m_nTopMargin( 0 ),
m_bTopMarginValid( false ),
m_nBottomMargin( 0 ),
m_bBottomMarginValid( false )
{
}
/*-- 18.02.2008 12:36:51---------------------------------------------------

  -----------------------------------------------------------------------*/
CellMarginHandler::~CellMarginHandler()
{
}
/*-- 18.02.2008 12:36:51---------------------------------------------------

  -----------------------------------------------------------------------*/
void CellMarginHandler::lcl_attribute(Id rName, Value & rVal)
{
    sal_Int32 nIntValue = rVal.getInt();
    (void)nIntValue;
    (void)rName;
    /* WRITERFILTERSTATUS: table: CellColor_attributedata */
    switch( rName )
    {
        case NS_ooxml::LN_CT_TblWidth_w:
            m_nValue = ConversionHelper::convertTwipToMM100( nIntValue );
        break;
        case NS_ooxml::LN_CT_TblWidth_type:
            OSL_ENSURE( NS_ooxml::LN_Value_ST_TblWidth_dxa == sal::static_int_cast<Id>(nIntValue), "cell margins work for absolute values, only");
        break;
        default:
            OSL_ENSURE( false, "unknown attribute");
    }
}
/*-- 18.02.2008 12:36:51---------------------------------------------------

  -----------------------------------------------------------------------*/
void CellMarginHandler::lcl_sprm(Sprm & rSprm)
{
    writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
    if( pProperties.get())
    {
        pProperties.get()->resolve( *this );
        switch( rSprm.getId() )
        {
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            case NS_ooxml::LN_CT_TblCellMar_top:
                m_nTopMargin = m_nValue;
                m_bTopMarginValid = true;
            break;
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            case NS_ooxml::LN_CT_TblCellMar_left:
                m_nLeftMargin = m_nValue;
                m_bLeftMarginValid = true;
            break;
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            case NS_ooxml::LN_CT_TblCellMar_bottom:
                m_nBottomMargin = m_nValue;
                m_bBottomMarginValid = true;
            break;
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
            case NS_ooxml::LN_CT_TblCellMar_right:
                m_nRightMargin = m_nValue;
                m_bRightMarginValid = true;
            break;
            default:
                OSL_ENSURE( false, "unknown attribute");
        }
    }
    m_nValue = 0;
}
} //namespace dmapper
} //namespace writerfilter
