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
#include <vcl/weld/Dialog.hxx>

namespace weld
{
class VCL_DLLPUBLIC Assistant : virtual public Dialog
{
    Link<const OUString&, bool> m_aJumpPageHdl;

protected:
    bool signal_jump_page(const OUString& rIdent)
    {
        if (notify_events_disabled())
            return true;
        return m_aJumpPageHdl.Call(rIdent);
    }

    virtual void do_set_current_page(int nPage) = 0;
    virtual void do_set_current_page(const OUString& rIdent) = 0;
    virtual void do_set_page_index(const OUString& rIdent, int nIndex) = 0;
    virtual void do_set_page_title(const OUString& rIdent, const OUString& rTitle) = 0;
    virtual void do_set_page_sensitive(const OUString& rIdent, bool bSensitive) = 0;

public:
    virtual int get_current_page() const = 0;
    virtual int get_n_pages() const = 0;
    virtual OUString get_page_ident(int nPage) const = 0;
    virtual OUString get_current_page_ident() const = 0;

    void set_current_page(int nPage);

    void set_current_page(const OUString& rIdent);

    // move the page rIdent to position nIndex
    void set_page_index(const OUString& rIdent, int nIndex);

    void set_page_title(const OUString& rIdent, const OUString& rTitle);

    virtual OUString get_page_title(const OUString& rIdent) const = 0;

    void set_page_sensitive(const OUString& rIdent, bool bSensitive);

    virtual weld::Container* append_page(const OUString& rIdent) = 0;

    virtual void set_page_side_help_id(const OUString& rHelpId) = 0;

    virtual void set_page_side_image(const OUString& rImage) = 0;

    void connect_jump_page(const Link<const OUString&, bool>& rLink) { m_aJumpPageHdl = rLink; }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
