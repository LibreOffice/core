/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/color.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld/Entry.hxx>
#include <vcl/weld/EntryFormatter.hxx>
#include <vcl/weld/FormattedSpinButton.hxx>

namespace weld
{
EntryFormatter::EntryFormatter(weld::FormattedSpinButton& rSpinButton)
    : m_rEntry(rSpinButton)
    , m_pSpinButton(&rSpinButton)
    , m_eOptions(Application::GetSettings().GetStyleSettings().GetSelectionOptions())
{
    Init();
}

EntryFormatter::EntryFormatter(weld::Entry& rEntry)
    : m_rEntry(rEntry)
    , m_pSpinButton(nullptr)
    , m_eOptions(Application::GetSettings().GetStyleSettings().GetSelectionOptions())
{
    Init();
}

EntryFormatter::~EntryFormatter()
{
    m_rEntry.connect_changed(Link<weld::Entry&, void>());
    m_rEntry.connect_focus_out(Link<weld::Widget&, void>());
    if (m_pSpinButton)
        m_pSpinButton->SetFormatter(nullptr);
}

void EntryFormatter::Init()
{
    m_rEntry.connect_changed(LINK(this, EntryFormatter, ModifyHdl));
    m_rEntry.connect_focus_out(LINK(this, EntryFormatter, FocusOutHdl));
    if (m_pSpinButton)
        m_pSpinButton->SetFormatter(this);
}

Selection EntryFormatter::GetEntrySelection() const
{
    int nStartPos, nEndPos;
    m_rEntry.get_selection_bounds(nStartPos, nEndPos);
    return Selection(nStartPos, nEndPos);
}

OUString EntryFormatter::GetEntryText() const { return m_rEntry.get_text(); }

void EntryFormatter::SetEntryText(const OUString& rText, const Selection& rSel)
{
    m_rEntry.set_text(rText);
    auto nMin = rSel.Min();
    auto nMax = rSel.Max();
    m_rEntry.select_region(nMin < 0 ? 0 : nMin, nMax == SELECTION_MAX ? -1 : nMax);
}

void EntryFormatter::SetEntryTextColor(const Color* pColor)
{
    m_rEntry.set_font_color(pColor ? *pColor : COL_AUTO);
}

void EntryFormatter::UpdateCurrentValue(double dCurrentValue)
{
    Formatter::UpdateCurrentValue(dCurrentValue);
    if (m_pSpinButton)
        m_pSpinButton->sync_value_from_formatter();
}

void EntryFormatter::ClearMinValue()
{
    Formatter::ClearMinValue();
    if (m_pSpinButton)
        m_pSpinButton->sync_range_from_formatter();
}

void EntryFormatter::SetMinValue(double dMin)
{
    Formatter::SetMinValue(dMin);
    if (m_pSpinButton)
        m_pSpinButton->sync_range_from_formatter();
}

void EntryFormatter::ClearMaxValue()
{
    Formatter::ClearMaxValue();
    if (m_pSpinButton)
        m_pSpinButton->sync_range_from_formatter();
}

void EntryFormatter::SetMaxValue(double dMin)
{
    Formatter::SetMaxValue(dMin);
    if (m_pSpinButton)
        m_pSpinButton->sync_range_from_formatter();
}

void EntryFormatter::SetSpinSize(double dStep)
{
    Formatter::SetSpinSize(dStep);
    if (m_pSpinButton)
        m_pSpinButton->sync_increments_from_formatter();
}

SelectionOptions EntryFormatter::GetEntrySelectionOptions() const { return m_eOptions; }

void EntryFormatter::FieldModified() { m_aModifyHdl.Call(m_rEntry); }

IMPL_LINK_NOARG(EntryFormatter, ModifyHdl, weld::Entry&, void)
{
    // This leads to FieldModified getting called at the end of Modify() and
    // FieldModified then calls any modification callback
    Modify();
}

IMPL_LINK_NOARG(EntryFormatter, FocusOutHdl, weld::Widget&, void)
{
    EntryLostFocus();
    if (m_pSpinButton)
        m_pSpinButton->signal_value_changed();
    m_aFocusOutHdl.Call(m_rEntry);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
