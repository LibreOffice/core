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

namespace weld
{
class VCL_DLLPUBLIC Toggleable : virtual public Widget
{
protected:
    Link<Toggleable&, void> m_aToggleHdl;
    TriState m_eSavedValue = TRISTATE_FALSE;

    void signal_toggled()
    {
        if (notify_events_disabled())
            return;
        m_aToggleHdl.Call(*this);
    }

    virtual void do_set_active(bool active) = 0;

public:
    void set_active(bool active)
    {
        disable_notify_events();
        do_set_active(active);
        enable_notify_events();
    }

    virtual bool get_active() const = 0;

    virtual TriState get_state() const
    {
        if (get_active())
            return TRISTATE_TRUE;
        return TRISTATE_FALSE;
    }

    void save_state() { m_eSavedValue = get_state(); }
    TriState get_saved_state() const { return m_eSavedValue; }
    bool get_state_changed_from_saved() const { return m_eSavedValue != get_state(); }

    virtual void connect_toggled(const Link<Toggleable&, void>& rLink) { m_aToggleHdl = rLink; }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
