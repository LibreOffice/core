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

#include <QtCustomStyle.hxx>
#include <QtFrame.hxx>
#include <QtFrame.moc>

#include <QtData.hxx>
#include <QtDragAndDrop.hxx>
#include <QtGraphics.hxx>
#include <QtInstance.hxx>
#include <QtMainWindow.hxx>
#include <QtSvpGraphics.hxx>
#include <QtTransferable.hxx>
#if CHECK_ANY_QT_USING_X11
#include <QtX11Support.hxx>
#endif

#include <QtCore/QLibraryInfo>
#include <QtCore/QMimeData>
#include <QtCore/QPoint>
#include <QtCore/QSize>
#include <QtCore/QThread>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QIcon>
#include <QtGui/QWindow>
#include <QtGui/QScreen>
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QtGui/QStyleHints>
#endif
#include <QtWidgets/QStyle>
#include <QtWidgets/QToolTip>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMainWindow>
#if CHECK_QT5_USING_X11
#include <QtX11Extras/QX11Info>
#endif

#include <window.h>
#include <vcl/qt/QtUtils.hxx>

#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>

#include <cairo.h>
#include <headless/svpgdi.hxx>

static void SvpDamageHandler(void* handle, sal_Int32 nExtentsX, sal_Int32 nExtentsY,
                             sal_Int32 nExtentsWidth, sal_Int32 nExtentsHeight)
{
    QtFrame* pThis = static_cast<QtFrame*>(handle);
    pThis->Damage(nExtentsX, nExtentsY, nExtentsWidth, nExtentsHeight);
}

QtFrame::QtFrame(QtFrame* pParent, SalFrameStyleFlags nStyle, bool bUseCairo)
    : m_pTopLevel(nullptr)
    , m_bUseCairo(bUseCairo)
    , m_bNullRegion(true)
    , m_bGraphicsInUse(false)
    , m_ePointerStyle(PointerStyle::Arrow)
    , m_pDragSource(nullptr)
    , m_pDropTarget(nullptr)
    , m_bDefaultSize(true)
    , m_bDefaultPos(true)
    , m_bFullScreen(false)
    , m_bFullScreenSpanAll(false)
#if CHECK_ANY_QT_USING_X11
    , m_nKeyModifiers(ModKeyFlags::NONE)
#endif
    , m_nInputLanguage(LANGUAGE_DONTKNOW)
{
    GetQtInstance().insertFrame(this);

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

    Qt::WindowFlags aWinFlags(Qt::Widget);
    if (!(nStyle & SalFrameStyleFlags::SYSTEMCHILD))
    {
        if (nStyle & SalFrameStyleFlags::INTRO)
            aWinFlags = Qt::SplashScreen;
        // floating toolbars are frameless tool windows
        // + they must be able to receive keyboard focus
        else if ((nStyle & SalFrameStyleFlags::FLOAT)
                 && (nStyle & SalFrameStyleFlags::OWNERDRAWDECORATION))
            aWinFlags = Qt::Tool | Qt::FramelessWindowHint;
        else if (nStyle & SalFrameStyleFlags::TOOLTIP)
            aWinFlags = Qt::ToolTip;
        // Can't use Qt::Popup, because it grabs the input focus and generates a focus-out event,
        // instantly auto-closing the LO's editable ComboBox popup.
        // On X11, the alternative Qt::Window | Qt::FramelessWindowHint | Qt::BypassWindowManagerHint
        // seems to work well enough, but at least on Wayland and WASM, this results in problems.
        // So while using Qt::ToolTip, the popups are wrongly advertised via accessibility, at least
        // the GUI seems to work on all platforms... what a mess.
        else if (isPopup())
            aWinFlags = Qt::ToolTip | Qt::FramelessWindowHint;
        else if (nStyle & SalFrameStyleFlags::TOOLWINDOW)
            aWinFlags = Qt::Tool;
        // top level windows can't be transient in Qt, so make them dialogs, if they have a parent. At least
        // the plasma shell relies on this setting to skip dialogs in the window list. And Qt Xcb will just
        // set transient for the types Dialog, Sheet, Tool, SplashScreen, ToolTip, Drawer and Popup.
        else if (nStyle & SalFrameStyleFlags::DIALOG || m_pParent)
            aWinFlags = Qt::Dialog;
        else
            aWinFlags = Qt::Window;
    }

    if (aWinFlags == Qt::Window)
    {
        m_pTopLevel = new QtMainWindow(*this, aWinFlags);
        m_pQWidget = new QtWidget(*this);
        m_pTopLevel->setCentralWidget(m_pQWidget);
        m_pTopLevel->setFocusProxy(m_pQWidget);
    }
    else
    {
        m_pQWidget = new QtWidget(*this, aWinFlags);
        // from Qt's POV the popup window doesn't have the input focus, so we must force tooltips...
        if (isPopup())
            m_pQWidget->setAttribute(Qt::WA_AlwaysShowToolTips);
    }

    FillSystemEnvData(m_aSystemData, m_pQWidget, this);

    SetIcon(SV_ICON_ID_OFFICE);
}

void QtFrame::screenChanged(QScreen*) { m_pQWidget->fakeResize(); }

