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

#include "KF5FilePicker.hxx"
#include "KF5SalFrame.hxx"
#include "KF5SalInstance.hxx"

using namespace com::sun::star;

KF5SalInstance::KF5SalInstance(std::unique_ptr<QApplication>& pQApp, bool bUseCairo)
    : Qt5Instance(pQApp, bUseCairo)
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mxToolkitName = OUString("kf5");
}

SalFrame* KF5SalInstance::CreateChildFrame(SystemParentData* /*pParent*/, SalFrameStyleFlags nStyle)
{
    return new KF5SalFrame(nullptr, nStyle, useCairo());
}

SalFrame* KF5SalInstance::CreateFrame(SalFrame* pParent, SalFrameStyleFlags nState)
{
    assert(!pParent || dynamic_cast<KF5SalFrame*>(pParent));
    return new KF5SalFrame(static_cast<KF5SalFrame*>(pParent), nState, useCairo());
}

bool KF5SalInstance::hasNativeFileSelection() const
{
    if (Application::GetDesktopEnvironment() == "PLASMA5")
        return true;
    return Qt5Instance::hasNativeFileSelection();
}

rtl::Reference<Qt5FilePicker>
KF5SalInstance::createPicker(css::uno::Reference<css::uno::XComponentContext> const& context,
                             QFileDialog::FileMode eMode)
{
    if (!IsMainThread())
    {
        SolarMutexGuard g;
        rtl::Reference<Qt5FilePicker> pPicker;
        RunInMainThread([&, this]() { pPicker = createPicker(context, eMode); });
        assert(pPicker);
        return pPicker;
    }

    // In order to insert custom controls, KF5FilePicker currently relies on KFileWidget
    // being used in the native file picker, which is only the case for KDE Plasma.
    // Therefore, return the plain qt5 one in order to not lose custom controls.
    if (Application::GetDesktopEnvironment() == "PLASMA5")
        return new KF5FilePicker(context, eMode);
    return Qt5Instance::createPicker(context, eMode);
}

extern "C" {
VCLPLUG_KF5_PUBLIC SalInstance* create_SalInstance()
{
    static const bool bUseCairo = (nullptr == getenv("SAL_VCL_KF5_USE_QFONT"));

    std::unique_ptr<char* []> pFakeArgv;
    std::unique_ptr<int> pFakeArgc;
    std::vector<FreeableCStr> aFakeArgvFreeable;
    Qt5Instance::AllocFakeCmdlineArgs(pFakeArgv, pFakeArgc, aFakeArgvFreeable);

    std::unique_ptr<QApplication> pQApp
        = Qt5Instance::CreateQApplication(*pFakeArgc, pFakeArgv.get());

    KF5SalInstance* pInstance = new KF5SalInstance(pQApp, bUseCairo);
    pInstance->MoveFakeCmdlineArgs(pFakeArgv, pFakeArgc, aFakeArgvFreeable);

    new Qt5Data(pInstance);

    return pInstance;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
