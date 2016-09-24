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

#define DECLARE_SERVICE( classname )    \
    OUString SAL_CALL classname##_getImplementationName() throw();   \
    css::uno::Sequence< OUString > SAL_CALL classname##_getSupportedServiceNames() throw(); \
    css::uno::Reference< css::uno::XInterface > SAL_CALL classname##_createInstance( \
            const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr ) throw( css::uno::Exception );

DECLARE_SERVICE( OOo2OasisTransformer )
DECLARE_SERVICE( Oasis2OOoTransformer )

DECLARE_SERVICE( XMLAutoTextEventImportOOO )
DECLARE_SERVICE( XMLMetaImportOOO )
DECLARE_SERVICE( XMLMathSettingsImportOOO )
DECLARE_SERVICE( XMLMathMetaImportOOO )
DECLARE_SERVICE( XMLCalcSettingsImportOOO )
DECLARE_SERVICE( XMLCalcMetaImportOOO )
DECLARE_SERVICE( XMLCalcContentImportOOO )
DECLARE_SERVICE( XMLCalcStylesImportOOO )
DECLARE_SERVICE( XMLCalcImportOOO )
DECLARE_SERVICE( XMLWriterSettingsImportOOO )
DECLARE_SERVICE( XMLWriterMetaImportOOO )
DECLARE_SERVICE( XMLWriterContentImportOOO )
DECLARE_SERVICE( XMLWriterStylesImportOOO )
DECLARE_SERVICE( XMLWriterImportOOO )
DECLARE_SERVICE( XMLChartContentImportOOO )
DECLARE_SERVICE( XMLChartStylesImportOOO )
DECLARE_SERVICE( XMLChartImportOOO )
DECLARE_SERVICE( XMLDrawSettingsImportOOO )
DECLARE_SERVICE( XMLDrawMetaImportOOO )
DECLARE_SERVICE( XMLDrawContentImportOOO )
DECLARE_SERVICE( XMLDrawStylesImportOOO )
DECLARE_SERVICE( XMLDrawImportOOO )
DECLARE_SERVICE( XMLImpressSettingsImportOOO )
DECLARE_SERVICE( XMLImpressMetaImportOOO )
DECLARE_SERVICE( XMLImpressContentImportOOO )
DECLARE_SERVICE( XMLImpressStylesImportOOO )
DECLARE_SERVICE( XMLImpressImportOOO )

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
