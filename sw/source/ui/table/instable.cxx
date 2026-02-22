/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <instable.hxx>
#include <shellres.hxx>
#include <modcfg.hxx>
#include <swmodule.hxx>
#include <sfx2/htmlmode.hxx>
#include <viewopt.hxx>
#include <comphelper/lok.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/Dialog.hxx>


void SwInsTableDlg::GetValues( OUString& rName, sal_uInt16& rRow, sal_uInt16& rCol,
                                SwInsertTableOptions& rInsTableOpts, OUString& rAutoName,
                                std::unique_ptr<SwTableAutoFormat>& prTAFormat )
{
    SwInsertTableFlags nInsMode = SwInsertTableFlags::NONE;
    if (comphelper::LibreOfficeKit::isActive())
        nInsMode = SwInsertTableFlags::DefaultBorder;
    rName = m_xNameEdit->get_text();
    rRow = m_xRowSpinButton->get_value();
    rCol = m_xColSpinButton->get_value();

    if (m_xHeaderCB->get_active())
        nInsMode |= SwInsertTableFlags::Headline;
    if (m_xRepeatHeaderCB->get_sensitive() && m_xRepeatHeaderCB->get_active())
        rInsTableOpts.mnRowsToRepeat = m_xRepeatHeaderNF->get_value();
    else
        rInsTableOpts.mnRowsToRepeat = 0;
    if (!m_xDontSplitCB->get_active())
        nInsMode |= SwInsertTableFlags::SplitLayout;
    if (m_xTAutoFormat && !comphelper::LibreOfficeKit::isActive())
    {
        prTAFormat.reset(new SwTableAutoFormat( *m_xTAutoFormat ));
        rAutoName = prTAFormat->GetName().toString();
    }

    rInsTableOpts.mnInsMode = nInsMode;
}

IMPL_LINK(SwInsTableDlg, TextFilterHdl, OUString&, rTest, bool)
{
    rTest = m_aTextFilter.filter(rTest);
    return true;
}

