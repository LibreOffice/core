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

#ifndef SDEXT_PRESENTER_COMPONENT_HXX
#define SDEXT_PRESENTER_COMPONENT_HXX

#include <com/sun/star/uno/XComponentContext.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

/** This string is replaced automatically by the makefile during the
    building of this extension.
*/
class PresenterComponent
{
public:
    const static ::rtl::OUString gsExtensionIdentifier;

    static ::rtl::OUString GetBasePath (
        const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext);
    static ::rtl::OUString GetBasePath (
        const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
        const ::rtl::OUString& rsExtensionIdentifier);
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
