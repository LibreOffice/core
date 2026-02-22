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
class VCL_DLLPUBLIC TextView : virtual public TextWidget
{
    friend class ::LOKTrigger;

protected:
    Link<TextView&, void> m_aChangeHdl;
    Link<TextView&, void> m_aVValueChangeHdl;

    void signal_changed() { m_aChangeHdl.Call(*this); }

    void signal_vadjustment_value_changed() { m_aVValueChangeHdl.Call(*this); }

public:
    virtual void set_monospace(bool bMonospace) = 0;
    // The maximum length of the entry. Use 0 for no maximum
    virtual void set_max_length(int nChars) = 0;
    int get_height_rows(int nRows) const;

    void connect_changed(const Link<TextView&, void>& rLink) { m_aChangeHdl = rLink; }

    // returns true if pressing up would move the cursor
    // doesn't matter if that move is to a previous line or to the start of the
    // current line just so long as the cursor would move
    virtual bool can_move_cursor_with_up() const = 0;

    // returns true if pressing down would move the cursor
    // doesn't matter if that move is to a next line or to the end of the
    // current line just so long as the cursor would move
    virtual bool can_move_cursor_with_down() const = 0;

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
