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

#include "dibpreview.hxx"
#include <osl/diagnose.h>

#include <com/sun/star/ui/dialogs/FilePreviewImageFormats.hpp>

#include <rtl/ustring.hxx>

#include <stdexcept>
#include <string>


using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Any;
using ::com::sun::star::lang::IllegalArgumentException;


namespace /* private */
{
    const PCWSTR CURRENT_INSTANCE = L"CurrInst";
};

#define PREVIEWWND_CLASS_NAME L"DIBPreviewWnd###"


// static member initialization


osl::Mutex CDIBPreview::s_Mutex;
ATOM CDIBPreview::s_ClassAtom = 0;
sal_Int32 CDIBPreview::s_RegisterDibPreviewWndCount = 0;


CDIBPreview::CDIBPreview(HINSTANCE instance,HWND parent,bool bShowWindow) :
    m_Instance(instance)
{
    RegisterDibPreviewWindowClass();

    DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    if (bShowWindow)
        dwStyle |= WS_VISIBLE;

    m_Hwnd = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        PREVIEWWND_CLASS_NAME,
        L"",
        dwStyle,
        0, 0, 0, 0,
        parent,
        nullptr,    // for child windows this will
                    // be used as child window identifier
        m_Instance,
        this         // pass a pointer to the current
                     // instance of this class
    );

    if (!IsWindow(m_Hwnd))
    {
        SAL_WARN("fpicker","Could not create preview window");
        UnregisterDibPreviewWindowClass();
        throw std::runtime_error("Could not create preview window");
    }
}


CDIBPreview::~CDIBPreview( )
{
    // remember: we don't have to destroy the
    // preview window because it will be destroyed
    // by its parent window (the FileOpen dialog)
    // but we have to unregister the window class
    //if ( m_bWndClassRegistered )
    UnregisterDibPreviewWindowClass();
}


sal_Int32 SAL_CALL CDIBPreview::getTargetColorDepth()
{
    HDC hdc = GetDC(m_Hwnd);
    int clrRes = 0;

    if (hdc)
        clrRes = GetDeviceCaps(hdc, COLORRES);

    return clrRes;
}


sal_Int32 SAL_CALL CDIBPreview::getAvailableWidth()
{
    RECT rect;
    bool bRet = GetClientRect(m_Hwnd,&rect);

    sal_Int32 cx = 0;

    if ( bRet )
        cx = rect.right;

    return cx;
}


sal_Int32 SAL_CALL CDIBPreview::getAvailableHeight()
{
    RECT rect;
    bool bRet = GetClientRect(m_Hwnd,&rect);

    sal_Int32 cy = 0;

    if ( bRet )
        cy = rect.bottom;

    return cy;
}


void SAL_CALL CDIBPreview::setImage(sal_Int16 aImageFormat, const Any& aImage)
{
    PreviewBase::setImage(aImageFormat,aImage);

    // if the any has no value we have an
    // empty Sequence which clears the
    // preview window
    osl::ClearableMutexGuard aGuard(m_PaintLock);

    m_Image.realloc(0);
    m_ImageData >>= m_Image;

    aGuard.clear();

    InvalidateRect(m_Hwnd,nullptr,false);
    UpdateWindow(m_Hwnd);
}


bool SAL_CALL CDIBPreview::setShowState(bool bShowState)
{
    PreviewBase::setShowState(bShowState);
    ShowWindow(m_Hwnd, m_bShowState ? SW_SHOW : SW_HIDE);
    return true;
}


bool SAL_CALL CDIBPreview::getShowState()
{
    return bool(IsWindowVisible(m_Hwnd));
}


HWND SAL_CALL CDIBPreview::getWindowHandle() const
{
    return m_Hwnd;
}


void SAL_CALL CDIBPreview::onPaint(HWND hWnd, HDC hDC)
{
    osl::MutexGuard aGuard(m_PaintLock);

    try
    {
        BITMAPFILEHEADER* pbmfh = reinterpret_cast<BITMAPFILEHEADER*>(m_Image.getArray());

        if ( !IsBadReadPtr( pbmfh, sizeof(BITMAPFILEHEADER)) &&
             (pbmfh->bfType == ('B' | ('M' << 8))) )
        {
            BITMAPINFO * pbmi  = reinterpret_cast<BITMAPINFO*>((pbmfh + 1));
            sal_uInt8 * pBits = reinterpret_cast<sal_uInt8*>(pbmfh) + pbmfh->bfOffBits;

            int cxDib = pbmi->bmiHeader.biWidth;
            int cyDib = abs (pbmi->bmiHeader.biHeight);

            SetStretchBltMode(hDC, COLORONCOLOR);

            int nWidth  = getAvailableWidth();
            int nHeight = getAvailableHeight();

            int nX = abs(nWidth - cxDib) / 2;
            int nY = abs(nHeight - cyDib) / 2;

            int GDIError = StretchDIBits(
                hDC, nX, nY, cxDib, cyDib,
                0, 0, cxDib, cyDib, pBits, pbmi,
                DIB_RGB_COLORS, SRCCOPY);

            OSL_ASSERT(0 != GDIError);

            // paint the border
            RECT rc;

            if (nY > 0)
            {
                // top
                rc.left   = 0;
                rc.top    = 0;
                rc.right  = nWidth;
                rc.bottom = nY;
                FillRect(hDC,&rc,reinterpret_cast<HBRUSH>(COLOR_INACTIVEBORDER + 1));

                // bottom
                rc.left   = 0;
                rc.top    = nHeight - nY - 1;
                rc.right  = nWidth;
                rc.bottom = nHeight;
                FillRect(hDC,&rc,reinterpret_cast<HBRUSH>(COLOR_INACTIVEBORDER + 1));
            }

            if (nX > 0)
            {
                // left
                rc.left   = 0;
                rc.top    = nY;
                rc.right  = nX;
                rc.bottom = nHeight - nY;
                FillRect(hDC,&rc,reinterpret_cast<HBRUSH>(COLOR_INACTIVEBORDER + 1));

                // right
                rc.left   = nWidth - nX - 1;
                rc.top    = nY;
                rc.right  = nWidth;
                rc.bottom = nHeight - nY;
                FillRect(hDC,&rc,reinterpret_cast<HBRUSH>(COLOR_INACTIVEBORDER + 1));
            }
        }
        else // clear background
        {
            RECT rc;
            GetClientRect(hWnd,&rc);
            FillRect(hDC,&rc,reinterpret_cast<HBRUSH>(COLOR_INACTIVEBORDER + 1));
        }
    }
    catch(...)
    {
        OSL_ASSERT(false);
    }
}


