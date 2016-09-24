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
#include <sfx2/sfxmodelfactory.hxx>

#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Sequence.hxx>

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
css::uno::Reference< css::uno::XInterface > SAL_CALL SwTextDocument_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory > &rSMgr, SfxModelFlags _nCreationFlags ) throw( css::uno::Exception );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
