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

#include <unotools/textsearch.hxx>
#include <com/sun/star/util/Date.hpp>
#include "importcontext.hxx"

namespace sax_fastparser { class FastAttributeList; }


class ScXMLCalculationSettingsContext : public ScXMLImportContext
{
    css::util::Date aNullDate;
    double fIterationEpsilon;
    sal_Int32 nIterationCount;
    sal_uInt16 nYear2000;
    utl::SearchParam::SearchType eSearchType;
    bool bIsIterationEnabled;
    bool bCalcAsShown;
    bool bIgnoreCase;
    bool bLookUpLabels;
    bool bMatchWholeCell;

public:
    ScXMLCalculationSettingsContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList);

    virtual ~ScXMLCalculationSettingsContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    void SetNullDate(const css::util::Date& aDate) { aNullDate = aDate; }
    void SetIterationStatus(const bool bValue) { bIsIterationEnabled = bValue; }
    void SetIterationCount(const sal_Int32 nValue) { nIterationCount = nValue; }
    void SetIterationEpsilon(const double fValue) { fIterationEpsilon = fValue; }
    virtual void SAL_CALL endFastElement( sal_Int32 Element ) override;
};

class ScXMLNullDateContext : public ScXMLImportContext
{
public:
    ScXMLNullDateContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList, ScXMLCalculationSettingsContext* pCalcSet);

    virtual ~ScXMLNullDateContext() override;
};

class ScXMLIterationContext : public ScXMLImportContext
{
public:
    ScXMLIterationContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList, ScXMLCalculationSettingsContext* pCalcSet);

    virtual ~ScXMLIterationContext() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
