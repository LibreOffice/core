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

OUString XMLAutoTextEventImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLAutoTextEventImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLAutoTextEventImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString XMLMetaImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLMetaImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLMetaImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString XMLMathSettingsImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLMathSettingsImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLMathSettingsImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString XMLMathMetaImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLMathMetaImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLMathMetaImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString XMLCalcSettingsImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLCalcSettingsImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLCalcSettingsImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString XMLCalcMetaImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLCalcMetaImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLCalcMetaImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString XMLCalcContentImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLCalcContentImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLCalcContentImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString XMLCalcStylesImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLCalcStylesImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLCalcStylesImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString XMLCalcImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLCalcImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLCalcImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString XMLWriterSettingsImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLWriterSettingsImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLWriterSettingsImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString XMLWriterMetaImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLWriterMetaImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLWriterMetaImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString XMLWriterContentImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLWriterContentImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLWriterContentImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString XMLWriterStylesImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLWriterStylesImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLWriterStylesImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString XMLWriterImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLWriterImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLWriterImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString XMLChartContentImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLChartContentImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLChartContentImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString XMLChartStylesImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLChartStylesImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLChartStylesImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString XMLChartImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLChartImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLChartImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString XMLDrawSettingsImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLDrawSettingsImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLDrawSettingsImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString XMLDrawMetaImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLDrawMetaImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLDrawMetaImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString XMLDrawContentImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLDrawContentImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLDrawContentImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString XMLDrawStylesImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLDrawStylesImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLDrawStylesImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString XMLDrawImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLDrawImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLDrawImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString XMLImpressSettingsImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLImpressSettingsImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLImpressSettingsImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString XMLImpressMetaImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLImpressMetaImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLImpressMetaImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString XMLImpressContentImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLImpressContentImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLImpressContentImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString XMLImpressStylesImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLImpressStylesImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLImpressStylesImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

OUString XMLImpressImportOOO_getImplementationName() throw();
css::uno::Sequence< OUString > XMLImpressImportOOO_getSupportedServiceNames() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface > XMLImpressImportOOO_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
