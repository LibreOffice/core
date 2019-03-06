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

#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <Qt5Bitmap.hxx>
#include <Qt5Clipboard.hxx>
#include <Qt5Data.hxx>
#include <Qt5DragAndDrop.hxx>
#include <Qt5FilePicker.hxx>
#include <Qt5Frame.hxx>
#include <Qt5Menu.hxx>
#include <Qt5Object.hxx>
#include <Qt5OpenGLContext.hxx>
#include <Qt5System.hxx>
#include <Qt5Timer.hxx>
#include <Qt5VirtualDevice.hxx>

#include <headless/svpvd.hxx>

#include <QtCore/QAbstractEventDispatcher>
#include <QtCore/QThread>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

#include <vclpluginapi.h>
#include <tools/debug.hxx>
#include <comphelper/flagguard.hxx>
#include <sal/log.hxx>
#include <osl/process.h>

#include <headless/svpbmp.hxx>

#include <mutex>
#include <condition_variable>

/// TODO: not much Qt5 specific here? could be generalised, esp. for OSX...
/// this subclass allows for the transfer of a closure for running on the main
/// thread, to handle all the thread affine stuff in Qt5; the SolarMutex is
/// "loaned" to the main thread for the execution of the closure.
/// @note it doesn't work to just use "emit" and signals/slots to move calls to
/// the main thread, because the other thread has the SolarMutex; the other
/// thread (typically) cannot release SolarMutex, because then the main thread
/// will handle all sorts of events and whatnot; this design ensures that the
/// main thread only runs the passed closure (unless the closure releases
/// SolarMutex itself, which should probably be avoided).
class Qt5YieldMutex : public SalYieldMutex
{
public:
    /// flag only accessed on main thread:
    /// main thread has "borrowed" SolarMutex from another thread
    bool m_bNoYieldLock = false;
    /// members for communication from non-main thread to main thread
    std::mutex m_RunInMainMutex;
    std::condition_variable m_InMainCondition;
    bool m_isWakeUpMain = false;
    std::function<void()> m_Closure; ///< code for main thread to run
    /// members for communication from main thread to non-main thread
    std::condition_variable m_ResultCondition;
    bool m_isResultReady = false;

    virtual bool IsCurrentThread() const override;
    virtual void doAcquire(sal_uInt32 nLockCount) override;
    virtual sal_uInt32 doRelease(bool const bUnlockAll) override;
};

bool Qt5YieldMutex::IsCurrentThread() const
{
    auto const* pSalInst(static_cast<Qt5Instance const*>(GetSalData()->m_pInstance));
    assert(pSalInst);
    if (pSalInst->IsMainThread() && m_bNoYieldLock)
    {
        return true; // main thread has borrowed SolarMutex
    }
    return SalYieldMutex::IsCurrentThread();
}

void Qt5YieldMutex::doAcquire(sal_uInt32 nLockCount)
{
    auto const* pSalInst(static_cast<Qt5Instance const*>(GetSalData()->m_pInstance));
    assert(pSalInst);
    if (!pSalInst->IsMainThread())
    {
        SalYieldMutex::doAcquire(nLockCount);
        return;
    }
    if (m_bNoYieldLock)
    {
        return; // special case for main thread: borrowed from other thread
    }
    do // main thread acquire...
    {
        std::function<void()> func; // copy of closure on thread stack
        {
            std::unique_lock<std::mutex> g(m_RunInMainMutex);
            if (m_aMutex.tryToAcquire())
            {
                // if there's a closure, the other thread holds m_aMutex
                assert(!m_Closure);
                m_isWakeUpMain = false;
                --nLockCount; // have acquired once!
                ++m_nCount;
                break;
            }
            m_InMainCondition.wait(g, [this]() { return m_isWakeUpMain; });
            m_isWakeUpMain = false;
            std::swap(func, m_Closure);
        }
        if (func)
        {
            assert(!m_bNoYieldLock);
            m_bNoYieldLock = true; // execute closure with borrowed SolarMutex
            func();
            m_bNoYieldLock = false;
            std::unique_lock<std::mutex> g(m_RunInMainMutex);
            assert(!m_isResultReady);
            m_isResultReady = true;
            m_ResultCondition.notify_all(); // unblock other thread
        }
    } while (true);
    SalYieldMutex::doAcquire(nLockCount);
}

