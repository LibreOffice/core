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


// - ScrollBar-Types -


enum ScrollType { SCROLL_DONTKNOW, SCROLL_LINEUP, SCROLL_LINEDOWN,
                  SCROLL_PAGEUP, SCROLL_PAGEDOWN, SCROLL_DRAG, SCROLL_SET };


// - ScrollBar -

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
    sal_uInt16      mnDragDraw;
    sal_uInt16      mnStateFlags;
    ScrollType      meScrollType;
    ScrollType      meDDScrollType;
    bool            mbCalcSize;
    bool            mbFullDrag;
    Link<ScrollBar*,void>       maScrollHdl;
    Link<ScrollBar*,void>       maEndScrollHdl;

    SAL_DLLPRIVATE Rectangle*   ImplFindPartRect( const Point& rPt );
    using Window::ImplInit;
    SAL_DLLPRIVATE void         ImplInit( vcl::Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void         ImplInitStyle( WinBits nStyle );
    SAL_DLLPRIVATE void         ImplUpdateRects( bool bUpdate = true );
    SAL_DLLPRIVATE long         ImplCalcThumbPos( long nPixPos );
    SAL_DLLPRIVATE long         ImplCalcThumbPosPix( long nPos );
    SAL_DLLPRIVATE void         ImplCalc( bool bUpdate = true );
    SAL_DLLPRIVATE void         ImplDraw(vcl::RenderContext& rRenderContext, sal_uInt16 nDrawFlags);
    using Window::ImplScroll;
    SAL_DLLPRIVATE long         ImplScroll( long nNewPos, bool bCallEndScroll );
    SAL_DLLPRIVATE long         ImplDoAction( bool bCallEndScroll );
    SAL_DLLPRIVATE void         ImplDoMouseAction( const Point& rPos, bool bCallAction = true );
    SAL_DLLPRIVATE void         ImplInvert();
    SAL_DLLPRIVATE bool         ImplDrawNative(vcl::RenderContext& rRenderContext, sal_uInt16 nDrawFlags);
    SAL_DLLPRIVATE void         ImplDragThumb( const Point& rMousePos );
    SAL_DLLPRIVATE Size         getCurrentCalcSize() const;
    DECL_DLLPRIVATE_LINK_TYPED( ImplAutoTimerHdl, Timer*, void );

public:
    explicit        ScrollBar( vcl::Window* pParent, WinBits nStyle = WB_VERT );
    virtual         ~ScrollBar();
    virtual void    dispose() override;

    virtual void MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual void Tracking(const TrackingEvent& rTEvt) override;
    virtual void KeyInput(const KeyEvent& rKEvt) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) override;
    virtual void Draw(OutputDevice* pDev, const Point& rPos, const Size& rSize, DrawFlags nFlags) override;
    virtual void Resize() override;
    virtual void StateChanged(StateChangedType nType) override;
    virtual void DataChanged(const DataChangedEvent& rDCEvt) override;
    virtual bool PreNotify(NotifyEvent& rNEvt) override;
    virtual void GetFocus() override;
    virtual void LoseFocus() override;
    virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;

    using Window::Scroll;
    void            Scroll();
    virtual void    EndScroll();

    long            DoScroll( long nNewPos );
    long            DoScrollAction( ScrollType eScrollType );

    void            EnableDrag( bool bEnable = true )
                        { mbFullDrag = bEnable; }

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

    void            SetScrollHdl( const Link<ScrollBar*,void>& rLink ) { maScrollHdl = rLink; }
    void            SetEndScrollHdl( const Link<ScrollBar*,void>& rLink ) { maEndScrollHdl = rLink; }
    const Link<ScrollBar*,void>&   GetEndScrollHdl() const { return maEndScrollHdl; }

    virtual Size    GetOptimalSize() const override;
};


// - ScrollBarBox -


class VCL_DLLPUBLIC ScrollBarBox : public vcl::Window
{
private:
    using Window::ImplInit;
    SAL_DLLPRIVATE void ImplInit( vcl::Window* pParent, WinBits nStyle );

    virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;

public:
    explicit        ScrollBarBox( vcl::Window* pParent, WinBits nStyle = 0 );

    virtual void    StateChanged( StateChangedType nType ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;
};

#endif // INCLUDED_VCL_SCRBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
