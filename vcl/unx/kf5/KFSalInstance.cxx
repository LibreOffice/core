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

#include <KConfigCore/KConfigGroup>
#include <KConfigCore/KSharedConfig>
#include <QtWidgets/QApplication>

#include <sal/log.hxx>

#include <QtData.hxx>

#include "KFFilePicker.hxx"
#include "KFSalInstance.hxx"

using namespace com::sun::star;

KFSalInstance::KFSalInstance(std::unique_ptr<QApplication>& pQApp)
    : QtInstance(pQApp)
{
    ImplSVData* pSVData = ImplGetSVData();
    const OUString sToolkit = u"kf" + OUString::number(QT_VERSION_MAJOR);
    pSVData->maAppData.mxToolkitName = constructToolkitID(sToolkit);
}

#if QT_VERSION < QT_VERSION_CHECK(6, 12, 0)
bool KFSalInstance::GetUseReducedAnimation()
{
    // since Qt < 6.12 doesn not have a standard way to detect whether animations should be reduced,
    // use the animation speed setting when on KDE Plasma, in accordance with how kde-gtk-config
    // sets the Gtk setting based on that:
    // https://invent.kde.org/plasma/kde-gtk-config/-/blob/881ae01ad361a03396f7f327365f225ef87688e8/kded/configvalueprovider.cpp#L239
    // (ideally, this should probably be done in the desktop backend rather than directly
    // in the VCL plugin)
    const OUString sDesktop = Application::GetDesktopEnvironment();
    if (sDesktop == "PLASMA5" || sDesktop == "PLASMA6")
    {
        KSharedConfigPtr pSharedConfig = KSharedConfig::openConfig();
        KConfigGroup aGeneralConfig = pSharedConfig->group(QStringLiteral("KDE"));
        const qreal fAnimationSpeedModifier
            = qMax(0.0, aGeneralConfig.readEntry("AnimationDurationFactor", 1.0));
        return qFuzzyIsNull(fAnimationSpeedModifier);
    }

    return QtInstance::GetUseReducedAnimation();
}
#endif

bool KFSalInstance::hasNativeFileSelection() const
{
    const OUString sDesktop = Application::GetDesktopEnvironment();
    if (sDesktop == "PLASMA5" || sDesktop == "PLASMA6")
        return true;
    return QtInstance::hasNativeFileSelection();
}

rtl::Reference<QtFilePicker>
KFSalInstance::createPicker(css::uno::Reference<css::uno::XComponentContext> const& context,
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

    // In order to insert custom controls, KFFilePicker currently relies on KFileWidget
    // being used in the native file picker, which is only the case for KDE Plasma.
    // Therefore, return the plain qt5/qt6 one in order to not lose custom controls otherwise.
    const OUString sDesktop = Application::GetDesktopEnvironment();
    if (sDesktop == "PLASMA5" || sDesktop == "PLASMA6")
        return new KFFilePicker(context, eMode);
    return QtInstance::createPicker(context, eMode);
}

extern "C" {
VCLPLUG_KF_PUBLIC SalInstance* create_SalInstance()
{
    std::unique_ptr<char* []> pFakeArgv;
    std::unique_ptr<int> pFakeArgc;
    std::vector<FreeableCStr> aFakeArgvFreeable;
    QtInstance::AllocFakeCmdlineArgs(pFakeArgv, pFakeArgc, aFakeArgvFreeable);

    std::unique_ptr<QApplication> pQApp
        = QtInstance::CreateQApplication(*pFakeArgc, pFakeArgv.get());

    KFSalInstance* pInstance = new KFSalInstance(pQApp);
    pInstance->MoveFakeCmdlineArgs(pFakeArgv, pFakeArgc, aFakeArgvFreeable);

    new QtData();

    return pInstance;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
