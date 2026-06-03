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
class VCL_DLLPUBLIC LinkButton : virtual public Widget
{
    Link<LinkButton&, bool> m_aActivateLinkHdl;

protected:
    bool signal_activate_link() { return m_aActivateLinkHdl.Call(*this); }

public:
    virtual void set_label(const OUString& rText) = 0;
    virtual OUString get_label() const = 0;
    virtual void set_label_wrap(bool wrap) = 0;
    virtual void set_uri(const OUString& rUri) = 0;
    virtual OUString get_uri() const = 0;

    void connect_activate_link(const Link<LinkButton&, bool>& rLink) { m_aActivateLinkHdl = rLink; }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
