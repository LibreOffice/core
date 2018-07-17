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

#include <Qt5Frame.hxx>

#include <Qt5Tools.hxx>
#include <Qt5Instance.hxx>
#include <Qt5Graphics.hxx>
#include <Qt5Widget.hxx>
#include <Qt5Data.hxx>
#include <Qt5Menu.hxx>

#include <QtCore/QPoint>
#include <QtCore/QSize>
#include <QtGui/QIcon>
#include <QtGui/QWindow>
#include <QtGui/QScreen>
#include <QtWidgets/QApplication>

#include <saldatabasic.hxx>
#include <window.h>
#include <vcl/layout.hxx>
#include <vcl/syswin.hxx>

#include <cairo.h>
#include <headless/svpgdi.hxx>

static void SvpDamageHandler(void* handle, sal_Int32 nExtentsX, sal_Int32 nExtentsY,
                             sal_Int32 nExtentsWidth, sal_Int32 nExtentsHeight)
{
    Qt5Frame* pThis = static_cast<Qt5Frame*>(handle);
    pThis->Damage(nExtentsX, nExtentsY, nExtentsWidth, nExtentsHeight);
}

Qt5Frame::Qt5Frame(Qt5Frame* pParent, SalFrameStyleFlags nStyle, bool bUseCairo)
    : m_bUseCairo(bUseCairo)
    , m_bGraphicsInUse(false)
    , m_ePointerStyle(PointerStyle::Arrow)
    , m_bDefaultSize(true)
    , m_bDefaultPos(true)
{
    Qt5Instance* pInst = static_cast<Qt5Instance*>(GetSalData()->m_pInstance);
    pInst->insertFrame(this);

    m_aDamageHandler.handle = this;
    m_aDamageHandler.damaged = ::SvpDamageHandler;

    if (nStyle & SalFrameStyleFlags::DEFAULT) // ensure default style
    {
        nStyle |= SalFrameStyleFlags::MOVEABLE | SalFrameStyleFlags::SIZEABLE
                  | SalFrameStyleFlags::CLOSEABLE;
        nStyle &= ~SalFrameStyleFlags::FLOAT;
    }

    m_nStyle = nStyle;
    m_pParent = pParent;

    Qt::WindowFlags aWinFlags;
    if (!(nStyle & SalFrameStyleFlags::SYSTEMCHILD))
    {
        if (nStyle & SalFrameStyleFlags::INTRO)
            aWinFlags |= Qt::SplashScreen;
        else if (nStyle & (SalFrameStyleFlags::FLOAT | SalFrameStyleFlags::TOOLTIP))
            aWinFlags |= Qt::ToolTip;
        else if ((nStyle & SalFrameStyleFlags::FLOAT)
                 && !(nStyle & SalFrameStyleFlags::OWNERDRAWDECORATION))
            aWinFlags |= Qt::Popup;
        else if (nStyle & SalFrameStyleFlags::DIALOG && pParent)
            aWinFlags |= Qt::Dialog;
        else if (nStyle & SalFrameStyleFlags::TOOLWINDOW)
            aWinFlags |= Qt::Tool;
        else if (nStyle & SalFrameStyleFlags::OWNERDRAWDECORATION)
            aWinFlags |= Qt::Window | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus;
        else
            aWinFlags |= Qt::Window;
    }

    if (pParent)
        m_pQWidget.reset(createQt5Widget(*this, aWinFlags));
    else
        m_pQWidget.reset(createQMainWindow(*this, aWinFlags));

    if (pParent && !(pParent->m_nStyle & SalFrameStyleFlags::PLUG))
    {
        QWindow* pParentWindow = pParent->GetQWidget()->window()->windowHandle();
        QWindow* pChildWindow = m_pQWidget->window()->windowHandle();
        if (pParentWindow != pChildWindow)
            pChildWindow->setTransientParent(pParentWindow);
    }

    // fake an initial geometry, gets updated via configure event or SetPosSize
    if (m_bDefaultPos || m_bDefaultSize)
    {
        Size aDefSize = CalcDefaultSize();
        maGeometry.nX = -1;
        maGeometry.nY = -1;
        maGeometry.nWidth = aDefSize.Width();
        maGeometry.nHeight = aDefSize.Height();
        maGeometry.nTopDecoration = 0;
        maGeometry.nBottomDecoration = 0;
        maGeometry.nLeftDecoration = 0;
        maGeometry.nRightDecoration = 0;
    }
}

