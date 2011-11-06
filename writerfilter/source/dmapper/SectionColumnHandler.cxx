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


#include <SectionColumnHandler.hxx>
#include <PropertyMap.hxx>
#include <doctok/resourceids.hxx>
#include <ConversionHelper.hxx>
#include <ooxml/resourceids.hxx>

#include "dmapperLoggers.hxx"

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;
//using namespace ::std;

/*-- 02.06.2008 13:36:24---------------------------------------------------

  -----------------------------------------------------------------------*/
SectionColumnHandler::SectionColumnHandler() :
LoggedProperties(dmapper_logger, "SectionColumnHandler"),
bEqualWidth( false ),
nSpace( 0 ),
nNum( 0 ),
bSep( false )
{
}
/*-- 02.06.2008 13:36:24---------------------------------------------------

  -----------------------------------------------------------------------*/
SectionColumnHandler::~SectionColumnHandler()
{
}
/*-- 02.06.2008 13:36:24---------------------------------------------------

  -----------------------------------------------------------------------*/
void SectionColumnHandler::lcl_attribute(Id rName, Value & rVal)
{
    sal_Int32 nIntValue = rVal.getInt();
    /* WRITERFILTERSTATUS: table: SectionColumnHandler_attributedata */
    switch( rName )
    {
        case NS_ooxml::LN_CT_Columns_equalWidth:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            bEqualWidth = (nIntValue != 0);
            break;
        case NS_ooxml::LN_CT_Columns_space:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            nSpace = ConversionHelper::convertTwipToMM100( nIntValue );
            break;
        case NS_ooxml::LN_CT_Columns_num:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            nNum = nIntValue;
            break;
        case NS_ooxml::LN_CT_Columns_sep:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            bSep = (nIntValue != 0);
            break;

        case NS_ooxml::LN_CT_Column_w:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            aTempColumn.nWidth = ConversionHelper::convertTwipToMM100( nIntValue );
            break;
        case NS_ooxml::LN_CT_Column_space:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            aTempColumn.nSpace = ConversionHelper::convertTwipToMM100( nIntValue );
            break;
        default:
            OSL_ENSURE( false, "SectionColumnHandler: unknown attribute");
    }
}
/*-- 02.06.2008 13:36:24---------------------------------------------------

  -----------------------------------------------------------------------*/
void SectionColumnHandler::lcl_sprm(Sprm & rSprm)
{
    /* WRITERFILTERSTATUS: table: SectionColumnHandler_sprm */
    switch( rSprm.getId())
    {
        case NS_ooxml::LN_CT_Columns_col:
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        {
            aTempColumn.nWidth = aTempColumn.nSpace = 0;
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties.get())
            {
                pProperties->resolve(*this);
                aCols.push_back(aTempColumn);
            }
        }
        break;
        default:
            OSL_ENSURE( false, "SectionColumnHandler: unknown sprm");
    }
}
} //namespace dmapper
} //namespace writerfilter