void QtFrame::FillSystemEnvData(SystemEnvData& rData, QWidget* pWidget, QtFrame* pFrame)
{
    assert(rData.platform == SystemEnvData::Platform::Invalid);
    assert(rData.toolkit == SystemEnvData::Toolkit::Invalid);
    if (QGuiApplication::platformName() == "wayland")
        rData.platform = SystemEnvData::Platform::Wayland;
    else if (QGuiApplication::platformName() == "xcb")
        rData.platform = SystemEnvData::Platform::Xcb;
    else if (QGuiApplication::platformName() == "wasm")
        rData.platform = SystemEnvData::Platform::WASM;
    else
    {
        // maybe add a SystemEnvData::Platform::Unsupported to avoid special cases and not abort?
        SAL_WARN("vcl.qt",
                 "Unsupported qt VCL platform: " << toOUString(QGuiApplication::platformName()));
        std::abort();
    }

    rData.toolkit = SystemEnvData::Toolkit::Qt;
    rData.pSalFrame = pFrame;
    rData.pWidget = pWidget;
}

QtFrame::~QtFrame()
{
    GetQtInstance().eraseFrame(this);
    delete asChild();
    m_aSystemData.pSalFrame = nullptr;
}

void QtFrame::Damage(sal_Int32 nExtentsX, sal_Int32 nExtentsY, sal_Int32 nExtentsWidth,
                     sal_Int32 nExtentsHeight) const
{
    GetQtInstance().EmscriptenLightweightRunInMainThread([
        this, r = scaledQRect(QRect(nExtentsX, nExtentsY, nExtentsWidth, nExtentsHeight),
                              1 / devicePixelRatioF())
    ] { m_pQWidget->update(r); });
}

SalGraphics* QtFrame::AcquireGraphics()
{
    if (m_bGraphicsInUse)
        return nullptr;

    m_bGraphicsInUse = true;

    if (m_bUseCairo)
    {
        if (!m_pSvpGraphics)
        {
            QSize aSize = m_pQWidget->size() * devicePixelRatioF();
            m_pSvpGraphics.reset(new QtSvpGraphics(this));
            m_pSurface.reset(
                cairo_image_surface_create(CAIRO_FORMAT_ARGB32, aSize.width(), aSize.height()));
            m_pSvpGraphics->setSurface(m_pSurface.get(),
                                       basegfx::B2IVector(aSize.width(), aSize.height()));
            cairo_surface_set_user_data(m_pSurface.get(), QtSvpGraphics::getDamageKey(),
                                        &m_aDamageHandler, nullptr);
        }
        return m_pSvpGraphics.get();
    }
    else
    {
        if (!m_pQtGraphics)
        {
            m_pQtGraphics.reset(new QtGraphics(this));
            m_pQImage.reset(
                new QImage(m_pQWidget->size() * devicePixelRatioF(), Qt_DefaultFormat32));
            m_pQImage->fill(Qt::transparent);
            m_pQtGraphics->ChangeQImage(m_pQImage.get());
        }
        return m_pQtGraphics.get();
    }
}

void QtFrame::ReleaseGraphics(SalGraphics* pSalGraph)
{
    (void)pSalGraph;
    if (m_bUseCairo)
        assert(pSalGraph == m_pSvpGraphics.get());
    else
        assert(pSalGraph == m_pQtGraphics.get());
    m_bGraphicsInUse = false;
}

bool QtFrame::PostEvent(std::unique_ptr<ImplSVEvent> pData)
{
    GetQtInstance().PostEvent(this, pData.release(), SalEvent::UserEvent);
    return true;
}

QWidget* QtFrame::asChild() const
{
    if (m_pTopLevel)
        return m_pTopLevel;
    return m_pQWidget;
}

qreal QtFrame::devicePixelRatioF() const
{
    return GetQtInstance().EmscriptenLightweightRunInMainThread(
        [child = asChild()] { return child->devicePixelRatioF(); });
}

bool QtFrame::isWindow() const { return asChild()->isWindow(); }

QWindow* QtFrame::windowHandle() const
{
    // set attribute 'Qt::WA_NativeWindow' first to make sure a window handle actually exists
    QWidget* pChild = asChild();
    assert(pChild->window() == pChild);
    switch (m_aSystemData.platform)
    {
        case SystemEnvData::Platform::WASM:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            // no idea, why Qt::WA_NativeWindow breaks the menubar for EMSCRIPTEN
            break;
#endif
        case SystemEnvData::Platform::Wayland:
        case SystemEnvData::Platform::Xcb:
            pChild->setAttribute(Qt::WA_NativeWindow);
            break;
        case SystemEnvData::Platform::Invalid:
            std::abort();
            break;
    }
    return pChild->windowHandle();
}

QScreen* QtFrame::screen() const { return asChild()->screen(); }

sal_Int32 QtFrame::screenNumber() const
{
    QScreen* pScreen = screen();
    const QList<QScreen*> screens = QApplication::screens();
    return screens.indexOf(pScreen);
}

bool QtFrame::GetUseDarkMode() const
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    const Qt::ColorScheme eColorScheme = QApplication::styleHints()->colorScheme();
    if (eColorScheme == Qt::ColorScheme::Dark)
        return true;
    if (eColorScheme == Qt::ColorScheme::Light)
        return false;
