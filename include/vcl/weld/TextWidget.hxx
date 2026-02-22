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
/* Abstract base class for Entry and TextView. */
class VCL_DLLPUBLIC TextWidget : virtual public Widget
{
private:
    OUString m_sSavedValue;

protected:
    Link<TextWidget&, void> m_aCursorPositionHdl;

    void signal_cursor_position();

    virtual void do_set_text(const OUString& rText) = 0;
    virtual void do_set_position(int nCursorPos) = 0;
    virtual void do_select_region(int nStartPos, int nEndPos) = 0;
    virtual void do_replace_selection(const OUString& rText) = 0;

public:
    void set_text(const OUString& rText);
    virtual OUString get_text() const = 0;

    // nCursorPos can be -1 to set to the end
    void set_position(int nCursorPos);
    virtual int get_position() const = 0;

    // if nStartPos or nEndPos is -1 the max available text pos will be used
    void select_region(int nStartPos, int nEndPos);

    void replace_selection(const OUString& rText);

    // returns true if the selection has nonzero length
    virtual bool get_selection_bounds(int& rStartPos, int& rEndPos) = 0;

    virtual void set_editable(bool bEditable) = 0;
    virtual bool get_editable() const = 0;

    // font size is in points, not pixels, e.g. see Window::[G]etPointFont
    virtual void set_font(const vcl::Font& rFont) = 0;

    /*
       Typically you want to avoid the temptation of customizing
       font colors.

       For the Entry subclass, if you want to set a warning or error state,
       see Entry::set_message_type instead where, if the toolkit supports it,
       a specific warning/error indicator is shown.
    */
    virtual void set_font_color(const Color& rColor) = 0;

    virtual void cut_clipboard();
    virtual void copy_clipboard();
    virtual void paste_clipboard();

    void save_value() { m_sSavedValue = get_text(); }
    OUString const& get_saved_value() const { return m_sSavedValue; }
    bool get_value_changed_from_saved() const { return m_sSavedValue != get_text(); }

    virtual void connect_cursor_position(const Link<TextWidget&, void>& rLink)
    {
        m_aCursorPositionHdl = rLink;
    }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
