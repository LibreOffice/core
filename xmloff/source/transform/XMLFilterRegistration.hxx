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

#ifndef INCLUDED_XMLOFF_SOURCE_TRANSFORM_XMLFILTERREGISTRATION_HXX
#define INCLUDED_XMLOFF_SOURCE_TRANSFORM_XMLFILTERREGISTRATION_HXX

#include <sal/config.h>

OUString OOo2OasisTransformer_getImplementationName() throw();
css::uno::Sequence< OUString > OOo2OasisTransformer_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > OOo2OasisTransformer_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString Oasis2OOoTransformer_getImplementationName() throw();
css::uno::Sequence< OUString > Oasis2OOoTransformer_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > Oasis2OOoTransformer_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLAutoTextEventImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLAutoTextEventImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLAutoTextEventImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLMetaImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLMetaImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLMetaImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLMathSettingsImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLMathSettingsImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLMathSettingsImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLMathMetaImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLMathMetaImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLMathMetaImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLCalcSettingsImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLCalcSettingsImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLCalcSettingsImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLCalcMetaImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLCalcMetaImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLCalcMetaImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLCalcContentImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLCalcContentImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLCalcContentImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLCalcStylesImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLCalcStylesImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLCalcStylesImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLCalcImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLCalcImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLCalcImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLWriterSettingsImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLWriterSettingsImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLWriterSettingsImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLWriterMetaImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLWriterMetaImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLWriterMetaImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLWriterContentImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLWriterContentImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLWriterContentImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLWriterStylesImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLWriterStylesImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLWriterStylesImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLWriterImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLWriterImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLWriterImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLChartContentImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLChartContentImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLChartContentImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLChartStylesImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLChartStylesImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLChartStylesImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLChartImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLChartImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLChartImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLDrawSettingsImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLDrawSettingsImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLDrawSettingsImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLDrawMetaImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLDrawMetaImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLDrawMetaImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLDrawContentImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLDrawContentImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLDrawContentImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLDrawStylesImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLDrawStylesImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLDrawStylesImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLDrawImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLDrawImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLDrawImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLImpressSettingsImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLImpressSettingsImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLImpressSettingsImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLImpressMetaImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLImpressMetaImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLImpressMetaImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLImpressContentImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLImpressContentImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLImpressContentImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLImpressStylesImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLImpressStylesImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLImpressStylesImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString SAL_CALL XMLImpressImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > SAL_CALL XMLImpressImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > SAL_CALL XMLImpressImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
