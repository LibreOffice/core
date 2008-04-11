/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salframe.h,v $
 * $Revision: 1.6 $
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

#include <vcl/sv.h>
#include <vcl/sysdata.hxx>
#include <vcl/salframe.hxx>


#ifndef PM_BIDI_INCLUDED
#include <pmbidi.h>
#endif

#ifndef __UCONV_H__
#include <uconv.h>
#endif

#ifndef __UNIDEF_H__
#include <unidef.h>
#endif

#ifndef __UNIKBD_H__
#include <unikbd.h>
#endif

//class SalFrame;
class Os2SalGraphics;

// ----------------
// - SalFrameData -
// ----------------

//class SalFrameData
class Os2SalFrame : public SalFrame
{
public:
    HWND                    mhWndFrame;             // HWND-Frame
    HWND                    mhWndClient;            // HWND-Client
    HAB                     mhAB;                   // HAB
    HPOINTER                mhPointer;              // Current MousePointer
    void*                   mpInst;                 // VCL-Instance
    //SALFRAMEPROC          mpProc;                 // VCL-Proc
    Os2SalGraphics*         mpGraphics;             // current frame graphics
    Os2SalFrame*                mpNextFrame;            // pointer to next frame
    SystemEnvData           maSysData;              // system data
    SalFrameState           maState;                // frame state
    int                     mnShowState;            // show state
    ULONG                   mnStyle;                // SalFrameStyle
    long                    mnWidth;                // Window-Witdth
    long                    mnHeight;               // Window-Height
    SWP                     maFullScreenRect;       // WindowRect befor FullScreenMode
    BOOL                    mbGraphics;             // is Graphics used
    BOOL                    mbAllwayOnTop;          // Allways on top modus
    BOOL                    mbVisible;              // Visible Show/Hide-Status
    BOOL                    mbMinHide;              // hide called from OS2
    BOOL                    mbHandleIME;            // TRUE: Wir handeln die IME-Messages
    BOOL                    mbConversionMode;       // TRUE: Wir befinden uns im Conversion-Modus
    BOOL                    mbCandidateMode;        // TRUE: Wir befinden uns im Candidate-Modus
    BOOL                    mbCaption;              // has window a caption
    BOOL                    mbBorder;               // has window a border
    BOOL                    mbFixBorder;            // has window a fixed border
    BOOL                    mbSizeBorder;           // has window a sizeable border
    BOOL                    mbNoIcon;               // is an window without an icon
    BOOL                    mbFloatWin;             // is a FloatingWindow
    BOOL                    mbFullScreen;           // TRUE: in full screen mode
    BOOL                    mbPresentation;         // TRUE: Presentation Mode running
    BOOL                    mbInShow;               // innerhalb eines Show-Aufrufs
    BOOL                    mbRestoreMaximize;      // Restore-Maximize
    BOOL                    mbInMoveMsg;            // Move-Message wird verarbeitet
    BOOL                    mbInSizeMsg;            // Size-Message wird verarbeitet
    BOOL                    mbFullScreenToolWin;    // WS_EX_TOOLWINDOW reset in FullScreenMode
    BOOL                    mbDefPos;               // default-position
    BOOL                    mbOverwriteState;       // TRUE: WindowState darf umgesetzt werden

    int                     mnMinWidth;             // min. client width in pixeln
    int                     mnMinHeight;            // min. client height in pixeln
    int                     mnMaxWidth;             // max. client width in pixeln
    int                     mnMaxHeight;            // max. client height in pixeln
    static ULONG            mnInputLang;            // current Input Language
    KHAND                   mnKeyboardHandle;       // current unicode keyboard
    static BOOL             mbInReparent;           // TRUE: ignore focus lost and gain due to reparenting

private:
    Os2SalFrame*            mpParentFrame;          // parent frame pointer

public:
    Os2SalFrame();
    virtual ~Os2SalFrame();

    virtual SalGraphics*        GetGraphics();
    virtual void                ReleaseGraphics( SalGraphics* pGraphics );
    virtual BOOL                PostEvent( void* pData );
    virtual void                SetTitle( const XubString& rTitle );
    virtual void                SetIcon( USHORT nIcon );
    virtual void                                SetMenu( SalMenu* pSalMenu );
    virtual void                                DrawMenuBar();
    virtual void                SetExtendedFrameStyle( SalExtStyle nExtStyle );
    virtual void                Show( BOOL bVisible, BOOL bNoActivate = FALSE );
    virtual void                Enable( BOOL bEnable );
    virtual void                SetMinClientSize( long nWidth, long nHeight );
    virtual void                SetMaxClientSize( long nWidth, long nHeight );
    virtual void                SetPosSize( long nX, long nY, long nWidth, long nHeight, USHORT nFlags );
    virtual void                GetClientSize( long& rWidth, long& rHeight );
    virtual void                GetWorkArea( RECTL& rRect );
    virtual void                GetWorkArea( Rectangle& rRect );
    virtual SalFrame*           GetParent() const;
    virtual void                SetWindowState( const SalFrameState* pState );
    virtual BOOL                GetWindowState( SalFrameState* pState );
    virtual void                ShowFullScreen( BOOL bFullScreen, sal_Int32 nDisplay );
    virtual void                StartPresentation( BOOL bStart );
    virtual void                SetAlwaysOnTop( BOOL bOnTop );
    virtual void                ToTop( USHORT nFlags );
    virtual void                SetPointer( PointerStyle ePointerStyle );
    virtual void                CaptureMouse( BOOL bMouse );
    virtual void                SetPointerPos( long nX, long nY );
    virtual void                Flush();
    virtual void                Sync();
    virtual void                SetInputContext( SalInputContext* pContext );
    virtual void                EndExtTextInput( USHORT nFlags );
    virtual String              GetKeyName( USHORT nKeyCode );
    virtual String              GetSymbolKeyName( const XubString& rFontName, USHORT nKeyCode );
    virtual BOOL                MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, KeyCode& rKeyCode );
    virtual LanguageType        GetInputLanguage();
    virtual SalBitmap*          SnapShot();
    virtual void                UpdateSettings( AllSettings& rSettings );
    virtual void                Beep( SoundType eSoundType );
    virtual const SystemEnvData*    GetSystemData() const;
    virtual SalPointerState     GetPointerState();
    virtual void                SetParent( SalFrame* pNewParent );
    virtual bool                SetPluginParent( SystemParentData* pNewParent );
    virtual void                SetBackgroundBitmap( SalBitmap* );
    virtual void                ResetClipRegion();
    virtual void                BeginSetClipRegion( ULONG nRects );
    virtual void                UnionClipRegion( long nX, long nY, long nWidth, long nHeight );
    virtual void                EndSetClipRegion();

};

#endif // _SV_SALFRAME_H
