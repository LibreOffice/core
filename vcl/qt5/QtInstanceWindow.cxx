/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceWindow.hxx>

QtInstanceWindow::QtInstanceWindow(QWidget* pWidget)
    : QtInstanceContainer(pWidget)
{
}

void QtInstanceWindow::set_title(const OUString& rTitle)
{
    SolarMutexGuard g;
    QtInstance* pQtInstance = GetQtInstance();
    if (!pQtInstance->IsMainThread())
    {
        pQtInstance->RunInMainThread([&] { set_title(rTitle); });
        return;
    }

    getQWidget()->setWindowTitle(toQString(rTitle));
}

OUString QtInstanceWindow::get_title() const
{
    SolarMutexGuard g;
    QtInstance* pQtInstance = GetQtInstance();
    if (!pQtInstance->IsMainThread())
    {
        OUString sTitle;
        pQtInstance->RunInMainThread([&] { sTitle = get_title(); });
        return sTitle;
    }

    return toOUString(getQWidget()->windowTitle());
}

void QtInstanceWindow::window_move(int, int) {}

void QtInstanceWindow::set_modal(bool) {}

bool QtInstanceWindow::get_modal() const { return true; }

bool QtInstanceWindow::get_resizable() const { return true; }

Size QtInstanceWindow::get_size() const { return Size(); }

Point QtInstanceWindow::get_position() const { return Point(); }

AbsoluteScreenPixelRectangle QtInstanceWindow::get_monitor_workarea() const
{
    return AbsoluteScreenPixelRectangle();
}

void QtInstanceWindow::set_centered_on_parent(bool) {}

bool QtInstanceWindow::has_toplevel_focus() const { return true; }

void QtInstanceWindow::present() {}

void QtInstanceWindow::change_default_widget(weld::Widget*, weld::Widget*) {}

bool QtInstanceWindow::is_default_widget(const weld::Widget*) const { return true; }

void QtInstanceWindow::set_window_state(const OUString&) {}

OUString QtInstanceWindow::get_window_state(vcl::WindowDataMask) const { return OUString(); }

css::uno::Reference<css::awt::XWindow> QtInstanceWindow::GetXWindow()
{
    return css::uno::Reference<css::awt::XWindow>();
}

SystemEnvData QtInstanceWindow::get_system_data() const
{
    const SystemEnvData* pEnvData = nullptr;

    return *pEnvData;
}

void QtInstanceWindow::resize_to_request() {}

weld::ScreenShotCollection QtInstanceWindow::collect_screenshot_data()
{
    return weld::ScreenShotCollection();
}

VclPtr<VirtualDevice> QtInstanceWindow::screenshot() { return nullptr; }

const vcl::ILibreOfficeKitNotifier* QtInstanceWindow::GetLOKNotifier() { return nullptr; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
