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
#include <SectionColumnHandler.hxx>
#include <PropertyMap.hxx>
#include <doctok/resourceids.hxx>
#include <ConversionHelper.hxx>
#include <ooxml/resourceids.hxx>

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;
//using namespace ::std;

/*-- 02.06.2008 13:36:24---------------------------------------------------

  -----------------------------------------------------------------------*/
SectionColumnHandler::SectionColumnHandler() :
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
void SectionColumnHandler::attribute(Id rName, Value & rVal)
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
void SectionColumnHandler::sprm(Sprm & rSprm)
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
