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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBAPAGESETUP_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBAPAGESETUP_HXX

#include <cppuhelper/implbase.hxx>
#include <ooo/vba/excel/XPageSetup.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbapagesetupbase.hxx>
#include <ooo/vba/excel/XlPaperSize.hpp>

typedef cppu::ImplInheritanceHelper< VbaPageSetupBase, ov::excel::XPageSetup > ScVbaPageSetup_BASE;

class ScVbaPageSetup :  public ScVbaPageSetup_BASE
{
    css::uno::Reference< css::sheet::XSpreadsheet > mxSheet;
    bool mbIsLandscape;
public:
    ScVbaPageSetup( const css::uno::Reference< ov::XHelperInterface >& xParent,
                    const css::uno::Reference< css::uno::XComponentContext >& xContext,
                    const css::uno::Reference< css::sheet::XSpreadsheet>& xSheet,
                    const css::uno::Reference< css::frame::XModel >& xModel) throw (css::uno::RuntimeException);
    virtual ~ScVbaPageSetup(){}

    // Attribute
    virtual OUString SAL_CALL getPrintArea() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPrintArea( const OUString& rAreas ) throw (css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getHeaderMargin() throw (css::uno::RuntimeException) override;
    void SAL_CALL setHeaderMargin( double margin ) throw (css::uno::RuntimeException) override;
    double SAL_CALL getFooterMargin() throw (css::uno::RuntimeException) override;
    void SAL_CALL setFooterMargin( double margin ) throw (css::uno::RuntimeException) override;
    virtual css::uno::Any SAL_CALL getFitToPagesTall() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setFitToPagesTall( const css::uno::Any& fitToPagesTall ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getFitToPagesWide() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setFitToPagesWide( const css::uno::Any& fitToPagesWide ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getZoom() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setZoom( const css::uno::Any& zoom ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getLeftHeader() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLeftHeader( const OUString& leftHeader ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getCenterHeader() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setCenterHeader( const OUString& centerHeader ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getRightHeader() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setRightHeader( const OUString& rightHeader ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getLeftFooter() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLeftFooter( const OUString& leftFooter ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getCenterFooter() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setCenterFooter( const OUString& centerFooter ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getRightFooter() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setRightFooter( const OUString& rightFooter ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getOrder() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setOrder( sal_Int32 order ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getFirstPageNumber() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setFirstPageNumber( sal_Int32 firstPageNumber ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getCenterVertically() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setCenterVertically( sal_Bool centerVertically ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getCenterHorizontally() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setCenterHorizontally( sal_Bool centerHorizontally ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getPrintHeadings() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPrintHeadings( sal_Bool printHeadings ) throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL getPrintGridlines() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPrintGridlines( sal_Bool _printgridlines ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::rtl::OUString SAL_CALL getPrintTitleRows() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPrintTitleRows( const ::rtl::OUString& _printtitlerows ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::rtl::OUString SAL_CALL getPrintTitleColumns() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPrintTitleColumns( const ::rtl::OUString& _printtitlecolumns ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getPaperSize() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPaperSize( sal_Int32 papersize ) throw (css::uno::RuntimeException, std::exception) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
