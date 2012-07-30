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

#ifndef _SVP_SVPFRAME_HXX

#include <vcl/sysdata.hxx>
#include <basegfx/range/b2ibox.hxx>

#include <salframe.hxx>
#include "svpelement.hxx"

#include <list>

class SvpSalInstance;
class SvpSalGraphics;

class SvpSalFrame : public SalFrame, public SvpElement
{
    SvpSalInstance*                     m_pInstance;
    SvpSalFrame*                        m_pParent;       // pointer to parent frame
    std::list< SvpSalFrame* >           m_aChildren;     // List of child frames
    sal_uLong                           m_nStyle;
    bool                                m_bVisible;
    bool                                m_bDamageTracking;
    bool                                m_bTopDown;
    sal_Int32                           m_nScanlineFormat;
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
                 sal_Int32 nScanlineFormat,
                 SystemParentData* pSystemParent = NULL );
    virtual ~SvpSalFrame();

    void GetFocus();
    void LoseFocus();
    void PostPaint(bool bImmediate) const;
    void AllocateFrame();

    // SvpElement
    virtual const basebmp::BitmapDeviceSharedPtr& getDevice() const { return m_aFrame; }

    // SalFrame
    virtual SalGraphics*        GetGraphics();
    virtual void                ReleaseGraphics( SalGraphics* pGraphics );

    virtual sal_Bool            PostEvent( void* pData );

    virtual void                SetTitle( const rtl::OUString& rTitle );
    virtual void                SetIcon( sal_uInt16 nIcon );
    virtual void                SetMenu( SalMenu* pMenu );
    virtual void                                DrawMenuBar();

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
    virtual void                SetBackgroundBitmap( SalBitmap* pBitmap );
    virtual void                ResetClipRegion();
    virtual void                BeginSetClipRegion( sal_uLong nRects );
    virtual void                UnionClipRegion( long nX, long nY, long nWidth, long nHeight );
    virtual void                EndSetClipRegion();

    // If enabled we can get damage notifications for regions immediately rendered to ...
    virtual void                enableDamageTracker( bool bOn = true );
    virtual void                damaged( const basegfx::B2IBox& /* rDamageRect */) {}

    /*TODO: functional implementation */
    virtual void                SetScreenNumber( unsigned int nScreen ) { (void)nScreen; }
    virtual void                SetApplicationID(const rtl::OUString &rApplicationID) { (void) rApplicationID; }
    bool IsVisible() { return m_bVisible; }

    static SvpSalFrame*         GetFocusFrame() { return s_pFocusFrame; }

};
#endif // _SVP_SVPFRAME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
