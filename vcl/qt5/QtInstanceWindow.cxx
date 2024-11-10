/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
    assert(false && "Not implemented yet");
    return true;
}

Size QtInstanceWindow::get_size() const
{
    assert(false && "Not implemented yet");
    return Size();
}

Point QtInstanceWindow::get_position() const
{
    assert(false && "Not implemented yet");
    return Point();
}

AbsoluteScreenPixelRectangle QtInstanceWindow::get_monitor_workarea() const
{
    assert(false && "Not implemented yet");
    return AbsoluteScreenPixelRectangle();
}

void QtInstanceWindow::set_centered_on_parent(bool) { assert(false && "Not implemented yet"); }

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

void QtInstanceWindow::set_window_state(const OUString&) { assert(false && "Not implemented yet"); }

OUString QtInstanceWindow::get_window_state(vcl::WindowDataMask) const
{
    assert(false && "Not implemented yet");
    return OUString();
}

css::uno::Reference<css::awt::XWindow> QtInstanceWindow::GetXWindow()
{
    assert(false && "Not implemented yet");
    return css::uno::Reference<css::awt::XWindow>();
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
            return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
