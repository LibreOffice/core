/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtInstanceContainer.hxx"
#include "QtXWindow.hxx"

class QtInstanceWindow : public QtInstanceContainer, public virtual weld::Window
{
    Q_OBJECT

    rtl::Reference<QtXWindow> m_xWindow;

public:
    QtInstanceWindow(QWidget* pWidget);
    virtual ~QtInstanceWindow();

    virtual void set_title(const OUString& rTitle) override;
    virtual OUString get_title() const override;
    virtual void window_move(int, int) override;
    virtual bool get_resizable() const override;
    virtual Size get_size() const override;
    virtual Point get_position() const override;
    virtual AbsoluteScreenPixelRectangle get_monitor_workarea() const override;

    virtual bool has_toplevel_focus() const override;
    virtual void present() override;

    virtual void change_default_widget(weld::Widget*, weld::Widget*) override;
    virtual bool is_default_widget(const weld::Widget*) const override;

    virtual void set_window_state(const OUString& rStr) override;
    virtual OUString get_window_state(vcl::WindowDataMask eMask) const override;

    virtual css::uno::Reference<css::awt::XWindow> GetXWindow() override;

    virtual SystemEnvData get_system_data() const override;

    virtual void resize_to_request() override;

    virtual weld::ScreenShotCollection collect_screenshot_data() override;

    virtual VclPtr<VirtualDevice> screenshot() override;

    virtual const vcl::ILibreOfficeKitNotifier* GetLOKNotifier() override;

    bool eventFilter(QObject* pObject, QEvent* pEvent) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