#endif

    // use same mechanism for determining dark mode preference as xdg-desktop-portal-kde, s.
    // https://invent.kde.org/plasma/xdg-desktop-portal-kde/-/blob/0a4237549debf9518f8cfbaf531456850c0729bd/src/settings.cpp#L213-227
    const QPalette aPalette = QApplication::palette();
    const int nWindowBackGroundGray = qGray(aPalette.window().color().rgb());
    return nWindowBackGroundGray < 192;
}

bool QtFrame::isMinimized() const { return asChild()->isMinimized(); }

bool QtFrame::isMaximized() const { return asChild()->isMaximized(); }

void QtFrame::SetWindowStateImpl(Qt::WindowStates eState)
{
    return asChild()->setWindowState(eState);
}

void QtFrame::SetTitle(const OUString& rTitle)
{
    GetQtInstance().RunInMainThread(
        [this, rTitle]() { m_pQWidget->window()->setWindowTitle(toQString(rTitle)); });
}

void QtFrame::SetIcon(sal_uInt16 nIcon)
{
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([this, nIcon]() { SetIcon(nIcon); });
        return;
    }

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

    if (QGuiApplication::platformName() == "wayland" && m_pQWidget->window()->isVisible())
    {
        // Qt currently doesn't provide API to directly set the app_id for a single
        // window/toplevel on Wayland, but the one set for the application is picked up
        // on hide/show, so do that.
        // An alternative would be to use private Qt API and low-level wayland API to set the
        // app_id directly, s. discussion in QTBUG-77182.
        const QString sOrigDesktopFileName = QGuiApplication::desktopFileName();
        QGuiApplication::setDesktopFileName(appicon);
        m_pQWidget->window()->hide();
        m_pQWidget->window()->show();
        QGuiApplication::setDesktopFileName(sOrigDesktopFileName);
    }
}

void QtFrame::SetMenu(SalMenu*) {}

void QtFrame::SetExtendedFrameStyle(SalExtStyle /*nExtStyle*/) { /* not needed */}

void QtFrame::Show(bool bVisible, bool bNoActivate)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([&] { Show(bVisible, bNoActivate); });
        return;
    }

    assert(m_pQWidget);
    if (bVisible == asChild()->isVisible())
        return;

    if (!bVisible) // hide
    {
        asChild()->setVisible(false);
        return;
    }

    QWindow* pChildWindow = windowHandle();
    connect(pChildWindow, &QWindow::screenChanged, this, &QtFrame::screenChanged,
            Qt::UniqueConnection);

    if (m_pParent && !(m_pParent->m_nStyle & SalFrameStyleFlags::PLUG))
    {
        QWindow* pParentWindow = m_pParent->windowHandle();
        if (pParentWindow && pChildWindow && (pParentWindow != pChildWindow))
            pChildWindow->setTransientParent(pParentWindow);
    }

    // show
    SetDefaultSize();

    QWidget* const pChild = asChild();
    pChild->setVisible(true);
    pChild->raise();
    if (!bNoActivate)
    {
        pChild->activateWindow();
        pChild->setFocus();
    }
}

void QtFrame::SetMinClientSize(tools::Long nWidth, tools::Long nHeight)
{
    if (!isChild())
    {
        const qreal fRatio = devicePixelRatioF();
        GetQtInstance().EmscriptenLightweightRunInMainThread(
            [ child = asChild(), w = round(nWidth / fRatio), h = round(nHeight / fRatio) ] {
                child->setMinimumSize(w, h);
            });
    }
}

void QtFrame::SetMaxClientSize(tools::Long nWidth, tools::Long nHeight)
{
    if (!isChild())
    {
        const qreal fRatio = devicePixelRatioF();
        asChild()->setMaximumSize(round(nWidth / fRatio), round(nHeight / fRatio));
    }
}

void QtFrame::SetDefaultPos()
{
    if (!m_bDefaultPos)
        return;

    // center on parent
    if (m_pParent)
    {
        const qreal fRatio = devicePixelRatioF();
        QWidget* const pParentWin = m_pParent->asChild()->window();
        QWidget* const pChildWin = asChild()->window();
        QPoint aPos = (pParentWin->rect().center() - pChildWin->rect().center()) * fRatio;
        SetPosSize(aPos.x(), aPos.y(), 0, 0, SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y);
        assert(!m_bDefaultPos);
    }
    else
        m_bDefaultPos = false;
}

Size QtFrame::CalcDefaultSize()
{
    assert(isWindow());

    Size aSize;
    if (!m_bFullScreen)
    {
        const QScreen* pScreen = screen();
        if (!pScreen)
            pScreen = QGuiApplication::screens().at(0);
        aSize = bestmaxFrameSizeForScreenSize(toSize(pScreen->size()));
    }
    else
    {
        if (!m_bFullScreenSpanAll)
        {
            aSize = toSize(screen()->size());
        }
        else
        {
            QScreen* pScreen = QGuiApplication::screenAt(QPoint(0, 0));
            aSize = toSize(pScreen->availableVirtualGeometry().size());
        }
    }

    return aSize;
}

void QtFrame::SetDefaultSize()
{
    if (!m_bDefaultSize)
        return;

    Size aDefSize = CalcDefaultSize();
    SetPosSize(0, 0, aDefSize.Width(), aDefSize.Height(),
               SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT);
    assert(!m_bDefaultSize);
}

