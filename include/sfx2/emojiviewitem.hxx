/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_SOURCE_INC_EMOJIVIEWITEM_HXX
#define INCLUDED_SFX2_SOURCE_INC_EMOJIVIEWITEM_HXX

#include <sfx2/thumbnailviewitem.hxx>

class EmojiViewItem : public ThumbnailViewItem
{
public:
    EmojiViewItem (ThumbnailView &rView, sal_uInt16 nId);

    virtual ~EmojiViewItem ();

    void setCategory (const OUString &rCategory) { msCategory = rCategory; }

    const OUString& getCategory () const { return msCategory; }

    virtual void Paint (drawinglayer::processor2d::BaseProcessor2D *pProcessor,
                        const ThumbnailItemAttributes *pAttrs) override;

    virtual void calculateItemsPosition (const long nThumbnailHeight, const long nDisplayHeight,
                                         const long nPadding, sal_uInt32 nMaxTextLength,
                                         const ThumbnailItemAttributes *pAttrs) override;
private:
    OUString msCategory;
};

#endif // INCLUDED_SFX2_SOURCE_INC_EMOJIVIEWITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
