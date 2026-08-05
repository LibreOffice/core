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

#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/scanner/XScannerManager2.hpp>

class ScannerManager final :
    public cppu::WeakImplHelper<
        css::scanner::XScannerManager2, css::awt::XBitmap, css::lang::XServiceInfo, css::lang::XInitialization>
{
    osl::Mutex                              maProtector;
    css::uno::Reference<css::awt::XWindow>  mxDialogParent;
    void*                                   mpData;

    static void                             AcquireData();
    void                                    ReleaseData();

public:

                                            ScannerManager();
    virtual                                 ~ScannerManager() override;

    // XScannerManager
    virtual cpo::uno::Sequence< css::scanner::ScannerContext >  getAvailableScanners() override;
    virtual bool               configureScanner( css::scanner::ScannerContext& scanner_context ) override;
    virtual bool               configureScannerAndScan( css::scanner::ScannerContext& scanner_context, const css::uno::Reference< css::lang::XEventListener >& rxListener ) override;
    virtual void                   startScan( const css::scanner::ScannerContext& scanner_context, const css::uno::Reference< css::lang::XEventListener >& rxListener ) override;
    virtual css::scanner::ScanError              getError( const css::scanner::ScannerContext& scanner_context ) override;
    virtual css::uno::Reference< css::awt::XBitmap >    getBitmap( const css::scanner::ScannerContext& scanner_context ) override;

    // XBitmap
    virtual css::awt::Size              getSize() override;
    virtual cpo::uno::Sequence< sal_Int8 >        getDIB() override;
    virtual cpo::uno::Sequence< sal_Int8 >        getMaskDIB() override;

    OUString getImplementationName() override;

    bool supportsService(OUString const & ServiceName) override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;

    virtual void                   initialize(const cpo::uno::Sequence<cpo::uno::Any>& rArguments) override;

#ifdef _WIN32
    void*                                   GetData() const { return mpData; }
#endif
    void                                    SetData( void* pData ) { ReleaseData(); mpData = pData; }
 };

/// @throws Exception
css::uno::Reference< css::uno::XInterface > ScannerManager_CreateInstance( const css::uno::Reference< css::lang::XMultiServiceFactory >& rxFactory );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
