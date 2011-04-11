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
#ifndef SW_VBA_PAGESETUP_HXX
#define SW_VBA_PAGESETUP_HXX

#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/word/XPageSetup.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbapagesetupbase.hxx>

typedef cppu::ImplInheritanceHelper1< VbaPageSetupBase, ooo::vba::word::XPageSetup > SwVbaPageSetup_BASE;

class SwVbaPageSetup :  public SwVbaPageSetup_BASE
{
private:
    rtl::OUString getStyleOfFirstPage() throw (css::uno::RuntimeException);

public:
    SwVbaPageSetup( const css::uno::Reference< ooo::vba::XHelperInterface >& xParent,
                    const css::uno::Reference< css::uno::XComponentContext >& xContext,
                    const css::uno::Reference< css::frame::XModel >& xModel,
                    const css::uno::Reference< css::beans::XPropertySet >& xProps ) throw (css::uno::RuntimeException);
    virtual ~SwVbaPageSetup(){}

    // Attributes
    virtual double SAL_CALL getGutter() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setGutter( double _gutter ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getHeaderDistance() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setHeaderDistance( double _headerdistance ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getFooterDistance() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setFooterDistance( double _footerdistance ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getDifferentFirstPageHeaderFooter() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDifferentFirstPageHeaderFooter( sal_Bool status ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getSectionStart() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setSectionStart( ::sal_Int32 _sectionstart ) throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
