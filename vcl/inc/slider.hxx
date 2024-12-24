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

#pragma once

#include <vcl/ctrl.hxx>

class Slider final : public Control
{
private:
    tools::Rectangle       maChannel1Rect;
    tools::Rectangle       maChannel2Rect;
    tools::Rectangle       maThumbRect;
    tools::Long            mnStartPos;
    tools::Long            mnMouseOff;
    tools::Long            mnThumbPixOffset;
    tools::Long            mnThumbPixRange;
    tools::Long            mnThumbPixPos;
    tools::Long            mnThumbSize;
    tools::Long            mnChannelPixRange;
    tools::Long            mnChannelPixTop;
    tools::Long            mnChannelPixBottom;
    tools::Long            mnMinRange;
    tools::Long            mnMaxRange;
    tools::Long            mnThumbPos;
    tools::Long            mnLineSize;
    tools::Long            mnPageSize;
    sal_uInt16      mnStateFlags;
    ScrollType      meScrollType;
    bool            mbCalcSize;

    Link<Slider*,void>   maSlideHdl;

    using Control::ImplInitSettings;
    using Window::ImplInit;
    void ImplInitSettings();
    void ImplUpdateRects( bool bUpdate = true );
    tools::Long ImplCalcThumbPos( tools::Long nPixPos ) const;
    tools::Long ImplCalcThumbPosPix( tools::Long nPos ) const;
    void ImplCalc( bool bUpdate = true );
    void ImplDraw(vcl::RenderContext& rRenderContext);
    bool ImplIsPageUp( const Point& rPos ) const;
    bool ImplIsPageDown( const Point& rPos ) const;
    tools::Long ImplSlide( tools::Long nNewPos );
    tools::Long ImplDoAction( );
    void ImplDoMouseAction( const Point& rPos, bool bCallAction );
    void ImplDoSlide( tools::Long nNewPos );
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

    void            SetRangeMin(tools::Long nNewRange);
    tools::Long            GetRangeMin() const { return mnMinRange; }
    void            SetRangeMax(tools::Long nNewRange);
    tools::Long            GetRangeMax() const { return mnMaxRange; }
    void            SetRange( const Range& rRange );
    void            SetThumbPos( tools::Long nThumbPos );
    tools::Long            GetThumbPos() const { return mnThumbPos; }
    void            SetLineSize( tools::Long nNewSize ) { mnLineSize = nNewSize; }
    tools::Long            GetLineSize() const { return mnLineSize; }
    void            SetPageSize( tools::Long nNewSize ) { mnPageSize = nNewSize; }
    tools::Long            GetPageSize() const { return mnPageSize; }

    Size            CalcWindowSizePixel() const;

    void            SetSlideHdl( const Link<Slider*,void>& rLink ) { maSlideHdl = rLink; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
