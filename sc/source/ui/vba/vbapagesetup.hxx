/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef SC_VBA_PAGESETUP_HXX
#define SC_VBA_PAGESETUP_HXX

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
