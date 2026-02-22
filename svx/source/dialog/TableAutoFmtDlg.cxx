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

#include <comphelper/dispatchcommand.hxx>
#include <comphelper/propertyvalue.hxx>
#include <svx/dialog/TableStylesDlg.hxx>
#include <svx/dialog/TableAutoFmtDlg.hxx>
#include <sfx2/basedlgs.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <vcl/svapp.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/Dialog.hxx>
#include <vcl/weld/MessageDialog.hxx>

namespace
{
constexpr size_t DEFAULT_STYLE = 0;

class SvxStringInputDlg : public SfxDialogController
{
private:
    std::unique_ptr<weld::Label> mxLabel;
    std::unique_ptr<weld::Entry> mxEdInput; // Edit obtains the focus.

public:
    SvxStringInputDlg(weld::Window* pParent, const OUString& rTitle, const OUString& rEditTitle,
                      const OUString& rDefault)
        : SfxDialogController(pParent, u"svx/ui/stringinput.ui"_ustr, u"StringInputDialog"_ustr)
        , mxLabel(m_xBuilder->weld_label(u"name"_ustr))
        , mxEdInput(m_xBuilder->weld_entry(u"edit"_ustr))
    {
        mxLabel->set_label(rEditTitle);
        m_xDialog->set_title(rTitle);
        mxEdInput->set_text(rDefault);
        mxEdInput->select_region(0, -1);
    }

    OUString GetInputString() const { return mxEdInput->get_text(); }
};
}

// AutoFormat-Dialog:
SvxTableAutoFmtDlg::SvxTableAutoFmtDlg(SvxAutoFormat& rFormat, const OUString& sFormatName,
                                       weld::Window* pParent, bool bWriter, bool bRTL)
    : weld::GenericDialogController(pParent, u"svx/ui/tableautofmtdlg.ui"_ustr,
                                    u"AutoFormatTableDialog"_ustr)
    , maStrTitle(SvxResId(RID_SVXSTR_ADD_AUTOFORMAT_TITLE))
    , maStrLabel(SvxResId(RID_SVXSTR_ADD_AUTOFORMAT_LABEL))
    , maStrClose(SvxResId(RID_SVXSTR_BTN_AUTOFORMAT_CLOSE))
    , maStrDelMsg(SvxResId(RID_SVXSTR_DEL_AUTOFORMAT_MSG))
    , maStrDelTitle(SvxResId(RID_SVXSTR_DEL_AUTOFORMAT_TITLE))
    , maStrRename(SvxResId(RID_SVXSTR_RENAME_AUTOFORMAT_TITLE))
    , mpFormat(rFormat)
    , maFormatName(sFormatName)
    , mnIndex(0)
    , mbWriter(bWriter)
    , mbRTL(bRTL)
    , maWndPreview(bRTL)
    , mxLbFormat(m_xBuilder->weld_tree_view(u"formatlb"_ustr))
    , mxBtnCancel(m_xBuilder->weld_button(u"cancel"_ustr))
    , mxBtnAdd(m_xBuilder->weld_button(u"add"_ustr))
    , mxBtnEdit(m_xBuilder->weld_button(u"edit"_ustr))
    , mxBtnRemove(m_xBuilder->weld_button(u"remove"_ustr))
    , mxBtnRename(m_xBuilder->weld_button(u"rename"_ustr))
    , mxBtnNumFormat(m_xBuilder->weld_check_button(u"numformatcb"_ustr))
    , mxBtnBorder(m_xBuilder->weld_check_button(u"bordercb"_ustr))
    , mxBtnFont(m_xBuilder->weld_check_button(u"fontcb"_ustr))
    , mxBtnPattern(m_xBuilder->weld_check_button(u"patterncb"_ustr))
    , mxBtnAlignment(m_xBuilder->weld_check_button(u"alignmentcb"_ustr))
    , mxBtnAdjust(m_xBuilder->weld_check_button(u"autofitcb"_ustr))
    , mxWndPreview(new weld::CustomWeld(*m_xBuilder, u"preview"_ustr, maWndPreview))
{
    const int nWidth = mxLbFormat->get_approximate_digit_width() * 32;
    const int nHeight = mxLbFormat->get_height_rows(8);
    mxLbFormat->set_size_request(nWidth, nHeight);
    mxWndPreview->set_size_request(nWidth, nHeight);

    Init();
}

