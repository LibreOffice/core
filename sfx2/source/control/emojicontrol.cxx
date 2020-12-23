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
#include <sfx2/thumbnailviewitem.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <officecfg/Office/Common.hxx>

const OStringLiteral FILTER_PEOPLE = "people";
const OStringLiteral FILTER_NATURE = "nature";
const OStringLiteral FILTER_FOOD = "food";
const OStringLiteral FILTER_ACTIVITY = "activity";
const OStringLiteral FILTER_TRAVEL = "travel";
const OStringLiteral FILTER_OBJECTS = "objects";
const OStringLiteral FILTER_SYMBOLS = "symbols";
const OStringLiteral FILTER_FLAGS = "flags";
const OStringLiteral FILTER_UNICODE9 = "unicode9";

using namespace com::sun::star;

SfxEmojiControl::SfxEmojiControl(const EmojiPopup* pControl, weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, "sfx/ui/emojicontrol.ui", "emojictrl")
    , mxPeopleBtn(m_xBuilder->weld_toggle_button(FILTER_PEOPLE))
    , mxNatureBtn(m_xBuilder->weld_toggle_button(FILTER_NATURE))
    , mxFoodBtn(m_xBuilder->weld_toggle_button(FILTER_FOOD))
    , mxActivityBtn(m_xBuilder->weld_toggle_button(FILTER_ACTIVITY))
    , mxTravelBtn(m_xBuilder->weld_toggle_button(FILTER_TRAVEL))
    , mxObjectsBtn(m_xBuilder->weld_toggle_button(FILTER_OBJECTS))
    , mxSymbolsBtn(m_xBuilder->weld_toggle_button(FILTER_SYMBOLS))
    , mxFlagsBtn(m_xBuilder->weld_toggle_button(FILTER_FLAGS))
    , mxUnicode9Btn(m_xBuilder->weld_toggle_button(FILTER_UNICODE9))
    , mxEmojiView(new EmojiView(m_xBuilder->weld_scrolled_window("emoji_win", true)))
    , mxEmojiWeld(new weld::CustomWeld(*m_xBuilder, "emoji_view", *mxEmojiView))
{
    ConvertLabelToUnicode(*mxPeopleBtn);
    ConvertLabelToUnicode(*mxNatureBtn);
    ConvertLabelToUnicode(*mxFoodBtn);
    ConvertLabelToUnicode(*mxActivityBtn);
    ConvertLabelToUnicode(*mxTravelBtn);
    ConvertLabelToUnicode(*mxObjectsBtn);
    ConvertLabelToUnicode(*mxSymbolsBtn);
    ConvertLabelToUnicode(*mxFlagsBtn);
    ConvertLabelToUnicode(*mxUnicode9Btn);

    mxPeopleBtn->connect_clicked(LINK(this, SfxEmojiControl, ActivatePageHdl));
    mxNatureBtn->connect_clicked(LINK(this, SfxEmojiControl, ActivatePageHdl));
    mxFoodBtn->connect_clicked(LINK(this, SfxEmojiControl, ActivatePageHdl));
    mxActivityBtn->connect_clicked(LINK(this, SfxEmojiControl, ActivatePageHdl));
    mxTravelBtn->connect_clicked(LINK(this, SfxEmojiControl, ActivatePageHdl));
    mxObjectsBtn->connect_clicked(LINK(this, SfxEmojiControl, ActivatePageHdl));
    mxSymbolsBtn->connect_clicked(LINK(this, SfxEmojiControl, ActivatePageHdl));
    mxFlagsBtn->connect_clicked(LINK(this, SfxEmojiControl, ActivatePageHdl));
    mxUnicode9Btn->connect_clicked(LINK(this, SfxEmojiControl, ActivatePageHdl));

    mxEmojiView->setItemMaxTextLength(ITEM_MAX_TEXT_LENGTH);
    mxEmojiView->setItemDimensions(ITEM_MAX_WIDTH, 0, ITEM_MAX_HEIGHT, ITEM_PADDING);

    mxEmojiView->Populate();
    ActivatePageHdl(*mxPeopleBtn);

    mxEmojiView->setInsertEmojiHdl(LINK(this, SfxEmojiControl, InsertHdl));
    mxEmojiView->ShowTooltips(true);
}

void SfxEmojiControl::GrabFocus()
{
    mxEmojiView->GrabFocus();
}

SfxEmojiControl::~SfxEmojiControl()
{
}

void SfxEmojiControl::ConvertLabelToUnicode(weld::ToggleButton& rBtn)
{
    OUStringBuffer sHexText;
    OUString sLabel = rBtn.get_label();
    sHexText.appendUtf32(sLabel.toUInt32(16));
    rBtn.set_label(sHexText.toString());
}

FILTER_CATEGORY SfxEmojiControl::getFilter(const weld::Button& rCurPageId) const
{
    if (&rCurPageId == mxPeopleBtn.get())
        return FILTER_CATEGORY::PEOPLE;
    else if (&rCurPageId == mxNatureBtn.get())
        return FILTER_CATEGORY::NATURE;
    else if (&rCurPageId == mxFoodBtn.get())
        return FILTER_CATEGORY::FOOD;
    else if (&rCurPageId == mxActivityBtn.get())
        return FILTER_CATEGORY::ACTIVITY;
    else if (&rCurPageId == mxTravelBtn.get())
        return FILTER_CATEGORY::TRAVEL;
    else if (&rCurPageId == mxObjectsBtn.get())
        return FILTER_CATEGORY::OBJECTS;
    else if (&rCurPageId == mxSymbolsBtn.get())
        return FILTER_CATEGORY::SYMBOLS;
    else if (&rCurPageId == mxFlagsBtn.get())
        return FILTER_CATEGORY::FLAGS;
    else if (&rCurPageId == mxUnicode9Btn.get())
        return FILTER_CATEGORY::UNICODE9;

    return FILTER_CATEGORY::PEOPLE;
}

IMPL_LINK(SfxEmojiControl, ActivatePageHdl, weld::Button&, rButton, void)
{
    mxPeopleBtn->set_active(&rButton == mxPeopleBtn.get());
    mxNatureBtn->set_active(&rButton == mxNatureBtn.get());
    mxFoodBtn->set_active(&rButton == mxFoodBtn.get());
    mxActivityBtn->set_active(&rButton == mxActivityBtn.get());
    mxTravelBtn->set_active(&rButton == mxTravelBtn.get());
    mxObjectsBtn->set_active(&rButton == mxObjectsBtn.get());
    mxSymbolsBtn->set_active(&rButton == mxSymbolsBtn.get());
    mxFlagsBtn->set_active(&rButton == mxFlagsBtn.get());
    mxUnicode9Btn->set_active(&rButton == mxUnicode9Btn.get());

    mxEmojiView->filterItems(ViewFilter_Category(getFilter(rButton)));
}

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
