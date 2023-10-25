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

#include <sal/config.h>

#include <string_view>

#include <vcl/sysdata.hxx>
#include <vcl/windowstate.hxx>
#include <salframe.hxx>
#include <svsys.h>

class WinSalGraphics;

class WinSalFrame final: public SalFrame
{
    vcl::WindowState m_eState;

public:
    HWND                    mhWnd;                  // Window handle
    HCURSOR                 mhCursor;               // cursor handle
    HIMC                    mhDefIMEContext;        // default IME-Context
    WinSalGraphics*         mpLocalGraphics;        // current main thread frame graphics
    WinSalGraphics*         mpThreadGraphics;       // current frame graphics for other threads (DCX_CACHE)
    WinSalFrame*            mpNextFrame;            // pointer to next frame
    HMENU                   mSelectedhMenu;         // the menu where highlighting is currently going on
    HMENU                   mLastActivatedhMenu;    // the menu that was most recently opened
    SystemEnvData           maSysData;              // system data
    int                     mnShowState;            // show state
    int                     mnMinWidth;             // min. client width in pixeln
    int                     mnMinHeight;            // min. client height in pixeln
    int                     mnMaxWidth;             // max. client width in pixeln
    int                     mnMaxHeight;            // max. client height in pixeln
    RECT                    maFullScreenRect;       // fullscreen rect
    int                     mnFullScreenShowState;  // fullscreen restore show state
    bool                    mbFullScreenCaption;    // WS_CAPTION reset in full screen mode.
    UINT                    mnInputLang;            // current Input Language
    UINT                    mnInputCodePage;        // current Input CodePage
    SalFrameStyleFlags      mnStyle;                // style
    bool                    mbGraphics;             // is Graphics used
    bool                    mbCaption;              // has window a caption
    bool                    mbBorder;               // has window a border
    bool                    mbFixBorder;            // has window a fixed border
    bool                    mbSizeBorder;           // has window a sizeable border
    bool                    mbNoIcon;               // is a window without an icon
    bool                    mbFloatWin;             // is a FloatingWindow
    bool                    mbPresentation;         // TRUE: Presentation Mode running
    bool                    mbInShow;               // inside a show call
    bool                    mbRestoreMaximize;      // Restore-Maximize
    bool                    mbInMoveMsg;            // Move-Message is being processed
    bool                    mbInSizeMsg;            // Size-Message is being processed
    bool                    mbFullScreenToolWin;    // WS_EX_TOOLWINDOW reset in FullScreenMode
    bool                    mbDefPos;               // default-position
    bool                    mbOverwriteState;       // TRUE: possible to change WindowState
    bool                    mbIME;                  // TRUE: We are in IME Mode
    bool                    mbHandleIME;            // TRUE: We are handling the IME-Messages
    bool                    mbSpezIME;              // TRUE: special IME
    bool                    mbAtCursorIME;          // TRUE: We are only handling some IME-Messages
    bool                    mbCandidateMode;        // TRUE: We are in Candidate-Mode
    static bool             mbInReparent;           // TRUE: ignore focus lost and gain due to reparenting

    RGNDATA*                mpClipRgnData;
    RECT*                   mpNextClipRect;
    bool                    mbFirstClipRect;
    sal_Int32               mnDisplay;              // Display used for Fullscreen, 0 is primary monitor
    bool                    mbPropertiesStored;     // has values stored in the window property store

    void updateScreenNumber();

private:
    void ImplSetParentFrame( HWND hNewParentWnd, bool bAsChild );
    bool InitFrameGraphicsDC( WinSalGraphics *pGraphics, HDC hDC, HWND hWnd );
    bool ReleaseFrameGraphicsDC( WinSalGraphics* pGraphics );

public:
    WinSalFrame();
    virtual ~WinSalFrame() override;

    virtual SalGraphics*        AcquireGraphics() override;
    virtual void                ReleaseGraphics( SalGraphics* pGraphics ) override;
    virtual bool                PostEvent(std::unique_ptr<ImplSVEvent> pData) override;
    virtual void                SetTitle( const OUString& rTitle ) override;
    virtual void                SetIcon( sal_uInt16 nIcon ) override;
    virtual void                SetMenu( SalMenu* pSalMenu ) override;
    virtual void                SetExtendedFrameStyle( SalExtStyle nExtStyle ) override;
    virtual void                Show( bool bVisible, bool bNoActivate = false ) override;
    virtual void                SetMinClientSize( tools::Long nWidth, tools::Long nHeight ) override;
    virtual void                SetMaxClientSize( tools::Long nWidth, tools::Long nHeight ) override;
    virtual void                SetPosSize( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, sal_uInt16 nFlags ) override;
    virtual void                GetClientSize( tools::Long& rWidth, tools::Long& rHeight ) override;
    virtual void                GetWorkArea( AbsoluteScreenPixelRectangle& rRect ) override;
    virtual SalFrame*           GetParent() const override;
    virtual void SetWindowState(const vcl::WindowData*) override;
    virtual bool GetWindowState(vcl::WindowData*) override;
    virtual void                ShowFullScreen( bool bFullScreen, sal_Int32 nDisplay ) override;
    virtual void                StartPresentation( bool bStart ) override;
    virtual void                SetAlwaysOnTop( bool bOnTop ) override;
    virtual void                ToTop( SalFrameToTop nFlags ) override;
    virtual void                SetPointer( PointerStyle ePointerStyle ) override;
    virtual void                CaptureMouse( bool bMouse ) override;
    virtual void                SetPointerPos( tools::Long nX, tools::Long nY ) override;
    using SalFrame::Flush;
    virtual void                Flush() override;
    virtual void                SetInputContext( SalInputContext* pContext ) override;
    virtual void                EndExtTextInput( EndExtTextInputFlags nFlags ) override;
    virtual OUString            GetKeyName( sal_uInt16 nKeyCode ) override;
    virtual bool                MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, vcl::KeyCode& rKeyCode ) override;
    virtual LanguageType        GetInputLanguage() override;
    virtual void                UpdateSettings( AllSettings& rSettings ) override;
    virtual void                Beep() override;
    virtual void                FlashWindow() const override;
    virtual const SystemEnvData*    GetSystemData() const override;
    virtual SalPointerState     GetPointerState() override;
    virtual KeyIndicatorState   GetIndicatorState() override;
    virtual void                SimulateKeyPress( sal_uInt16 nKeyCode ) override;
    virtual void                SetParent( SalFrame* pNewParent ) override;
    virtual void                SetPluginParent( SystemParentData* pNewParent ) override;
    virtual void                SetScreenNumber( unsigned int ) override;
    virtual void                SetApplicationID( const OUString &rApplicationID ) override;
    virtual void                ResetClipRegion() override;
    virtual void                BeginSetClipRegion( sal_uInt32 nRects ) override;
    virtual void                UnionClipRegion( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight ) override;
    virtual void                EndSetClipRegion() override;
    virtual void                UpdateDarkMode() override;
    virtual bool                GetUseDarkMode() const override;
    virtual bool                GetUseReducedAnimation() const override;

    constexpr vcl::WindowState state() const { return m_eState; }
    void UpdateFrameState();
    constexpr bool isFullScreen() const { return bool(m_eState & vcl::WindowState::FullScreen); }
};

void ImplSalGetWorkArea( HWND hWnd, RECT *pRect, const RECT *pParentRect );

bool UseDarkMode();
bool OSSupportsDarkMode();

// get foreign key names
namespace vcl_sal {
    OUString getKeysReplacementName(
        std::u16string_view pLang,
        LONG nSymbol );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
