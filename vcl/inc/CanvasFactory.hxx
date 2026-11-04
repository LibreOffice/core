/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <sal/config.h>

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <mutex>
#include <unordered_map>

/** The CanvasFactory is used to create the Canvas
    objects, evaluating the user's configuration preferences from

    <code>/org.openoffice.VCL/Settings/Canvas/PreferredServices</code>.

    The latter specifies a string list of service names to use.

    <p>
    You can use its createCanvas method to create Canvas objects,
    passing an empty string as service specifier (default).
    If you want to manually override the configured service list, you can pass
    a service name to try first.
    </p>
*/
class CanvasFactory : public ::cppu::WeakImplHelper<>
{
    typedef std::pair<OUString, css::uno::Sequence<OUString>> AvailPair;

    mutable std::mutex m_mutex;
    css::uno::Reference<css::uno::XComponentContext> m_xContext;
    std::unordered_map<OUString, css::uno::Sequence<OUString>> m_aAvailableImplementations;
    std::unordered_map<OUString, css::uno::Sequence<OUString>> m_aAcceleratedImplementations;
    std::unordered_map<OUString, css::uno::Sequence<OUString>> m_aAAImplementations;
    mutable std::unordered_map<OUString, OUString> m_aCachedImplementations;
    mutable bool m_bCacheHasForcedLastImpl;
    mutable bool m_bCacheHasUseAcceleratedEntry;
    mutable bool m_bCacheHasUseAAEntry;

    void checkConfigFlag(bool& r_bFlag, bool& r_CacheFlag, bool bCurrentConfigValue) const;
    css::uno::Reference<css::uno::XInterface>
    use(OUString const& serviceName, css::uno::Sequence<css::uno::Any> const& args,
        css::uno::Reference<css::uno::XComponentContext> const& xContext) const;
    css::uno::Reference<XInterface>
    lookupAndUse(OUString const& serviceName, css::uno::Sequence<css::uno::Any> const& args,
                 css::uno::Reference<css::uno::XComponentContext> const& xContext) const;

public:
    virtual ~CanvasFactory() override;
    explicit CanvasFactory(css::uno::Reference<css::uno::XComponentContext> const& xContext);

    css::uno::Reference<XInterface>
    createCanvas(OUString const& name, css::uno::Sequence<css::uno::Any> const& args,
                 css::uno::Reference<css::uno::XComponentContext> const& xContext);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
