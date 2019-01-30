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

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/Desktop.hpp>

#include "twain32shim.hxx"

#include <comphelper/processfactory.hxx>
#include <comphelper/scopeguard.hxx>
#include <config_folders.h>
#include <o3tl/char16_t2wchar_t.hxx>
#include <osl/conditn.hxx>
#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <rtl/bootstrap.hxx>
#include <salhelper/thread.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/stream.hxx>
#include <tools/helpers.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include "scanner.hxx"

namespace
{

enum TwainState
{
    TWAIN_STATE_NONE = 0,
    TWAIN_STATE_SCANNING = 1,
    TWAIN_STATE_DONE = 2,
    TWAIN_STATE_CANCELED = 3
};

struct HANDLEDeleter
{
    using pointer = HANDLE;
    void operator()(HANDLE h) { CloseHandle(h); }
};

using ScopedHANDLE = std::unique_ptr<HANDLE, HANDLEDeleter>;

class Twain
{
public:
    Twain();
    ~Twain();

    bool SelectSource(ScannerManager& rMgr, const VclPtr<vcl::Window>& xTopWindow);
    bool PerformTransfer(ScannerManager& rMgr,
                         const css::uno::Reference<css::lang::XEventListener>& rxListener,
                         const VclPtr<vcl::Window>& xTopWindow);
    void WaitReadyForNextTask();

    TwainState GetState() const { return meState; }

private:
    friend class ShimListenerThread;
    class ShimListenerThread : public salhelper::Thread
    {
    public:
        ShimListenerThread(const VclPtr<vcl::Window>& xTopWindow);
        ~ShimListenerThread() override;
        void execute() override;
        const OUString& getError() { return msErrorReported; }

        // These methods are executed outside of own thread
        bool WaitInitialization();
        bool WaitRequestResult();
        void DontNotify() { mbDontNotify = true; }
        void RequestDestroy();
        bool RequestSelectSource();
        bool RequestInitXfer();

    private:
        VclPtr<vcl::Window> mxTopWindow; // the window that we made modal
        bool mbDontNotify = false;
        HWND mhWndShim = nullptr; // shim main window handle
        OUString msErrorReported;
        osl::Condition mcInitCompleted; // initially not set
        bool mbInitSucceeded = false;
        osl::Condition mcGotRequestResult;
        bool mbRequestResult = false;

        void SendShimRequest(WPARAM nRequest);
        bool SendShimRequestWithResult(WPARAM nRequest);
        void NotificationHdl(WPARAM nEvent, LPARAM lParam);
        void NotifyOwner(WPARAM nEvent);
        void NotifyXFerOwner(LPARAM nHandle);
    };
    css::uno::Reference<css::lang::XEventListener> mxListener;
    css::uno::Reference<css::scanner::XScannerManager> mxMgr;
    ScannerManager* mpCurMgr = nullptr;
    TwainState meState = TWAIN_STATE_NONE;
    rtl::Reference<ShimListenerThread> mpThread;
    osl::Mutex maMutex;

    DECL_LINK(ImpNotifyHdl, void*, void);
    DECL_LINK(ImpNotifyXferHdl, void*, void);
    void Notify(WPARAM nEvent); // called by shim communication thread to notify me
    void NotifyXFer(LPARAM nHandle); // called by shim communication thread to notify me

    bool InitializeNewShim(ScannerManager& rMgr, const VclPtr<vcl::Window>& xTopWindow);

