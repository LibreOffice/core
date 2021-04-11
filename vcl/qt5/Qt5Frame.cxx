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

#include <Qt5Data.hxx>
#include <Qt5DragAndDrop.hxx>
#include <Qt5Graphics.hxx>
#include <Qt5Instance.hxx>
#include <Qt5MainWindow.hxx>
#include <Qt5Menu.hxx>
#include <Qt5SvpGraphics.hxx>
#include <Qt5System.hxx>
#include <Qt5Tools.hxx>
#include <Qt5Transferable.hxx>
#include <Qt5Widget.hxx>

#include <QtCore/QMimeData>
#include <QtCore/QPoint>
#include <QtCore/QSize>
#include <QtCore/QThread>
#include <QtCore/QVersionNumber>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QIcon>
#include <QtGui/QWindow>
#include <QtGui/QScreen>
#include <QtWidgets/QStyle>
#include <QtWidgets/QToolTip>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMainWindow>

#if QT5_USING_X11
#include <QtX11Extras/QX11Info>
#include <xcb/xproto.h>
#if QT5_HAVE_XCB_ICCCM
#include <xcb/xcb_icccm.h>
#endif
#endif

#include <saldatabasic.hxx>
#include <window.h>
#include <vcl/syswin.hxx>

#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>

#include <cairo.h>
#include <headless/svpgdi.hxx>

#if QT5_USING_X11 && QT5_HAVE_XCB_ICCCM
static bool g_bNeedsWmHintsWindowGroup = true;
static xcb_atom_t g_aXcbClientLeaderAtom = 0;
#endif

static void SvpDamageHandler(void* handle, sal_Int32 nExtentsX, sal_Int32 nExtentsY,
                             sal_Int32 nExtentsWidth, sal_Int32 nExtentsHeight)
{
    Qt5Frame* pThis = static_cast<Qt5Frame*>(handle);
    pThis->Damage(nExtentsX, nExtentsY, nExtentsWidth, nExtentsHeight);
}

namespace
{
sal_Int32 screenNumber(const QScreen* pScreen)
{
    const QList<QScreen*> screens = QApplication::screens();

    sal_Int32 nScreen = 0;
    bool bFound = false;
    for (const QScreen* pCurScreen : screens)
    {
        if (pScreen == pCurScreen)
        {
            bFound = true;
            break;
        }
        nScreen++;
    }

    return bFound ? nScreen : -1;
}
}

Qt5Frame::Qt5Frame(Qt5Frame* pParent, SalFrameStyleFlags nStyle, bool bUseCairo)
    : m_pTopLevel(nullptr)
    , m_bUseCairo(bUseCairo)
    , m_bNullRegion(true)
    , m_bGraphicsInUse(false)
    , m_bGraphicsInvalid(false)
    , m_ePointerStyle(PointerStyle::Arrow)
    , m_pDragSource(nullptr)
    , m_pDropTarget(nullptr)
    , m_bInDrag(false)
    , m_bDefaultSize(true)
    , m_bDefaultPos(true)
    , m_bFullScreen(false)
    , m_bFullScreenSpanAll(false)
    , m_nInputLanguage(LANGUAGE_DONTKNOW)
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
        else if (nStyle & SalFrameStyleFlags::TOOLWINDOW)
            aWinFlags |= Qt::Tool;
        // top level windows can't be transient in Qt, so make them dialogs, if they have a parent. At least
        // the plasma shell relies on this setting to skip dialogs in the window list. And Qt Xcb will just
        // set transient for the types Dialog, Sheet, Tool, SplashScreen, ToolTip, Drawer and Popup.
        else if (nStyle & SalFrameStyleFlags::DIALOG || m_pParent)
            aWinFlags |= Qt::Dialog;
        else
            aWinFlags |= Qt::Window;
    }

    if (aWinFlags == Qt::Window)
    {
        m_pTopLevel = new Qt5MainWindow(*this, aWinFlags);
        m_pQWidget = new Qt5Widget(*this, aWinFlags);
        m_pTopLevel->setCentralWidget(m_pQWidget);
        m_pTopLevel->setFocusProxy(m_pQWidget);
    }
    else
        m_pQWidget = new Qt5Widget(*this, aWinFlags);

    if (pParent && !(pParent->m_nStyle & SalFrameStyleFlags::PLUG))
    {
        QWindow* pParentWindow = pParent->GetQWidget()->window()->windowHandle();
        QWindow* pChildWindow = asChild()->window()->windowHandle();
        if (pParentWindow && pChildWindow && (pParentWindow != pChildWindow))
            pChildWindow->setTransientParent(pParentWindow);
    }

    // Calling 'QWidget::winId()' implicitly enables native windows to be used
    // rather than "alien widgets" that are unknown to the windowing system,
    // s. https://doc.qt.io/qt-5/qwidget.html#native-widgets-vs-alien-widgets
    // Avoid this on Wayland due to problems with missing 'mouseMoveEvent's,
    // s. tdf#122293/QTBUG-75766
    const bool bWayland = QGuiApplication::platformName() == "wayland";
    if (!bWayland)
        m_aSystemData.SetWindowHandle(m_pQWidget->winId());
    else
    {
        // TODO implement as needed for Wayland,
        // s.a. commit c0d4f3ad3307c which did this for gtk3
        // QPlatformNativeInterface* native = QGuiApplication::platformNativeInterface();
        // m_aSystemData.pDisplay = native->nativeResourceForWindow("display", nullptr);
        // m_aSystemData.aWindow = reinterpret_cast<unsigned long>(
        //     native->nativeResourceForWindow("surface", m_pQWidget->windowHandle()));
    }

    m_aSystemData.aShellWindow = reinterpret_cast<sal_IntPtr>(this);
    //m_aSystemData.pSalFrame = this;
    m_aSystemData.pWidget = m_pQWidget;
    //m_aSystemData.nScreen = m_nXScreen.getXScreen();
    m_aSystemData.toolkit = SystemEnvData::Toolkit::Qt5;
    if (!bWayland)
        m_aSystemData.platform = SystemEnvData::Platform::Xcb;
    else
        m_aSystemData.platform = SystemEnvData::Platform::Wayland;

    SetIcon(SV_ICON_ID_OFFICE);

    fixICCCMwindowGroup();
}

