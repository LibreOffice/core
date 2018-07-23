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

#include <Qt5Instance.hxx>
#include <Qt5Instance.moc>

#include <Qt5Bitmap.hxx>
#include <Qt5Data.hxx>
#include <Qt5FilePicker.hxx>
#include <Qt5Frame.hxx>
#include <Qt5Menu.hxx>
#include <Qt5Object.hxx>
#include <Qt5System.hxx>
#include <Qt5Timer.hxx>
#include <Qt5VirtualDevice.hxx>

#include <headless/svpvd.hxx>

#include <QtCore/QAbstractEventDispatcher>
#include <QtCore/QThread>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

#include <vclpluginapi.h>
#include <sal/log.hxx>
#include <osl/process.h>

#include <headless/svpbmp.hxx>

Qt5Instance::Qt5Instance(SalYieldMutex* pMutex, bool bUseCairo)
    : SalGenericInstance(pMutex)
    , m_postUserEventId(-1)
    , m_bUseCairo(bUseCairo)
{
    ImplSVData* pSVData = ImplGetSVData();
    delete pSVData->maAppData.mpToolkitName;
    if (bUseCairo)
        pSVData->maAppData.mpToolkitName = new OUString("qt5+cairo");
    else
        pSVData->maAppData.mpToolkitName = new OUString("qt5");

    m_postUserEventId = QEvent::registerEventType();

    // this one needs to be blocking, so that the handling in main thread
    // is processed before the thread emitting the signal continues
    connect(this, SIGNAL(ImplYieldSignal(bool, bool)), this, SLOT(ImplYield(bool, bool)),
            Qt::BlockingQueuedConnection);
}

Qt5Instance::~Qt5Instance()
{
    // force freeing the QApplication before freeing the arguments,
    // as it uses references to the provided arguments!
    m_pQApplication.reset();
    for (int i = 0; i < *m_pFakeArgc; i++)
        free(m_pFakeArgvFreeable[i]);
}

SalFrame* Qt5Instance::CreateChildFrame(SystemParentData* /*pParent*/, SalFrameStyleFlags nStyle)
{
    return new Qt5Frame(nullptr, nStyle, m_bUseCairo);
}

SalFrame* Qt5Instance::CreateFrame(SalFrame* pParent, SalFrameStyleFlags nStyle)
{
    assert(!pParent || dynamic_cast<Qt5Frame*>(pParent));
    return new Qt5Frame(static_cast<Qt5Frame*>(pParent), nStyle, m_bUseCairo);
}

void Qt5Instance::DestroyFrame(SalFrame* pFrame) { delete pFrame; }

SalObject* Qt5Instance::CreateObject(SalFrame* pParent, SystemWindowData*, bool bShow)
{
    assert(!pParent || dynamic_cast<Qt5Frame*>(pParent));
    return new Qt5Object(static_cast<Qt5Frame*>(pParent), bShow);
}

void Qt5Instance::DestroyObject(SalObject* pObject) { delete pObject; }

std::unique_ptr<SalVirtualDevice>
Qt5Instance::CreateVirtualDevice(SalGraphics* pGraphics, long& nDX, long& nDY, DeviceFormat eFormat,
                                 const SystemGraphicsData* /* pData */)
{
    if (m_bUseCairo)
    {
        SvpSalGraphics* pSvpSalGraphics = dynamic_cast<SvpSalGraphics*>(pGraphics);
        assert(pSvpSalGraphics);
        std::unique_ptr<SalVirtualDevice> pVD(
            new SvpSalVirtualDevice(eFormat, pSvpSalGraphics->getSurface()));
        pVD->SetSize(nDX, nDY);
        return pVD;
    }
    else
    {
        std::unique_ptr<SalVirtualDevice> pVD(new Qt5VirtualDevice(eFormat, 1));
        pVD->SetSize(nDX, nDY);
        return pVD;
    }
}

std::unique_ptr<SalMenu> Qt5Instance::CreateMenu(bool bMenuBar, Menu* pVCLMenu)
{
    Qt5Menu* pSalMenu = new Qt5Menu(bMenuBar);
    pSalMenu->SetMenu(pVCLMenu);
    return std::unique_ptr<SalMenu>(pSalMenu);
}

