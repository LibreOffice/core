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
#include <Qt5Frame.moc>

#include <Qt5Tools.hxx>
#include <Qt5Instance.hxx>
#include <Qt5Graphics.hxx>
#include <Qt5Widget.hxx>
#include <Qt5MainWindow.hxx>
#include <Qt5Data.hxx>
#include <Qt5Menu.hxx>
#include <Qt5DragAndDrop.hxx>

#include <QtCore/QMimeData>
#include <QtCore/QPoint>
#include <QtCore/QSize>
#include <QtGui/QIcon>
#include <QtGui/QWindow>
#include <QtGui/QScreen>
#include <QtWidgets/QStyle>
#include <QtWidgets/QToolTip>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMainWindow>

#include <saldatabasic.hxx>
#include <window.h>
#include <vcl/layout.hxx>
#include <vcl/syswin.hxx>

#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>

#include <cairo.h>
#include <headless/svpgdi.hxx>

static void SvpDamageHandler(void* handle, sal_Int32 nExtentsX, sal_Int32 nExtentsY,
                             sal_Int32 nExtentsWidth, sal_Int32 nExtentsHeight)
{
    Qt5Frame* pThis = static_cast<Qt5Frame*>(handle);
    pThis->Damage(nExtentsX, nExtentsY, nExtentsWidth, nExtentsHeight);
}

Qt5Frame::Qt5Frame(Qt5Frame* pParent, SalFrameStyleFlags nStyle, bool bUseCairo)
    : m_pTopLevel(nullptr)
    , m_bUseCairo(bUseCairo)
    , m_pSvpGraphics(nullptr)
    , m_bNullRegion(true)
    , m_bGraphicsInUse(false)
    , m_ePointerStyle(PointerStyle::Arrow)
    , m_pDragSource(nullptr)
    , m_pDropTarget(nullptr)
    , m_bInDrag(false)
    , m_bDefaultSize(true)
    , m_bDefaultPos(true)
    , m_bFullScreen(false)
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
        // floating toolbars are frameless tool windows
        // + they must be able to receive keyboard focus
        else if ((nStyle & SalFrameStyleFlags::FLOAT)
                 && (nStyle & SalFrameStyleFlags::OWNERDRAWDECORATION))
            aWinFlags |= Qt::Tool | Qt::FramelessWindowHint;
        else if (nStyle & (SalFrameStyleFlags::FLOAT | SalFrameStyleFlags::TOOLTIP))
            aWinFlags |= Qt::ToolTip;
        else if ((nStyle & SalFrameStyleFlags::FLOAT)
                 && !(nStyle & SalFrameStyleFlags::OWNERDRAWDECORATION))
            aWinFlags |= Qt::Popup;
        else if (nStyle & SalFrameStyleFlags::DIALOG && pParent)
            aWinFlags |= Qt::Dialog;
        else if (nStyle & SalFrameStyleFlags::TOOLWINDOW)
            aWinFlags |= Qt::Tool;
        else
            aWinFlags |= Qt::Window;
    }

    if (aWinFlags == Qt::Window)
    {
        QWidget* pParentWidget = nullptr;
        if (m_pParent)
        {
            pParentWidget
                = (m_pParent->m_pTopLevel) ? m_pParent->m_pTopLevel : m_pParent->m_pQWidget;
        }

        m_pTopLevel = new Qt5MainWindow(*this, pParentWidget, aWinFlags);
        m_pQWidget = new Qt5Widget(*this, aWinFlags);
        m_pTopLevel->setCentralWidget(m_pQWidget);
    }
    else
        m_pQWidget = new Qt5Widget(*this, aWinFlags);

    connect(this, &Qt5Frame::tooltipRequest, static_cast<Qt5Widget*>(m_pQWidget),
            &Qt5Widget::showTooltip);

    if (pParent && !(pParent->m_nStyle & SalFrameStyleFlags::PLUG))
    {
        QWindow* pParentWindow = pParent->GetQWidget()->window()->windowHandle();
        QWindow* pChildWindow = (m_pTopLevel ? m_pTopLevel->window()->windowHandle()
                                             : m_pQWidget->window()->windowHandle());
        if (pParentWindow && pChildWindow && (pParentWindow != pChildWindow))
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

    m_aSystemData.nSize = sizeof(SystemEnvData);
    m_aSystemData.aWindow = m_pQWidget->winId();
    m_aSystemData.aShellWindow = reinterpret_cast<sal_IntPtr>(this);
    //m_aSystemData.pSalFrame = this;
    //m_aSystemData.pWidget = m_pQWidget;
    //m_aSystemData.nScreen = m_nXScreen.getXScreen();
    m_aSystemData.pToolkit = "qt5";

    SetIcon(SV_ICON_ID_OFFICE);
}

