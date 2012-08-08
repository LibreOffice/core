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

#include <tchar.h>
#include "dibpreview.hxx"
#include <osl/diagnose.h>

#include <com/sun/star/ui/dialogs/FilePreviewImageFormats.hpp>

#include <rtl/ustring.hxx>

#include <stdexcept>
#include <string>

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Any;
using ::com::sun::star::lang::IllegalArgumentException;
using rtl::OUString;

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

namespace /* private */
{
    const LPCTSTR CURRENT_INSTANCE = TEXT("CurrInst");
};

//------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------

#define PREVIEWWND_CLASS_NAME TEXT("DIBPreviewWnd###")

// means 3 pixel left and 3 pixel right
#define HORZ_BODER_SPACE    6

// means 3 pixel top and 3 pixel bottom
#define VERT_BORDER_SPACE   6

//---------------------------------------------------
// static member initialization
//---------------------------------------------------

osl::Mutex CDIBPreview::s_Mutex;
ATOM CDIBPreview::s_ClassAtom = 0;
sal_Int32 CDIBPreview::s_RegisterDibPreviewWndCount = 0;

//---------------------------------------------------
//
//---------------------------------------------------

CDIBPreview::CDIBPreview(HINSTANCE instance,HWND parent,sal_Bool bShowWindow) :
    m_Instance(instance)
{
    RegisterDibPreviewWindowClass();

    DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    if (bShowWindow)
        dwStyle |= WS_VISIBLE;

    m_Hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        PREVIEWWND_CLASS_NAME,
        TEXT(""),
        dwStyle,
        0, 0, 0, 0,
        parent,
        (HMENU)0x0, // for child windows this will
                    // be used as child window identifier
        m_Instance,
        (LPVOID)this // pass a pointer to the current
                     // instance of this class
    );

    bool bSuccess = IsWindow(m_Hwnd);

    OSL_POSTCOND(bSuccess,"Coud not create preview window");

    if (!bSuccess)
    {
        UnregisterDibPreviewWindowClass();
        throw std::runtime_error("Could not create preview window");
    }
}

//---------------------------------------------------
//
//---------------------------------------------------

CDIBPreview::~CDIBPreview( )
{
    // remember: we don't have to destroy the
    // preview window because it will be destroyed
    // by it's parent window (the FileOpen dialog)
    // but we have to unregister the window class
    //if ( m_bWndClassRegistered )
    UnregisterDibPreviewWindowClass();
}

//-------------------------------
//
//-------------------------------

sal_Int32 SAL_CALL CDIBPreview::getTargetColorDepth() throw (RuntimeException)
{
    HDC hdc = GetDC(m_Hwnd);
    int clrRes = 0;

    if (hdc)
        clrRes = GetDeviceCaps(hdc, COLORRES);

    return clrRes;
}

//-------------------------------
//
//-------------------------------

sal_Int32 SAL_CALL CDIBPreview::getAvailableWidth() throw (RuntimeException)
{
    RECT rect;
    bool bRet = GetClientRect(m_Hwnd,&rect);

    sal_Int32 cx = 0;

    if ( bRet )
        cx = rect.right;

    return cx;
}

//-------------------------------
//
//-------------------------------

sal_Int32 SAL_CALL CDIBPreview::getAvailableHeight() throw (RuntimeException)
{
    RECT rect;
    bool bRet = GetClientRect(m_Hwnd,&rect);

    sal_Int32 cy = 0;

    if ( bRet )
        cy = rect.bottom;

    return cy;
}

//-------------------------------
//
//-------------------------------

void SAL_CALL CDIBPreview::setImage(sal_Int16 aImageFormat, const Any& aImage)
    throw (IllegalArgumentException, RuntimeException)
{
    PreviewBase::setImage(aImageFormat,aImage);

    // if the any has no value we have an
    // empty Sequence which clears the
    // preview window
    osl::ClearableMutexGuard aGuard(m_PaintLock);

    m_Image.realloc(0);
    m_ImageData >>= m_Image;

    aGuard.clear();

    InvalidateRect(m_Hwnd,NULL,sal_False);
    UpdateWindow(m_Hwnd);
}

//-------------------------------
//
//-------------------------------

sal_Bool SAL_CALL CDIBPreview::setShowState(sal_Bool bShowState) throw (RuntimeException)
{
    PreviewBase::setShowState(bShowState);
    ShowWindow(m_Hwnd, m_bShowState ? SW_SHOW : SW_HIDE);
    return sal_True;
}

//-------------------------------
//
//-------------------------------

sal_Bool SAL_CALL CDIBPreview::getShowState() throw (RuntimeException)
{
    return (sal_Bool)IsWindowVisible(m_Hwnd);
}

//-------------------------------
//
//-------------------------------

HWND SAL_CALL CDIBPreview::getWindowHandle() const
{
    return m_Hwnd;
}

//---------------------------------------------------
//
//---------------------------------------------------

