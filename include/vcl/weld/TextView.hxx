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
class VCL_DLLPUBLIC TextView : virtual public Widget
{
    friend class ::LOKTrigger;

private:
    OUString m_sSavedValue;

protected:
    Link<TextView&, void> m_aChangeHdl;
    Link<TextView&, void> m_aVValueChangeHdl;
    Link<TextView&, void> m_aCursorPositionHdl;

    void signal_changed() { m_aChangeHdl.Call(*this); }

    void signal_cursor_position()
    {
        if (notify_events_disabled())
            return;
        m_aCursorPositionHdl.Call(*this);
    }

    void signal_vadjustment_value_changed() { m_aVValueChangeHdl.Call(*this); }

    virtual void do_set_text(const OUString& rText) = 0;
    virtual void do_select_region(int nStartPos, int nEndPos) = 0;
    virtual void do_replace_selection(const OUString& rText) = 0;

public:
    void set_text(const OUString& rText);
    virtual OUString get_text() const = 0;

    // if nStartPos or nEndPos is -1 the max available text pos will be used
    void select_region(int nStartPos, int nEndPos);

    // returns true if the selection has nonzero length
    virtual bool get_selection_bounds(int& rStartPos, int& rEndPos) = 0;

    void replace_selection(const OUString& rText);

    virtual void set_editable(bool bEditable) = 0;
    virtual bool get_editable() const = 0;
    virtual void set_monospace(bool bMonospace) = 0;
    // The maximum length of the entry. Use 0 for no maximum
    virtual void set_max_length(int nChars) = 0;
    int get_height_rows(int nRows) const;

    // font size is in points, not pixels, e.g. see Window::[G]etPointFont
    virtual void set_font(const vcl::Font& rFont) = 0;

    /*
       Typically you want to avoid the temptation of customizing
       font colors
    */
    virtual void set_font_color(const Color& rColor) = 0;

    void save_value() { m_sSavedValue = get_text(); }
    bool get_value_changed_from_saved() const { return m_sSavedValue != get_text(); }

    void connect_changed(const Link<TextView&, void>& rLink) { m_aChangeHdl = rLink; }
    virtual void connect_cursor_position(const Link<TextView&, void>& rLink)
    {
        m_aCursorPositionHdl = rLink;
    }

    // returns true if pressing up would move the cursor
    // doesn't matter if that move is to a previous line or to the start of the
    // current line just so long as the cursor would move
    virtual bool can_move_cursor_with_up() const = 0;

    // returns true if pressing down would move the cursor
    // doesn't matter if that move is to a next line or to the end of the
    // current line just so long as the cursor would move
    virtual bool can_move_cursor_with_down() const = 0;

    virtual void cut_clipboard() = 0;
    virtual void copy_clipboard() = 0;
    virtual void paste_clipboard() = 0;

    virtual void set_alignment(TxtAlign eXAlign) = 0;

    virtual int vadjustment_get_value() const = 0;
    virtual int vadjustment_get_upper() const = 0;
    virtual int vadjustment_get_page_size() const = 0;
    virtual void vadjustment_set_value(int value) = 0;
    void connect_vadjustment_value_changed(const Link<TextView&, void>& rLink)
    {
        m_aVValueChangeHdl = rLink;
    }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
