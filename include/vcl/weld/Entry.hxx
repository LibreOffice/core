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
#include <vcl/weld/TextWidget.hxx>

enum class TxtAlign;

namespace weld
{
class VCL_DLLPUBLIC Entry : virtual public TextWidget
{
protected:
    Link<Entry&, void> m_aChangeHdl;
    Link<OUString&, bool> m_aInsertTextHdl;
    Link<Entry&, bool> m_aActivateHdl;

    friend class ::LOKTrigger;

    void signal_changed();

    void signal_activated();

public:
    virtual void set_width_chars(int nChars) = 0;
    virtual int get_width_chars() const = 0;
    // The maximum length of the entry. Use 0 for no maximum
    virtual void set_max_length(int nChars) = 0;

    virtual void set_visibility(bool bVisible) = 0;
    virtual void set_message_type(EntryMessageType eType) = 0;
    virtual void set_placeholder_text(const OUString& rText) = 0;

    virtual void set_overwrite_mode(bool bOn) = 0;
    virtual bool get_overwrite_mode() const = 0;

    virtual void connect_changed(const Link<Entry&, void>& rLink) { m_aChangeHdl = rLink; }
    void connect_insert_text(const Link<OUString&, bool>& rLink) { m_aInsertTextHdl = rLink; }
    // callback returns true to indicated no further processing of activate wanted
    void connect_activate(const Link<Entry&, bool>& rLink) { m_aActivateHdl = rLink; }

    virtual void set_alignment(TxtAlign eXAlign) = 0;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
