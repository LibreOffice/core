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

#ifndef INCLUDED_VCL_SPLIT_HXX
#define INCLUDED_VCL_SPLIT_HXX

#include <vcl/dllapi.h>
#include <vcl/window.hxx>

#define SPLITTER_DEFAULTSTEPSIZE 0xFFFF

class VCL_DLLPUBLIC Splitter : public Window
{
private:
    Window*             mpRefWin;
    long                mnSplitPos;
    long                mnLastSplitPos;
    long                mnStartSplitPos;
    Point               maDragPos;
    Rectangle           maDragRect;
    bool                mbHorzSplit;
    bool                mbDragFull;
    bool                mbKbdSplitting;
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
    SAL_DLLPRIVATE bool      ImplSplitterActive();
    SAL_DLLPRIVATE Splitter* ImplFindSibling();
    SAL_DLLPRIVATE void      ImplRestoreSplitter();
    SAL_DLLPRIVATE void      ImplInitHorVer(bool bNew);

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE           Splitter (const Splitter &);
    SAL_DLLPRIVATE           Splitter& operator= (const Splitter &);

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void      ImplInit( Window* pParent, WinBits nWinStyle );

public:
    explicit            Splitter( Window* pParent, WinBits nStyle = WB_VSCROLL );
    explicit            Splitter( Window* pParent, const ResId& );
    virtual             ~Splitter();

    virtual void        StartSplit();
    virtual void        EndSplit();
    virtual void        Split();
    virtual void        Splitting( Point& rSplitPos );

    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        Tracking( const TrackingEvent& rTEvt );

    virtual bool        Notify( NotifyEvent& rNEvt );

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

    bool            IsHorizontal() const { return mbHorzSplit; }
    void                SetHorizontal(bool bNew);

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

#endif // INCLUDED_VCL_SPLIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
