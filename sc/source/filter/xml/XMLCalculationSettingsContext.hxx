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

#ifndef INCLUDED_SC_SOURCE_FILTER_XML_XMLCALCULATIONSETTINGSCONTEXT_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_XMLCALCULATIONSETTINGSCONTEXT_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/util/Date.hpp>
#include "xmlimprt.hxx"


class ScXMLCalculationSettingsContext : public SvXMLImportContext
{
    com::sun::star::util::Date aNullDate;
    double fIterationEpsilon;
    sal_Int32 nIterationCount;
    sal_uInt16 nYear2000;
    bool bIsIterationEnabled;
    bool bCalcAsShown;
    bool bIgnoreCase;
    bool bLookUpLabels;
    bool bMatchWholeCell;
    bool bUseRegularExpressions;

    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }
public:
    ScXMLCalculationSettingsContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList);

    virtual ~ScXMLCalculationSettingsContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) override;

    void SetNullDate(const com::sun::star::util::Date& aDate) { aNullDate = aDate; }
    void SetIterationStatus(const bool bValue) { bIsIterationEnabled = bValue; }
    void SetIterationCount(const sal_Int32 nValue) { nIterationCount = nValue; }
    void SetIterationEpsilon(const double fValue) { fIterationEpsilon = fValue; }
    virtual void EndElement() override;
};

class ScXMLNullDateContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }
public:
    ScXMLNullDateContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList, ScXMLCalculationSettingsContext* pCalcSet);

    virtual ~ScXMLNullDateContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLIterationContext : public SvXMLImportContext
{
    const ScXMLImport& GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }
public:
    ScXMLIterationContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList, ScXMLCalculationSettingsContext* pCalcSet);

    virtual ~ScXMLIterationContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
