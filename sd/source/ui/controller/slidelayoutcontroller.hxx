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

#include <svtools/popupwindowcontroller.hxx>

namespace sd
{
class SlideLayoutController : public svt::PopupWindowController
{
public:
    SlideLayoutController(const css::uno::Reference<css::uno::XComponentContext>& rxContext,
                          bool bInsertPage);

    virtual std::unique_ptr<WeldToolbarPopup> weldPopupWindow() override;
    virtual VclPtr<vcl::Window> createVclPopupWindow(vcl::Window* pParent) override;

    // XInitialization
    virtual void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& aArguments) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

private:
    bool mbInsertPage;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
