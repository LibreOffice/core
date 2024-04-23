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

#include <modcfg.hxx>
#include <sfx2/htmlmode.hxx>
#include <viewopt.hxx>
#include <swmodule.hxx>
#include <convert.hxx>
#include <tablemgr.hxx>
#include <view.hxx>
#include <tblafmt.hxx>

#include <strings.hrc>
#include <swabstdlg.hxx>
#include <swuiexp.hxx>
#include <memory>

#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <sal/log.hxx>
#include <shellres.hxx>

//keep the state of the buttons on runtime
static int nSaveButtonState = -1; // 0: tab, 1: semicolon, 2: paragraph, 3: other, -1: not yet used
static bool bIsKeepColumn = true;
static sal_Unicode uOther = ',';

void SwConvertTableDlg::GetValues(sal_Unicode& rDelim, SwInsertTableOptions& rInsTableOpts,
                                  SwTableAutoFormat const*& prTAFormat)
{
    if (m_xTabBtn->get_active())
    {
        //0x0b mustn't be set when re-converting table into text
        bIsKeepColumn = !m_xKeepColumn->get_visible() || m_xKeepColumn->get_active();
        rDelim = bIsKeepColumn ? 0x09 : 0x0b;
        nSaveButtonState = 0;
    }
    else if (m_xSemiBtn->get_active())
    {
        rDelim = ';';
        nSaveButtonState = 1;
    }
    else if (m_xOtherBtn->get_active() && !m_xOtherEd->get_text().isEmpty())
    {
        uOther = m_xOtherEd->get_text()[0];
        rDelim = uOther;
        nSaveButtonState = 3;
    }
    else
    {
        nSaveButtonState = 2;
        rDelim = cParaDelim;
        if (m_xOtherBtn->get_active())
        {
            nSaveButtonState = 3;
            uOther = 0;
        }
    }

    SwInsertTableFlags nInsMode = SwInsertTableFlags::NONE;
    if (m_xHeaderCB->get_active())
        nInsMode |= SwInsertTableFlags::Headline;
    if (m_xRepeatHeaderCB->get_sensitive() && m_xRepeatHeaderCB->get_active())
        rInsTableOpts.mnRowsToRepeat = m_xRepeatHeaderNF->get_value();
    else
        rInsTableOpts.mnRowsToRepeat = 0;
    if (!m_xDontSplitCB->get_active())
        nInsMode |= SwInsertTableFlags::SplitLayout;

    if (mxTAutoFormat)
        prTAFormat = new SwTableAutoFormat(*mxTAutoFormat);

    rInsTableOpts.mnInsMode = nInsMode;
}