Qt5Frame::~Qt5Frame()
{
    Qt5Instance* pInst = static_cast<Qt5Instance*>(GetSalData()->m_pInstance);
    pInst->eraseFrame(this);
    if (m_pTopLevel)
        delete m_pTopLevel;
    else
        delete m_pQWidget;
    m_aSystemData.aShellWindow = 0;
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
    int width = 640;
    int height = 480;
    m_pSvpGraphics = pSvpSalGraphics;
    m_pSurface.reset(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height));
    m_pSvpGraphics->setSurface(m_pSurface.get(), basegfx::B2IVector(width, height));
    cairo_surface_set_user_data(m_pSurface.get(), SvpSalGraphics::getDamageKey(), &m_aDamageHandler,
                                nullptr);
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
            m_pQImage->fill(Qt::transparent);
            m_pQt5Graphics->ChangeQImage(m_pQImage.get());
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

bool Qt5Frame::PostEvent(std::unique_ptr<ImplSVEvent> pData)
{
    Qt5Instance* pInst = static_cast<Qt5Instance*>(GetSalData()->m_pInstance);
    pInst->PostEvent(this, pData.release(), SalEvent::UserEvent);
    return true;
}

bool Qt5Frame::isWindow() const
{
    if (m_pTopLevel)
        return m_pTopLevel->isWindow();
    else
        return m_pQWidget->isWindow();
}

QWindow* Qt5Frame::windowHandle() const
{
    if (m_pTopLevel)
        return m_pTopLevel->windowHandle();
    else
        return m_pQWidget->windowHandle();
}

QScreen* Qt5Frame::screen() const
{
    QWindow* const pWindow = windowHandle();
    if (pWindow)
        return pWindow->screen();
    else
        return nullptr;
}

bool Qt5Frame::isMinimized() const
{
    if (m_pTopLevel)
        return m_pTopLevel->isMinimized();
    else
        return m_pQWidget->isMinimized();
}

bool Qt5Frame::isMaximized() const
{
    if (m_pTopLevel)
        return m_pTopLevel->isMaximized();
    else
        return m_pQWidget->isMaximized();
}

void Qt5Frame::SetWindowStateImpl(Qt::WindowStates eState)
{
    if (m_pTopLevel)
        m_pTopLevel->setWindowState(eState);
    else
        m_pQWidget->setWindowState(eState);
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
        || !isWindow())
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

void Qt5Frame::DrawMenuBar() { /* not needed */}

void Qt5Frame::SetExtendedFrameStyle(SalExtStyle /*nExtStyle*/) { /* not needed */}

void Qt5Frame::setVisible(bool bVisible)
{
    if (m_pTopLevel)
        m_pTopLevel->setVisible(bVisible);
    else
        m_pQWidget->setVisible(bVisible);
}

void Qt5Frame::Show(bool bVisible, bool /*bNoActivate*/)
{
    assert(m_pQWidget);

    if (m_bDefaultSize)
        SetDefaultSize();

    auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
    assert(pSalInst);
    pSalInst->RunInMainThread([this, bVisible]() { setVisible(bVisible); });
}

void Qt5Frame::SetMinClientSize(long nWidth, long nHeight)
{
    if (!isChild())
    {
        if (m_pTopLevel)
            m_pTopLevel->setMinimumSize(nWidth, nHeight);
        else
            m_pQWidget->setMinimumSize(nWidth, nHeight);
    }
}

void Qt5Frame::SetMaxClientSize(long nWidth, long nHeight)
{
    if (!isChild())
    {
        if (m_pTopLevel)
            m_pTopLevel->setMaximumSize(nWidth, nHeight);
        else
            m_pQWidget->setMaximumSize(nWidth, nHeight);
    }
}

