/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sfx2/thumbnailview.hxx>

//unicode item defines
#define ITEM_MAX_WIDTH 30
#define ITEM_MAX_HEIGHT 30
#define ITEM_PADDING 5
#define ITEM_MAX_TEXT_LENGTH 10

enum class FILTER_CATEGORY
{
    PEOPLE,
    NATURE,
    FOOD,
    ACTIVITY,
    TRAVEL,
    OBJECTS,
    SYMBOLS,
    FLAGS,
    UNICODE9
};

// Display unicode emojis depending on the category
class ViewFilter_Category final
{
public:
    ViewFilter_Category(FILTER_CATEGORY rCategory)
        : mCategory(rCategory)
    {
    }

    bool operator()(const ThumbnailViewItem* pItem);

    static bool isFilteredCategory(FILTER_CATEGORY filter, const OUString& rCategory);

private:
    FILTER_CATEGORY mCategory;
};

class EmojiView final : public ThumbnailView
{
public:
    EmojiView(std::unique_ptr<weld::ScrolledWindow> xWindow);

    virtual ~EmojiView() override;

    // Fill view with emojis
    void Populate();

    void setInsertEmojiHdl(const Link<ThumbnailViewItem*, void>& rLink);

    void AppendItem(const OUString& rTitle, const OUString& rCategory, const OUString& rName);

private:
    virtual bool MouseButtonDown(const MouseEvent& rMEvt) override;

    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

    virtual bool KeyInput(const KeyEvent& rKEvt) override;

    std::string msJSONData;

    Link<ThumbnailViewItem*, void> maInsertEmojiHdl;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
