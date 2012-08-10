/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _SV_SPLIT_HXX
#define _SV_SPLIT_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/window.hxx>

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
    sal_Bool                mbHorzSplit;
    sal_Bool                mbDragFull;
    sal_Bool                mbKbdSplitting;
    long                mbInKeyEvent;
    long                mnKeyboardStepSize;
    Link                maStartSplitHdl;
    Link                maSplitHdl;
    Link                maEndSplitHdl;

    SAL_DLLPRIVATE void      ImplInitSplitterData();
    SAL_DLLPRIVATE void      ImplDrawSplitter();
    SAL_DLLPRIVATE void      ImplSplitMousePos( Point& rPos );
    SAL_DLLPRIVATE void      ImplStartKbdSplitting();
    SAL_DLLPRIVATE void      ImplKbdTracking( KeyCode aKeyCode );
    SAL_DLLPRIVATE sal_Bool      ImplSplitterActive();
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
    virtual void        EndSplit();
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

    sal_Bool                IsHorizontal() const { return mbHorzSplit; }

    // set the stepsize of the splitter for cursor movement
    // the default is 10% of the reference window's width/height
    void                SetKeyboardStepSize( long nStepSize );

    void                SetStartSplitHdl( const Link& rLink ) { maStartSplitHdl = rLink; }
    const Link&         GetStartSplitHdl() const { return maStartSplitHdl; }
    void                SetSplitHdl( const Link& rLink ) { maSplitHdl = rLink; }
    void                SetEndSplitHdl( const Link& rLink ) { maEndSplitHdl = rLink; }
    const Link&         GetEndSplitHdl() const { return maEndSplitHdl; }
    const Link&         GetSplitHdl() const { return maSplitHdl; }
};

#endif  // _SV_SPLIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
