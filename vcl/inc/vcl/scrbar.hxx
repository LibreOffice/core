/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scrbar.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:09:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_SCRBAR_HXX
#define _SV_SCRBAR_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif
#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _SV_CTRL_HXX
#include <vcl/ctrl.hxx>
#endif

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
    USHORT          mnDragDraw;
    USHORT          mnStateFlags;
    ScrollType      meScrollType;
    ScrollType      meDDScrollType;
    BOOL            mbCalcSize;
    BOOL            mbFullDrag;
    Link            maScrollHdl;
    Link            maEndScrollHdl;

    SAL_DLLPRIVATE Rectangle*   ImplFindPartRect( const Point& rPt );
    using Window::ImplInit;
    SAL_DLLPRIVATE void         ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void         ImplInitStyle( WinBits nStyle );
    SAL_DLLPRIVATE void         ImplLoadRes( const ResId& rResId );
    SAL_DLLPRIVATE void         ImplUpdateRects( BOOL bUpdate = TRUE );
    SAL_DLLPRIVATE long         ImplCalcThumbPos( long nPixPos );
    SAL_DLLPRIVATE long         ImplCalcThumbPosPix( long nPos );
    SAL_DLLPRIVATE void         ImplCalc( BOOL bUpdate = TRUE );
    SAL_DLLPRIVATE void         ImplDraw( USHORT nDrawFlags, OutputDevice* pOutDev  );
    using Window::ImplScroll;
    SAL_DLLPRIVATE long         ImplScroll( long nNewPos, BOOL bCallEndScroll );
    SAL_DLLPRIVATE long         ImplDoAction( BOOL bCallEndScroll );
    SAL_DLLPRIVATE void         ImplDoMouseAction( const Point& rPos, BOOL bCallAction = TRUE );
    SAL_DLLPRIVATE void         ImplInvert();
    SAL_DLLPRIVATE BOOL         ImplDrawNative( USHORT nDrawFlags );
    SAL_DLLPRIVATE void         ImplDragThumb( const Point& rMousePos );
    DECL_DLLPRIVATE_LINK(       ImplTimerHdl, Timer* );
    DECL_DLLPRIVATE_LINK(       ImplAutoTimerHdl, AutoTimer* );

public:
                    ScrollBar( Window* pParent, WinBits nStyle = WB_VERT );
                    ScrollBar( Window* pParent, const ResId& rResId );
                    ~ScrollBar();

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    Tracking( const TrackingEvent& rTEvt );
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, ULONG nFlags );
    virtual void    Resize();
    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    virtual long    PreNotify( NotifyEvent& rNEvt );
    virtual void    GetFocus();
    virtual void    LoseFocus();

    using Window::Scroll;
    virtual void    Scroll();
    virtual void    EndScroll();

    long            DoScroll( long nNewPos );
    long            DoScrollAction( ScrollType eScrollType );

    void            EnableDrag( BOOL bEnable = TRUE )
                        { mbFullDrag = bEnable; }
    BOOL            IsDragEnabled() const { return mbFullDrag; }

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
                    ScrollBarBox( Window* pParent, WinBits nStyle = 0 );
                    ScrollBarBox( Window* pParent, const ResId& rResId );

    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
};

#endif // _SV_SCRBAR_HXX
