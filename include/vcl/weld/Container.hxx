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
#include <vcl/weld/Widget.hxx>

namespace com::sun::star::awt
{
class XWindow;
}

namespace weld
{
class VCL_DLLPUBLIC Container : virtual public Widget
{
    Link<Container&, void> m_aContainerFocusChangedHdl;

protected:
    void signal_container_focus_changed() { m_aContainerFocusChangedHdl.Call(*this); }

public:
    // remove from old container and add to new container in one go
    // new container can be null to just remove from old container
    virtual void move(weld::Widget* pWidget, weld::Container* pNewParent) = 0;
    // create an XWindow as a child of this container. The XWindow is
    // suitable to contain css::awt::XControl items
    virtual css::uno::Reference<css::awt::XWindow> CreateChildFrame() = 0;
    // rLink is called when the focus transitions from a widget outside the container
    // to a widget inside the container or vice versa
    virtual void connect_container_focus_changed(const Link<Container&, void>& rLink)
    {
        m_aContainerFocusChangedHdl = rLink;
    }
    // causes a child of the container to have the keyboard focus
    virtual void child_grab_focus() = 0;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
