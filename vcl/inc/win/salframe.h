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

#ifndef _SV_SALFRAME_H
#define _SV_SALFRAME_H

#include <vcl/sysdata.hxx>
#include <salframe.hxx>

class WinSalGraphics;

// ----------------
// - WinSalFrame -
// ----------------

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
    sal_uIntPtr                   mnStyle;                // style
    sal_Bool                    mbGraphics;             // is Graphics used
    sal_Bool                    mbCaption;              // has window a caption
    sal_Bool                    mbBorder;               // has window a border
    sal_Bool                    mbFixBorder;            // has window a fixed border
    sal_Bool                    mbSizeBorder;           // has window a sizeable border
    sal_Bool                    mbNoIcon;               // is an window without an icon
    sal_Bool                    mbFloatWin;             // is a FloatingWindow
    sal_Bool                    mbFullScreen;           // TRUE: in full screen mode
    sal_Bool                    mbPresentation;         // TRUE: Presentation Mode running
    sal_Bool                    mbInShow;               // innerhalb eines Show-Aufrufs
    sal_Bool                    mbRestoreMaximize;      // Restore-Maximize
    sal_Bool                    mbInMoveMsg;            // Move-Message wird verarbeitet
    sal_Bool                    mbInSizeMsg;            // Size-Message wird verarbeitet
    sal_Bool                    mbFullScreenToolWin;    // WS_EX_TOOLWINDOW reset in FullScreenMode
    sal_Bool                    mbDefPos;               // default-position
    sal_Bool                    mbOverwriteState;       // TRUE: WindowState darf umgesetzt werden
    sal_Bool                    mbIME;                  // TRUE: We are in IME Mode
    sal_Bool                    mbHandleIME;            // TRUE: Wir handeln die IME-Messages
    sal_Bool                    mbSpezIME;              // TRUE: Spez IME
    sal_Bool                    mbAtCursorIME;          // TRUE: Wir behandeln nur einige IME-Messages
    sal_Bool                    mbCandidateMode;        // TRUE: Wir befinden uns im Candidate-Modus
    static sal_Bool             mbInReparent;           // TRUE: ignore focus lost and gain due to reparenting

    RGNDATA*                mpClipRgnData;
    RECT*                   mpNextClipRect;
    sal_Bool                    mbFirstClipRect;
    sal_Int32               mnDisplay;              // Display used for Fullscreen, 0 is primary monitor

    void updateScreenNumber();
public:
    WinSalFrame();
    virtual ~WinSalFrame();

    virtual SalGraphics*        GetGraphics();
    virtual void                ReleaseGraphics( SalGraphics* pGraphics );
    virtual sal_Bool                PostEvent( void* pData );
    virtual void                SetTitle( const rtl::OUString& rTitle );
    virtual void                SetIcon( sal_uInt16 nIcon );
    virtual void                                SetMenu( SalMenu* pSalMenu );
    virtual void                                DrawMenuBar();
    virtual void                SetExtendedFrameStyle( SalExtStyle nExtStyle );
    virtual void                Show( sal_Bool bVisible, sal_Bool bNoActivate = FALSE );
    virtual void                Enable( sal_Bool bEnable );
    virtual void                SetMinClientSize( long nWidth, long nHeight );
    virtual void                SetMaxClientSize( long nWidth, long nHeight );
    virtual void                SetPosSize( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags );
    virtual void                GetClientSize( long& rWidth, long& rHeight );
    virtual void                GetWorkArea( Rectangle& rRect );
    virtual SalFrame*           GetParent() const;
    virtual void                SetWindowState( const SalFrameState* pState );
    virtual sal_Bool                GetWindowState( SalFrameState* pState );
    virtual void                ShowFullScreen( sal_Bool bFullScreen, sal_Int32 nDisplay );
    virtual void                StartPresentation( sal_Bool bStart );
    virtual void                SetAlwaysOnTop( sal_Bool bOnTop );
    virtual void                ToTop( sal_uInt16 nFlags );
    virtual void                SetPointer( PointerStyle ePointerStyle );
    virtual void                CaptureMouse( sal_Bool bMouse );
    virtual void                SetPointerPos( long nX, long nY );
    using SalFrame::Flush;
    virtual void                Flush();
    virtual void                Sync();
    virtual void                SetInputContext( SalInputContext* pContext );
    virtual void                EndExtTextInput( sal_uInt16 nFlags );
    virtual rtl::OUString              GetKeyName( sal_uInt16 nKeyCode );
    virtual sal_Bool                MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, KeyCode& rKeyCode );
    virtual LanguageType        GetInputLanguage();
    virtual void                UpdateSettings( AllSettings& rSettings );
    virtual const SystemEnvData*    GetSystemData() const;
    virtual SalPointerState     GetPointerState();
    virtual SalIndicatorState   GetIndicatorState();
    virtual void                SimulateKeyPress( sal_uInt16 nKeyCode );
    virtual void                SetParent( SalFrame* pNewParent );
    virtual bool                SetPluginParent( SystemParentData* pNewParent );
    virtual void                SetBackgroundBitmap( SalBitmap* );
    virtual void                SetScreenNumber( unsigned int );
    virtual void                SetApplicationID( const rtl::OUString &rApplicationID );
    virtual void                ResetClipRegion();
    virtual void                BeginSetClipRegion( sal_uIntPtr nRects );
    virtual void                UnionClipRegion( long nX, long nY, long nWidth, long nHeight );
    virtual void                EndSetClipRegion();
};

void ImplSalGetWorkArea( HWND hWnd, RECT *pRect, const RECT *pParentRect );

#endif // _SV_SALFRAME_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
