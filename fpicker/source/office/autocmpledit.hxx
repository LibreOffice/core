/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/idle.hxx>
#include <vcl/weld.hxx>
#include <vector>

class AutocompleteEdit
{
private:
    std::unique_ptr<weld::Entry> m_xEntry;

    std::vector<OUString> m_aEntries;
    std::vector<OUString> m_aMatching;
    Idle m_aChangedIdle;
    Link<weld::Entry&, void> m_aChangeHdl;

    DECL_LINK(ChangedHdl, weld::Entry&, void);
    DECL_LINK(TryAutoComplete, Timer*, void);

    bool Match(std::u16string_view rText);

public:
    AutocompleteEdit(std::unique_ptr<weld::Entry> xEntry);

    void show() { m_xEntry->show(); }
    void set_sensitive(bool bSensitive) { m_xEntry->set_sensitive(bSensitive); }
    OUString get_text() const { return m_xEntry->get_text(); }
    void set_text(const OUString& rText) { m_xEntry->set_text(rText); }
    void grab_focus() { m_xEntry->grab_focus(); }
    void select_region(int nStartPos, int nEndPos) { m_xEntry->select_region(nStartPos, nEndPos); }

    void connect_changed(const Link<weld::Entry&, void>& rLink) { m_aChangeHdl = rLink; }
    void connect_focus_in(const Link<weld::Widget&, void>& rLink)
    {
        m_xEntry->connect_focus_in(rLink);
    }

    void AddEntry(const OUString& rEntry);
    void ClearEntries();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
