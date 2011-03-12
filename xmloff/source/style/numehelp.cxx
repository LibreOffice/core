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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#include "xmloff/numehelp.hxx"

#include <xmloff/nmspmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlexp.hxx>
#include <com/sun/star/uno/Reference.h>
#include <rtl/ustring.hxx>
#include <svl/zforlist.hxx>
#include <com/sun/star/util/NumberFormat.hpp>
#include <rtl/math.hxx>
#include <tools/debug.hxx>
#include <rtl/ustrbuf.hxx>

using namespace com::sun::star;
using namespace xmloff::token;

#define XML_TYPE "Type"
#define XML_CURRENCYSYMBOL "CurrencySymbol"
#define XML_CURRENCYABBREVIATION "CurrencyAbbreviation"
#define XML_STANDARDFORMAT "StandardFormat"

XMLNumberFormatAttributesExportHelper::XMLNumberFormatAttributesExportHelper(
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& xTempNumberFormatsSupplier)
    : xNumberFormats(xTempNumberFormatsSupplier.is() ? xTempNumberFormatsSupplier->getNumberFormats() : ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats > ()),
    pExport(NULL),
    sStandardFormat(RTL_CONSTASCII_USTRINGPARAM(XML_STANDARDFORMAT)),
    sType(RTL_CONSTASCII_USTRINGPARAM(XML_TYPE)),
    msCurrencySymbol(RTL_CONSTASCII_USTRINGPARAM(XML_CURRENCYSYMBOL)),
    msCurrencyAbbreviation(RTL_CONSTASCII_USTRINGPARAM(XML_CURRENCYABBREVIATION)),
    aNumberFormats()
{
}

XMLNumberFormatAttributesExportHelper::XMLNumberFormatAttributesExportHelper(
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& xTempNumberFormatsSupplier,
            SvXMLExport& rTempExport )
:   xNumberFormats(xTempNumberFormatsSupplier.is() ? xTempNumberFormatsSupplier->getNumberFormats() : ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats > ()),
    pExport(&rTempExport),
    sStandardFormat(RTL_CONSTASCII_USTRINGPARAM(XML_STANDARDFORMAT)),
    sType(RTL_CONSTASCII_USTRINGPARAM(XML_TYPE)),
    sAttrValueType(rTempExport.GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_OFFICE, GetXMLToken(XML_VALUE_TYPE))),
    sAttrValue(rTempExport.GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_OFFICE, GetXMLToken(XML_VALUE))),
    sAttrDateValue(rTempExport.GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_OFFICE, GetXMLToken(XML_DATE_VALUE))),
    sAttrTimeValue(rTempExport.GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_OFFICE, GetXMLToken(XML_TIME_VALUE))),
    sAttrBooleanValue(rTempExport.GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_OFFICE, GetXMLToken(XML_BOOLEAN_VALUE))),
    sAttrStringValue(rTempExport.GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_OFFICE, GetXMLToken(XML_STRING_VALUE))),
    sAttrCurrency(rTempExport.GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_OFFICE, GetXMLToken(XML_CURRENCY))),
    msCurrencySymbol(RTL_CONSTASCII_USTRINGPARAM(XML_CURRENCYSYMBOL)),
    msCurrencyAbbreviation(RTL_CONSTASCII_USTRINGPARAM(XML_CURRENCYABBREVIATION)),
    aNumberFormats()
{
}

XMLNumberFormatAttributesExportHelper::~XMLNumberFormatAttributesExportHelper()
{
}

sal_Int16 XMLNumberFormatAttributesExportHelper::GetCellType(const sal_Int32 nNumberFormat, rtl::OUString& sCurrency, sal_Bool& bIsStandard)
{
    XMLNumberFormat aFormat(sEmpty, nNumberFormat, 0);
    XMLNumberFormatSet::iterator aItr(aNumberFormats.find(aFormat));
    XMLNumberFormatSet::iterator aEndItr(aNumberFormats.end());
    if (aItr != aEndItr)
    {
        bIsStandard = aItr->bIsStandard;
        sCurrency = aItr->sCurrency;
        return aItr->nType;
    }
    else
    {
        aFormat.nType = GetCellType(nNumberFormat, bIsStandard);
        aFormat.bIsStandard = bIsStandard;
        if ((aFormat.nType & ~util::NumberFormat::DEFINED) == util::NumberFormat::CURRENCY)
            if (GetCurrencySymbol(nNumberFormat, aFormat.sCurrency))
                sCurrency = aFormat.sCurrency;
        aNumberFormats.insert(aFormat);
        return aFormat.nType;
    }
}