    void Reset(); // cleanup thread and manager
};

static Twain aTwain;

Twain::ShimListenerThread::ShimListenerThread(const VclPtr<vcl::Window>& xTopWindow)
    : salhelper::Thread("TWAINShimListenerThread")
    , mxTopWindow(xTopWindow)
{
    if (mxTopWindow)
    {
        mxTopWindow->IncModalCount(); // the operation is modal to the frame
    }
}

Twain::ShimListenerThread::~ShimListenerThread()
{
    if (mxTopWindow)
    {
        mxTopWindow->DecModalCount(); // unblock the frame
    }
}

bool Twain::ShimListenerThread::WaitInitialization()
{
    mcInitCompleted.wait();
    return mbInitSucceeded;
}

bool Twain::ShimListenerThread::WaitRequestResult()
{
    mcGotRequestResult.wait();
    return mbRequestResult;
}

void Twain::ShimListenerThread::SendShimRequest(WPARAM nRequest)
{
    if (mhWndShim)
        PostMessageW(mhWndShim, WM_TWAIN_REQUEST, nRequest, 0);
}

bool Twain::ShimListenerThread::SendShimRequestWithResult(WPARAM nRequest)
{
    mcGotRequestResult.reset();
    mbRequestResult = false;
    SendShimRequest(nRequest);
    return WaitRequestResult();
}

void Twain::ShimListenerThread::RequestDestroy() { SendShimRequest(TWAIN_REQUEST_QUIT); }

bool Twain::ShimListenerThread::RequestSelectSource()
{
    assert(mbInitSucceeded);
    return SendShimRequestWithResult(TWAIN_REQUEST_SELECTSOURCE);
}

bool Twain::ShimListenerThread::RequestInitXfer()
{
    assert(mbInitSucceeded);
    return SendShimRequestWithResult(TWAIN_REQUEST_INITXFER);
}

void Twain::ShimListenerThread::NotifyOwner(WPARAM nEvent)
{
    if (!mbDontNotify)
        aTwain.Notify(nEvent);
}

void Twain::ShimListenerThread::NotifyXFerOwner(LPARAM nHandle)
{
    if (!mbDontNotify)
        aTwain.NotifyXFer(nHandle);
}

// May only be called from the own thread, so no threading issues modifying self
void Twain::ShimListenerThread::NotificationHdl(WPARAM nEvent, LPARAM lParam)
{
    switch (nEvent)
    {
        case TWAIN_EVENT_NOTIFYHWND: // shim reported its main HWND for communications
            if (!mcInitCompleted.check()) // only if not yet initialized!
            {
                // Owner is still waiting mcInitCompleted in its Twain::InitializeNewShim,
                // holding its access mutex
                mhWndShim = reinterpret_cast<HWND>(lParam);

                mbInitSucceeded = lParam != 0;
                mcInitCompleted.set();
            }
            break;
        case TWAIN_EVENT_SCANNING:
            NotifyOwner(nEvent);
            break;
        case TWAIN_EVENT_XFER:
            NotifyXFerOwner(lParam);
            break;
        case TWAIN_EVENT_REQUESTRESULT:
            mbRequestResult = lParam;
            mcGotRequestResult.set();
            break;
            // We don't handle TWAIN_EVENT_QUIT notification from shim, because we send it ourselves
            // in the end of execute()
    }
}

// Spawn a separate 32-bit process to use TWAIN on Windows, and listen for its notifications
void Twain::ShimListenerThread::execute()
{
    MSG msg;
    // Initialize thread message queue before launching shim process
    PeekMessageW(&msg, nullptr, 0, 0, PM_NOREMOVE);

    try
    {
        ScopedHANDLE hShimProcess;
        {
            // Determine twain32shim executable URL:
            OUString shimURL("$BRAND_BASE_DIR/" LIBO_BIN_FOLDER "/twain32shim.exe");
            rtl::Bootstrap::expandMacros(shimURL);

            OUString sCmdLine;
            if (osl::FileBase::getSystemPathFromFileURL(shimURL, sCmdLine) != osl::FileBase::E_None)
                throw std::exception("getSystemPathFromFileURL failed!");

            HANDLE hDup;
            if (!DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(),
                                 &hDup, SYNCHRONIZE | THREAD_QUERY_LIMITED_INFORMATION, TRUE, 0))
                ThrowLastError("DuplicateHandle");
            // we will not need our copy as soon as shim has its own inherited one
            ScopedHANDLE hScopedDup(hDup);
            DWORD nDup = reinterpret_cast<DWORD>(hDup);
            if (reinterpret_cast<HANDLE>(nDup) != hDup)
                throw std::exception("HANDLE does not fit to 32 bit - cannot pass to shim!");

            // Send this thread handle as the first parameter
            sCmdLine = "\"" + sCmdLine + "\" " + OUString::number(nDup);

            // We need a WinAPI HANDLE of the process to be able to wait on it and detect the process
            // termination; so use WinAPI to start the process, not osl_executeProcess.

            STARTUPINFOW si{ sizeof(si) }; // null-initialize all but cb
            PROCESS_INFORMATION pi;

            if (!CreateProcessW(nullptr, const_cast<LPWSTR>(o3tl::toW(sCmdLine.getStr())), nullptr,
                                nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
                ThrowLastError("CreateProcessW");

            CloseHandle(pi.hThread);
            hShimProcess.reset(pi.hProcess);
        }
        HANDLE h = hShimProcess.get();
        while (true)
        {
            DWORD nWaitResult = MsgWaitForMultipleObjects(1, &h, FALSE, INFINITE, QS_POSTMESSAGE);
            // Process any messages in queue before checking if we need to break, to not loose
            // possible pending notifications
            while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                // process it here
                if (msg.message == WM_TWAIN_EVENT)
                {
                    NotificationHdl(msg.wParam, msg.lParam);
                }
            }
            if (nWaitResult == WAIT_OBJECT_0)
            {
                // shim process exited - return
                break;
            }
            if (nWaitResult == WAIT_FAILED)
            {
                // Some Win32 error - report and return
                ThrowLastError("MsgWaitForMultipleObjects");
            }
        }
    }
    catch (const std::exception& e)
    {
        msErrorReported = OUString(e.what(), strlen(e.what()), RTL_TEXTENCODING_UTF8);
        // allow owner to resume (in case the condition isn't set yet)
        mcInitCompleted.set(); // let mbInitSucceeded keep its (maybe false) value!
    }
    // allow owner to resume (in case the conditions isn't set yet)
    mcGotRequestResult.set();
    NotifyOwner(TWAIN_EVENT_QUIT);
}

Twain::Twain() {}

Twain::~Twain()
{
    osl::MutexGuard aGuard(maMutex);
    if (mpThread)
    {
        mpThread->DontNotify();
        mpThread->RequestDestroy();
        mpThread->join();
        mpThread.clear();
    }
}

void Twain::Reset()
{
    mpThread->join();
    if (!mpThread->getError().isEmpty())
        SAL_WARN("extensions.scanner", mpThread->getError());
    mpThread.clear();
    mpCurMgr = nullptr;
    mxMgr.clear();
}

bool Twain::InitializeNewShim(ScannerManager& rMgr, const VclPtr<vcl::Window>& xTopWindow)
{
    osl::MutexGuard aGuard(maMutex);
    if (mpThread)
        return false; // Have a shim for another task already!

    // hold reference to ScannerManager, to prevent premature death
    mxMgr.set(static_cast<OWeakObject*>(mpCurMgr = &rMgr),
              css::uno::UNO_QUERY);

    mpThread.set(new ShimListenerThread(xTopWindow));
    mpThread->launch();
    const bool bSuccess = mpThread->WaitInitialization();
    if (!bSuccess)
        Reset();

    return bSuccess;
}

void Twain::Notify(WPARAM nEvent)
{
    Application::PostUserEvent(LINK(this, Twain, ImpNotifyHdl), reinterpret_cast<void*>(nEvent));
}

void Twain::NotifyXFer(LPARAM nHandle)
{
    Application::PostUserEvent(LINK(this, Twain, ImpNotifyXferHdl),
                               reinterpret_cast<void*>(nHandle));
}

bool Twain::SelectSource(ScannerManager& rMgr, const VclPtr<vcl::Window>& xTopWindow)
{
    osl::MutexGuard aGuard(maMutex);
    bool bRet = false;

    if (InitializeNewShim(rMgr, xTopWindow))
    {
        meState = TWAIN_STATE_NONE;
        bRet = mpThread->RequestSelectSource();
    }

    return bRet;
}

bool Twain::PerformTransfer(ScannerManager& rMgr,
                            const css::uno::Reference<css::lang::XEventListener>& rxListener,
                            const VclPtr<vcl::Window>& xTopWindow)
{
    osl::MutexGuard aGuard(maMutex);
    bool bRet = false;

    if (InitializeNewShim(rMgr, xTopWindow))
    {
        mxListener = rxListener;
        meState = TWAIN_STATE_NONE;
        bRet = mpThread->RequestInitXfer();
    }

    return bRet;
}

void Twain::WaitReadyForNextTask()
{
    while ([&]() {
        osl::MutexGuard aGuard(maMutex);
        return bool(mpThread);
    }())
    {
        Application::Reschedule(true);
    }
}

IMPL_LINK(Twain, ImpNotifyHdl, void*, pParam, void)
{
    osl::MutexGuard aGuard(maMutex);
    WPARAM nEvent = reinterpret_cast<WPARAM>(pParam);
    switch (nEvent)
    {
        case TWAIN_EVENT_SCANNING:
            meState = TWAIN_STATE_SCANNING;
            break;

        case TWAIN_EVENT_QUIT:
        {
            if (meState != TWAIN_STATE_DONE)
                meState = TWAIN_STATE_CANCELED;

            css::lang::EventObject event(mxMgr); // mxMgr will be cleared below

            if (mpThread)
                Reset();

            if (mxListener.is())
            {
                mxListener->disposing(event);
                mxListener.clear();
            }
        }
        break;

        default:
            break;
    }
}

IMPL_LINK(Twain, ImpNotifyXferHdl, void*, pParam, void)
{
    osl::MutexGuard aGuard(maMutex);
    if (mpThread)
    {
        mpCurMgr->SetData(pParam);
        meState = pParam ? TWAIN_STATE_DONE : TWAIN_STATE_CANCELED;

        css::lang::EventObject event(mxMgr); // mxMgr will be cleared below

        Reset();

        if (mxListener.is())
            mxListener->disposing(css::lang::EventObject(mxMgr));
    }

    mxListener.clear();
}

VclPtr<vcl::Window> ImplGetActiveFrameWindow()
{
    try
    {
        // query desktop instance
        css::uno::Reference<css::frame::XDesktop2> xDesktop
            = css::frame::Desktop::create(comphelper::getProcessComponentContext());
        if (css::uno::Reference<css::frame::XFrame> xActiveFrame = xDesktop->getActiveFrame())
            return VCLUnoHelper::GetWindow(xActiveFrame->getComponentWindow());
    }
    catch (const css::uno::Exception&)
    {
    }
    SAL_WARN("extensions.scanner", "ImplGetActiveFrame: Could not determine active frame!");
    return nullptr;
}

} // namespace

