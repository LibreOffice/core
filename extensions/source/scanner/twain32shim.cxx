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

/*
 * twain32shim.exe is a separate 32-bit executable that serves as a shim
 * between LibreOffice and Windows' 32-bit TWAIN component. Without it,
 * it's impossible for 64-bit program to use TWAIN on Windows.
 * Using 64-bit TWAIN DSM library from twain.org to avoid using the shim
 * is not an option, because scanner manufacturers only provide 32-bit
 * drivers, and 64-bit drivers are only offered as 3rd-party commercial
 * products. The shim is also used in 32-bit LibreOffice for uniformity.
*/

#include "twain32shim.hxx"
#include <tools/helpers.hxx>
#include <twain/twain.h>

#define WM_TWAIN_FALLBACK (WM_SHIM_INTERNAL + 0)

namespace
{
long FixToLong(const TW_FIX32& rFix)
{
    return static_cast<long>(floor(rFix.Whole + rFix.Frac / 65536. + 0.5));
}

const wchar_t sTwainWndClass[] = L"TwainClass";

class ImpTwain
{
public:
    ImpTwain(HANDLE hParentThread);
    ~ImpTwain();

private:
    TW_IDENTITY m_aAppId;
    TW_IDENTITY m_aSrcId;
    DWORD m_nParentThreadId;
    HANDLE m_hProc;
    DSMENTRYPROC m_pDSM = nullptr;
    HMODULE m_hMod = nullptr;
    ULONG_PTR m_nCurState = 1;
    HWND m_hTwainWnd = nullptr;
    HHOOK m_hTwainHook = nullptr;
    HANDLE m_hMap = nullptr; // the *duplicated* handle

    static bool IsTwainClassWnd(HWND hWnd);
    static ImpTwain* GetImpFromWnd(HWND hWnd);
    static void ImplCreateWnd(HWND hWnd, LPARAM lParam);
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK MsgHook(int nCode, WPARAM wParam, LPARAM lParam);

    void Destroy();
    bool SelectSource();
    bool InitXfer();

    void NotifyParent(WPARAM nEvent, LPARAM lParam);
    bool ImplHandleMsg(MSG* pMsg);
    void ImplOpenSourceManager();
    void ImplOpenSource();
    bool ImplEnableSource();
    void ImplXfer();
    void ImplFallback(WPARAM nEvent);

    void ImplFallbackHdl(WPARAM nEvent);
    void ImplRequestHdl(WPARAM nRequest);
};

//static
bool ImpTwain::IsTwainClassWnd(HWND hWnd)
{
    const int nBufSize = SAL_N_ELEMENTS(sTwainWndClass);
    wchar_t sClassName[nBufSize];
    return (GetClassNameW(hWnd, sClassName, nBufSize) && wcscmp(sClassName, sTwainWndClass) == 0);
}

//static
ImpTwain* ImpTwain::GetImpFromWnd(HWND hWnd)
{
    if (!IsTwainClassWnd(hWnd))
        return nullptr;
    return reinterpret_cast<ImpTwain*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
}

//static
void ImpTwain::ImplCreateWnd(HWND hWnd, LPARAM lParam)
{
    CREATESTRUCT* pCS = reinterpret_cast<CREATESTRUCT*>(lParam);
    if (pCS && IsTwainClassWnd(hWnd))
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCS->lpCreateParams));
}

// static
LRESULT CALLBACK ImpTwain::WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    ImpTwain* pImpTwain = GetImpFromWnd(hWnd);
    switch (nMsg)
    {
        case WM_CREATE:
            ImplCreateWnd(hWnd, lParam);
            break;
        case WM_TWAIN_FALLBACK:
            if (pImpTwain)
                pImpTwain->ImplFallbackHdl(wParam);
            break;
        case WM_TWAIN_REQUEST:
            if (pImpTwain)
                pImpTwain->ImplRequestHdl(wParam);
            break;
    }
    return DefWindowProcW(hWnd, nMsg, wParam, lParam);
}

