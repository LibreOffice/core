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
#include <vcl/weld/Entry.hxx>

namespace weld
{
class VCL_DLLPUBLIC PatternFormatter final
{
public:
    PatternFormatter(weld::Entry& rEntry);
    ~PatternFormatter();

    weld::Entry& get_widget() { return m_rEntry; }

    void SetMask(const OString& rEditMask, const OUString& rLiteralMask);
    void SetStrictFormat(bool bStrict);
    void ReformatAll();

    /* PatternFormatter will set listeners to "changed", "focus-out", "focus-in"
       and  "key-press" of the Entry so users that want to add their own listeners
       to those must set them through this formatter and not directly on that entry.
    */
    void connect_changed(const Link<weld::Entry&, void>& rLink) { m_aModifyHdl = rLink; }
    void connect_focus_out(const Link<weld::Widget&, void>& rLink) { m_aFocusOutHdl = rLink; }
    void connect_focus_in(const Link<weld::Widget&, void>& rLink) { m_aFocusInHdl = rLink; }
    void connect_key_press(const Link<const KeyEvent&, bool>& rLink) { m_aKeyPressHdl = rLink; }

    SAL_DLLPRIVATE void Modify();

private:
    weld::Entry& m_rEntry;
    Link<weld::Entry&, void> m_aModifyHdl;
    Link<weld::Widget&, void> m_aFocusInHdl;
    Link<weld::Widget&, void> m_aFocusOutHdl;
    Link<const KeyEvent&, bool> m_aKeyPressHdl;
    bool m_bStrictFormat;
    bool m_bSameMask;
    bool m_bReformat;
    bool m_bInPattKeyInput;
    OString m_aEditMask;
    OUString m_aLiteralMask;

    SAL_DLLPRIVATE void EntryGainFocus();
    SAL_DLLPRIVATE void EntryLostFocus();
    DECL_DLLPRIVATE_LINK(ModifyHdl, weld::Entry&, void);
    DECL_DLLPRIVATE_LINK(FocusInHdl, weld::Widget&, void);
    DECL_DLLPRIVATE_LINK(FocusOutHdl, weld::Widget&, void);
    DECL_DLLPRIVATE_LINK(KeyInputHdl, const KeyEvent&, bool);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
