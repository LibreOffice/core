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

#include <algorithm>

#include <sal/types.h>
#include <sal/log.hxx>

#include <stdafx.h>
#include <stddef.h>
#include <syswinwrapper.hxx>

// windowserrorstring.hxx includes postwin.h, which #undef OPAQUE, so "#redef" it
#include <comphelper/windowserrorstring.hxx>
#ifdef OPAQUE
#error OPAQUE should not be defined!?
#endif
#define OPAQUE 2

static HCURSOR afxCursors[10] = { nullptr, };
static HBRUSH afxHalftoneBrush = nullptr;

namespace {

// the struct below is used to determine the qualities of a particular handle
struct AFX_HANDLEINFO
{
    size_t nOffsetX;    // offset within RECT for X coordinate
    size_t nOffsetY;    // offset within RECT for Y coordinate
    int nCenterX;       // adjust X by Width()/2 * this number
    int nCenterY;       // adjust Y by Height()/2 * this number
    int nHandleX;       // adjust X by handle size * this number
    int nHandleY;       // adjust Y by handle size * this number
    int nInvertX;       // handle converts to this when X inverted
    int nInvertY;       // handle converts to this when Y inverted
};

}

// this array describes all 8 handles (clock-wise)
const AFX_HANDLEINFO afxHandleInfo[] =
{
    // corner handles (top-left, top-right, bottom-right, bottom-left
    { offsetof(RECT, left), offsetof(RECT, top),        0, 0,  0,  0, 1, 3 },
    { offsetof(RECT, right), offsetof(RECT, top),       0, 0, -1,  0, 0, 2 },
    { offsetof(RECT, right), offsetof(RECT, bottom),    0, 0, -1, -1, 3, 1 },
    { offsetof(RECT, left), offsetof(RECT, bottom),     0, 0,  0, -1, 2, 0 },

    // side handles (top, right, bottom, left)
    { offsetof(RECT, left), offsetof(RECT, top),        1, 0,  0,  0, 4, 6 },
    { offsetof(RECT, right), offsetof(RECT, top),       0, 1, -1,  0, 7, 5 },
    { offsetof(RECT, left), offsetof(RECT, bottom),     1, 0,  0, -1, 6, 4 },
    { offsetof(RECT, left), offsetof(RECT, top),        0, 1,  0,  0, 5, 7 }
};

namespace {

// the struct below gives us information on the layout of a RECT struct and
//  the relationship between its members
struct AFX_RECTINFO
{
    size_t nOffsetAcross;   // offset of opposite point (ie. left->right)
    int nSignAcross;        // sign relative to that point (ie. add/subtract)
};

}

// this array is indexed by the offset of the RECT member / sizeof(int)
const AFX_RECTINFO afxRectInfo[] =
{
    { offsetof(RECT, right), +1 },
    { offsetof(RECT, bottom), +1 },
    { offsetof(RECT, left), -1 },
    { offsetof(RECT, top), -1 },
};


static HBRUSH HalftoneBrush()
{
    if (afxHalftoneBrush == nullptr)
    {
        WORD grayPattern[8];
        for (int i = 0; i < 8; i++)
            grayPattern[i] = static_cast<WORD>(0x5555 << (i & 1));
        HBITMAP grayBitmap = CreateBitmap(8, 8, 1, 1, &grayPattern);
        if (grayBitmap != nullptr)
        {
            afxHalftoneBrush = CreatePatternBrush(grayBitmap);
            DeleteObject(grayBitmap);
        }
    }
    return afxHalftoneBrush;
}