void ScannerManager::AcquireData() {}

void ScannerManager::ReleaseData()
{
    if (mpData)
    {
        CloseHandle(static_cast<HANDLE>(mpData));
        mpData = nullptr;
    }
}

css::awt::Size ScannerManager::getSize()
{
    css::awt::Size aRet;

    if (mpData)
    {
        HANDLE hMap = static_cast<HANDLE>(mpData);
        // map full size
        const sal_Int8* pMap = static_cast<sal_Int8*>(MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0));
        if (pMap)
        {
            const BITMAPINFOHEADER* pBIH = reinterpret_cast<const BITMAPINFOHEADER*>(pMap + 4);
            aRet.Width = pBIH->biWidth;
            aRet.Height = pBIH->biHeight;

            UnmapViewOfFile(pMap);
        }
    }

    return aRet;
}

css::uno::Sequence<sal_Int8> ScannerManager::getDIB()
{
    css::uno::Sequence<sal_Int8> aRet;

    if (mpData)
    {
        HANDLE hMap = static_cast<HANDLE>(mpData);
        // map full size
        const sal_Int8* pMap = static_cast<sal_Int8*>(MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0));
        if (pMap)
        {
            DWORD nDIBSize;
            memcpy(&nDIBSize, pMap, 4); // size of the following DIB

            const BITMAPINFOHEADER* pBIH = reinterpret_cast<const BITMAPINFOHEADER*>(pMap + 4);

            sal_uInt32 nColEntries = 0;

            switch (pBIH->biBitCount)
            {
                case 1:
                case 4:
                case 8:
                    nColEntries = pBIH->biClrUsed ? pBIH->biClrUsed : (1 << pBIH->biBitCount);
                    break;

                case 24:
                    nColEntries = pBIH->biClrUsed ? pBIH->biClrUsed : 0;
                    break;

                case 16:
                case 32:
                    nColEntries = pBIH->biClrUsed;
                    if (pBIH->biCompression == BI_BITFIELDS)
                        nColEntries += 3;
                    break;
            }

            aRet = css::uno::Sequence<sal_Int8>(sizeof(BITMAPFILEHEADER) + nDIBSize);

            sal_Int8* pBuf = aRet.getArray();
            SvMemoryStream* pMemStm
                = new SvMemoryStream(pBuf, sizeof(BITMAPFILEHEADER), StreamMode::WRITE);

            pMemStm->WriteChar('B').WriteChar('M').WriteUInt32(0).WriteUInt32(0);
            pMemStm->WriteUInt32(sizeof(BITMAPFILEHEADER) + pBIH->biSize
                                 + (nColEntries * sizeof(RGBQUAD)));

            delete pMemStm;
            memcpy(pBuf + sizeof(BITMAPFILEHEADER), pBIH, nDIBSize);

            UnmapViewOfFile(pMap);
        }

        ReleaseData();
    }

    return aRet;
}