void Qt5Frame::Center()
{
    if (m_pParent)
    {
        QWidget* pWindow = m_pParent->GetQWidget()->window();
        QWidget* const pWidget = m_pTopLevel ? m_pTopLevel : m_pQWidget;
        pWidget->move(pWindow->frameGeometry().topLeft() + pWindow->rect().center()
                      - pWidget->rect().center());
    }
}

Size Qt5Frame::CalcDefaultSize()
{
    assert(isWindow());
    QSize qSize(0, 0);
    QScreen* pScreen = screen();
    if (pScreen)
        qSize = pScreen->size();
    else
        qSize = QApplication::desktop()->screenGeometry(0).size();

    Size aSize = toSize(qSize);
    if (!m_bFullScreen)
        aSize = bestmaxFrameSizeForScreenSize(aSize);

    return aSize;
}

void Qt5Frame::SetDefaultSize()
{
    Size aDefSize = CalcDefaultSize();
    SetPosSize(0, 0, aDefSize.Width(), aDefSize.Height(),
               SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT);
}

void Qt5Frame::SetPosSize(long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags)
{
    if (!isWindow() || isChild(true, false))
        return;

    if ((nFlags & (SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT))
        && (nWidth > 0 && nHeight > 0) // sometimes stupid things happen
    )
    {
        m_bDefaultSize = false;
        if (isChild(false) || !m_pQWidget->isMaximized())
        {
            QWidget* const pWidget = m_pTopLevel ? m_pTopLevel : m_pQWidget;

            if (m_nStyle & SalFrameStyleFlags::SIZEABLE)
                pWidget->resize(nWidth, nHeight);
            else
                pWidget->setFixedSize(nWidth, nHeight);
        }
    }
    else if (m_bDefaultSize)
        SetDefaultSize();

    m_bDefaultSize = false;

    if (nFlags & (SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y))
    {
        if (m_pParent)
        {
            QRect aRect;
            if (m_pParent->GetTopLevelWindow())
                aRect = m_pParent->GetTopLevelWindow()->geometry();
            else
                aRect = m_pParent->GetQWidget()->geometry();

            nX += aRect.x();
            nY += aRect.y();
        }

        maGeometry.nX = nX;
        maGeometry.nY = nY;

        m_bDefaultPos = false;
        if (m_pTopLevel)
            m_pTopLevel->move(nX, nY);
        else
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
    if (!isWindow())
        return;
    QScreen* pScreen = screen();
    if (!pScreen)
        return;

    QSize aSize = pScreen->availableVirtualSize();
    rRect = tools::Rectangle(0, 0, aSize.width(), aSize.height());
}

SalFrame* Qt5Frame::GetParent() const { return m_pParent; }

void Qt5Frame::SetModal(bool bModal)
{
    if (isWindow())
    {
        auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
        assert(pSalInst);
        pSalInst->RunInMainThread([this, bModal]() {
            bool wasVisible = windowHandle()->isVisible();

            // modality change is only effective if the window is hidden
            if (wasVisible)
            {
                windowHandle()->hide();
            }

            windowHandle()->setModality(bModal ? Qt::WindowModal : Qt::NonModal);

            // and shown again if it was visible
            if (wasVisible)
            {
                windowHandle()->show();
            }
        });
    }
}

bool Qt5Frame::GetModal() const { return isWindow() && windowHandle()->isModal(); }

void Qt5Frame::SetWindowState(const SalFrameState* pState)
{
    if (!isWindow() || !pState || isChild(true, false))
        return;

    const WindowStateMask nMaxGeometryMask
        = WindowStateMask::X | WindowStateMask::Y | WindowStateMask::Width | WindowStateMask::Height
          | WindowStateMask::MaximizedX | WindowStateMask::MaximizedY
          | WindowStateMask::MaximizedWidth | WindowStateMask::MaximizedHeight;

    if ((pState->mnMask & WindowStateMask::State) && (pState->mnState & WindowStateState::Maximized)
        && !isMaximized() && (pState->mnMask & nMaxGeometryMask) == nMaxGeometryMask)
    {
        if (m_pTopLevel)
        {
            m_pTopLevel->resize(pState->mnWidth, pState->mnHeight);
            m_pTopLevel->move(pState->mnX, pState->mnY);
        }
        else
        {
            m_pQWidget->resize(pState->mnWidth, pState->mnHeight);
            m_pQWidget->move(pState->mnX, pState->mnY);
        }
        SetWindowStateImpl(Qt::WindowMaximized);
    }
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
        if (pState->mnState & WindowStateState::Maximized)
            SetWindowStateImpl(Qt::WindowMaximized);
        else if (pState->mnState & WindowStateState::Minimized)
            SetWindowStateImpl(Qt::WindowMinimized);
        else
            SetWindowStateImpl(Qt::WindowNoState);
    }
}

