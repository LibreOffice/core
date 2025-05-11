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

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <framework/windowstatehelper.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/syswin.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/window.hxx>

namespace framework
{
// for use from SfxViewShell inherited classes
OUString WindowStateHelper::GetFromWindow(vcl::Window* pWin)
{
    // SOLAR SAFE -> ------------------------
    SolarMutexGuard aSolarGuard;

    // getting to system window here is necessary
    while (pWin && !pWin->IsSystemWindow())
    {
        pWin = pWin->GetParent();
    }
    if (pWin)
    {
        vcl::WindowDataMask const nMask
            = vcl::WindowDataMask::All & ~vcl::WindowDataMask::Minimized;
        return static_cast<SystemWindow*>(pWin)->GetWindowState(nMask);
    }

    return {};
    // <- SOLAR SAFE ------------------------
}

// for use from application exports
OUString WindowStateHelper::GetFromModel(const css::uno::Reference<css::frame::XModel>& xModel)
{
    if (!xModel)
        return {};

    auto xController = xModel->getCurrentController();
    if (!xController)
        return {};

    auto xFrame = xController->getFrame();
    if (!xFrame)
        return {};

    if (auto xWindow = xFrame->getContainerWindow())
    {
        // SOLAR SAFE FOR VCL HELPER -> ------------------------
        SolarMutexGuard aSolarGuard;
        return WindowStateHelper::GetFromWindow(VCLUnoHelper::GetWindow(xWindow));
        // <- SOLAR SAFE ------------------------
    }

    return {};
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