void QtFrame::SetPosSize(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight,
                         sal_uInt16 nFlags)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([&] { SetPosSize(nX, nY, nWidth, nHeight, nFlags); });
        return;
    }

    if (!isWindow() || isChild(true, false))
        return;

    if (nFlags & (SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT))
    {
        if (isChild(false) || !m_pQWidget->isMaximized())
        {
            if (!(nFlags & SAL_FRAME_POSSIZE_WIDTH))
                nWidth = GetWidth();
            else if (!(nFlags & SAL_FRAME_POSSIZE_HEIGHT))
                nHeight = GetHeight();

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
        }
    }

    if (!(nFlags & (SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y)))
    {
        if (nFlags & (SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT))
            SetDefaultPos();
        return;
    }

    if (m_pParent)
    {
        const SalFrameGeometry aParentGeometry = m_pParent->GetUnmirroredGeometry();
        if (QGuiApplication::isRightToLeft())
            nX = aParentGeometry.x() + aParentGeometry.width() - nX - GetWidth() - 1;
        else
            nX += aParentGeometry.x();
        nY += aParentGeometry.y();
    }

    if (!(nFlags & SAL_FRAME_POSSIZE_X))
        nX = GetUnmirroredGeometry().x();
    else if (!(nFlags & SAL_FRAME_POSSIZE_Y))
        nY = GetUnmirroredGeometry().y();

    m_bDefaultPos = false;
    asChild()->move(round(nX / devicePixelRatioF()), round(nY / devicePixelRatioF()));
}

void QtFrame::GetClientSize(tools::Long& rWidth, tools::Long& rHeight)
{
    rWidth = round(m_pQWidget->width() * devicePixelRatioF());
    rHeight = round(m_pQWidget->height() * devicePixelRatioF());
}

SalFrameGeometry QtFrame::GetUnmirroredGeometry() const
{
    SalFrameGeometry aGeometry;

    const qreal fRatio = devicePixelRatioF();
    const QPoint aScreenPos = m_pQWidget->mapToGlobal(QPoint(0, 0));
    aGeometry.setX(aScreenPos.x() * fRatio);
    aGeometry.setY(aScreenPos.y() * fRatio);
    aGeometry.setWidth(m_pQWidget->width() * fRatio);
    aGeometry.setHeight(m_pQWidget->height() * fRatio);

    aGeometry.setScreen(std::max(sal_Int32(0), screenNumber()));

    return aGeometry;
}

void QtFrame::GetWorkArea(AbsoluteScreenPixelRectangle& rRect)
{
    if (!isWindow())
        return;
    QScreen* pScreen = screen();
    if (!pScreen)
        return;

    QSize aSize = pScreen->availableVirtualSize() * devicePixelRatioF();
    rRect = AbsoluteScreenPixelRectangle(0, 0, aSize.width(), aSize.height());
}

SalFrame* QtFrame::GetParent() const { return m_pParent; }

void QtFrame::SetModal(bool bModal)
{
    if (!isWindow() || asChild()->isModal() == bModal)
        return;

    GetQtInstance().RunInMainThread([this, bModal]() {

        QWidget* const pChild = asChild();
        const bool bWasVisible = pChild->isVisible();

        // modality change is only effective if the window is hidden
        if (bWasVisible)
        {
            pChild->hide();
            if (QGuiApplication::platformName() == "xcb")
            {
                SAL_WARN("vcl.qt", "SetModal called after Show - apply delay");
                // tdf#152979 give QXcbConnection some time to avoid
                // "qt.qpa.xcb: internal error:  void QXcbWindow::setNetWmStateOnUnmappedWindow() called on mapped window"
                QThread::msleep(100);
            }
        }

        pChild->setWindowModality(bModal ? Qt::WindowModal : Qt::NonModal);

        if (bWasVisible)
            pChild->show();
    });
}

void QtFrame::SetWindowState(const vcl::WindowData* pState)
{
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([this, pState]() { SetWindowState(pState); });
        return;
    }

    if (!isWindow() || !pState || isChild(true, false))
        return;

    const vcl::WindowDataMask nMaxGeometryMask
        = vcl::WindowDataMask::PosSize | vcl::WindowDataMask::MaximizedX
          | vcl::WindowDataMask::MaximizedY | vcl::WindowDataMask::MaximizedWidth
          | vcl::WindowDataMask::MaximizedHeight;

    if ((pState->mask() & vcl::WindowDataMask::State)
        && (pState->state() & vcl::WindowState::Maximized) && !isMaximized()
        && (pState->mask() & nMaxGeometryMask) == nMaxGeometryMask)
    {
        const qreal fRatio = devicePixelRatioF();
        QWidget* const pChild = asChild();
        pChild->resize(ceil(pState->width() / fRatio), ceil(pState->height() / fRatio));
        pChild->move(ceil(pState->x() / fRatio), ceil(pState->y() / fRatio));
        SetWindowStateImpl(Qt::WindowMaximized);
    }
    else if (pState->mask() & vcl::WindowDataMask::PosSize)
    {
        sal_uInt16 nPosSizeFlags = 0;
        if (pState->mask() & vcl::WindowDataMask::X)
            nPosSizeFlags |= SAL_FRAME_POSSIZE_X;
        if (pState->mask() & vcl::WindowDataMask::Y)
            nPosSizeFlags |= SAL_FRAME_POSSIZE_Y;
        if (pState->mask() & vcl::WindowDataMask::Width)
            nPosSizeFlags |= SAL_FRAME_POSSIZE_WIDTH;
        if (pState->mask() & vcl::WindowDataMask::Height)
            nPosSizeFlags |= SAL_FRAME_POSSIZE_HEIGHT;
        SetPosSize(pState->x(), pState->y(), pState->width(), pState->height(), nPosSizeFlags);
    }
    else if (pState->mask() & vcl::WindowDataMask::State && !isChild())
    {
        if (pState->state() & vcl::WindowState::Maximized)
            SetWindowStateImpl(Qt::WindowMaximized);
        else if (pState->state() & vcl::WindowState::Minimized)
            SetWindowStateImpl(Qt::WindowMinimized);
        else
            SetWindowStateImpl(Qt::WindowNoState);
    }
}