bool Qt5Frame::GetWindowState(SalFrameState* pState)
{
    pState->mnState = WindowStateState::Normal;
    pState->mnMask = WindowStateMask::State;
    if (isMinimized() /*|| !windowHandle()*/)
        pState->mnState |= WindowStateState::Minimized;
    else if (isMaximized())
    {
        pState->mnState |= WindowStateState::Maximized;
    }
    else
    {
        QRect rect = m_pTopLevel ? m_pTopLevel->geometry() : m_pQWidget->geometry();
        pState->mnX = rect.x();
        pState->mnY = rect.y();
        pState->mnWidth = rect.width();
        pState->mnHeight = rect.height();
        pState->mnMask |= WindowStateMask::X | WindowStateMask::Y | WindowStateMask::Width
                          | WindowStateMask::Height;
    }

    return true;
}

void Qt5Frame::ShowFullScreen(bool bFullScreen, sal_Int32 nScreen)
{
    // only top-level windows can go fullscreen
    assert(m_pTopLevel);

    m_bFullScreen = bFullScreen;

    // show it if it isn't shown yet
    if (!isWindow())
        m_pTopLevel->show();

    if (m_bFullScreen)
    {
        m_aRestoreGeometry = m_pTopLevel->geometry();
        // do that before going fullscreen
        SetScreenNumber(nScreen);
        windowHandle()->showFullScreen();
    }
    else
    {
        windowHandle()->showNormal();
        m_pTopLevel->setGeometry(m_aRestoreGeometry);
        m_aRestoreGeometry = QRect();
    }
}

void Qt5Frame::StartPresentation(bool)
{
    // meh - so there's no Qt platform independent solution - defer to
    // KDE5 impl. For everyone else:
    // https://forum.qt.io/topic/38504/solved-qdialog-in-fullscreen-disable-os-screensaver
}

void Qt5Frame::SetAlwaysOnTop(bool bOnTop)
{
    QWidget* const pWidget = m_pTopLevel ? m_pTopLevel : m_pQWidget;
    const Qt::WindowFlags flags = pWidget->windowFlags();
    if (bOnTop)
        pWidget->setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
    else
        pWidget->setWindowFlags(flags & ~(Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint));
}

void Qt5Frame::ToTop(SalFrameToTop nFlags)
{
    QWidget* const pWidget = m_pTopLevel ? m_pTopLevel : m_pQWidget;

    if (isWindow() && !(nFlags & SalFrameToTop::GrabFocusOnly))
        pWidget->raise();
    if ((nFlags & SalFrameToTop::RestoreWhenMin) || (nFlags & SalFrameToTop::ForegroundTask))
        pWidget->activateWindow();
    else if ((nFlags & SalFrameToTop::GrabFocus) || (nFlags & SalFrameToTop::GrabFocusOnly))
        m_pQWidget->setFocus();
}

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

void Qt5Frame::CaptureMouse(bool bMouse)
{
    static const char* pEnv = getenv("SAL_NO_MOUSEGRABS");
    if (pEnv && *pEnv)
        return;

    if (bMouse)
        m_pQWidget->grabMouse();
    else
        m_pQWidget->releaseMouse();
}

void Qt5Frame::SetPointerPos(long nX, long nY)
{
    QCursor aCursor = m_pQWidget->cursor();
    QCursor::setPos(m_pQWidget->mapToGlobal(QPoint(nX, nY)));
    m_pQWidget->setCursor(aCursor);
}

void Qt5Frame::Flush()
{
    // was: QGuiApplication::sync();
    // but FIXME it causes too many issues, figure out sth better

    // unclear if we need to also flush cairo surface - gtk3 backend
    // does not do it. QPainter in Qt5Widget::paintEvent() is
    // destroyed, so that state should be safely flushed.
}

