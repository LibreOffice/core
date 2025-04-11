/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceWindow.hxx>
#include <QtInstanceWindow.moc>

#include <vcl/qt/QtUtils.hxx>

#include <QtGui/QWindow>

QtInstanceWindow::QtInstanceWindow(QWidget* pWidget)
    : QtInstanceContainer(pWidget)
{
    // install event filter to handle window activation/deactivation events
    pWidget->installEventFilter(this);
}

QtInstanceWindow::~QtInstanceWindow()
{
    if (m_xWindow.is())
        m_xWindow->clear();
}

void QtInstanceWindow::set_title(const OUString& rTitle)
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        rQtInstance.RunInMainThread([&] { set_title(rTitle); });
        return;
    }

    getQWidget()->setWindowTitle(toQString(rTitle));
}

OUString QtInstanceWindow::get_title() const
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        OUString sTitle;
        rQtInstance.RunInMainThread([&] { sTitle = get_title(); });
        return sTitle;
    }

    return toOUString(getQWidget()->windowTitle());
}

void QtInstanceWindow::window_move(int, int) { assert(false && "Not implemented yet"); }

bool QtInstanceWindow::get_resizable() const
{
    SolarMutexGuard g;
    bool bResizable = true;
    GetQtInstance().RunInMainThread([&] {
        const QSizePolicy aSizePolicy = getQWidget()->sizePolicy();
        bResizable = (aSizePolicy.horizontalPolicy() != QSizePolicy::Policy::Fixed)
                     || (aSizePolicy.verticalPolicy() != QSizePolicy::Policy::Fixed);
    });
    return bResizable;
}

Size QtInstanceWindow::get_size() const
{
    SolarMutexGuard g;

    Size aSize;
    GetQtInstance().RunInMainThread([&] { aSize = toSize(getQWidget()->size()); });
    return aSize;
}

Point QtInstanceWindow::get_position() const
{
    SolarMutexGuard g;

    Point aPosition;
    GetQtInstance().RunInMainThread(
        [&] { aPosition = toPoint(getQWidget()->geometry().topLeft()); });
    return aPosition;
}

AbsoluteScreenPixelRectangle QtInstanceWindow::get_monitor_workarea() const
{
    assert(false && "Not implemented yet");
    return AbsoluteScreenPixelRectangle();
}

bool QtInstanceWindow::has_toplevel_focus() const
{
    SolarMutexGuard g;
    bool bFocus;
    GetQtInstance().RunInMainThread([&] { bFocus = QApplication::activeWindow() == getQWidget(); });
    return bFocus;
}

void QtInstanceWindow::present()
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] {
        if (QWindow* pWindow = getQWidget()->windowHandle())
            pWindow->requestActivate();
    });
}

void QtInstanceWindow::change_default_widget(weld::Widget*, weld::Widget*)
{
    assert(false && "Not implemented yet");
}

bool QtInstanceWindow::is_default_widget(const weld::Widget*) const
{
    assert(false && "Not implemented yet");
    return true;
}

void QtInstanceWindow::set_window_state(const OUString& rStr)
{
    SolarMutexGuard g;

    const vcl::WindowData aData(rStr);
    const vcl::WindowDataMask eMask = aData.mask();

    GetQtInstance().RunInMainThread([&] {
        QRect aGeometry = getQWidget()->geometry();
        if (eMask & vcl::WindowDataMask::X)
            aGeometry.setX(aData.x());
        if (eMask & vcl::WindowDataMask::Y)
            aGeometry.setY(aData.y());
        if (eMask & vcl::WindowDataMask::Width)
            aGeometry.setWidth(aData.width());
        if (eMask & vcl::WindowDataMask::Height)
            aGeometry.setHeight(aData.height());

        getQWidget()->setGeometry(aGeometry);

        if (eMask & vcl::WindowDataMask::State)
        {
            const vcl::WindowState eState = aData.state();
            if (eState & vcl::WindowState::Normal)
                getQWidget()->showNormal();
            else if (eState & vcl::WindowState::Maximized)
                getQWidget()->showMaximized();
            else if (eState & vcl::WindowState::Minimized)
                getQWidget()->showMinimized();
        }
    });
}

OUString QtInstanceWindow::get_window_state(vcl::WindowDataMask eMask) const
{
    SolarMutexGuard g;

    vcl::WindowData aData;
    GetQtInstance().RunInMainThread([&] {
        QRect aGeometry = getQWidget()->geometry();
        if (eMask & vcl::WindowDataMask::X)
            aData.setX(aGeometry.x());
        if (eMask & vcl::WindowDataMask::Y)
            aData.setY(aGeometry.y());
        if (eMask & vcl::WindowDataMask::Width)
            aData.setWidth(aGeometry.width());
        if (eMask & vcl::WindowDataMask::Height)
            aData.setHeight(aGeometry.height());
        if (eMask & vcl::WindowDataMask::State)
        {
            vcl::WindowState nState = vcl::WindowState::NONE;
            if (getQWidget()->isMaximized())
                nState |= vcl::WindowState::Maximized;
            else if (getQWidget()->isMinimized())
                nState |= vcl::WindowState::Minimized;
            else
                nState |= vcl::WindowState::Normal;
            aData.setState(nState);
        }
    });

    return aData.toStr();
}

css::uno::Reference<css::awt::XWindow> QtInstanceWindow::GetXWindow()
{
    if (!m_xWindow.is())
        m_xWindow.set(new QtXWindow(this));
    return m_xWindow;
}

SystemEnvData QtInstanceWindow::get_system_data() const
{
    assert(false && "Not implemented yet");
    const SystemEnvData* pEnvData = nullptr;

    return *pEnvData;
}

void QtInstanceWindow::resize_to_request() { assert(false && "Not implemented yet"); }

weld::ScreenShotCollection QtInstanceWindow::collect_screenshot_data()
{
    assert(false && "Not implemented yet");
    return weld::ScreenShotCollection();
}

VclPtr<VirtualDevice> QtInstanceWindow::screenshot()
{
    assert(false && "Not implemented yet");
    return nullptr;
}

const vcl::ILibreOfficeKitNotifier* QtInstanceWindow::GetLOKNotifier() { return nullptr; }

bool QtInstanceWindow::eventFilter(QObject* pObject, QEvent* pEvent)
{
    if (pObject != getQWidget())
        return false;

    switch (pEvent->type())
    {
        case QEvent::WindowActivate:
        case QEvent::WindowDeactivate:
            signal_container_focus_changed();
            return false;
        default:
            return QtInstanceContainer::eventFilter(pObject, pEvent);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
