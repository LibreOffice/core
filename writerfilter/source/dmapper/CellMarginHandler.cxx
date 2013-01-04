/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
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

CellMarginHandler::~CellMarginHandler()
{
}

void CellMarginHandler::lcl_attribute(Id rName, Value & rVal)
{
    sal_Int32 nIntValue = rVal.getInt();
    (void)nIntValue;
    (void)rName;
    switch( rName )
    {
        case NS_ooxml::LN_CT_TblWidth_w:
            m_nValue = ConversionHelper::convertTwipToMM100( nIntValue );
        break;
        case NS_ooxml::LN_CT_TblWidth_type:
            OSL_ENSURE( NS_ooxml::LN_Value_ST_TblWidth_dxa == sal::static_int_cast<Id>(nIntValue), "cell margins work for absolute values, only");
        break;
        default:
            OSL_FAIL( "unknown attribute");
    }
}

void CellMarginHandler::lcl_sprm(Sprm & rSprm)
{
    writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
    if( pProperties.get())
    {
        pProperties.get()->resolve( *this );
        bool rtl = false; // TODO
        switch( rSprm.getId() )
        {
            case NS_ooxml::LN_CT_TblCellMar_top:
            case NS_ooxml::LN_CT_TcMar_top:
                m_nTopMargin = m_nValue;
                m_bTopMarginValid = true;
            break;
            case NS_ooxml::LN_CT_TblCellMar_start:
                if( rtl )
                {
                    m_nRightMargin = m_nValue;
                    m_bRightMarginValid = true;
                }
                else
                {
                    m_nLeftMargin = m_nValue;
                    m_bLeftMarginValid = true;
                }
            break;
            case NS_ooxml::LN_CT_TblCellMar_left:
            case NS_ooxml::LN_CT_TcMar_left:
                m_nLeftMargin = m_nValue;
                m_bLeftMarginValid = true;
            break;
            case NS_ooxml::LN_CT_TblCellMar_bottom:
            case NS_ooxml::LN_CT_TcMar_bottom:
                m_nBottomMargin = m_nValue;
                m_bBottomMarginValid = true;
            break;
            case NS_ooxml::LN_CT_TblCellMar_end:
                if( rtl )
                {
                    m_nLeftMargin = m_nValue;
                    m_bLeftMarginValid = true;
                }
                else
                {
                    m_nRightMargin = m_nValue;
                    m_bRightMarginValid = true;
                }
            break;
            case NS_ooxml::LN_CT_TblCellMar_right:
            case NS_ooxml::LN_CT_TcMar_right:
                m_nRightMargin = m_nValue;
                m_bRightMarginValid = true;
            break;
            default:
                OSL_FAIL( "unknown sprm");
        }
    }
    m_nValue = 0;
}
} //namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