Qt5Frame::~Qt5Frame()
{
    Qt5Instance* pInst = static_cast<Qt5Instance*>(GetSalData()->m_pInstance);
    pInst->eraseFrame(this);
}

void Qt5Frame::Damage(sal_Int32 nExtentsX, sal_Int32 nExtentsY, sal_Int32 nExtentsWidth,
                      sal_Int32 nExtentsHeight) const
{
    m_pQWidget->update(nExtentsX, nExtentsY, nExtentsWidth, nExtentsHeight);
}

void Qt5Frame::TriggerPaintEvent()
{
    QSize aSize(m_pQWidget->size());
    SalPaintEvent aPaintEvt(0, 0, aSize.width(), aSize.height(), true);
    CallCallback(SalEvent::Paint, &aPaintEvt);
}

void Qt5Frame::TriggerPaintEvent(QRect aRect)
{
    SalPaintEvent aPaintEvt(aRect.x(), aRect.y(), aRect.width(), aRect.height(), true);
    CallCallback(SalEvent::Paint, &aPaintEvt);
}

void Qt5Frame::InitSvpSalGraphics(SvpSalGraphics* pSvpSalGraphics)
{
    int width = m_pQWidget->size().width();
    int height = m_pQWidget->size().height();
    m_pSvpGraphics = pSvpSalGraphics;
    m_pSurface.reset(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height));
    m_pSvpGraphics->setSurface(m_pSurface.get(), basegfx::B2IVector(width, height));
    cairo_surface_set_user_data(m_pSurface.get(), SvpSalGraphics::getDamageKey(), &m_aDamageHandler,
                                nullptr);
    TriggerPaintEvent();
}

SalGraphics* Qt5Frame::AcquireGraphics()
{
    if (m_bGraphicsInUse)
        return nullptr;

    m_bGraphicsInUse = true;

    if (m_bUseCairo)
    {
        if (!m_pOurSvpGraphics.get())
        {
            m_pOurSvpGraphics.reset(new SvpSalGraphics());
            InitSvpSalGraphics(m_pOurSvpGraphics.get());
        }
        return m_pOurSvpGraphics.get();
    }
    else
    {
        if (!m_pQt5Graphics.get())
        {
            m_pQt5Graphics.reset(new Qt5Graphics(this));
            m_pQImage.reset(new QImage(m_pQWidget->size(), Qt5_DefaultFormat32));
            m_pQt5Graphics->ChangeQImage(m_pQImage.get());
            TriggerPaintEvent();
        }
        return m_pQt5Graphics.get();
    }
}

void Qt5Frame::ReleaseGraphics(SalGraphics* pSalGraph)
{
    (void)pSalGraph;
    if (m_bUseCairo)
        assert(pSalGraph == m_pOurSvpGraphics.get());
    else
        assert(pSalGraph == m_pQt5Graphics.get());
    m_bGraphicsInUse = false;
}

bool Qt5Frame::PostEvent(ImplSVEvent* pData)
{
    Qt5Instance* pInst = static_cast<Qt5Instance*>(GetSalData()->m_pInstance);
    pInst->PostEvent(this, pData, SalEvent::UserEvent);
    return true;
}

void Qt5Frame::SetTitle(const OUString& rTitle)
{
    m_pQWidget->window()->setWindowTitle(toQString(rTitle));
}