bool QtFrame::GetWindowState(vcl::WindowData* pState)
{
    pState->setState(vcl::WindowState::Normal);
    pState->setMask(vcl::WindowDataMask::State);
    if (isMinimized())
        pState->rState() |= vcl::WindowState::Minimized;
    else if (isMaximized())
        pState->rState() |= vcl::WindowState::Maximized;
    else
    {
        // we want the frame position and the client area size
        QRect rect = scaledQRect({ asChild()->pos(), asChild()->size() }, devicePixelRatioF());
        pState->setPosSize(toRectangle(rect));
        pState->rMask() |= vcl::WindowDataMask::PosSize;
    }

    return true;
}

void QtFrame::ShowFullScreen(bool bFullScreen, sal_Int32 nScreen)
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
        m_nRestoreScreen = std::max(sal_Int32(0), screenNumber());
        SetScreenNumber(m_bFullScreenSpanAll ? m_nRestoreScreen : nScreen);
        if (!m_bFullScreenSpanAll)
            m_pTopLevel->showFullScreen();
        else
            m_pTopLevel->showNormal();
    }
    else
    {
        SetScreenNumber(m_nRestoreScreen);
        m_pTopLevel->showNormal();
        m_pTopLevel->setGeometry(m_aRestoreGeometry);
    }
}

void QtFrame::StartPresentation(bool bStart)
{
#if !defined EMSCRIPTEN
    assert(m_aSystemData.platform != SystemEnvData::Platform::Invalid);

#if CHECK_QT5_USING_X11
    unsigned int nRootWindow(0);
    std::optional<Display*> aDisplay;
    if (QX11Info::isPlatformX11())
    {
        nRootWindow = QX11Info::appRootWindow();
        aDisplay = QX11Info::display();
    }
    m_SessionManagerInhibitor.inhibit(bStart, u"presentation", APPLICATION_INHIBIT_IDLE,
                                      nRootWindow, aDisplay);
#else
    m_SessionManagerInhibitor.inhibit(bStart, u"presentation", APPLICATION_INHIBIT_IDLE);
#endif
#else
    Q_UNUSED(bStart)
#endif
}

void QtFrame::SetAlwaysOnTop(bool bOnTop)
{
    QWidget* const pWidget = asChild();
    const Qt::WindowFlags flags = pWidget->windowFlags();
    if (bOnTop)
        pWidget->setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
    else
        pWidget->setWindowFlags(flags & ~(Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint));
}

void QtFrame::ToTop(SalFrameToTop nFlags)
{
    GetQtInstance().RunInMainThread([this, nFlags]() {
        QWidget* const pWidget = asChild();
        if (isWindow() && !(nFlags & SalFrameToTop::GrabFocusOnly))
            pWidget->raise();
        if ((nFlags & SalFrameToTop::RestoreWhenMin) || (nFlags & SalFrameToTop::ForegroundTask))
        {
            if (nFlags & SalFrameToTop::RestoreWhenMin)
                pWidget->setWindowState(pWidget->windowState() & ~Qt::WindowMinimized);
            pWidget->activateWindow();
        }
        else if ((nFlags & SalFrameToTop::GrabFocus) || (nFlags & SalFrameToTop::GrabFocusOnly))
        {
            if (!(nFlags & SalFrameToTop::GrabFocusOnly))
                pWidget->activateWindow();
            pWidget->setFocus(Qt::OtherFocusReason);
        }
    });
}

void QtFrame::SetPointer(PointerStyle ePointerStyle)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] {
        if (ePointerStyle == m_ePointerStyle)
            return;
        m_ePointerStyle = ePointerStyle;

        m_pQWidget->setCursor(GetQtData()->getCursor(ePointerStyle));
    });
}

void QtFrame::CaptureMouse(bool bMouse)
{
    static const char* pEnv = getenv("SAL_NO_MOUSEGRABS");
    if (pEnv && *pEnv)
        return;

    if (bMouse)
        m_pQWidget->grabMouse();
    else
        m_pQWidget->releaseMouse();
}

void QtFrame::SetPointerPos(tools::Long nX, tools::Long nY)
{
    // some cursor already exists (and it has m_ePointerStyle shape)
    // so here we just reposition it
    QCursor::setPos(m_pQWidget->mapToGlobal(QPoint(nX, nY) / devicePixelRatioF()));
}

