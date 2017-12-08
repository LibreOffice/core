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

#ifndef INCLUDED_OOX_INC_SERVICES_HXX
#define INCLUDED_OOX_INC_SERVICES_HXX

#include <sal/config.h>

namespace oox {
    namespace core {
        extern OUString FastTokenHandler_getImplementationName();
        extern css::uno::Sequence< OUString > FastTokenHandler_getSupportedServiceNames();
        /// @throws css::uno::Exception
        extern css::uno::Reference< css::uno::XInterface > FastTokenHandler_createInstance(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    }
    namespace core {
        extern OUString FilterDetect_getImplementationName();
        extern css::uno::Sequence< OUString > FilterDetect_getSupportedServiceNames();
        /// @throws css::uno::Exception
        extern css::uno::Reference< css::uno::XInterface > FilterDetect_createInstance(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    }
    namespace docprop {
        extern OUString DocumentPropertiesImport_getImplementationName();
        extern css::uno::Sequence< OUString > DocumentPropertiesImport_getSupportedServiceNames();
        /// @throws css::uno::Exception
        extern css::uno::Reference< css::uno::XInterface > DocumentPropertiesImport_createInstance(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    }
    namespace ppt {
        extern OUString PowerPointImport_getImplementationName();
        extern css::uno::Sequence< OUString > PowerPointImport_getSupportedServiceNames();
        /// @throws css::uno::Exception
        extern css::uno::Reference< css::uno::XInterface > PowerPointImport_createInstance(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    }
    namespace ppt {
        extern OUString QuickDiagrammingImport_getImplementationName();
        extern css::uno::Sequence< OUString > QuickDiagrammingImport_getSupportedServiceNames();
        /// @throws css::uno::Exception
        extern css::uno::Reference< css::uno::XInterface > QuickDiagrammingImport_createInstance(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    }
    namespace ppt {
        extern OUString QuickDiagrammingLayout_getImplementationName();
        extern css::uno::Sequence< OUString > QuickDiagrammingLayout_getSupportedServiceNames();
        /// @throws css::uno::Exception
        extern css::uno::Reference< css::uno::XInterface > QuickDiagrammingLayout_createInstance(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    }
    namespace shape {
        extern OUString ShapeContextHandler_getImplementationName();
        extern css::uno::Sequence< OUString > ShapeContextHandler_getSupportedServiceNames();
        /// @throws css::uno::Exception
        extern css::uno::Reference< css::uno::XInterface > ShapeContextHandler_createInstance(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
