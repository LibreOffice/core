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
#include <vclpluginapi.h>

#include "Qt5Tools.hxx"

#include <headless/svpgdi.hxx>
#include <vcl/svapp.hxx>
#include <vcl/sysdata.hxx>

#include <QtCore/QObject>

class Qt5Graphics;
class Qt5Instance;
class Qt5Menu;
class QWidget;
class Qt5MainWindow;
class Qt5DragSource;
class Qt5DropTarget;
class QMimeData;
class QPaintDevice;
class QScreen;
class QImage;
class SvpSalGraphics;

class VCLPLUG_QT5_PUBLIC Qt5Frame : public QObject, public SalFrame
{
    Q_OBJECT

    friend class Qt5Widget;

    QWidget* m_pQWidget;
    Qt5MainWindow* m_pTopLevel;

    const bool m_bUseCairo;
    std::unique_ptr<QImage> m_pQImage;
    std::unique_ptr<Qt5Graphics> m_pQt5Graphics;
    UniqueCairoSurface m_pSurface;
    std::unique_ptr<SvpSalGraphics> m_pOurSvpGraphics;
    // in base class, this ptr is the same as m_pOurSvpGraphic
    // in derived class, it can point to a derivative
    // of SvpSalGraphics (which the derived class then owns)
    SvpSalGraphics* m_pSvpGraphics;
    DamageHandler m_aDamageHandler;
    QRegion m_aRegion;
    bool m_bNullRegion;

    bool m_bGraphicsInUse;
    SalFrameStyleFlags m_nStyle;
    Qt5Frame* m_pParent;
    PointerStyle m_ePointerStyle;

    SystemEnvData m_aSystemData;

    Qt5Menu* m_pSalMenu;

    Qt5DragSource* m_pDragSource;
    Qt5DropTarget* m_pDropTarget;
    bool m_bInDrag;

    bool m_bDefaultSize;
    bool m_bDefaultPos;
    bool m_bFullScreen;
    QRect m_aRestoreGeometry;

    void Center();
    Size CalcDefaultSize();
    void SetDefaultSize();

    bool isChild(bool bPlug = true, bool bSysChild = true) const
    {
        SalFrameStyleFlags nMask = SalFrameStyleFlags::NONE;
        if (bPlug)
            nMask |= SalFrameStyleFlags::PLUG;
        if (bSysChild)
            nMask |= SalFrameStyleFlags::SYSTEMCHILD;
        return bool(m_nStyle & nMask);
    }

    bool isWindow() const;
    QWindow* windowHandle() const;
    QScreen* screen() const;
    bool isMinimized() const;
    bool isMaximized() const;
    void SetWindowStateImpl(Qt::WindowStates eState);

    void TriggerPaintEvent();
    void TriggerPaintEvent(QRect aRect);

private:
    void setVisible(bool);

Q_SIGNALS:
    void tooltipRequest(const OUString& rTooltip);

public:
    Qt5Frame(Qt5Frame* pParent, SalFrameStyleFlags nSalFrameStyle, bool bUseCairo);
    virtual ~Qt5Frame() override;

    QWidget* GetQWidget() const { return m_pQWidget; }
    Qt5MainWindow* GetTopLevelWindow() const { return m_pTopLevel; }

    void Damage(sal_Int32 nExtentsX, sal_Int32 nExtentsY, sal_Int32 nExtentsWidth,
                sal_Int32 nExtentsHeight) const;

    virtual void InitSvpSalGraphics(SvpSalGraphics* pSvpSalGraphics);
    virtual SalGraphics* AcquireGraphics() override;
    virtual void ReleaseGraphics(SalGraphics* pGraphics) override;

    virtual bool PostEvent(std::unique_ptr<ImplSVEvent> pData) override;

    virtual void SetTitle(const OUString& rTitle) override;
    virtual void SetIcon(sal_uInt16 nIcon) override;
    virtual void SetMenu(SalMenu* pMenu) override;
    virtual void DrawMenuBar() override;

    virtual void registerDragSource(Qt5DragSource* pDragSource);
    virtual void deregisterDragSource(Qt5DragSource const* pDragSource);
    virtual void registerDropTarget(Qt5DropTarget* pDropTarget);
    virtual void deregisterDropTarget(Qt5DropTarget const* pDropTarget);
    void draggingStarted(const int x, const int y, Qt::DropActions eActions,
                         const QMimeData* pQMimeData);
    void dropping(const int x, const int y, const QMimeData* pQMimeData);

    virtual void SetExtendedFrameStyle(SalExtStyle nExtStyle) override;
    virtual void Show(bool bVisible, bool bNoActivate = false) override;
    virtual void SetMinClientSize(long nWidth, long nHeight) override;
    virtual void SetMaxClientSize(long nWidth, long nHeight) override;
    virtual void SetPosSize(long nX, long nY, long nWidth, long nHeight,
                            sal_uInt16 nFlags) override;
    virtual void GetClientSize(long& rWidth, long& rHeight) override;
    virtual void GetWorkArea(tools::Rectangle& rRect) override;
    virtual SalFrame* GetParent() const override;
    virtual void SetModal(bool bModal) override;
    virtual bool GetModal() const override;
    virtual void SetWindowState(const SalFrameState* pState) override;
    virtual bool GetWindowState(SalFrameState* pState) override;
    virtual void ShowFullScreen(bool bFullScreen, sal_Int32 nDisplay) override;
    virtual void StartPresentation(bool bStart) override;
    virtual void SetAlwaysOnTop(bool bOnTop) override;
    virtual void ToTop(SalFrameToTop nFlags) override;
    virtual void SetPointer(PointerStyle ePointerStyle) override;
    virtual void CaptureMouse(bool bMouse) override;
    virtual void SetPointerPos(long nX, long nY) override;
    virtual bool ShowTooltip(const OUString& rText, const tools::Rectangle& rHelpArea) override;
    using SalFrame::Flush;
    virtual void Flush() override;
    virtual void SetInputContext(SalInputContext* pContext) override;
    virtual void EndExtTextInput(EndExtTextInputFlags nFlags) override;
    virtual OUString GetKeyName(sal_uInt16 nKeyCode) override;
    virtual bool MapUnicodeToKeyCode(sal_Unicode aUnicode, LanguageType aLangType,
                                     vcl::KeyCode& rKeyCode) override;
    virtual LanguageType GetInputLanguage() override;
    virtual void UpdateSettings(AllSettings& rSettings) override;
    virtual void Beep() override;
    virtual const SystemEnvData* GetSystemData() const override { return &m_aSystemData; }
    virtual SalPointerState GetPointerState() override;
    virtual KeyIndicatorState GetIndicatorState() override;
    virtual void SimulateKeyPress(sal_uInt16 nKeyCode) override;
    virtual void SetParent(SalFrame* pNewParent) override;
    virtual bool SetPluginParent(SystemParentData* pNewParent) override;
    virtual void ResetClipRegion() override;
    virtual void BeginSetClipRegion(sal_uInt32 nRects) override;
    virtual void UnionClipRegion(long nX, long nY, long nWidth, long nHeight) override;
    virtual void EndSetClipRegion() override;

    virtual void SetScreenNumber(unsigned int) override;
    virtual void SetApplicationID(const OUString&) override;

    inline bool CallCallback(SalEvent nEvent, const void* pEvent) const;
};

inline bool Qt5Frame::CallCallback(SalEvent nEvent, const void* pEvent) const
{
    SolarMutexGuard aGuard;
    return SalFrame::CallCallback(nEvent, pEvent);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
