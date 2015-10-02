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

#ifndef INCLUDED_VCL_INC_HEADLESS_SVPFRAME_HXX
#define INCLUDED_VCL_INC_HEADLESS_SVPFRAME_HXX

#include <vcl/sysdata.hxx>
#include <basegfx/range/b2ibox.hxx>
#include <basebmp/bitmapdevice.hxx>

#include <salframe.hxx>

#include <list>

#ifdef IOS
#define SvpSalInstance AquaSalInstance
#define SvpSalGraphics AquaSalGraphics
#endif

class SvpSalInstance;
class SvpSalGraphics;

class VCL_DLLPUBLIC SvpSalFrame : public SalFrame
{
    SvpSalInstance*                     m_pInstance;
    SvpSalFrame*                        m_pParent;       // pointer to parent frame
    std::list< SvpSalFrame* >           m_aChildren;     // List of child frames
    SalFrameStyleFlags                  m_nStyle;
    bool                                m_bVisible;
    bool                                m_bTopDown;
#ifndef IOS
    basebmp::BitmapDeviceSharedPtr      m_aFrame;
    bool                                m_bDamageTracking;
#endif
    basebmp::Format                     m_nScanlineFormat;
    long                                m_nMinWidth;
    long                                m_nMinHeight;
    long                                m_nMaxWidth;
    long                                m_nMaxHeight;

    SystemEnvData                       m_aSystemChildData;

    std::list< SvpSalGraphics* >        m_aGraphics;

    static SvpSalFrame*       s_pFocusFrame;
public:
    SvpSalFrame( SvpSalInstance* pInstance,
                 SalFrame* pParent,
                 SalFrameStyleFlags nSalFrameStyle,
                 bool      bTopDown,
                 basebmp::Format nScanlineFormat,
                 SystemParentData* pSystemParent = NULL );
    virtual ~SvpSalFrame();

    void GetFocus();
    void LoseFocus();
    void PostPaint(bool bImmediate) const;

#if defined ANDROID
    const basebmp::BitmapDeviceSharedPtr& getDevice() const { return m_aFrame; }
#endif

    // SalFrame
    virtual SalGraphics*        AcquireGraphics() SAL_OVERRIDE;
    virtual void                ReleaseGraphics( SalGraphics* pGraphics ) SAL_OVERRIDE;

    virtual bool                PostEvent(ImplSVEvent* pData) SAL_OVERRIDE;

    virtual void                SetTitle( const OUString& rTitle ) SAL_OVERRIDE;
    virtual void                SetIcon( sal_uInt16 nIcon ) SAL_OVERRIDE;
    virtual void                SetMenu( SalMenu* pMenu ) SAL_OVERRIDE;
    virtual void                DrawMenuBar() SAL_OVERRIDE;

    virtual void                SetExtendedFrameStyle( SalExtStyle nExtStyle ) SAL_OVERRIDE;
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
    using SalFrame::Flush;
    virtual void                Flush() SAL_OVERRIDE;
    virtual void                Sync() SAL_OVERRIDE;
    virtual void                SetInputContext( SalInputContext* pContext ) SAL_OVERRIDE;
    virtual void                EndExtTextInput( EndExtTextInputFlags nFlags ) SAL_OVERRIDE;
    virtual OUString            GetKeyName( sal_uInt16 nKeyCode ) SAL_OVERRIDE;
    virtual bool                MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, vcl::KeyCode& rKeyCode ) SAL_OVERRIDE;
    virtual LanguageType        GetInputLanguage() SAL_OVERRIDE;
    virtual void                UpdateSettings( AllSettings& rSettings ) SAL_OVERRIDE;
    virtual void                Beep() SAL_OVERRIDE;
    virtual const SystemEnvData* GetSystemData() const SAL_OVERRIDE;
    virtual SalPointerState     GetPointerState() SAL_OVERRIDE;
    virtual KeyIndicatorState   GetIndicatorState() SAL_OVERRIDE;
    virtual void                SimulateKeyPress( sal_uInt16 nKeyCode ) SAL_OVERRIDE;
    virtual void                SetParent( SalFrame* pNewParent ) SAL_OVERRIDE;
    virtual bool                SetPluginParent( SystemParentData* pNewParent ) SAL_OVERRIDE;
    virtual void                ResetClipRegion() SAL_OVERRIDE;
    virtual void                BeginSetClipRegion( sal_uLong nRects ) SAL_OVERRIDE;
    virtual void                UnionClipRegion( long nX, long nY, long nWidth, long nHeight ) SAL_OVERRIDE;
    virtual void                EndSetClipRegion() SAL_OVERRIDE;

#ifdef ANDROID
    // If enabled we can get damage notifications for regions immediately rendered to ...
    void                        enableDamageTracker( bool bOn = true );
#endif

    /*TODO: functional implementation */
    virtual void                SetScreenNumber( unsigned int nScreen ) SAL_OVERRIDE { (void)nScreen; }
    virtual void                SetApplicationID(const OUString &rApplicationID) SAL_OVERRIDE { (void) rApplicationID; }

    static SvpSalFrame*         GetFocusFrame() { return s_pFocusFrame; }

};

#endif // INCLUDED_VCL_INC_HEADLESS_SVPFRAME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
