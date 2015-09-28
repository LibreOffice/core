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

#ifndef INCLUDED_VCL_INC_OSX_SALFRAME_H
#define INCLUDED_VCL_INC_OSX_SALFRAME_H

#include <premac.h>
#include <IOKit/pwr_mgt/IOPMLib.h>
#include <postmac.h>

#include "vcl/sysdata.hxx"

#include "osx/salmenu.h"
#include "osx/saldata.hxx"
#include "osx/osxvcltypes.h"

#include "salframe.hxx"

#include <vector>
#include <utility>
#include <stdexcept>

class AquaSalGraphics;
class AquaSalFrame;
class AquaSalTimer;
class AquaSalInstance;
class AquaSalMenu;
class AquaBlinker;

typedef struct SalFrame::SalPointerState SalPointerState;

// - AquaSalFrame -

class AquaSalFrame : public SalFrame
{
public:
    NSWindow*                       mpNSWindow;             // Cocoa window
    NSView*                         mpNSView;               // Cocoa view (actually a custom view)
    NSMenuItem*                     mpDockMenuEntry;        // entry in the dynamic dock menu
    NSRect                          maScreenRect;           // for mirroring purposes
    AquaSalGraphics*                mpGraphics;             // current frame graphics
    AquaSalFrame*                   mpParent;               // pointer to parent frame
     SystemEnvData                  maSysData;              // system data
    int                             mnMinWidth;             // min. client width in pixels
    int                             mnMinHeight;            // min. client height in pixels
    int                             mnMaxWidth;             // max. client width in pixels
    int                             mnMaxHeight;            // max. client height in pixels
    NSRect                          maFullScreenRect;       // old window size when in FullScreen
    bool                            mbGraphics:1;           // is Graphics used?
    bool                            mbFullScreen:1;         // is Window in FullScreen?
    bool                            mbShown:1;
    bool                            mbInitShow:1;
    bool                            mbPositioned:1;
    bool                            mbSized:1;
    bool                            mbPresentation:1;

    sal_uLong                           mnStyle;
    unsigned int                    mnStyleMask;            // our style mask from NSWindow creation

    sal_uInt64                      mnLastEventTime;
    unsigned int                    mnLastModifierFlags;
    AquaSalMenu*                    mpMenu;

    SalExtStyle                     mnExtStyle;             // currently document frames are marked this way

    PointerStyle                    mePointerStyle;         // currently active pointer style

    NSTrackingRectTag               mnTrackingRectTag;      // used to get enter/leave messages

    CGMutablePathRef                mrClippingPath;         // used for "shaping"
    std::vector< CGRect >           maClippingRects;

    std::list<AquaBlinker*>         maBlinkers;

    Rectangle                       maInvalidRect;

    InputContextFlags               mnICOptions;

    // To prevent display sleep during presentation
    IOPMAssertionID                 mnAssertionID;

public:
    /** Constructor

        Creates a system window and connects this frame with it.

        @throws std::runtime_error in case window creation fails
    */
    AquaSalFrame( SalFrame* pParent, sal_uLong salFrameStyle );

    virtual ~AquaSalFrame();

    virtual SalGraphics*        AcquireGraphics() SAL_OVERRIDE;
    virtual void                ReleaseGraphics( SalGraphics* pGraphics ) SAL_OVERRIDE;
    virtual bool                PostEvent(ImplSVEvent* pData) SAL_OVERRIDE;
    virtual void                SetTitle( const OUString& rTitle ) SAL_OVERRIDE;
    virtual void                SetIcon( sal_uInt16 nIcon ) SAL_OVERRIDE;
    virtual void                SetRepresentedURL( const OUString& ) SAL_OVERRIDE;
    virtual void                SetMenu( SalMenu* pSalMenu ) SAL_OVERRIDE;
    virtual void                DrawMenuBar() SAL_OVERRIDE;
    virtual void                Show( bool bVisible, bool bNoActivate = false ) SAL_OVERRIDE;
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
    virtual void                Flush( void ) SAL_OVERRIDE;
    virtual void                Flush( const Rectangle& ) SAL_OVERRIDE;
    virtual void                Sync() SAL_OVERRIDE;
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
    virtual void                SetExtendedFrameStyle( SalExtStyle ) SAL_OVERRIDE;
    virtual void                SetScreenNumber(unsigned int) SAL_OVERRIDE;
    virtual void                SetApplicationID( const OUString &rApplicationID ) SAL_OVERRIDE;

    // shaped system windows
    // set clip region to none (-> rectangular windows, normal state)
    virtual void ResetClipRegion() SAL_OVERRIDE;
    // start setting the clipregion consisting of nRects rectangles
    virtual void BeginSetClipRegion( sal_uLong nRects ) SAL_OVERRIDE;
    // add a rectangle to the clip region
    virtual void UnionClipRegion( long nX, long nY, long nWidth, long nHeight ) SAL_OVERRIDE;
    // done setting up the clipregion
    virtual void EndSetClipRegion() SAL_OVERRIDE;

    void UpdateFrameGeometry();

    // trigger painting of the window
    void SendPaintEvent( const Rectangle* pRect = NULL );

    static bool isAlive( const AquaSalFrame* pFrame )
    { return GetSalData()->maFrameCheck.find( pFrame ) != GetSalData()->maFrameCheck.end(); }

    static AquaSalFrame* GetCaptureFrame() { return s_pCaptureFrame; }

    NSWindow* getNSWindow() const { return mpNSWindow; }
    NSView* getNSView() const { return mpNSView; }
    unsigned int getStyleMask() const { return mnStyleMask; }

    void getResolution( sal_Int32& o_rDPIX, sal_Int32& o_rDPIY );

    // actually the following methods do the same thing: flipping y coordinates
    // but having two of them makes clearer what the coordinate system
    // is supposed to be before and after
    void VCLToCocoa( NSRect& io_rRect, bool bRelativeToScreen = true );
    void CocoaToVCL( NSRect& io_rRect, bool bRelativeToScreen = true );

    void VCLToCocoa( NSPoint& io_rPoint, bool bRelativeToScreen = true );
    void CocoaToVCL( NSPoint& io_Point, bool bRelativeToScreen = true );

    NSCursor* getCurrentCursor() const;

    CGMutablePathRef getClipPath() const { return mrClippingPath; }

    // called by VCL_NSApplication to indicate screen settings have changed
    void screenParametersChanged();

 private: // methods
    /** do things on initial show (like centering on parent or on screen)
    */
    void initShow();

    void initWindowAndView();

 private: // data
    static AquaSalFrame*                   s_pCaptureFrame;

    AquaSalFrame( const AquaSalFrame& ) SAL_DELETED_FUNCTION;
    AquaSalFrame& operator=(const AquaSalFrame&) SAL_DELETED_FUNCTION;
};

#endif // INCLUDED_VCL_INC_OSX_SALFRAME_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