bool Qt5Frame::ShowTooltip(const OUString& rText, const tools::Rectangle& /*rHelpArea*/)
{
    emit tooltipRequest(rText);
    return true;
}

// do we even need it? void Qt5Frame::Flush(const tools::Rectangle& /*rRect*/) {}

void Qt5Frame::SetInputContext(SalInputContext* pContext)
{
    if (!pContext)
        return;

    if (!(pContext->mnOptions & InputContextFlags::Text))
        return;

    m_pQWidget->setAttribute(Qt::WA_InputMethodEnabled);
}

void Qt5Frame::EndExtTextInput(EndExtTextInputFlags /*nFlags*/)
{
    // TODO fwd to IM handler
}

OUString Qt5Frame::GetKeyName(sal_uInt16 nKeyCode)
{
    vcl::KeyCode vclKeyCode(nKeyCode);
    int nCode = vclKeyCode.GetCode();
    int nRetCode = 0;

    if (nCode >= KEY_0 && nCode <= KEY_9)
        nRetCode = (nCode - KEY_0) + Qt::Key_0;
    else if (nCode >= KEY_A && nCode <= KEY_Z)
        nRetCode = (nCode - KEY_A) + Qt::Key_A;
    else if (nCode >= KEY_F1 && nCode <= KEY_F26)
        nRetCode = (nCode - KEY_F1) + Qt::Key_F1;
    else
    {
        switch (nCode)
        {
            case KEY_DOWN:
                nRetCode = Qt::Key_Down;
                break;
            case KEY_UP:
                nRetCode = Qt::Key_Up;
                break;
            case KEY_LEFT:
                nRetCode = Qt::Key_Left;
                break;
            case KEY_RIGHT:
                nRetCode = Qt::Key_Right;
                break;
            case KEY_HOME:
                nRetCode = Qt::Key_Home;
                break;
            case KEY_END:
                nRetCode = Qt::Key_End;
                break;
            case KEY_PAGEUP:
                nRetCode = Qt::Key_PageUp;
                break;
            case KEY_PAGEDOWN:
                nRetCode = Qt::Key_PageDown;
                break;
            case KEY_RETURN:
                nRetCode = Qt::Key_Return;
                break;
            case KEY_ESCAPE:
                nRetCode = Qt::Key_Escape;
                break;
            case KEY_TAB:
                nRetCode = Qt::Key_Tab;
                break;
            case KEY_BACKSPACE:
                nRetCode = Qt::Key_Backspace;
                break;
            case KEY_SPACE:
                nRetCode = Qt::Key_Space;
                break;
            case KEY_INSERT:
                nRetCode = Qt::Key_Insert;
                break;
            case KEY_DELETE:
                nRetCode = Qt::Key_Delete;
                break;
            case KEY_ADD:
                nRetCode = Qt::Key_Plus;
                break;
            case KEY_SUBTRACT:
                nRetCode = Qt::Key_Minus;
                break;
            case KEY_MULTIPLY:
                nRetCode = Qt::Key_Asterisk;
                break;
            case KEY_DIVIDE:
                nRetCode = Qt::Key_Slash;
                break;
            case KEY_POINT:
                nRetCode = Qt::Key_Period;
                break;
            case KEY_COMMA:
                nRetCode = Qt::Key_Comma;
                break;
            case KEY_LESS:
                nRetCode = Qt::Key_Less;
                break;
            case KEY_GREATER:
                nRetCode = Qt::Key_Greater;
                break;
            case KEY_EQUAL:
                nRetCode = Qt::Key_Equal;
                break;
            case KEY_FIND:
                nRetCode = Qt::Key_Find;
                break;
            case KEY_CONTEXTMENU:
                nRetCode = Qt::Key_Menu;
                break;
            case KEY_HELP:
                nRetCode = Qt::Key_Help;
                break;
            case KEY_UNDO:
                nRetCode = Qt::Key_Undo;
                break;
            case KEY_REPEAT:
                nRetCode = Qt::Key_Redo;
                break;
            case KEY_TILDE:
                nRetCode = Qt::Key_AsciiTilde;
                break;
            case KEY_QUOTELEFT:
                nRetCode = Qt::Key_QuoteLeft;
                break;
            case KEY_BRACKETLEFT:
                nRetCode = Qt::Key_BracketLeft;
                break;
            case KEY_BRACKETRIGHT:
                nRetCode = Qt::Key_BracketRight;
                break;
            case KEY_SEMICOLON:
                nRetCode = Qt::Key_Semicolon;
                break;

            // Special cases
            case KEY_COPY:
                nRetCode = Qt::Key_Copy;
                break;
            case KEY_CUT:
                nRetCode = Qt::Key_Cut;
                break;
            case KEY_PASTE:
                nRetCode = Qt::Key_Paste;
                break;
            case KEY_OPEN:
                nRetCode = Qt::Key_Open;
                break;
        }
    }

    if (vclKeyCode.IsShift())
        nRetCode += Qt::SHIFT;
    if (vclKeyCode.IsMod1())
        nRetCode += Qt::CTRL;
    if (vclKeyCode.IsMod2())
        nRetCode += Qt::ALT;

    QKeySequence keySeq(nRetCode);
    OUString sKeyName = toOUString(keySeq.toString());

    return sKeyName;
}

