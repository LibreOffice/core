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

#pragma once
#if 1

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include <sal/types.h>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

#include <set>

class SvXMLExport;
namespace rtl
{
    class OUString;
}

struct XMLNumberFormat
{
    rtl::OUString   sCurrency;
    sal_Int32       nNumberFormat;
    sal_Int16       nType;
    sal_Bool        bIsStandard : 1;
    XMLNumberFormat() : nNumberFormat(0), nType(0) {}
    XMLNumberFormat(const rtl::OUString& sTempCurrency, sal_Int32 nTempFormat,
        sal_Int16 nTempType) : sCurrency(sTempCurrency), nNumberFormat(nTempFormat),
            nType(nTempType) {}
};

struct LessNumberFormat
{
    sal_Bool operator() (const XMLNumberFormat& rValue1, const XMLNumberFormat& rValue2) const
    {
        return rValue1.nNumberFormat < rValue2.nNumberFormat;
    }
};

typedef std::set<XMLNumberFormat, LessNumberFormat> XMLNumberFormatSet;

class XMLOFF_DLLPUBLIC XMLNumberFormatAttributesExportHelper
{
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats > xNumberFormats;
    SvXMLExport*        pExport;
    const rtl::OUString sEmpty;
    const rtl::OUString sStandardFormat;
    const rtl::OUString sType;
    const rtl::OUString sAttrValueType;
    const rtl::OUString sAttrValue;
    const rtl::OUString sAttrDateValue;
    const rtl::OUString sAttrTimeValue;
    const rtl::OUString sAttrBooleanValue;
    const rtl::OUString sAttrStringValue;
    const rtl::OUString sAttrCurrency;
    const rtl::OUString msCurrencySymbol;
    const rtl::OUString msCurrencyAbbreviation;
    XMLNumberFormatSet  aNumberFormats;
public :
    XMLNumberFormatAttributesExportHelper(::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& xNumberFormatsSupplier);
    XMLNumberFormatAttributesExportHelper(::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& xNumberFormatsSupplier,
                                            SvXMLExport& rExport );
    ~XMLNumberFormatAttributesExportHelper();
    void SetExport(SvXMLExport* pExp) { this->pExport = pExp; }

    sal_Int16 GetCellType(const sal_Int32 nNumberFormat, rtl::OUString& sCurrency, bool& bIsStandard);
    static void WriteAttributes(SvXMLExport& rXMLExport,
                                const sal_Int16 nTypeKey,
                                const double& rValue,
                                const rtl::OUString& rCurrencySymbol,
                                sal_Bool bExportValue = sal_True);
    static sal_Bool GetCurrencySymbol(const sal_Int32 nNumberFormat, rtl::OUString& rCurrencySymbol,
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > & xNumberFormatsSupplier);
    static sal_Int16 GetCellType(const sal_Int32 nNumberFormat, sal_Bool& bIsStandard,
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > & xNumberFormatsSupplier);
    static void SetNumberFormatAttributes(SvXMLExport& rXMLExport,
                                          const sal_Int32 nNumberFormat,
                                          const double& rValue,
                                          sal_Bool bExportValue = sal_True);
    static void SetNumberFormatAttributes(SvXMLExport& rXMLExport,
                                          const rtl::OUString& rValue,
                                          const rtl::OUString& rCharacters,
                                          sal_Bool bExportValue = sal_True,
                                          sal_Bool bExportTypeAttribute = sal_True);

    sal_Bool GetCurrencySymbol(const sal_Int32 nNumberFormat, rtl::OUString& rCurrencySymbol);
    sal_Int16 GetCellType(const sal_Int32 nNumberFormat, bool& bIsStandard);
    void WriteAttributes(const sal_Int16 nTypeKey,
                                          const double& rValue,
                                          const rtl::OUString& rCurrencySymbol,
                                          sal_Bool bExportValue = sal_True);
    void SetNumberFormatAttributes(const sal_Int32 nNumberFormat,
                                          const double& rValue,
                                          sal_Bool bExportValue = sal_True);
    void SetNumberFormatAttributes(const rtl::OUString& rValue,
                                          const rtl::OUString& rCharacters,
                                          sal_Bool bExportValue = sal_True,
                                          sal_Bool bExportTypeAttribute = sal_True);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