void Qt5Frame::SetIcon(sal_uInt16 nIcon)
{
    if (m_nStyle
            & (SalFrameStyleFlags::PLUG | SalFrameStyleFlags::SYSTEMCHILD
               | SalFrameStyleFlags::FLOAT | SalFrameStyleFlags::INTRO
               | SalFrameStyleFlags::OWNERDRAWDECORATION)
        || !m_pQWidget->isWindow())
        return;

    const char* appicon;

    if (nIcon == SV_ICON_ID_TEXT)
        appicon = "libreoffice-writer";
    else if (nIcon == SV_ICON_ID_SPREADSHEET)
        appicon = "libreoffice-calc";
    else if (nIcon == SV_ICON_ID_DRAWING)
        appicon = "libreoffice-draw";
    else if (nIcon == SV_ICON_ID_PRESENTATION)
        appicon = "libreoffice-impress";
    else if (nIcon == SV_ICON_ID_DATABASE)
        appicon = "libreoffice-base";
    else if (nIcon == SV_ICON_ID_FORMULA)
        appicon = "libreoffice-math";
    else
        appicon = "libreoffice-startcenter";

    QIcon aIcon = QIcon::fromTheme(appicon);
    m_pQWidget->window()->setWindowIcon(aIcon);
}

void Qt5Frame::SetMenu(SalMenu* pMenu) { m_pSalMenu = static_cast<Qt5Menu*>(pMenu); }

void Qt5Frame::DrawMenuBar() {}

void Qt5Frame::SetExtendedFrameStyle(SalExtStyle /*nExtStyle*/) {}

void Qt5Frame::Show(bool bVisible, bool /*bNoActivate*/)
{
    assert(m_pQWidget.get());
    m_pQWidget->setVisible(bVisible);
}

void Qt5Frame::SetMinClientSize(long nWidth, long nHeight)
{
    if (!isChild())
        m_pQWidget->setMinimumSize(nWidth, nHeight);
}

void Qt5Frame::SetMaxClientSize(long nWidth, long nHeight)
{
    if (!isChild())
        m_pQWidget->setMaximumSize(nWidth, nHeight);
}

void Qt5Frame::Center()
{
    if (m_pParent)
    {
        QWidget* pWindow = m_pParent->GetQWidget()->window();
        m_pQWidget->move(pWindow->frameGeometry().topLeft() + pWindow->rect().center()
                         - m_pQWidget->rect().center());
    }
}

Size Qt5Frame::CalcDefaultSize()
{
    assert(m_pQWidget->isWindow());
    QScreen* pScreen = m_pQWidget->windowHandle()->screen();
    if (!pScreen)
        return Size();
    return bestmaxFrameSizeForScreenSize(toSize(pScreen->size()));
}

void Qt5Frame::SetDefaultSize()
{
    Size aDefSize = CalcDefaultSize();
    SetPosSize(0, 0, aDefSize.Width(), aDefSize.Height(),
               SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT);
}

void Qt5Frame::SetPosSize(long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags)
{
    if (!m_pQWidget->isWindow() || isChild(true, false))
        return;

    if ((nFlags & (SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT))
        && (nWidth > 0 && nHeight > 0) // sometimes stupid things happen
    )
    {
        m_bDefaultSize = false;
        if (isChild(false) || !m_pQWidget->isMaximized())
        {
            m_pQWidget->resize(nWidth, nHeight);
        }
    }
    else if (m_bDefaultSize)
        SetDefaultSize();

    m_bDefaultSize = false;

    if (nFlags & (SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y))
    {
        if (m_pParent)
        {
            QRect aRect = m_pParent->GetQWidget()->geometry();
            nX += aRect.x();
            nY += aRect.y();
        }

        maGeometry.nX = nX;
        maGeometry.nY = nY;

        m_bDefaultPos = false;
        m_pQWidget->move(nX, nY);
    }
    else if (m_bDefaultPos)
        Center();

    m_bDefaultPos = false;
}

