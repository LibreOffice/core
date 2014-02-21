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

#ifndef INCLUDED_VCL_SCRBAR_HXX
#define INCLUDED_VCL_SCRBAR_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/ctrl.hxx>

class AutoTimer;

// -------------------
// - ScrollBar-Types -
// -------------------

enum ScrollType { SCROLL_DONTKNOW, SCROLL_LINEUP, SCROLL_LINEDOWN,
                  SCROLL_PAGEUP, SCROLL_PAGEDOWN, SCROLL_DRAG, SCROLL_SET };

// -------------
// - ScrollBar -
// -------------
struct ImplScrollBarData;

class VCL_DLLPUBLIC ScrollBar : public Control
{
private:
    Rectangle       maBtn1Rect;
    Rectangle       maBtn2Rect;
    Rectangle       maPage1Rect;
    Rectangle       maPage2Rect;
    Rectangle       maThumbRect;
    ImplScrollBarData* mpData;
    long            mnStartPos;
    long            mnMouseOff;
    long            mnThumbPixRange;
    long            mnThumbPixPos;
    long            mnThumbPixSize;
    long            mnMinRange;
    long            mnMaxRange;
    long            mnThumbPos;
    long            mnVisibleSize;
    long            mnLineSize;
    long            mnPageSize;
    long            mnDelta;
    sal_uInt16          mnDragDraw;
    sal_uInt16          mnStateFlags;
    ScrollType      meScrollType;
    ScrollType      meDDScrollType;
    bool            mbCalcSize;
    bool            mbFullDrag;
    Link            maScrollHdl;
    Link            maEndScrollHdl;

    SAL_DLLPRIVATE Rectangle*   ImplFindPartRect( const Point& rPt );
    using Window::ImplInit;
    SAL_DLLPRIVATE void         ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void         ImplInitStyle( WinBits nStyle );
    SAL_DLLPRIVATE void         ImplLoadRes( const ResId& rResId );
    SAL_DLLPRIVATE void         ImplUpdateRects( bool bUpdate = true );
    SAL_DLLPRIVATE long         ImplCalcThumbPos( long nPixPos );
    SAL_DLLPRIVATE long         ImplCalcThumbPosPix( long nPos );
    SAL_DLLPRIVATE void         ImplCalc( bool bUpdate = true );
    SAL_DLLPRIVATE void         ImplDraw( sal_uInt16 nDrawFlags, OutputDevice* pOutDev  );
    using Window::ImplScroll;
    SAL_DLLPRIVATE long         ImplScroll( long nNewPos, bool bCallEndScroll );
    SAL_DLLPRIVATE long         ImplDoAction( bool bCallEndScroll );
    SAL_DLLPRIVATE void         ImplDoMouseAction( const Point& rPos, bool bCallAction = true );
    SAL_DLLPRIVATE void         ImplInvert();
    SAL_DLLPRIVATE bool     ImplDrawNative( sal_uInt16 nDrawFlags );
    SAL_DLLPRIVATE void         ImplDragThumb( const Point& rMousePos );
    SAL_DLLPRIVATE Size         getCurrentCalcSize() const;
    DECL_DLLPRIVATE_LINK(       ImplTimerHdl, Timer* );
    DECL_DLLPRIVATE_LINK(       ImplAutoTimerHdl, void* );

public:
    explicit        ScrollBar( Window* pParent, WinBits nStyle = WB_VERT );
    explicit        ScrollBar( Window* pParent, const ResId& );
    virtual         ~ScrollBar();

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    Tracking( const TrackingEvent& rTEvt );
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
    virtual void    Resize();
    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    virtual bool    PreNotify( NotifyEvent& rNEvt );
    virtual void    GetFocus();
    virtual void    LoseFocus();

    using Window::Scroll;
    virtual void    Scroll();
    virtual void    EndScroll();

    long            DoScroll( long nNewPos );
    long            DoScrollAction( ScrollType eScrollType );

    void            EnableDrag( bool bEnable = true )
                        { mbFullDrag = bEnable; }
    bool            IsDragEnabled() const { return mbFullDrag; }

    void            SetRangeMin( long nNewRange );
    long            GetRangeMin() const { return mnMinRange; }
    void            SetRangeMax( long nNewRange );
    long            GetRangeMax() const { return mnMaxRange; }
    void            SetRange( const Range& rRange );
    Range           GetRange() const { return Range( GetRangeMin(), GetRangeMax() ); }
    void            SetThumbPos( long nThumbPos );
    long            GetThumbPos() const { return mnThumbPos; }
    void            SetLineSize( long nNewSize ) { mnLineSize = nNewSize; }
    long            GetLineSize() const { return mnLineSize; }
    void            SetPageSize( long nNewSize ) { mnPageSize = nNewSize; }
    long            GetPageSize() const { return mnPageSize; }
    void            SetVisibleSize( long nNewSize );
    long            GetVisibleSize() const { return mnVisibleSize; }

    long            GetDelta() const { return mnDelta; }
    ScrollType      GetType() const { return meScrollType; }

    void            SetScrollHdl( const Link& rLink ) { maScrollHdl = rLink; }
    const Link&     GetScrollHdl() const { return maScrollHdl;    }
    void            SetEndScrollHdl( const Link& rLink ) { maEndScrollHdl = rLink; }
    const Link&     GetEndScrollHdl() const { return maEndScrollHdl; }

    virtual Size    GetOptimalSize() const;
};

// ----------------
// - ScrollBarBox -
// ----------------

class VCL_DLLPUBLIC ScrollBarBox : public Window
{
private:
    using Window::ImplInit;
    SAL_DLLPRIVATE void ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void ImplInitSettings();

public:
    explicit        ScrollBarBox( Window* pParent, WinBits nStyle = 0 );

    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
};

#endif // INCLUDED_VCL_SCRBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