// static
LRESULT CALLBACK ImpTwain::MsgHook(int nCode, WPARAM wParam, LPARAM lParam)
{
    MSG* pMsg = reinterpret_cast<MSG*>(lParam);
    if (nCode >= 0 && pMsg)
    {
        ImpTwain* pImpTwain = GetImpFromWnd(pMsg->hwnd);
        if (pImpTwain && pImpTwain->ImplHandleMsg(pMsg))
        {
            pMsg->message = WM_USER;
            pMsg->lParam = 0;

            return 0;
        }
    }

    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

HANDLE GetProcOfThread(HANDLE hThread)
{
    DWORD nProcId = GetProcessIdOfThread(hThread);
    if (!nProcId)
        ThrowLastError("GetProcessIdOfThread");
    HANDLE hRet = OpenProcess(PROCESS_DUP_HANDLE, FALSE, nProcId);
    if (!hRet)
        ThrowLastError("OpenProcess");
    return hRet;
}

ImpTwain::ImpTwain(HANDLE hParentThread)
    : m_nParentThreadId(GetThreadId(hParentThread))
    , m_hProc(GetProcOfThread(hParentThread))
{
    m_aAppId.Id = 0;
    m_aAppId.Version.MajorNum = 1;
    m_aAppId.Version.MinorNum = 0;
    m_aAppId.Version.Language = TWLG_USA;
    m_aAppId.Version.Country = TWCY_USA;
    m_aAppId.ProtocolMajor = TWON_PROTOCOLMAJOR;
    m_aAppId.ProtocolMinor = TWON_PROTOCOLMINOR;
    m_aAppId.SupportedGroups = DG_IMAGE | DG_CONTROL;
    strncpy(m_aAppId.Version.Info, "8.0", 32);
    m_aAppId.Version.Info[32] = m_aAppId.Version.Info[33] = 0;
    strncpy(m_aAppId.Manufacturer, "Sun Microsystems", 32);
    m_aAppId.Manufacturer[32] = m_aAppId.Manufacturer[33] = 0;
    strncpy(m_aAppId.ProductFamily, "Office", 32);
    m_aAppId.ProductFamily[32] = m_aAppId.ProductFamily[33] = 0;
    strncpy(m_aAppId.ProductName, "Office", 32);
    m_aAppId.ProductName[32] = m_aAppId.ProductName[33] = 0;

    WNDCLASSW aWc = { 0,       &WndProc, 0,       sizeof(WNDCLASSW), GetModuleHandleW(nullptr),
                      nullptr, nullptr,  nullptr, nullptr,           sTwainWndClass };
    if (!RegisterClassW(&aWc))
        ThrowLastError("RegisterClassW");
    m_hTwainWnd = CreateWindowExW(WS_EX_TOPMOST, aWc.lpszClassName, L"TWAIN", 0, 0, 0, 0, 0,
                                  HWND_DESKTOP, nullptr, aWc.hInstance, this);
    if (!m_hTwainWnd)
        ThrowLastError("CreateWindowExW");
    m_hTwainHook = SetWindowsHookExW(WH_GETMESSAGE, &MsgHook, nullptr, GetCurrentThreadId());
    if (!m_hTwainHook)
        ThrowLastError("SetWindowsHookExW");

    NotifyParent(TWAIN_EVENT_NOTIFYHWND, reinterpret_cast<LPARAM>(m_hTwainWnd));
}

ImpTwain::~ImpTwain()
{
    DestroyWindow(m_hTwainWnd);
    UnhookWindowsHookEx(m_hTwainHook);
}

void ImpTwain::Destroy() { ImplFallback(TWAIN_EVENT_QUIT); }

bool ImpTwain::SelectSource()
{
    TW_UINT16 nRet = TWRC_FAILURE;

    ImplOpenSourceManager();

    if (3 == m_nCurState)
    {
        TW_IDENTITY aIdent;

        aIdent.Id = 0;
        aIdent.ProductName[0] = '\0';
        NotifyParent(TWAIN_EVENT_SCANNING, 0);
        nRet = m_pDSM(&m_aAppId, nullptr, DG_CONTROL, DAT_IDENTITY, MSG_USERSELECT, &aIdent);
    }

    ImplFallback(TWAIN_EVENT_QUIT);

    return (TWRC_SUCCESS == nRet);
}

bool ImpTwain::InitXfer()
{
    bool bRet = false;

    ImplOpenSourceManager();

    if (3 == m_nCurState)
    {
        ImplOpenSource();

        if (4 == m_nCurState)
            bRet = ImplEnableSource();
    }

    if (!bRet)
        ImplFallback(TWAIN_EVENT_QUIT);

    return bRet;
}

void ImpTwain::ImplOpenSourceManager()
{
    if (1 == m_nCurState)
    {
        if ((m_hMod = LoadLibraryW(L"TWAIN_32.DLL")))
        {
            m_nCurState = 2;

            m_pDSM = reinterpret_cast<DSMENTRYPROC>(GetProcAddress(m_hMod, "DSM_Entry"));
            if (m_pDSM
                && (m_pDSM(&m_aAppId, nullptr, DG_CONTROL, DAT_PARENT, MSG_OPENDSM, &m_hTwainWnd)
                    == TWRC_SUCCESS))
            {
                m_nCurState = 3;
            }
        }
    }
}

void ImpTwain::ImplOpenSource()
{
    if (3 == m_nCurState)
    {
        if ((m_pDSM(&m_aAppId, nullptr, DG_CONTROL, DAT_IDENTITY, MSG_GETDEFAULT, &m_aSrcId)
             == TWRC_SUCCESS)
            && (m_pDSM(&m_aAppId, nullptr, DG_CONTROL, DAT_IDENTITY, MSG_OPENDS, &m_aSrcId)
                == TWRC_SUCCESS))
        {
            TW_CAPABILITY aCap
                = { CAP_XFERCOUNT, TWON_ONEVALUE, GlobalAlloc(GHND, sizeof(TW_ONEVALUE)) };
            TW_ONEVALUE* pVal = static_cast<TW_ONEVALUE*>(GlobalLock(aCap.hContainer));

            pVal->ItemType = TWTY_INT16;
            pVal->Item = 1;
            GlobalUnlock(aCap.hContainer);
            m_pDSM(&m_aAppId, &m_aSrcId, DG_CONTROL, DAT_CAPABILITY, MSG_SET, &aCap);
            GlobalFree(aCap.hContainer);
            m_nCurState = 4;
        }
    }
}

bool ImpTwain::ImplEnableSource()
{
    bool bRet = false;

    if (4 == m_nCurState)
    {
        TW_USERINTERFACE aUI = { true, true, m_hTwainWnd };

        NotifyParent(TWAIN_EVENT_SCANNING, 0);
        m_nCurState = 5;

        if (m_pDSM(&m_aAppId, &m_aSrcId, DG_CONTROL, DAT_USERINTERFACE, MSG_ENABLEDS, &aUI)
            == TWRC_SUCCESS)
        {
            bRet = true;
        }
        else
        {
            // dialog failed
            m_nCurState = 4;
        }
    }

    return bRet;
}

void ImpTwain::NotifyParent(WPARAM nEvent, LPARAM lParam)
{
    PostThreadMessageW(m_nParentThreadId, WM_TWAIN_EVENT, nEvent, lParam);
}

bool ImpTwain::ImplHandleMsg(MSG* pMsg)
{
    TW_UINT16 nRet;
    TW_EVENT aEvt = { pMsg, MSG_NULL };

    if (m_pDSM)
        nRet = m_pDSM(&m_aAppId, &m_aSrcId, DG_CONTROL, DAT_EVENT, MSG_PROCESSEVENT, &aEvt);
    else
        nRet = TWRC_NOTDSEVENT;

    if (aEvt.TWMessage != MSG_NULL)
    {
        switch (aEvt.TWMessage)
        {
            case MSG_XFERREADY:
            {
                WPARAM nEvent = TWAIN_EVENT_QUIT;

                if (5 == m_nCurState)
                {
                    m_nCurState = 6;
                    ImplXfer();

                    if (m_hMap)
                        nEvent = TWAIN_EVENT_XFER;
                }
                else if (7 == m_nCurState && m_hMap)
                {
                    // Already sent TWAIN_EVENT_XFER; not processed yet;
                    // duplicate event
                    nEvent = TWAIN_EVENT_NONE;
                }

                ImplFallback(nEvent);
            }
            break;

            case MSG_CLOSEDSREQ:
                ImplFallback(TWAIN_EVENT_QUIT);
                break;

            default:
                break;
        }
    }
    else
        nRet = TWRC_NOTDSEVENT;

    return (TWRC_DSEVENT == nRet);
}

void ImpTwain::ImplXfer()
{
    if (m_nCurState == 6)
    {
        TW_IMAGEINFO aInfo;
        HANDLE hDIB = nullptr;
        long nWidth, nHeight, nXRes, nYRes;

        if (m_pDSM(&m_aAppId, &m_aSrcId, DG_IMAGE, DAT_IMAGEINFO, MSG_GET, &aInfo) == TWRC_SUCCESS)
        {
            nWidth = aInfo.ImageWidth;
            nHeight = aInfo.ImageLength;
            nXRes = FixToLong(aInfo.XResolution);
            nYRes = FixToLong(aInfo.YResolution);
        }
        else
            nWidth = nHeight = nXRes = nYRes = -1;

        switch (m_pDSM(&m_aAppId, &m_aSrcId, DG_IMAGE, DAT_IMAGENATIVEXFER, MSG_GET, &hDIB))
        {
            case TWRC_CANCEL:
                m_nCurState = 7;
                break;

            case TWRC_XFERDONE:
            {
                if (hDIB)
                {
                    m_hMap = nullptr;
                    const HGLOBAL hGlob = static_cast<HGLOBAL>(hDIB);
                    const SIZE_T nDIBSize = GlobalSize(hGlob);
                    const DWORD nMapSize = nDIBSize + 4; // leading 4 bytes for size
                    if (nMapSize > nDIBSize) // check for wrap
                    {
                        if (LPVOID pBmpMem = GlobalLock(hGlob))
                        {
                            if ((nXRes != -1) && (nYRes != -1) && (nWidth != -1) && (nHeight != -1))
                            {
                                // set resolution of bitmap
                                BITMAPINFOHEADER* pBIH = static_cast<BITMAPINFOHEADER*>(pBmpMem);
                                static const double fFactor = 100.0 / 2.54;

                                pBIH->biXPelsPerMeter = FRound(fFactor * nXRes);
                                pBIH->biYPelsPerMeter = FRound(fFactor * nYRes);
                            }

                            HANDLE hMap = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr,
                                                             PAGE_READWRITE, 0, nMapSize, nullptr);
                            if (hMap)
                            {
                                LPVOID pMap = MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, nMapSize);
                                if (pMap)
                                {
                                    memcpy(pMap, &nMapSize, 4); // size of the following DIB
                                    memcpy(static_cast<char*>(pMap) + 4, pBmpMem, nDIBSize);
                                    FlushViewOfFile(pMap, nDIBSize);
                                    UnmapViewOfFile(pMap);

                                    DuplicateHandle(GetCurrentProcess(), hMap, m_hProc, &m_hMap, 0,
                                                    FALSE, DUPLICATE_SAME_ACCESS);
                                }

                                CloseHandle(hMap);
                            }

                            GlobalUnlock(hGlob);
                        }
                    }
                }

                GlobalFree(static_cast<HGLOBAL>(hDIB));

                m_nCurState = 7;
            }
            break;

            default:
                break;
        }
    }
}