void Qt5Frame::GetClientSize(long& rWidth, long& rHeight)
{
    rWidth = m_pQWidget->width();
    rHeight = m_pQWidget->height();
}

void Qt5Frame::GetWorkArea(tools::Rectangle& rRect)
{
    if (!m_pQWidget->isWindow())
        return;
    QScreen* pScreen = m_pQWidget->windowHandle()->screen();
    if (!pScreen)
        return;

    QSize aSize = pScreen->availableVirtualSize();
    rRect = tools::Rectangle(0, 0, aSize.width(), aSize.height());
}

SalFrame* Qt5Frame::GetParent() const { return m_pParent; }

void Qt5Frame::SetModal(bool bModal)
{
    if (m_pQWidget->isWindow())
    {
        // modality change is only effective if the window is hidden
        m_pQWidget->windowHandle()->hide();
        m_pQWidget->windowHandle()->setModality(bModal ? Qt::WindowModal : Qt::NonModal);
        // and shown again
        m_pQWidget->windowHandle()->show();
    }
}

void Qt5Frame::SetWindowState(const SalFrameState* pState)
{
    if (!m_pQWidget->isWindow() || !pState || isChild(true, false))
        return;

    const WindowStateMask nMaxGeometryMask
        = WindowStateMask::X | WindowStateMask::Y | WindowStateMask::Width | WindowStateMask::Height
          | WindowStateMask::MaximizedX | WindowStateMask::MaximizedY
          | WindowStateMask::MaximizedWidth | WindowStateMask::MaximizedHeight;

    if ((pState->mnMask & WindowStateMask::State) && (pState->mnState & WindowStateState::Maximized)
        && (pState->mnMask & nMaxGeometryMask) == nMaxGeometryMask)
        m_pQWidget->showMaximized();
    else if (pState->mnMask
             & (WindowStateMask::X | WindowStateMask::Y | WindowStateMask::Width
                | WindowStateMask::Height))
    {
        sal_uInt16 nPosSizeFlags = 0;
        QPoint aPos = m_pQWidget->pos();
        QPoint aParentPos;
        if (m_pParent)
            aParentPos = m_pParent->GetQWidget()->window()->pos();
        long nX = pState->mnX - aParentPos.x();
        long nY = pState->mnY - aParentPos.y();
        if (pState->mnMask & WindowStateMask::X)
            nPosSizeFlags |= SAL_FRAME_POSSIZE_X;
        else
            nX = aPos.x() - aParentPos.x();
        if (pState->mnMask & WindowStateMask::Y)
            nPosSizeFlags |= SAL_FRAME_POSSIZE_Y;
        else
            nY = aPos.y() - aParentPos.y();
        if (pState->mnMask & WindowStateMask::Width)
            nPosSizeFlags |= SAL_FRAME_POSSIZE_WIDTH;
        if (pState->mnMask & WindowStateMask::Height)
            nPosSizeFlags |= SAL_FRAME_POSSIZE_HEIGHT;
        SetPosSize(nX, nY, pState->mnWidth, pState->mnHeight, nPosSizeFlags);
    }
    else if (pState->mnMask & WindowStateMask::State && !isChild())
    {
        if ((pState->mnState & WindowStateState::Minimized) && m_pQWidget->isWindow())
            m_pQWidget->showMinimized();
        else
            m_pQWidget->showNormal();
    }
}

bool Qt5Frame::GetWindowState(SalFrameState* pState)
{
    pState->mnState = WindowStateState::Normal;
    pState->mnMask = WindowStateMask::State;
    if (m_pQWidget->isMinimized() || !m_pQWidget->windowHandle())
        pState->mnState |= WindowStateState::Minimized;
    else if (m_pQWidget->isMaximized())
    {
        pState->mnState |= WindowStateState::Maximized;
    }
    else
    {
        QRect rect = m_pQWidget->geometry();
        pState->mnX = rect.x();
        pState->mnY = rect.y();
        pState->mnWidth = rect.width();
        pState->mnHeight = rect.height();
        pState->mnMask |= WindowStateMask::X | WindowStateMask::Y | WindowStateMask::Width
                          | WindowStateMask::Height;
    }

    return true;
}