std::unique_ptr<SalMenuItem> Qt5Instance::CreateMenuItem(const SalItemParams& rItemData)
{
    return std::unique_ptr<SalMenuItem>(new Qt5MenuItem(&rItemData));
}

SalTimer* Qt5Instance::CreateSalTimer() { return new Qt5Timer(); }

SalSystem* Qt5Instance::CreateSalSystem() { return new Qt5System(); }

std::shared_ptr<SalBitmap> Qt5Instance::CreateSalBitmap()
{
    if (m_bUseCairo)
        return std::make_shared<SvpSalBitmap>();
    else
        return std::make_shared<Qt5Bitmap>();
}

bool Qt5Instance::ImplYield(bool bWait, bool bHandleAllCurrentEvents)
{
    // Re-aquire the guard for user events when called via Q_EMIT ImplYieldSignal
    SolarMutexGuard aGuard;
    bool wasEvent = DispatchUserEvents(bHandleAllCurrentEvents);
    if (!bHandleAllCurrentEvents && wasEvent)
        return true;

    /**
     * Quoting the Qt docs: [QAbstractEventDispatcher::processEvents] processes
     * pending events that match flags until there are no more events to process.
     */
    SolarMutexReleaser aReleaser;
    QAbstractEventDispatcher* dispatcher = QAbstractEventDispatcher::instance(qApp->thread());
    if (bWait && !wasEvent)
        wasEvent = dispatcher->processEvents(QEventLoop::WaitForMoreEvents);
    else
        wasEvent = dispatcher->processEvents(QEventLoop::AllEvents) || wasEvent;
    return wasEvent;
}

bool Qt5Instance::DoYield(bool bWait, bool bHandleAllCurrentEvents)
{
    bool bWasEvent = false;
    if (qApp->thread() == QThread::currentThread())
    {
        bWasEvent = ImplYield(bWait, bHandleAllCurrentEvents);
        if (bWasEvent)
            m_aWaitingYieldCond.set();
    }
    else
    {
        {
            SolarMutexReleaser aReleaser;
            bWasEvent = Q_EMIT ImplYieldSignal(false, bHandleAllCurrentEvents);
        }
        if (!bWasEvent && bWait)
        {
            m_aWaitingYieldCond.reset();
            SolarMutexReleaser aReleaser;
            m_aWaitingYieldCond.wait();
            bWasEvent = true;
        }
    }
    return bWasEvent;
}

bool Qt5Instance::AnyInput(VclInputFlags /*nType*/) { return false; }

OUString Qt5Instance::GetConnectionIdentifier() { return OUString(); }

void Qt5Instance::AddToRecentDocumentList(const OUString&, const OUString&, const OUString&) {}

OpenGLContext* Qt5Instance::CreateOpenGLContext() { return nullptr; }

bool Qt5Instance::IsMainThread() const { return qApp->thread() != QThread::currentThread(); }

void Qt5Instance::TriggerUserEventProcessing()
{
    QApplication::postEvent(this, new QEvent(QEvent::Type(m_postUserEventId)));
}

void Qt5Instance::ProcessEvent(SalUserEvent aEvent)
{
    aEvent.m_pFrame->CallCallback(aEvent.m_nEvent, aEvent.m_pData);
}

css::uno::Reference<css::ui::dialogs::XFilePicker2>
Qt5Instance::createFilePicker(const css::uno::Reference<css::uno::XComponentContext>&)
{
    return css::uno::Reference<css::ui::dialogs::XFilePicker2>(
        new Qt5FilePicker(QFileDialog::ExistingFile));
}

css::uno::Reference<css::ui::dialogs::XFolderPicker2>
Qt5Instance::createFolderPicker(const css::uno::Reference<css::uno::XComponentContext>&)
{
    return css::uno::Reference<css::ui::dialogs::XFolderPicker2>(
        new Qt5FilePicker(QFileDialog::Directory));
}

extern "C" {
VCLPLUG_QT5_PUBLIC SalInstance* create_SalInstance()
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

    static const bool bUseCairo = (nullptr != getenv("SAL_VCL_QT5_USE_CAIRO"));
    Qt5Instance* pInstance = new Qt5Instance(new SalYieldMutex(), bUseCairo);

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