static void DrawDragRect(
    HDC hDC,LPRECT lpRect,SIZE size,
    LPRECT lpRectLast,SIZE sizeLast,
    HBRUSH hBrush = nullptr,HBRUSH hBrushLast = nullptr)
{
    // first, determine the update region and select it
    HRGN rgnNew;
    HRGN rgnOutside,rgnInside;
    rgnOutside = CreateRectRgnIndirect(lpRect);
    RECT rect = *lpRect;
    InflateRect(&rect,-size.cx, -size.cy);
    IntersectRect(&rect,&rect,lpRect);
    rgnInside = CreateRectRgnIndirect(&rect);
    rgnNew = CreateRectRgn(0, 0, 0, 0);
    CombineRgn(rgnNew,rgnOutside,rgnInside,RGN_XOR);

    HBRUSH hBrushOld = nullptr;
    if (hBrush == nullptr)
        hBrush = HalftoneBrush();
    if (hBrushLast == nullptr)
        hBrushLast = hBrush;

    HRGN rgnLast(nullptr);
    HRGN rgnUpdate(nullptr);
    if (lpRectLast != nullptr)
    {
        // find difference between new region and old region
        rgnLast = CreateRectRgn(0, 0, 0, 0);
        SetRectRgn(
            rgnOutside,
            lpRectLast->left,
            lpRectLast->top,
            lpRectLast->right,
            lpRectLast->bottom);
        rect = *lpRectLast;
        InflateRect(&rect,-sizeLast.cx, -sizeLast.cy);
        IntersectRect(&rect,&rect, lpRectLast);
        SetRectRgn(rgnInside,rect.left,rect.top,rect.right,rect.bottom);
        CombineRgn(rgnLast,rgnOutside,rgnInside, RGN_XOR);

//      // only diff them if brushes are the same
        if (hBrush == hBrushLast)
        {
            rgnUpdate = CreateRectRgn(0, 0, 0, 0);
            CombineRgn(rgnUpdate,rgnLast,rgnNew, RGN_XOR);
        }
    }
    if (hBrush != hBrushLast && lpRectLast != nullptr)
    {
        // brushes are different -- erase old region first
        SelectClipRgn(hDC,rgnLast);
        GetClipBox(hDC,&rect);
        hBrushOld = static_cast<HBRUSH>(SelectObject(hDC,static_cast<HGDIOBJ>(hBrushLast)));
        PatBlt(hDC,rect.left,rect.top,(rect.right-rect.left),(rect.bottom-rect.top),PATINVERT);

        SelectObject(hDC,static_cast<HGDIOBJ>(hBrushOld));
        hBrushOld = nullptr;
    }

    // draw into the update/new region
    SelectClipRgn(hDC,rgnUpdate);

    GetClipBox(hDC,&rect);
    hBrushOld = static_cast<HBRUSH>(SelectObject(hDC, static_cast<HGDIOBJ>(hBrush)));
    PatBlt(hDC,rect.left, rect.top,(rect.right-rect.left),(rect.bottom-rect.top), PATINVERT);

    // cleanup DC
    if (hBrushOld != nullptr)
        SelectObject(hDC, static_cast<HGDIOBJ>(hBrushOld));
    SelectClipRgn(hDC,nullptr);
}


void winwrap::TransformRect(LPRECT rect,HWND pWnd,HWND pWndClipTo)
{
    POINT pt;
    pt.x = rect->left;pt.y = rect->top;
    ClientToScreen(pWnd,&pt);
    ScreenToClient(pWndClipTo,&pt);
    rect->left = pt.x; rect->top = pt.y;

    pt.x = rect->right;pt.y = rect->bottom;
    ClientToScreen(pWnd,&pt);
    ScreenToClient(pWndClipTo,&pt);
    rect->right = pt.x; rect->bottom = pt.y;
}


static void NormalizeRect(LPRECT rp)
{
    if(rp->left > rp->right)
        std::swap(rp->left, rp->right);
    if(rp->top > rp->bottom)
        std::swap(rp->top, rp->bottom);
}


using namespace winwrap;


Tracker::Tracker()
{
}


Tracker::Tracker(LPCRECT lpSrcRect, UINT nStyle)
{
    Construct();
    CopyRect(&m_rect,lpSrcRect);
    m_nStyle = nStyle;
}

static HBRUSH afxHatchBrush = nullptr;
static HPEN afxBlackDottedPen = nullptr;
static int afxHandleSize = 0;