bool Qt5Frame::MapUnicodeToKeyCode(sal_Unicode /*aUnicode*/, LanguageType /*aLangType*/,
                                   vcl::KeyCode& /*rKeyCode*/)
{
    // not supported yet
    return false;
}

LanguageType Qt5Frame::GetInputLanguage()
{
    // fallback
    return LANGUAGE_DONTKNOW;
}

static Color toColor(const QColor& rColor)
{
    return Color(rColor.red(), rColor.green(), rColor.blue());
}

void Qt5Frame::UpdateSettings(AllSettings& rSettings)
{
    if (Qt5Data::noNativeControls())
        return;

    StyleSettings style(rSettings.GetStyleSettings());

    // General settings
    QPalette pal = QApplication::palette();

    style.SetToolbarIconSize(ToolbarIconSize::Large);

    Color aFore = toColor(pal.color(QPalette::Active, QPalette::WindowText));
    Color aBack = toColor(pal.color(QPalette::Active, QPalette::Window));
    Color aText = toColor(pal.color(QPalette::Active, QPalette::Text));
    Color aBase = toColor(pal.color(QPalette::Active, QPalette::Base));
    Color aButn = toColor(pal.color(QPalette::Active, QPalette::ButtonText));
    Color aMid = toColor(pal.color(QPalette::Active, QPalette::Mid));
    Color aHigh = toColor(pal.color(QPalette::Active, QPalette::Highlight));
    Color aHighText = toColor(pal.color(QPalette::Active, QPalette::HighlightedText));

    style.SetSkipDisabledInMenus(true);

    // Foreground
    style.SetRadioCheckTextColor(aFore);
    style.SetLabelTextColor(aFore);
    style.SetDialogTextColor(aFore);
    style.SetGroupTextColor(aFore);

    // Text
    style.SetFieldTextColor(aText);
    style.SetFieldRolloverTextColor(aText);
    style.SetWindowTextColor(aText);
    style.SetToolTextColor(aText);

    // Base
    style.SetFieldColor(aBase);
    style.SetWindowColor(aBase);
    style.SetActiveTabColor(aBase);

    // Buttons
    style.SetButtonTextColor(aButn);
    style.SetButtonRolloverTextColor(aButn);
    style.SetButtonPressedRolloverTextColor(aButn);

    // Tabs
    style.SetTabTextColor(aButn);
    style.SetTabRolloverTextColor(aButn);
    style.SetTabHighlightTextColor(aButn);

    // Disable color
    style.SetDisableColor(toColor(pal.color(QPalette::Disabled, QPalette::WindowText)));

    // Background
    style.BatchSetBackgrounds(aBack);
    style.SetInactiveTabColor(aBack);

    // Workspace
    style.SetWorkspaceColor(aMid);

    // Selection
    style.SetHighlightColor(aHigh);
    style.SetHighlightTextColor(aHighText);

    // Tooltip
    style.SetHelpColor(toColor(QToolTip::palette().color(QPalette::Active, QPalette::ToolTipBase)));
    style.SetHelpTextColor(
        toColor(QToolTip::palette().color(QPalette::Active, QPalette::ToolTipText)));

    const int flash_time = QApplication::cursorFlashTime();
    style.SetCursorBlinkTime(flash_time != 0 ? flash_time / 2 : STYLE_CURSOR_NOBLINKTIME);

    // Menu
    std::unique_ptr<QMenuBar> pMenuBar = std::unique_ptr<QMenuBar>(new QMenuBar());
    QPalette qMenuCG = pMenuBar->palette();

    // Menu text and background color, theme specific
    Color aMenuFore = toColor(qMenuCG.color(QPalette::WindowText));
    Color aMenuBack = toColor(qMenuCG.color(QPalette::Window));

    style.SetMenuTextColor(aMenuFore);
    style.SetMenuBarTextColor(style.GetPersonaMenuBarTextColor().get_value_or(aMenuFore));
    style.SetMenuColor(aMenuBack);
    style.SetMenuBarColor(aMenuBack);
    style.SetMenuHighlightColor(toColor(qMenuCG.color(QPalette::Highlight)));
    style.SetMenuHighlightTextColor(toColor(qMenuCG.color(QPalette::HighlightedText)));

    // set special menubar highlight text color
    if (QApplication::style()->inherits("HighContrastStyle"))
        ImplGetSVData()->maNWFData.maMenuBarHighlightTextColor
            = toColor(qMenuCG.color(QPalette::HighlightedText));
    else
        ImplGetSVData()->maNWFData.maMenuBarHighlightTextColor = aMenuFore;

    // set menubar rollover color
    if (pMenuBar->style()->styleHint(QStyle::SH_MenuBar_MouseTracking))
    {
        style.SetMenuBarRolloverColor(toColor(qMenuCG.color(QPalette::Highlight)));
        style.SetMenuBarRolloverTextColor(ImplGetSVData()->maNWFData.maMenuBarHighlightTextColor);
    }
    else
    {
        style.SetMenuBarRolloverColor(aMenuBack);
        style.SetMenuBarRolloverTextColor(aMenuFore);
    }
    style.SetMenuBarHighlightTextColor(style.GetMenuHighlightTextColor());

    // Scroll bar size
    style.SetScrollBarSize(QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent));
    style.SetMinThumbSize(QApplication::style()->pixelMetric(QStyle::PM_ScrollBarSliderMin));

    // These colors are used for the ruler text and marks
    style.SetShadowColor(toColor(pal.color(QPalette::Disabled, QPalette::WindowText)));
    style.SetDarkShadowColor(toColor(pal.color(QPalette::Inactive, QPalette::WindowText)));

    rSettings.SetStyleSettings(style);
}