void XMLNumberFormatAttributesExportHelper::WriteAttributes(SvXMLExport& rXMLExport,
                                const sal_Int16 nTypeKey,
                                const double& rValue,
                                const rtl::OUString& rCurrency,
                                sal_Bool bExportValue)
{
    sal_Bool bWasSetTypeAttribute = sal_False;
    switch(nTypeKey & ~util::NumberFormat::DEFINED)
    {
    case 0:
    case util::NumberFormat::NUMBER:
    case util::NumberFormat::SCIENTIFIC:
    case util::NumberFormat::FRACTION:
        {
            if (!bWasSetTypeAttribute)
            {
                rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_FLOAT);
                bWasSetTypeAttribute = sal_True;
            }
        }       // No Break
    case util::NumberFormat::PERCENT:
        {
            if (!bWasSetTypeAttribute)
            {
                rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_PERCENTAGE);
                bWasSetTypeAttribute = sal_True;
            }
        }       // No Break
    case util::NumberFormat::CURRENCY:
        {
            if (!bWasSetTypeAttribute)
            {
                rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_CURRENCY);
                if (rCurrency.getLength() > 0)
                    rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_CURRENCY, rCurrency);
                bWasSetTypeAttribute = sal_True;
            }

            if (bExportValue)
            {
                rtl::OUString sValue( ::rtl::math::doubleToUString( rValue,
                            rtl_math_StringFormat_Automatic,
                            rtl_math_DecimalPlaces_Max, '.', sal_True));
                rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE, sValue);
            }
        }
        break;
    case util::NumberFormat::DATE:
    case util::NumberFormat::DATETIME:
        {
            if (!bWasSetTypeAttribute)
            {
                rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_DATE);
                bWasSetTypeAttribute = sal_True;
            }
            if (bExportValue)
            {
                if ( rXMLExport.SetNullDateOnUnitConverter() )
                {
                    rtl::OUStringBuffer sBuffer;
                    rXMLExport.GetMM100UnitConverter().convertDateTime(sBuffer, rValue);
                    rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_DATE_VALUE, sBuffer.makeStringAndClear());
                }
            }
        }
        break;
    case util::NumberFormat::TIME:
        {
            if (!bWasSetTypeAttribute)
            {
                rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_TIME);
                bWasSetTypeAttribute = sal_True;
            }
            if (bExportValue)
            {
                rtl::OUStringBuffer sBuffer;
                rXMLExport.GetMM100UnitConverter().convertTime(sBuffer, rValue);
                rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_TIME_VALUE, sBuffer.makeStringAndClear());
            }
        }
        break;
    case util::NumberFormat::LOGICAL:
        {
            if (!bWasSetTypeAttribute)
            {
                rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_BOOLEAN);
                bWasSetTypeAttribute = sal_True;
            }
            if (bExportValue)
            {
                double fTempValue = rValue;
                if (::rtl::math::approxEqual( fTempValue, 1.0 ))
                {
                    rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_BOOLEAN_VALUE, XML_TRUE);
                }
                else
                {
                    if (::rtl::math::approxEqual( rValue, 0.0 ))
                    {
                        rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_BOOLEAN_VALUE, XML_FALSE);
                    }
                    else
                    {
                        rtl::OUString sValue( ::rtl::math::doubleToUString(
                                    fTempValue,
                                    rtl_math_StringFormat_Automatic,
                                    rtl_math_DecimalPlaces_Max, '.',
                                    sal_True));
                        rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_BOOLEAN_VALUE, sValue);
                    }
                }
            }
        }
        break;
    case util::NumberFormat::TEXT:
        {
            if (!bWasSetTypeAttribute)
            {
                rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_FLOAT);
                bWasSetTypeAttribute = sal_True;
                if (bExportValue)
                {
                    rtl::OUString sValue( ::rtl::math::doubleToUString( rValue,
                                rtl_math_StringFormat_Automatic,
                                rtl_math_DecimalPlaces_Max, '.', sal_True));
                    rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE, sValue);
                }
            }
        }
        break;
    }
}