void Tracker::Construct()
{
    static bool bInitialized = false;
    if (!bInitialized)
    {
        if (afxHatchBrush == nullptr)
        {
            // create the hatch pattern + bitmap
            WORD hatchPattern[8];
            WORD wPattern = 0x1111;
            for (int i = 0; i < 4; i++)
            {
                hatchPattern[i] = wPattern;
                hatchPattern[i+4] = wPattern;
                wPattern <<= 1;
            }
            HBITMAP hatchBitmap = CreateBitmap(8, 8, 1, 1,&hatchPattern);

            // create black hatched brush
            afxHatchBrush = CreatePatternBrush(hatchBitmap);
            DeleteObject(hatchBitmap);
        }

        if (afxBlackDottedPen == nullptr)
        {
            // create black dotted pen
            afxBlackDottedPen = CreatePen(PS_DOT, 0, RGB(0, 0, 0));
        }

        // get default handle size from Windows profile setting
        static const WCHAR szWindows[] = L"windows";
        static const WCHAR szInplaceBorderWidth[] = L"oleinplaceborderwidth";
        afxHandleSize = GetProfileIntW(szWindows, szInplaceBorderWidth, 4);
        bInitialized = true;

        afxCursors[0] = afxCursors[2] = LoadCursor(nullptr,IDC_SIZENWSE);
        afxCursors[4] = afxCursors[6] = LoadCursor(nullptr,IDC_SIZENS);
        afxCursors[1] = afxCursors[3] = LoadCursor(nullptr,IDC_SIZENESW);
        afxCursors[5] = afxCursors[7] = LoadCursor(nullptr,IDC_SIZEWE);
        afxCursors[8] = LoadCursor(nullptr,IDC_SIZEALL);
    }

    m_nStyle = 0;
    m_nHandleSize = afxHandleSize;
    m_sizeMin.cy = m_sizeMin.cx = m_nHandleSize*2;

    SetRectEmpty(&m_rectLast);
    m_sizeLast.cx = m_sizeLast.cy = 0;
    m_bErase = FALSE;
    m_bFinalErase =  FALSE;
}

Tracker::~Tracker()
{
}


int Tracker::HitTest(POINT point) const
{
    TrackerHit hitResult = hitNothing;

    RECT rectTrue;
    GetTrueRect(&rectTrue);
    NormalizeRect(&rectTrue);
    if (PtInRect(&rectTrue,point))
    {
        if ((m_nStyle & (resizeInside|resizeOutside)) != 0)
            hitResult = static_cast<TrackerHit>(HitTestHandles(point));
        else
            hitResult = hitMiddle;
    }
    return hitResult;
}


BOOL Tracker::SetCursor(HWND pWnd, UINT nHitTest) const
{
    // trackers should only be in client area
    if (nHitTest != HTCLIENT)
        return FALSE;

    // convert cursor position to client co-ordinates
    POINT point;
    GetCursorPos(&point);
    ScreenToClient(pWnd,&point);

    // do hittest and normalize hit
    int nHandle = HitTestHandles(point);
    if (nHandle < 0)
        return FALSE;

    // need to normalize the hittest such that we get proper cursors
    nHandle = NormalizeHit(nHandle);

    // handle special case of hitting area between handles
    //  (logically the same -- handled as a move -- but different cursor)
    if (nHandle == hitMiddle && !PtInRect(&m_rect,point))
    {
        // only for trackers with hatchedBorder (ie. in-place resizing)
        if (m_nStyle & hatchedBorder)
            nHandle = TrackerHit(9);
    }

    ::SetCursor(afxCursors[nHandle]);
    return TRUE;
}


BOOL Tracker::Track(HWND hWnd,POINT point,BOOL bAllowInvert,
                    HWND hWndClipTo)
{
    // perform hit testing on the handles
    int nHandle = HitTestHandles(point);
    if (nHandle < 0)
    {
        // didn't hit a handle, so just return FALSE
        return FALSE;
    }

    // otherwise, call helper function to do the tracking
    m_bAllowInvert = bAllowInvert;
    SetCursor(hWnd,nHandle);
    return TrackHandle(nHandle, hWnd, point, hWndClipTo);
}