SwConvertTableDlg::SwConvertTableDlg(SwView& rView, bool bToTable)
    : SfxDialogController(rView.GetFrameWeld(), "modules/swriter/ui/converttexttable.ui",
                          "ConvertTextTableDialog")
    , m_aStrTitle(SwResId(STR_ADD_AUTOFORMAT_TITLE))
    , m_aStrLabel(SwResId(STR_ADD_AUTOFORMAT_LABEL))
    , m_aStrClose(SwResId(STR_BTN_AUTOFORMAT_CLOSE))
    , m_aStrDelTitle(SwResId(STR_DEL_AUTOFORMAT_TITLE))
    , m_aStrDelMsg(SwResId(STR_DEL_AUTOFORMAT_MSG))
    , m_aStrRenameTitle(SwResId(STR_RENAME_AUTOFORMAT_TITLE))
    , m_aStrInvalidFormat(SwResId(STR_INVALID_AUTOFORMAT_NAME))
    , m_nIndex(0)
    , m_nDfltStylePos(0)
    , m_bCoreDataChanged(false)
    , m_xTableTable(new SwTableAutoFormatTable)
    , m_xTabBtn(m_xBuilder->weld_radio_button("tabs"))
    , m_xSemiBtn(m_xBuilder->weld_radio_button("semicolons"))
    , m_xParaBtn(m_xBuilder->weld_radio_button("paragraph"))
    , m_xOtherBtn(m_xBuilder->weld_radio_button("other"))
    , m_xOtherEd(m_xBuilder->weld_entry("othered"))
    , m_xKeepColumn(m_xBuilder->weld_check_button("keepcolumn"))
    , m_xOptions(m_xBuilder->weld_container("options"))
    , m_xHeaderCB(m_xBuilder->weld_check_button("headingcb"))
    , m_xRepeatHeaderCB(m_xBuilder->weld_check_button("repeatheading"))
    , m_xRepeatRows(m_xBuilder->weld_container("repeatrows"))
    , m_xRepeatHeaderNF(m_xBuilder->weld_spin_button("repeatheadersb"))
    , m_xDontSplitCB(m_xBuilder->weld_check_button("dontsplitcb"))
    , m_xLbFormat(m_xBuilder->weld_tree_view("formatlb"))
    , m_xBtnNumFormat(m_xBuilder->weld_check_button("numformatcb"))
    , m_xBtnBorder(m_xBuilder->weld_check_button("bordercb"))
    , m_xBtnFont(m_xBuilder->weld_check_button("fontcb"))
    , m_xBtnPattern(m_xBuilder->weld_check_button("patterncb"))
    , m_xBtnAlignment(m_xBuilder->weld_check_button("alignmentcb"))
    , m_xWndPreview(new weld::CustomWeld(*m_xBuilder, "preview", m_aWndPreview))
    , m_pShell(&rView.GetWrtShell())
{
    m_aWndPreview.DetectRTL(&rView.GetWrtShell());
    m_xTableTable->Load();

    const int nWidth = m_xLbFormat->get_approximate_digit_width() * 32;
    const int nHeight = m_xLbFormat->get_height_rows(8);
    m_xLbFormat->set_size_request(nWidth, nHeight);
    m_xWndPreview->set_size_request(nWidth, nHeight);

    if (nSaveButtonState > -1)
    {
        switch (nSaveButtonState)
        {
            case 0:
                m_xTabBtn->set_active(true);
                m_xKeepColumn->set_active(bIsKeepColumn);
                break;
            case 1:
                m_xSemiBtn->set_active(true);
                break;
            case 2:
                m_xParaBtn->set_active(true);
                break;
            case 3:
                m_xOtherBtn->set_active(true);
                if (uOther)
                    m_xOtherEd->set_text(OUString(uOther));
                break;
        }
    }
    if (bToTable)
    {
        m_xDialog->set_title(SwResId(STR_CONVERT_TEXT_TABLE));
        m_xKeepColumn->show();
        m_xKeepColumn->set_sensitive(m_xTabBtn->get_active());
    }
    else
    {
        //hide insert options
        m_xOptions->hide();
    }
    m_xKeepColumn->save_state();

    Link<weld::Toggleable&, void> aLk(LINK(this, SwConvertTableDlg, BtnHdl));
    m_xTabBtn->connect_toggled(aLk);
    m_xSemiBtn->connect_toggled(aLk);
    m_xParaBtn->connect_toggled(aLk);
    m_xOtherBtn->connect_toggled(aLk);
    m_xOtherEd->set_sensitive(m_xOtherBtn->get_active());

    const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    bool bHTMLMode = 0 != (::GetHtmlMode(rView.GetDocShell()) & HTMLMODE_ON);

    SwInsertTableOptions aInsOpts = pModOpt->GetInsTableFlags(bHTMLMode);
    SwInsertTableFlags nInsTableFlags = aInsOpts.mnInsMode;

    m_xHeaderCB->set_active(bool(nInsTableFlags & SwInsertTableFlags::Headline));
    m_xRepeatHeaderCB->set_active(aInsOpts.mnRowsToRepeat > 0);
    m_xDontSplitCB->set_active(!(nInsTableFlags & SwInsertTableFlags::SplitLayout));

    m_xHeaderCB->connect_toggled(LINK(this, SwConvertTableDlg, CheckBoxHdl));
    m_xRepeatHeaderCB->connect_toggled(LINK(this, SwConvertTableDlg, RepeatHeaderCheckBoxHdl));
    RepeatHeaderCheckBoxHdl(*m_xRepeatHeaderCB);
    CheckBoxHdl(*m_xHeaderCB);
    Init();
}

SwConvertTableDlg::~SwConvertTableDlg()
{
    try
    {
        if (m_bCoreDataChanged)
            m_xTableTable->Save();
    }
    catch (...)
    {
    }
    m_xTableTable.reset();
}

void SwConvertTableDlg::Init()
{
    const SwTableAutoFormat* pSelFormat = mxTAutoFormat.get();
    Link<weld::Toggleable&, void> aLk(LINK(this, SwConvertTableDlg, CheckHdl));
    m_xBtnBorder->connect_toggled(aLk);
    m_xBtnFont->connect_toggled(aLk);
    m_xBtnPattern->connect_toggled(aLk);
    m_xBtnAlignment->connect_toggled(aLk);
    m_xBtnNumFormat->connect_toggled(aLk);

    m_xLbFormat->connect_changed(LINK(this, SwConvertTableDlg, SelFormatHdl));

    // Then the list to be expanded by the entry "- none -".
    m_xLbFormat->append_text(SwViewShell::GetShellRes()->aStrNone);
    m_nDfltStylePos = 1;
    m_nIndex = 255;

    for (sal_uInt8 i = 0, nCount = static_cast<sal_uInt8>(m_xTableTable->size()); i < nCount; i++)
    {
        SwTableAutoFormat const& rFormat = (*m_xTableTable)[i];
        m_xLbFormat->append_text(rFormat.GetName());
        if (pSelFormat && rFormat.GetName() == pSelFormat->GetName())
            m_nIndex = i;
    }

    m_xLbFormat->select(255 != m_nIndex ? (m_nDfltStylePos + m_nIndex) : 0);
    SelFormatHdl(*m_xLbFormat);
}

