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
#include <sfx2/sfxmodelfactory.hxx>

//Math document
css::uno::Sequence< OUString >
        SmDocument_getSupportedServiceNames() throw();
OUString
        SmDocument_getImplementationName() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface >
        SmDocument_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr, SfxModelFlags _nCreationFlags);

//MathML import
css::uno::Sequence< OUString >
        SmXMLImport_getSupportedServiceNames() throw();
OUString
        SmXMLImport_getImplementationName() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface >
        SmXMLImport_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr);
css::uno::Sequence< OUString >
        SmXMLImportMeta_getSupportedServiceNames() throw();
OUString
        SmXMLImportMeta_getImplementationName() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface >
        SmXMLImportMeta_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr);
css::uno::Sequence< OUString >
        SmXMLImportSettings_getSupportedServiceNames() throw();
OUString SmXMLImportSettings_getImplementationName() throw();
/// @throws css::uno::Exception
css::uno::Reference< css::uno::XInterface >
        SmXMLImportSettings_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & rSMgr);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