sal_Bool XMLNumberFormatAttributesExportHelper::GetCurrencySymbol(const sal_Int32 nNumberFormat, rtl::OUString& sCurrencySymbol,
    uno::Reference <util::XNumberFormatsSupplier>& xNumberFormatsSupplier)
{
    if (xNumberFormatsSupplier.is())
    {
        uno::Reference <util::XNumberFormats> xNumberFormats(xNumberFormatsSupplier->getNumberFormats());
        if (xNumberFormats.is())
        {
            try
            {
                uno::Reference <beans::XPropertySet> xNumberPropertySet(xNumberFormats->getByKey(nNumberFormat));
                if ( xNumberPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_CURRENCYSYMBOL))) >>= sCurrencySymbol)
                {
                    rtl::OUString sCurrencyAbbreviation;
                    if ( xNumberPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_CURRENCYABBREVIATION))) >>= sCurrencyAbbreviation)
                    {
                        if ( sCurrencyAbbreviation.getLength() != 0 )
                            sCurrencySymbol = sCurrencyAbbreviation;
                        else
                        {
                            if ( sCurrencySymbol.getLength() == 1 && sCurrencySymbol.toChar() == NfCurrencyEntry::GetEuroSymbol() )
                                sCurrencySymbol = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("EUR"));
                        }
                    }
                    return sal_True;
                }
            }
            catch ( uno::Exception& )
            {
                OSL_FAIL("Numberformat not found");
            }
        }
    }
    return sal_False;
}


sal_Int16 XMLNumberFormatAttributesExportHelper::GetCellType(const sal_Int32 nNumberFormat, sal_Bool& bIsStandard,
    uno::Reference <util::XNumberFormatsSupplier>& xNumberFormatsSupplier)
{
    if (xNumberFormatsSupplier.is())
    {
        uno::Reference <util::XNumberFormats> xNumberFormats(xNumberFormatsSupplier->getNumberFormats());
        if (xNumberFormats.is())
        {
            try
            {
                uno::Reference <beans::XPropertySet> xNumberPropertySet(xNumberFormats->getByKey(nNumberFormat));
                xNumberPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STANDARDFORMAT))) >>= bIsStandard;
                sal_Int16 nNumberType = sal_Int16();
                if ( xNumberPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_TYPE))) >>= nNumberType )
                {
                    return nNumberType;
                }
            }
            catch ( uno::Exception& )
            {
                OSL_FAIL("Numberformat not found");
            }
        }
    }
    return 0;
}

void XMLNumberFormatAttributesExportHelper::SetNumberFormatAttributes(SvXMLExport& rXMLExport,
    const sal_Int32 nNumberFormat, const double& rValue, sal_Bool bExportValue)
{
    sal_Bool bIsStandard;
    sal_Int16 nTypeKey = GetCellType(nNumberFormat, bIsStandard, rXMLExport.GetNumberFormatsSupplier());
    rtl::OUString sCurrency;
    if ((nTypeKey & ~util::NumberFormat::DEFINED) == util::NumberFormat::CURRENCY)
        GetCurrencySymbol(nNumberFormat, sCurrency, rXMLExport.GetNumberFormatsSupplier());
    WriteAttributes(rXMLExport, nTypeKey, rValue, sCurrency, bExportValue);
}

void XMLNumberFormatAttributesExportHelper::SetNumberFormatAttributes(SvXMLExport& rXMLExport,
    const rtl::OUString& rValue, const rtl::OUString& rCharacters,
    sal_Bool bExportValue, sal_Bool bExportTypeAttribute)
{
    if (bExportTypeAttribute)
        rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_STRING);
    if (bExportValue && rValue.getLength() && (rValue != rCharacters))
        rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_STRING_VALUE, rValue);
}

