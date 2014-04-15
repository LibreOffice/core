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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_UNOFREG_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_UNOFREG_HXX

#include <sal/config.h>

#include <com/sun/star/uno/Exception.hpp>

namespace com { namespace sun { namespace star { namespace lang {
    class XMultiServiceFactory;
} } } }

// module
css::uno::Sequence< OUString > SAL_CALL SwUnoModule_getSupportedServiceNames() throw();
OUString SAL_CALL SwUnoModule_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SwUnoModule_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > &rSMgr ) throw( css::uno::Exception );

// writer documents
css::uno::Sequence< OUString > SAL_CALL SwTextDocument_getSupportedServiceNames() throw();
OUString SAL_CALL SwTextDocument_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SwTextDocument_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > &rSMgr, const sal_uInt64 _nCreationFlags ) throw( css::uno::Exception );
css::uno::Sequence< OUString > SAL_CALL SwWebDocument_getSupportedServiceNames() throw();
OUString SAL_CALL SwWebDocument_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SwWebDocument_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > &rSMgr ) throw( css::uno::Exception );
css::uno::Sequence< OUString > SAL_CALL SwGlobalDocument_getSupportedServiceNames() throw();
OUString SAL_CALL SwGlobalDocument_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SwGlobalDocument_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > &rSMgr ) throw( css::uno::Exception );

// xml import
css::uno::Sequence< OUString > SAL_CALL SwXMLImport_getSupportedServiceNames() throw();
OUString SAL_CALL SwXMLImport_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SwXMLImport_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr)  throw( css::uno::Exception );
css::uno::Sequence< OUString > SAL_CALL SwXMLImportStyles_getSupportedServiceNames() throw();
OUString SAL_CALL SwXMLImportStyles_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SwXMLImportStyles_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr)    throw( css::uno::Exception );
css::uno::Sequence< OUString > SAL_CALL SwXMLImportContent_getSupportedServiceNames() throw();
OUString SAL_CALL SwXMLImportContent_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SwXMLImportContent_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr)   throw( css::uno::Exception );

css::uno::Sequence< OUString > SAL_CALL     SwXMLImportMeta_getSupportedServiceNames() throw();
OUString SAL_CALL     SwXMLImportMeta_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SwXMLImportMeta_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr)  throw( css::uno::Exception );

css::uno::Sequence< OUString > SAL_CALL SwXMLImportSettings_getSupportedServiceNames() throw();
OUString SAL_CALL SwXMLImportSettings_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SwXMLImportSettings_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr)  throw( css::uno::Exception );

// xml export (OOo)
css::uno::Sequence< OUString > SAL_CALL SwXMLExportOOO_getSupportedServiceNames() throw();
OUString SAL_CALL SwXMLExportOOO_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SwXMLExportOOO_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr)   throw( css::uno::Exception );
css::uno::Sequence< OUString > SAL_CALL SwXMLExportContentOOO_getSupportedServiceNames() throw();
OUString SAL_CALL SwXMLExportContentOOO_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SwXMLExportContentOOO_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr)    throw( css::uno::Exception );
css::uno::Sequence< OUString > SAL_CALL SwXMLExportStylesOOO_getSupportedServiceNames() throw();
OUString SAL_CALL SwXMLExportStylesOOO_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SwXMLExportStylesOOO_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr) throw( css::uno::Exception );
css::uno::Sequence< OUString > SAL_CALL SwXMLExportMetaOOO_getSupportedServiceNames() throw();
OUString SAL_CALL SwXMLExportMetaOOO_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SwXMLExportMetaOOO_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr)   throw( css::uno::Exception );
css::uno::Sequence< OUString > SAL_CALL SwXMLExportSettingsOOO_getSupportedServiceNames() throw();
OUString SAL_CALL SwXMLExportSettingsOOO_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SwXMLExportSettingsOOO_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr)   throw( css::uno::Exception );

// xml export (OASIS)
css::uno::Sequence< OUString > SAL_CALL SwXMLExport_getSupportedServiceNames() throw();
OUString SAL_CALL SwXMLExport_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SwXMLExport_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr)  throw( css::uno::Exception );
css::uno::Sequence< OUString > SAL_CALL SwXMLExportContent_getSupportedServiceNames() throw();
OUString SAL_CALL SwXMLExportContent_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SwXMLExportContent_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr)   throw( css::uno::Exception );
css::uno::Sequence< OUString > SAL_CALL SwXMLExportStyles_getSupportedServiceNames() throw();
OUString SAL_CALL SwXMLExportStyles_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SwXMLExportStyles_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr)    throw( css::uno::Exception );
css::uno::Sequence< OUString > SAL_CALL SwXMLExportMeta_getSupportedServiceNames() throw();
OUString SAL_CALL SwXMLExportMeta_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SwXMLExportMeta_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr)  throw( css::uno::Exception );
css::uno::Sequence< OUString > SAL_CALL SwXMLExportSettings_getSupportedServiceNames() throw();
OUString SAL_CALL SwXMLExportSettings_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SwXMLExportSettings_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr)  throw( css::uno::Exception );

//API objects
css::uno::Sequence< OUString > SAL_CALL SwXAutoTextContainer_getSupportedServiceNames() throw();
OUString SAL_CALL SwXAutoTextContainer_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SwXAutoTextContainer_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr) throw( css::uno::Exception );

css::uno::Sequence< OUString > SAL_CALL SwXModule_getSupportedServiceNames() throw();
OUString SAL_CALL SwXModule_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SwXModule_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr) throw( css::uno::Exception );

css::uno::Sequence< OUString > SAL_CALL SwXMailMerge_getSupportedServiceNames() throw();
OUString SAL_CALL SwXMailMerge_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SwXMailMerge_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr) throw( css::uno::Exception );

css::uno::Sequence< OUString > SAL_CALL SwXMailMerge_getSupportedServiceNames() throw();
OUString SAL_CALL SwXMailMerge_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL SwXMailMerge_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr) throw( css::uno::Exception );

// Layout dump filter
css::uno::Sequence< OUString > SAL_CALL LayoutDumpFilter_getSupportedServiceNames() throw();
OUString SAL_CALL LayoutDumpFilter_getImplementationName() throw();
css::uno::Reference< css::uno::XInterface > SAL_CALL LayoutDumpFilter_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > &rSMgr ) throw( css::uno::Exception );


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
