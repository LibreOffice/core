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

#pragma once

#include <vcl/sysdata.hxx>

#include <salframe.hxx>

#include <vector>

#ifdef IOS
#include <quartz/salgdi.h>
#define SvpSalInstance AquaSalInstance
#define SvpSalGraphics AquaSalGraphics
#endif

class SvpSalInstance;
class SvpSalGraphics;

class SvpSalFrame : public SalFrame
{
    SvpSalInstance*                     m_pInstance;
    SvpSalFrame*                        m_pParent;       // pointer to parent frame
    std::vector< SvpSalFrame* >         m_aChildren;     // Vector of child frames
    SalFrameStyleFlags                  m_nStyle;
    bool                                m_bVisible;
#ifndef IOS
    cairo_surface_t*                    m_pSurface;
#endif
    tools::Long                                m_nMinWidth;
    tools::Long                                m_nMinHeight;
    tools::Long                                m_nMaxWidth;
    tools::Long                                m_nMaxHeight;

    SystemEnvData                       m_aSystemChildData;

    std::vector< SvpSalGraphics* >      m_aGraphics;

    static SvpSalFrame*       s_pFocusFrame;
    OUString m_sTitle;

public:
    SvpSalFrame( SvpSalInstance* pInstance,
                 SalFrame* pParent,
                 SalFrameStyleFlags nSalFrameStyle );
    virtual ~SvpSalFrame() override;

    void GetFocus();
    void LoseFocus();
    void PostPaint() const;

    const OUString& title() const { return m_sTitle; }
    SalFrameStyleFlags style() const { return m_nStyle; }
    bool isVisible() const { return m_bVisible; }
    bool hasFocus() const { return s_pFocusFrame == this; }

    // SalFrame
    virtual SalGraphics*        AcquireGraphics() override;
    virtual void                ReleaseGraphics( SalGraphics* pGraphics ) override;

    virtual bool                PostEvent(std::unique_ptr<ImplSVEvent> pData) override;

    virtual void                SetTitle( const OUString& rTitle ) override;
    virtual void                SetIcon( sal_uInt16 nIcon ) override;
    virtual void                SetMenu( SalMenu* pMenu ) override;

    virtual void                SetExtendedFrameStyle( SalExtStyle nExtStyle ) override;
    virtual void                Show( bool bVisible, bool bNoActivate = false ) override;
    virtual void                SetMinClientSize( tools::Long nWidth, tools::Long nHeight ) override;
    virtual void                SetMaxClientSize( tools::Long nWidth, tools::Long nHeight ) override;
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
    using SalFrame::Flush;
    virtual void                Flush() override;
    virtual void                SetInputContext( SalInputContext* pContext ) override;
    virtual void                EndExtTextInput( EndExtTextInputFlags nFlags ) override;
    virtual OUString            GetKeyName( sal_uInt16 nKeyCode ) override;
    virtual bool                MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, vcl::KeyCode& rKeyCode ) override;
    virtual LanguageType        GetInputLanguage() override;
    virtual void                UpdateSettings( AllSettings& rSettings ) override;
    virtual void                Beep() override;
    virtual const SystemEnvData& GetSystemData() const override;
    virtual SalPointerState     GetPointerState() override;
    virtual KeyIndicatorState   GetIndicatorState() override;
    virtual void                SimulateKeyPress( sal_uInt16 nKeyCode ) override;
    virtual void                SetParent( SalFrame* pNewParent ) override;
    virtual void                SetPluginParent( SystemParentData* pNewParent ) override;
    virtual void                ResetClipRegion() override;
    virtual void                BeginSetClipRegion( sal_uInt32 nRects ) override;
    virtual void                UnionClipRegion( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight ) override;
    virtual void                EndSetClipRegion() override;

    /*TODO: functional implementation */
    virtual void                SetScreenNumber( unsigned int ) override {}
    virtual void                SetApplicationID(const OUString &) override {}

private:
    basegfx::B2IVector GetSurfaceFrameSize() const;
};

template <typename charT, typename traits>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& stream,
                                                     const SvpSalFrame& frame)
{
    stream << &frame << " (vis " << frame.isVisible() <<  " focus " << frame.hasFocus();
    stream << " style " << std::hex << std::setfill('0') << std::setw(8) << static_cast<sal_uInt32>(frame.style());
    OUString sTitle = frame.title();
    if (!sTitle.isEmpty())
        stream << " '" << sTitle << "'";
    stream << ")";
    return stream;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
