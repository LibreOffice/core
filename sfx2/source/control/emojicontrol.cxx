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

#include <sfx2/emojiview.hxx>
#include <sfx2/emojiviewitem.hxx>
#include <sfx2/emojicontrol.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <officecfg/Office/Common.hxx>

const char FILTER_PEOPLE[]    = "people";
const char FILTER_NATURE[]    = "nature";
const char FILTER_FOOD[]      = "food";
const char FILTER_ACTIVITY[]  = "activity";
const char FILTER_TRAVEL[]    = "travel";
const char FILTER_OBJECTS[]   = "objects";
const char FILTER_SYMBOLS[]   = "symbols";
const char FILTER_FLAGS[]     = "flags";
const char FILTER_UNICODE9[]  = "unicode9";

using namespace com::sun::star;

SfxEmojiControl::SfxEmojiControl(sal_uInt16 nId, const css::uno::Reference< css::frame::XFrame >& rFrame)
    : SfxPopupWindow(nId, "emojictrl", "sfx/ui/emojicontrol.ui", rFrame)
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

    mpEmojiView->setInsertEmojiHdl(LINK(this, SfxEmojiControl, InsertHdl));
    mpEmojiView->Show();

    mpTabControl->SetActivatePageHdl(LINK(this, SfxEmojiControl, ActivatePageHdl));
}

SfxEmojiControl::~SfxEmojiControl()
{
    disposeOnce();
}


void SfxEmojiControl::dispose()
{
    mpTabControl.clear();
    mpEmojiView.clear();

    SfxPopupWindow::dispose();
}

void SfxEmojiControl::ConvertLabelToUnicode(sal_uInt16 nPageId)
{
    OUStringBuffer sHexText = "";
    OUString sLabel = mpTabControl->GetPageText(nPageId);
    sHexText.appendUtf32(sLabel.toUInt32(16));
    mpTabControl->SetPageText(nPageId, sHexText.toString());
}

FILTER_CATEGORY SfxEmojiControl::getCurrentFilter()
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

IMPL_LINK_NOARG_TYPED(SfxEmojiControl, ActivatePageHdl, TabControl*, void)
{
    mpEmojiView->filterItems(ViewFilter_Category(getCurrentFilter()));
}

IMPL_STATIC_LINK_TYPED(SfxEmojiControl, InsertHdl, ThumbnailViewItem*, pItem, void)
{
    OUStringBuffer sHexText = "";
    sHexText.appendUtf32(OUString(pItem->getTitle()).toUInt32(16));

    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );
    OUString sFontName(officecfg::Office::Common::Misc::EmojiFont::get(xContext));

    uno::Sequence<beans::PropertyValue> aArgs(2);
    aArgs[0].Name = OUString::fromUtf8("Symbols");
    aArgs[0].Value = uno::makeAny(sHexText.toString());
    //add font settings here
    aArgs[1].Name = OUString::fromUtf8("FontName");
    aArgs[1].Value = uno::makeAny(sFontName);

    comphelper::dispatchCommand(".uno:InsertSymbol", aArgs);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
