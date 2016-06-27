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

const char FILTER_PEOPLE[]    = "people";
const char FILTER_NATURE[]    = "nature";
const char FILTER_FOOD[]      = "food";
const char FILTER_ACTIVITY[]  = "activity";
const char FILTER_TRAVEL[]    = "travel";
const char FILTER_OBJECTS[]   = "objects";
const char FILTER_SYMBOLS[]   = "symbols";
const char FILTER_FLAGS[]     = "flags";
const char FILTER_UNICODE9[]  = "unicode9";

SfxEmojiDlg::SfxEmojiDlg(vcl::Window *parent)
    : ModalDialog(parent, "EmojiWindow", "sfx/ui/emojiwindow.ui")
{
    get(mpTabControl, "tab_control");
    get(mpEmojiView, "emoji_view");

    sal_uInt16 nCurPageId = mpTabControl->GetPageId(FILTER_PEOPLE);
    TabPage *pTabPage = mpTabControl->GetTabPage(nCurPageId);
    ConvertLabelToUnicode(nCurPageId);
    pTabPage->Show();

    nCurPageId = mpTabControl->GetPageId(FILTER_NATURE);
    mpTabControl->SetTabPage(nCurPageId, pTabPage);
    ConvertLabelToUnicode(nCurPageId);
    pTabPage->Show();

    nCurPageId = mpTabControl->GetPageId(FILTER_FOOD);
    mpTabControl->SetTabPage(nCurPageId, pTabPage);
    ConvertLabelToUnicode(nCurPageId);
    pTabPage->Show();

    nCurPageId = mpTabControl->GetPageId(FILTER_ACTIVITY);
    mpTabControl->SetTabPage(nCurPageId, pTabPage);
    ConvertLabelToUnicode(nCurPageId);
    pTabPage->Show();

    nCurPageId = mpTabControl->GetPageId(FILTER_TRAVEL);
    mpTabControl->SetTabPage(nCurPageId, pTabPage);
    ConvertLabelToUnicode(nCurPageId);
    pTabPage->Show();

    nCurPageId = mpTabControl->GetPageId(FILTER_OBJECTS);
    mpTabControl->SetTabPage(nCurPageId, pTabPage);
    ConvertLabelToUnicode(nCurPageId);
    pTabPage->Show();

    nCurPageId = mpTabControl->GetPageId(FILTER_SYMBOLS);
    mpTabControl->SetTabPage(nCurPageId, pTabPage);
    ConvertLabelToUnicode(nCurPageId);
    pTabPage->Show();

    nCurPageId = mpTabControl->GetPageId(FILTER_FLAGS);
    mpTabControl->SetTabPage(nCurPageId, pTabPage);
    ConvertLabelToUnicode(nCurPageId);
    pTabPage->Show();

    nCurPageId = mpTabControl->GetPageId(FILTER_UNICODE9);
    mpTabControl->SetTabPage(nCurPageId, pTabPage);
    ConvertLabelToUnicode(nCurPageId);
    pTabPage->Show();

    mpEmojiView->SetStyle(mpEmojiView->GetStyle() | WB_VSCROLL);
    mpEmojiView->setItemMaxTextLength(ITEM_MAX_TEXT_LENGTH);
    mpEmojiView->setItemDimensions(ITEM_MAX_WIDTH, 0, ITEM_MAX_HEIGHT, ITEM_PADDING);
    mpEmojiView->Populate();
    mpEmojiView->filterItems(ViewFilter_Category(FILTER_CATEGORY::PEOPLE));
    mpEmojiView->Show();

    mpTabControl->SetActivatePageHdl(LINK(this, SfxEmojiDlg, ActivatePageHdl));
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

FILTER_CATEGORY SfxEmojiDlg::getCurrentFilter()
{
    const sal_uInt16 nCurPageId = mpTabControl->GetCurPageId();

    if (nCurPageId == mpTabControl->GetPageId(FILTER_PEOPLE))
        return FILTER_CATEGORY::PEOPLE;
    else if (nCurPageId == mpTabControl->GetPageId(FILTER_NATURE))
        return FILTER_CATEGORY::NATURE;
    else if (nCurPageId == mpTabControl->GetPageId(FILTER_FOOD))
        return FILTER_CATEGORY::FOOD;
    else if (nCurPageId == mpTabControl->GetPageId(FILTER_ACTIVITY))
        return FILTER_CATEGORY::ACTIVITY;
    else if (nCurPageId == mpTabControl->GetPageId(FILTER_TRAVEL))
        return FILTER_CATEGORY::TRAVEL;
    else if (nCurPageId == mpTabControl->GetPageId(FILTER_OBJECTS))
        return FILTER_CATEGORY::OBJECTS;
    else if (nCurPageId == mpTabControl->GetPageId(FILTER_SYMBOLS))
        return FILTER_CATEGORY::SYMBOLS;
    else if (nCurPageId == mpTabControl->GetPageId(FILTER_FLAGS))
        return FILTER_CATEGORY::FLAGS;
    else if (nCurPageId == mpTabControl->GetPageId(FILTER_UNICODE9))
        return FILTER_CATEGORY::UNICODE9;

    return FILTER_CATEGORY::PEOPLE;
}

IMPL_LINK_NOARG_TYPED(SfxEmojiDlg, ActivatePageHdl, TabControl*, void)
{
    mpEmojiView->filterItems(ViewFilter_Category(getCurrentFilter()));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