void SAL_CALL CDIBPreview::onPaint(HWND hWnd, HDC hDC)
{
    BITMAPFILEHEADER*  pbmfh;
    BITMAPINFO      *  pbmi;
    sal_uInt8            *  pBits;
    int                cxDib;
    int                cyDib;

    osl::MutexGuard aGuard(m_PaintLock);

    try
    {
        pbmfh = reinterpret_cast<BITMAPFILEHEADER*>(m_Image.getArray());

        if ( !IsBadReadPtr( pbmfh, sizeof(BITMAPFILEHEADER)) &&
             (pbmfh->bfType == ('B' | ('M' << 8))) )
        {
            pbmi  = reinterpret_cast<BITMAPINFO*>((pbmfh + 1));
            pBits = reinterpret_cast<sal_uInt8*>(((DWORD)pbmfh) + pbmfh->bfOffBits);

            cxDib =      pbmi->bmiHeader.biWidth;
            cyDib = abs (pbmi->bmiHeader.biHeight);

            SetStretchBltMode(hDC, COLORONCOLOR);

            int nWidth  = getAvailableWidth();
            int nHeight = getAvailableHeight();

            int nX = abs(nWidth - cxDib) / 2;
            int nY = abs(nHeight - cyDib) / 2;

#if OSL_DEBUG_LEVEL > 0
            int GDIError = GDI_ERROR;
            GDIError =
#endif
            StretchDIBits(
                hDC, nX, nY, cxDib, cyDib,
                0, 0, cxDib, cyDib, pBits, pbmi,
                DIB_RGB_COLORS, SRCCOPY);

            OSL_ASSERT(GDI_ERROR != GDIError);

            // paint the border
            RECT rc;

            if (nY > 0)
            {
                // top
                rc.left   = 0;
                rc.top    = 0;
                rc.right  = nWidth;
                rc.bottom = nY;
                FillRect(hDC,&rc,(HBRUSH)(COLOR_INACTIVEBORDER + 1));

                // bottom
                rc.left   = 0;
                rc.top    = nHeight - nY - 1;
                rc.right  = nWidth;
                rc.bottom = nHeight;
                FillRect(hDC,&rc,(HBRUSH)(COLOR_INACTIVEBORDER + 1));
            }

            if (nX > 0)
            {
                // left
                rc.left   = 0;
                rc.top    = nY;
                rc.right  = nX;
                rc.bottom = nHeight - nY;
                FillRect(hDC,&rc,(HBRUSH)(COLOR_INACTIVEBORDER + 1));

                // right
                rc.left   = nWidth - nX - 1;
                rc.top    = nY;
                rc.right  = nWidth;
                rc.bottom = nHeight - nY;
                FillRect(hDC,&rc,(HBRUSH)(COLOR_INACTIVEBORDER + 1));
            }
        }
        else // clear background
        {
            RECT rc;
            GetClientRect(hWnd,&rc);
            FillRect(hDC,&rc,(HBRUSH)(COLOR_INACTIVEBORDER + 1));
        }
    }
    catch(...)
    {
        OSL_ASSERT(sal_False);
    }
}

//---------------------------------------------------
//
//---------------------------------------------------

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
            SetProp(hWnd, CURRENT_INSTANCE, lpcs->lpCreateParams);
        }
        break;

    // we remove the window property which connects
    // a class instance with a window class
    case WM_NCDESTROY:
        {
            // RemoveProp returns the saved value on success
            if (reinterpret_cast<CDIBPreview*>(
                    RemoveProp(hWnd, CURRENT_INSTANCE)) == NULL)
            {
                OSL_ASSERT(false);
            }
        }
        break;

    case WM_PAINT:
    {
        CDIBPreview* pImpl = reinterpret_cast<CDIBPreview*>(
            GetProp(hWnd, CURRENT_INSTANCE));

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

//---------------------------------------------------
//
//---------------------------------------------------

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
        wndClsEx.hbrBackground = (HBRUSH)(COLOR_INACTIVEBORDER + 1);
        wndClsEx.lpszClassName = PREVIEWWND_CLASS_NAME;

        // register the preview window class
        // !!! Win95 -   the window class will be unregistered automaticly
        //               if the dll is unloaded
        //     Win2000 - the window class must be unregistered manually
        //               if the dll is unloaded
        s_ClassAtom = RegisterClassEx(&wndClsEx);

        OSL_POSTCOND(s_ClassAtom,"Could  not register preview window class");

        if (0 == s_ClassAtom)
            throw std::runtime_error("Preview window class could not be registered");
    }

    // increment the register class counter
    // so that we keep track of the number
    // of class registrations
    //if ( 0 != s_ClassAtom )
    s_RegisterDibPreviewWndCount++;

    return s_ClassAtom;
}

//---------------------------------------------------
//
//---------------------------------------------------

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
        UnregisterClass((LPCTSTR)MAKELONG(s_ClassAtom,0),m_Instance);
        s_ClassAtom = 0;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
