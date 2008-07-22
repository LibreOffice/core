/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CellColorHandler.cxx,v $
 * $Revision: 1.7 $
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
#include <CellColorHandler.hxx>
#include <PropertyMap.hxx>
#include <doctok/resourceids.hxx>
#include <ConversionHelper.hxx>
#include <ooxml/resourceids.hxx>

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;
using namespace ::writerfilter;
//using namespace ::std;

/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
CellColorHandler::CellColorHandler() :
    m_nShadowType( 0 ),
    m_nColor( 0xffffffff ),
    m_nFillColor( 0xffffffff ),
    m_bOOXMLColor( false ),
    m_bParagraph( false )
{
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
CellColorHandler::~CellColorHandler()
{
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void CellColorHandler::attribute(Id rName, Value & rVal)
{
    sal_Int32 nIntValue = rVal.getInt();
    (void)nIntValue;
    (void)rName;
    /* WRITERFILTERSTATUS: table: CellColor_attributedata */
    switch( rName )
    {
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_cellTopColor:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_cellLeftColor:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_cellBottomColor:
        /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_rtf::LN_cellRightColor:
            // nIntValue contains the color, directly
        break;
        /* WRITERFILTERSTATUS: done: 50, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Shd_val:
        {
            //might be clear, pct5...90, some hatch types
            //TODO: The values need symbolic names!
            m_nShadowType = nIntValue; //clear == 0, solid: 1, pct5: 2, pct50:8, pct95: x3c, horzStripe:0x0e, thinVertStripe: 0x15
            m_bOOXMLColor = m_nShadowType != 0;
        }
        break;
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Shd_fill:
            m_nFillColor = nIntValue;
        break;
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Shd_color:
            //color of the shading
            m_nColor = nIntValue;
        break;
//        case NS_rtf::LN_rgbrc:
//        {
//            writerfilter::Reference<Properties>::Pointer_t pProperties = rVal.getProperties();
//            if( pProperties.get())
//            {
//                pProperties->resolve(*this);
//                //
//            }
//        }
//        break;
        default:
            OSL_ENSURE( false, "unknown attribute");
    }
}
/*-- 24.04.2007 09:06:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void CellColorHandler::sprm(Sprm & rSprm)
{
    (void)rSprm;
}
/*-- 24.04.2007 09:09:01---------------------------------------------------

  -----------------------------------------------------------------------*/
TablePropertyMapPtr  CellColorHandler::getProperties()
{
    TablePropertyMapPtr pPropertyMap(new TablePropertyMap);
    if( m_bOOXMLColor )
    {
        pPropertyMap->Insert( m_bParagraph ? PROP_PARA_BACK_COLOR : PROP_BACK_COLOR, false,
                                uno::makeAny( m_nShadowType ? m_nColor : m_nFillColor ));
    }
    return pPropertyMap;
}
} //namespace dmapper
} //namespace writerfilter
