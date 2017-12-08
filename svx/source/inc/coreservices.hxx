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
OUString ExtrusionDepthController_getImplementationName();
/// @throws css::uno::RuntimeException
css::uno::Reference< css::uno::XInterface > ExtrusionDepthController_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > &);
/// @throws css::uno::RuntimeException
css::uno::Sequence< OUString > ExtrusionDepthController_getSupportedServiceNames();

OUString ExtrusionDirectionControl_getImplementationName();
/// @throws css::uno::RuntimeException
css::uno::Reference< css::uno::XInterface > ExtrusionDirectionControl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > &);
/// @throws css::uno::RuntimeException
css::uno::Sequence< OUString > ExtrusionDirectionControl_getSupportedServiceNames();

OUString ExtrusionLightingControl_getImplementationName();
/// @throws css::uno::RuntimeException
css::uno::Reference< css::uno::XInterface > ExtrusionLightingControl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > &);
/// @throws css::uno::RuntimeException
css::uno::Sequence< OUString > ExtrusionLightingControl_getSupportedServiceNames();

OUString ExtrusionSurfaceControl_getImplementationName();
/// @throws css::uno::RuntimeException
css::uno::Reference< css::uno::XInterface > ExtrusionSurfaceControl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > &);
/// @throws css::uno::RuntimeException
css::uno::Sequence< OUString > ExtrusionSurfaceControl_getSupportedServiceNames();

OUString FontworkAlignmentControl_getImplementationName();
/// @throws css::uno::RuntimeException
css::uno::Reference< css::uno::XInterface > FontworkAlignmentControl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > &);
/// @throws css::uno::RuntimeException
css::uno::Sequence< OUString > FontworkAlignmentControl_getSupportedServiceNames();

OUString FontworkCharacterSpacingControl_getImplementationName();
/// @throws css::uno::RuntimeException
css::uno::Reference< css::uno::XInterface > FontworkCharacterSpacingControl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > &);
/// @throws css::uno::RuntimeException
css::uno::Sequence< OUString > FontworkCharacterSpacingControl_getSupportedServiceNames();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
