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

#include <QtCore/QAbstractEventDispatcher>
#include <QtCore/QThread>
#include <QtWidgets/QApplication>
#include <QtGui/QClipboard>
#include <QtWidgets/QFrame>

#include <osl/process.h>
#include <sal/log.hxx>

#include "KDE5FilePicker.hxx"
#include "KDE5SalData.hxx"
#include "KDE5SalInstance.hxx"
#include "KDE5SalFrame.hxx"

using namespace com::sun::star;

KDE5SalInstance::KDE5SalInstance()
    : Qt5Instance(true)
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mxToolkitName = OUString("kde5");

    KDE5SalData::initNWF();
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

uno::Reference<ui::dialogs::XFilePicker2>
KDE5SalInstance::createFilePicker(const uno::Reference<uno::XComponentContext>& xMSF)
{
    if (!IsMainThread())
    {
        SolarMutexGuard g;
        uno::Reference<ui::dialogs::XFilePicker2> xRet;
        RunInMainThread(
            std::function([&xRet, this, xMSF]() { xRet = this->createFilePicker(xMSF); }));
        assert(xRet);
        return xRet;
    }

    // In order to insert custom controls, KDE5FilePicker currently relies on KFileWidget
    // being used in the native file picker, which is only the case for KDE Plasma.
    // Therefore, return the plain qt5 one in order to not lose custom controls.
    if (Application::GetDesktopEnvironment() == "KDE5")
    {
        return uno::Reference<ui::dialogs::XFilePicker2>(
            new KDE5FilePicker(QFileDialog::ExistingFile));
    }
    return Qt5Instance::createFilePicker(xMSF);
}

uno::Reference<ui::dialogs::XFolderPicker2>
KDE5SalInstance::createFolderPicker(const uno::Reference<uno::XComponentContext>& /*xMSF*/)
{
    return uno::Reference<ui::dialogs::XFolderPicker2>(new KDE5FilePicker(QFileDialog::Directory));
}

bool KDE5SalInstance::IsMainThread() const { return qApp->thread() == QThread::currentThread(); }

extern "C" {
VCLPLUG_KDE5_PUBLIC SalInstance* create_SalInstance()
{
    OString aVersion(qVersion());
    SAL_INFO("vcl.qt5", "qt version string is " << aVersion);

    QApplication* pQApplication;
    char** pFakeArgvFreeable = nullptr;

    int nFakeArgc = 2;
    const sal_uInt32 nParams = osl_getCommandArgCount();
    OString aDisplay;
    OUString aParam, aBin;

    for (sal_uInt32 nIdx = 0; nIdx < nParams; ++nIdx)
    {
        osl_getCommandArg(nIdx, &aParam.pData);
        if (aParam != "-display")
            continue;
        if (!pFakeArgvFreeable)
        {
            pFakeArgvFreeable = new char*[nFakeArgc + 2];
            pFakeArgvFreeable[nFakeArgc++] = strdup("-display");
        }
        else
            free(pFakeArgvFreeable[nFakeArgc]);

        ++nIdx;
        osl_getCommandArg(nIdx, &aParam.pData);
        aDisplay = OUStringToOString(aParam, osl_getThreadTextEncoding());
        pFakeArgvFreeable[nFakeArgc] = strdup(aDisplay.getStr());
    }
    if (!pFakeArgvFreeable)
        pFakeArgvFreeable = new char*[nFakeArgc];
    else
        nFakeArgc++;

    osl_getExecutableFile(&aParam.pData);
    osl_getSystemPathFromFileURL(aParam.pData, &aBin.pData);
    OString aExec = OUStringToOString(aBin, osl_getThreadTextEncoding());
    pFakeArgvFreeable[0] = strdup(aExec.getStr());
    pFakeArgvFreeable[1] = strdup("--nocrashhandler");

    char** pFakeArgv = new char*[nFakeArgc];
    for (int i = 0; i < nFakeArgc; i++)
        pFakeArgv[i] = pFakeArgvFreeable[i];

    char* session_manager = nullptr;
    if (getenv("SESSION_MANAGER") != nullptr)
    {
        session_manager = strdup(getenv("SESSION_MANAGER"));
        unsetenv("SESSION_MANAGER");
    }

    int* pFakeArgc = new int;
    *pFakeArgc = nFakeArgc;
    pQApplication = new QApplication(*pFakeArgc, pFakeArgv);

    if (session_manager != nullptr)
    {
        // coverity[tainted_string] - trusted source for setenv
        setenv("SESSION_MANAGER", session_manager, 1);
        free(session_manager);
    }

    QApplication::setQuitOnLastWindowClosed(false);

    KDE5SalInstance* pInstance = new KDE5SalInstance();

    // initialize SalData
    new KDE5SalData(pInstance);

    pInstance->m_pQApplication.reset(pQApplication);
    pInstance->m_pFakeArgvFreeable.reset(pFakeArgvFreeable);
    pInstance->m_pFakeArgv.reset(pFakeArgv);
    pInstance->m_pFakeArgc.reset(pFakeArgc);

    return pInstance;
}
}

#include <KDE5SalInstance.moc>
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
