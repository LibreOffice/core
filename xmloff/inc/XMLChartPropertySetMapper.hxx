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

#include <xmloff/xmlprmap.hxx>
#include <xmloff/xmlexppr.hxx>
#include <xmloff/xmlimppr.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/prhdlfac.hxx>

class SvXMLExport;

class XMLChartPropHdlFactory final : public XMLPropertyHandlerFactory
{
private:
    SvXMLExport const*const m_pExport;

public:
    XMLChartPropHdlFactory(SvXMLExport const*);
    virtual ~XMLChartPropHdlFactory() override;
    virtual const XMLPropertyHandler* GetPropertyHandler( sal_Int32 nType ) const override;
};

class XMLChartPropertySetMapper final : public XMLPropertySetMapper
{
public:
    explicit XMLChartPropertySetMapper(SvXMLExport const* pExport);
            virtual ~XMLChartPropertySetMapper() override;
};

class XMLChartExportPropertyMapper final : public SvXMLExportPropertyMapper
{
private:
    SvXMLExport& mrExport;
    css::uno::Reference< css::chart2::XChartDocument > mxChartDoc;

    virtual void ContextFilter(
        bool bEnableFoFontFamily,
        ::std::vector< XMLPropertyState >& rProperties,
        const css::uno::Reference<css::beans::XPropertySet >& rPropSet ) const override;

    /// this method is called for every item that has the MID_FLAG_ELEMENT_EXPORT flag set
    virtual void handleElementItem(
        SvXMLExport& rExport,
        const XMLPropertyState& rProperty, SvXmlExportFlags nFlags,
        const ::std::vector< XMLPropertyState > *pProperties,
        sal_uInt32 nIdx  ) const override;

    /// this method is called for every item that has the MID_FLAG_SPECIAL_ITEM_EXPORT flag set
    virtual void handleSpecialItem(
        comphelper::AttributeList& rAttrList, const XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter, const SvXMLNamespaceMap& rNamespaceMap,
        const ::std::vector< XMLPropertyState > *pProperties,
        sal_uInt32 nIdx  ) const override;

public:
    XMLChartExportPropertyMapper( const rtl::Reference< XMLPropertySetMapper >& rMapper,
                                  SvXMLExport& rExport );
    virtual ~XMLChartExportPropertyMapper() override;

    void setChartDoc( const css::uno::Reference< css::chart2::XChartDocument >& xChartDoc );
};

class XMLChartImportPropertyMapper final : public SvXMLImportPropertyMapper
{
private:
    SvXMLImport& mrImport;

public:
    XMLChartImportPropertyMapper( const rtl::Reference< XMLPropertySetMapper >& rMapper,
                                  const SvXMLImport& rImport );
    virtual ~XMLChartImportPropertyMapper() override;

    virtual bool handleSpecialItem(
        XMLPropertyState& rProperty,
        ::std::vector< XMLPropertyState >& rProperties,
        const OUString& rValue,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap ) const override;

    virtual void finished(std::vector<XMLPropertyState>& rProperties, sal_Int32 nStartIndex,
                          sal_Int32 nEndIndex, const sal_uInt32 nPropType) const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
