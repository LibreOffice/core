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


#include <xmloff/numehelp.hxx>

#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlexp.hxx>
#include <com/sun/star/uno/Reference.h>
#include <rtl/ustring.hxx>
#include <svl/zforlist.hxx>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <sax/tools/converter.hxx>
#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>

using namespace com::sun::star;
using namespace xmloff::token;

static const OUStringLiteral gsStandardFormat("StandardFormat");
static const OUStringLiteral gsType("Type");
static const OUStringLiteral gsCurrencySymbol("CurrencySymbol");
static const OUStringLiteral gsCurrencyAbbreviation("CurrencyAbbreviation");

XMLNumberFormatAttributesExportHelper::XMLNumberFormatAttributesExportHelper(
            css::uno::Reference< css::util::XNumberFormatsSupplier > const & xTempNumberFormatsSupplier)
    : xNumberFormats(xTempNumberFormatsSupplier.is() ? xTempNumberFormatsSupplier->getNumberFormats() : css::uno::Reference< css::util::XNumberFormats > ()),
    pExport(nullptr),
    aNumberFormats()
{
}

XMLNumberFormatAttributesExportHelper::XMLNumberFormatAttributesExportHelper(
            css::uno::Reference< css::util::XNumberFormatsSupplier > const & xTempNumberFormatsSupplier,
            SvXMLExport& rTempExport )
:   xNumberFormats(xTempNumberFormatsSupplier.is() ? xTempNumberFormatsSupplier->getNumberFormats() : css::uno::Reference< css::util::XNumberFormats > ()),
    pExport(&rTempExport),
    sAttrValue(rTempExport.GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_OFFICE, GetXMLToken(XML_VALUE))),
    sAttrDateValue(rTempExport.GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_OFFICE, GetXMLToken(XML_DATE_VALUE))),
    sAttrTimeValue(rTempExport.GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_OFFICE, GetXMLToken(XML_TIME_VALUE))),
    sAttrBooleanValue(rTempExport.GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_OFFICE, GetXMLToken(XML_BOOLEAN_VALUE))),
    sAttrStringValue(rTempExport.GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_OFFICE, GetXMLToken(XML_STRING_VALUE))),
    sAttrCurrency(rTempExport.GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_OFFICE, GetXMLToken(XML_CURRENCY))),
    aNumberFormats()
{
}

XMLNumberFormatAttributesExportHelper::~XMLNumberFormatAttributesExportHelper()
{
}