sal_uInt32 Qt5YieldMutex::doRelease(bool const bUnlockAll)
{
    auto const* pSalInst(static_cast<Qt5Instance const*>(GetSalData()->m_pInstance));
    assert(pSalInst);
    if (pSalInst->IsMainThread() && m_bNoYieldLock)
    {
        return 1; // dummy value
    }

    std::unique_lock<std::mutex> g(m_RunInMainMutex);
    // read m_nCount before doRelease (it's guarded by m_aMutex)
    bool const isReleased(bUnlockAll || m_nCount == 1);
    sal_uInt32 nCount = SalYieldMutex::doRelease(bUnlockAll);
    if (isReleased && !pSalInst->IsMainThread())
    {
        m_isWakeUpMain = true;
        m_InMainCondition.notify_all(); // unblock main thread
    }
    return nCount;
}

// this could be abstracted to be independent of Qt5 by passing in the
// event-trigger as another function parameter...
// it could also be a template of the return type, then it could return the
// result of func... but then how to handle the result in doAcquire?
void Qt5Instance::RunInMainThread(std::function<void()> func)
{
    DBG_TESTSOLARMUTEX();
    if (IsMainThread())
    {
        func();
        return;
    }

    Qt5YieldMutex* const pMutex(static_cast<Qt5YieldMutex*>(GetYieldMutex()));
    {
        std::unique_lock<std::mutex> g(pMutex->m_RunInMainMutex);
        assert(!pMutex->m_Closure);
        pMutex->m_Closure = func;
        // unblock main thread in case it is blocked on condition
        pMutex->m_isWakeUpMain = true;
        pMutex->m_InMainCondition.notify_all();
    }
    // wake up main thread in case it is blocked on event queue
    // TriggerUserEventProcessing() appears to be insufficient in case the
    // main thread does QEventLoop::WaitForMoreEvents
    Q_EMIT ImplRunInMainSignal();
    {
        std::unique_lock<std::mutex> g(pMutex->m_RunInMainMutex);
        pMutex->m_ResultCondition.wait(g, [pMutex]() { return pMutex->m_isResultReady; });
        pMutex->m_isResultReady = false;
    }
}

void Qt5Instance::ImplRunInMain()
{
    SolarMutexGuard g; // trigger the dispatch code in Qt5YieldMutex::doAcquire
    (void)this; // suppress unhelpful [loplugin:staticmethods]; can't be static
}

Qt5Instance::Qt5Instance(bool bUseCairo)
    : SalGenericInstance(std::make_unique<Qt5YieldMutex>())
    , m_postUserEventId(-1)
    , m_bUseCairo(bUseCairo)
{
    ImplSVData* pSVData = ImplGetSVData();
    if (bUseCairo)
        pSVData->maAppData.mxToolkitName = OUString("qt5+cairo");
    else
        pSVData->maAppData.mxToolkitName = OUString("qt5");

    m_postUserEventId = QEvent::registerEventType();

    // this one needs to be blocking, so that the handling in main thread
    // is processed before the thread emitting the signal continues
    connect(this, SIGNAL(ImplYieldSignal(bool, bool)), this, SLOT(ImplYield(bool, bool)),
            Qt::BlockingQueuedConnection);
    connect(this, &Qt5Instance::ImplRunInMainSignal, this, &Qt5Instance::ImplRunInMain,
            Qt::QueuedConnection); // no Blocking!

    // this one needs to be queued non-blocking
    // in order to have this event arriving to correct event processing loop
    connect(this, &Qt5Instance::deleteObjectLaterSignal, this,
            [](QObject* pObject) { Qt5Instance::deleteObjectLater(pObject); },
            Qt::QueuedConnection);
}

Qt5Instance::~Qt5Instance()
{
    // force freeing the QApplication before freeing the arguments,
    // as it uses references to the provided arguments!
    m_pQApplication.reset();
    for (int i = 0; i < *m_pFakeArgc; i++)
        free(m_pFakeArgvFreeable[i]);
}

