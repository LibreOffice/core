/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SV_SLIDER_HXX
#define _SV_SLIDER_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/ctrl.hxx>
// for enum ScrollType
#include <vcl/scrbar.hxx>

// ----------
// - Slider -
// ----------

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
    sal_uInt16          mnDragDraw;
    sal_uInt16          mnStateFlags;
    ScrollType      meScrollType;
    sal_Bool            mbCalcSize;
    sal_Bool            mbFullDrag;
    Link            maSlideHdl;
    Link            maEndSlideHdl;

    using Control::ImplInitSettings;
    using Window::ImplInit;
    SAL_DLLPRIVATE void ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void ImplLoadRes( const ResId& rResId );
    SAL_DLLPRIVATE void ImplInitSettings();
    SAL_DLLPRIVATE void ImplUpdateRects( sal_Bool bUpdate = sal_True );
    SAL_DLLPRIVATE long ImplCalcThumbPos( long nPixPos );
    SAL_DLLPRIVATE long ImplCalcThumbPosPix( long nPos );
    SAL_DLLPRIVATE void ImplCalc( sal_Bool bUpdate = sal_True );
    SAL_DLLPRIVATE void ImplDraw( sal_uInt16 nDrawFlags );
    SAL_DLLPRIVATE sal_Bool ImplIsPageUp( const Point& rPos );
    SAL_DLLPRIVATE sal_Bool ImplIsPageDown( const Point& rPos );
    SAL_DLLPRIVATE long ImplSlide( long nNewPos, sal_Bool bCallEndSlide );
    SAL_DLLPRIVATE long ImplDoAction( sal_Bool bCallEndSlide );
    SAL_DLLPRIVATE void ImplDoMouseAction( const Point& rPos, sal_Bool bCallAction = sal_True );
    SAL_DLLPRIVATE long ImplDoSlide( long nNewPos );
    SAL_DLLPRIVATE long ImplDoSlideAction( ScrollType eScrollType );

public:
                    Slider( Window* pParent, WinBits nStyle = WB_HORZ );
                    Slider( Window* pParent, const ResId& rResId );

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    Tracking( const TrackingEvent& rTEvt );
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    Resize();
    virtual void    RequestHelp( const HelpEvent& rHEvt );
    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    virtual void    Slide();
    virtual void    EndSlide();

    void            EnableDrag( sal_Bool bEnable = sal_True )
                        { mbFullDrag = bEnable; }
    sal_Bool            IsDragEnabled() const { return mbFullDrag; }

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

    long            GetDelta() const { return mnDelta; }

    Size            CalcWindowSizePixel();

    void            SetSlideHdl( const Link& rLink ) { maSlideHdl = rLink; }
    const Link&     GetSlideHdl() const { return maSlideHdl;    }
    void            SetEndSlideHdl( const Link& rLink ) { maEndSlideHdl = rLink; }
    const Link&     GetEndSlideHdl() const { return maEndSlideHdl; }
};

#endif // _SV_SLIDER_HXX
