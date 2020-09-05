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
#include "Qt5SvpVirtualDevice.hxx"
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
#include <unx/gstsink.hxx>
#include <headless/svpbmp.hxx>

#include <mutex>
#include <condition_variable>

namespace
{
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
}

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
            std::scoped_lock<std::mutex> g(m_RunInMainMutex);
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

    std::scoped_lock<std::mutex> g(m_RunInMainMutex);
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
        std::scoped_lock<std::mutex> g(pMutex->m_RunInMainMutex);
        assert(!pMutex->m_Closure);
        pMutex->m_Closure = func;
        // unblock main thread in case it is blocked on condition
        pMutex->m_isWakeUpMain = true;
        pMutex->m_InMainCondition.notify_all();
    }

    TriggerUserEventProcessing();
    {
        std::unique_lock<std::mutex> g(pMutex->m_RunInMainMutex);
        pMutex->m_ResultCondition.wait(g, [pMutex]() { return pMutex->m_isResultReady; });
        pMutex->m_isResultReady = false;
    }
}

Qt5Instance::Qt5Instance(std::unique_ptr<QApplication>& pQApp, bool bUseCairo)
    : SalGenericInstance(std::make_unique<Qt5YieldMutex>())
    , m_bUseCairo(bUseCairo)
    , m_pTimer(nullptr)
    , m_bSleeping(false)
    , m_pQApplication(std::move(pQApp))
    , m_aUpdateStyleTimer("vcl::qt5 m_aUpdateStyleTimer")
    , m_bUpdateFonts(false)
{
    ImplSVData* pSVData = ImplGetSVData();
    if (bUseCairo)
        pSVData->maAppData.mxToolkitName = OUString("qt5+cairo");
    else
        pSVData->maAppData.mxToolkitName = OUString("qt5");

    // this one needs to be blocking, so that the handling in main thread
    // is processed before the thread emitting the signal continues
    connect(this, SIGNAL(ImplYieldSignal(bool, bool)), this, SLOT(ImplYield(bool, bool)),
            Qt::BlockingQueuedConnection);

    // this one needs to be queued non-blocking
    // in order to have this event arriving to correct event processing loop
    connect(this, &Qt5Instance::deleteObjectLaterSignal, this,
            [](QObject* pObject) { Qt5Instance::deleteObjectLater(pObject); },
            Qt::QueuedConnection);

    m_aUpdateStyleTimer.SetTimeout(50);
    m_aUpdateStyleTimer.SetInvokeHandler(LINK(this, Qt5Instance, updateStyleHdl));

    QAbstractEventDispatcher* dispatcher = QAbstractEventDispatcher::instance(qApp->thread());
    connect(dispatcher, &QAbstractEventDispatcher::awake, this, [this]() { m_bSleeping = false; });
    connect(dispatcher, &QAbstractEventDispatcher::aboutToBlock, this,
            [this]() { m_bSleeping = true; });
}

Qt5Instance::~Qt5Instance()
{
    // force freeing the QApplication before freeing the arguments,
    // as it uses references to the provided arguments!
    m_pQApplication.reset();
}

