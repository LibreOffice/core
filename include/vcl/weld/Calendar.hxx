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

class Date;

namespace weld
{
class VCL_DLLPUBLIC Calendar : virtual public Widget
{
    Link<Calendar&, void> m_aSelectedHdl;
    Link<Calendar&, void> m_aActivatedHdl;

protected:
    void signal_selected()
    {
        if (notify_events_disabled())
            return;
        m_aSelectedHdl.Call(*this);
    }

    void signal_activated()
    {
        if (notify_events_disabled())
            return;
        m_aActivatedHdl.Call(*this);
    }

public:
    void connect_selected(const Link<Calendar&, void>& rLink) { m_aSelectedHdl = rLink; }
    void connect_activated(const Link<Calendar&, void>& rLink) { m_aActivatedHdl = rLink; }

    virtual void set_date(const Date& rDate) = 0;
    virtual Date get_date() const = 0;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
