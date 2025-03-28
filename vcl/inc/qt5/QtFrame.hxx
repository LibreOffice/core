/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <config_vclplug.h>

#include <salframe.hxx>
#include <vclpluginapi.h>

#include "QtTools.hxx"
#include "QtWidget.hxx"

#include <headless/svpgdi.hxx>
#include <vcl/svapp.hxx>
#include <vcl/sysdata.hxx>

#include <QtCore/QObject>

#if !defined EMSCRIPTEN
#include <unx/sessioninhibitor.hxx>
#endif
#if CHECK_ANY_QT_USING_X11
// any better way to get rid of the X11 / Qt type clashes?
#undef Bool
#undef CursorShape
#undef Expose
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut
#undef FontChange
#undef None
#undef Status
#undef Unsorted
#endif

class QtDragSource;
class QtDropTarget;
class QtGraphics;
class QtInstance;
class QtMainWindow;
class QtMenu;
class QtSvpGraphics;

class QDragMoveEvent;
class QDropEvent;
class QImage;
class QMimeData;
class QPaintDevice;
class QScreen;
class QWidget;

class VCLPLUG_QT_PUBLIC QtFrame : public QObject, public SalFrame
{
    Q_OBJECT

    friend class QtWidget;

    QtWidget* m_pQWidget;
    QtMainWindow* m_pTopLevel;

    const bool m_bUseCairo;
    std::unique_ptr<QImage> m_pQImage;
    std::unique_ptr<QtGraphics> m_pQtGraphics;
    UniqueCairoSurface m_pSurface;
    std::unique_ptr<QtSvpGraphics> m_pSvpGraphics;
    DamageHandler m_aDamageHandler;
    QRegion m_aRegion;
    bool m_bNullRegion;

    bool m_bGraphicsInUse;
    SalFrameStyleFlags m_nStyle;
    QtFrame* m_pParent;
    PointerStyle m_ePointerStyle;

    SystemEnvData m_aSystemData;

    QtDragSource* m_pDragSource;
    QtDropTarget* m_pDropTarget;
    bool m_bInDrag;

    bool m_bDefaultSize;
    bool m_bDefaultPos;
    bool m_bFullScreen;
    bool m_bFullScreenSpanAll;
    sal_uInt32 m_nRestoreScreen;
    QRect m_aRestoreGeometry;

#if !defined EMSCRIPTEN
    SessionManagerInhibitor m_SessionManagerInhibitor;
#endif
#if CHECK_ANY_QT_USING_X11
    ModKeyFlags m_nKeyModifiers;
#endif

    LanguageType m_nInputLanguage;

    OUString m_aTooltipText;
    QRect m_aTooltipArea;

    void SetDefaultPos();
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
    sal_Int32 screenNumber() const;
    bool isMinimized() const;
    bool isMaximized() const;
    void SetWindowStateImpl(Qt::WindowStates eState);

private Q_SLOTS:
    void screenChanged(QScreen*);

public:
    QtFrame(QtFrame* pParent, SalFrameStyleFlags nSalFrameStyle, bool bUseCairo);
    virtual ~QtFrame() override;

    QWidget* GetQWidget() const { return m_pQWidget; }
    QtMainWindow* GetTopLevelWindow() const { return m_pTopLevel; }
    QWidget* asChild() const;
    qreal devicePixelRatioF() const;

    void Damage(sal_Int32 nExtentsX, sal_Int32 nExtentsY, sal_Int32 nExtentsWidth,
                sal_Int32 nExtentsHeight) const;

    virtual SalGraphics* AcquireGraphics() override;
    virtual void ReleaseGraphics(SalGraphics* pGraphics) override;

    virtual bool PostEvent(std::unique_ptr<ImplSVEvent> pData) override;

    virtual void SetTitle(const OUString& rTitle) override;
    virtual void SetIcon(sal_uInt16 nIcon) override;
    virtual void SetMenu(SalMenu* pMenu) override;

    virtual void registerDragSource(QtDragSource* pDragSource);
    virtual void deregisterDragSource(QtDragSource const* pDragSource);
    virtual void registerDropTarget(QtDropTarget* pDropTarget);
    virtual void deregisterDropTarget(QtDropTarget const* pDropTarget);

    void handleDragLeave();
    void handleDragMove(QDragMoveEvent* pEvent);
    void handleDrop(QDropEvent* pEvent);
    void handleMoveEvent(QMoveEvent* pEvent);
    void handlePaintEvent(const QPaintEvent* pEvent, QWidget* pWidget);
    void handleResizeEvent(const QResizeEvent* pEvent);

    virtual void SetExtendedFrameStyle(SalExtStyle nExtStyle) override;
    virtual void Show(bool bVisible, bool bNoActivate = false) override;
    virtual void SetMinClientSize(tools::Long nWidth, tools::Long nHeight) override;
    virtual void SetMaxClientSize(tools::Long nWidth, tools::Long nHeight) override;
    virtual void SetPosSize(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight,
                            sal_uInt16 nFlags) override;
    virtual void GetClientSize(tools::Long& rWidth, tools::Long& rHeight) override;
    SalFrameGeometry GetUnmirroredGeometry() const override;
    virtual void GetWorkArea(AbsoluteScreenPixelRectangle& rRect) override;
    virtual SalFrame* GetParent() const override;
    virtual void SetModal(bool bModal) override;
    virtual void SetWindowState(const vcl::WindowData*) override;
    virtual bool GetWindowState(vcl::WindowData*) override;
    virtual void ShowFullScreen(bool bFullScreen, sal_Int32 nDisplay) override;
    virtual void StartPresentation(bool bStart) override;
    virtual void SetAlwaysOnTop(bool bOnTop) override;
    virtual void ToTop(SalFrameToTop nFlags) override;
    virtual void SetPointer(PointerStyle ePointerStyle) override;
    virtual void CaptureMouse(bool bMouse) override;
    virtual void SetPointerPos(tools::Long nX, tools::Long nY) override;
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
    virtual const SystemEnvData& GetSystemData() const override { return m_aSystemData; }
    virtual SalPointerState GetPointerState() override;
    virtual KeyIndicatorState GetIndicatorState() override;
    virtual void SimulateKeyPress(sal_uInt16 nKeyCode) override;
    virtual void SetParent(SalFrame* pNewParent) override;
    virtual void SetPluginParent(SystemParentData* pNewParent) override;
    virtual void ResetClipRegion() override;
    virtual void BeginSetClipRegion(sal_uInt32 nRects) override;
    virtual void UnionClipRegion(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                 tools::Long nHeight) override;
    virtual void EndSetClipRegion() override;

    virtual void SetScreenNumber(unsigned int) override;
    virtual void SetApplicationID(const OUString&) override;
    virtual void ResolveWindowHandle(SystemEnvData& rData) const override;
    virtual bool GetUseDarkMode() const override;
    virtual bool GetUseReducedAnimation() const override;

    inline bool CallCallback(SalEvent nEvent, const void* pEvent) const;

    void setInputLanguage(LanguageType);
    inline bool isPopup() const;
    static void FillSystemEnvData(SystemEnvData&, sal_IntPtr pWindow, QWidget* pWidget);
};

inline bool QtFrame::CallCallback(SalEvent nEvent, const void* pEvent) const
{
    SolarMutexGuard aGuard;
    return SalFrame::CallCallback(nEvent, pEvent);
}

inline bool QtFrame::isPopup() const
{
    return ((m_nStyle & SalFrameStyleFlags::FLOAT)
            && !(m_nStyle & SalFrameStyleFlags::OWNERDRAWDECORATION));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