sal_Bool XMLNumberFormatAttributesExportHelper::GetCurrencySymbol(const sal_Int32 nNumberFormat, rtl::OUString& rCurrencySymbol)
{
    if (!xNumberFormats.is() && pExport && pExport->GetNumberFormatsSupplier().is())
        xNumberFormats.set(pExport->GetNumberFormatsSupplier()->getNumberFormats());

    if (xNumberFormats.is())
    {
        try
        {
            uno::Reference <beans::XPropertySet> xNumberPropertySet(xNumberFormats->getByKey(nNumberFormat));
            if ( xNumberPropertySet->getPropertyValue(msCurrencySymbol) >>= rCurrencySymbol)
            {
                rtl::OUString sCurrencyAbbreviation;
                if ( xNumberPropertySet->getPropertyValue(msCurrencyAbbreviation) >>= sCurrencyAbbreviation)
                {
                    if ( sCurrencyAbbreviation.getLength() != 0 )
                        rCurrencySymbol = sCurrencyAbbreviation;
                    else
                    {
                        if ( rCurrencySymbol.getLength() == 1 && rCurrencySymbol.toChar() == NfCurrencyEntry::GetEuroSymbol() )
                            rCurrencySymbol = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("EUR"));
                    }
                }
                return sal_True;
            }
        }
        catch ( uno::Exception& )
        {
            OSL_FAIL("Numberformat not found");
        }
    }
    return sal_False;
}

sal_Int16 XMLNumberFormatAttributesExportHelper::GetCellType(const sal_Int32 nNumberFormat, sal_Bool& bIsStandard)
{
    if (!xNumberFormats.is() && pExport && pExport->GetNumberFormatsSupplier().is())
        xNumberFormats.set(pExport->GetNumberFormatsSupplier()->getNumberFormats());

    if (xNumberFormats.is())
    {
        try
        {
            uno::Reference <beans::XPropertySet> xNumberPropertySet(xNumberFormats->getByKey(nNumberFormat));
            if (xNumberPropertySet.is())
            {
                xNumberPropertySet->getPropertyValue(sStandardFormat) >>= bIsStandard;
                sal_Int16 nNumberType = sal_Int16();
                if ( xNumberPropertySet->getPropertyValue(sType) >>= nNumberType )
                {
                    return nNumberType;
                }
            }
        }
        catch ( uno::Exception& )
        {
            OSL_FAIL("Numberformat not found");
        }
    }
    return 0;
}

