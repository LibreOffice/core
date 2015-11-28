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

#ifndef INCLUDED_EMBEDSERV_SOURCE_INC_SYSWINWRAPPER_HXX
#define INCLUDED_EMBEDSERV_SOURCE_INC_SYSWINWRAPPER_HXX

#include <windows.h>

/**
 ** CWindow:  Our basic window class.
 **/


class DocumentHolder;


namespace winwrap {


    void TransformRect(LPRECT rect,HWND pWnd,HWND pWndClipTo);


    LRESULT APIENTRY HatchWndProc(
        HWND hWnd, UINT iMsg
        , WPARAM wParam, LPARAM lParam);


    BOOL HatchWindowRegister(HINSTANCE hInst);

    class CWindow
    {
    protected:
        HINSTANCE   m_hInst;            //Task instance
        HWND        m_hWnd;             //Window handle of the window

    public:
        //Standard Class Functions
        CWindow(HINSTANCE);
        ~CWindow();

        //Just returns members.  No need to modify
        HWND        Window();
        HINSTANCE   Instance();
    };



    class Tracker {
    public:
        // Constructors
        Tracker();
        Tracker(LPCRECT lpSrcRect, UINT nStyle);

        // Style Flags
        enum StyleFlags
        {
            solidLine = 1, dottedLine = 2, hatchedBorder = 4,
            resizeInside = 8, resizeOutside = 16, hatchInside = 32,
        };

        // Hit-Test codes
        enum TrackerHit
        {
            hitNothing = -1,
            hitTopLeft = 0, hitTopRight = 1,
            hitBottomRight = 2, hitBottomLeft = 3,
            hitTop = 4, hitRight = 5, hitBottom = 6,
            hitLeft = 7, hitMiddle = 8
        };

        // Attributes
        UINT m_nStyle;      // current state
        RECT m_rect;       // current position (always in pixels)
        SIZE m_sizeMin;    // minimum X and Y size during track operation
        int m_nHandleSize;  // size of resize handles (default from WIN.INI)

        // Operations
        void Draw(HDC hDC) const;
        void GetTrueRect(LPRECT lpTrueRect) const;
        BOOL SetCursor(HWND hWnd,UINT nHitTest) const;
        BOOL Track(HWND hWnd,POINT point,BOOL bAllowInvert = FALSE,
                   HWND hWndClipTo = NULL);
//         BOOL TrackRubberBand(HWND hWnd,POINT point,BOOL bAllowInvert = TRUE);
        int HitTest(POINT point) const;
        int NormalizeHit(int nHandle) const;

        // Overridables
        virtual void DrawTrackerRect(
            LPRECT lpRect, HWND hWndClipTo,
            HDC hDC, HWND hWnd);
        virtual void AdjustRect(int nHandle, LPRECT lpRect);
        virtual void OnChangedRect(const RECT& rectOld);
        virtual UINT GetHandleMask() const;

// Implementation
    public:
        virtual ~Tracker();

protected:
        BOOL m_bAllowInvert; // flag passed to Track or TrackRubberBand
        RECT m_rectLast;
        SIZE m_sizeLast;
        BOOL m_bErase;       // TRUE if DrawTrackerRect is called for erasing
        BOOL m_bFinalErase;  // TRUE if DragTrackerRect called for final erase

        // implementation helpers
        int HitTestHandles(POINT point) const;
        void GetHandleRect(int nHandle,RECT* pHandleRect) const;
        void GetModifyPointers(
            int nHandle,int**ppx, int**ppy, int* px, int*py);
        virtual int GetHandleSize(LPRECT lpRect = NULL) const;
        BOOL TrackHandle(int nHandle,HWND hWnd,POINT point,HWND hWndClipTo);
        void Construct();
    };



//Width of the border
#define HATCHWIN_BORDERWIDTHDEFAULT     4


    class CHatchWin : public CWindow
    {
        friend LRESULT APIENTRY HatchWndProc(HWND, UINT, WPARAM, LPARAM);

    public:

        const DocumentHolder* m_pDocHolder;
        Tracker               m_aTracker;

        int         m_dBorder;
        int         m_dBorderOrg;
        WORD        m_uID;
        HWND        m_hWndParent;
        HWND        m_hWndKid;
        HWND        m_hWndAssociate;
        RECT        m_rcPos;
        RECT        m_rcClip;

    public:
        CHatchWin(HINSTANCE,const DocumentHolder*);
        ~CHatchWin();

        BOOL        Init(HWND, WORD, HWND);

        HWND        HwndAssociateSet(HWND);
        HWND        HwndAssociateGet();

        void        RectsSet(LPRECT, LPRECT);
        void        ChildSet(HWND);
        void        ShowHatch(BOOL);
        void        SetTrans();
    };

}

#endif // INCLUDED_EMBEDSERV_SOURCE_INC_SYSWINWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