LRESULT CALLBACK CDIBPreview::WndProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;

    switch(uMsg)
    {

    // we connect a pointer to the current instance
    // with a window instance via SetProp
    case WM_CREATE:
        {
            LPCREATESTRUCT lpcs =
                reinterpret_cast< LPCREATESTRUCT >(lParam);

            OSL_ASSERT(lpcs->lpCreateParams);

            // connect the instance handle to the window
            SetPropW(hWnd, CURRENT_INSTANCE, lpcs->lpCreateParams);
        }
        break;

    // we remove the window property which connects
    // a class instance with a window class
    case WM_NCDESTROY:
        {
            // RemoveProp returns the saved value on success
            if (static_cast<CDIBPreview*>(
                    RemovePropW(hWnd, CURRENT_INSTANCE)) == nullptr)
            {
                OSL_ASSERT(false);
            }
        }
        break;

    case WM_PAINT:
    {
        CDIBPreview* pImpl = static_cast<CDIBPreview*>(
            GetPropW(hWnd, CURRENT_INSTANCE));

        OSL_ASSERT(pImpl);

        HDC         hDC;
        PAINTSTRUCT ps;

        hDC = BeginPaint(hWnd,&ps);
        pImpl->onPaint(hWnd,hDC);
        EndPaint(hWnd,&ps);
    }
    break;

    // ignore this message in order to
    // avoid flickering during paint
    case WM_ERASEBKGND:
        lResult = 1;
        break;

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return lResult;
}


ATOM SAL_CALL CDIBPreview::RegisterDibPreviewWindowClass()
{
    osl::MutexGuard aGuard( s_Mutex );

    if (0 == s_ClassAtom)
    {
        // register the preview window class
        WNDCLASSEX wndClsEx;
        ZeroMemory(&wndClsEx, sizeof(wndClsEx));

        wndClsEx.cbSize        = sizeof(wndClsEx);
        wndClsEx.style         = CS_HREDRAW | CS_VREDRAW;
        wndClsEx.lpfnWndProc   = CDIBPreview::WndProc;
        wndClsEx.hInstance     = m_Instance;
        wndClsEx.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_INACTIVEBORDER + 1);
        wndClsEx.lpszClassName = PREVIEWWND_CLASS_NAME;

        // register the preview window class
        // !!! Win95 -   the window class will be unregistered automatically
        //               if the dll is unloaded
        //     Win2000 - the window class must be unregistered manually
        //               if the dll is unloaded
        s_ClassAtom = RegisterClassExW(&wndClsEx);
        if (0 == s_ClassAtom)
        {
            SAL_WARN("fpicker", "Could not register preview window class");
            throw std::runtime_error("Preview window class could not be registered");
        }
    }

    // increment the register class counter
    // so that we keep track of the number
    // of class registrations
    //if ( 0 != s_ClassAtom )
    s_RegisterDibPreviewWndCount++;

    return s_ClassAtom;
}


void SAL_CALL CDIBPreview::UnregisterDibPreviewWindowClass()
{
    osl::MutexGuard aGuard( s_Mutex );

    OSL_ASSERT( ( (0 != s_ClassAtom) && (s_RegisterDibPreviewWndCount > 0)) ||
                ( (0 == s_ClassAtom) && (0 == s_RegisterDibPreviewWndCount) ) );

    // update the register class counter
    // and unregister the window class if
    // counter drops to zero
    if (0 != s_ClassAtom)
    {
        s_RegisterDibPreviewWndCount--;
        OSL_ASSERT(s_RegisterDibPreviewWndCount >= 0);
    }

    if (0 == s_RegisterDibPreviewWndCount)
    {
        UnregisterClassW(reinterpret_cast<PCWSTR>((DWORD_PTR)MAKELONG(s_ClassAtom,0)),m_Instance);
        s_ClassAtom = 0;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
