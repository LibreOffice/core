/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: syswinwrapper.hxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


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
        ~CWindow(void);

        //Just returns members.  No need to modify
        HWND        Window(void);
        HINSTANCE   Instance(void);
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
        UINT        m_uID;
        HWND        m_hWndParent;
        HWND        m_hWndKid;
        HWND        m_hWndAssociate;
        RECT        m_rcPos;
        RECT        m_rcClip;

    public:
        CHatchWin(HINSTANCE,const DocumentHolder*);
        ~CHatchWin(void);

        BOOL        Init(HWND, UINT, HWND);

        HWND        HwndAssociateSet(HWND);
        HWND        HwndAssociateGet(void);

        void        RectsSet(LPRECT, LPRECT);
        void        ChildSet(HWND);
        void        ShowHatch(BOOL);
        void        SetTrans();
    };

}