BOOL Tracker::TrackHandle(int nHandle,HWND hWnd,POINT point,HWND hWndClipTo)
{
    // don't handle if capture already set
    if (GetCapture() != nullptr)
        return FALSE;

    // save original width & height in pixels
    int nWidth = m_rect.right - m_rect.left;
    int nHeight = m_rect.bottom - m_rect.top;

    // set capture to the window which received this message
    SetCapture(hWnd);
    UpdateWindow(hWnd);
    if (hWndClipTo != nullptr)
        UpdateWindow(hWndClipTo);
    RECT rectSave = m_rect;

    // find out what x/y coords we are supposed to modify
    int *px, *py;
    int xDiff, yDiff;
    GetModifyPointers(nHandle, &px, &py, &xDiff, &yDiff);
    xDiff = point.x - xDiff;
    yDiff = point.y - yDiff;

    // get DC for drawing
    HDC hDrawDC;
    if (hWndClipTo != nullptr)
    {
        // clip to arbitrary window by using adjusted Window DC
        hDrawDC = GetDCEx(hWndClipTo,nullptr, DCX_CACHE);
    }
    else
    {
        // otherwise, just use normal DC
        hDrawDC = GetDC(hWnd);
    }

    RECT rectOld;
    bool bMoved = false;

    // get messages until capture lost or cancelled/accepted
    for (;;)
    {
        MSG msg;
        int const bRet = GetMessageW(&msg, nullptr, 0, 0);
        SAL_WARN_IF(-1 == bRet, "embedserv", "GetMessageW failed: " << WindowsErrorString(GetLastError()));
        if (-1 == bRet || 0 == bRet)
            break;

        if (GetCapture() != hWnd)
            break;

        switch (msg.message)
        {
            // handle movement/accept messages
        case WM_LBUTTONUP:
        case WM_MOUSEMOVE:
            rectOld = m_rect;
            // handle resize cases (and part of move)
            if (px != nullptr)
                *px = static_cast<int>(static_cast<short>(LOWORD(msg.lParam))) - xDiff;
            if (py != nullptr)
                *py = static_cast<int>(static_cast<short>(HIWORD(msg.lParam))) - yDiff;

            // handle move case
            if (nHandle == hitMiddle)
            {
                m_rect.right = m_rect.left + nWidth;
                m_rect.bottom = m_rect.top + nHeight;
            }
            // allow caller to adjust the rectangle if necessary
            AdjustRect(nHandle,&m_rect);

            // only redraw and callback if the rect actually changed!
            m_bFinalErase = (msg.message == WM_LBUTTONUP);
            if (!EqualRect(&rectOld,&m_rect) || m_bFinalErase)
            {
                if (bMoved)
                {
                    m_bErase = TRUE;
                    DrawTrackerRect(&rectOld,hWndClipTo,hDrawDC,hWnd);
                }
                OnChangedRect(rectOld);
                if (msg.message != WM_LBUTTONUP)
                    bMoved = true;
            }
            if (m_bFinalErase)
                goto ExitLoop;

            if (!EqualRect(&rectOld,&m_rect))
            {
                m_bErase = FALSE;
                DrawTrackerRect(&m_rect,hWndClipTo,hDrawDC,hWnd);
            }
            break;

            // handle cancel messages
        case WM_KEYDOWN:
            if (msg.wParam != VK_ESCAPE)
                break;
            [[fallthrough]];
        case WM_RBUTTONDOWN:
            if (bMoved)
            {
                m_bErase = m_bFinalErase = TRUE;
                DrawTrackerRect(&m_rect, hWndClipTo, hDrawDC, hWnd);
            }
            m_rect = rectSave;
            goto ExitLoop;

            // just dispatch rest of the messages
        default:
            DispatchMessageW(&msg);
            break;
        }
    }

  ExitLoop:
    if (hWndClipTo != nullptr)
        ReleaseDC(hWndClipTo,hDrawDC);
    else
        ReleaseDC(hWnd,hDrawDC);
    ReleaseCapture();

    // restore rect in case bMoved is still FALSE
    if (!bMoved)
        m_rect = rectSave;
    m_bFinalErase = FALSE;
    m_bErase = FALSE;

    // return TRUE only if rect has changed
    return !EqualRect(&rectSave,&m_rect);
}


void Tracker::OnChangedRect(const RECT& /*rectOld*/)
{
}


