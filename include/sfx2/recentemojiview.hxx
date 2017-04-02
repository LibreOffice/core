/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_SOURCE_INC_RECENTEMOJIVIEW_HXX
#define INCLUDED_SFX2_SOURCE_INC_RECENTEMOJIVIEW_HXX

#include <sfx2/thumbnailview.hxx>
#include <sfx2/emojiview.hxx>

class EmojiViewItem;

class RecentEmojiView : public ThumbnailView
{
public:
    RecentEmojiView ( vcl::Window* pParent);

    virtual ~RecentEmojiView () override;

    // Fill view with recent emojis
    void Populate ();

    std::deque<OUString> getRecentEmojiList();

    void setInsertEmojiHdl (const Link<ThumbnailViewItem*, void> &rLink);

    void AppendItem(const OUString &rTitle, const OUString &rCategory, const OUString &rName );

protected:
    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;

    virtual void KeyInput( const KeyEvent& rKEvt ) override;

    virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;

protected:

    Link<ThumbnailViewItem*, void> maInsertEmojiHdl;
    std::deque<OUString> maRecentEmoji;
};

#endif // INCLUDED_SFX2_SOURCE_INC_RECENTEMOJIVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
