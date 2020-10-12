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

#include <vcl/dllapi.h>
#include <vcl/ctrl.hxx>
#include <memory>


enum class ScrollType
{
    DontKnow,
    LineUp, LineDown,
    PageUp, PageDown,
    Drag
};


struct ImplScrollBarData;

class VCL_DLLPUBLIC ScrollBar : public Control
{
private:
    tools::Rectangle       maBtn1Rect;
    tools::Rectangle       maBtn2Rect;
    tools::Rectangle       maPage1Rect;
    tools::Rectangle       maPage2Rect;
    tools::Rectangle       maThumbRect;
    tools::Rectangle       maTrackRect;
    std::unique_ptr<ImplScrollBarData> mpData;
    tools::Long            mnStartPos;
    tools::Long            mnMouseOff;
    tools::Long            mnThumbPixRange;
    tools::Long            mnThumbPixPos;
    tools::Long            mnThumbPixSize;
    tools::Long            mnMinRange;
    tools::Long            mnMaxRange;
    tools::Long            mnThumbPos;
    tools::Long            mnVisibleSize;
    tools::Long            mnLineSize;
    tools::Long            mnPageSize;
    tools::Long            mnDelta;
    sal_uInt16      mnStateFlags;
    ScrollType      meScrollType;
    bool            mbCalcSize;
    bool            mbFullDrag;
    Link<ScrollBar*,void>       maScrollHdl;
    Link<ScrollBar*,void>       maEndScrollHdl;

    SAL_DLLPRIVATE tools::Rectangle*   ImplFindPartRect( const Point& rPt );
    using Window::ImplInit;
    SAL_DLLPRIVATE void         ImplInit( vcl::Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void         ImplInitStyle( WinBits nStyle );
    SAL_DLLPRIVATE void         ImplUpdateRects( bool bUpdate = true );
    SAL_DLLPRIVATE tools::Long         ImplCalcThumbPos( tools::Long nPixPos );
    SAL_DLLPRIVATE tools::Long         ImplCalcThumbPosPix( tools::Long nPos );
    SAL_DLLPRIVATE void         ImplCalc( bool bUpdate = true );
    SAL_DLLPRIVATE void         ImplDraw(vcl::RenderContext& rRenderContext);
    using Window::ImplScroll;
    SAL_DLLPRIVATE tools::Long         ImplScroll( tools::Long nNewPos, bool bCallEndScroll );
    SAL_DLLPRIVATE tools::Long         ImplDoAction( bool bCallEndScroll );
    SAL_DLLPRIVATE void         ImplDoMouseAction( const Point& rPos, bool bCallAction = true );
    SAL_DLLPRIVATE void         ImplInvert();
    SAL_DLLPRIVATE bool         ImplDrawNative(vcl::RenderContext& rRenderContext, sal_uInt16 nDrawFlags);
    SAL_DLLPRIVATE void         ImplDragThumb( const Point& rMousePos );
    SAL_DLLPRIVATE Size         getCurrentCalcSize() const;
    DECL_DLLPRIVATE_LINK( ImplAutoTimerHdl, Timer*, void );

public:
    explicit        ScrollBar( vcl::Window* pParent, WinBits nStyle = WB_VERT );
    virtual         ~ScrollBar() override;
    virtual void    dispose() override;

    virtual void MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual void Tracking(const TrackingEvent& rTEvt) override;
    virtual void KeyInput(const KeyEvent& rKEvt) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual void Draw(OutputDevice* pDev, const Point& rPos, DrawFlags nFlags) override;
    virtual void Move() override;
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

    tools::Long            DoScroll( tools::Long nNewPos );
    tools::Long            DoScrollAction( ScrollType eScrollType );

    void            EnableDrag() { mbFullDrag = true; }

    void            SetRangeMin( tools::Long nNewRange );
    tools::Long            GetRangeMin() const { return mnMinRange; }
    void            SetRangeMax( tools::Long nNewRange );
    tools::Long            GetRangeMax() const { return mnMaxRange; }
    void            SetRange( const Range& rRange );
    Range           GetRange() const { return Range( GetRangeMin(), GetRangeMax() ); }
    void            SetThumbPos( tools::Long nThumbPos );
    tools::Long            GetThumbPos() const { return mnThumbPos; }
    void            SetLineSize( tools::Long nNewSize ) { mnLineSize = nNewSize; }
    tools::Long            GetLineSize() const { return mnLineSize; }
    void            SetPageSize( tools::Long nNewSize ) { mnPageSize = nNewSize; }
    tools::Long            GetPageSize() const { return mnPageSize; }
    void            SetVisibleSize( tools::Long nNewSize );
    tools::Long            GetVisibleSize() const { return mnVisibleSize; }

    tools::Long            GetDelta() const { return mnDelta; }
    ScrollType      GetType() const { return meScrollType; }

    void            SetScrollHdl( const Link<ScrollBar*,void>& rLink ) { maScrollHdl = rLink; }
    const Link<ScrollBar*,void>&   GetScrollHdl() const { return maScrollHdl; }
    void            SetEndScrollHdl( const Link<ScrollBar*,void>& rLink ) { maEndScrollHdl = rLink; }

    virtual Size    GetOptimalSize() const override;
};


class VCL_DLLPUBLIC ScrollBarBox final : public vcl::Window
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
