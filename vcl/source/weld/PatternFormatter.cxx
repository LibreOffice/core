/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/weld/PatternFormatter.hxx>

namespace weld
{
PatternFormatter::PatternFormatter(weld::Entry& rEntry)
    : m_rEntry(rEntry)
    , m_bStrictFormat(false)
    , m_bSameMask(true)
    , m_bReformat(false)
    , m_bInPattKeyInput(false)
{
    m_rEntry.connect_changed(LINK(this, PatternFormatter, ModifyHdl));
    m_rEntry.connect_focus_in(LINK(this, PatternFormatter, FocusInHdl));
    m_rEntry.connect_focus_out(LINK(this, PatternFormatter, FocusOutHdl));
    m_rEntry.connect_key_press(LINK(this, PatternFormatter, KeyInputHdl));
}

IMPL_LINK_NOARG(PatternFormatter, ModifyHdl, weld::Entry&, void) { Modify(); }

IMPL_LINK_NOARG(PatternFormatter, FocusOutHdl, weld::Widget&, void)
{
    EntryLostFocus();
    m_aFocusOutHdl.Call(m_rEntry);
}

IMPL_LINK_NOARG(PatternFormatter, FocusInHdl, weld::Widget&, void)
{
    EntryGainFocus();
    m_aFocusInHdl.Call(m_rEntry);
}

PatternFormatter::~PatternFormatter()
{
    m_rEntry.connect_changed(Link<weld::Entry&, void>());
    m_rEntry.connect_focus_out(Link<weld::Widget&, void>());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
