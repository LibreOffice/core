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



MeasureHandler::MeasureHandler() :
LoggedProperties(dmapper_logger, "MeasureHandler"),
m_nMeasureValue( 0 ),
m_nUnit( -1 ),
m_nRowHeightSizeType( text::SizeType::MIN )
{
}


MeasureHandler::~MeasureHandler()
{
}


void MeasureHandler::lcl_attribute(Id rName, Value & rVal)
{
    sal_Int32 nIntValue = rVal.getInt();
    (void)rName;
    switch( rName )
    {
        case NS_rtf::LN_unit:
        case NS_ooxml::LN_CT_TblWidth_type:// = 90668;
            //can be: NS_ooxml::LN_Value_ST_TblWidth_nil, NS_ooxml::LN_Value_ST_TblWidth_pct,
            //        NS_ooxml::LN_Value_ST_TblWidth_dxa, NS_ooxml::LN_Value_ST_TblWidth_auto;
            m_nUnit = nIntValue;
        break;
        case NS_ooxml::LN_CT_Height_hRule: // 90666;
        {
            ::rtl::OUString sHeightType = rVal.getString();
            if( sHeightType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "exact" ) ) )
                m_nRowHeightSizeType = text::SizeType::FIX;
        }
        break;
        case NS_rtf::LN_trleft:
        case NS_rtf::LN_preferredWidth:
        case NS_ooxml::LN_CT_TblWidth_w:// = 90667;
            m_nMeasureValue = nIntValue;
        break;
        case NS_ooxml::LN_CT_Height_val: // 90665 -- a string value
        {
            m_nUnit = NS_ooxml::LN_Value_ST_TblWidth_dxa;
            ::rtl::OUString sHeight = rVal.getString();
            m_nMeasureValue = sHeight.toInt32();
        }
        break;
        default:
            OSL_FAIL( "unknown attribute");
    }
}


void MeasureHandler::lcl_sprm(Sprm & rSprm)
{
    (void)rSprm;
}


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


bool MeasureHandler::isAutoWidth() const
{
    return sal::static_int_cast<Id>(m_nUnit) == NS_ooxml::LN_Value_ST_TblWidth_auto;
}

} //namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
