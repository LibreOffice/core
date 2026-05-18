/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/dllapi.h>
#include <vcl/weld/weld.hxx>

namespace weld
{
class VCL_DLLPUBLIC Window : virtual public Container
{
protected:
    Link<Widget&, bool> m_aHelpRequestHdl;

public:
    virtual void set_title(const OUString& rTitle) = 0;
    virtual OUString get_title() const = 0;
    virtual void window_move(int x, int y) = 0;
    virtual bool get_resizable() const = 0;
    virtual Size get_size() const = 0;
    virtual Point get_position() const = 0;

    // returns whether the widget that has focus is within this Window
    // (its very possible to move this to weld::Container if that becomes
    // desirable)
    virtual bool has_toplevel_focus() const = 0;
    virtual void present() = 0;

    virtual void set_window_state(const OUString& rStr) = 0;
    virtual OUString get_window_state(vcl::WindowDataMask nMask) const = 0;

    virtual css::uno::Reference<css::awt::XWindow> GetXWindow() = 0;

    void connect_help(const Link<Widget&, bool>& rLink) { m_aHelpRequestHdl = rLink; }

    virtual SystemEnvData get_system_data() const = 0;

    virtual void resize_to_request() = 0;

    // collect positions of widgets and their help ids for screenshot purposes
    virtual ScreenShotCollection collect_screenshot_data() = 0;

    // render the widget to an output device
    virtual VclPtr<VirtualDevice> screenshot() = 0;

    virtual const vcl::ILibreOfficeKitNotifier* GetLOKNotifier() = 0;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
