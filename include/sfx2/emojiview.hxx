/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_SOURCE_INC_EMOJIVIEW_HXX
#define INCLUDED_SFX2_SOURCE_INC_EMOJIVIEW_HXX

#include <sfx2/thumbnailview.hxx>

//unicode item defines
#define ITEM_MAX_WIDTH 20
#define ITEM_MAX_HEIGHT 20
#define ITEM_PADDING 5
#define ITEM_MAX_TEXT_LENGTH 10

class EmojiViewItem;

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
class ViewFilter_Category
{
public:

    ViewFilter_Category (FILTER_CATEGORY rCategory)
        : mCategory(rCategory)
    {}

    virtual ~ViewFilter_Category () {}

    bool operator () (const ThumbnailViewItem *pItem);

    static bool isFilteredCategory(FILTER_CATEGORY filter, const OUString &rCategory);

protected:

    FILTER_CATEGORY mCategory;
};


class EmojiView : public ThumbnailView
{
public:
    EmojiView ( vcl::Window* pParent, WinBits nWinStyle = WB_TABSTOP | WB_VSCROLL);

    virtual ~EmojiView ();

    // Fill view with emoji unicodes
    void Populate ();

    void setInsertEmojiHdl (const Link<ThumbnailViewItem*, void> &rLink);

    void AppendItem(const OUString &rTitle, const OUString &rCategory );

protected:
    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;

    virtual void KeyInput( const KeyEvent& rKEvt ) override;

protected:
    std::string msJSONData;

    Link<ThumbnailViewItem*, void> maInsertEmojiHdl;
};

#endif // INCLUDED_SFX2_SOURCE_INC_EMOJIVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
