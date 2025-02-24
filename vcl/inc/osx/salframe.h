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

#include <tools/long.hxx>
#include <vcl/sysdata.hxx>

#include <osx/salinst.h>
#include <osx/salmenu.h>
#include <osx/saldata.hxx>
#include <osx/osxvcltypes.h>

#include <salframe.hxx>

#include <vector>
#include <utility>
#include <stdexcept>

class AquaSalGraphics;
class AquaSalFrame;
class AquaSalTimer;
class AquaSalInstance;
class AquaSalMenu;

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
    bool                            mbGraphics;             // is Graphics used?
    bool                            mbShown;
    bool                            mbInitShow;
    bool                            mbPositioned;
    bool                            mbSized;
    bool                            mbPresentation;

    SalFrameStyleFlags              mnStyle;
    unsigned int                    mnStyleMask;            // our style mask from NSWindow creation

    sal_uInt64                      mnLastEventTime;
    unsigned int                    mnLastModifierFlags;
    AquaSalMenu*                    mpMenu;

    SalExtStyle                     mnExtStyle;             // currently document frames are marked this way

    PointerStyle                    mePointerStyle;         // currently active pointer style

    NSRect                          maTrackingRect;

    CGMutablePathRef                mrClippingPath;         // used for "shaping"
    std::vector< CGRect >           maClippingRects;

    tools::Rectangle                       maInvalidRect;

    InputContextFlags               mnICOptions;

    // To prevent display sleep during presentation
    IOPMAssertionID                 mnAssertionID;

    NSRect                          maFrameRect;
    NSRect                          maContentRect;

    bool                            mbGeometryDidChange;

    int                             mnBlinkCursorDelay;

    // tdf#155266 force flush after scrolling
    bool                            mbForceFlushScrolling;
    // tdf#164428 force flush after drawing a progress bar
    bool                            mbForceFlushProgressBar;

    // Is window in LibreOffice full screen mode
    bool                            mbInternalFullScreen;
    // Window size to restore to when exiting LibreOffice full screen mode
    NSRect                          maInternalFullScreenRestoreRect;
    // Desired window size when entering exiting LibreOffice full screen mode
    NSRect                          maInternalFullScreenExpectedRect;

    // Is window in native full screen mode
    bool                            mbNativeFullScreen;
    // Window size to restore to when exiting LibreOffice full screen mode
    NSRect                          maNativeFullScreenRestoreRect;

public:
    /** Constructor

        Creates a system window and connects this frame with it.

        @throws std::runtime_error in case window creation fails
    */
    AquaSalFrame( SalFrame* pParent, SalFrameStyleFlags salFrameStyle );

    virtual ~AquaSalFrame() override;

    virtual SalGraphics*        AcquireGraphics() override;
    virtual void                ReleaseGraphics( SalGraphics* pGraphics ) override;
    virtual bool                PostEvent(std::unique_ptr<ImplSVEvent> pData) override;
    virtual void                SetTitle( const OUString& rTitle ) override;
    virtual void                SetIcon( sal_uInt16 nIcon ) override;
    virtual void                SetRepresentedURL( const OUString& ) override;
    virtual void                SetMenu( SalMenu* pSalMenu ) override;
    virtual void                Show( bool bVisible, bool bNoActivate = false ) override;
    virtual void                SetMinClientSize( tools::Long nWidth, tools::Long nHeight )
        override;
    virtual void                SetMaxClientSize( tools::Long nWidth, tools::Long nHeight )
        override;
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
    virtual void                Flush( void ) override;
    virtual void                Flush( const tools::Rectangle& ) override;
    virtual void                SetInputContext( SalInputContext* pContext ) override;
    virtual void                EndExtTextInput( EndExtTextInputFlags nFlags ) override;
    virtual OUString            GetKeyName( sal_uInt16 nKeyCode ) override;
    virtual bool                MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, vcl::KeyCode& rKeyCode ) override;
    virtual LanguageType        GetInputLanguage() override;
    virtual void                UpdateSettings( AllSettings& rSettings ) override;
    virtual void                Beep() override;
    virtual const SystemEnvData&  GetSystemData() const override;
    virtual SalPointerState     GetPointerState() override;
    virtual KeyIndicatorState   GetIndicatorState() override;
    virtual void                SimulateKeyPress( sal_uInt16 nKeyCode ) override;
    virtual void                SetParent( SalFrame* pNewParent ) override;
    virtual void                SetPluginParent( SystemParentData* pNewParent ) override;
    virtual void                SetExtendedFrameStyle( SalExtStyle ) override;
    virtual void                SetScreenNumber(unsigned int) override;
    virtual void                SetApplicationID( const OUString &rApplicationID ) override;

    // shaped system windows
    // set clip region to none (-> rectangular windows, normal state)
    virtual void ResetClipRegion() override;
    // start setting the clipregion consisting of nRects rectangles
    virtual void BeginSetClipRegion( sal_uInt32 nRects ) override;
    // add a rectangle to the clip region
    virtual void UnionClipRegion(
        tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight ) override;
    // done setting up the clipregion
    virtual void EndSetClipRegion() override;
    virtual void UpdateDarkMode() override;
    virtual bool GetUseDarkMode() const override;
    virtual bool GetUseReducedAnimation() const override;

    void UpdateFrameGeometry();

    // trigger painting of the window
    void SendPaintEvent( const tools::Rectangle* pRect = nullptr );

    static inline bool isAlive( const AquaSalFrame* pFrame );

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

    NSCursor* getCurrentCursor();

    CGMutablePathRef getClipPath() const { return mrClippingPath; }

    // called by VCL_NSApplication to indicate screen settings have changed
    void screenParametersChanged();

protected:
    SalEvent PreparePosSize(
        tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, sal_uInt16 nFlags);

private: // methods
    /** do things on initial show (like centering on parent or on screen)
    */
    void initShow();

    void initWindowAndView();

    void doShowFullScreen( bool bFullScreen, sal_Int32 nDisplay );

    void doResetClipRegion();

    bool doFlush();

    void doUpdateSettings( AllSettings& rSettings );

private: // data
    static AquaSalFrame*       s_pCaptureFrame;

    AquaSalFrame( const AquaSalFrame& ) = delete;
    AquaSalFrame& operator=(const AquaSalFrame&) = delete;
};

inline bool AquaSalFrame::isAlive( const AquaSalFrame* pFrame )
{
    AquaSalInstance *pInst = GetSalData()->mpInstance;
    return pInst && pInst->isFrameAlive( pFrame );
}

#endif // INCLUDED_VCL_INC_OSX_SALFRAME_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
