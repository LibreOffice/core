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

#ifndef INCLUDED_VCL_INC_WIN_SALFRAME_H
#define INCLUDED_VCL_INC_WIN_SALFRAME_H

#include <vcl/sysdata.hxx>
#include <salframe.hxx>
#include <svsys.h>

class WinSalGraphics;

// - WinSalFrame -

class WinSalFrame : public SalFrame
{
public:
    HWND                    mhWnd;                  // Window handle
    HCURSOR                 mhCursor;               // cursor handle
    HIMC                    mhDefIMEContext;        // default IME-Context
    WinSalGraphics*         mpGraphics;             // current frame graphics
    WinSalGraphics*         mpGraphics2;            // current frame graphics for other threads
    WinSalFrame*            mpNextFrame;            // pointer to next frame
    HMENU                   mSelectedhMenu;         // the menu where highlighting is currently going on
    HMENU                   mLastActivatedhMenu;    // the menu that was most recently opened
    SystemEnvData           maSysData;              // system data
    SalFrameState           maState;                // frame state
    int                     mnShowState;            // show state
    long                    mnWidth;                // client width in pixeln
    long                    mnHeight;               // client height in pixeln
    int                     mnMinWidth;             // min. client width in pixeln
    int                     mnMinHeight;            // min. client height in pixeln
    int                     mnMaxWidth;             // max. client width in pixeln
    int                     mnMaxHeight;            // max. client height in pixeln
    RECT                    maFullScreenRect;       // fullscreen rect
    int                     mnFullScreenShowState;  // fullscreen restore show state
    UINT                    mnInputLang;            // current Input Language
    UINT                    mnInputCodePage;        // current Input CodePage
    sal_uIntPtr             mnStyle;                // style
    bool                    mbGraphics;             // is Graphics used
    bool                    mbCaption;              // has window a caption
    bool                    mbBorder;               // has window a border
    bool                    mbFixBorder;            // has window a fixed border
    bool                    mbSizeBorder;           // has window a sizeable border
    bool                    mbNoIcon;               // is an window without an icon
    bool                    mbFloatWin;             // is a FloatingWindow
    bool                    mbFullScreen;           // TRUE: in full screen mode
    bool                    mbPresentation;         // TRUE: Presentation Mode running
    bool                    mbInShow;               // innerhalb eines Show-Aufrufs
    bool                    mbRestoreMaximize;      // Restore-Maximize
    bool                    mbInMoveMsg;            // Move-Message wird verarbeitet
    bool                    mbInSizeMsg;            // Size-Message wird verarbeitet
    bool                    mbFullScreenToolWin;    // WS_EX_TOOLWINDOW reset in FullScreenMode
    bool                    mbDefPos;               // default-position
    bool                    mbOverwriteState;       // TRUE: WindowState darf umgesetzt werden
    bool                    mbIME;                  // TRUE: We are in IME Mode
    bool                    mbHandleIME;            // TRUE: Wir handeln die IME-Messages
    bool                    mbSpezIME;              // TRUE: Spez IME
    bool                    mbAtCursorIME;          // TRUE: Wir behandeln nur einige IME-Messages
    bool                    mbCandidateMode;        // TRUE: Wir befinden uns im Candidate-Modus
    static bool             mbInReparent;           // TRUE: ignore focus lost and gain due to reparenting

    RGNDATA*                mpClipRgnData;
    RECT*                   mpNextClipRect;
    bool                    mbFirstClipRect;
    sal_Int32               mnDisplay;              // Display used for Fullscreen, 0 is primary monitor
    bool                    mbPropertiesStored;     // has values stored in the window property store

    void updateScreenNumber();
public:
    WinSalFrame();
    virtual ~WinSalFrame();