void QtFrame::Flush()
{
    // was: QGuiApplication::sync();
    // but FIXME it causes too many issues, figure out sth better

    // unclear if we need to also flush cairo surface - gtk3 backend
    // does not do it. QPainter in QtWidget::paintEvent() is
    // destroyed, so that state should be safely flushed.
}

bool QtFrame::ShowTooltip(const OUString& rText, const tools::Rectangle& rHelpArea)
{
    QRect aHelpArea(toQRect(rHelpArea));
    if (QGuiApplication::isRightToLeft())
        aHelpArea.moveLeft(GetWidth() - aHelpArea.width() - aHelpArea.left() - 1);
    m_aTooltipText = rText;
    m_aTooltipArea = aHelpArea;
    return true;
}

void QtFrame::SetInputContext(SalInputContext* pContext)
{
    if (!pContext)
        return;

    if (!(pContext->mnOptions & InputContextFlags::Text))
        return;

    m_pQWidget->setAttribute(Qt::WA_InputMethodEnabled);
}

void QtFrame::EndExtTextInput(EndExtTextInputFlags /*nFlags*/) { m_pQWidget->endExtTextInput(); }

OUString QtFrame::GetKeyName(sal_uInt16 nKeyCode)
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
            case KEY_NUMBERSIGN:
                nRetCode = Qt::Key_NumberSign;
                break;
            case KEY_XF86FORWARD:
                nRetCode = Qt::Key_Forward;
                break;
            case KEY_XF86BACK:
                nRetCode = Qt::Key_Back;
                break;
            case KEY_COLON:
                nRetCode = Qt::Key_Colon;
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

bool QtFrame::MapUnicodeToKeyCode(sal_Unicode /*aUnicode*/, LanguageType /*aLangType*/,
                                  vcl::KeyCode& /*rKeyCode*/)
{
    // not supported yet
    return false;
}

LanguageType QtFrame::GetInputLanguage() { return m_nInputLanguage; }

void QtFrame::setInputLanguage(LanguageType nInputLanguage)
{
    if (nInputLanguage == m_nInputLanguage)
        return;
    m_nInputLanguage = nInputLanguage;
    CallCallback(SalEvent::InputLanguageChange, nullptr);
}

void QtFrame::UpdateSettings(AllSettings& rSettings)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] {
        if (QtData::noNativeControls())
            return;
        QtCustomStyle::LoadCustomStyle(GetUseDarkMode());

        StyleSettings style(rSettings.GetStyleSettings());
        const css::lang::Locale aLocale = rSettings.GetUILanguageTag().getLocale();

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
        style.SetListBoxWindowTextColor(aText);
        style.SetWindowTextColor(aText);
        style.SetToolTextColor(aText);

        // Base
        style.SetFieldColor(aBase);
        style.SetActiveTabColor(aBase);
        style.SetListBoxWindowBackgroundColor(aBase);
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
        style.SetWindowColor(aBack);

        // Workspace
        style.SetWorkspaceColor(aMid);

        // Selection
        // https://invent.kde.org/plasma/plasma-workspace/-/merge_requests/305
        style.SetAccentColor(aHigh);
        style.SetHighlightColor(aHigh);
        style.SetHighlightTextColor(aHighText);
        style.SetListBoxWindowHighlightColor(aHigh);
        style.SetListBoxWindowHighlightTextColor(aHighText);
        style.SetActiveColor(aHigh);
        style.SetActiveTextColor(aHighText);

        // Links
        style.SetLinkColor(aLink);
        style.SetVisitedLinkColor(aVisitedLink);

        // Tooltip
        style.SetHelpColor(
            toColor(QToolTip::palette().color(QPalette::Active, QPalette::ToolTipBase)));
        style.SetHelpTextColor(
            toColor(QToolTip::palette().color(QPalette::Active, QPalette::ToolTipText)));

        // Menu
        std::unique_ptr<QMenuBar> pMenuBar = std::make_unique<QMenuBar>();
        QPalette qMenuCG = pMenuBar->palette();

        // Menu text and background color, theme specific
        Color aMenuFore = toColor(qMenuCG.color(QPalette::WindowText));
        Color aMenuBack = toColor(qMenuCG.color(QPalette::Window));

        style.SetMenuTextColor(aMenuFore);
        style.SetMenuBarTextColor(aMenuFore);
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
            style.SetMenuBarRolloverTextColor(
                ImplGetSVData()->maNWFData.maMenuBarHighlightTextColor);
        }
        else
        {
            style.SetMenuBarRolloverColor(aMenuBack);
            style.SetMenuBarRolloverTextColor(aMenuFore);
        }
        style.SetMenuBarHighlightTextColor(style.GetMenuHighlightTextColor());

        // Default fonts
        vcl::Font aFont;
        if (toVclFont(QApplication::font(), aLocale, aFont))
        {
            style.BatchSetFonts(aFont, aFont);
            aFont.SetWeight(WEIGHT_BOLD);
            style.SetTitleFont(aFont);
            style.SetFloatTitleFont(aFont);
        }

        // Tooltip font
        if (toVclFont(QToolTip::font(), aLocale, aFont))
            style.SetHelpFont(aFont);

        // Menu bar font
        if (toVclFont(pMenuBar->font(), aLocale, aFont))
            style.SetMenuFont(aFont);

        // Icon theme
        const bool bPreferDarkTheme = GetUseDarkMode();
        style.SetPreferredIconTheme(toOUString(QIcon::themeName()), bPreferDarkTheme);

        // Scroll bar size
        style.SetScrollBarSize(QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent));
        style.SetMinThumbSize(QApplication::style()->pixelMetric(QStyle::PM_ScrollBarSliderMin));

        // These colors are used for the ruler text and marks
        style.SetShadowColor(toColor(pal.color(QPalette::Disabled, QPalette::WindowText)));
        style.SetDarkShadowColor(toColor(pal.color(QPalette::Inactive, QPalette::WindowText)));

        // match native QComboBox behavior of putting text cursor to end of text
        // without text selection when combobox entry is selected
        style.SetComboBoxTextSelectionMode(ComboBoxTextSelectionMode::CursorToEnd);

        // Cursor blink interval
        int nFlashTime = QApplication::cursorFlashTime();
        style.SetCursorBlinkTime(nFlashTime != 0 ? nFlashTime / 2 : STYLE_CURSOR_NOBLINKTIME);
        style.SetSystemColorsLoaded(true);

        rSettings.SetStyleSettings(style);
    });
}

