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

class Slider final : public Control
{
private:
    tools::Rectangle       maChannel1Rect;
    tools::Rectangle       maChannel2Rect;
    tools::Rectangle       maThumbRect;
    long            mnStartPos;
    long            mnMouseOff;
    long            mnThumbPixOffset;
    long            mnThumbPixRange;
    long            mnThumbPixPos;
    long            mnThumbSize;
    long            mnChannelPixRange;
    long            mnChannelPixTop;
    long            mnChannelPixBottom;
    long            mnMinRange;
    long            mnMaxRange;
    long            mnThumbPos;
    long            mnLineSize;
    long            mnPageSize;
    sal_uInt16      mnStateFlags;
    ScrollType      meScrollType;
    bool            mbCalcSize;

    Link<Slider*,void>   maSlideHdl;

    using Control::ImplInitSettings;
    using Window::ImplInit;
    void ImplInit( vcl::Window* pParent, WinBits nStyle );
    void ImplInitSettings();
    void ImplUpdateRects( bool bUpdate = true );
    long ImplCalcThumbPos( long nPixPos );
    long ImplCalcThumbPosPix( long nPos );
    void ImplCalc( bool bUpdate = true );
    void ImplDraw(vcl::RenderContext& rRenderContext);
    bool ImplIsPageUp( const Point& rPos );
    bool ImplIsPageDown( const Point& rPos );
    long ImplSlide( long nNewPos );
    long ImplDoAction( );
    void ImplDoMouseAction( const Point& rPos, bool bCallAction );
    void ImplDoSlide( long nNewPos );
    void ImplDoSlideAction( ScrollType eScrollType );

public:
                    Slider( vcl::Window* pParent, WinBits nStyle);
    virtual         ~Slider() override;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void    Tracking( const TrackingEvent& rTEvt ) override;
    virtual void    KeyInput( const KeyEvent& rKEvt ) override;
    virtual void    Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual void    Resize() override;
    virtual void    StateChanged( StateChangedType nType ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

    void            Slide();

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

    void            SetSlideHdl( const Link<Slider*,void>& rLink ) { maSlideHdl = rLink; }
};

#endif // INCLUDED_VCL_SLIDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