    virtual SalGraphics*        AcquireGraphics() SAL_OVERRIDE;
    virtual void                ReleaseGraphics( SalGraphics* pGraphics ) SAL_OVERRIDE;
    virtual bool                PostEvent( void* pData ) SAL_OVERRIDE;
    virtual void                SetTitle( const OUString& rTitle ) SAL_OVERRIDE;
    virtual void                SetIcon( sal_uInt16 nIcon ) SAL_OVERRIDE;
    virtual void                SetMenu( SalMenu* pSalMenu ) SAL_OVERRIDE;
    virtual void                DrawMenuBar() SAL_OVERRIDE;
    virtual void                SetExtendedFrameStyle( SalExtStyle nExtStyle ) SAL_OVERRIDE;
    virtual void                Show( bool bVisible, bool bNoActivate = FALSE ) SAL_OVERRIDE;
    virtual void                SetMinClientSize( long nWidth, long nHeight ) SAL_OVERRIDE;
    virtual void                SetMaxClientSize( long nWidth, long nHeight ) SAL_OVERRIDE;
    virtual void                SetPosSize( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags ) SAL_OVERRIDE;
    virtual void                GetClientSize( long& rWidth, long& rHeight ) SAL_OVERRIDE;
    virtual void                GetWorkArea( Rectangle& rRect ) SAL_OVERRIDE;
    virtual SalFrame*           GetParent() const SAL_OVERRIDE;
    virtual void                SetWindowState( const SalFrameState* pState ) SAL_OVERRIDE;
    virtual bool                GetWindowState( SalFrameState* pState ) SAL_OVERRIDE;
    virtual void                ShowFullScreen( bool bFullScreen, sal_Int32 nDisplay ) SAL_OVERRIDE;
    virtual void                StartPresentation( bool bStart ) SAL_OVERRIDE;
    virtual void                SetAlwaysOnTop( bool bOnTop ) SAL_OVERRIDE;
    virtual void                ToTop( sal_uInt16 nFlags ) SAL_OVERRIDE;
    virtual void                SetPointer( PointerStyle ePointerStyle ) SAL_OVERRIDE;
    virtual void                CaptureMouse( bool bMouse ) SAL_OVERRIDE;
    virtual void                SetPointerPos( long nX, long nY ) SAL_OVERRIDE;
    using SalFrame::Flush;
    virtual void                Flush() SAL_OVERRIDE;
    virtual void                SetInputContext( SalInputContext* pContext ) SAL_OVERRIDE;
    virtual void                EndExtTextInput( EndExtTextInputFlags nFlags ) SAL_OVERRIDE;
    virtual OUString            GetKeyName( sal_uInt16 nKeyCode ) SAL_OVERRIDE;
    virtual bool                MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, vcl::KeyCode& rKeyCode ) SAL_OVERRIDE;
    virtual LanguageType        GetInputLanguage() SAL_OVERRIDE;
    virtual void                UpdateSettings( AllSettings& rSettings ) SAL_OVERRIDE;
    virtual void                Beep() SAL_OVERRIDE;
    virtual const SystemEnvData*    GetSystemData() const SAL_OVERRIDE;
    virtual SalPointerState     GetPointerState() SAL_OVERRIDE;
    virtual KeyIndicatorState   GetIndicatorState() SAL_OVERRIDE;
    virtual void                SimulateKeyPress( sal_uInt16 nKeyCode ) SAL_OVERRIDE;
    virtual void                SetParent( SalFrame* pNewParent ) SAL_OVERRIDE;
    virtual bool                SetPluginParent( SystemParentData* pNewParent ) SAL_OVERRIDE;
    virtual void                SetScreenNumber( unsigned int ) SAL_OVERRIDE;
    virtual void                SetApplicationID( const OUString &rApplicationID ) SAL_OVERRIDE;
    virtual void                ResetClipRegion() SAL_OVERRIDE;
    virtual void                BeginSetClipRegion( sal_uIntPtr nRects ) SAL_OVERRIDE;
    virtual void                UnionClipRegion( long nX, long nY, long nWidth, long nHeight ) SAL_OVERRIDE;
    virtual void                EndSetClipRegion() SAL_OVERRIDE;
};

void ImplSalGetWorkArea( HWND hWnd, RECT *pRect, const RECT *pParentRect );

// get foreign key names
namespace vcl_sal {
    OUString getKeysReplacementName(
        OUString pLang,
        LONG nSymbol );
}

#endif // INCLUDED_VCL_INC_WIN_SALFRAME_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
