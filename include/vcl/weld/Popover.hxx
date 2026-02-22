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
class VCL_DLLPUBLIC Popover : virtual public Container
{
    friend class ::LOKTrigger;

private:
    Link<weld::Popover&, void> m_aCloseHdl;

protected:
    void signal_closed() { m_aCloseHdl.Call(*this); }

public:
    virtual void popup_at_rect(weld::Widget* pParent, const tools::Rectangle& rRect,
                               Placement ePlace = Placement::Under)
        = 0;
    virtual void popdown() = 0;

    virtual void resize_to_request() = 0;

    void connect_closed(const Link<weld::Popover&, void>& rLink) { m_aCloseHdl = rLink; }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
