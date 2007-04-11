/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: split.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:11:48 $
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

#ifndef _SV_SPLIT_HXX
#define _SV_SPLIT_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif
#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

#define SPLITTER_DEFAULTSTEPSIZE 0xFFFF

// ------------
// - Splitter -
// ------------

class VCL_DLLPUBLIC Splitter : public Window
{
private:
    Window*             mpRefWin;
    long                mnSplitPos;
    long                mnLastSplitPos;
    long                mnStartSplitPos;
    Point               maDragPos;
    Rectangle           maDragRect;
    BOOL                mbHorzSplit;
    BOOL                mbDragFull;
    BOOL                mbKbdSplitting;
    long                mbInKeyEvent;
    long                mnKeyboardStepSize;
    Link                maStartSplitHdl;
    Link                maSplitHdl;

    SAL_DLLPRIVATE void      ImplInitSplitterData();
    SAL_DLLPRIVATE void      ImplDrawSplitter();
    SAL_DLLPRIVATE void      ImplSplitMousePos( Point& rPos );
    SAL_DLLPRIVATE void      ImplStartKbdSplitting();
    SAL_DLLPRIVATE void      ImplKbdTracking( KeyCode aKeyCode );
    SAL_DLLPRIVATE BOOL      ImplSplitterActive();
    SAL_DLLPRIVATE Splitter* ImplFindSibling();
    SAL_DLLPRIVATE void      ImplRestoreSplitter();

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE           Splitter (const Splitter &);
    SAL_DLLPRIVATE           Splitter& operator= (const Splitter &);

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void      ImplInit( Window* pParent, WinBits nWinStyle );

public:
                        Splitter( Window* pParent, WinBits nStyle = WB_VSCROLL );
                        Splitter( Window* pParent, const ResId& rResId );
                        ~Splitter();

    virtual void        StartSplit();
    virtual void        Split();
    virtual void        Splitting( Point& rSplitPos );

    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        Tracking( const TrackingEvent& rTEvt );

    virtual long        Notify( NotifyEvent& rNEvt );

    virtual void        GetFocus();
    virtual void        LoseFocus();
    virtual void        KeyInput( const KeyEvent& rKEvt );
    virtual void        Paint( const Rectangle& rPaintRect );

    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

    void                StartDrag();

    void                SetDragRectPixel( const Rectangle& rDragRect,
                                          Window* pRefWin = NULL );
    const Rectangle&    GetDragRectPixel() const { return maDragRect; }
    Window*             GetDragWindow() const { return mpRefWin; }

    virtual void        SetSplitPosPixel( long nPos );
    long                GetSplitPosPixel() const { return mnSplitPos; }

    void                SetLastSplitPosPixel( long nNewPos );
    long                GetLastSplitPosPixel() const { return mnLastSplitPos; }

    BOOL                IsHorizontal() const { return mbHorzSplit; }

    // set the stepsize of the splitter for cursor movement
    // the default is 10% of the reference window's width/height
    void                SetKeyboardStepSize( long nStepSize );
    long                GetKeyboardStepSize() const;

    void                SetStartSplitHdl( const Link& rLink ) { maStartSplitHdl = rLink; }
    const Link&         GetStartSplitHdl() const { return maStartSplitHdl; }
    void                SetSplitHdl( const Link& rLink ) { maSplitHdl = rLink; }
    const Link&         GetSplitHdl() const { return maSplitHdl; }
};

#endif  // _SV_SPLIT_HXX