void Tracker::AdjustRect(int nHandle, LPRECT)
{
    if(nHandle == hitMiddle)
        return;

    // convert the handle into locations within m_rect
    int *px, *py;
    GetModifyPointers(nHandle, &px, &py, nullptr, nullptr);

    // enforce minimum width
    int nNewWidth = m_rect.right - m_rect.left;
    int nAbsWidth = m_bAllowInvert ? abs(nNewWidth) : nNewWidth;
    if (px != nullptr && nAbsWidth < m_sizeMin.cx)
    {
        nNewWidth = nAbsWidth != 0 ? nNewWidth / nAbsWidth : 1;
        const AFX_RECTINFO* pRectInfo =
            &afxRectInfo[px - reinterpret_cast<int*>(&m_rect)];
        *px = *reinterpret_cast<int*>(reinterpret_cast<BYTE*>(&m_rect) + pRectInfo->nOffsetAcross) +
            nNewWidth * m_sizeMin.cx * -pRectInfo->nSignAcross;
    }

    // enforce minimum height
    int nNewHeight = m_rect.bottom - m_rect.top;
    int nAbsHeight = m_bAllowInvert ? abs(nNewHeight) : nNewHeight;
    if (py != nullptr && nAbsHeight < m_sizeMin.cy)
    {
        nNewHeight = nAbsHeight != 0 ? nNewHeight / nAbsHeight : 1;
        const AFX_RECTINFO* pRectInfo =
            &afxRectInfo[py - reinterpret_cast<int*>(&m_rect)];
        *py = *reinterpret_cast<int*>(reinterpret_cast<BYTE*>(&m_rect) + pRectInfo->nOffsetAcross) +
            nNewHeight * m_sizeMin.cy * -pRectInfo->nSignAcross;
    }
}


void Tracker::DrawTrackerRect(
    LPRECT lpRect,HWND pWndClipTo,HDC pDC,HWND pWnd)
{
    // first, normalize the rectangle for drawing
    RECT rect = *lpRect;
    NormalizeRect(&rect);

    // convert to client coordinates
    if (pWndClipTo != nullptr)
        TransformRect(&rect,pWnd,pWndClipTo);

    SIZE size;
    size.cx = 0; size.cy = 0;
    if (!m_bFinalErase)
    {
        // otherwise, size depends on the style
        if (m_nStyle & hatchedBorder)
        {
            size.cx = size.cy = std::max(1,GetHandleSize(&rect)-1);
            InflateRect(&rect,size.cx,size.cy);
        }
        else
        {
            size.cx = 1; // CX_BORDER;
            size.cy = 1; // CY_BORDER;
        }
    }

    // and draw it
    if (m_bFinalErase || !m_bErase)
        DrawDragRect(pDC,&rect,size,&m_rectLast,m_sizeLast);

    // remember last rectangles
    m_rectLast = rect;
    m_sizeLast = size;
}