void Qt5Frame::fixICCCMwindowGroup()
{
#if QT5_USING_X11 && QT5_HAVE_XCB_ICCCM
    // older Qt5 just sets WM_CLIENT_LEADER, but not the XCB_ICCCM_WM_HINT_WINDOW_GROUP
    // see Qt commit 0de4b326d8 ("xcb: fix issue with dialogs hidden by other windows")
    // or QTBUG-46626. So LO has to set this itself to help some WMs.
    if (!g_bNeedsWmHintsWindowGroup)
        return;
    g_bNeedsWmHintsWindowGroup = false;

    if (QGuiApplication::platformName() != "xcb")
        return;
    if (QVersionNumber::fromString(qVersion()) >= QVersionNumber(5, 12))
        return;

    xcb_connection_t* conn = QX11Info::connection();
    xcb_window_t win = asChild()->winId();

    xcb_icccm_wm_hints_t hints;

    xcb_get_property_cookie_t prop_cookie = xcb_icccm_get_wm_hints_unchecked(conn, win);
    if (!xcb_icccm_get_wm_hints_reply(conn, prop_cookie, &hints, nullptr))
        return;

    if (hints.flags & XCB_ICCCM_WM_HINT_WINDOW_GROUP)
        return;

    if (g_aXcbClientLeaderAtom == 0)
    {
        const char* const leader_name = "WM_CLIENT_LEADER\0";
        xcb_intern_atom_cookie_t atom_cookie
            = xcb_intern_atom(conn, 1, strlen(leader_name), leader_name);
        xcb_intern_atom_reply_t* atom_reply = xcb_intern_atom_reply(conn, atom_cookie, nullptr);
        if (!atom_reply)
            return;
        g_aXcbClientLeaderAtom = atom_reply->atom;
        free(atom_reply);
    }

    g_bNeedsWmHintsWindowGroup = true;

    prop_cookie = xcb_get_property(conn, 0, win, g_aXcbClientLeaderAtom, XCB_ATOM_WINDOW, 0, 1);
    xcb_get_property_reply_t* prop_reply = xcb_get_property_reply(conn, prop_cookie, nullptr);
    if (!prop_reply)
        return;

    if (xcb_get_property_value_length(prop_reply) != 4)
    {
        free(prop_reply);
        return;
    }

    xcb_window_t leader = *static_cast<xcb_window_t*>(xcb_get_property_value(prop_reply));
    free(prop_reply);

    hints.flags |= XCB_ICCCM_WM_HINT_WINDOW_GROUP;
    hints.window_group = leader;
    xcb_icccm_set_wm_hints(conn, win, &hints);
#else
    (void)this; // avoid loplugin:staticmethods
#endif
}

Qt5Frame::~Qt5Frame()
{
    Qt5Instance* pInst = static_cast<Qt5Instance*>(GetSalData()->m_pInstance);
    pInst->eraseFrame(this);
    delete asChild();
    m_aSystemData.aShellWindow = 0;
}

void Qt5Frame::Damage(sal_Int32 nExtentsX, sal_Int32 nExtentsY, sal_Int32 nExtentsWidth,
                      sal_Int32 nExtentsHeight) const
{
    m_pQWidget->update(scaledQRect(QRect(nExtentsX, nExtentsY, nExtentsWidth, nExtentsHeight),
                                   1 / devicePixelRatioF()));
}