css::uno::Sequence<ScannerContext> SAL_CALL ScannerManager::getAvailableScanners()
{
    osl::MutexGuard aGuard(maProtector);
    css::uno::Sequence<ScannerContext> aRet(1);

    aRet.getArray()[0].ScannerName = "TWAIN";
    aRet.getArray()[0].InternalData = 0;

    return aRet;
}

sal_Bool SAL_CALL ScannerManager::configureScannerAndScan(
    ScannerContext& rContext, const css::uno::Reference<css::lang::XEventListener>& rxListener)
{
    osl::MutexGuard aGuard(maProtector);
    css::uno::Reference<XScannerManager> xThis(this);

    if (rContext.InternalData != 0 || rContext.ScannerName != "TWAIN")
        throw ScannerException("Scanner does not exist", xThis, ScanError_InvalidContext);

    ReleaseData();

    VclPtr<vcl::Window> xTopWindow = ImplGetActiveFrameWindow();
    if (xTopWindow)
        xTopWindow->IncModalCount(); // to avoid changes between the two operations that each
                                     // block the window
    comphelper::ScopeGuard aModalGuard([xTopWindow]() {
        if (xTopWindow)
            xTopWindow->DecModalCount();
    });

    const bool bSelected = aTwain.SelectSource(*this, xTopWindow);
    if (bSelected)
    {
        aTwain.WaitReadyForNextTask();
        aTwain.PerformTransfer(*this, rxListener, xTopWindow);
    }
    return bSelected;
}

