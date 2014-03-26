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
m_nWidth( 0 ),
m_nType( 0 ),
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
            m_nWidth = nIntValue;
            m_nValue = ConversionHelper::convertTwipToMM100Unsigned( nIntValue );
        break;
        case NS_ooxml::LN_CT_TblWidth_type:
            OSL_ENSURE( NS_ooxml::LN_Value_ST_TblWidth_dxa == sal::static_int_cast<Id>(nIntValue), "cell margins work for absolute values, only");
            m_nType = nIntValue;
        break;
        default:
            OSL_FAIL( "unknown attribute");
    }
}

void CellMarginHandler::createGrabBag(const OUString& aName)
{
    if (m_aInteropGrabBagName.isEmpty())
        return;

    beans::PropertyValue aRet;
    aRet.Name = aName;

    uno::Sequence<beans::PropertyValue> aSeq(2);
    aSeq[0].Name = "w";
    aSeq[0].Value = uno::makeAny(m_nWidth);
    aSeq[1].Name = "type";
    switch (m_nType)
    {
        case NS_ooxml::LN_Value_ST_TblWidth_nil: aSeq[1].Value = uno::makeAny(OUString("nil")); break;
        case NS_ooxml::LN_Value_ST_TblWidth_pct: aSeq[1].Value = uno::makeAny(OUString("pct")); break;
        case NS_ooxml::LN_Value_ST_TblWidth_dxa: aSeq[1].Value = uno::makeAny(OUString("dxa")); break;
        case NS_ooxml::LN_Value_ST_TblWidth_auto: aSeq[1].Value = uno::makeAny(OUString("auto")); break;
    }

    aRet.Value = uno::makeAny(aSeq);
    m_aInteropGrabBag.push_back(aRet);
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
                createGrabBag("top");
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
                createGrabBag("start");
            break;
            case NS_ooxml::LN_CT_TblCellMar_left:
            case NS_ooxml::LN_CT_TcMar_left:
                m_nLeftMargin = m_nValue;
                m_bLeftMarginValid = true;
                createGrabBag("left");
            break;
            case NS_ooxml::LN_CT_TblCellMar_bottom:
            case NS_ooxml::LN_CT_TcMar_bottom:
                m_nBottomMargin = m_nValue;
                m_bBottomMarginValid = true;
                createGrabBag("bottom");
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
                createGrabBag("end");
            break;
            case NS_ooxml::LN_CT_TblCellMar_right:
            case NS_ooxml::LN_CT_TcMar_right:
                m_nRightMargin = m_nValue;
                m_bRightMarginValid = true;
                createGrabBag("right");
            break;
            default:
                OSL_FAIL( "unknown sprm");
        }
    }
    m_nValue = 0;
}

void CellMarginHandler::enableInteropGrabBag(const OUString& aName)
{
    m_aInteropGrabBagName = aName;
}

beans::PropertyValue CellMarginHandler::getInteropGrabBag()
{
    beans::PropertyValue aRet;
    aRet.Name = m_aInteropGrabBagName;

    uno::Sequence<beans::PropertyValue> aSeq(m_aInteropGrabBag.size());
    beans::PropertyValue* pSeq = aSeq.getArray();
    for (std::vector<beans::PropertyValue>::iterator i = m_aInteropGrabBag.begin(); i != m_aInteropGrabBag.end(); ++i)
        *pSeq++ = *i;

    aRet.Value = uno::makeAny(aSeq);
    return aRet;
}

} //namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