SalGraphics* Qt5Frame::AcquireGraphics()
{
    if (m_bGraphicsInUse)
        return nullptr;

    m_bGraphicsInUse = true;

    if (m_bUseCairo)
    {
        if (!m_pSvpGraphics || m_bGraphicsInvalid)
        {
            QSize aSize = m_pQWidget->size() * devicePixelRatioF();
            m_pSvpGraphics.reset(new Qt5SvpGraphics(this));
            m_pSurface.reset(
                cairo_image_surface_create(CAIRO_FORMAT_ARGB32, aSize.width(), aSize.height()));
            m_pSvpGraphics->setSurface(m_pSurface.get(),
                                       basegfx::B2IVector(aSize.width(), aSize.height()));
            cairo_surface_set_user_data(m_pSurface.get(), Qt5SvpGraphics::getDamageKey(),
                                        &m_aDamageHandler, nullptr);
            m_bGraphicsInvalid = false;
        }
        return m_pSvpGraphics.get();
    }
    else
    {
        if (!m_pQt5Graphics || m_bGraphicsInvalid)
        {
            m_pQt5Graphics.reset(new Qt5Graphics(this));
            m_pQImage.reset(
                new QImage(m_pQWidget->size() * devicePixelRatioF(), Qt5_DefaultFormat32));
            m_pQImage->fill(Qt::transparent);
            m_pQt5Graphics->ChangeQImage(m_pQImage.get());
            m_bGraphicsInvalid = false;
        }
        return m_pQt5Graphics.get();
    }
}