sal_Int16 XMLNumberFormatAttributesExportHelper::GetCellType(const sal_Int32 nNumberFormat, OUString& sCurrency, bool& bIsStandard)
{
    XMLNumberFormat aFormat(nNumberFormat);
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
                                const OUString& rCurrency,
                                bool bExportValue)
{
    bool bWasSetTypeAttribute = false;
    switch(nTypeKey & ~util::NumberFormat::DEFINED)
    {
    case 0:
    case util::NumberFormat::NUMBER:
    case util::NumberFormat::SCIENTIFIC:
    case util::NumberFormat::FRACTION:
        {
            rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_FLOAT);
            bWasSetTypeAttribute = true;
            [[fallthrough]];
        }
    case util::NumberFormat::PERCENT:
        {
            if (!bWasSetTypeAttribute)
            {
                rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_PERCENTAGE);
                bWasSetTypeAttribute = true;
            }
            [[fallthrough]];
        }
    case util::NumberFormat::CURRENCY:
        {
            if (!bWasSetTypeAttribute)
            {
                rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_CURRENCY);
                if (!rCurrency.isEmpty())
                    rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_CURRENCY, rCurrency);
            }

            if (bExportValue)
            {
                OUString sValue( ::rtl::math::doubleToUString( rValue,
                            rtl_math_StringFormat_Automatic,
                            rtl_math_DecimalPlaces_Max, '.', true));
                rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE, sValue);
            }
        }
        break;
    case util::NumberFormat::DATE:
    case util::NumberFormat::DATETIME:
        {
            rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_DATE);
            if (bExportValue)
            {
                if ( rXMLExport.SetNullDateOnUnitConverter() )
                {
                    OUStringBuffer sBuffer;
                    rXMLExport.GetMM100UnitConverter().convertDateTime(sBuffer, rValue);
                    rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_DATE_VALUE, sBuffer.makeStringAndClear());
                }
            }
        }
        break;
    case util::NumberFormat::TIME:
        {
            rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_TIME);
            if (bExportValue)
            {
                OUStringBuffer sBuffer;
                ::sax::Converter::convertDuration(sBuffer, rValue);
                rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_TIME_VALUE, sBuffer.makeStringAndClear());
            }
        }
        break;
    case util::NumberFormat::LOGICAL:
        {
            rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_BOOLEAN);
            if (bExportValue)
            {
                double fTempValue = rValue;
                if (::rtl::math::approxEqual( fTempValue, 1.0 ))
                {
                    rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_BOOLEAN_VALUE, XML_TRUE);
                }
                else
                {
                    if (rValue == 0.0)
                    {
                        rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_BOOLEAN_VALUE, XML_FALSE);
                    }
                    else
                    {
                        OUString sValue( ::rtl::math::doubleToUString(
                                    fTempValue,
                                    rtl_math_StringFormat_Automatic,
                                    rtl_math_DecimalPlaces_Max, '.',
                                    true));
                        rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_BOOLEAN_VALUE, sValue);
                    }
                }
            }
        }
        break;
    case util::NumberFormat::TEXT:
        {
            rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_FLOAT);
            if (bExportValue)
            {
                OUString sValue( ::rtl::math::doubleToUString( rValue,
                            rtl_math_StringFormat_Automatic,
                            rtl_math_DecimalPlaces_Max, '.', true));
                rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE, sValue);
            }
        }
        break;
    }
}

bool XMLNumberFormatAttributesExportHelper::GetCurrencySymbol(const sal_Int32 nNumberFormat, OUString& sCurrencySymbol,
    uno::Reference <util::XNumberFormatsSupplier> const & xNumberFormatsSupplier)
{
    if (xNumberFormatsSupplier.is())
    {
        uno::Reference <util::XNumberFormats> xNumberFormats(xNumberFormatsSupplier->getNumberFormats());
        if (xNumberFormats.is())
        {
            try
            {
                uno::Reference <beans::XPropertySet> xNumberPropertySet(xNumberFormats->getByKey(nNumberFormat));
                if ( xNumberPropertySet->getPropertyValue(gsCurrencySymbol) >>= sCurrencySymbol)
                {
                    OUString sCurrencyAbbreviation;
                    if ( xNumberPropertySet->getPropertyValue(gsCurrencyAbbreviation) >>= sCurrencyAbbreviation)
                    {
                        if ( !sCurrencyAbbreviation.isEmpty())
                            sCurrencySymbol = sCurrencyAbbreviation;
                        else
                        {
                            if ( sCurrencySymbol.getLength() == 1 && sCurrencySymbol.toChar() == NfCurrencyEntry::GetEuroSymbol() )
                                sCurrencySymbol = "EUR";
                        }
                    }
                    return true;
                }
            }
            catch ( uno::Exception& )
            {
                OSL_FAIL("Numberformat not found");
            }
        }
    }
    return false;
}