void Tracker::Draw(HDC hDC) const
{
    // set initial DC state
    SetMapMode(hDC,MM_TEXT);
    SetViewportOrgEx(hDC,0, 0,nullptr);
    SetWindowOrgEx(hDC,0, 0,nullptr);

    // get normalized rectangle
    RECT rect = m_rect;
    NormalizeRect(&rect);

    HPEN pOldPen = nullptr;
    HBRUSH pOldBrush = nullptr;
    HGDIOBJ pTemp;
    int nOldROP;

    // draw lines
    if ((m_nStyle & (dottedLine|solidLine)) != 0)
    {
        if (m_nStyle & dottedLine)
            pOldPen = static_cast<HPEN>(SelectObject(hDC,afxBlackDottedPen));
        else
            pOldPen = static_cast<HPEN>(SelectObject(hDC,reinterpret_cast<HGDIOBJ>(BLACK_PEN)));
        pOldBrush = static_cast<HBRUSH>(SelectObject(hDC,reinterpret_cast<HGDIOBJ>(NULL_BRUSH)));
        nOldROP = SetROP2(hDC,R2_COPYPEN);
        InflateRect(&rect,+1, +1);   // borders are one pixel outside
        Rectangle(hDC,rect.left, rect.top, rect.right, rect.bottom);
        SetROP2(hDC,nOldROP);
    }

    // if hatchBrush is going to be used, need to unrealize it
    if ((m_nStyle & (hatchInside|hatchedBorder)) != 0)
        UnrealizeObject(static_cast<HGDIOBJ>(afxHatchBrush));

    // hatch inside
    if ((m_nStyle & hatchInside) != 0)
    {
        pTemp = SelectObject(hDC,reinterpret_cast<HGDIOBJ>(NULL_PEN));
        if (pOldPen == nullptr)
            pOldPen = static_cast<HPEN>(pTemp);
        pTemp = SelectObject(hDC,static_cast<HGDIOBJ>(afxHatchBrush));
        if (pOldBrush == nullptr)
            pOldBrush = static_cast<HBRUSH>(pTemp);
        SetBkMode(hDC,TRANSPARENT);
        nOldROP = SetROP2(hDC,R2_MASKNOTPEN);
        Rectangle(hDC,rect.left+1, rect.top+1, rect.right, rect.bottom);
        SetROP2(hDC,nOldROP);
    }

    // draw hatched border
    if ((m_nStyle & hatchedBorder) != 0)
    {
        pTemp = SelectObject(hDC,static_cast<HGDIOBJ>(afxHatchBrush));
        if (pOldBrush == nullptr)
            pOldBrush = static_cast<HBRUSH>(pTemp);
        SetBkMode(hDC,OPAQUE);
        RECT rectTrue;
        GetTrueRect(&rectTrue);
        PatBlt(hDC,rectTrue.left, rectTrue.top, rectTrue.right-rectTrue.left,
               rect.top-rectTrue.top, 0x000F0001 /* Pn */);
        PatBlt(hDC,rectTrue.left, rect.bottom,
               rectTrue.right-rectTrue.left,
               rectTrue.bottom-rect.bottom, 0x000F0001 /* Pn */);
        PatBlt(hDC,rectTrue.left, rect.top, rect.left-rectTrue.left,
               rect.bottom-rect.top, 0x000F0001 /* Pn */);
        PatBlt(hDC,rect.right, rect.top, rectTrue.right-rect.right,
               rect.bottom-rect.top, 0x000F0001 /* Pn */);
    }

    // draw resize handles
    if ((m_nStyle & (resizeInside|resizeOutside)) != 0)
    {
        UINT mask = GetHandleMask();
        HBRUSH hbrush = CreateSolidBrush(RGB(0,0,0));
        for (int i = 0; i < 8; ++i)
        {
            if (mask & (1<<i))
            {
                GetHandleRect(static_cast<TrackerHit>(i), &rect);
                // FillSolidRect(hDC,rect, RGB(0, 0, 0));
                FillRect(hDC,&rect,hbrush);
            }
        }
        DeleteObject(hbrush);
    }

    // cleanup pDC state
    if (pOldPen != nullptr)
        SelectObject(hDC,pOldPen);
    if (pOldBrush != nullptr)
        SelectObject(hDC,pOldBrush);
    RestoreDC(hDC,-1);
}


void Tracker::GetHandleRect(int nHandle,RECT* pHandleRect) const
{
    // get normalized rectangle of the tracker
    RECT rectT = m_rect;
    NormalizeRect(&rectT);
    if ((m_nStyle & (solidLine|dottedLine)) != 0)
        InflateRect(&rectT,+1, +1);

    // since the rectangle itself was normalized, we also have to invert the
    //  resize handles.
    nHandle = NormalizeHit(nHandle);

    // handle case of resize handles outside the tracker
    int size = GetHandleSize();
    if (m_nStyle & resizeOutside)
        InflateRect(&rectT,size-1, size-1);

    // calculate position of the resize handle
    int nWidth = rectT.right - rectT.left;
    int nHeight = rectT.bottom - rectT.top;
    RECT rect;
    const AFX_HANDLEINFO* pHandleInfo = &afxHandleInfo[nHandle];
    rect.left = *reinterpret_cast<int*>(reinterpret_cast<BYTE*>(&rectT) + pHandleInfo->nOffsetX);
    rect.top = *reinterpret_cast<int*>(reinterpret_cast<BYTE*>(&rectT) + pHandleInfo->nOffsetY);
    rect.left += size * pHandleInfo->nHandleX;
    rect.top += size * pHandleInfo->nHandleY;
    rect.left += pHandleInfo->nCenterX * (nWidth - size) / 2;
    rect.top += pHandleInfo->nCenterY * (nHeight - size) / 2;
    rect.right = rect.left + size;
    rect.bottom = rect.top + size;

    *pHandleRect = rect;
}


