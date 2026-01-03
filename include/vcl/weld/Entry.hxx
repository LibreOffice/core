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
class VCL_DLLPUBLIC Entry : virtual public Widget
{
private:
    OUString m_sSavedValue;

protected:
    Link<Entry&, void> m_aChangeHdl;
    Link<OUString&, bool> m_aInsertTextHdl;
    Link<Entry&, void> m_aCursorPositionHdl;
    Link<Entry&, bool> m_aActivateHdl;

    friend class ::LOKTrigger;

    void signal_changed();

    void signal_activated();

    void signal_cursor_position();

    virtual void do_set_text(const OUString& rText) = 0;
    virtual void do_select_region(int nStartPos, int nEndPos) = 0;
    virtual void do_set_position(int nCursorPos) = 0;

public:
    void set_text(const OUString& rText);

    virtual OUString get_text() const = 0;
    virtual void set_width_chars(int nChars) = 0;
    virtual int get_width_chars() const = 0;
    // The maximum length of the entry. Use 0 for no maximum
    virtual void set_max_length(int nChars) = 0;

    // nEndPos can be -1 in order to select all text
    void select_region(int nStartPos, int nEndPos);

    // returns true if the selection has nonzero length
    virtual bool get_selection_bounds(int& rStartPos, int& rEndPos) = 0;
    virtual void replace_selection(const OUString& rText) = 0;

    // nCursorPos can be -1 to set to the end
    void set_position(int nCursorPos);

    virtual int get_position() const = 0;
    virtual void set_editable(bool bEditable) = 0;
    virtual bool get_editable() const = 0;
    virtual void set_visibility(bool bVisible) = 0;
    virtual void set_message_type(EntryMessageType eType) = 0;
    virtual void set_placeholder_text(const OUString& rText) = 0;

    virtual void set_overwrite_mode(bool bOn) = 0;
    virtual bool get_overwrite_mode() const = 0;

    // font size is in points, not pixels, e.g. see Window::[G]etPointFont
    virtual void set_font(const vcl::Font& rFont) = 0;

    /*
       If you want to set a warning or error state, see set_message_type
       instead where, if the toolkit supports it, a specific warning/error
       indicator is shown.

       This explicit text color method exists to support rendering the
       SvNumberformat color feature.
    */
    virtual void set_font_color(const Color& rColor) = 0;

    virtual void connect_changed(const Link<Entry&, void>& rLink) { m_aChangeHdl = rLink; }
    void connect_insert_text(const Link<OUString&, bool>& rLink) { m_aInsertTextHdl = rLink; }
    // callback returns true to indicated no further processing of activate wanted
    void connect_activate(const Link<Entry&, bool>& rLink) { m_aActivateHdl = rLink; }
    virtual void connect_cursor_position(const Link<Entry&, void>& rLink)
    {
        m_aCursorPositionHdl = rLink;
    }

    virtual void cut_clipboard() = 0;
    virtual void copy_clipboard() = 0;
    virtual void paste_clipboard() = 0;

    virtual void set_alignment(TxtAlign eXAlign) = 0;

    void save_value() { m_sSavedValue = get_text(); }
    OUString const& get_saved_value() const { return m_sSavedValue; }
    bool get_value_changed_from_saved() const { return m_sSavedValue != get_text(); }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
