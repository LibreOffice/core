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

#include <ooo/vba/word/XSection.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <cppuhelper/implbase1.hxx>

typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XSection > SwVbaSection_BASE;

class SwVbaSection : public SwVbaSection_BASE
{
private:
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::beans::XPropertySet > mxPageProps;

public:
    SwVbaSection( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::frame::XModel >& xModel, const css::uno::Reference< css::beans::XPropertySet >& xProps ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaSection();

    // Attributes
    virtual ::sal_Bool SAL_CALL getProtectedForForms() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setProtectedForForms( ::sal_Bool _protectedforforms ) throw (css::uno::RuntimeException);

    // Methods
    virtual css::uno::Any SAL_CALL Headers( const css::uno::Any& index ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Footers( const css::uno::Any& index ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL PageSetup(  ) throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SW_VBA_SECTION_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
