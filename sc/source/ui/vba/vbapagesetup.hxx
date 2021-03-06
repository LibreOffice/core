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

#include <cppuhelper/implbase.hxx>
#include <ooo/vba/excel/XPageSetup.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <vbahelper/vbapagesetupbase.hxx>

typedef cppu::ImplInheritanceHelper<VbaPageSetupBase, ov::excel::XPageSetup> ScVbaPageSetup_BASE;

class ScVbaPageSetup : public ScVbaPageSetup_BASE
{
    css::uno::Reference<css::sheet::XSpreadsheet> mxSheet;
    bool mbIsLandscape;

public:
    /// @throws css::uno::RuntimeException
    ScVbaPageSetup(const css::uno::Reference<ov::XHelperInterface>& xParent,
                   const css::uno::Reference<css::uno::XComponentContext>& xContext,
                   const css::uno::Reference<css::sheet::XSpreadsheet>& xSheet,
                   const css::uno::Reference<css::frame::XModel>& xModel);

    // Attribute
    virtual OUString SAL_CALL getPrintArea() override;
    virtual void SAL_CALL setPrintArea(const OUString& rAreas) override;
    virtual double SAL_CALL getHeaderMargin() override;
    void SAL_CALL setHeaderMargin(double margin) override;
    double SAL_CALL getFooterMargin() override;
    void SAL_CALL setFooterMargin(double margin) override;
    virtual css::uno::Any SAL_CALL getFitToPagesTall() override;
    virtual void SAL_CALL setFitToPagesTall(const css::uno::Any& fitToPagesTall) override;
    virtual css::uno::Any SAL_CALL getFitToPagesWide() override;
    virtual void SAL_CALL setFitToPagesWide(const css::uno::Any& fitToPagesWide) override;
    virtual css::uno::Any SAL_CALL getZoom() override;
    virtual void SAL_CALL setZoom(const css::uno::Any& zoom) override;
    virtual OUString SAL_CALL getLeftHeader() override;
    virtual void SAL_CALL setLeftHeader(const OUString& leftHeader) override;
    virtual OUString SAL_CALL getCenterHeader() override;
    virtual void SAL_CALL setCenterHeader(const OUString& centerHeader) override;
    virtual OUString SAL_CALL getRightHeader() override;
    virtual void SAL_CALL setRightHeader(const OUString& rightHeader) override;
    virtual OUString SAL_CALL getLeftFooter() override;
    virtual void SAL_CALL setLeftFooter(const OUString& leftFooter) override;
    virtual OUString SAL_CALL getCenterFooter() override;
    virtual void SAL_CALL setCenterFooter(const OUString& centerFooter) override;
    virtual OUString SAL_CALL getRightFooter() override;
    virtual void SAL_CALL setRightFooter(const OUString& rightFooter) override;
    virtual sal_Int32 SAL_CALL getOrder() override;
    virtual void SAL_CALL setOrder(sal_Int32 order) override;
    virtual sal_Int32 SAL_CALL getFirstPageNumber() override;
    virtual void SAL_CALL setFirstPageNumber(sal_Int32 firstPageNumber) override;
    virtual sal_Bool SAL_CALL getCenterVertically() override;
    virtual void SAL_CALL setCenterVertically(sal_Bool centerVertically) override;
    virtual sal_Bool SAL_CALL getCenterHorizontally() override;
    virtual void SAL_CALL setCenterHorizontally(sal_Bool centerHorizontally) override;
    virtual sal_Bool SAL_CALL getPrintHeadings() override;
    virtual void SAL_CALL setPrintHeadings(sal_Bool printHeadings) override;

    virtual sal_Bool SAL_CALL getPrintGridlines() override;
    virtual void SAL_CALL setPrintGridlines(sal_Bool _printgridlines) override;
    virtual OUString SAL_CALL getPrintTitleRows() override;
    virtual void SAL_CALL setPrintTitleRows(const OUString& _printtitlerows) override;
    virtual OUString SAL_CALL getPrintTitleColumns() override;
    virtual void SAL_CALL setPrintTitleColumns(const OUString& _printtitlecolumns) override;
    virtual sal_Int32 SAL_CALL getPaperSize() override;
    virtual void SAL_CALL setPaperSize(sal_Int32 papersize) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
