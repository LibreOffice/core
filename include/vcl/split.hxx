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
#include <vcl/keycod.hxx>
#include <vcl/window.hxx>

#define SPLITTER_DEFAULTSTEPSIZE 0xFFFF

class VCL_DLLPUBLIC Splitter : public vcl::Window
{
private:
    VclPtr<vcl::Window> mpRefWin;
    long                mnSplitPos;
    long                mnLastSplitPos;
    long                mnStartSplitPos;
    Point               maDragPos;
    tools::Rectangle           maDragRect;
    bool                mbHorzSplit;
    bool                mbDragFull;
    bool                mbKbdSplitting;
    bool                mbInKeyEvent;
    long                mnKeyboardStepSize;
    Link<Splitter*,void>   maStartSplitHdl;
    Link<Splitter*,void>   maSplitHdl;
    Link<Splitter*,void>   maEndSplitHdl;

    SAL_DLLPRIVATE void      ImplDrawSplitter();
    SAL_DLLPRIVATE void      ImplSplitMousePos( Point& rPos );
    SAL_DLLPRIVATE void      ImplStartKbdSplitting();
    SAL_DLLPRIVATE void      ImplKbdTracking( vcl::KeyCode aKeyCode );
    SAL_DLLPRIVATE bool      ImplSplitterActive();
    SAL_DLLPRIVATE Splitter* ImplFindSibling();
    SAL_DLLPRIVATE void      ImplRestoreSplitter();
    SAL_DLLPRIVATE void      ImplInitHorVer(bool bNew);

                             Splitter (const Splitter &) = delete;
                             Splitter& operator= (const Splitter &) = delete;

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void      ImplInit( vcl::Window* pParent, WinBits nWinStyle );

public:
    explicit            Splitter( vcl::Window* pParent, WinBits nStyle = WB_VSCROLL );
    virtual             ~Splitter() override;
    virtual void        dispose() override;

    void                StartSplit();
    void                EndSplit();
    void                Split();

    virtual void        MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void        Tracking( const TrackingEvent& rTEvt ) override;

    virtual void        GetFocus() override;
    virtual void        LoseFocus() override;
    virtual void        KeyInput( const KeyEvent& rKEvt ) override;
    virtual void        Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rPaintRect ) override;
    virtual Size        GetOptimalSize() const override;

    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;

    void                StartDrag();

    void                SetDragRectPixel( const tools::Rectangle& rDragRect,
                                          vcl::Window* pRefWin = nullptr );

    void                SetSplitPosPixel( long nPos );
    long                GetSplitPosPixel() const { return mnSplitPos; }

    bool                IsHorizontal() const { return mbHorzSplit; }
    void                SetHorizontal(bool bNew);

    // set the stepsize of the splitter for cursor movement
    // the default is 10% of the reference window's width/height
    void                SetKeyboardStepSize( long nStepSize );

    void                SetStartSplitHdl( const Link<Splitter*,void>& rLink ) { maStartSplitHdl = rLink; }
    void                SetSplitHdl( const Link<Splitter*,void>& rLink ) { maSplitHdl = rLink; }
    void                SetEndSplitHdl( const Link<Splitter*,void>& rLink ) { maEndSplitHdl = rLink; }
};

#endif // INCLUDED_VCL_SPLIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
