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

#pragma once

#include "fwkdllapi.h"
#include <comphelper/compbase.hxx>
#include <com/sun/star/frame/XFrame2.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

typedef comphelper::WeakComponentImplHelper<> TaskCreatorService_BASE;

class FWK_DLLPUBLIC TaskCreatorService : public TaskCreatorService_BASE
{
private:
    /** @short  the global uno service manager.
        @descr  Must be used to create own needed services.
     */
    css::uno::Reference<css::uno::XComponentContext> m_xContext;

public:
    explicit TaskCreatorService(css::uno::Reference<css::uno::XComponentContext> xContext);

    css::uno::Reference<css::frame::XFrame2>
    createInstance(const css::uno::Sequence<css::uno::Any>& lArguments);

private:
    css::uno::Reference<css::awt::XWindow>
    implts_createContainerWindow(const css::uno::Reference<css::awt::XWindow>& xParentWindow,
                                 const css::awt::Rectangle& aPosSize, bool bTopWindow);

    static void implts_applyDocStyleToWindow(const css::uno::Reference<css::awt::XWindow>& xWindow);

    css::uno::Reference<css::frame::XFrame2>
    implts_createFrame(const css::uno::Reference<css::frame::XFrame>& xParentFrame,
                       const css::uno::Reference<css::awt::XWindow>& xContainerWindow,
                       const OUString& sName);

    void
    implts_establishWindowStateListener(const css::uno::Reference<css::frame::XFrame2>& xFrame);
    void implts_establishTitleBarUpdate(const css::uno::Reference<css::frame::XFrame2>& xFrame);

    static void
    implts_establishDocModifyListener(const css::uno::Reference<css::frame::XFrame2>& xFrame);

    static OUString impl_filterNames(const OUString& sName);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
