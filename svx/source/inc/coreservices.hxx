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

#ifndef INCLUDED_SVX_SOURCE_INC_CORESERVICES_HXX
#define INCLUDED_SVX_SOURCE_INC_CORESERVICES_HXX

#include <sal/config.h>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace com { namespace sun { namespace star {
    namespace lang { class XMultiServiceFactory; }
    namespace uno { class XInterface; }
} } }

namespace svx
{
OUString SAL_CALL ExtrusionDepthController_getImplementationName();
/// @throws css::uno::RuntimeException
css::uno::Reference< css::uno::XInterface > SAL_CALL ExtrusionDepthController_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > &);
/// @throws css::uno::RuntimeException
css::uno::Sequence< OUString > SAL_CALL ExtrusionDepthController_getSupportedServiceNames();

OUString SAL_CALL ExtrusionDirectionControl_getImplementationName();
/// @throws css::uno::RuntimeException
css::uno::Reference< css::uno::XInterface > SAL_CALL ExtrusionDirectionControl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > &);
/// @throws css::uno::RuntimeException
css::uno::Sequence< OUString > SAL_CALL ExtrusionDirectionControl_getSupportedServiceNames();

OUString SAL_CALL ExtrusionLightingControl_getImplementationName();
/// @throws css::uno::RuntimeException
css::uno::Reference< css::uno::XInterface > SAL_CALL ExtrusionLightingControl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > &);
/// @throws css::uno::RuntimeException
css::uno::Sequence< OUString > SAL_CALL ExtrusionLightingControl_getSupportedServiceNames();

OUString SAL_CALL ExtrusionSurfaceControl_getImplementationName();
/// @throws css::uno::RuntimeException
css::uno::Reference< css::uno::XInterface > SAL_CALL ExtrusionSurfaceControl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > &);
/// @throws css::uno::RuntimeException
css::uno::Sequence< OUString > SAL_CALL ExtrusionSurfaceControl_getSupportedServiceNames();

OUString SAL_CALL FontworkAlignmentControl_getImplementationName();
/// @throws css::uno::RuntimeException
css::uno::Reference< css::uno::XInterface > SAL_CALL FontworkAlignmentControl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > &);
/// @throws css::uno::RuntimeException
css::uno::Sequence< OUString > SAL_CALL FontworkAlignmentControl_getSupportedServiceNames();

OUString SAL_CALL FontworkCharacterSpacingControl_getImplementationName();
/// @throws css::uno::RuntimeException
css::uno::Reference< css::uno::XInterface > SAL_CALL FontworkCharacterSpacingControl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > &);
/// @throws css::uno::RuntimeException
css::uno::Sequence< OUString > SAL_CALL FontworkCharacterSpacingControl_getSupportedServiceNames();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
