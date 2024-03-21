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

#include <memory>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <sal/log.hxx>
#include <strings.hrc>
#include <shellres.hxx>
#include <tautofmt.hxx>

namespace
{
class SwStringInputDlg : public SfxDialogController
{
private:
    std::unique_ptr<weld::Label> m_xLabel;
    std::unique_ptr<weld::Entry> m_xEdInput; // Edit obtains the focus.

public:
    SwStringInputDlg(weld::Window* pParent, const OUString& rTitle, const OUString& rEditTitle,
                     const OUString& rDefault)
        : SfxDialogController(pParent, "modules/swriter/ui/stringinput.ui", "StringInputDialog")
        , m_xLabel(m_xBuilder->weld_label("name"))
        , m_xEdInput(m_xBuilder->weld_entry("edit"))
    {
        m_xLabel->set_label(rEditTitle);
        m_xDialog->set_title(rTitle);
        m_xEdInput->set_text(rDefault);
        m_xEdInput->select_region(0, -1);
    }

    OUString GetInputString() const { return m_xEdInput->get_text(); }
};
}

// AutoFormat-Dialogue:
SwAutoFormatDlg::SwAutoFormatDlg(weld::Window* pParent, SwWrtShell* pWrtShell, bool bAutoFormat,
                                 const SwTableAutoFormat* pSelFormat)
    : SfxDialogController(pParent, "modules/swriter/ui/autoformattable.ui", "AutoFormatTableDialog")
    , m_aStrTitle(SwResId(STR_ADD_AUTOFORMAT_TITLE))
    , m_aStrLabel(SwResId(STR_ADD_AUTOFORMAT_LABEL))
    , m_aStrClose(SwResId(STR_BTN_AUTOFORMAT_CLOSE))
    , m_aStrDelTitle(SwResId(STR_DEL_AUTOFORMAT_TITLE))
    , m_aStrDelMsg(SwResId(STR_DEL_AUTOFORMAT_MSG))
    , m_aStrRenameTitle(SwResId(STR_RENAME_AUTOFORMAT_TITLE))
    , m_aStrInvalidFormat(SwResId(STR_INVALID_AUTOFORMAT_NAME))
    , m_pShell(pWrtShell)
    , m_nIndex(0)
    , m_nDfltStylePos(0)
    , m_bCoreDataChanged(false)
    , m_bSetAutoFormat(bAutoFormat)
    , m_xTableTable(new SwTableAutoFormatTable)
    , m_xLbFormat(m_xBuilder->weld_tree_view("formatlb"))
    , m_xBtnNumFormat(m_xBuilder->weld_check_button("numformatcb"))
    , m_xBtnBorder(m_xBuilder->weld_check_button("bordercb"))
    , m_xBtnFont(m_xBuilder->weld_check_button("fontcb"))
    , m_xBtnPattern(m_xBuilder->weld_check_button("patterncb"))
    , m_xBtnAlignment(m_xBuilder->weld_check_button("alignmentcb"))
    , m_xBtnCancel(m_xBuilder->weld_button("cancel"))
    , m_xBtnAdd(m_xBuilder->weld_button("add"))
    , m_xBtnRemove(m_xBuilder->weld_button("remove"))
    , m_xBtnRename(m_xBuilder->weld_button("rename"))
    , m_xWndPreview(new weld::CustomWeld(*m_xBuilder, "preview", m_aWndPreview))
{
    m_aWndPreview.DetectRTL(pWrtShell);
    m_xTableTable->Load();

    const int nWidth = m_xLbFormat->get_approximate_digit_width() * 32;
    const int nHeight = m_xLbFormat->get_height_rows(8);
    m_xLbFormat->set_size_request(nWidth, nHeight);
    m_xWndPreview->set_size_request(nWidth, nHeight);

    Init(pSelFormat);
}

SwAutoFormatDlg::~SwAutoFormatDlg()
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