void Qt5Frame::Beep() { QApplication::beep(); }

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

void Qt5Frame::SimulateKeyPress(sal_uInt16 nKeyCode)
{
    SAL_WARN("vcl.kde5", "missing simulate keypress " << nKeyCode);
}

void Qt5Frame::SetParent(SalFrame* pNewParent) { m_pParent = static_cast<Qt5Frame*>(pNewParent); }

bool Qt5Frame::SetPluginParent(SystemParentData* /*pNewParent*/)
{
    //FIXME: no SetPluginParent impl. for kde5
    return false;
}

void Qt5Frame::ResetClipRegion() { m_bNullRegion = true; }

void Qt5Frame::BeginSetClipRegion(sal_uInt32)
{
    m_aRegion = QRegion(QRect(QPoint(0, 0), m_pQWidget->size()));
}

void Qt5Frame::UnionClipRegion(long nX, long nY, long nWidth, long nHeight)
{
    m_aRegion = m_aRegion.united(QRegion(nX, nY, nWidth, nHeight));
}

void Qt5Frame::EndSetClipRegion() { m_bNullRegion = false; }

void Qt5Frame::SetScreenNumber(unsigned int nScreen)
{
    if (isWindow())
    {
        QWindow* const pWindow = windowHandle();
        if (pWindow)
        {
            QList<QScreen*> screens = QApplication::screens();
            if (static_cast<int>(nScreen) < screens.size())
            {
                QWidget* const pWidget = m_pTopLevel ? m_pTopLevel : m_pQWidget;
                pWindow->setScreen(QApplication::screens()[nScreen]);

                // setScreen by itself has no effect, explicitly move the widget to
                // the new screen
                QRect screenGeo = QApplication::desktop()->screenGeometry(nScreen);
                pWidget->move(screenGeo.topLeft());
            }
            else
            {
                // index outta bounds, use primary screen
                QScreen* primaryScreen = QApplication::primaryScreen();
                pWindow->setScreen(primaryScreen);
            }
        }
    }
}

