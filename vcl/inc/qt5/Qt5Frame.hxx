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

#include <salframe.hxx>

#include "Qt5Tools.hxx"

#include <headless/svpgdi.hxx>

class Qt5Graphics;
class Qt5Instance;
class Qt5Widget;
class QWidget;
class QPaintDevice;
class QImage;
class SvpSalGraphics;

class Qt5Frame : public SalFrame
{
    friend class Qt5Widget;

    std::unique_ptr<QWidget> m_pQWidget;

    const bool m_bUseCairo;
    std::unique_ptr<QImage> m_pQImage;
    std::unique_ptr<Qt5Graphics> m_pQt5Graphics;
    UniqueCairoSurface m_pSurface;
    std::unique_ptr<SvpSalGraphics> m_pSvpGraphics;
    DamageHandler m_aDamageHandler;

    bool m_bGraphicsInUse;
    SalFrameStyleFlags m_nStyle;
    Qt5Frame* m_pParent;
    PointerStyle m_ePointerStyle;

    bool m_bDefaultSize;
    bool m_bDefaultPos;

    void Center();
    Size CalcDefaultSize();
    void SetDefaultSize();

    bool isChild(bool bPlug = true, bool bSysChild = true)
    {
        SalFrameStyleFlags nMask = SalFrameStyleFlags::NONE;
        if (bPlug)
            nMask |= SalFrameStyleFlags::PLUG;
        if (bSysChild)
            nMask |= SalFrameStyleFlags::SYSTEMCHILD;
        return bool(m_nStyle & nMask);
    }

    void TriggerPaintEvent();
    void TriggerPaintEvent(QRect aRect);

public:
    Qt5Frame(Qt5Frame* pParent, SalFrameStyleFlags nSalFrameStyle, bool bUseCairo);
    virtual ~Qt5Frame() override;

    QWidget* GetQWidget() const { return m_pQWidget.get(); }

    void Damage(sal_Int32 nExtentsX, sal_Int32 nExtentsY, sal_Int32 nExtentsWidth,
                sal_Int32 nExtentsHeight) const;

    virtual SalGraphics* AcquireGraphics() override;
    virtual void ReleaseGraphics(SalGraphics* pGraphics) override;

    virtual bool PostEvent(ImplSVEvent* pData) override;

    virtual void SetTitle(const OUString& rTitle) override;
    virtual void SetIcon(sal_uInt16 nIcon) override;
    virtual void SetMenu(SalMenu* pMenu) override;
    virtual void DrawMenuBar() override;

    virtual void SetExtendedFrameStyle(SalExtStyle nExtStyle) override;
    virtual void Show(bool bVisible, bool bNoActivate = false) override;
    virtual void SetMinClientSize(long nWidth, long nHeight) override;
    virtual void SetMaxClientSize(long nWidth, long nHeight) override;
    virtual void SetPosSize(long nX, long nY, long nWidth, long nHeight,
                            sal_uInt16 nFlags) override;
    virtual void GetClientSize(long& rWidth, long& rHeight) override;
    virtual void GetWorkArea(tools::Rectangle& rRect) override;
    virtual SalFrame* GetParent() const override;
    virtual void SetWindowState(const SalFrameState* pState) override;
    virtual bool GetWindowState(SalFrameState* pState) override;
    virtual void ShowFullScreen(bool bFullScreen, sal_Int32 nDisplay) override;
    virtual void StartPresentation(bool bStart) override;
    virtual void SetAlwaysOnTop(bool bOnTop) override;
    virtual void ToTop(SalFrameToTop nFlags) override;
    virtual void SetPointer(PointerStyle ePointerStyle) override;
    virtual void CaptureMouse(bool bMouse) override;
    virtual void SetPointerPos(long nX, long nY) override;
    virtual void Flush() override;
    virtual void Flush(const tools::Rectangle& rRect) override;
    virtual void SetInputContext(SalInputContext* pContext) override;
    virtual void EndExtTextInput(EndExtTextInputFlags nFlags) override;
    virtual OUString GetKeyName(sal_uInt16 nKeyCode) override;
    virtual bool MapUnicodeToKeyCode(sal_Unicode aUnicode, LanguageType aLangType,
                                     vcl::KeyCode& rKeyCode) override;
    virtual LanguageType GetInputLanguage() override;
    virtual void UpdateSettings(AllSettings& rSettings) override;
    virtual void Beep() override;
    virtual const SystemEnvData* GetSystemData() const override;
    virtual SalPointerState GetPointerState() override;
    virtual KeyIndicatorState GetIndicatorState() override;
    virtual void SimulateKeyPress(sal_uInt16 nKeyCode) override;
    virtual void SetParent(SalFrame* pNewParent) override;
    virtual bool SetPluginParent(SystemParentData* pNewParent) override;
    virtual void ResetClipRegion() override;
    virtual void BeginSetClipRegion(sal_uLong nRects) override;
    virtual void UnionClipRegion(long nX, long nY, long nWidth, long nHeight) override;
    virtual void EndSetClipRegion() override;

    virtual void SetScreenNumber(unsigned int) override;
    virtual void SetApplicationID(const OUString&) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
