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

#ifndef _SVP_SVPFRAME_HXX

#include <vcl/sysdata.hxx>
#include <basegfx/range/b2ibox.hxx>
#include <basebmp/bitmapdevice.hxx>

#include <salframe.hxx>

#include <list>

#ifdef IOS
#define SvpSalGraphics AquaSalGraphics
#endif

class SvpSalInstance;
class SvpSalGraphics;

class SvpSalFrame : public SalFrame
{
    SvpSalInstance*                     m_pInstance;
    SvpSalFrame*                        m_pParent;       // pointer to parent frame
    std::list< SvpSalFrame* >           m_aChildren;     // List of child frames
    sal_uLong                           m_nStyle;
    bool                                m_bVisible;
    bool                                m_bDamageTracking;
    bool                                m_bTopDown;
    basebmp::Format                     m_nScanlineFormat;
    long                                m_nMinWidth;
    long                                m_nMinHeight;
    long                                m_nMaxWidth;
    long                                m_nMaxHeight;

    SystemEnvData                       m_aSystemChildData;

    basebmp::BitmapDeviceSharedPtr      m_aFrame;
    std::list< SvpSalGraphics* >        m_aGraphics;

    static SvpSalFrame*       s_pFocusFrame;
public:
    SvpSalFrame( SvpSalInstance* pInstance,
                 SalFrame* pParent,
                 sal_uLong nSalFrameStyle,
                 bool      bTopDown,
                 basebmp::Format nScanlineFormat,
                 SystemParentData* pSystemParent = NULL );
    virtual ~SvpSalFrame();

    void GetFocus();
    void LoseFocus();
    void PostPaint(bool bImmediate) const;
    void AllocateFrame();

#if defined IOS || defined ANDROID
    const basebmp::BitmapDeviceSharedPtr& getDevice() const { return m_aFrame; }
#endif

    // SalFrame
    virtual SalGraphics*        GetGraphics();
    virtual void                ReleaseGraphics( SalGraphics* pGraphics );

    virtual sal_Bool            PostEvent( void* pData );

    virtual void                SetTitle( const OUString& rTitle );
    virtual void                SetIcon( sal_uInt16 nIcon );
    virtual void                SetMenu( SalMenu* pMenu );
    virtual void                DrawMenuBar();

    virtual void                SetExtendedFrameStyle( SalExtStyle nExtStyle );
    virtual void                Show( sal_Bool bVisible, sal_Bool bNoActivate = sal_False );
    virtual void                Enable( sal_Bool bEnable );
    virtual void                SetMinClientSize( long nWidth, long nHeight );
    virtual void                SetMaxClientSize( long nWidth, long nHeight );
    virtual void                SetPosSize( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags );
    virtual void                GetClientSize( long& rWidth, long& rHeight );
    virtual void                GetWorkArea( Rectangle& rRect );
    virtual SalFrame*           GetParent() const;
    virtual void                SetWindowState( const SalFrameState* pState );
    virtual sal_Bool            GetWindowState( SalFrameState* pState );
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
    virtual OUString       GetKeyName( sal_uInt16 nKeyCode );
    virtual sal_Bool            MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, KeyCode& rKeyCode );
    virtual LanguageType        GetInputLanguage();
    virtual void                UpdateSettings( AllSettings& rSettings );
    virtual void                Beep();
    virtual const SystemEnvData* GetSystemData() const;
    virtual SalPointerState     GetPointerState();
    virtual SalIndicatorState   GetIndicatorState();
    virtual void                SimulateKeyPress( sal_uInt16 nKeyCode );
    virtual void                SetParent( SalFrame* pNewParent );
    virtual bool                SetPluginParent( SystemParentData* pNewParent );
    virtual void                ResetClipRegion();
    virtual void                BeginSetClipRegion( sal_uLong nRects );
    virtual void                UnionClipRegion( long nX, long nY, long nWidth, long nHeight );
    virtual void                EndSetClipRegion();

    // If enabled we can get damage notifications for regions immediately rendered to ...
    virtual void                enableDamageTracker( bool bOn = true );
    virtual void                damaged( const basegfx::B2IBox& /* rDamageRect */) {}

    /*TODO: functional implementation */
    virtual void                SetScreenNumber( unsigned int nScreen ) { (void)nScreen; }
    virtual void                SetApplicationID(const OUString &rApplicationID) { (void) rApplicationID; }
    bool                        IsVisible() { return m_bVisible; }

    static SvpSalFrame*         GetFocusFrame() { return s_pFocusFrame; }

};
#endif // _SVP_SVPFRAME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
