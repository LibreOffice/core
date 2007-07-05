/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salframe.h,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-05 08:12:19 $
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

#ifndef _SV_SALFRAME_H
#define _SV_SALFRAME_H

#include <salframe.hxx>

#include <salmenu.h>

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

#ifndef _SV_SYSDATA_HXX
#include <sysdata.hxx>
#endif

#include <aquavcltypes.h>
#include <vector>
#include <utility>
#include <stdexcept>

class AquaSalGraphics;
class AquaSalFrame;
class AquaSalTimer;
class AquaSalInstance;

typedef struct SalFrame::SalPointerState SalPointerState;

// ----------------
// - AquaSalFrame -
// ----------------

class AquaSalFrame : public SalFrame
{
public:
    CarbonWindowRef         mrWindow;               // Window handle
    AquaSalGraphics*        mpGraphics;             // current frame graphics
    AquaSalFrame*           mpParent;               // pointer to parent frame
    void*                   mpInst;                 // instance handle for callback
     SystemEnvData          maSysData;              // system data
    long                    mnWidth;                // client width in pixels
    long                    mnHeight;               // client height in pixels
    int                     mnMinWidth;             // min. client width in pixels
    int                     mnMinHeight;            // min. client height in pixels
    int                     mnMaxWidth;             // max. client width in pixels
    int                     mnMaxHeight;            // max. client height in pixels
    Rect                    maFullScreenRect;       // old window size when in FullScreen
    WindowAttributes        maFullScreenAttr;       // old window attributes when in FullScreen
    BOOL                    mbGraphics;             // is Graphics used?
    BOOL                    mbFullScreen;           // is Window in FullScreen?
    AquaSalInstance*        mpSalInstance;
    bool                    mbShown;
    bool                    mbInitShow;
    bool                    mbPositioned;
    bool                    mbSized;

    ULONG                   mnStyle;

    TSMDocumentID       maTsmDocumentId;
    SalExtTextInputEvent    maInputEvent;       // preedit text
public:
    /** Constructor

        Creates a system window and connects this frame with it.

        @throws std::runtime_error in case window creation fails
    */
    AquaSalFrame(SalFrame* pParent, ULONG salFrameStyle, AquaSalInstance* pSalInstance);

    virtual ~AquaSalFrame();

    virtual SalGraphics*        GetGraphics();
    virtual void                ReleaseGraphics( SalGraphics* pGraphics );
    virtual BOOL                PostEvent( void* pData );
    BOOL                        PostTimerEvent( AquaSalTimer *pTimer );
    virtual void                SetTitle( const XubString& rTitle );
    virtual void                SetIcon( USHORT nIcon );
    virtual void                SetMenu( SalMenu* pSalMenu );
    virtual void                DrawMenuBar();
    virtual void                Show( BOOL bVisible, BOOL bNoActivate = FALSE );
    virtual void                Enable( BOOL bEnable );
    virtual void                SetMinClientSize( long nWidth, long nHeight );
    virtual void                SetMaxClientSize( long nWidth, long nHeight );
    virtual void                SetPosSize( long nX, long nY, long nWidth, long nHeight, USHORT nFlags );
    virtual void                GetClientSize( long& rWidth, long& rHeight );
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
    virtual void                SetExtendedFrameStyle( SalExtStyle );
    virtual void                SetBackgroundBitmap( SalBitmap* );

    // shaped system windows
    // set clip region to none (-> rectangular windows, normal state)
    virtual void ResetClipRegion();
    // start setting the clipregion consisting of nRects rectangles
    virtual void BeginSetClipRegion( ULONG nRects );
    // add a rectangle to the clip region
    virtual void UnionClipRegion( long nX, long nY, long nWidth, long nHeight );
    // done setting up the clipregion
    virtual void EndSetClipRegion();

    virtual void SetClientSize( long nWidth, long nHeight );

    void UpdateFrameGeometry();

    // trigger painting of the window
    void SendPaintEvent();

    void ActivateTSM();
    void DeactivateTSM();

    static bool isAlive( const AquaSalFrame* pFrame )
    { return GetSalData()->maFrameCheck.find( pFrame ) != GetSalData()->maFrameCheck.end(); }

    static AquaSalFrame* GetCaptureFrame() { return s_pCaptureFrame; }

 private: // methods

    /** Create a new system window.
        The newly create window will be associated whith this frame.

        @param pParent
        the parent of the window may be NULL

        @param nSalFrameStyle
        the style of the new window

        @throws std::runtime_error in case window creation fails
     */
    void CreateNewSystemWindow(CarbonWindowRef pParent, ULONG nSalFrameStyle);

    BOOL ImplPostUserEvent( UInt32 eventKind, void *pData );

    /** Install a window event handler

        The window event handler and the corresponding Universal Procedure Pointer (UPP) pointer
        need to be save during destruction of the frame instance we have to unregister all installed
        event handlers and dispose the UPP.

        @param upp
        a universal procedure pointer

        @param nEvents
        number of events to register

        @param eventSpec
        the event specification

        @return the status of the registration see Carbon Event Manager reference for details
     */
    OSStatus InstallAndRegisterEventHandler(EventHandlerUPP upp, size_t nEvents, const EventTypeSpec* eventSpec);
    void DeinstallAndUnregisterAllEventHandler();

    /** do things on initial show (like centering on parent or on screen)
    */
    void initShow();

 private: // data

    typedef std::pair<EventHandlerUPP, EventHandlerRef> SysWindowEventHandlerData_t;
    typedef std::vector<SysWindowEventHandlerData_t> SysWindowEventHandlerDataContainer_t;
    SysWindowEventHandlerDataContainer_t mSysWindowEventHandlerDataContainer;

    // Menu associated with this SalFrame
    SalMenu *mpMenu;

    static SysWindowEventHandlerData_t     s_aOverlayEvtHandler;
    static AquaSalFrame*                   s_pCaptureFrame;
    static CarbonWindowRef                 s_rOverlay; // window handle for overlay (needed in CaptureMouse)

    // make AquaSalFrame non copyable
    AquaSalFrame( const AquaSalFrame& );
    AquaSalFrame& operator=(const AquaSalFrame&);
};

#endif // _SV_SALFRAME_H
