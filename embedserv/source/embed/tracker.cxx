/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */
#ifdef _MSC_VER
#pragma warning(disable : 4917 4555)
#endif

#include "stdafx.h"
#include <stddef.h>
#include "syswinwrapper.hxx"


HCURSOR _afxCursors[10] = { 0, };
HBRUSH _afxHalftoneBrush = 0;



struct AFX_HANDLEINFO
{
    size_t nOffsetX;    
    size_t nOffsetY;    
    int nCenterX;       
    int nCenterY;       
    int nHandleX;       
    int nHandleY;       
    int nInvertX;       
    int nInvertY;       
};


const AFX_HANDLEINFO _afxHandleInfo[] =
{
    
    { offsetof(RECT, left), offsetof(RECT, top),        0, 0,  0,  0, 1, 3 },
    { offsetof(RECT, right), offsetof(RECT, top),       0, 0, -1,  0, 0, 2 },
    { offsetof(RECT, right), offsetof(RECT, bottom),    0, 0, -1, -1, 3, 1 },
    { offsetof(RECT, left), offsetof(RECT, bottom),     0, 0,  0, -1, 2, 0 },

    
    { offsetof(RECT, left), offsetof(RECT, top),        1, 0,  0,  0, 4, 6 },
    { offsetof(RECT, right), offsetof(RECT, top),       0, 1, -1,  0, 7, 5 },
    { offsetof(RECT, left), offsetof(RECT, bottom),     1, 0,  0, -1, 6, 4 },
    { offsetof(RECT, left), offsetof(RECT, top),        0, 1,  0,  0, 5, 7 }
};



struct AFX_RECTINFO
{
    size_t nOffsetAcross;   
    int nSignAcross;        
};


const AFX_RECTINFO _afxRectInfo[] =
{
    { offsetof(RECT, right), +1 },
    { offsetof(RECT, bottom), +1 },
    { offsetof(RECT, left), -1 },
    { offsetof(RECT, top), -1 },
};


HBRUSH HalftoneBrush()
{
    if (_afxHalftoneBrush == NULL)
    {
        WORD grayPattern[8];
        for (int i = 0; i < 8; i++)
            grayPattern[i] = (WORD)(0x5555 << (i & 1));
        HBITMAP grayBitmap = CreateBitmap(8, 8, 1, 1, &grayPattern);
        if (grayBitmap != NULL)
        {
            _afxHalftoneBrush = CreatePatternBrush(grayBitmap);
            DeleteObject(grayBitmap);
        }
    }
    return _afxHalftoneBrush;
}