void Qt5Instance::deleteObjectLater(QObject* pObject) { pObject->deleteLater(); }

SalFrame* Qt5Instance::CreateChildFrame(SystemParentData* /*pParent*/, SalFrameStyleFlags nStyle)
{
    return new Qt5Frame(nullptr, nStyle, m_bUseCairo);
}

SalFrame* Qt5Instance::CreateFrame(SalFrame* pParent, SalFrameStyleFlags nStyle)
{
    assert(!pParent || dynamic_cast<Qt5Frame*>(pParent));
    return new Qt5Frame(static_cast<Qt5Frame*>(pParent), nStyle, m_bUseCairo);
}

void Qt5Instance::DestroyFrame(SalFrame* pFrame)
{
    if (pFrame)
    {
        assert(dynamic_cast<Qt5Frame*>(pFrame));
        Q_EMIT deleteObjectLaterSignal(static_cast<Qt5Frame*>(pFrame));
    }
}

SalObject* Qt5Instance::CreateObject(SalFrame* pParent, SystemWindowData*, bool bShow)
{
    assert(!pParent || dynamic_cast<Qt5Frame*>(pParent));
    return new Qt5Object(static_cast<Qt5Frame*>(pParent), bShow);
}

void Qt5Instance::DestroyObject(SalObject* pObject)
{
    if (pObject)
    {
        assert(dynamic_cast<Qt5Object*>(pObject));
        Q_EMIT deleteObjectLaterSignal(static_cast<Qt5Object*>(pObject));
    }
}

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
    std::unique_ptr<SalMenu> pRet;
    RunInMainThread([&pRet, bMenuBar, pVCLMenu]() {
        Qt5Menu* pSalMenu = new Qt5Menu(bMenuBar);
        pRet.reset(pSalMenu);
        pSalMenu->SetMenu(pVCLMenu);
    });
    assert(pRet);
    return pRet;
}

std::unique_ptr<SalMenuItem> Qt5Instance::CreateMenuItem(const SalItemParams& rItemData)
{
    return std::unique_ptr<SalMenuItem>(new Qt5MenuItem(&rItemData));
}

SalTimer* Qt5Instance::CreateSalTimer() { return new Qt5Timer(); }

SalSystem* Qt5Instance::CreateSalSystem() { return new Qt5System; }

std::shared_ptr<SalBitmap> Qt5Instance::CreateSalBitmap()
{
    if (m_bUseCairo)
        return std::make_shared<SvpSalBitmap>();
    else
        return std::make_shared<Qt5Bitmap>();
}

bool Qt5Instance::ImplYield(bool bWait, bool bHandleAllCurrentEvents)
{
    // Re-acquire the guard for user events when called via Q_EMIT ImplYieldSignal
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

OpenGLContext* Qt5Instance::CreateOpenGLContext() { return new Qt5OpenGLContext; }

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

css::uno::Reference<css::uno::XInterface>
Qt5Instance::CreateClipboard(const css::uno::Sequence<css::uno::Any>& arguments)
{
    OUString sel;
    if (arguments.getLength() == 0)
    {
        sel = "CLIPBOARD";
    }
    else if (arguments.getLength() != 1 || !(arguments[0] >>= sel))
    {
        throw css::lang::IllegalArgumentException("bad Qt5Instance::CreateClipboard arguments",
                                                  css::uno::Reference<css::uno::XInterface>(), -1);
    }

    auto it = m_aClipboards.find(sel);
    if (it != m_aClipboards.end())
    {
        return it->second;
    }

    css::uno::Reference<css::uno::XInterface> xClipboard(
        static_cast<cppu::OWeakObject*>(new VclQt5Clipboard(sel)));
    m_aClipboards[sel] = xClipboard;

    return xClipboard;
}

Reference<XInterface> Qt5Instance::CreateDragSource()
{
    return Reference<XInterface>(static_cast<cppu::OWeakObject*>(new Qt5DragSource()));
}

Reference<XInterface> Qt5Instance::CreateDropTarget()
{
    return Reference<XInterface>(static_cast<cppu::OWeakObject*>(new Qt5DropTarget()));
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
    Qt5Instance* pInstance = new Qt5Instance(bUseCairo);

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