void QtFrame::Beep() { QApplication::beep(); }

SalFrame::SalPointerState QtFrame::GetPointerState()
{
    SalPointerState aState;
    aState.maPos = toPoint(QCursor::pos() * devicePixelRatioF());
    SalFrameGeometry aGeometry = GetUnmirroredGeometry();
    aState.maPos.Move(-aGeometry.x(), -aGeometry.y());
    aState.mnState = toVclMouseButtons(QGuiApplication::mouseButtons())
                     | toVclKeyboardModifiers(QGuiApplication::keyboardModifiers());
    return aState;
}

KeyIndicatorState QtFrame::GetIndicatorState() { return KeyIndicatorState(); }

void QtFrame::SimulateKeyPress(sal_uInt16 nKeyCode)
{
    SAL_WARN("vcl.qt", "missing simulate keypress " << nKeyCode);
}

// don't set QWidget parents; this breaks popups on Wayland, like the LO ComboBox or ColorPicker!
void QtFrame::SetParent(SalFrame* pNewParent) { m_pParent = static_cast<QtFrame*>(pNewParent); }

void QtFrame::SetPluginParent(SystemParentData* /*pNewParent*/)
{
    //FIXME: no SetPluginParent impl. for qt5
}

void QtFrame::ResetClipRegion() { m_bNullRegion = true; }

void QtFrame::BeginSetClipRegion(sal_uInt32)
{
    m_aRegion = QRegion(QRect(QPoint(0, 0), m_pQWidget->size()));
}

void QtFrame::UnionClipRegion(tools::Long nX, tools::Long nY, tools::Long nWidth,
                              tools::Long nHeight)
{
    m_aRegion
        = m_aRegion.united(scaledQRect(QRect(nX, nY, nWidth, nHeight), 1 / devicePixelRatioF()));
}

void QtFrame::EndSetClipRegion() { m_bNullRegion = false; }

void QtFrame::SetScreenNumber(unsigned int nScreen)
{
    if (!isWindow())
        return;

    QWindow* const pWindow = windowHandle();
    if (!pWindow)
        return;

    QList<QScreen*> screens = QApplication::screens();
    if (static_cast<int>(nScreen) >= screens.size() && !m_bFullScreenSpanAll)
    {
        SAL_WARN("vcl.qt", "Ignoring request to set invalid screen number");
        return;
    }

    QRect screenGeo;

    if (!m_bFullScreenSpanAll)
    {
        screenGeo = QGuiApplication::screens().at(nScreen)->geometry();
        pWindow->setScreen(QApplication::screens().at(nScreen));
    }
    else // special case: fullscreen over all available screens
    {
        assert(m_bFullScreen);
        // left-most screen
        QScreen* pScreen = QGuiApplication::screenAt(QPoint(0, 0));
        // entire virtual desktop
        screenGeo = pScreen->availableVirtualGeometry();
        pWindow->setScreen(pScreen);
        pWindow->setGeometry(screenGeo);
    }

    // setScreen by itself has no effect, explicitly move the widget to
    // the new screen
    GetQtInstance().EmscriptenLightweightRunInMainThread(
        [ child = asChild(), topLeft = screenGeo.topLeft() ] { child->move(topLeft); });
}

void QtFrame::SetApplicationID(const OUString& rWMClass)
{
#if CHECK_QT5_USING_X11
    assert(m_aSystemData.platform != SystemEnvData::Platform::Invalid);
    if (m_aSystemData.platform != SystemEnvData::Platform::Xcb || !m_pTopLevel)
        return;

    QtX11Support::setApplicationID(m_pTopLevel->winId(), rWMClass);
#else
    Q_UNUSED(rWMClass);
#endif
}