void SvxTableAutoFmtDlg::Init()
{
    Link<weld::Toggleable&, void> aLk(LINK(this, SvxTableAutoFmtDlg, CheckHdl));
    mxLbFormat->connect_selection_changed(LINK(this, SvxTableAutoFmtDlg, SelFormatHdl));
    mxBtnNumFormat->connect_toggled(aLk);
    mxBtnBorder->connect_toggled(aLk);
    mxBtnFont->connect_toggled(aLk);
    mxBtnPattern->connect_toggled(aLk);
    mxBtnAlignment->connect_toggled(aLk);

    mxBtnAdd->connect_clicked(LINK(this, SvxTableAutoFmtDlg, AddHdl));
    mxBtnEdit->connect_clicked(LINK(this, SvxTableAutoFmtDlg, EditHdl));
    mxBtnRemove->connect_clicked(LINK(this, SvxTableAutoFmtDlg, RemoveHdl));
    mxBtnRename->connect_clicked(LINK(this, SvxTableAutoFmtDlg, RenameHdl));

    if (!mpFormat.size())
    {
        SAL_WARN("svx", "No 'Table Styles' in 'tablestyles.xml'");
        return;
    }

    mnIndex = DEFAULT_STYLE;
    PopulateFormatList();
    mxLbFormat->select(mnIndex);
    SelFormatHdl(*mxLbFormat);

    // Hide adjust button in Writer
    if (mbWriter)
    {
        mxBtnAdjust->set_visible(false);
    }

    UpdateUIState();
    UpdateChecks();
}

void SvxTableAutoFmtDlg::PopulateFormatList()
{
    mxLbFormat->freeze();
    mxLbFormat->clear();

    for (size_t i = 0; i < mpFormat.size(); ++i)
    {
        if (const SvxAutoFormatData* rFormat = mpFormat.GetData(i))
        {
            if (rFormat->GetName() == maFormatName)
                mnIndex = i;
            mxLbFormat->append_text(rFormat->GetName());
        }
    }

    mxLbFormat->thaw();
}

void SvxTableAutoFmtDlg::UpdateUIState()
{
    const bool bCanModify = mnIndex != DEFAULT_STYLE;
    mxBtnRename->set_sensitive(bCanModify);
    mxBtnRemove->set_sensitive(bCanModify);
}

void SvxTableAutoFmtDlg::UpdateChecks()
{
    if (const SvxAutoFormatData* pData = mpFormat.GetData(mnIndex))
    {
        mxBtnNumFormat->set_active(pData->IsValueFormat());
        mxBtnBorder->set_active(pData->IsFrame());
        mxBtnFont->set_active(pData->IsFont());
        mxBtnPattern->set_active(pData->IsBackground());
        mxBtnAlignment->set_active(pData->IsJustify());
        mxBtnAdjust->set_active(pData->IsWidthHeight());
    }
}

OUString SvxTableAutoFmtDlg::GenerateUniqueStyleName()
{
    OUString sBase = "Untitled";
    OUString sName = sBase + "1";
    int nCounter = 1;

    while (mpFormat.FindAutoFormat(sName))
    {
        nCounter++;
        sName = sBase + OUString::number(nCounter);
    }
    return sName;
}

// Event Handlers
IMPL_LINK(SvxTableAutoFmtDlg, CheckHdl, weld::Toggleable&, rBtn, void)
{
    SvxAutoFormatData* rData = mpFormat.GetData(mnIndex);
    if (!rData)
        return;

    const bool bCheck = rBtn.get_active();

    if (&rBtn == mxBtnNumFormat.get())
        rData->SetValueFormat(bCheck);
    else if (&rBtn == mxBtnBorder.get())
        rData->SetFrame(bCheck);
    else if (&rBtn == mxBtnFont.get())
        rData->SetFont(bCheck);
    else if (&rBtn == mxBtnPattern.get())
        rData->SetBackground(bCheck);
    else if (&rBtn == mxBtnAlignment.get())
        rData->SetJustify(bCheck);
    else
        rData->SetWidthHeight(bCheck);

    maWndPreview.NotifyChange(mpFormat.GetResolvedStyle(rData));
}

IMPL_LINK_NOARG(SvxTableAutoFmtDlg, RemoveHdl, weld::Button&, void)
{
    OUString aMessage = maStrDelMsg + "\n'" + mxLbFormat->get_selected_text() + "'\n";

    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
        m_xDialog.get(), VclMessageType::Question, VclButtonsType::OkCancel, maStrDelTitle));
    xBox->set_secondary_text(aMessage);

    if (xBox->run() == RET_OK)
    {
        mpFormat.ReleaseAutoFormat(mxLbFormat->get_selected_text());
        mxLbFormat->remove(mnIndex--);
        UpdateUIState();
        SelFormatHdl(*mxLbFormat);
    }
}

