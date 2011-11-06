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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "NumberFormatterWrapper.hxx"
#include "macros.hxx"
#include <comphelper/processfactory.hxx>
// header for class SvNumberFormatsSupplierObj
#include <svl/numuno.hxx>
// header for class SvNumberformat
#include <svl/zformat.hxx>
#include <tools/color.hxx>
#include <i18npool/mslangid.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/util/DateTime.hpp>

//.............................................................................
namespace chart
{
//.............................................................................
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

/*
sal_Int32 FixedNumberFormatter::getTextAndColor( double fUnscaledValueForText, rtl::OUString& rLabel ) const
{
    sal_Int32 nLabelColor = Color(COL_BLUE).GetColor(); //@todo get this from somewheres
    rLabel = getFormattedString( fUnscaledValueForText, nLabelColor );
    return nLabelColor;
}
*/

rtl::OUString FixedNumberFormatter::getFormattedString( double fValue, sal_Int32& rLabelColor, bool& rbColorChanged ) const
{
    return m_aNumberFormatterWrapper.getFormattedString(
        m_nNumberFormatKey, fValue, rLabelColor, rbColorChanged );
}

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

NumberFormatterWrapper::NumberFormatterWrapper( const uno::Reference< util::XNumberFormatsSupplier >& xSupplier )
                    : m_xNumberFormatsSupplier(xSupplier)
                    , m_pNumberFormatter(NULL)

{
    uno::Reference<beans::XPropertySet> xProp(m_xNumberFormatsSupplier,uno::UNO_QUERY);
    rtl::OUString sNullDate( RTL_CONSTASCII_USTRINGPARAM("NullDate"));
    if ( xProp.is() && xProp->getPropertySetInfo()->hasPropertyByName(sNullDate) )
        m_aNullDate = xProp->getPropertyValue(sNullDate);
    SvNumberFormatsSupplierObj* pSupplierObj = SvNumberFormatsSupplierObj::getImplementation( xSupplier );
    if( pSupplierObj )
        m_pNumberFormatter = pSupplierObj->GetNumberFormatter();
    DBG_ASSERT(m_pNumberFormatter,"need a numberformatter");
}

NumberFormatterWrapper::~NumberFormatterWrapper()
{
}

SvNumberFormatter* NumberFormatterWrapper::getSvNumberFormatter() const
{
    return m_pNumberFormatter;
}

Date NumberFormatterWrapper::getNullDate() const
{
    sal_uInt16 nYear = 1899,nDay = 30,nMonth = 12;
    Date aRet(nDay,nMonth,nYear);

    util::DateTime aUtilDate;
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

rtl::OUString NumberFormatterWrapper::getFormattedString(
    sal_Int32 nNumberFormatKey, double fValue, sal_Int32& rLabelColor, bool& rbColorChanged ) const
{
    String aText;
    Color* pTextColor = NULL;
    if( !m_pNumberFormatter )
    {
        DBG_ERROR("Need a NumberFormatter");
        return aText;
    }
    // i99104 handle null date correctly
    sal_uInt16 nYear = 1899,nDay = 30,nMonth = 12;
    if ( m_aNullDate.hasValue() )
    {
        Date* pDate = m_pNumberFormatter->GetNullDate();
        if ( pDate )
        {
            nYear = pDate->GetYear();
            nMonth = pDate->GetMonth();
            nDay = pDate->GetDay();
        } // if ( pDate )
        util::DateTime aNewNullDate;
        m_aNullDate >>= aNewNullDate;
        m_pNumberFormatter->ChangeNullDate(aNewNullDate.Day,aNewNullDate.Month,aNewNullDate.Year);
    }
    m_pNumberFormatter->GetOutputString(
        fValue, nNumberFormatKey, aText, &pTextColor);
    if ( m_aNullDate.hasValue() )
    {
        m_pNumberFormatter->ChangeNullDate(nDay,nMonth,nYear);
    }
    rtl::OUString aRet( aText );

    if(pTextColor)
    {
        rbColorChanged = true;
        rLabelColor = pTextColor->GetColor();
    }
    else
        rbColorChanged = false;

    return aRet;
}

// to get the language type use MsLangId::convertLocaleToLanguage( rNumberFormat.aLocale )

/*
    uno::Reference< i18n::XNumberFormatCode > xNumberFormatCode(
        m_xCC->getServiceManager()->createInstanceWithContext( C2U(
        "com.sun.star.i18n.NumberFormatMapper" ), m_xCC ), uno::UNO_QUERY );

    i18n::NumberFormatCode aNumberFormatCode = xNumberFormatCode->getDefault (
        i18n::KNumberFormatType::MEDIUM,
        i18n::KNumberFormatUsage::SCIENTIFIC_NUMBER,
        aLocale );

    uno::Sequence< i18n::NumberFormatCode > aListOfNumberFormatCode = xNumberFormatCode->getAllFormatCode(
        i18n::KNumberFormatUsage::SCIENTIFIC_NUMBER,
        aLocale );

    i18n::NumberFormatCode aNumberFormatCode0 = aListOfNumberFormatCode[0];
    i18n::NumberFormatCode aNumberFormatCode1 = aListOfNumberFormatCode[1];
*/

//.............................................................................
} //namespace chart
//.............................................................................