void QtFrame::ResolveWindowHandle(SystemEnvData& rData) const
{
    if (!rData.pWidget)
        return;
    assert(rData.platform != SystemEnvData::Platform::Invalid);
    // Calling QWidget::winId() implicitly enables native windows to be used instead
    // of "alien widgets" that don't have a native widget associated with them,
    // s. https://doc.qt.io/qt-6/qwidget.html#native-widgets-vs-alien-widgets
    // Avoid native widgets with Qt 5 on Wayland and with Qt 6 altogether as they
    // cause unresponsive UI, s. tdf#122293/QTBUG-75766 and tdf#160565
    // (for qt5 xcb, they're needed for video playback)
    if (rData.platform != SystemEnvData::Platform::Wayland
        && QLibraryInfo::version().majorVersion() < 6)
    {
        rData.SetWindowHandle(static_cast<QWidget*>(rData.pWidget)->winId());
    }
}

bool QtFrame::GetUseReducedAnimation() const { return GetQtInstance().GetUseReducedAnimation(); }

// Drag'n'drop foo

void QtFrame::registerDragSource(QtDragSource* pDragSource)
{
    assert(!m_pDragSource);
    m_pDragSource = pDragSource;
}

void QtFrame::registerDropTarget(QtDropTarget* pDropTarget)
{
    assert(!m_pDropTarget);
    m_pDropTarget = pDropTarget;

    GetQtInstance().RunInMainThread([this]() { m_pQWidget->setAcceptDrops(true); });
}

void QtFrame::handleDragEnter(QDragEnterEvent* pEvent)
{
    assert(pEvent);
    assert(m_pDropTarget);

    m_pDropTarget->handleDragEnterEvent(*pEvent, devicePixelRatioF());
}

void QtFrame::handleDragMove(QDragMoveEvent* pEvent)
{
    assert(pEvent);
    assert(m_pDropTarget);

    m_pDropTarget->handleDragMoveEvent(*pEvent, devicePixelRatioF());
}

void QtFrame::handleDrop(QDropEvent* pEvent)
{
    assert(pEvent);
    assert(m_pDropTarget);

    m_pDropTarget->handleDropEvent(*pEvent, devicePixelRatioF());

    // inform the drag source of the drag-origin frame of the drop result
    if (pEvent->source())
    {
        QtWidget* pWidget = qobject_cast<QtWidget*>(pEvent->source());
        assert(pWidget); // AFAIK there shouldn't be any non-Qt5Widget as source in LO itself
        if (pWidget)
            pWidget->frame().m_pDragSource->fire_dragEnd(m_pDropTarget->proposedDropAction(),
                                                         m_pDropTarget->dropSuccessful());
    }
}

void QtFrame::handleDragLeave() { m_pDropTarget->dragExit(); }

void QtFrame::handleMoveEvent(QMoveEvent*) { CallCallback(SalEvent::Move, nullptr); }

void QtFrame::handlePaintEvent(const QPaintEvent* pEvent, QWidget* pWidget)
{
    QPainter p(pWidget);
    if (!m_bNullRegion)
        p.setClipRegion(m_aRegion);

    QImage aImage;
    if (m_bUseCairo)
    {
        cairo_surface_t* pSurface = m_pSurface.get();
        cairo_surface_flush(pSurface);

        aImage = QImage(cairo_image_surface_get_data(pSurface),
                        cairo_image_surface_get_width(pSurface),
                        cairo_image_surface_get_height(pSurface), Qt_DefaultFormat32);
    }
    else
        aImage = *m_pQImage;

    const qreal fRatio = devicePixelRatioF();
    aImage.setDevicePixelRatio(fRatio);
    QRectF source(pEvent->rect().topLeft() * fRatio, pEvent->rect().size() * fRatio);
    p.drawImage(pEvent->rect(), aImage, source);
}

void QtFrame::handleResizeEvent(const QResizeEvent* pEvent)
{
    const qreal fRatio = devicePixelRatioF();
    const int nWidth = ceil(pEvent->size().width() * fRatio);
    const int nHeight = ceil(pEvent->size().height() * fRatio);

    if (m_bUseCairo)
    {
        if (m_pSurface)
        {
            const int nOldWidth = cairo_image_surface_get_width(m_pSurface.get());
            const int nOldHeight = cairo_image_surface_get_height(m_pSurface.get());
            if (nOldWidth != nWidth || nOldHeight != nHeight)
            {
                cairo_surface_t* pSurface
                    = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, nWidth, nHeight);
                cairo_surface_set_user_data(pSurface, SvpSalGraphics::getDamageKey(),
                                            &m_aDamageHandler, nullptr);
                m_pSvpGraphics->setSurface(pSurface, basegfx::B2IVector(nWidth, nHeight));
                UniqueCairoSurface old_surface(m_pSurface.release());
                m_pSurface.reset(pSurface);

                const int nMinWidth = qMin(nOldWidth, nWidth);
                const int nMinHeight = qMin(nOldHeight, nHeight);
                SalTwoRect rect(0, 0, nMinWidth, nMinHeight, 0, 0, nMinWidth, nMinHeight);
                m_pSvpGraphics->copySource(rect, old_surface.get());
            }
        }
    }
    else
    {
        if (m_pQImage && m_pQImage->size() != QSize(nWidth, nHeight))
        {
            QImage* pImage = new QImage(m_pQImage->copy(0, 0, nWidth, nHeight));
            m_pQtGraphics->ChangeQImage(pImage);
            m_pQImage.reset(pImage);
        }
    }

    CallCallback(SalEvent::Resize, nullptr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