void ImpTwain::ImplFallback(WPARAM nEvent)
{
    PostMessageW(m_hTwainWnd, WM_TWAIN_FALLBACK, nEvent, 0);
}

void ImpTwain::ImplFallbackHdl(WPARAM nEvent)
{
    bool bFallback = true;

    switch (m_nCurState)
    {
        case 7:
        case 6:
        {
            TW_PENDINGXFERS aXfers;

            if (m_pDSM(&m_aAppId, &m_aSrcId, DG_CONTROL, DAT_PENDINGXFERS, MSG_ENDXFER, &aXfers)
                == TWRC_SUCCESS)
            {
                if (aXfers.Count != 0)
                    m_pDSM(&m_aAppId, &m_aSrcId, DG_CONTROL, DAT_PENDINGXFERS, MSG_RESET, &aXfers);
            }

            m_nCurState = 5;
        }
        break;

        case 5:
        {
            TW_USERINTERFACE aUI = { true, true, m_hTwainWnd };

            m_pDSM(&m_aAppId, &m_aSrcId, DG_CONTROL, DAT_USERINTERFACE, MSG_DISABLEDS, &aUI);
            m_nCurState = 4;
        }
        break;

        case 4:
        {
            m_pDSM(&m_aAppId, nullptr, DG_CONTROL, DAT_IDENTITY, MSG_CLOSEDS, &m_aSrcId);
            m_nCurState = 3;
        }
        break;

        case 3:
        {
            m_pDSM(&m_aAppId, nullptr, DG_CONTROL, DAT_PARENT, MSG_CLOSEDSM, &m_hTwainWnd);
            m_nCurState = 2;
        }
        break;

        case 2:
        {
            m_pDSM = nullptr;
            FreeLibrary(m_hMod);
            m_hMod = nullptr;
            m_nCurState = 1;
        }
        break;

        default:
        {
            if (nEvent > TWAIN_EVENT_NONE)
                NotifyParent(nEvent, reinterpret_cast<LPARAM>(m_hMap));
            if (nEvent == TWAIN_EVENT_QUIT)
                PostQuitMessage(0);

            bFallback = false;
        }
        break;
    }

    if (bFallback)
        ImplFallback(nEvent);
}

