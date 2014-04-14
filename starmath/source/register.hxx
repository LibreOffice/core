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

#ifndef INCLUDED_STARMATH_SOURCE_REGISTER_HXX
#define INCLUDED_STARMATH_SOURCE_REGISTER_HXX

#include <sal/config.h>

//Math document
css::uno::Sequence< OUString > SAL_CALL
        SmDocument_getSupportedServiceNames() throw();
OUString SAL_CALL
        SmDocument_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface >SAL_CALL
        SmDocument_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr, const sal_uInt64 _nCreationFlags) throw( css::uno::Exception );

//MathML import
css::uno::Sequence< OUString > SAL_CALL
        SmXMLImport_getSupportedServiceNames() throw();
OUString SAL_CALL
        SmXMLImport_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL
        SmXMLImport_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr) throw( css::uno::Exception );
css::uno::Sequence< OUString > SAL_CALL
        SmXMLImportMeta_getSupportedServiceNames() throw();
OUString SAL_CALL
        SmXMLImportMeta_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL
        SmXMLImportMeta_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr) throw( css::uno::Exception );
css::uno::Sequence< OUString > SAL_CALL
        SmXMLImportSettings_getSupportedServiceNames() throw();
OUString SAL_CALL SmXMLImportSettings_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL
        SmXMLImportSettings_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr) throw( css::uno::Exception );

//MathML export
css::uno::Sequence< OUString > SAL_CALL
        SmXMLExport_getSupportedServiceNames() throw();
OUString SAL_CALL
        SmXMLExport_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL
        SmXMLExport_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr) throw( css::uno::Exception );
css::uno::Sequence< OUString > SAL_CALL
        SmXMLExportMetaOOO_getSupportedServiceNames() throw();
OUString SAL_CALL
        SmXMLExportMetaOOO_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL
        SmXMLExportMetaOOO_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr) throw( css::uno::Exception );
css::uno::Sequence< OUString > SAL_CALL
        SmXMLExportMeta_getSupportedServiceNames() throw();
OUString SAL_CALL
        SmXMLExportMeta_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL
        SmXMLExportMeta_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr) throw( css::uno::Exception );
css::uno::Sequence< OUString > SAL_CALL
        SmXMLExportSettingsOOO_getSupportedServiceNames() throw();
OUString SAL_CALL
        SmXMLExportSettingsOOO_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL
        SmXMLExportSettingsOOO_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr) throw( css::uno::Exception );
css::uno::Sequence< OUString > SAL_CALL
        SmXMLExportSettings_getSupportedServiceNames() throw();
OUString SAL_CALL
        SmXMLExportSettings_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL
        SmXMLExportSettings_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr) throw( css::uno::Exception );
css::uno::Sequence< OUString > SAL_CALL
        SmXMLExportContent_getSupportedServiceNames() throw();
OUString SAL_CALL
        SmXMLExportContent_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL
        SmXMLExportContent_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr) throw( css::uno::Exception );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