IMPL_LINK_NOARG(SvxTableAutoFmtDlg, RenameHdl, weld::Button&, void)
{
    bool bOk = false;
    while (!bOk)
    {
        SvxStringInputDlg aDlg(m_xDialog.get(), maStrRename, maStrLabel,
                               mxLbFormat->get_selected_text());
        if (aDlg.run() == RET_OK)
        {
            const OUString aFormatName(aDlg.GetInputString().trim());

            // Check for empty string and duplicate names
            if (aFormatName.isEmpty() || aFormatName.indexOf(".") != -1
                || aFormatName.indexOf("-") != -1 || mpFormat.FindAutoFormat(aFormatName))
            {
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
                    m_xDialog.get(), VclMessageType::Error, VclButtonsType::Ok,
                    SvxResId(RID_SVXSTR_INVALID_AUTOFORMAT_NAME)));
                xBox->run();
                continue;
            }

            // Perform rename
            if (SvxAutoFormatData* pData = mpFormat.FindAutoFormat(mxLbFormat->get_selected_text()))
            {
                pData->SetName(aFormatName);
                mxLbFormat->set_text(mnIndex, aFormatName);
                mxLbFormat->select(mnIndex);
            }

            bOk = true;
        }
        else
        {
            bOk = true;
        }
    }
}

IMPL_LINK_NOARG(SvxTableAutoFmtDlg, SelFormatHdl, weld::TreeView&, void)
{
    mnIndex = mxLbFormat->get_selected_index();
    UpdateChecks();
    UpdateUIState();

    if (const SvxAutoFormatData* pData = mpFormat.GetData(mnIndex))
        maWndPreview.NotifyChange(mpFormat.GetResolvedStyle(pData));
}

OUString SvxTableAutoFmtDlg::GetCurrFormatName()
{
    const SvxAutoFormatData* pData = mpFormat.GetData(mnIndex);
    return pData ? pData->GetName() : OUString();
}

IMPL_LINK_NOARG(SvxTableAutoFmtDlg, AddHdl, weld::Button&, void)
{
    SvxAutoFormatData* pNewData = mpFormat.GetDefaultData();
    pNewData->SetName(GenerateUniqueStyleName());
    pNewData->SetParent(mxLbFormat->get_selected_text());

    bool bNewStyle = true;
    SvxTableStylesDlg aDlg(m_xDialog.get(), bNewStyle, mpFormat, *pNewData, mbRTL);

    aDlg.getDialog()->set_modal(true);
    if (aDlg.run() == RET_OK)
    {
        mpFormat.InsertAutoFormat(pNewData);
        mxLbFormat->append_text(pNewData->GetName());
        mxLbFormat->select_text(pNewData->GetName());
        SelFormatHdl(*mxLbFormat);
    }
    aDlg.getDialog()->set_modal(false);

    if (!mbWriter)
        mpFormat.Save();
}

IMPL_LINK_NOARG(SvxTableAutoFmtDlg, EditHdl, weld::Button&, void)
{
    bool bNewStyle = false;
    OUString sOldName = mpFormat.GetData(mnIndex)->GetName();
    SvxTableStylesDlg aDlg(m_xDialog.get(), bNewStyle, mpFormat, *mpFormat.GetData(mnIndex), mbRTL);

    aDlg.getDialog()->set_modal(true);
    if (aDlg.run() == RET_OK)
    {
        // If name of the format is changed then update the parent-style of formats which inherit this style
        OUString sNewName = mpFormat.GetData(mnIndex)->GetName();
        if (sOldName != sNewName)
        {
            for (size_t i = 0; i < mpFormat.size(); i++)
            {
                if (mpFormat.GetData(i)->GetParent() == sOldName)
                    mpFormat.GetData(i)->SetParent(sNewName);
            }
        }

        mxLbFormat->set_text(mnIndex, mpFormat.GetData(mnIndex)->GetName());
        SelFormatHdl(*mxLbFormat);

        // For Writer this will reapply the styles to the tables
        if (mbWriter)
        {
            css::uno::Sequence<css::beans::PropertyValue> aArgs
                = { comphelper::makePropertyValue(u"aFormatName"_ustr, sNewName),
                    comphelper::makePropertyValue(u"aOldName"_ustr, sOldName) };
            comphelper::dispatchCommand(u".uno:ResetAutoFormats"_ustr, aArgs);
        }
        else
        {
            mpFormat.Save();
        }
    }
    aDlg.getDialog()->set_modal(false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