sal_Int16 XMLNumberFormatAttributesExportHelper::GetCellType(const sal_Int32 nNumberFormat, bool& bIsStandard,
    uno::Reference <util::XNumberFormatsSupplier> const & xNumberFormatsSupplier)
{
    if (xNumberFormatsSupplier.is())
    {
        uno::Reference <util::XNumberFormats> xNumberFormats(xNumberFormatsSupplier->getNumberFormats());
        if (xNumberFormats.is())
        {
            try
            {
                uno::Reference <beans::XPropertySet> xNumberPropertySet(xNumberFormats->getByKey(nNumberFormat));
                xNumberPropertySet->getPropertyValue(gsStandardFormat) >>= bIsStandard;
                sal_Int16 nNumberType = sal_Int16();
                if ( xNumberPropertySet->getPropertyValue(gsType) >>= nNumberType )
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
    const sal_Int32 nNumberFormat, const double& rValue, bool bExportValue)
{
    bool bIsStandard;
    sal_Int16 nTypeKey = GetCellType(nNumberFormat, bIsStandard, rXMLExport.GetNumberFormatsSupplier());
    OUString sCurrency;
    if ((nTypeKey & ~util::NumberFormat::DEFINED) == util::NumberFormat::CURRENCY)
        GetCurrencySymbol(nNumberFormat, sCurrency, rXMLExport.GetNumberFormatsSupplier());
    WriteAttributes(rXMLExport, nTypeKey, rValue, sCurrency, bExportValue);
}

void XMLNumberFormatAttributesExportHelper::SetNumberFormatAttributes(SvXMLExport& rXMLExport,
    const OUString& rValue, const OUString& rCharacters,
    bool bExportValue, bool bExportTypeAttribute)
{
    if (bExportTypeAttribute)
        rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_STRING);
    if (bExportValue && !rValue.isEmpty() && (rValue != rCharacters))
        rXMLExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_STRING_VALUE, rValue);
}

bool XMLNumberFormatAttributesExportHelper::GetCurrencySymbol(const sal_Int32 nNumberFormat, OUString& rCurrencySymbol)
{
    if (!xNumberFormats.is() && pExport && pExport->GetNumberFormatsSupplier().is())
        xNumberFormats.set(pExport->GetNumberFormatsSupplier()->getNumberFormats());

    if (xNumberFormats.is())
    {
        try
        {
            uno::Reference <beans::XPropertySet> xNumberPropertySet(xNumberFormats->getByKey(nNumberFormat));
            if ( xNumberPropertySet->getPropertyValue(gsCurrencySymbol) >>= rCurrencySymbol)
            {
                OUString sCurrencyAbbreviation;
                if ( xNumberPropertySet->getPropertyValue(gsCurrencyAbbreviation) >>= sCurrencyAbbreviation)
                {
                    if ( !sCurrencyAbbreviation.isEmpty())
                        rCurrencySymbol = sCurrencyAbbreviation;
                    else
                    {
                        if ( rCurrencySymbol.getLength() == 1 && rCurrencySymbol.toChar() == NfCurrencyEntry::GetEuroSymbol() )
                            rCurrencySymbol = "EUR";
                    }
                }
                return true;
            }
        }
        catch ( uno::Exception& )
        {
            OSL_FAIL("Numberformat not found");
        }
    }
    return false;
}

sal_Int16 XMLNumberFormatAttributesExportHelper::GetCellType(const sal_Int32 nNumberFormat, bool& bIsStandard)
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
                xNumberPropertySet->getPropertyValue(gsStandardFormat) >>= bIsStandard;
                sal_Int16 nNumberType = sal_Int16();
                if ( xNumberPropertySet->getPropertyValue(gsType) >>= nNumberType )
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
                                const OUString& rCurrency,
                                bool bExportValue, sal_uInt16 nNamespace)
{
    if (!pExport)
        return;

    bool bWasSetTypeAttribute = false;
    OUString sAttrValType = pExport->GetNamespaceMap().GetQNameByKey( nNamespace, GetXMLToken(XML_VALUE_TYPE));
    switch(nTypeKey & ~util::NumberFormat::DEFINED)
    {
    case 0:
    case util::NumberFormat::NUMBER:
    case util::NumberFormat::SCIENTIFIC:
    case util::NumberFormat::FRACTION:
        {
            pExport->AddAttribute(sAttrValType, XML_FLOAT);
            bWasSetTypeAttribute = true;
            [[fallthrough]];
        }
    case util::NumberFormat::PERCENT:
        {
            if (!bWasSetTypeAttribute)
            {
                pExport->AddAttribute(sAttrValType, XML_PERCENTAGE);
                bWasSetTypeAttribute = true;
            }
            [[fallthrough]];
        }
    case util::NumberFormat::CURRENCY:
        {
            if (!bWasSetTypeAttribute)
            {
                pExport->AddAttribute(sAttrValType, XML_CURRENCY);
                if (!rCurrency.isEmpty())
                    pExport->AddAttribute(sAttrCurrency, rCurrency);
            }

            if (bExportValue)
            {
                OUString sValue( ::rtl::math::doubleToUString( rValue,
                            rtl_math_StringFormat_Automatic,
                            rtl_math_DecimalPlaces_Max, '.', true));
                pExport->AddAttribute(sAttrValue, sValue);
            }
        }
        break;
    case util::NumberFormat::DATE:
    case util::NumberFormat::DATETIME:
        {
            pExport->AddAttribute(sAttrValType, XML_DATE);
            if (bExportValue)
            {
                if ( pExport->SetNullDateOnUnitConverter() )
                {
                    OUStringBuffer sBuffer;
                    pExport->GetMM100UnitConverter().convertDateTime(sBuffer, rValue);
                    pExport->AddAttribute(sAttrDateValue, sBuffer.makeStringAndClear());
                }
            }
        }
        break;
    case util::NumberFormat::TIME:
        {
            pExport->AddAttribute(sAttrValType, XML_TIME);
            if (bExportValue)
            {
                OUStringBuffer sBuffer;
                ::sax::Converter::convertDuration(sBuffer, rValue);
                pExport->AddAttribute(sAttrTimeValue, sBuffer.makeStringAndClear());
            }
        }
        break;
    case util::NumberFormat::LOGICAL:
        {
            pExport->AddAttribute(sAttrValType, XML_BOOLEAN);
            if (bExportValue)
            {
                double fTempValue = rValue;
                if (::rtl::math::approxEqual( fTempValue, 1.0 ))
                {
                    pExport->AddAttribute(sAttrBooleanValue, XML_TRUE);
                }
                else
                {
                    if (rValue == 0.0)
                    {
                        pExport->AddAttribute(sAttrBooleanValue, XML_FALSE);
                    }
                    else
                    {
                        OUString sValue( ::rtl::math::doubleToUString(
                                    fTempValue,
                                    rtl_math_StringFormat_Automatic,
                                    rtl_math_DecimalPlaces_Max, '.',
                                    true));
                        pExport->AddAttribute(sAttrBooleanValue, sValue);
                    }
                }
            }
        }
        break;
    case util::NumberFormat::TEXT:
        {
            pExport->AddAttribute(sAttrValType, XML_FLOAT);
            if (bExportValue)
            {
                OUString sValue( ::rtl::math::doubleToUString( rValue,
                            rtl_math_StringFormat_Automatic,
                            rtl_math_DecimalPlaces_Max, '.', true));
                pExport->AddAttribute(sAttrValue, sValue);
            }
        }
        break;
    }
}

void XMLNumberFormatAttributesExportHelper::SetNumberFormatAttributes(
    const sal_Int32 nNumberFormat, const double& rValue, bool bExportValue,
    sal_uInt16 nNamespace, bool bExportCurrencySymbol)
{
    if (pExport)
    {
        bool bIsStandard;
        OUString sCurrency;
        sal_Int16 nTypeKey = GetCellType(nNumberFormat, sCurrency, bIsStandard);
        if(!bExportCurrencySymbol)
            sCurrency.clear();

        WriteAttributes(nTypeKey, rValue, sCurrency, bExportValue, nNamespace);
    }
    else {
        OSL_FAIL("no SvXMLExport given");
    }
}

void XMLNumberFormatAttributesExportHelper::SetNumberFormatAttributes(
    const OUString& rValue, const OUString& rCharacters,
    bool bExportValue,
    sal_uInt16 nNamespace)
{
    if (pExport)
    {
        pExport->AddAttribute(nNamespace, XML_VALUE_TYPE, XML_STRING);
        if (bExportValue && !rValue.isEmpty() && (rValue != rCharacters))
            pExport->AddAttribute(sAttrStringValue, rValue);
    }
    else {
        OSL_FAIL("no SvXMLExport given");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
