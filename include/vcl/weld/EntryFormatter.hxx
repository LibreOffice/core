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
#include <vcl/formatter.hxx>

namespace weld
{
class Entry;
class FormattedSpinButton;
class Widget;

class VCL_DLLPUBLIC EntryFormatter : public Formatter
{
public:
    EntryFormatter(weld::Entry& rEntry);
    EntryFormatter(weld::FormattedSpinButton& rSpinButton);

    weld::Entry& get_widget() { return m_rEntry; }

    // public Formatter overrides, drives interactions with the Entry
    SAL_DLLPRIVATE virtual Selection GetEntrySelection() const override;
    SAL_DLLPRIVATE virtual OUString GetEntryText() const override;
    SAL_DLLPRIVATE virtual void SetEntryText(const OUString& rText, const Selection& rSel) override;
    SAL_DLLPRIVATE virtual void SetEntryTextColor(const Color* pColor) override;
    SAL_DLLPRIVATE virtual SelectionOptions GetEntrySelectionOptions() const override;
    SAL_DLLPRIVATE virtual void FieldModified() override;

    // public Formatter overrides, drives optional SpinButton settings
    SAL_DLLPRIVATE virtual void ClearMinValue() override;
    SAL_DLLPRIVATE virtual void SetMinValue(double dMin) override;
    SAL_DLLPRIVATE virtual void ClearMaxValue() override;
    SAL_DLLPRIVATE virtual void SetMaxValue(double dMin) override;

    SAL_DLLPRIVATE virtual void SetSpinSize(double dStep) override;

    void SetEntrySelectionOptions(SelectionOptions eOptions) { m_eOptions = eOptions; }

    /* EntryFormatter will set listeners to "changed" and "focus-out" of the
       Entry so users that want to add their own listeners to those must set
       them through this formatter and not directly on that entry.

       If EntryFormatter is used with a weld::FormattedSpinButton this is
       handled transparently by the FormattedSpinButton for the user and the
       handlers can be set on the FormattedSpinButton
    */
    void connect_changed(const Link<weld::Entry&, void>& rLink) { m_aModifyHdl = rLink; }
    void connect_focus_out(const Link<weld::Widget&, void>& rLink) { m_aFocusOutHdl = rLink; }

    SAL_DLLPRIVATE virtual ~EntryFormatter() override;

private:
    weld::Entry& m_rEntry;
    weld::FormattedSpinButton* m_pSpinButton;
    Link<weld::Entry&, void> m_aModifyHdl;
    Link<weld::Widget&, void> m_aFocusOutHdl;
    SelectionOptions m_eOptions;
    DECL_DLLPRIVATE_LINK(ModifyHdl, weld::Entry&, void);
    DECL_DLLPRIVATE_LINK(FocusOutHdl, weld::Widget&, void);
    SAL_DLLPRIVATE void Init();

    // private Formatter overrides
    SAL_DLLPRIVATE virtual void UpdateCurrentValue(double dCurrentValue) override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