void SAL_CALL
ScannerManager::startScan(const ScannerContext& rContext,
                          const css::uno::Reference<css::lang::XEventListener>& rxListener)
{
    osl::MutexGuard aGuard(maProtector);
    css::uno::Reference<XScannerManager> xThis(this);

    if (rContext.InternalData != 0 || rContext.ScannerName != "TWAIN")
        throw ScannerException("Scanner does not exist", xThis, ScanError_InvalidContext);

    ReleaseData();
    aTwain.PerformTransfer(*this, rxListener, ImplGetActiveFrameWindow());
}

ScanError SAL_CALL ScannerManager::getError(const ScannerContext& rContext)
{
    osl::MutexGuard aGuard(maProtector);
    css::uno::Reference<XScannerManager> xThis(this);

    if (rContext.InternalData != 0 || rContext.ScannerName != "TWAIN")
        throw ScannerException("Scanner does not exist", xThis, ScanError_InvalidContext);

    return ((aTwain.GetState() == TWAIN_STATE_CANCELED) ? ScanError_ScanCanceled
                                                        : ScanError_ScanErrorNone);
}

css::uno::Reference<css::awt::XBitmap>
    SAL_CALL ScannerManager::getBitmap(const ScannerContext& /*rContext*/)
{
    osl::MutexGuard aGuard(maProtector);
    return css::uno::Reference<css::awt::XBitmap>(this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
