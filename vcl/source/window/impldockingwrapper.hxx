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

#include <vcl/dockwin.hxx>
#include <memory>
#include <vector>

/** ImplDockingWindowWrapper
 *
 *  ImplDockingWindowWrapper obsoletes the DockingWindow class.
 *  It is better because it can make a "normal window" dockable.
 *  All DockingWindows should be converted the new class.
 */

class ImplDockingWindowWrapper final
{
    friend class ::vcl::Window;
    friend class DockingManager;
    friend class DockingWindow;

private:

    // the original 'Docking'window
    VclPtr<vcl::Window>    mpDockingWindow;

    // the original DockingWindow members
    VclPtr<FloatingWindow> mpFloatWin;
    VclPtr<vcl::Window>    mpOldBorderWin;
    VclPtr<vcl::Window>    mpParent;
    Link<FloatingWindow*,void> maPopupModeEndHdl;
    Point           maFloatPos;
    Point           maDockPos;
    Point           maMouseOff;
    Size            maMinOutSize;
    Size            maMaxOutSize;
    tools::Rectangle       maDragArea;
    tools::Long            mnTrackX;
    tools::Long            mnTrackY;
    tools::Long            mnTrackWidth;
    tools::Long            mnTrackHeight;
    sal_Int32       mnDockLeft;
    sal_Int32       mnDockTop;
    sal_Int32       mnDockRight;
    sal_Int32       mnDockBottom;
    WinBits         mnFloatBits;
    bool            mbDockCanceled:1,
                    mbDocking:1,
                    mbLastFloatMode:1,
                    mbDockBtn:1,
                    mbHideBtn:1,
                    mbStartDockingEnabled:1,
                    mbLocked:1;

                    DECL_LINK( PopupModeEnd, FloatingWindow*, void );
    void            ImplEnableStartDocking()  { mbStartDockingEnabled = true; }
    bool            ImplStartDockingEnabled() const { return mbStartDockingEnabled; }
    void            ImplPreparePopupMode();

public:
    ImplDockingWindowWrapper( const vcl::Window *pWindow );
    ~ImplDockingWindowWrapper();

    vcl::Window*    GetWindow()     { return mpDockingWindow; }
    void            ImplStartDocking( const Point& rPos );

    // those methods actually call the corresponding handlers
    void            StartDocking( const Point& rPos, tools::Rectangle const & rRect );
    bool            Docking( const Point& rPos, tools::Rectangle& rRect );
    void            EndDocking( const tools::Rectangle& rRect, bool bFloatMode );
    bool            PrepareToggleFloatingMode();
    void            ToggleFloatingMode();

    void            SetDragArea( const tools::Rectangle& rRect );
    const tools::Rectangle& GetDragArea() const { return maDragArea;}

    void            Lock();
    void            Unlock();
    bool            IsLocked() const { return mbLocked;}

    void            StartPopupMode( const tools::Rectangle& rRect, FloatWinPopupFlags nPopupModeFlags );
    void            StartPopupMode( ToolBox* pParentToolBox, FloatWinPopupFlags nPopupModeFlags );
    bool            IsInPopupMode() const;

    void            SetPopupModeEndHdl( const Link<FloatingWindow*,void>& rLink ) { maPopupModeEndHdl = rLink; }

    void            TitleButtonClick( TitleButton nButton );
    void            Resizing( Size& rSize );
    void            Tracking( const TrackingEvent& rTEvt );

    void            ShowMenuTitleButton( bool bVisible );

    void            SetMinOutputSizePixel( const Size& rSize );

    void            SetMaxOutputSizePixel( const Size& rSize );

    bool            IsDocking() const { return mbDocking; }
    bool            IsDockingCanceled() const { return mbDockCanceled; }

    void            SetFloatingMode( bool bFloatMode );
    bool            IsFloatingMode() const;
    SystemWindow*   GetFloatingWindow() const;

    void            SetFloatStyle( WinBits nWinStyle );
    WinBits         GetFloatStyle() const { return mnFloatBits;}

    void            setPosSizePixel( tools::Long nX, tools::Long nY,
                                     tools::Long nWidth, tools::Long nHeight,
                                     PosSizeFlags nFlags );
    Point           GetPosPixel() const;
    Size            GetSizePixel() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
