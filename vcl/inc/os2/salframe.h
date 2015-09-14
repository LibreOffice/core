/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SV_SALFRAME_H
#define _SV_SALFRAME_H

#include <vcl/sysdata.hxx>
#include <salframe.hxx>

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
    sal_Bool                    mbGraphics;             // is Graphics used
    sal_Bool                    mbAllwayOnTop;          // Allways on top modus
    sal_Bool                    mbVisible;              // Visible Show/Hide-Status
    sal_Bool                    mbMinHide;              // hide called from OS2
    sal_Bool                    mbHandleIME;            // TRUE: Wir handeln die IME-Messages
    sal_Bool                    mbConversionMode;       // TRUE: Wir befinden uns im Conversion-Modus
    sal_Bool                    mbCandidateMode;        // TRUE: Wir befinden uns im Candidate-Modus
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

    int                     mnMinWidth;             // min. client width in pixeln
    int                     mnMinHeight;            // min. client height in pixeln
    int                     mnMaxWidth;             // max. client width in pixeln
    int                     mnMaxHeight;            // max. client height in pixeln
    static ULONG            mnInputLang;            // current Input Language
    KHAND                   mnKeyboardHandle;       // current unicode keyboard
    static sal_Bool             mbInReparent;           // TRUE: ignore focus lost and gain due to reparenting

private:
    Os2SalFrame*            mpParentFrame;          // parent frame pointer

public:
    Os2SalFrame();
    virtual ~Os2SalFrame();

    virtual SalGraphics*        GetGraphics();
    virtual void                ReleaseGraphics( SalGraphics* pGraphics );
    virtual sal_Bool                PostEvent( void* pData );
    virtual void                SetTitle( const XubString& rTitle );
    virtual void                SetIcon( USHORT nIcon );
    virtual void                                SetMenu( SalMenu* pSalMenu );
    virtual void                                DrawMenuBar();
    virtual void                SetExtendedFrameStyle( SalExtStyle nExtStyle );
    virtual void                Show( sal_Bool bVisible, sal_Bool bNoActivate = FALSE );
    virtual void                Enable( sal_Bool bEnable );
    virtual void                SetMinClientSize( long nWidth, long nHeight );
    virtual void                SetMaxClientSize( long nWidth, long nHeight );
    virtual void                SetPosSize( long nX, long nY, long nWidth, long nHeight, USHORT nFlags );
    virtual void                GetClientSize( long& rWidth, long& rHeight );
    virtual void                GetWorkArea( RECTL& rRect );
    virtual void                GetWorkArea( Rectangle& rRect );
    virtual SalFrame*           GetParent() const;
    virtual void                SetWindowState( const SalFrameState* pState );
    virtual sal_Bool                GetWindowState( SalFrameState* pState );
    virtual void                ShowFullScreen( sal_Bool bFullScreen, sal_Int32 nDisplay );
    virtual void                StartPresentation( sal_Bool bStart );
    virtual void                SetAlwaysOnTop( sal_Bool bOnTop );
    virtual void                ToTop( USHORT nFlags );
    virtual void                SetPointer( PointerStyle ePointerStyle );
    virtual void                CaptureMouse( sal_Bool bMouse );
    virtual void                SetPointerPos( long nX, long nY );
    virtual void                Flush();
    virtual void                Sync();
    virtual void                SetInputContext( SalInputContext* pContext );
    virtual void                EndExtTextInput( USHORT nFlags );
    virtual String              GetKeyName( USHORT nKeyCode );
    virtual String              GetSymbolKeyName( const XubString& rFontName, USHORT nKeyCode );
    virtual sal_Bool                MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, KeyCode& rKeyCode );
    virtual LanguageType        GetInputLanguage();
    virtual SalBitmap*          SnapShot();
    virtual void                UpdateSettings( AllSettings& rSettings );
    virtual void                Beep( SoundType eSoundType );
    virtual const SystemEnvData*    GetSystemData() const;
    virtual SalPointerState     GetPointerState();
    virtual void                SetParent( SalFrame* pNewParent );
    virtual bool                SetPluginParent( SystemParentData* pNewParent );
    virtual void                SetBackgroundBitmap( SalBitmap* );
    virtual void                SetScreenNumber( unsigned int );
    virtual void                ResetClipRegion();
    virtual void                BeginSetClipRegion( ULONG nRects );
    virtual void                UnionClipRegion( long nX, long nY, long nWidth, long nHeight );
    virtual void                EndSetClipRegion();

};

#endif // _SV_SALFRAME_H