void DrawDragRect(
    HDC hDC,LPRECT lpRect,SIZE size,
    LPRECT lpRectLast,SIZE sizeLast,
    HBRUSH hBrush = NULL,HBRUSH hBrushLast = NULL)
{
    
    HRGN rgnNew;
    HRGN rgnOutside,rgnInside;
    rgnOutside = CreateRectRgnIndirect(lpRect);
    RECT rect = *lpRect;
    InflateRect(&rect,-size.cx, -size.cy);
    IntersectRect(&rect,&rect,lpRect);
    rgnInside = CreateRectRgnIndirect(&rect);
    rgnNew = CreateRectRgn(0, 0, 0, 0);
    CombineRgn(rgnNew,rgnOutside,rgnInside,RGN_XOR);

    HBRUSH hBrushOld = NULL;
    if (hBrush == NULL)
        hBrush = HalftoneBrush();
    if (hBrushLast == NULL)
        hBrushLast = hBrush;

    HRGN rgnLast(NULL);
    HRGN rgnUpdate(NULL);
    if (lpRectLast != NULL)
    {
        
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


        if (hBrush == hBrushLast)
        {
            rgnUpdate = CreateRectRgn(0, 0, 0, 0);
            CombineRgn(rgnUpdate,rgnLast,rgnNew, RGN_XOR);
        }
    }
    if (hBrush != hBrushLast && lpRectLast != NULL)
    {
        
        SelectClipRgn(hDC,rgnLast);
        GetClipBox(hDC,&rect);
        hBrushOld = (HBRUSH)SelectObject(hDC,(HGDIOBJ)hBrushLast);
        PatBlt(hDC,rect.left,rect.top,(rect.right-rect.left),(rect.bottom-rect.top),PATINVERT);

        SelectObject(hDC,(HGDIOBJ)hBrushOld);
        hBrushOld = NULL;
    }

    
    SelectClipRgn(hDC,rgnUpdate);

    GetClipBox(hDC,&rect);
    hBrushOld = (HBRUSH) SelectObject(hDC,(HGDIOBJ) hBrush);
    PatBlt(hDC,rect.left, rect.top,(rect.right-rect.left),(rect.bottom-rect.top), PATINVERT);

    
    if (hBrushOld != NULL)
        SelectObject(hDC,(HGDIOBJ)hBrushOld);
    SelectClipRgn(hDC,NULL);
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


void NormalizeRect(LPRECT rp)
{
    if(rp->left > rp->right) {
        UINT tmp = rp->left;
        rp->left = rp->right;
        rp->right = tmp;
    }

    if(rp->top > rp->bottom) {
        UINT tmp = rp->top;
        rp->top = rp->bottom;
        rp->bottom = tmp;
    }
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

HBRUSH _afxHatchBrush = 0;
HPEN _afxBlackDottedPen = 0;
int _afxHandleSize = 0;


void Tracker::Construct()
{
    static BOOL bInitialized = false;
    if (!bInitialized)
    {
        if (_afxHatchBrush == NULL)
        {
            
            WORD hatchPattern[8];
            WORD wPattern = 0x1111;
            for (int i = 0; i < 4; i++)
            {
                hatchPattern[i] = wPattern;
                hatchPattern[i+4] = wPattern;
                wPattern <<= 1;
            }
            HBITMAP hatchBitmap = CreateBitmap(8, 8, 1, 1,&hatchPattern);

            
            _afxHatchBrush = CreatePatternBrush(hatchBitmap);
            DeleteObject(hatchBitmap);
        }

        if (_afxBlackDottedPen == NULL)
        {
            
            _afxBlackDottedPen = CreatePen(PS_DOT, 0, RGB(0, 0, 0));
        }

        
        static const TCHAR szWindows[] = TEXT("windows");
        static const TCHAR szInplaceBorderWidth[] =
            TEXT("oleinplaceborderwidth");
        _afxHandleSize = GetProfileInt(szWindows, szInplaceBorderWidth, 4);
        bInitialized = TRUE;

        _afxCursors[0] = _afxCursors[2] = LoadCursor(0,IDC_SIZENWSE);
        _afxCursors[4] = _afxCursors[6] = LoadCursor(0,IDC_SIZENS);
        _afxCursors[1] = _afxCursors[3] = LoadCursor(0,IDC_SIZENESW);
        _afxCursors[5] = _afxCursors[7] = LoadCursor(0,IDC_SIZEWE);
        _afxCursors[8] = LoadCursor(0,IDC_SIZEALL);
    }

    m_nStyle = 0;
    m_nHandleSize = _afxHandleSize;
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
            hitResult = (TrackerHit)HitTestHandles(point);
        else
            hitResult = hitMiddle;
    }
    return hitResult;
}


BOOL Tracker::SetCursor(HWND pWnd, UINT nHitTest) const
{
    
    if (nHitTest != HTCLIENT)
        return FALSE;

    
    POINT point;
    GetCursorPos(&point);
    ScreenToClient(pWnd,&point);

    
    int nHandle = HitTestHandles(point);
    if (nHandle < 0)
        return FALSE;

    
    nHandle = NormalizeHit(nHandle);

    
    
    if (nHandle == hitMiddle && !PtInRect(&m_rect,point))
    {
        
        if (m_nStyle & hatchedBorder)
            nHandle = (TrackerHit)9;
    }

    ::SetCursor(_afxCursors[nHandle]);
    return TRUE;
}



BOOL Tracker::Track(HWND hWnd,POINT point,BOOL bAllowInvert,
                    HWND hWndClipTo)
{
    
    int nHandle = HitTestHandles(point);
    if (nHandle < 0)
    {
        
        return FALSE;
    }

    
    m_bAllowInvert = bAllowInvert;
    SetCursor(hWnd,nHandle);
    return TrackHandle(nHandle, hWnd, point, hWndClipTo);
}


BOOL Tracker::TrackHandle(int nHandle,HWND hWnd,POINT point,HWND hWndClipTo)
{
    
    if (GetCapture() != NULL)
        return FALSE;

    
    int nWidth = m_rect.right - m_rect.left;
    int nHeight = m_rect.bottom - m_rect.top;

    
    SetCapture(hWnd);
    UpdateWindow(hWnd);
    if (hWndClipTo != NULL)
        UpdateWindow(hWndClipTo);
    RECT rectSave = m_rect;

    
    int *px, *py;
    int xDiff, yDiff;
    GetModifyPointers(nHandle, &px, &py, &xDiff, &yDiff);
    xDiff = point.x - xDiff;
    yDiff = point.y - yDiff;

    
    HDC hDrawDC;
    if (hWndClipTo != NULL)
    {
        
        hDrawDC = GetDCEx(hWndClipTo,NULL, DCX_CACHE);
    }
    else
    {
        
        hDrawDC = GetDC(hWnd);
    }

    RECT rectOld;
    BOOL bMoved = FALSE;

    
    for (;;)
    {
        MSG msg;
        GetMessage(&msg, NULL, 0, 0);

        if (GetCapture() != hWnd)
            break;

        switch (msg.message)
        {
            
        case WM_LBUTTONUP:
        case WM_MOUSEMOVE:
            rectOld = m_rect;
            
            if (px != NULL)
                *px = (int)(short)LOWORD(msg.lParam) - xDiff;
            if (py != NULL)
                *py = (int)(short)HIWORD(msg.lParam) - yDiff;

            
            if (nHandle == hitMiddle)
            {
                m_rect.right = m_rect.left + nWidth;
                m_rect.bottom = m_rect.top + nHeight;
            }
            
            AdjustRect(nHandle,&m_rect);

            
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
                    bMoved = TRUE;
            }
            if (m_bFinalErase)
                goto ExitLoop;

            if (!EqualRect(&rectOld,&m_rect))
            {
                m_bErase = FALSE;
                DrawTrackerRect(&m_rect,hWndClipTo,hDrawDC,hWnd);
            }
            break;

            
        case WM_KEYDOWN:
            if (msg.wParam != VK_ESCAPE)
                break;
        case WM_RBUTTONDOWN:
            if (bMoved)
            {
                m_bErase = m_bFinalErase = TRUE;
                DrawTrackerRect(&m_rect, hWndClipTo, hDrawDC, hWnd);
            }
            m_rect = rectSave;
            goto ExitLoop;

            
        default:
            DispatchMessage(&msg);
            break;
        }
    }

  ExitLoop:
    if (hWndClipTo != NULL)
        ReleaseDC(hWndClipTo,hDrawDC);
    else
        ReleaseDC(hWnd,hDrawDC);
    ReleaseCapture();

    
    if (!bMoved)
        m_rect = rectSave;
    m_bFinalErase = FALSE;
    m_bErase = FALSE;

    
    return !EqualRect(&rectSave,&m_rect);
}


