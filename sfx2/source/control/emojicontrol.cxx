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

#include <emojicontrol.hxx>
#include <sfx2/emojipopup.hxx>
#include <emojiview.hxx>
#include <thumbnailviewitem.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/tabpage.hxx>
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

SfxEmojiControl::SfxEmojiControl(EmojiPopup* pControl, weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, "sfx/ui/emojicontrol.ui", "emojictrl")
    , mxTabControl(m_xBuilder->weld_notebook("tabcontrol"))
    , mxPeopleView(new EmojiView(m_xBuilder->weld_scrolled_window("people_emoji_win")))
    , mxPeopleWeld(new weld::CustomWeld(*m_xBuilder, "people_emoji_view", *mxPeopleView))
{
    ConvertLabelToUnicode(FILTER_PEOPLE);
    ConvertLabelToUnicode(FILTER_NATURE);
    ConvertLabelToUnicode(FILTER_FOOD);
    ConvertLabelToUnicode(FILTER_ACTIVITY);
    ConvertLabelToUnicode(FILTER_TRAVEL);
    ConvertLabelToUnicode(FILTER_OBJECTS);
    ConvertLabelToUnicode(FILTER_SYMBOLS);
    ConvertLabelToUnicode(FILTER_FLAGS);
    ConvertLabelToUnicode(FILTER_UNICODE9);

#if 0
    vcl::Font rFont = mxTabControl->GetControlFont();
    rFont.SetFontHeight(TAB_FONT_SIZE);
    mxTabControl->SetControlFont(rFont);
#endif

#if 0
    pTabPage->Show();
    mpEmojiView->SetStyle(mpEmojiView->GetStyle() | WB_VSCROLL);
    mpEmojiView->setItemMaxTextLength(ITEM_MAX_TEXT_LENGTH);
    mpEmojiView->setItemDimensions(ITEM_MAX_WIDTH, 0, ITEM_MAX_HEIGHT, ITEM_PADDING);

    mpEmojiView->Populate();
    mpEmojiView->filterItems(ViewFilter_Category(FILTER_CATEGORY::PEOPLE));

    mpEmojiView->setInsertEmojiHdl(LINK(this, SfxEmojiControl, InsertHdl));
    mpEmojiView->Show();
    mpEmojiView->ShowTooltips(true);
#endif

    mxTabControl->connect_enter_page(LINK(this, SfxEmojiControl, ActivatePageHdl));
}

void SfxEmojiControl::GrabFocus()
{
    mxTabControl->grab_focus();
}

SfxEmojiControl::~SfxEmojiControl()
{
}

void SfxEmojiControl::ConvertLabelToUnicode(const OString& rPageId)
{
    OUStringBuffer sHexText;
    OUString sLabel = mxTabControl->get_tab_label_text(rPageId);
    sHexText.appendUtf32(sLabel.toUInt32(16));
    mxTabControl->set_tab_label_text(rPageId, sHexText.toString());
}

FILTER_CATEGORY SfxEmojiControl::getCurrentFilter() const
{
    const OString sCurPageId = mxTabControl->get_current_page_ident();

    if (sCurPageId == FILTER_PEOPLE)
        return FILTER_CATEGORY::PEOPLE;
    else if (sCurPageId == FILTER_NATURE)
        return FILTER_CATEGORY::NATURE;
    else if (sCurPageId == FILTER_FOOD)
        return FILTER_CATEGORY::FOOD;
    else if (sCurPageId == FILTER_ACTIVITY)
        return FILTER_CATEGORY::ACTIVITY;
    else if (sCurPageId == FILTER_TRAVEL)
        return FILTER_CATEGORY::TRAVEL;
    else if (sCurPageId == FILTER_OBJECTS)
        return FILTER_CATEGORY::OBJECTS;
    else if (sCurPageId == FILTER_SYMBOLS)
        return FILTER_CATEGORY::SYMBOLS;
    else if (sCurPageId == FILTER_FLAGS)
        return FILTER_CATEGORY::FLAGS;
    else if (sCurPageId == FILTER_UNICODE9)
        return FILTER_CATEGORY::UNICODE9;

    return FILTER_CATEGORY::PEOPLE;
}

IMPL_LINK_NOARG(SfxEmojiControl, ActivatePageHdl, const OString&, void)
{
//    mpEmojiView->filterItems(ViewFilter_Category(getCurrentFilter()));
}

#if 0
IMPL_STATIC_LINK(SfxEmojiControl, InsertHdl, ThumbnailViewItem*, pItem, void)
{
    const OUString& sHexText = pItem->getTitle();
    sal_uInt32 cEmojiChar = sHexText.toUInt32(16);

    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );
    OUString sFontName(officecfg::Office::Common::Misc::EmojiFont::get(xContext));

    uno::Sequence<beans::PropertyValue> aArgs( comphelper::InitPropertySequence({
            { "Symbols", uno::Any(OUString(&cEmojiChar, 1)) },
            // add font settings here
            { "FontName", uno::Any(sFontName) }
        }));

    comphelper::dispatchCommand(".uno:InsertSymbol", aArgs);
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
