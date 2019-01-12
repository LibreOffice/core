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

#ifndef INCLUDED_XMLOFF_NUMEHELP_HXX
#define INCLUDED_XMLOFF_NUMEHELP_HXX

#include <sal/config.h>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/dllapi.h>
#include <sal/types.h>
#include <com/sun/star/uno/Reference.hxx>

#include <set>

namespace com { namespace sun { namespace star { namespace util { class XNumberFormats; } } } }
namespace com { namespace sun { namespace star { namespace util { class XNumberFormatsSupplier; } } } }

class SvXMLExport;

struct XMLNumberFormat
{
    OUString   sCurrency;
    sal_Int32 const  nNumberFormat;
    sal_Int16  nType;
    bool       bIsStandard : 1;

    XMLNumberFormat(sal_Int32 nTempFormat)
        : nNumberFormat(nTempFormat)
        , nType(0)
        , bIsStandard(false)
    {
    }
};

struct LessNumberFormat
{
    bool operator() (const XMLNumberFormat& rValue1, const XMLNumberFormat& rValue2) const
    {
        return rValue1.nNumberFormat < rValue2.nNumberFormat;
    }
};

typedef std::set<XMLNumberFormat, LessNumberFormat> XMLNumberFormatSet;

class XMLOFF_DLLPUBLIC XMLNumberFormatAttributesExportHelper
{
    css::uno::Reference< css::util::XNumberFormats > xNumberFormats;
    SvXMLExport*        pExport;
    const OUString sAttrValue;
    const OUString sAttrDateValue;
    const OUString sAttrTimeValue;
    const OUString sAttrBooleanValue;
    const OUString sAttrStringValue;
    const OUString sAttrCurrency;
    XMLNumberFormatSet  aNumberFormats;
public:
    XMLNumberFormatAttributesExportHelper(css::uno::Reference< css::util::XNumberFormatsSupplier > const & xNumberFormatsSupplier);
    XMLNumberFormatAttributesExportHelper(css::uno::Reference< css::util::XNumberFormatsSupplier > const & xNumberFormatsSupplier,
                                            SvXMLExport& rExport );
    ~XMLNumberFormatAttributesExportHelper();

    sal_Int16 GetCellType(const sal_Int32 nNumberFormat, OUString& sCurrency, bool& bIsStandard);
    static void WriteAttributes(SvXMLExport& rXMLExport,
                                const sal_Int16 nTypeKey,
                                const double& rValue,
                                const OUString& rCurrencySymbol,
                                bool bExportValue);
    static bool GetCurrencySymbol(const sal_Int32 nNumberFormat, OUString& rCurrencySymbol,
        css::uno::Reference< css::util::XNumberFormatsSupplier > const & xNumberFormatsSupplier);
    static sal_Int16 GetCellType(const sal_Int32 nNumberFormat, bool& bIsStandard,
        css::uno::Reference< css::util::XNumberFormatsSupplier > const & xNumberFormatsSupplier);
    static void SetNumberFormatAttributes(SvXMLExport& rXMLExport,
                                          const sal_Int32 nNumberFormat,
                                          const double& rValue,
                                          bool bExportValue = true);
    static void SetNumberFormatAttributes(SvXMLExport& rXMLExport,
                                          const OUString& rValue,
                                          const OUString& rCharacters,
                                          bool bExportValue,
                                          bool bExportTypeAttribute = true);

    bool GetCurrencySymbol(const sal_Int32 nNumberFormat, OUString& rCurrencySymbol);
    sal_Int16 GetCellType(const sal_Int32 nNumberFormat, bool& bIsStandard);
    void WriteAttributes(const sal_Int16 nTypeKey,
                                          const double& rValue,
                                          const OUString& rCurrencySymbol,
                                          bool bExportValue, sal_uInt16 nNamespace = XML_NAMESPACE_OFFICE);
    void SetNumberFormatAttributes(const sal_Int32 nNumberFormat,
                                          const double& rValue,
                                          bool bExportValue = true,
                                          sal_uInt16 nNamespace = XML_NAMESPACE_OFFICE, bool bExportCurrencySymbol = true);
    void SetNumberFormatAttributes(const OUString& rValue,
                                          const OUString& rCharacters,
                                          bool bExportValue = true,
                                          sal_uInt16 nNamespace = XML_NAMESPACE_OFFICE);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
