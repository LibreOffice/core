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

//#include "KDE4FilePicker.hxx"
#include <QtCore/QAbstractEventDispatcher>
#include <QtCore/QThread>
#include <QtWidgets/QApplication>
#include <QtGui/QClipboard>
#include <QtWidgets/QFrame>

#include <osl/process.h>
#include <qt5/Qt5Data.hxx>

#include "KDE5SalInstance.hxx"
#include "KDE5SalFrame.hxx"
#include "KDE5XLib.hxx"
#include "KDE5SalDisplay.hxx"

using namespace com::sun::star;

KDE5SalInstance::KDE5SalInstance(SalYieldMutex* pMutex)
    :Qt5Instance( pMutex, true )
{
    ImplSVData* pSVData = ImplGetSVData();
    delete pSVData->maAppData.mpToolkitName;
    pSVData->maAppData.mpToolkitName = new OUString("kde5");
}

SalFrame* KDE5SalInstance::CreateFrame( SalFrame *pParent, SalFrameStyleFlags nState )
{
    return new KDE5SalFrame( static_cast<KDE5SalFrame*>(pParent), nState, true );
}

uno::Reference< ui::dialogs::XFilePicker2 > KDE5SalInstance::createFilePicker(
    const uno::Reference< uno::XComponentContext >& xMSF )
{
    /*KDEXLib* kdeXLib = static_cast<KDEXLib*>( mpXLib );
    if (kdeXLib->allowKdeDialogs())
        return uno::Reference< ui::dialogs::XFilePicker2 >(
            kdeXLib->createFilePicker(xMSF) );
    else*/
        return SalInstance::createFilePicker( xMSF );
}

bool KDE5SalInstance::IsMainThread() const
{
    return qApp->thread() == QThread::currentThread();
}

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

    KDE5SalInstance* pInstance = new KDE5SalInstance(new SalYieldMutex());

    // initialize SalData
    new Qt5Data(pInstance);

    pInstance->m_pQApplication.reset(pQApplication);
    pInstance->m_pFakeArgvFreeable.reset(pFakeArgvFreeable);
    pInstance->m_pFakeArgv.reset(pFakeArgv);
    pInstance->m_pFakeArgc.reset(pFakeArgc);

    return pInstance;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