int Tracker::GetHandleSize(LPRECT lpRect) const
{
    LPCRECT rect = lpRect == nullptr ? &m_rect : lpRect;

    int size = m_nHandleSize;
    if (!(m_nStyle & resizeOutside))
    {
        // make sure size is small enough for the size of the rect
        int sizeMax = std::min(abs(rect->right - rect->left),
                          abs(rect->bottom - rect->top));
        if (size * 2 > sizeMax)
            size = sizeMax / 2;
    }
    return size;
}


UINT Tracker::GetHandleMask() const
{
    UINT mask = 0x0F;   // always have 4 corner handles
    int size = m_nHandleSize*3;
    if (abs(m_rect.right - m_rect.left) - size > 4)
        mask |= 0x50;
    if (abs(m_rect.bottom - m_rect.top) - size > 4)
        mask |= 0xA0;
    return mask;
}


void Tracker::GetTrueRect(LPRECT lpTrueRect) const
{
    RECT rect = m_rect;
    NormalizeRect(&rect);
    int nInflateBy = 0;
    if ((m_nStyle & (resizeOutside|hatchedBorder)) != 0)
        nInflateBy += GetHandleSize() - 1;
    if ((m_nStyle & (solidLine|dottedLine)) != 0)
        ++nInflateBy;
    InflateRect(&rect,nInflateBy, nInflateBy);
    *lpTrueRect = rect;
}


int Tracker::NormalizeHit(int nHandle) const
{
    if (nHandle == hitMiddle || nHandle == hitNothing)
        return nHandle;
    const AFX_HANDLEINFO* pHandleInfo = &afxHandleInfo[nHandle];
    if (m_rect.right - m_rect.left < 0)
    {
        nHandle = static_cast<TrackerHit>(pHandleInfo->nInvertX);
        pHandleInfo = &afxHandleInfo[nHandle];
    }
    if (m_rect.bottom - m_rect.top < 0)
        nHandle = static_cast<TrackerHit>(pHandleInfo->nInvertY);
    return nHandle;
}


int Tracker::HitTestHandles(POINT point) const
{
    RECT rect;
    UINT mask = GetHandleMask();

    // see if hit anywhere inside the tracker
    GetTrueRect(&rect);
    if (!PtInRect(&rect,point))
        return hitNothing;  // totally missed

    // see if we hit a handle
    for (int i = 0; i < 8; ++i)
    {
        if (mask & (1<<i))
        {
            GetHandleRect(static_cast<TrackerHit>(i), &rect);
            if (PtInRect(&rect,point))
                return static_cast<TrackerHit>(i);
        }
    }

    // last of all, check for non-hit outside of object, between resize handles
    if ((m_nStyle & hatchedBorder) == 0)
    {
        rect = m_rect;
        NormalizeRect(&rect);
        if ((m_nStyle & (dottedLine|solidLine)) != 0)
            InflateRect(&rect,+1, +1);
        if (!PtInRect(&rect,point))
            return hitNothing;  // must have been between resize handles
    }
    return hitMiddle;   // no handle hit, but hit object (or object border)
}


void Tracker::GetModifyPointers(
    int nHandle, int** ppx, int** ppy, int* px, int* py)
{
    if (nHandle == hitMiddle)
        nHandle = hitTopLeft;   // same as hitting top-left

    *ppx = nullptr;
    *ppy = nullptr;

    // fill in the part of the rect that this handle modifies
    //  (Note: handles that map to themselves along a given axis when that
    //   axis is inverted don't modify the value on that axis)

    const AFX_HANDLEINFO* pHandleInfo = &afxHandleInfo[nHandle];
    if (pHandleInfo->nInvertX != nHandle)
    {
        *ppx = reinterpret_cast<int*>(reinterpret_cast<BYTE*>(&m_rect) + pHandleInfo->nOffsetX);
        if (px != nullptr)
            *px = **ppx;
    }
    else
    {
        // middle handle on X axis
        if (px != nullptr)
            *px = m_rect.left + (m_rect.left-m_rect.right) / 2;
    }
    if (pHandleInfo->nInvertY != nHandle)
    {
        *ppy = reinterpret_cast<int*>(reinterpret_cast<BYTE*>(&m_rect) + pHandleInfo->nOffsetY);
        if (py != nullptr)
            *py = **ppy;
    }
    else
    {
        // middle handle on Y axis
        if (py != nullptr)
            *py = m_rect.top + (m_rect.top-m_rect.bottom) / 2;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