void ImpTwain::ImplRequestHdl(WPARAM nRequest)
{
    switch (nRequest)
    {
        case TWAIN_REQUEST_QUIT:
            Destroy();
            break;
        case TWAIN_REQUEST_SELECTSOURCE:
            NotifyParent(TWAIN_EVENT_REQUESTRESULT, LPARAM(SelectSource()));
            break;
        case TWAIN_REQUEST_INITXFER:
            NotifyParent(TWAIN_EVENT_REQUESTRESULT, LPARAM(InitXfer()));
            break;
    }
}
} // namespace

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argc != 2)
        return 1; // Wrong argument count
    // 1st argument is parent thread handle; must be inherited.
    // HANDLE is 32-bit in 32-bit applications, so wcstoul is OK.
    HANDLE hParentThread = reinterpret_cast<HANDLE>(wcstoul(argv[1], nullptr, 10));
    LocalFree(argv);
    if (!hParentThread)
        return 2; // Invalid parent thread handle argument value

    int nRet = 0;
    try
    {
        ImpTwain aImpTwain(hParentThread); // creates main window

        MSG msg;
        while (true)
        {
            DWORD nWaitResult
                = MsgWaitForMultipleObjects(1, &hParentThread, FALSE, INFINITE, QS_ALLINPUT);
            if (nWaitResult == WAIT_OBJECT_0)
                return 5; // Parent process' thread died before we exited
            if (nWaitResult == WAIT_FAILED)
                return 6; // Some Win32 error
            // nWaitResult == WAIT_OBJECT_0 + nCount => an event is in queue
            bool bQuit = false;
            while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                // process it here
                if (msg.message == WM_QUIT)
                {
                    bQuit = true;
                    nRet = msg.wParam;
                }
                else
                {
                    TranslateMessage(&msg);
                    DispatchMessageW(&msg);
                }
            }
            if (bQuit)
                break;
        }
    }
    catch (const std::exception& e)
    {
        printf("Exception thrown: %s", e.what());
        nRet = 7;
    }
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
