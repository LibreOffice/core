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

#include <sal/config.h>

#include <utility>

#include <QtWidgets/QApplication>

#include <sal/log.hxx>

#include <Qt5Data.hxx>

#include "KDE5FilePicker.hxx"
#include "KDE5SalFrame.hxx"
#include "KDE5SalInstance.hxx"

using namespace com::sun::star;

KDE5SalInstance::KDE5SalInstance(std::unique_ptr<QApplication>& pQApp)
    : Qt5Instance(pQApp, true)
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mxToolkitName = OUString("kde5");
}

SalFrame* KDE5SalInstance::CreateFrame(SalFrame* pParent, SalFrameStyleFlags nState)
{
    SalFrame* pRet(nullptr);
    RunInMainThread(std::function([&pRet, pParent, nState]() {
        pRet = new KDE5SalFrame(static_cast<KDE5SalFrame*>(pParent), nState, true);
    }));
    assert(pRet);
    return pRet;
}

bool KDE5SalInstance::hasNativeFileSelection() const
{
    if (Application::GetDesktopEnvironment() == "KDE5")
        return true;
    return Qt5Instance::hasNativeFileSelection();
}

Qt5FilePicker*
KDE5SalInstance::createPicker(css::uno::Reference<css::uno::XComponentContext> const& context,
                              QFileDialog::FileMode eMode)
{
    if (!IsMainThread())
    {
        SolarMutexGuard g;
        Qt5FilePicker* pPicker;
        RunInMainThread(std::function([&, this]() { pPicker = createPicker(context, eMode); }));
        assert(pPicker);
        return pPicker;
    }

    // In order to insert custom controls, KDE5FilePicker currently relies on KFileWidget
    // being used in the native file picker, which is only the case for KDE Plasma.
    // Therefore, return the plain qt5 one in order to not lose custom controls.
    if (Application::GetDesktopEnvironment() == "KDE5")
        return new KDE5FilePicker(context, eMode);
    return Qt5Instance::createPicker(context, eMode);
}

extern "C" {
VCLPLUG_KDE5_PUBLIC SalInstance* create_SalInstance()
{
    std::unique_ptr<char* []> pFakeArgv;
    std::unique_ptr<int> pFakeArgc;
    std::vector<FreeableCStr> aFakeArgvFreeable;
    Qt5Instance::AllocFakeCmdlineArgs(pFakeArgv, pFakeArgc, aFakeArgvFreeable);

    std::unique_ptr<QApplication> pQApp
        = Qt5Instance::CreateQApplication(*pFakeArgc, pFakeArgv.get());

    KDE5SalInstance* pInstance = new KDE5SalInstance(pQApp);
    pInstance->MoveFakeCmdlineArgs(pFakeArgv, pFakeArgc, aFakeArgvFreeable);

    new Qt5Data(pInstance);

    return pInstance;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
