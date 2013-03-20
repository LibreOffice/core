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

#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/excel/XPageSetup.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbapagesetupbase.hxx>

typedef cppu::ImplInheritanceHelper1< VbaPageSetupBase, ov::excel::XPageSetup > ScVbaPageSetup_BASE;

class ScVbaPageSetup :  public ScVbaPageSetup_BASE
{
    css::uno::Reference< css::sheet::XSpreadsheet > mxSheet;
public:
    ScVbaPageSetup( const css::uno::Reference< ov::XHelperInterface >& xParent,
                    const css::uno::Reference< css::uno::XComponentContext >& xContext,
                    const css::uno::Reference< css::sheet::XSpreadsheet>& xSheet,
                    const css::uno::Reference< css::frame::XModel >& xModel) throw (css::uno::RuntimeException);
    virtual ~ScVbaPageSetup(){}

    // Attribute
    virtual rtl::OUString SAL_CALL getPrintArea() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setPrintArea( const rtl::OUString& rAreas ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getHeaderMargin() throw (css::uno::RuntimeException);
    void SAL_CALL setHeaderMargin( double margin ) throw (css::uno::RuntimeException);
    double SAL_CALL getFooterMargin() throw (css::uno::RuntimeException);
    void SAL_CALL setFooterMargin( double margin ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getFitToPagesTall() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setFitToPagesTall( const css::uno::Any& fitToPagesTall ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getFitToPagesWide() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setFitToPagesWide( const css::uno::Any& fitToPagesWide ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getZoom() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setZoom( const css::uno::Any& zoom ) throw (css::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getLeftHeader() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLeftHeader( const rtl::OUString& leftHeader ) throw (css::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getCenterHeader() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setCenterHeader( const rtl::OUString& centerHeader ) throw (css::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getRightHeader() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRightHeader( const rtl::OUString& rightHeader ) throw (css::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getLeftFooter() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLeftFooter( const rtl::OUString& leftFooter ) throw (css::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getCenterFooter() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setCenterFooter( const rtl::OUString& centerFooter ) throw (css::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getRightFooter() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRightFooter( const rtl::OUString& rightFooter ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getOrder() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setOrder( sal_Int32 order ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getFirstPageNumber() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setFirstPageNumber( sal_Int32 firstPageNumber ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getCenterVertically() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setCenterVertically( sal_Bool centerVertically ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getCenterHorizontally() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setCenterHorizontally( sal_Bool centerHorizontally ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getPrintHeadings() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setPrintHeadings( sal_Bool printHeadings ) throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