void Qt5Frame::SetApplicationID(const OUString&)
{
    // So the hope is that QGuiApplication deals with this properly..
}

// Drag'n'drop foo

Qt5DragSource* Qt5DragSource::m_ActiveDragSource;

void Qt5Frame::registerDragSource(Qt5DragSource* pDragSource)
{
    assert(!m_pDragSource);
    m_pDragSource = pDragSource;
}

void Qt5Frame::deregisterDragSource(Qt5DragSource const* pDragSource)
{
    assert(m_pDragSource == pDragSource);
    (void)pDragSource;
    m_pDragSource = nullptr;
}

void Qt5Frame::registerDropTarget(Qt5DropTarget* pDropTarget)
{
    assert(!m_pDropTarget);
    m_pDropTarget = pDropTarget;
}

void Qt5Frame::deregisterDropTarget(Qt5DropTarget const* pDropTarget)
{
    assert(m_pDropTarget == pDropTarget);
    (void)pDropTarget;
    m_pDropTarget = nullptr;
}

void Qt5Frame::draggingStarted(const int x, const int y, Qt::DropActions eActions,
                               const QMimeData* pQMimeData)
{
    assert(m_pDropTarget);

    css::datatransfer::dnd::DropTargetDragEnterEvent aEvent;
    aEvent.Source = static_cast<css::datatransfer::dnd::XDropTarget*>(m_pDropTarget);
    aEvent.Context = static_cast<css::datatransfer::dnd::XDropTargetDragContext*>(m_pDropTarget);
    aEvent.LocationX = x;
    aEvent.LocationY = y;
    aEvent.DropAction = getPreferredDropAction(eActions);
    aEvent.SourceActions = toVclDropActions(eActions);

    css::uno::Reference<css::datatransfer::XTransferable> xTransferable;
    if (!pQMimeData->hasFormat(sInternalMimeType))
        xTransferable = new Qt5DnDTransferable(pQMimeData);
    else
        xTransferable = Qt5DragSource::m_ActiveDragSource->GetTransferable();

    if (!m_bInDrag && xTransferable.is())
    {
        css::uno::Sequence<css::datatransfer::DataFlavor> aFormats
            = xTransferable->getTransferDataFlavors();
        aEvent.SupportedDataFlavors = aFormats;

        m_pDropTarget->fire_dragEnter(aEvent);
        m_bInDrag = true;
    }
    else
        m_pDropTarget->fire_dragOver(aEvent);
}

void Qt5Frame::dropping(const int x, const int y, const QMimeData* pQMimeData)
{
    assert(m_pDropTarget);

    css::datatransfer::dnd::DropTargetDropEvent aEvent;
    aEvent.Source = static_cast<css::datatransfer::dnd::XDropTarget*>(m_pDropTarget);
    aEvent.Context = static_cast<css::datatransfer::dnd::XDropTargetDropContext*>(m_pDropTarget);
    aEvent.LocationX = x;
    aEvent.LocationY = y;
    aEvent.DropAction = m_pDropTarget->proposedDragAction()
                        | css::datatransfer::dnd::DNDConstants::ACTION_DEFAULT;
    aEvent.SourceActions = css::datatransfer::dnd::DNDConstants::ACTION_MOVE;

    css::uno::Reference<css::datatransfer::XTransferable> xTransferable;
    if (!pQMimeData->hasFormat(sInternalMimeType))
        xTransferable = new Qt5DnDTransferable(pQMimeData);
    else
        xTransferable = Qt5DragSource::m_ActiveDragSource->GetTransferable();
    aEvent.Transferable = xTransferable;

    m_pDropTarget->fire_drop(aEvent);
    m_bInDrag = false;

    if (m_pDragSource)
    {
        m_pDragSource->fire_dragEnd(m_pDropTarget->proposedDragAction());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
