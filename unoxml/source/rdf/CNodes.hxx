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

#ifndef _CNODES_HXX
#define _CNODES_HXX

#include <sal/config.h>
#include <cppuhelper/factory.hxx>


// component helper namespace
namespace comp_CBlankNode {

// component and service helper functions:
OUString SAL_CALL _getImplementationName();
css::uno::Sequence< OUString > SAL_CALL _getSupportedServiceNames();
css::uno::Reference< css::uno::XInterface > SAL_CALL _create( css::uno::Reference< css::uno::XComponentContext > const & context );

} // closing component helper namespace


// component helper namespace
namespace comp_CURI {

// component and service helper functions:
OUString SAL_CALL _getImplementationName();
css::uno::Sequence< OUString > SAL_CALL _getSupportedServiceNames();
css::uno::Reference< css::uno::XInterface > SAL_CALL _create( css::uno::Reference< css::uno::XComponentContext > const & context );

} // closing component helper namespace


// component helper namespace
namespace comp_CLiteral {

// component and service helper functions:
OUString SAL_CALL _getImplementationName();
css::uno::Sequence< OUString > SAL_CALL _getSupportedServiceNames();
css::uno::Reference< css::uno::XInterface > SAL_CALL _create( css::uno::Reference< css::uno::XComponentContext > const & context );

} // closing component helper namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