void Tracker::OnChangedRect(const RECT& /*rectOld*/)
{
}


void Tracker::AdjustRect(int nHandle, LPRECT)
{
    if(nHandle == hitMiddle)
        return;

    
    int *px, *py;
    GetModifyPointers(nHandle, &px, &py, NULL, NULL);

    
    int nNewWidth = m_rect.right - m_rect.left;
    int nAbsWidth = m_bAllowInvert ? abs(nNewWidth) : nNewWidth;
    if (px != NULL && nAbsWidth < m_sizeMin.cx)
    {
        nNewWidth = nAbsWidth != 0 ? nNewWidth / nAbsWidth : 1;
        const AFX_RECTINFO* pRectInfo =
            &_afxRectInfo[(int*)px - (int*)&m_rect];
        *px = *(int*)((BYTE*)&m_rect + pRectInfo->nOffsetAcross) +
            nNewWidth * m_sizeMin.cx * -pRectInfo->nSignAcross;
    }

    
    int nNewHeight = m_rect.bottom - m_rect.top;
    int nAbsHeight = m_bAllowInvert ? abs(nNewHeight) : nNewHeight;
    if (py != NULL && nAbsHeight < m_sizeMin.cy)
    {
        nNewHeight = nAbsHeight != 0 ? nNewHeight / nAbsHeight : 1;
        const AFX_RECTINFO* pRectInfo =
            &_afxRectInfo[(int*)py - (int*)&m_rect];
        *py = *(int*)((BYTE*)&m_rect + pRectInfo->nOffsetAcross) +
            nNewHeight * m_sizeMin.cy * -pRectInfo->nSignAcross;
    }
}