SwInsTableDlg::SwInsTableDlg(SwView& rView)
    : SfxDialogController(rView.GetFrameWeld(), u"modules/swriter/ui/inserttable.ui"_ustr,
                          u"InsertTableDialog"_ustr)
    , m_aTextFilter(u" .<>"_ustr)
    , m_pShell(&rView.GetWrtShell())
    , m_nEnteredValRepeatHeaderNF(-1)
    , m_aWndPreview(rView.GetWrtShell().IsCursorInTable() ? rView.GetWrtShell().IsTableRightToLeft()
                                                          : AllSettings::GetLayoutRTL())
    , m_xNameEdit(m_xBuilder->weld_entry(u"nameedit"_ustr))
    , m_xWarning(m_xBuilder->weld_label(u"lbwarning"_ustr))
    , m_xColSpinButton(m_xBuilder->weld_spin_button(u"colspin"_ustr))
    , m_xRowSpinButton(m_xBuilder->weld_spin_button(u"rowspin"_ustr))
    , m_xHeaderCB(m_xBuilder->weld_check_button(u"headercb"_ustr))
    , m_xRepeatHeaderCB(m_xBuilder->weld_check_button(u"repeatcb"_ustr))
    , m_xRepeatHeaderNF(m_xBuilder->weld_spin_button(u"repeatheaderspin"_ustr))
    , m_xRepeatGroup(m_xBuilder->weld_widget(u"repeatgroup"_ustr))
    , m_xDontSplitCB(m_xBuilder->weld_check_button(u"dontsplitcb"_ustr))
    , m_xInsertBtn(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xLbFormat(m_xBuilder->weld_tree_view(u"formatlbinstable"_ustr))
    , m_xWndPreview(new weld::CustomWeld(*m_xBuilder, u"previewinstable"_ustr, m_aWndPreview))
    , m_xStyleFrame(m_xBuilder->weld_frame(u"stylesframe"_ustr))
{
    if (comphelper::LibreOfficeKit::isActive())
        m_xStyleFrame->hide();

    const int nWidth = m_xLbFormat->get_approximate_digit_width() * 32;
    const int nHeight = m_xLbFormat->get_height_rows(8);
    m_xLbFormat->set_size_request(nWidth, nHeight);
    m_xWndPreview->set_size_request(nWidth, nHeight);

    m_xNameEdit->connect_insert_text(LINK(this, SwInsTableDlg, TextFilterHdl));
    m_xNameEdit->set_text(m_pShell->GetUniqueTableName().toString());
    m_xNameEdit->connect_changed(LINK(this, SwInsTableDlg, ModifyName));
    m_xRowSpinButton->connect_changed(LINK(this, SwInsTableDlg, ModifyRowCol));
    m_xColSpinButton->connect_changed(LINK(this, SwInsTableDlg, ModifyRowCol));

    m_xInsertBtn->connect_clicked(LINK(this, SwInsTableDlg, OKHdl));

    bool bHTMLMode = 0 != (::GetHtmlMode(rView.GetDocShell())&HTMLMODE_ON);
    const SwModuleOptions* pModOpt = SwModule::get()->GetModuleConfig();

    SwInsertTableOptions aInsOpts = pModOpt->GetInsTableFlags(bHTMLMode);
    SwInsertTableFlags nInsTableFlags = aInsOpts.mnInsMode;

    m_xHeaderCB->set_active(bool(nInsTableFlags & SwInsertTableFlags::Headline));
    m_xRepeatHeaderCB->set_active(aInsOpts.mnRowsToRepeat > 0);
    if (bHTMLMode)
        m_xDontSplitCB->hide();
    else
        m_xDontSplitCB->set_active(!(nInsTableFlags & SwInsertTableFlags::SplitLayout));

    m_xRepeatHeaderNF->connect_value_changed( LINK( this, SwInsTableDlg, ModifyRepeatHeaderNF_Hdl ) );
    m_xHeaderCB->connect_toggled( LINK( this, SwInsTableDlg, CheckBoxHdl ) );
    m_xRepeatHeaderCB->connect_toggled( LINK( this, SwInsTableDlg, RepeatHeaderCheckBoxHdl ) );
    RepeatHeaderCheckBoxHdl(*m_xRepeatHeaderCB);
    CheckBoxHdl(*m_xHeaderCB);

    sal_Int64 nMax = m_xRowSpinButton->get_value();
    if( nMax <= 1 )
        nMax = 1;
    else
        --nMax;
    m_xRepeatHeaderNF->set_max( nMax );

    InitAutoTableFormat();
    m_xWarning->set_label_type(weld::LabelType::Warning);
}

void SwInsTableDlg::InitAutoTableFormat()
{
    m_xLbFormat->connect_selection_changed(LINK(this, SwInsTableDlg, SelFormatHdl));

    m_xTableTable.reset(new SwTableAutoFormatTable);

    for (size_t i = 0; i < m_xTableTable->size(); i++)
    {
        m_xLbFormat->append_text((*m_xTableTable)[i].GetName().toString());
    }

    m_xLbFormat->select(0);

    SelFormatHdl( *m_xLbFormat );
}

IMPL_LINK_NOARG(SwInsTableDlg, SelFormatHdl, weld::TreeView&, void)
{
    // Get index of selected item from the listbox
    int styleIdx = m_xLbFormat->get_selected_index();
    assert(styleIdx != -1 && "nothing selected");
    m_aWndPreview.NotifyChange(m_xTableTable->GetResolvedStyle(m_xTableTable->GetData(styleIdx)));
}

IMPL_LINK_NOARG(SwInsTableDlg, OKHdl, weld::Button&, void)
{
    int styleIdx = m_xLbFormat->get_selected_index();
    assert(styleIdx != -1 && "nothing selected");
    // The value 0 is used for the "None" style
    if (styleIdx > 0)
        m_pShell->SetTableStyle((*m_xTableTable)[styleIdx]);

    if( m_xTAutoFormat )
        *m_xTAutoFormat = (*m_xTableTable)[styleIdx];
    else
        m_xTAutoFormat.reset(new SwTableAutoFormat((*m_xTableTable)[styleIdx]));

    m_xDialog->response(RET_OK);
}

IMPL_LINK( SwInsTableDlg, ModifyName, weld::Entry&, rEdit, void )
{
    OUString sTableName = rEdit.get_text();
    m_xInsertBtn->set_sensitive(m_pShell->GetTableStyle(UIName(sTableName)) == nullptr);
}

// We use weld::Entry's "changed" notification here, not weld::SpinButton's "value_changed", because
// the latter only fires after the control looses focus; so the notification would not fire during
// typing a big number, so that user typing it and immediately clicking "Insert" would not see the
// warning.
// Since the notification is called in weld::Entry context, we can only rely on what's available for
// used weld::Entry's notification; specifically, we have to call spin buttons' get_text() instead
// of get_value(), because the latter is not guaranteed to return an up-to-date value at this point
// (depends on vcl plugin used).
IMPL_LINK( SwInsTableDlg, ModifyRowCol, weld::Entry&, rEdit, void )
{
    sal_Int64 nRow = m_xRowSpinButton->get_text().toInt64();
    sal_Int64 nCol = m_xColSpinButton->get_text().toInt64();
    if (nRow > 255)
    {
        m_xRowSpinButton->set_message_type(weld::EntryMessageType::Warning);
        m_xWarning->set_visible(true);
    }
    else
    {
        m_xRowSpinButton->set_message_type(weld::EntryMessageType::Normal);
    }
    if (nCol > 63)
    {
        m_xColSpinButton->set_message_type(weld::EntryMessageType::Warning);
        m_xWarning->set_visible(true);
    }
    else
    {
        m_xColSpinButton->set_message_type(weld::EntryMessageType::Normal);
    }
    if (nRow <= 255 && nCol <= 63)
    {
        m_xWarning->set_visible(false);
    }
    if (&rEdit == m_xColSpinButton.get())
        return;

    if(!nRow)
        nRow = 1;

    // adjust depending NF for repeated rows
    sal_Int64 nMax = ( nRow == 1 )? 1 : nRow - 1 ;
    sal_Int64 nActVal = m_xRepeatHeaderNF->get_value();

    m_xRepeatHeaderNF->set_max( nMax );

    if( nActVal > nMax )
        m_xRepeatHeaderNF->set_value( nMax );
    else if( nActVal < m_nEnteredValRepeatHeaderNF )
        m_xRepeatHeaderNF->set_value(std::min(m_nEnteredValRepeatHeaderNF, nMax));
}

IMPL_LINK_NOARG(SwInsTableDlg, CheckBoxHdl, weld::Toggleable&, void)
{
    m_xRepeatHeaderCB->set_sensitive(m_xHeaderCB->get_active());
    RepeatHeaderCheckBoxHdl(*m_xRepeatHeaderCB);
}

IMPL_LINK_NOARG(SwInsTableDlg, RepeatHeaderCheckBoxHdl, weld::Toggleable&, void)
{
    m_xRepeatGroup->set_sensitive(m_xHeaderCB->get_active() && m_xRepeatHeaderCB->get_active());
}

IMPL_LINK_NOARG(SwInsTableDlg, ModifyRepeatHeaderNF_Hdl, weld::SpinButton&, void)
{
    m_nEnteredValRepeatHeaderNF = m_xRepeatHeaderNF->get_value();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
