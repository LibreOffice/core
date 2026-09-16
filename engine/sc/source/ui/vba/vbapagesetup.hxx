/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <cpo/uno/XComponentContext.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <vbahelper/vbapagesetupbase.hxx>

typedef cppu::ImplInheritanceHelper<VbaPageSetupBase, ov::excel::XPageSetup> ScVbaPageSetup_BASE;

class ScVbaPageSetup : public ScVbaPageSetup_BASE
{
    cpo::uno::Reference<css::frame::XModel> mxModel;
    cpo::uno::Reference<css::sheet::XSpreadsheet> mxSheet;
    bool mbIsLandscape;

public:
    /// @throws cpo::uno::RuntimeException
    ScVbaPageSetup(const cpo::uno::Reference<ov::XHelperInterface>& xParent,
                   const cpo::uno::Reference<cpo::uno::XComponentContext>& xContext,
                   cpo::uno::Reference<css::sheet::XSpreadsheet> xSheet,
                   const cpo::uno::Reference<css::frame::XModel>& xModel);

    // Attribute
    virtual OUString getPrintArea() override;
    virtual void setPrintArea(const OUString& rAreas) override;
    virtual double getHeaderMargin() override;
    void setHeaderMargin(double margin) override;
    double getFooterMargin() override;
    void setFooterMargin(double margin) override;
    virtual cpo::uno::Any getFitToPagesTall() override;
    virtual void setFitToPagesTall(const cpo::uno::Any& fitToPagesTall) override;
    virtual cpo::uno::Any getFitToPagesWide() override;
    virtual void setFitToPagesWide(const cpo::uno::Any& fitToPagesWide) override;
    virtual cpo::uno::Any getZoom() override;
    virtual void setZoom(const cpo::uno::Any& zoom) override;
    virtual OUString getLeftHeader() override;
    virtual void setLeftHeader(const OUString& leftHeader) override;
    virtual OUString getCenterHeader() override;
    virtual void setCenterHeader(const OUString& centerHeader) override;
    virtual OUString getRightHeader() override;
    virtual void setRightHeader(const OUString& rightHeader) override;
    virtual OUString getLeftFooter() override;
    virtual void setLeftFooter(const OUString& leftFooter) override;
    virtual OUString getCenterFooter() override;
    virtual void setCenterFooter(const OUString& centerFooter) override;
    virtual OUString getRightFooter() override;
    virtual void setRightFooter(const OUString& rightFooter) override;
    virtual sal_Int32 getOrder() override;
    virtual void setOrder(sal_Int32 order) override;
    virtual sal_Int32 getFirstPageNumber() override;
    virtual void setFirstPageNumber(sal_Int32 firstPageNumber) override;
    virtual bool getCenterVertically() override;
    virtual void setCenterVertically(bool centerVertically) override;
    virtual bool getCenterHorizontally() override;
    virtual void setCenterHorizontally(bool centerHorizontally) override;
    virtual bool getPrintHeadings() override;
    virtual void setPrintHeadings(bool printHeadings) override;

    virtual bool getPrintGridlines() override;
    virtual void setPrintGridlines(bool _printgridlines) override;
    virtual OUString getPrintTitleRows() override;
    virtual void setPrintTitleRows(const OUString& _printtitlerows) override;
    virtual OUString getPrintTitleColumns() override;
    virtual void setPrintTitleColumns(const OUString& _printtitlecolumns) override;
    virtual sal_Int32 getPaperSize() override;
    virtual void setPaperSize(sal_Int32 papersize) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