void Qt5Frame::ReleaseGraphics(SalGraphics* pSalGraph)
{
    (void)pSalGraph;
    if (m_bUseCairo)
        assert(pSalGraph == m_pSvpGraphics.get());
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

QWidget* Qt5Frame::asChild() const { return m_pTopLevel ? m_pTopLevel : m_pQWidget; }

qreal Qt5Frame::devicePixelRatioF() const { return asChild()->devicePixelRatioF(); }

bool Qt5Frame::isWindow() const { return asChild()->isWindow(); }

QWindow* Qt5Frame::windowHandle() const
{
    // set attribute 'Qt::WA_NativeWindow' first to make sure a window handle actually exists
    QWidget* pChild = asChild();
    pChild->setAttribute(Qt::WA_NativeWindow);
    return pChild->windowHandle();
}

QScreen* Qt5Frame::screen() const
{
    QWindow* const pWindow = windowHandle();
    return pWindow ? pWindow->screen() : nullptr;
}

bool Qt5Frame::isMinimized() const { return asChild()->isMinimized(); }

bool Qt5Frame::isMaximized() const { return asChild()->isMaximized(); }

void Qt5Frame::SetWindowStateImpl(Qt::WindowStates eState)
{
    return asChild()->setWindowState(eState);
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

    QString appicon;

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

void Qt5Frame::SetMenu(SalMenu*) {}

void Qt5Frame::DrawMenuBar() { /* not needed */}

void Qt5Frame::SetExtendedFrameStyle(SalExtStyle /*nExtStyle*/) { /* not needed */}

void Qt5Frame::Show(bool bVisible, bool /*bNoActivate*/)
{
    assert(m_pQWidget);

    SetDefaultSize();
    SetDefaultPos();

    auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
    assert(pSalInst);
    pSalInst->RunInMainThread([this, bVisible]() { asChild()->setVisible(bVisible); });
}

void Qt5Frame::SetMinClientSize(tools::Long nWidth, tools::Long nHeight)
{
    if (!isChild())
    {
        const qreal fRatio = devicePixelRatioF();
        asChild()->setMinimumSize(round(nWidth / fRatio), round(nHeight / fRatio));
    }
}

void Qt5Frame::SetMaxClientSize(tools::Long nWidth, tools::Long nHeight)
{
    if (!isChild())
    {
        const qreal fRatio = devicePixelRatioF();
        asChild()->setMaximumSize(round(nWidth / fRatio), round(nHeight / fRatio));
    }
}

void Qt5Frame::SetDefaultPos()
{
    if (!m_bDefaultPos)
        return;

    // center on parent
    if (m_pParent)
    {
        const qreal fRatio = devicePixelRatioF();
        QWidget* const pWindow = m_pParent->GetQWidget()->window();
        QWidget* const pWidget = asChild();
        QPoint aPos = pWindow->rect().center() - pWidget->rect().center();
        SetPosSize(round(aPos.x() * fRatio), round(aPos.y() * fRatio), 0, 0,
                   SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y);
        assert(!m_bDefaultPos);
    }
    else
        m_bDefaultPos = false;
}

Size Qt5Frame::CalcDefaultSize()
{
    assert(isWindow());

    Size aSize;
    if (!m_bFullScreen)
    {
        const QScreen* pScreen = screen();
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        aSize = bestmaxFrameSizeForScreenSize(
            toSize(pScreen ? pScreen->size() : QApplication::desktop()->screenGeometry(0).size()));
        SAL_WNODEPRECATED_DECLARATIONS_POP
    }
    else
    {
        if (!m_bFullScreenSpanAll)
        {
            SAL_WNODEPRECATED_DECLARATIONS_PUSH
            aSize = toSize(
                QApplication::desktop()->screenGeometry(maGeometry.nDisplayScreenNumber).size());
            SAL_WNODEPRECATED_DECLARATIONS_POP
        }
        else
        {
            SAL_WNODEPRECATED_DECLARATIONS_PUSH
            int nLeftScreen = QApplication::desktop()->screenNumber(QPoint(0, 0));
            SAL_WNODEPRECATED_DECLARATIONS_POP
            aSize = toSize(QApplication::screens()[nLeftScreen]->availableVirtualGeometry().size());
        }
    }

    return aSize;
}

void Qt5Frame::SetDefaultSize()
{
    if (!m_bDefaultSize)
        return;

    Size aDefSize = CalcDefaultSize();
    SetPosSize(0, 0, aDefSize.Width(), aDefSize.Height(),
               SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT);
    assert(!m_bDefaultSize);
}

void Qt5Frame::SetPosSize(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight,
                          sal_uInt16 nFlags)
{
    if (!isWindow() || isChild(true, false))
        return;

    if (nFlags & (SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT))
    {
        if (isChild(false) || !m_pQWidget->isMaximized())
        {
            if (!(nFlags & SAL_FRAME_POSSIZE_WIDTH))
                nWidth = maGeometry.nWidth;
            else if (!(nFlags & SAL_FRAME_POSSIZE_HEIGHT))
                nHeight = maGeometry.nHeight;

            if (nWidth > 0 && nHeight > 0)
            {
                m_bDefaultSize = false;
                const int nNewWidth = round(nWidth / devicePixelRatioF());
                const int nNewHeight = round(nHeight / devicePixelRatioF());
                if (m_nStyle & SalFrameStyleFlags::SIZEABLE)
                    asChild()->resize(nNewWidth, nNewHeight);
                else
                    asChild()->setFixedSize(nNewWidth, nNewHeight);
            }

            // assume the resize happened
            // needed for calculations and will eventually be corrected by events
            if (nWidth > 0)
                maGeometry.nWidth = nWidth;
            if (nHeight > 0)
                maGeometry.nHeight = nHeight;
        }
    }

    if (!(nFlags & (SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y)))
        return;

    if (m_pParent)
    {
        const SalFrameGeometry& aParentGeometry = m_pParent->maGeometry;
        if (QGuiApplication::isRightToLeft())
            nX = aParentGeometry.nX + aParentGeometry.nWidth - nX - maGeometry.nWidth - 1;
        else
            nX += aParentGeometry.nX;
        nY += aParentGeometry.nY;

        Qt5MainWindow* pTopLevel = m_pParent->GetTopLevelWindow();
        if (pTopLevel && pTopLevel->menuBar() && pTopLevel->menuBar()->isVisible())
            nY += round(pTopLevel->menuBar()->geometry().height() * devicePixelRatioF());
    }

    if (!(nFlags & SAL_FRAME_POSSIZE_X))
        nX = maGeometry.nX;
    else if (!(nFlags & SAL_FRAME_POSSIZE_Y))
        nY = maGeometry.nY;

    // assume the reposition happened
    // needed for calculations and will eventually be corrected by events later
    maGeometry.nX = nX;
    maGeometry.nY = nY;

    m_bDefaultPos = false;
    asChild()->move(round(nX / devicePixelRatioF()), round(nY / devicePixelRatioF()));
}

void Qt5Frame::GetClientSize(tools::Long& rWidth, tools::Long& rHeight)
{
    rWidth = round(m_pQWidget->width() * devicePixelRatioF());
    rHeight = round(m_pQWidget->height() * devicePixelRatioF());
}

void Qt5Frame::GetWorkArea(tools::Rectangle& rRect)
{
    if (!isWindow())
        return;
    QScreen* pScreen = screen();
    if (!pScreen)
        return;

    QSize aSize = pScreen->availableVirtualSize() * devicePixelRatioF();
    rRect = tools::Rectangle(0, 0, aSize.width(), aSize.height());
}

SalFrame* Qt5Frame::GetParent() const { return m_pParent; }

void Qt5Frame::SetModal(bool bModal)
{
    if (!isWindow())
        return;

    auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
    assert(pSalInst);
    pSalInst->RunInMainThread([this, bModal]() {

        QWidget* const pChild = asChild();
        const bool bWasVisible = pChild->isVisible();

        // modality change is only effective if the window is hidden
        if (bWasVisible)
            pChild->hide();

        pChild->setWindowModality(bModal ? Qt::WindowModal : Qt::NonModal);

        if (bWasVisible)
            pChild->show();
    });
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
        const qreal fRatio = devicePixelRatioF();
        QWidget* const pChild = asChild();
        pChild->resize(ceil(pState->mnWidth / fRatio), ceil(pState->mnHeight / fRatio));
        pChild->move(ceil(pState->mnX / fRatio), ceil(pState->mnY / fRatio));
        SetWindowStateImpl(Qt::WindowMaximized);
    }
    else if (pState->mnMask
             & (WindowStateMask::X | WindowStateMask::Y | WindowStateMask::Width
                | WindowStateMask::Height))
    {
        sal_uInt16 nPosSizeFlags = 0;
        if (pState->mnMask & WindowStateMask::X)
            nPosSizeFlags |= SAL_FRAME_POSSIZE_X;
        if (pState->mnMask & WindowStateMask::Y)
            nPosSizeFlags |= SAL_FRAME_POSSIZE_Y;
        if (pState->mnMask & WindowStateMask::Width)
            nPosSizeFlags |= SAL_FRAME_POSSIZE_WIDTH;
        if (pState->mnMask & WindowStateMask::Height)
            nPosSizeFlags |= SAL_FRAME_POSSIZE_HEIGHT;
        SetPosSize(pState->mnX, pState->mnY, pState->mnWidth, pState->mnHeight, nPosSizeFlags);
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
        // geometry() is the drawable area, which is wanted here
        QRect rect = scaledQRect(asChild()->geometry(), devicePixelRatioF());
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

    if (m_bFullScreen == bFullScreen)
        return;

    m_bFullScreen = bFullScreen;
    m_bFullScreenSpanAll = m_bFullScreen && (nScreen < 0);

    // show it if it isn't shown yet
    if (!isWindow())
        m_pTopLevel->show();

    if (m_bFullScreen)
    {
        m_aRestoreGeometry = m_pTopLevel->geometry();
        m_nRestoreScreen = maGeometry.nDisplayScreenNumber;
        SetScreenNumber(m_bFullScreenSpanAll ? m_nRestoreScreen : nScreen);
        if (!m_bFullScreenSpanAll)
            windowHandle()->showFullScreen();
        else
            windowHandle()->showNormal();
    }
    else
    {
        SetScreenNumber(m_nRestoreScreen);
        windowHandle()->showNormal();
        m_pTopLevel->setGeometry(m_aRestoreGeometry);
    }
}

void Qt5Frame::StartPresentation(bool bStart)
{
// meh - so there's no Qt platform independent solution
// https://forum.qt.io/topic/38504/solved-qdialog-in-fullscreen-disable-os-screensaver
#if QT5_USING_X11
    std::optional<unsigned int> aRootWindow;
    std::optional<Display*> aDisplay;

    if (QX11Info::isPlatformX11())
    {
        aRootWindow = QX11Info::appRootWindow();
        aDisplay = QX11Info::display();
    }

    m_ScreenSaverInhibitor.inhibit(bStart, u"presentation", QX11Info::isPlatformX11(), aRootWindow,
                                   aDisplay);
#else
    (void)bStart;
#endif
}

void Qt5Frame::SetAlwaysOnTop(bool bOnTop)
{
    QWidget* const pWidget = asChild();
    const Qt::WindowFlags flags = pWidget->windowFlags();
    if (bOnTop)
        pWidget->setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
    else
        pWidget->setWindowFlags(flags & ~(Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint));
}

void Qt5Frame::ToTop(SalFrameToTop nFlags)
{
    QWidget* const pWidget = asChild();
    if (isWindow() && !(nFlags & SalFrameToTop::GrabFocusOnly))
        pWidget->raise();
    if ((nFlags & SalFrameToTop::RestoreWhenMin) || (nFlags & SalFrameToTop::ForegroundTask))
        pWidget->activateWindow();
    else if ((nFlags & SalFrameToTop::GrabFocus) || (nFlags & SalFrameToTop::GrabFocusOnly))
    {
        pWidget->activateWindow();
        pWidget->setFocus();
    }
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

void Qt5Frame::SetPointerPos(tools::Long nX, tools::Long nY)
{
    // some cursor already exists (and it has m_ePointerStyle shape)
    // so here we just reposition it
    QCursor::setPos(m_pQWidget->mapToGlobal(QPoint(nX, nY)));
}

void Qt5Frame::Flush()
{
    // was: QGuiApplication::sync();
    // but FIXME it causes too many issues, figure out sth better

    // unclear if we need to also flush cairo surface - gtk3 backend
    // does not do it. QPainter in Qt5Widget::paintEvent() is
    // destroyed, so that state should be safely flushed.
}

bool Qt5Frame::ShowTooltip(const OUString& rText, const tools::Rectangle& rHelpArea)
{
    QRect aHelpArea(toQRect(rHelpArea));
    if (QGuiApplication::isRightToLeft())
        aHelpArea.moveLeft(maGeometry.nWidth - aHelpArea.width() - aHelpArea.left() - 1);
    QToolTip::showText(QCursor::pos(), toQString(rText), m_pQWidget, aHelpArea);
    return true;
}

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
    Qt5Widget* pQt5Widget = static_cast<Qt5Widget*>(m_pQWidget);
    if (pQt5Widget)
        pQt5Widget->endExtTextInput();
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

LanguageType Qt5Frame::GetInputLanguage() { return m_nInputLanguage; }

void Qt5Frame::setInputLanguage(LanguageType nInputLanguage)
{
    if (nInputLanguage == m_nInputLanguage)
        return;
    m_nInputLanguage = nInputLanguage;
    CallCallback(SalEvent::InputLanguageChange, nullptr);
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
    Color aLink = toColor(pal.color(QPalette::Active, QPalette::Link));
    Color aVisitedLink = toColor(pal.color(QPalette::Active, QPalette::LinkVisited));

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
    style.SetAlternatingRowColor(toColor(pal.color(QPalette::Active, QPalette::AlternateBase)));

    // Buttons
    style.SetDefaultButtonTextColor(aButn);
    style.SetButtonTextColor(aButn);
    style.SetDefaultActionButtonTextColor(aButn);
    style.SetActionButtonTextColor(aButn);
    style.SetFlatButtonTextColor(aButn);
    style.SetDefaultButtonRolloverTextColor(aButn);
    style.SetButtonRolloverTextColor(aButn);
    style.SetDefaultActionButtonRolloverTextColor(aButn);
    style.SetActionButtonRolloverTextColor(aButn);
    style.SetFlatButtonRolloverTextColor(aButn);
    style.SetDefaultButtonPressedRolloverTextColor(aButn);
    style.SetButtonPressedRolloverTextColor(aButn);
    style.SetDefaultActionButtonPressedRolloverTextColor(aButn);
    style.SetActionButtonPressedRolloverTextColor(aButn);
    style.SetFlatButtonPressedRolloverTextColor(aButn);

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
    style.SetActiveColor(aHigh);
    style.SetActiveTextColor(aHighText);

    // Links
    style.SetLinkColor(aLink);
    style.SetVisitedLinkColor(aVisitedLink);

    // Tooltip
    style.SetHelpColor(toColor(QToolTip::palette().color(QPalette::Active, QPalette::ToolTipBase)));
    style.SetHelpTextColor(
        toColor(QToolTip::palette().color(QPalette::Active, QPalette::ToolTipText)));

    const int flash_time = QApplication::cursorFlashTime();
    style.SetCursorBlinkTime(flash_time != 0 ? flash_time / 2 : STYLE_CURSOR_NOBLINKTIME);

    // Menu
    std::unique_ptr<QMenuBar> pMenuBar = std::make_unique<QMenuBar>();
    QPalette qMenuCG = pMenuBar->palette();

    // Menu text and background color, theme specific
    Color aMenuFore = toColor(qMenuCG.color(QPalette::WindowText));
    Color aMenuBack = toColor(qMenuCG.color(QPalette::Window));

    style.SetMenuTextColor(aMenuFore);
    style.SetMenuBarTextColor(style.GetPersonaMenuBarTextColor().value_or(aMenuFore));
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

    // Icon theme
    style.SetPreferredIconTheme(toOUString(QIcon::themeName()));

    // Scroll bar size
    style.SetScrollBarSize(QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent));
    style.SetMinThumbSize(QApplication::style()->pixelMetric(QStyle::PM_ScrollBarSliderMin));

    // These colors are used for the ruler text and marks
    style.SetShadowColor(toColor(pal.color(QPalette::Disabled, QPalette::WindowText)));
    style.SetDarkShadowColor(toColor(pal.color(QPalette::Inactive, QPalette::WindowText)));

    m_bGraphicsInvalid = true;
    rSettings.SetStyleSettings(style);
}

void Qt5Frame::Beep() { QApplication::beep(); }

SalFrame::SalPointerState Qt5Frame::GetPointerState()
{
    SalPointerState aState;
    aState.maPos = toPoint(QCursor::pos() * devicePixelRatioF());
    aState.maPos.Move(-maGeometry.nX, -maGeometry.nY);
    aState.mnState = GetMouseModCode(QGuiApplication::mouseButtons())
                     | GetKeyModCode(QGuiApplication::keyboardModifiers());
    return aState;
}

KeyIndicatorState Qt5Frame::GetIndicatorState() { return KeyIndicatorState(); }

void Qt5Frame::SimulateKeyPress(sal_uInt16 nKeyCode)
{
    SAL_WARN("vcl.qt5", "missing simulate keypress " << nKeyCode);
}

void Qt5Frame::SetParent(SalFrame* pNewParent) { m_pParent = static_cast<Qt5Frame*>(pNewParent); }

bool Qt5Frame::SetPluginParent(SystemParentData* /*pNewParent*/)
{
    //FIXME: no SetPluginParent impl. for qt5
    return false;
}

void Qt5Frame::ResetClipRegion() { m_bNullRegion = true; }

void Qt5Frame::BeginSetClipRegion(sal_uInt32)
{
    m_aRegion = QRegion(QRect(QPoint(0, 0), m_pQWidget->size()));
}

void Qt5Frame::UnionClipRegion(tools::Long nX, tools::Long nY, tools::Long nWidth,
                               tools::Long nHeight)
{
    m_aRegion
        = m_aRegion.united(scaledQRect(QRect(nX, nY, nWidth, nHeight), 1 / devicePixelRatioF()));
}

void Qt5Frame::EndSetClipRegion() { m_bNullRegion = false; }

void Qt5Frame::SetScreenNumber(unsigned int nScreen)
{
    if (!isWindow())
        return;

    QWindow* const pWindow = windowHandle();
    if (!pWindow)
        return;

    QList<QScreen*> screens = QApplication::screens();
    if (static_cast<int>(nScreen) < screens.size() || m_bFullScreenSpanAll)
    {
        QRect screenGeo;

        if (!m_bFullScreenSpanAll)
        {
            SAL_WNODEPRECATED_DECLARATIONS_PUSH
            screenGeo = QApplication::desktop()->screenGeometry(nScreen);
            SAL_WNODEPRECATED_DECLARATIONS_POP
            pWindow->setScreen(QApplication::screens()[nScreen]);
        }
        else // special case: fullscreen over all available screens
        {
            assert(m_bFullScreen);
            // left-most screen
            SAL_WNODEPRECATED_DECLARATIONS_PUSH
            int nLeftScreen = QApplication::desktop()->screenNumber(QPoint(0, 0));
            SAL_WNODEPRECATED_DECLARATIONS_POP
            // entire virtual desktop
            screenGeo = QApplication::screens()[nLeftScreen]->availableVirtualGeometry();
            pWindow->setScreen(QApplication::screens()[nLeftScreen]);
            pWindow->setGeometry(screenGeo);
            nScreen = nLeftScreen;
        }

        // setScreen by itself has no effect, explicitly move the widget to
        // the new screen
        asChild()->move(screenGeo.topLeft());
    }
    else
    {
        // index outta bounds, use primary screen
        QScreen* primaryScreen = QApplication::primaryScreen();
        pWindow->setScreen(primaryScreen);
        nScreen = static_cast<sal_uInt32>(screenNumber(primaryScreen));
    }

    maGeometry.nDisplayScreenNumber = nScreen;
}

void Qt5Frame::SetApplicationID(const OUString& rWMClass)
{
#if QT5_USING_X11
    if (QGuiApplication::platformName() != "xcb" || !m_pTopLevel)
        return;

    OString aResClass = OUStringToOString(rWMClass, RTL_TEXTENCODING_ASCII_US);
    const char* pResClass
        = !aResClass.isEmpty() ? aResClass.getStr() : SalGenericSystem::getFrameClassName();
    OString aResName = SalGenericSystem::getFrameResName();

    // the WM_CLASS data consists of two concatenated cstrings, including the terminating '\0' chars
    const uint32_t data_len = aResName.getLength() + 1 + strlen(pResClass) + 1;
    char* data = new char[data_len];
    memcpy(data, aResName.getStr(), aResName.getLength() + 1);
    memcpy(data + aResName.getLength() + 1, pResClass, strlen(pResClass) + 1);

    xcb_change_property(QX11Info::connection(), XCB_PROP_MODE_REPLACE, m_pTopLevel->winId(),
                        XCB_ATOM_WM_CLASS, XCB_ATOM_STRING, 8, data_len, data);
    delete[] data;
#else
    (void)rWMClass;
#endif
}

// Drag'n'drop foo

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
    m_pQWidget->setAcceptDrops(true);
}

void Qt5Frame::deregisterDropTarget(Qt5DropTarget const* pDropTarget)
{
    assert(m_pDropTarget == pDropTarget);
    (void)pDropTarget;
    m_pDropTarget = nullptr;
}

static css::uno::Reference<css::datatransfer::XTransferable>
lcl_getXTransferable(const QMimeData* pMimeData)
{
    css::uno::Reference<css::datatransfer::XTransferable> xTransferable;
    const Qt5MimeData* pQt5MimeData = dynamic_cast<const Qt5MimeData*>(pMimeData);
    if (!pQt5MimeData)
        xTransferable = new Qt5DnDTransferable(pMimeData);
    else
        xTransferable = pQt5MimeData->xTransferable();
    return xTransferable;
}

static sal_Int8 lcl_getUserDropAction(const QDropEvent* pEvent, const sal_Int8 nSourceActions,
                                      const QMimeData* pMimeData)
{
    // we completely ignore all proposals by the Qt event, as they don't
    // match at all with the preferred LO DnD actions.

    // check the key modifiers to detect a user-overridden DnD action
    const Qt::KeyboardModifiers eKeyMod = pEvent->keyboardModifiers();
    sal_Int8 nUserDropAction = 0;
    if ((eKeyMod & Qt::ShiftModifier) && !(eKeyMod & Qt::ControlModifier))
        nUserDropAction = css::datatransfer::dnd::DNDConstants::ACTION_MOVE;
    else if ((eKeyMod & Qt::ControlModifier) && !(eKeyMod & Qt::ShiftModifier))
        nUserDropAction = css::datatransfer::dnd::DNDConstants::ACTION_COPY;
    else if ((eKeyMod & Qt::ShiftModifier) && (eKeyMod & Qt::ControlModifier))
        nUserDropAction = css::datatransfer::dnd::DNDConstants::ACTION_LINK;
    nUserDropAction &= nSourceActions;

    // select the default DnD action, if there isn't a user preference
    if (0 == nUserDropAction)
    {
        // default LO internal action is move, but default external action is copy
        nUserDropAction = dynamic_cast<const Qt5MimeData*>(pMimeData)
                              ? css::datatransfer::dnd::DNDConstants::ACTION_MOVE
                              : css::datatransfer::dnd::DNDConstants::ACTION_COPY;
        nUserDropAction &= nSourceActions;

        // if the default doesn't match any allowed source action, fall back to the
        // preferred of all allowed source actions
        if (0 == nUserDropAction)
            nUserDropAction = toVclDropAction(getPreferredDropAction(nSourceActions));

        // this is "our" preference, but actually we would even prefer any default,
        // if there is any
        nUserDropAction |= css::datatransfer::dnd::DNDConstants::ACTION_DEFAULT;
    }
    return nUserDropAction;
}

void Qt5Frame::handleDragMove(QDragMoveEvent* pEvent)
{
    assert(m_pDropTarget);

    // prepare our suggested drop action for the drop target
    const sal_Int8 nSourceActions = toVclDropActions(pEvent->possibleActions());
    const QMimeData* pMimeData = pEvent->mimeData();
    const sal_Int8 nUserDropAction = lcl_getUserDropAction(pEvent, nSourceActions, pMimeData);
    const Point aPos = toPoint(pEvent->pos() * devicePixelRatioF());

    css::datatransfer::dnd::DropTargetDragEnterEvent aEvent;
    aEvent.Source = static_cast<css::datatransfer::dnd::XDropTarget*>(m_pDropTarget);
    aEvent.Context = static_cast<css::datatransfer::dnd::XDropTargetDragContext*>(m_pDropTarget);
    aEvent.LocationX = aPos.X();
    aEvent.LocationY = aPos.Y();
    aEvent.DropAction = nUserDropAction;
    aEvent.SourceActions = nSourceActions;

    // ask the drop target to accept our drop action
    if (!m_bInDrag)
    {
        aEvent.SupportedDataFlavors = lcl_getXTransferable(pMimeData)->getTransferDataFlavors();
        m_pDropTarget->fire_dragEnter(aEvent);
        m_bInDrag = true;
    }
    else
        m_pDropTarget->fire_dragOver(aEvent);

    // the drop target accepted our drop action => inform Qt
    if (m_pDropTarget->proposedDropAction() != 0)
    {
        pEvent->setDropAction(getPreferredDropAction(m_pDropTarget->proposedDropAction()));
        pEvent->accept();
    }
    else // or maybe someone else likes it?
        pEvent->ignore();
}

void Qt5Frame::handleDrop(QDropEvent* pEvent)
{
    assert(m_pDropTarget);

    // prepare our suggested drop action for the drop target
    const sal_Int8 nSourceActions = toVclDropActions(pEvent->possibleActions());
    const sal_Int8 nUserDropAction
        = lcl_getUserDropAction(pEvent, nSourceActions, pEvent->mimeData());
    const Point aPos = toPoint(pEvent->pos() * devicePixelRatioF());

    css::datatransfer::dnd::DropTargetDropEvent aEvent;
    aEvent.Source = static_cast<css::datatransfer::dnd::XDropTarget*>(m_pDropTarget);
    aEvent.Context = static_cast<css::datatransfer::dnd::XDropTargetDropContext*>(m_pDropTarget);
    aEvent.LocationX = aPos.X();
    aEvent.LocationY = aPos.Y();
    aEvent.SourceActions = nSourceActions;
    aEvent.DropAction = nUserDropAction;
    aEvent.Transferable = lcl_getXTransferable(pEvent->mimeData());

    // ask the drop target to accept our drop action
    m_pDropTarget->fire_drop(aEvent);
    m_bInDrag = false;

    const bool bDropSuccessful = m_pDropTarget->dropSuccessful();
    const sal_Int8 nDropAction = m_pDropTarget->proposedDropAction();

    // inform the drag source of the drag-origin frame of the drop result
    if (pEvent->source())
    {
        Qt5Widget* pWidget = dynamic_cast<Qt5Widget*>(pEvent->source());
        assert(pWidget); // AFAIK there shouldn't be any non-Qt5Widget as source in LO itself
        if (pWidget)
            pWidget->frame().m_pDragSource->fire_dragEnd(nDropAction, bDropSuccessful);
    }

    // the drop target accepted our drop action => inform Qt
    if (bDropSuccessful)
    {
        pEvent->setDropAction(getPreferredDropAction(nDropAction));
        pEvent->accept();
    }
    else // or maybe someone else likes it?
        pEvent->ignore();
}

void Qt5Frame::handleDragLeave()
{
    css::datatransfer::dnd::DropTargetEvent aEvent;
    aEvent.Source = static_cast<css::datatransfer::dnd::XDropTarget*>(m_pDropTarget);
    m_pDropTarget->fire_dragExit(aEvent);
    m_bInDrag = false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
