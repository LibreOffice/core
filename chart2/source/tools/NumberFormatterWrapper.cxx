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

#include "NumberFormatterWrapper.hxx"
#include "macros.hxx"
#include <comphelper/processfactory.hxx>
#include <svl/numuno.hxx>
#include <svl/zformat.hxx>
#include <tools/color.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <com/sun/star/util/Date.hpp>
#include <osl/diagnose.h>

namespace chart
{
using namespace ::com::sun::star;

FixedNumberFormatter::FixedNumberFormatter(
                const uno::Reference< util::XNumberFormatsSupplier >& xSupplier
                , sal_Int32 nNumberFormatKey )
            : m_aNumberFormatterWrapper(xSupplier)
            , m_nNumberFormatKey( nNumberFormatKey )
{
}

FixedNumberFormatter::~FixedNumberFormatter()
{
}

OUString FixedNumberFormatter::getFormattedString( double fValue, sal_Int32& rLabelColor, bool& rbColorChanged ) const
{
    return m_aNumberFormatterWrapper.getFormattedString(
        m_nNumberFormatKey, fValue, rLabelColor, rbColorChanged );
}

NumberFormatterWrapper::NumberFormatterWrapper( const uno::Reference< util::XNumberFormatsSupplier >& xSupplier )
                    : m_xNumberFormatsSupplier(xSupplier)
                    , m_pNumberFormatter(nullptr)

{
    uno::Reference<beans::XPropertySet> xProp(m_xNumberFormatsSupplier,uno::UNO_QUERY);
    OUString sNullDate( "NullDate" );
    if ( xProp.is() && xProp->getPropertySetInfo()->hasPropertyByName(sNullDate) )
        m_aNullDate = xProp->getPropertyValue(sNullDate);
    SvNumberFormatsSupplierObj* pSupplierObj = SvNumberFormatsSupplierObj::getImplementation( xSupplier );
    if( pSupplierObj )
        m_pNumberFormatter = pSupplierObj->GetNumberFormatter();
    SAL_WARN_IF(!m_pNumberFormatter,"chart2.tools","need a numberformatter");
}

NumberFormatterWrapper::~NumberFormatterWrapper()
{
}

Date NumberFormatterWrapper::getNullDate() const
{
    Date aRet(30,12,1899);

    util::Date aUtilDate;
    if( m_aNullDate.hasValue() && (m_aNullDate >>= aUtilDate) )
    {
        aRet = Date(aUtilDate.Day,aUtilDate.Month,aUtilDate.Year);
    }
    else if( m_pNumberFormatter )
    {
        Date* pDate = m_pNumberFormatter->GetNullDate();
        if( pDate )
            aRet = *pDate;
    }
    return aRet;
}

OUString NumberFormatterWrapper::getFormattedString( sal_Int32 nNumberFormatKey, double fValue,
                                                     sal_Int32& rLabelColor, bool& rbColorChanged ) const
{
    OUString aText;
    Color* pTextColor = nullptr;
    if( !m_pNumberFormatter )
    {
        OSL_FAIL("Need a NumberFormatter");
        return aText;
    }
    // i99104 handle null date correctly
    sal_Int16 nYear = 1899;
    sal_uInt16 nDay = 30,nMonth = 12;
    if ( m_aNullDate.hasValue() )
    {
        Date* pDate = m_pNumberFormatter->GetNullDate();
        if ( pDate )
        {
            nYear = pDate->GetYear();
            nMonth = pDate->GetMonth();
            nDay = pDate->GetDay();
        } // if ( pDate )
        util::Date aNewNullDate;
        m_aNullDate >>= aNewNullDate;
        m_pNumberFormatter->ChangeNullDate(aNewNullDate.Day,aNewNullDate.Month,aNewNullDate.Year);
    }
    m_pNumberFormatter->GetOutputString(fValue, nNumberFormatKey, aText, &pTextColor);
    if ( m_aNullDate.hasValue() )
    {
        m_pNumberFormatter->ChangeNullDate(nDay,nMonth,nYear);
    }

    if(pTextColor)
    {
        rbColorChanged = true;
        rLabelColor = pTextColor->GetColor();
    }
    else
        rbColorChanged = false;

    return aText;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