void SwAutoFormatDlg::Init(const SwTableAutoFormat* pSelFormat)
{
    Link<weld::Toggleable&, void> aLk(LINK(this, SwAutoFormatDlg, CheckHdl));
    m_xBtnBorder->connect_toggled(aLk);
    m_xBtnFont->connect_toggled(aLk);
    m_xBtnPattern->connect_toggled(aLk);
    m_xBtnAlignment->connect_toggled(aLk);
    m_xBtnNumFormat->connect_toggled(aLk);

    m_xBtnAdd->connect_clicked(LINK(this, SwAutoFormatDlg, AddHdl));
    m_xBtnRemove->connect_clicked(LINK(this, SwAutoFormatDlg, RemoveHdl));
    m_xBtnRename->connect_clicked(LINK(this, SwAutoFormatDlg, RenameHdl));
    m_xLbFormat->connect_changed(LINK(this, SwAutoFormatDlg, SelFormatHdl));

    m_xBtnAdd->set_sensitive(m_bSetAutoFormat);

    m_nIndex = 0;
    if (!m_bSetAutoFormat)
    {
        // Then the list to be expanded by the entry "- none -".
        m_xLbFormat->append_text(SwViewShell::GetShellRes()->aStrNone);
        m_nDfltStylePos = 1;
        m_nIndex = 255;
    }

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

void SwAutoFormatDlg::UpdateChecks(const SwTableAutoFormat& rFormat, bool bEnable)
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

std::unique_ptr<SwTableAutoFormat> SwAutoFormatDlg::FillAutoFormatOfIndex() const
{
    if (255 != m_nIndex)
    {
        return std::make_unique<SwTableAutoFormat>((*m_xTableTable)[m_nIndex]);
    }

    return nullptr;
}

// Handler:
IMPL_LINK(SwAutoFormatDlg, CheckHdl, weld::Toggleable&, rBtn, void)
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
            m_xBtnCancel->set_label(m_aStrClose);
            m_bCoreDataChanged = true;
        }

        m_aWndPreview.NotifyChange(rData);
    }
}

IMPL_LINK_NOARG(SwAutoFormatDlg, AddHdl, weld::Button&, void)
{
    bool bOk = false, bFormatInserted = false;
    while (!bOk)
    {
        SwStringInputDlg aDlg(m_xDialog.get(), m_aStrTitle, m_aStrLabel, OUString());
        if (RET_OK == aDlg.run())
        {
            const OUString aFormatName(aDlg.GetInputString());

            if (!aFormatName.isEmpty())
            {
                size_t n;
                for (n = 0; n < m_xTableTable->size(); ++n)
                    if ((*m_xTableTable)[n].GetName() == aFormatName)
                        break;

                if (n >= m_xTableTable->size())
                {
                    // Format with the name does not already exist, so take up.
                    std::unique_ptr<SwTableAutoFormat> pNewData(new SwTableAutoFormat(aFormatName));
                    bool bGetOk = m_pShell->GetTableAutoFormat(*pNewData);
                    SAL_WARN_IF(!bGetOk, "sw.ui", "GetTableAutoFormat failed for: " << aFormatName);

                    // Insert sorted!!
                    for (n = 1; n < m_xTableTable->size(); ++n)
                        if ((*m_xTableTable)[n].GetName() > aFormatName)
                            break;

                    m_xTableTable->InsertAutoFormat(n, std::move(pNewData));
                    m_xLbFormat->insert_text(m_nDfltStylePos + n, aFormatName);
                    m_xLbFormat->select(m_nDfltStylePos + n);
                    bFormatInserted = true;
                    m_xBtnAdd->set_sensitive(false);
                    if (!m_bCoreDataChanged)
                    {
                        m_xBtnCancel->set_label(m_aStrClose);
                        m_bCoreDataChanged = true;
                    }

                    SelFormatHdl(*m_xLbFormat);
                    bOk = true;
                }
            }

            if (!bFormatInserted)
            {
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
                    m_xDialog.get(), VclMessageType::Error, VclButtonsType::OkCancel,
                    m_aStrInvalidFormat));
                bOk = RET_CANCEL == xBox->run();
            }
        }
        else
            bOk = true;
    }
}

