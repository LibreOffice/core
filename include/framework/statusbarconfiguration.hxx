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
#ifndef INCLUDED_FRAMEWORK_STATUSBARCONFIGURATION_HXX
#define INCLUDED_FRAMEWORK_STATUSBARCONFIGURATION_HXX

#include <framework/fwedllapi.h>

namespace com::sun::star::container { class XIndexAccess; }
namespace com::sun::star::container { class XIndexContainer; }
namespace com::sun::star::io { class XInputStream; }
namespace com::sun::star::io { class XOutputStream; }
namespace com::sun::star::uno { class XComponentContext; }
namespace com::sun::star::uno { template <typename > class Reference; }

namespace framework
{

class FWE_DLLPUBLIC StatusBarConfiguration
{
    public:
        static bool LoadStatusBar(
            const css::uno::Reference< css::uno::XComponentContext >& xContext,
            const css::uno::Reference< css::io::XInputStream >& xInputStream,
            const css::uno::Reference< css::container::XIndexContainer >& rStatusbarConfiguration );

        static bool StoreStatusBar(
            const css::uno::Reference< css::uno::XComponentContext >& xContext,
            const css::uno::Reference< css::io::XOutputStream >& xOutputStream,
            const css::uno::Reference< css::container::XIndexAccess >& rStatusbarConfiguration );
};

} // namespace framework

#endif // INCLUDED_FRAMEWORK_STATUSBARCONFIGURATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
