/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/emojiview.hxx>
#include <sfx2/emojiviewitem.hxx>
#include <sfx2/emojidlg.hxx>

SfxEmojiDlg::SfxEmojiDlg(vcl::Window *parent)
    : ModalDialog(parent, "EmojiWindow", "sfx/ui/emojiwindow.ui")
{
    get(mpTabControl, "tab_control");
    get(mpEmojiView, "emoji_view");

    sal_uInt16 nCurPageId = mpTabControl->GetPageId("people");
    TabPage *pTabPage = mpTabControl->GetTabPage(nCurPageId);
    ConvertLabelToUnicode(nCurPageId);
    pTabPage->Show();

    nCurPageId = mpTabControl->GetPageId("nature");
    mpTabControl->SetTabPage(nCurPageId, pTabPage);
    ConvertLabelToUnicode(nCurPageId);
    pTabPage->Show();

    nCurPageId = mpTabControl->GetPageId("food");
    mpTabControl->SetTabPage(nCurPageId, pTabPage);
    ConvertLabelToUnicode(nCurPageId);
    pTabPage->Show();

    nCurPageId = mpTabControl->GetPageId("activity");
    mpTabControl->SetTabPage(nCurPageId, pTabPage);
    ConvertLabelToUnicode(nCurPageId);
    pTabPage->Show();

    nCurPageId = mpTabControl->GetPageId("travel");
    mpTabControl->SetTabPage(nCurPageId, pTabPage);
    ConvertLabelToUnicode(nCurPageId);
    pTabPage->Show();

    nCurPageId = mpTabControl->GetPageId("objects");
    mpTabControl->SetTabPage(nCurPageId, pTabPage);
    ConvertLabelToUnicode(nCurPageId);
    pTabPage->Show();

    nCurPageId = mpTabControl->GetPageId("symbols");
    mpTabControl->SetTabPage(nCurPageId, pTabPage);
    ConvertLabelToUnicode(nCurPageId);
    pTabPage->Show();

    nCurPageId = mpTabControl->GetPageId("flags");
    mpTabControl->SetTabPage(nCurPageId, pTabPage);
    ConvertLabelToUnicode(nCurPageId);
    pTabPage->Show();

    nCurPageId = mpTabControl->GetPageId("diversity");
    mpTabControl->SetTabPage(nCurPageId, pTabPage);
    ConvertLabelToUnicode(nCurPageId);
    pTabPage->Show();

    nCurPageId = mpTabControl->GetPageId("unicode9");
    mpTabControl->SetTabPage(nCurPageId, pTabPage);
    ConvertLabelToUnicode(nCurPageId);
    pTabPage->Show();

    mpEmojiView->SetStyle(mpEmojiView->GetStyle() | WB_VSCROLL);
    mpEmojiView->setItemMaxTextLength(ITEM_MAX_TEXT_LENGTH);
    mpEmojiView->setItemDimensions(ITEM_MAX_WIDTH, 0, ITEM_MAX_HEIGHT, ITEM_PADDING);
    mpEmojiView->Populate();
    mpEmojiView->Show();
}

SfxEmojiDlg::~SfxEmojiDlg()
{
    disposeOnce();
}


void SfxEmojiDlg::dispose()
{
    mpTabControl.clear();
    mpEmojiView.clear();

    ModalDialog::dispose();
}

void SfxEmojiDlg::ConvertLabelToUnicode(sal_uInt16 nPageId)
{
    OUStringBuffer sHexText = "";
    OUString sLabel = mpTabControl->GetPageText(nPageId);
    sHexText.appendUtf32(sLabel.toUInt32(16));
    mpTabControl->SetPageText(nPageId, sHexText.toString());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
