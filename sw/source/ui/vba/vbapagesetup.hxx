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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBAPAGESETUP_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBAPAGESETUP_HXX

#include <cppuhelper/implbase.hxx>
#include <ooo/vba/word/XPageSetup.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbapagesetupbase.hxx>

typedef cppu::ImplInheritanceHelper< VbaPageSetupBase, ooo::vba::word::XPageSetup > SwVbaPageSetup_BASE;

class SwVbaPageSetup :  public SwVbaPageSetup_BASE
{
private:
    OUString getStyleOfFirstPage() throw (css::uno::RuntimeException);

public:
    SwVbaPageSetup( const css::uno::Reference< ooo::vba::XHelperInterface >& xParent,
                    const css::uno::Reference< css::uno::XComponentContext >& xContext,
                    const css::uno::Reference< css::frame::XModel >& xModel,
                    const css::uno::Reference< css::beans::XPropertySet >& xProps ) throw (css::uno::RuntimeException);
    virtual ~SwVbaPageSetup(){}

    // Attributes
    virtual double SAL_CALL getGutter() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setGutter( double _gutter ) throw (css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getHeaderDistance() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setHeaderDistance( double _headerdistance ) throw (css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getFooterDistance() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setFooterDistance( double _footerdistance ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getDifferentFirstPageHeaderFooter() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDifferentFirstPageHeaderFooter( sal_Bool status ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getSectionStart() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSectionStart( ::sal_Int32 _sectionstart ) throw (css::uno::RuntimeException, std::exception) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