void Tracker::DrawTrackerRect(
    LPRECT lpRect,HWND pWndClipTo,HDC pDC,HWND pWnd)
{
    
    RECT rect = *lpRect;
    NormalizeRect(&rect);

    
    if (pWndClipTo != NULL)
        TransformRect(&rect,pWnd,pWndClipTo);

    SIZE size;
    size.cx = 0; size.cy = 0;
    if (!m_bFinalErase)
    {
        
        if (m_nStyle & hatchedBorder)
        {
            size.cx = size.cy = max(1,GetHandleSize(&rect)-1);
            InflateRect(&rect,size.cx,size.cy);
        }
        else
        {
            size.cx = 1; 
            size.cy = 1; 
        }
    }

    
    if (m_bFinalErase || !m_bErase)
        DrawDragRect(pDC,&rect,size,&m_rectLast,m_sizeLast);

    
    m_rectLast = rect;
    m_sizeLast = size;
}


void Tracker::Draw(HDC hDC) const
{
    
    SetMapMode(hDC,MM_TEXT);
    SetViewportOrgEx(hDC,0, 0,NULL);
    SetWindowOrgEx(hDC,0, 0,NULL);

    
    RECT rect = m_rect;
    NormalizeRect(&rect);

    HPEN pOldPen = NULL;
    HBRUSH pOldBrush = NULL;
    HGDIOBJ pTemp;
    int nOldROP;

    
    if ((m_nStyle & (dottedLine|solidLine)) != 0)
    {
        if (m_nStyle & dottedLine)
            pOldPen = (HPEN)SelectObject(hDC,_afxBlackDottedPen);
        else
            pOldPen = (HPEN)SelectObject(hDC,(HGDIOBJ)BLACK_PEN);
        pOldBrush = (HBRUSH)SelectObject(hDC,(HGDIOBJ)NULL_BRUSH);
        nOldROP = SetROP2(hDC,R2_COPYPEN);
        InflateRect(&rect,+1, +1);   
        Rectangle(hDC,rect.left, rect.top, rect.right, rect.bottom);
        SetROP2(hDC,nOldROP);
    }

    
    if ((m_nStyle & (hatchInside|hatchedBorder)) != 0)
        UnrealizeObject((HGDIOBJ)_afxHatchBrush);

    
    if ((m_nStyle & hatchInside) != 0)
    {
        pTemp = SelectObject(hDC,(HGDIOBJ)NULL_PEN);
        if (pOldPen == NULL)
            pOldPen = (HPEN)pTemp;
        pTemp = SelectObject(hDC,(HGDIOBJ)_afxHatchBrush);
        if (pOldBrush == NULL)
            pOldBrush = (HBRUSH)pTemp;
        SetBkMode(hDC,TRANSPARENT);
        nOldROP = SetROP2(hDC,R2_MASKNOTPEN);
        Rectangle(hDC,rect.left+1, rect.top+1, rect.right, rect.bottom);
        SetROP2(hDC,nOldROP);
    }

    
    if ((m_nStyle & hatchedBorder) != 0)
    {
        pTemp = SelectObject(hDC,(HGDIOBJ)_afxHatchBrush);
        if (pOldBrush == NULL)
            pOldBrush = (HBRUSH)pTemp;
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

    
    if ((m_nStyle & (resizeInside|resizeOutside)) != 0)
    {
        UINT mask = GetHandleMask();
        HBRUSH hbrush = CreateSolidBrush(RGB(0,0,0));
        for (int i = 0; i < 8; ++i)
        {
            if (mask & (1<<i))
            {
                GetHandleRect((TrackerHit)i, &rect);
                
                FillRect(hDC,&rect,hbrush);
            }
        }
        DeleteObject(hbrush);
    }

    
    if (pOldPen != NULL)
        SelectObject(hDC,pOldPen);
    if (pOldBrush != NULL)
        SelectObject(hDC,pOldBrush);
    RestoreDC(hDC,-1);
}


void Tracker::GetHandleRect(int nHandle,RECT* pHandleRect) const
{
    
    RECT rectT = m_rect;
    NormalizeRect(&rectT);
    if ((m_nStyle & (solidLine|dottedLine)) != 0)
        InflateRect(&rectT,+1, +1);

    
    
    nHandle = NormalizeHit(nHandle);

    
    int size = GetHandleSize();
    if (m_nStyle & resizeOutside)
        InflateRect(&rectT,size-1, size-1);

    
    int nWidth = rectT.right - rectT.left;
    int nHeight = rectT.bottom - rectT.top;
    RECT rect;
    const AFX_HANDLEINFO* pHandleInfo = &_afxHandleInfo[nHandle];
    rect.left = *(int*)((BYTE*)&rectT + pHandleInfo->nOffsetX);
    rect.top = *(int*)((BYTE*)&rectT + pHandleInfo->nOffsetY);
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
    if (lpRect == NULL)
        lpRect = (LPRECT)&m_rect;

    int size = m_nHandleSize;
    if (!(m_nStyle & resizeOutside))
    {
        
        int sizeMax = min(abs(lpRect->right - lpRect->left),
                          abs(lpRect->bottom - lpRect->top));
        if (size * 2 > sizeMax)
            size = sizeMax / 2;
    }
    return size;
}


UINT Tracker::GetHandleMask() const
{
    UINT mask = 0x0F;   
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
    const AFX_HANDLEINFO* pHandleInfo = &_afxHandleInfo[nHandle];
    if (m_rect.right - m_rect.left < 0)
    {
        nHandle = (TrackerHit)pHandleInfo->nInvertX;
        pHandleInfo = &_afxHandleInfo[nHandle];
    }
    if (m_rect.bottom - m_rect.top < 0)
        nHandle = (TrackerHit)pHandleInfo->nInvertY;
    return nHandle;
}


int Tracker::HitTestHandles(POINT point) const
{
    RECT rect;
    UINT mask = GetHandleMask();

    
    GetTrueRect(&rect);
    if (!PtInRect(&rect,point))
        return hitNothing;  

    
    for (int i = 0; i < 8; ++i)
    {
        if (mask & (1<<i))
        {
            GetHandleRect((TrackerHit)i, &rect);
            if (PtInRect(&rect,point))
                return (TrackerHit)i;
        }
    }

    
    if ((m_nStyle & hatchedBorder) == 0)
    {
        RECT rect = m_rect;
        NormalizeRect(&rect);
        if ((m_nStyle & dottedLine|solidLine) != 0)
            InflateRect(&rect,+1, +1);
        if (!PtInRect(&rect,point))
            return hitNothing;  
    }
    return hitMiddle;   
}



void Tracker::GetModifyPointers(
    int nHandle, int** ppx, int** ppy, int* px, int* py)
{
    if (nHandle == hitMiddle)
        nHandle = hitTopLeft;   

    *ppx = NULL;
    *ppy = NULL;

    
    
    

    const AFX_HANDLEINFO* pHandleInfo = &_afxHandleInfo[nHandle];
    if (pHandleInfo->nInvertX != nHandle)
    {
        *ppx = (int*)((BYTE*)&m_rect + pHandleInfo->nOffsetX);
        if (px != NULL)
            *px = **ppx;
    }
    else
    {
        
        if (px != NULL)
            *px = m_rect.left + (m_rect.left-m_rect.right) / 2;
    }
    if (pHandleInfo->nInvertY != nHandle)
    {
        *ppy = (int*)((BYTE*)&m_rect + pHandleInfo->nOffsetY);
        if (py != NULL)
            *py = **ppy;
    }
    else
    {
        
        if (py != NULL)
            *py = m_rect.top + (m_rect.top-m_rect.bottom) / 2;
    }
}




#if defined(_MSC_VER)
#pragma warning(disable: 4505)
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
