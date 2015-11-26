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

#ifndef INCLUDED_XMLOFF_SOURCE_STYLE_PAGEMASTEREXPORTPROPMAPPER_HXX
#define INCLUDED_XMLOFF_SOURCE_STYLE_PAGEMASTEREXPORTPROPMAPPER_HXX

#include <xmloff/xmlexppr.hxx>
#include "XMLBackgroundImageExport.hxx"
#include "XMLTextColumnsExport.hxx"
#include "XMLFootnoteSeparatorExport.hxx"

class XMLPageMasterExportPropMapper : public SvXMLExportPropertyMapper
{
protected:
    XMLBackgroundImageExport aBackgroundImageExport;
    XMLTextColumnsExport aTextColumnsExport;
    XMLFootnoteSeparatorExport aFootnoteSeparatorExport;

    virtual void        ContextFilter(
                            bool bEnableFoFontFamily,
                            ::std::vector< XMLPropertyState >& rProperties,
                            css::uno::Reference< css::beans::XPropertySet > rPropSet
                            ) const override;

public:
                        XMLPageMasterExportPropMapper(
                             const rtl::Reference< XMLPropertySetMapper >& rMapper,
                            SvXMLExport& rExport
                            );
    virtual             ~XMLPageMasterExportPropMapper();

    virtual void        handleElementItem(
                            SvXMLExport& rExport,
                            const XMLPropertyState& rProperty,
                            SvXmlExportFlags nFlags,
                            const ::std::vector< XMLPropertyState >* pProperties = nullptr,
                            sal_uInt32 nIdx = 0
                            ) const override;
    virtual void        handleSpecialItem(
                            SvXMLAttributeList& rAttrList,
                            const XMLPropertyState& rProperty,
                            const SvXMLUnitConverter& rUnitConverter,
                            const SvXMLNamespaceMap& rNamespaceMap,
                            const ::std::vector< XMLPropertyState >* pProperties = nullptr,
                            sal_uInt32 nIdx = 0
                            ) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
