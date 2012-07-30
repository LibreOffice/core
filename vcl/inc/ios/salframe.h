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

#include "vcl/sysdata.hxx"

#include "ios/salmenu.h"
#include "ios/saldata.hxx"
#include "ios/iosvcltypes.h"

#include "salframe.hxx"

#include <vector>
#include <utility>
#include <stdexcept>

#include <boost/shared_ptr.hpp>

class IosSalGraphics;
class IosSalFrame;
class IosSalTimer;
class IosSalInstance;
class IosSalMenu;
class IosBlinker;

typedef struct SalFrame::SalPointerState SalPointerState;

// ----------------
// - IosSalFrame -
// ----------------

class IosSalFrame : public SalFrame
{
public:
    UIWindow*                       mpWindow;
    UIView*                         mpView;
    CGRect                          maScreenRect;           // for mirroring purposes
    IosSalGraphics*         mpGraphics;
    IosSalFrame*            mpParent;
    SystemEnvData           maSysData;
    int                             mnMinWidth;             // min. client width in pixels
    int                             mnMinHeight;            // min. client height in pixels
    int                             mnMaxWidth;             // max. client width in pixels
    int                             mnMaxHeight;            // max. client height in pixels
    CGRect                          maFullScreenRect;       // old window size when in FullScreen
    bool                mbGraphics:1;           // is Graphics used?
    bool                            mbShown:1;
    bool                            mbInitShow:1;
    bool                            mbPositioned:1;
    bool                            mbSized:1;
    bool                            mbPresentation:1;

    sal_uLong                       mnStyle;
    unsigned int                    mnStyleMask;            // our style mask from UIWindow creation

    sal_uLong                       mnLastEventTime;
    unsigned int                    mnLastModifierFlags;
    IosSalMenu*                     mpMenu;

    SalExtStyle                     mnExtStyle;             // currently document frames are marked this way

    PointerStyle                    mePointerStyle;         // currently active pointer style

    CGMutablePathRef                mrClippingPath;         // used for "shaping"
    std::vector< CGRect >           maClippingRects;

    std::list<IosBlinker*>          maBlinkers;

    Rectangle                       maInvalidRect;

    sal_uLong                       mnICOptions;

    boost::shared_ptr< Timer >      mpActivityTimer; // Timer to prevent system sleep during presentation
public:
    /** Constructor

        Creates a system window and connects this frame with it.

        @throws std::runtime_error in case window creation fails
    */
    IosSalFrame( SalFrame* pParent, sal_uLong salFrameStyle );

    virtual ~IosSalFrame();

    virtual SalGraphics*        GetGraphics();
    virtual void                ReleaseGraphics( SalGraphics* pGraphics );
    virtual sal_Bool                PostEvent( void* pData );
    virtual void                SetTitle( const rtl::OUString& rTitle );
    virtual void                SetIcon( sal_uInt16 nIcon );
    virtual void                SetRepresentedURL( const rtl::OUString& );
    virtual void                SetMenu( SalMenu* pSalMenu );
    virtual void                DrawMenuBar();
    virtual void                Show( sal_Bool bVisible, sal_Bool bNoActivate = sal_False );
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
    virtual void                Flush( void );
    virtual void                Flush( const Rectangle& );
    virtual void                Sync();
    virtual void                SetInputContext( SalInputContext* pContext );
    virtual void                EndExtTextInput( sal_uInt16 nFlags );
    virtual rtl::OUString       GetKeyName( sal_uInt16 nKeyCode );
    virtual sal_Bool                MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, KeyCode& rKeyCode );
    virtual LanguageType        GetInputLanguage();
    virtual void                UpdateSettings( AllSettings& rSettings );
    virtual const SystemEnvData*    GetSystemData() const;
    virtual SalPointerState     GetPointerState();
    virtual SalIndicatorState   GetIndicatorState();
    virtual void                SimulateKeyPress( sal_uInt16 nKeyCode );
    virtual void                SetParent( SalFrame* pNewParent );
    virtual bool                SetPluginParent( SystemParentData* pNewParent );
    virtual void                SetExtendedFrameStyle( SalExtStyle );
    virtual void                SetBackgroundBitmap( SalBitmap* );
    virtual void                SetScreenNumber(unsigned int);
    virtual void                SetApplicationID( const rtl::OUString &rApplicationID );

    // shaped system windows
    // set clip region to none (-> rectangular windows, normal state)
    virtual void ResetClipRegion();
    // start setting the clipregion consisting of nRects rectangles
    virtual void BeginSetClipRegion( sal_uLong nRects );
    // add a rectangle to the clip region
    virtual void UnionClipRegion( long nX, long nY, long nWidth, long nHeight );
    // done setting up the clipregion
    virtual void EndSetClipRegion();

    virtual void SetClientSize( long nWidth, long nHeight );

    void UpdateFrameGeometry();

    // trigger painting of the window
    void SendPaintEvent( const Rectangle* pRect = NULL );

    static bool isAlive( const IosSalFrame* pFrame )
    { return GetSalData()->maFrameCheck.find( pFrame ) != GetSalData()->maFrameCheck.end(); }

    static IosSalFrame* GetCaptureFrame() { return s_pCaptureFrame; }

    UIWindow* getWindow() const { return mpWindow; }
    UIView* getView() const { return mpView; }
    unsigned int getStyleMask() const { return mnStyleMask; }

    void getResolution( long& o_rDPIX, long& o_rDPIY );

    // actually the follwing methods do the same thing: flipping y coordinates
    // but having two of them makes clearer what the coordinate system
    // is supposed to be before and after
    void VCLToCocoaTouch( CGRect& io_rRect, bool bRelativeToScreen = true );
    void CocoaTouchToVCL( CGRect& io_rRect, bool bRelativeToScreen = true );

    void VCLToCocoaTouch( CGPoint& io_rPoint, bool bRelativeToScreen = true );
    void CocoaTouchToVCL( CGPoint& io_Point, bool bRelativeToScreen = true );

    CGMutablePathRef getClipPath() const { return mrClippingPath; }

    // called by VCL_UIApplication to indicate screen settings have changed
    void screenParametersChanged();

 private: // methods
    /** do things on initial show (like centering on parent or on screen)
    */
    void initShow();

    void initWindowAndView();

 private: // data
    static IosSalFrame*                   s_pCaptureFrame;

    // make IosSalFrame non copyable
    IosSalFrame( const IosSalFrame& );
    IosSalFrame& operator=(const IosSalFrame&);
};

#endif // _SV_SALFRAME_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
