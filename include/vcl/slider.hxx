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

#ifndef INCLUDED_VCL_SLIDER_HXX
#define INCLUDED_VCL_SLIDER_HXX

#include <vcl/dllapi.h>
#include <vcl/ctrl.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/field.hxx>

class VCL_DLLPUBLIC Slider : public Control
{
private:
    Rectangle       maChannel1Rect;
    Rectangle       maChannel2Rect;
    Rectangle       maThumbRect;
    long            mnStartPos;
    long            mnMouseOff;
    long            mnThumbPixOffset;
    long            mnThumbPixRange;
    long            mnThumbPixPos;
    long            mnChannelPixOffset;
    long            mnChannelPixRange;
    long            mnChannelPixTop;
    long            mnChannelPixBottom;
    long            mnMinRange;
    long            mnMaxRange;
    long            mnThumbPos;
    long            mnLineSize;
    long            mnPageSize;
    long            mnDelta;
    sal_uInt16      mnDragDraw;
    sal_uInt16      mnStateFlags;
    ScrollType      meScrollType;
    bool            mbCalcSize;
    bool            mbFullDrag;

    VclPtr<NumericField> mpLinkedField;

    Link<Slider*,void>   maSlideHdl;
    Link<Slider*,void>   maEndSlideHdl;

    DECL_LINK(LinkedFieldModifyHdl, NumericField*);
    DECL_LINK_TYPED(LinkedFieldLoseFocusHdl, Control&, void);

    using Control::ImplInitSettings;
    using Window::ImplInit;
    SAL_DLLPRIVATE void ImplInit( vcl::Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void ImplInitSettings();
    SAL_DLLPRIVATE void ImplUpdateRects( bool bUpdate = true );
    SAL_DLLPRIVATE long ImplCalcThumbPos( long nPixPos );
    SAL_DLLPRIVATE long ImplCalcThumbPosPix( long nPos );
    SAL_DLLPRIVATE void ImplCalc( bool bUpdate = true );
    SAL_DLLPRIVATE void ImplDraw(vcl::RenderContext& rRenderContext);
    SAL_DLLPRIVATE bool ImplIsPageUp( const Point& rPos );
    SAL_DLLPRIVATE bool ImplIsPageDown( const Point& rPos );
    SAL_DLLPRIVATE long ImplSlide( long nNewPos, bool bCallEndSlide );
    SAL_DLLPRIVATE long ImplDoAction( bool bCallEndSlide );
    SAL_DLLPRIVATE void ImplDoMouseAction( const Point& rPos, bool bCallAction = true );
    SAL_DLLPRIVATE long ImplDoSlide( long nNewPos );
    SAL_DLLPRIVATE long ImplDoSlideAction( ScrollType eScrollType );
    SAL_DLLPRIVATE void ImplSetFieldLink(const Link<>& rLink);
    SAL_DLLPRIVATE void ImplUpdateLinkedField();

public:
                    Slider( vcl::Window* pParent, WinBits nStyle = WB_HORZ );
    virtual         ~Slider();
    virtual void    dispose() SAL_OVERRIDE;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    Tracking( const TrackingEvent& rTEvt ) SAL_OVERRIDE;
    virtual void    KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
    virtual void    Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) SAL_OVERRIDE;
    virtual void    Resize() SAL_OVERRIDE;
    virtual void    RequestHelp( const HelpEvent& rHEvt ) SAL_OVERRIDE;
    virtual void    StateChanged( StateChangedType nType ) SAL_OVERRIDE;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    void            Slide();
    void            EndSlide();

    void            SetRangeMin(long nNewRange);
    long            GetRangeMin() const { return mnMinRange; }
    void            SetRangeMax(long nNewRange);
    long            GetRangeMax() const { return mnMaxRange; }
    void            SetRange( const Range& rRange );
    void            SetThumbPos( long nThumbPos );
    long            GetThumbPos() const { return mnThumbPos; }
    void            SetLineSize( long nNewSize ) { mnLineSize = nNewSize; }
    long            GetLineSize() const { return mnLineSize; }
    void            SetPageSize( long nNewSize ) { mnPageSize = nNewSize; }
    long            GetPageSize() const { return mnPageSize; }

    Size            CalcWindowSizePixel();

    void            SetLinkedField(VclPtr<NumericField> pField);

    void            SetSlideHdl( const Link<Slider*,void>& rLink ) { maSlideHdl = rLink; }
    void            SetEndSlideHdl( const Link<Slider*,void>& rLink ) { maEndSlideHdl = rLink; }
};

#endif // INCLUDED_VCL_SLIDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
