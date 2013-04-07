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
#ifndef SW_VBA_ADDIN_HXX
#define SW_VBA_ADDIN_HXX

#include <ooo/vba/word/XAddin.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <cppuhelper/implbase1.hxx>

typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XAddin > SwVbaAddin_BASE;

class SwVbaAddin : public SwVbaAddin_BASE
{
private:
    OUString msFileURL;
    sal_Bool mbAutoload;
    sal_Bool mbInstalled;

public:
    SwVbaAddin( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const OUString& rFileURL, sal_Bool bAutoload ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaAddin();

    // Attributes
    virtual OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setName( const OUString& _name ) throw (css::uno::RuntimeException);
    virtual OUString SAL_CALL getPath() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getAutoload() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getInstalled() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setInstalled( ::sal_Bool _installed ) throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual OUString getServiceImplName();
    virtual css::uno::Sequence<OUString> getServiceNames();
};
#endif /* SW_VBA_ADDIN_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