void Qt5Instance::AfterAppInit()
{
    // set the default application icon via desktop file just on Wayland,
    // as this otherwise overrides the individual desktop icons on X11.
    if (QGuiApplication::platformName() == "wayland")
        QGuiApplication::setDesktopFileName(QStringLiteral("libreoffice-startcenter.desktop"));
    QGuiApplication::setLayoutDirection(AllSettings::GetLayoutRTL() ? Qt::RightToLeft
                                                                    : Qt::LeftToRight);
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

std::unique_ptr<SalVirtualDevice> Qt5Instance::CreateVirtualDevice(SalGraphics* pGraphics,
                                                                   long& nDX, long& nDY,
                                                                   DeviceFormat eFormat,
                                                                   const SystemGraphicsData* pGd)
{
    if (m_bUseCairo)
    {
        SvpSalGraphics* pSvpSalGraphics = dynamic_cast<Qt5SvpGraphics*>(pGraphics);
        assert(pSvpSalGraphics);
        // tdf#127529 see SvpSalInstance::CreateVirtualDevice for the rare case of a non-null pPreExistingTarget
        cairo_surface_t* pPreExistingTarget
            = pGd ? static_cast<cairo_surface_t*>(pGd->pSurface) : nullptr;
        std::unique_ptr<SalVirtualDevice> pVD(
            new Qt5SvpVirtualDevice(eFormat, pSvpSalGraphics->getSurface(), pPreExistingTarget));
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

SalTimer* Qt5Instance::CreateSalTimer()
{
    m_pTimer = new Qt5Timer();
    return m_pTimer;
}

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

bool Qt5Instance::AnyInput(VclInputFlags nType)
{
    bool bResult = false;
    if (nType & VclInputFlags::TIMER)
        bResult |= (m_pTimer && m_pTimer->remainingTime() == 0);
    if (nType & VclInputFlags::OTHER)
        bResult |= !m_bSleeping;
    return bResult;
}

OUString Qt5Instance::GetConnectionIdentifier() { return OUString(); }

void Qt5Instance::AddToRecentDocumentList(const OUString&, const OUString&, const OUString&) {}

OpenGLContext* Qt5Instance::CreateOpenGLContext() { return new Qt5OpenGLContext; }

bool Qt5Instance::IsMainThread() const
{
    return !qApp || (qApp->thread() == QThread::currentThread());
}

void Qt5Instance::TriggerUserEventProcessing()
{
    QAbstractEventDispatcher* dispatcher = QAbstractEventDispatcher::instance(qApp->thread());
    dispatcher->wakeUp();
}

void Qt5Instance::ProcessEvent(SalUserEvent aEvent)
{
    aEvent.m_pFrame->CallCallback(aEvent.m_nEvent, aEvent.m_pData);
}

Qt5FilePicker*
Qt5Instance::createPicker(css::uno::Reference<css::uno::XComponentContext> const& context,
                          QFileDialog::FileMode eMode)
{
    if (!IsMainThread())
    {
        SolarMutexGuard g;
        Qt5FilePicker* pPicker;
        RunInMainThread([&, this]() { pPicker = createPicker(context, eMode); });
        assert(pPicker);
        return pPicker;
    }

    return new Qt5FilePicker(context, eMode);
}

css::uno::Reference<css::ui::dialogs::XFilePicker2>
Qt5Instance::createFilePicker(const css::uno::Reference<css::uno::XComponentContext>& context)
{
    return css::uno::Reference<css::ui::dialogs::XFilePicker2>(
        createPicker(context, QFileDialog::ExistingFile));
}

css::uno::Reference<css::ui::dialogs::XFolderPicker2>
Qt5Instance::createFolderPicker(const css::uno::Reference<css::uno::XComponentContext>& context)
{
    return css::uno::Reference<css::ui::dialogs::XFolderPicker2>(
        createPicker(context, QFileDialog::Directory));
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

    // This could also use RunInMain, but SolarMutexGuard is enough
    // since at this point we're not accessing the clipboard, just get the
    // accessor to the clipboard.
    SolarMutexGuard aGuard;

    auto it = m_aClipboards.find(sel);
    if (it != m_aClipboards.end())
        return it->second;

    css::uno::Reference<css::uno::XInterface> xClipboard = Qt5Clipboard::create(sel);
    if (xClipboard.is())
        m_aClipboards[sel] = xClipboard;

    return xClipboard;
}

css::uno::Reference<css::uno::XInterface> Qt5Instance::CreateDragSource()
{
    return css::uno::Reference<css::uno::XInterface>(
        static_cast<cppu::OWeakObject*>(new Qt5DragSource()));
}

css::uno::Reference<css::uno::XInterface> Qt5Instance::CreateDropTarget()
{
    return css::uno::Reference<css::uno::XInterface>(
        static_cast<cppu::OWeakObject*>(new Qt5DropTarget()));
}

IMPL_LINK_NOARG(Qt5Instance, updateStyleHdl, Timer*, void)
{
    SolarMutexGuard aGuard;
    SalFrame* pFrame = anyFrame();
    if (pFrame)
    {
        pFrame->CallCallback(SalEvent::SettingsChanged, nullptr);
        if (m_bUpdateFonts)
        {
            pFrame->CallCallback(SalEvent::FontChanged, nullptr);
            m_bUpdateFonts = false;
        }
    }
}

void Qt5Instance::UpdateStyle(bool bFontsChanged)
{
    if (bFontsChanged)
        m_bUpdateFonts = true;
    if (!m_aUpdateStyleTimer.IsActive())
        m_aUpdateStyleTimer.Start();
}

void* Qt5Instance::CreateGStreamerSink(const SystemChildWindow* pWindow)
{
#if ENABLE_GSTREAMER_1_0 && QT5_HAVE_GOBJECT
    auto pSymbol = gstElementFactoryNameSymbol();
    if (!pSymbol)
        return nullptr;

    const SystemEnvData* pEnvData = pWindow->GetSystemData();
    if (!pEnvData)
        return nullptr;

    if (pEnvData->platform != SystemEnvData::Platform::Wayland)
        return nullptr;

    GstElement* pVideosink = pSymbol("qwidget5videosink", "qwidget5videosink");
    if (pVideosink)
    {
        QWidget* pQWidget = static_cast<QWidget*>(pEnvData->pWidget);
        g_object_set(G_OBJECT(pVideosink), "widget", pQWidget, nullptr);
    }
    else
    {
        SAL_WARN("vcl.qt5", "Couldn't initialize qwidget5videosink."
                            " Video playback might not work as expected."
                            " Please install Qt5 packages for QtGStreamer.");
        // with no videosink explicitly set, GStreamer will open its own (misplaced) window(s) to display video
    }

    return pVideosink;
#else
    (void*)pWindow;
    return nullptr;
#endif
}

void Qt5Instance::AllocFakeCmdlineArgs(std::unique_ptr<char* []>& rFakeArgv,
                                       std::unique_ptr<int>& rFakeArgc,
                                       std::vector<FreeableCStr>& rFakeArgvFreeable)
{
    OString aVersion(qVersion());
    SAL_INFO("vcl.qt5", "qt version string is " << aVersion);

    const sal_uInt32 nParams = osl_getCommandArgCount();
    sal_uInt32 nDisplayValueIdx = 0;
    OUString aParam, aBin;

    for (sal_uInt32 nIdx = 0; nIdx < nParams; ++nIdx)
    {
        osl_getCommandArg(nIdx, &aParam.pData);
        if (aParam != "-display")
            continue;
        ++nIdx;
        nDisplayValueIdx = nIdx;
    }

    osl_getExecutableFile(&aParam.pData);
    osl_getSystemPathFromFileURL(aParam.pData, &aBin.pData);
    OString aExec = OUStringToOString(aBin, osl_getThreadTextEncoding());

    std::vector<FreeableCStr> aFakeArgvFreeable;
    aFakeArgvFreeable.reserve(4);
    aFakeArgvFreeable.emplace_back(strdup(aExec.getStr()));
    aFakeArgvFreeable.emplace_back(strdup("--nocrashhandler"));
    if (nDisplayValueIdx)
    {
        aFakeArgvFreeable.emplace_back(strdup("-display"));
        osl_getCommandArg(nDisplayValueIdx, &aParam.pData);
        OString aDisplay = OUStringToOString(aParam, osl_getThreadTextEncoding());
        aFakeArgvFreeable.emplace_back(strdup(aDisplay.getStr()));
    }
    rFakeArgvFreeable.swap(aFakeArgvFreeable);

    const int nFakeArgc = rFakeArgvFreeable.size();
    rFakeArgv.reset(new char*[nFakeArgc]);
    for (int i = 0; i < nFakeArgc; i++)
        rFakeArgv[i] = rFakeArgvFreeable[i].get();

    rFakeArgc.reset(new int);
    *rFakeArgc = nFakeArgc;
}

void Qt5Instance::MoveFakeCmdlineArgs(std::unique_ptr<char* []>& rFakeArgv,
                                      std::unique_ptr<int>& rFakeArgc,
                                      std::vector<FreeableCStr>& rFakeArgvFreeable)
{
    m_pFakeArgv = std::move(rFakeArgv);
    m_pFakeArgc = std::move(rFakeArgc);
    m_pFakeArgvFreeable.swap(rFakeArgvFreeable);
}

std::unique_ptr<QApplication> Qt5Instance::CreateQApplication(int& nArgc, char** pArgv)
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    // for scaled icons in the native menus
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    FreeableCStr session_manager;
    if (getenv("SESSION_MANAGER") != nullptr)
    {
        session_manager.reset(strdup(getenv("SESSION_MANAGER")));
        unsetenv("SESSION_MANAGER");
    }

    std::unique_ptr<QApplication> pQApp = std::make_unique<QApplication>(nArgc, pArgv);

    if (session_manager != nullptr)
    {
        // coverity[tainted_string] - trusted source for setenv
        setenv("SESSION_MANAGER", session_manager.get(), 1);
    }

    QApplication::setQuitOnLastWindowClosed(false);
    return pQApp;
}

extern "C" {
VCLPLUG_QT5_PUBLIC SalInstance* create_SalInstance()
{
    static const bool bUseCairo = (nullptr != getenv("SAL_VCL_QT5_USE_CAIRO"));

    std::unique_ptr<char* []> pFakeArgv;
    std::unique_ptr<int> pFakeArgc;
    std::vector<FreeableCStr> aFakeArgvFreeable;
    Qt5Instance::AllocFakeCmdlineArgs(pFakeArgv, pFakeArgc, aFakeArgvFreeable);

    std::unique_ptr<QApplication> pQApp
        = Qt5Instance::CreateQApplication(*pFakeArgc, pFakeArgv.get());

    Qt5Instance* pInstance = new Qt5Instance(pQApp, bUseCairo);
    pInstance->MoveFakeCmdlineArgs(pFakeArgv, pFakeArgc, aFakeArgvFreeable);

    new Qt5Data(pInstance);

    return pInstance;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
