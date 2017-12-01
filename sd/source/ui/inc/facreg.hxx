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

#ifndef INCLUDED_SD_SOURCE_UI_INC_FACREG_HXX
#define INCLUDED_SD_SOURCE_UI_INC_FACREG_HXX

#include <sal/config.h>
#include <sfx2/sfxmodelfactory.hxx>

namespace com { namespace sun { namespace star {
    namespace lang { class XMultiServiceFactory; }
    namespace uno { class XComponentContext; }
} } }

css::uno::Reference< css::uno::XInterface > SAL_CALL SdDrawingDocument_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > & _rxFactory, SfxModelFlags _nCreationFlags );
/// @throws css::uno::RuntimeException
OUString SAL_CALL SdDrawingDocument_getImplementationName();
/// @throws css::uno::RuntimeException
css::uno::Sequence< OUString > SAL_CALL SdDrawingDocument_getSupportedServiceNames();

css::uno::Reference< css::uno::XInterface > SAL_CALL SdPresentationDocument_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > & _rxFactory, SfxModelFlags _nCreationFlags );
/// @throws css::uno::RuntimeException
OUString SAL_CALL SdPresentationDocument_getImplementationName();
/// @throws css::uno::RuntimeException
css::uno::Sequence< OUString > SAL_CALL SdPresentationDocument_getSupportedServiceNames();

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
