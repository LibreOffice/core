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

#include "SchXMLDataTableContext.hxx"

#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmluconv.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XDataTable.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

using namespace ::xmloff::token;
using namespace css;

SchXMLDataTableContext::SchXMLDataTableContext(SchXMLImportHelper& rImpHelper, SvXMLImport& rImport)
    : SvXMLImportContext(rImport)
    , mrImportHelper(rImpHelper)
{
}

void SchXMLDataTableContext::startFastElement(
    sal_Int32 /*nElement*/, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    printf("SchXMLDataTableContext::startFastElement\n");
    auto xChartDocument = mrImportHelper.GetChartDocument();
    if (!xChartDocument.is())
        return;

    uno::Reference<chart2::XChartDocument> xNewChartDocument(xChartDocument, uno::UNO_QUERY);
    if (!xNewChartDocument.is())
        return;

    uno::Reference<chart2::XDiagram> xDiagram(xNewChartDocument->getFirstDiagram());
    if (!xDiagram.is())
        return;

    uno::Reference<lang::XMultiServiceFactory> xFactory = comphelper::getProcessServiceFactory();
    uno::Reference<chart2::XDataTable> xDataTable(
        xFactory->createInstance("com.sun.star.chart2.DataTable"), uno::UNO_QUERY);
    if (!xDataTable.is())
        return;

    xDiagram->setDataTable(xDataTable);

    OUString sAutoStyleName;

    for (auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        if (aIter.getToken() == XML_ELEMENT(CHART, XML_STYLE_NAME))
            sAutoStyleName = aIter.toString();
        else
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
    }

    // set properties
    uno::Reference<beans::XPropertySet> xPropertySet(xDataTable, uno::UNO_QUERY);

    if (!sAutoStyleName.isEmpty() && xPropertySet.is())
    {
        mrImportHelper.FillAutoStyle(sAutoStyleName, xPropertySet);
    }
}

SchXMLDataTableContext::~SchXMLDataTableContext() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
