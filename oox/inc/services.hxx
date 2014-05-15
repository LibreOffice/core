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

#define DECLARE_FUNCTIONS( className )                                                  \
extern OUString SAL_CALL className##_getImplementationName();                   \
extern css::uno::Sequence< OUString > SAL_CALL className##_getSupportedServiceNames(); \
extern css::uno::Reference< css::uno::XInterface > SAL_CALL className##_createInstance(                     \
const css::uno::Reference< css::uno::XComponentContext >& rxContext ) throw (css::uno::Exception)

namespace oox {
    namespace core {    DECLARE_FUNCTIONS( FastTokenHandler );          }
    namespace core {    DECLARE_FUNCTIONS( FilterDetect );              }
    namespace docprop { DECLARE_FUNCTIONS( DocumentPropertiesImport );  }
    namespace ppt {     DECLARE_FUNCTIONS( PowerPointImport );          }
    namespace ppt {     DECLARE_FUNCTIONS( QuickDiagrammingImport );    }
    namespace ppt {     DECLARE_FUNCTIONS( QuickDiagrammingLayout );    }
    namespace shape {   DECLARE_FUNCTIONS( ShapeContextHandler );       }
}

#undef DECLARE_FUNCTIONS

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