IMPL_LINK_NOARG(SwAutoFormatDlg, RemoveHdl, weld::Button&, void)
{
    OUString aMessage = m_aStrDelMsg + "\n\n" + m_xLbFormat->get_selected_text() + "\n";

    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
        m_xDialog.get(), VclMessageType::Question, VclButtonsType::OkCancel, m_aStrDelTitle));
    xBox->set_secondary_text(aMessage);

    if (xBox->run() == RET_OK)
    {
        sal_uInt8 nIndex = m_nIndex;

        m_xLbFormat->remove(m_nDfltStylePos + nIndex);
        m_xLbFormat->select(m_nDfltStylePos + nIndex - 1);

        m_xTableTable->EraseAutoFormat(nIndex);
        m_nIndex = nIndex - 1;

        if (!m_nIndex)
        {
            m_xBtnRemove->set_sensitive(false);
            m_xBtnRename->set_sensitive(false);
        }

        if (!m_bCoreDataChanged)
        {
            m_xBtnCancel->set_label(m_aStrClose);
            m_bCoreDataChanged = true;
        }
    }

    SelFormatHdl(*m_xLbFormat);
}

IMPL_LINK_NOARG(SwAutoFormatDlg, RenameHdl, weld::Button&, void)
{
    bool bOk = false;
    while (!bOk)
    {
        SwStringInputDlg aDlg(m_xDialog.get(), m_aStrRenameTitle, m_aStrLabel,
                              m_xLbFormat->get_selected_text());
        if (aDlg.run() == RET_OK)
        {
            bool bFormatRenamed = false;
            const OUString aFormatName(aDlg.GetInputString());

            if (!aFormatName.isEmpty())
            {
                size_t n;
                for (n = 0; n < m_xTableTable->size(); ++n)
                    if ((*m_xTableTable)[n].GetName() == aFormatName)
                        break;

                if (n >= m_xTableTable->size())
                {
                    sal_uInt8 nIndex = m_nIndex;

                    // no format with this name exists, so rename it
                    m_xLbFormat->remove(m_nDfltStylePos + nIndex);
                    std::unique_ptr<SwTableAutoFormat> p(m_xTableTable->ReleaseAutoFormat(nIndex));

                    p->SetName(aFormatName);

                    // keep all arrays sorted!
                    for (n = 1; n < m_xTableTable->size(); ++n)
                        if ((*m_xTableTable)[n].GetName() > aFormatName)
                        {
                            break;
                        }

                    m_xTableTable->InsertAutoFormat(n, std::move(p));
                    m_xLbFormat->insert_text(m_nDfltStylePos + n, aFormatName);
                    m_xLbFormat->select(m_nDfltStylePos + n);

                    if (!m_bCoreDataChanged)
                    {
                        m_xBtnCancel->set_label(m_aStrClose);
                        m_bCoreDataChanged = true;
                    }

                    SelFormatHdl(*m_xLbFormat);
                    bOk = true;
                    bFormatRenamed = true;
                }
            }

            if (!bFormatRenamed)
            {
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
                    m_xDialog.get(), VclMessageType::Error, VclButtonsType::OkCancel,
                    m_aStrInvalidFormat));
                bOk = RET_CANCEL == xBox->run();
            }
        }
        else
            bOk = true;
    }
}

IMPL_LINK_NOARG(SwAutoFormatDlg, SelFormatHdl, weld::TreeView&, void)
{
    bool bBtnEnable = false;
    sal_uInt8 nOldIdx = m_nIndex;
    int nSelPos = m_xLbFormat->get_selected_index();
    if (nSelPos >= m_nDfltStylePos)
    {
        m_nIndex = nSelPos - m_nDfltStylePos;
        m_aWndPreview.NotifyChange((*m_xTableTable)[m_nIndex]);
        bBtnEnable = 0 != m_nIndex;
        UpdateChecks((*m_xTableTable)[m_nIndex], true);
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

    m_xBtnRemove->set_sensitive(bBtnEnable);
    m_xBtnRename->set_sensitive(bBtnEnable);
}

void SwAutoFormatDlg::Apply()
{
    if (m_bSetAutoFormat)
        m_pShell->SetTableStyle((*m_xTableTable)[m_nIndex]);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