void XMLNumberFormatAttributesExportHelper::WriteAttributes(
                                const sal_Int16 nTypeKey,
                                const double& rValue,
                                const rtl::OUString& rCurrency,
                                sal_Bool bExportValue)
{
    if (!pExport)
        return;

    sal_Bool bWasSetTypeAttribute = sal_False;
    switch(nTypeKey & ~util::NumberFormat::DEFINED)
    {
    case 0:
    case util::NumberFormat::NUMBER:
    case util::NumberFormat::SCIENTIFIC:
    case util::NumberFormat::FRACTION:
        {
            if (!bWasSetTypeAttribute)
            {
                pExport->AddAttribute(sAttrValueType, XML_FLOAT);
                bWasSetTypeAttribute = sal_True;
            }
        }       // No Break
    case util::NumberFormat::PERCENT:
        {
            if (!bWasSetTypeAttribute)
            {
                pExport->AddAttribute(sAttrValueType, XML_PERCENTAGE);
                bWasSetTypeAttribute = sal_True;
            }
        }       // No Break
    case util::NumberFormat::CURRENCY:
        {
            if (!bWasSetTypeAttribute)
            {
                pExport->AddAttribute(sAttrValueType, XML_CURRENCY);
                if (rCurrency.getLength() > 0)
                    pExport->AddAttribute(sAttrCurrency, rCurrency);
                bWasSetTypeAttribute = sal_True;
            }

            if (bExportValue)
            {
                rtl::OUString sValue( ::rtl::math::doubleToUString( rValue,
                            rtl_math_StringFormat_Automatic,
                            rtl_math_DecimalPlaces_Max, '.', sal_True));
                pExport->AddAttribute(sAttrValue, sValue);
            }
        }
        break;
    case util::NumberFormat::DATE:
    case util::NumberFormat::DATETIME:
        {
            if (!bWasSetTypeAttribute)
            {
                pExport->AddAttribute(sAttrValueType, XML_DATE);
                bWasSetTypeAttribute = sal_True;
            }
            if (bExportValue)
            {
                if ( pExport->SetNullDateOnUnitConverter() )
                {
                    rtl::OUStringBuffer sBuffer;
                    pExport->GetMM100UnitConverter().convertDateTime(sBuffer, rValue);
                    pExport->AddAttribute(sAttrDateValue, sBuffer.makeStringAndClear());
                }
            }
        }
        break;
    case util::NumberFormat::TIME:
        {
            if (!bWasSetTypeAttribute)
            {
                pExport->AddAttribute(sAttrValueType, XML_TIME);
                bWasSetTypeAttribute = sal_True;
            }
            if (bExportValue)
            {
                rtl::OUStringBuffer sBuffer;
                pExport->GetMM100UnitConverter().convertTime(sBuffer, rValue);
                pExport->AddAttribute(sAttrTimeValue, sBuffer.makeStringAndClear());
            }
        }
        break;
    case util::NumberFormat::LOGICAL:
        {
            if (!bWasSetTypeAttribute)
            {
                pExport->AddAttribute(sAttrValueType, XML_BOOLEAN);
                bWasSetTypeAttribute = sal_True;
            }
            if (bExportValue)
            {
                double fTempValue = rValue;
                if (::rtl::math::approxEqual( fTempValue, 1.0 ))
                {
                    pExport->AddAttribute(sAttrBooleanValue, XML_TRUE);
                }
                else
                {
                    if (::rtl::math::approxEqual( rValue, 0.0 ))
                    {
                        pExport->AddAttribute(sAttrBooleanValue, XML_FALSE);
                    }
                    else
                    {
                        rtl::OUString sValue( ::rtl::math::doubleToUString(
                                    fTempValue,
                                    rtl_math_StringFormat_Automatic,
                                    rtl_math_DecimalPlaces_Max, '.',
                                    sal_True));
                        pExport->AddAttribute(sAttrBooleanValue, sValue);
                    }
                }
            }
        }
        break;
    case util::NumberFormat::TEXT:
        {
            if (!bWasSetTypeAttribute)
            {
                pExport->AddAttribute(sAttrValueType, XML_FLOAT);
                bWasSetTypeAttribute = sal_True;
                if (bExportValue)
                {
                    rtl::OUString sValue( ::rtl::math::doubleToUString( rValue,
                                rtl_math_StringFormat_Automatic,
                                rtl_math_DecimalPlaces_Max, '.', sal_True));
                    pExport->AddAttribute(sAttrValue, sValue);
                }
            }
        }
        break;
    }
}

void XMLNumberFormatAttributesExportHelper::SetNumberFormatAttributes(
    const sal_Int32 nNumberFormat, const double& rValue, sal_Bool bExportValue)
{
    if (pExport)
    {
        sal_Bool bIsStandard;
        rtl::OUString sCurrency;
        sal_Int16 nTypeKey = GetCellType(nNumberFormat, sCurrency, bIsStandard);
        WriteAttributes(nTypeKey, rValue, sCurrency, bExportValue);
    }
    else {
        OSL_FAIL("no SvXMLExport given");
    }
}

void XMLNumberFormatAttributesExportHelper::SetNumberFormatAttributes(
    const rtl::OUString& rValue, const rtl::OUString& rCharacters,
    sal_Bool bExportValue, sal_Bool bExportTypeAttribute)
{
    if (pExport)
    {
        if (bExportTypeAttribute)
            pExport->AddAttribute(sAttrValueType, XML_STRING);
        if (bExportValue && rValue.getLength() && (rValue != rCharacters))
            pExport->AddAttribute(sAttrStringValue, rValue);
    }
    else {
        OSL_FAIL("no SvXMLExport given");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
