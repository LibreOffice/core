/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <CellMarginHandler.hxx>
#include <PropertyMap.hxx>
#include <doctok/resourceids.hxx>
#include <ConversionHelper.hxx>
#include <ooxml/resourceids.hxx>

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;
using namespace ::writerfilter;
//using namespace ::std;

/*-- 18.02.2008 12:36:51---------------------------------------------------

  -----------------------------------------------------------------------*/
CellMarginHandler::CellMarginHandler() :
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
void CellMarginHandler::attribute(Id rName, Value & rVal)
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
void CellMarginHandler::sprm(Sprm & rSprm)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