void SwConvertTableDlg::UpdateChecks(const SwTableAutoFormat& rFormat, bool bEnable)
{
    m_xBtnNumFormat->set_sensitive(bEnable);
    m_xBtnNumFormat->set_active(rFormat.IsValueFormat());

    m_xBtnBorder->set_sensitive(bEnable);
    m_xBtnBorder->set_active(rFormat.IsFrame());

    m_xBtnFont->set_sensitive(bEnable);
    m_xBtnFont->set_active(rFormat.IsFont());

    m_xBtnPattern->set_sensitive(bEnable);
    m_xBtnPattern->set_active(rFormat.IsBackground());

    m_xBtnAlignment->set_sensitive(bEnable);
    m_xBtnAlignment->set_active(rFormat.IsJustify());
}

std::unique_ptr<SwTableAutoFormat> SwConvertTableDlg::FillAutoFormatOfIndex() const
{
    if (255 != m_nIndex)
    {
        return std::make_unique<SwTableAutoFormat>((*m_xTableTable)[m_nIndex]);
    }

    return nullptr;
}

IMPL_LINK(SwConvertTableDlg, CheckHdl, weld::Toggleable&, rBtn, void)
{
    if (m_nIndex == 255)
        return;

    SwTableAutoFormat& rData = (*m_xTableTable)[m_nIndex];
    bool bCheck = rBtn.get_active(), bDataChgd = true;

    if (&rBtn == m_xBtnNumFormat.get())
        rData.SetValueFormat(bCheck);
    else if (&rBtn == m_xBtnBorder.get())
        rData.SetFrame(bCheck);
    else if (&rBtn == m_xBtnFont.get())
        rData.SetFont(bCheck);
    else if (&rBtn == m_xBtnPattern.get())
        rData.SetBackground(bCheck);
    else if (&rBtn == m_xBtnAlignment.get())
        rData.SetJustify(bCheck);
    else
        bDataChgd = false;

    if (bDataChgd)
    {
        if (!m_bCoreDataChanged)
        {
            m_bCoreDataChanged = true;
        }

        m_aWndPreview.NotifyChange(rData);
        mxTAutoFormat = FillAutoFormatOfIndex();
    }
}

IMPL_LINK_NOARG(SwConvertTableDlg, SelFormatHdl, weld::TreeView&, void)
{
    sal_uInt8 nOldIdx = m_nIndex;
    int nSelPos = m_xLbFormat->get_selected_index();
    if (nSelPos >= m_nDfltStylePos)
    {
        m_nIndex = nSelPos - m_nDfltStylePos;
        m_aWndPreview.NotifyChange((*m_xTableTable)[m_nIndex]);
        UpdateChecks((*m_xTableTable)[m_nIndex], true);
        mxTAutoFormat = FillAutoFormatOfIndex();
    }
    else
    {
        m_nIndex = 255;

        SwTableAutoFormat aTmp(SwViewShell::GetShellRes()->aStrNone);
        aTmp.SetFont(false);
        aTmp.SetJustify(false);
        aTmp.SetFrame(false);
        aTmp.SetBackground(false);
        aTmp.SetValueFormat(false);
        aTmp.SetWidthHeight(false);

        if (nOldIdx != m_nIndex)
            m_aWndPreview.NotifyChange(aTmp);
        UpdateChecks(aTmp, false);
    }
}

IMPL_LINK(SwConvertTableDlg, BtnHdl, weld::Toggleable&, rButton, void)
{
    if (!rButton.get_active())
        return;
    if (m_xTabBtn->get_active())
        m_xKeepColumn->set_state(m_xKeepColumn->get_saved_state());
    else
    {
        if (m_xKeepColumn->get_sensitive())
            m_xKeepColumn->save_state();
        m_xKeepColumn->set_active(true);
    }
    m_xKeepColumn->set_sensitive(m_xTabBtn->get_active());
    m_xOtherEd->set_sensitive(m_xOtherBtn->get_active());
}

IMPL_LINK_NOARG(SwConvertTableDlg, CheckBoxHdl, weld::Toggleable&, void)
{
    m_xRepeatHeaderCB->set_sensitive(m_xHeaderCB->get_active());
    RepeatHeaderCheckBoxHdl(*m_xRepeatHeaderCB);
}

IMPL_LINK_NOARG(SwConvertTableDlg, RepeatHeaderCheckBoxHdl, weld::Toggleable&, void)
{
    bool bEnable = m_xHeaderCB->get_active() && m_xRepeatHeaderCB->get_active();
    m_xRepeatRows->set_sensitive(bEnable);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