void Qt5Frame::ShowFullScreen(bool /*bFullScreen*/, sal_Int32 /*nDisplay*/) {}

void Qt5Frame::StartPresentation(bool /*bStart*/) {}

void Qt5Frame::SetAlwaysOnTop(bool /*bOnTop*/) {}

void Qt5Frame::ToTop(SalFrameToTop /*nFlags*/) {}

void Qt5Frame::SetPointer(PointerStyle ePointerStyle)
{
    QWindow* pWindow = m_pQWidget->window()->windowHandle();
    if (!pWindow)
        return;
    if (ePointerStyle == m_ePointerStyle)
        return;
    m_ePointerStyle = ePointerStyle;

    pWindow->setCursor(static_cast<Qt5Data*>(GetSalData())->getCursor(ePointerStyle));
}

void Qt5Frame::CaptureMouse(bool /*bMouse*/) {}

void Qt5Frame::SetPointerPos(long /*nX*/, long /*nY*/) {}

void Qt5Frame::Flush() {}

void Qt5Frame::Flush(const tools::Rectangle& /*rRect*/) {}

void Qt5Frame::SetInputContext(SalInputContext* /*pContext*/) {}

void Qt5Frame::EndExtTextInput(EndExtTextInputFlags /*nFlags*/) {}

OUString Qt5Frame::GetKeyName(sal_uInt16 /*nKeyCode*/) { return OUString(); }

bool Qt5Frame::MapUnicodeToKeyCode(sal_Unicode /*aUnicode*/, LanguageType /*aLangType*/,
                                   vcl::KeyCode& /*rKeyCode*/)
{
    return false;
}

LanguageType Qt5Frame::GetInputLanguage() { return LANGUAGE_DONTKNOW; }

void Qt5Frame::UpdateSettings(AllSettings& rSettings)
{
    StyleSettings style(rSettings.GetStyleSettings());

    const int flash_time = QApplication::cursorFlashTime();
    style.SetCursorBlinkTime(flash_time != 0 ? flash_time / 2 : STYLE_CURSOR_NOBLINKTIME);

    rSettings.SetStyleSettings(style);
}

void Qt5Frame::Beep() {}

const SystemEnvData* Qt5Frame::GetSystemData() const { return nullptr; }

SalFrame::SalPointerState Qt5Frame::GetPointerState()
{
    SalPointerState aState;
    QPoint pos = QCursor::pos();
    aState.maPos = Point(pos.x(), pos.y());
    aState.mnState = GetMouseModCode(QGuiApplication::mouseButtons())
                     | GetKeyModCode(QGuiApplication::keyboardModifiers());
    return aState;
}

KeyIndicatorState Qt5Frame::GetIndicatorState() { return KeyIndicatorState(); }

void Qt5Frame::SimulateKeyPress(sal_uInt16 /*nKeyCode*/) {}

void Qt5Frame::SetParent(SalFrame* pNewParent) { m_pParent = static_cast<Qt5Frame*>(pNewParent); }

bool Qt5Frame::SetPluginParent(SystemParentData* /*pNewParent*/) { return false; }

void Qt5Frame::ResetClipRegion() {}

void Qt5Frame::BeginSetClipRegion(sal_uLong /*nRects*/) {}

void Qt5Frame::UnionClipRegion(long /*nX*/, long /*nY*/, long /*nWidth*/, long /*nHeight*/) {}

void Qt5Frame::EndSetClipRegion() {}

void Qt5Frame::SetScreenNumber(unsigned int) {}

void Qt5Frame::SetApplicationID(const OUString&) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
