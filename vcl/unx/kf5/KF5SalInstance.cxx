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

#include <QtData.hxx>

#include "KF5FilePicker.hxx"
#include "KF5SalInstance.hxx"

using namespace com::sun::star;

KF5SalInstance::KF5SalInstance(std::unique_ptr<QApplication>& pQApp, bool bUseCairo)
    : QtInstance(pQApp, bUseCairo)
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mxToolkitName = constructToolkitID(u"kf5");
}

bool KF5SalInstance::hasNativeFileSelection() const
{
    const OUString sDesktop = Application::GetDesktopEnvironment();
    if (sDesktop == "PLASMA5" || sDesktop == "PLASMA6")
        return true;
    return QtInstance::hasNativeFileSelection();
}

rtl::Reference<QtFilePicker>
KF5SalInstance::createPicker(css::uno::Reference<css::uno::XComponentContext> const& context,
                             QFileDialog::FileMode eMode)
{
    if (!IsMainThread())
    {
        SolarMutexGuard g;
        rtl::Reference<QtFilePicker> pPicker;
        RunInMainThread([&, this]() { pPicker = createPicker(context, eMode); });
        assert(pPicker);
        return pPicker;
    }

    // In order to insert custom controls, KF5FilePicker currently relies on KFileWidget
    // being used in the native file picker, which is only the case for KDE Plasma.
    // Therefore, return the plain qt5/qt6 one in order to not lose custom controls otherwise.
    const OUString sDesktop = Application::GetDesktopEnvironment();
    if (sDesktop == "PLASMA5" || sDesktop == "PLASMA6")
        return new KF5FilePicker(context, eMode);
    return QtInstance::createPicker(context, eMode);
}

extern "C" {
VCLPLUG_KF5_PUBLIC SalInstance* create_SalInstance()
{
    static const bool bUseCairo = (nullptr == getenv("SAL_VCL_QT_USE_QFONT"));

    std::unique_ptr<char* []> pFakeArgv;
    std::unique_ptr<int> pFakeArgc;
    std::vector<FreeableCStr> aFakeArgvFreeable;
    QtInstance::AllocFakeCmdlineArgs(pFakeArgv, pFakeArgc, aFakeArgvFreeable);

    std::unique_ptr<QApplication> pQApp
        = QtInstance::CreateQApplication(*pFakeArgc, pFakeArgv.get());

    KF5SalInstance* pInstance = new KF5SalInstance(pQApp, bUseCairo);
    pInstance->MoveFakeCmdlineArgs(pFakeArgv, pFakeArgc, aFakeArgvFreeable);

    new QtData();

    return pInstance;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
