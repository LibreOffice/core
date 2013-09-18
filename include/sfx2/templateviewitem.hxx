/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef TEMPLATEVIEWITEM_HXX
#define TEMPLATEVIEWITEM_HXX

#include <sfx2/thumbnailviewitem.hxx>

class TemplateViewItem : public ThumbnailViewItem
{
public:

    TemplateViewItem (ThumbnailView &rView, sal_uInt16 nId);

    virtual ~TemplateViewItem ();

    void setPath (const OUString &rPath) { maPath = rPath; }

    const OUString& getPath () const { return maPath; }

    void setAuthor (const OUString &rAuthor) { maAuthor = rAuthor; }

    const OUString& getAuthor () const { return maAuthor; }

    void setKeywords (const OUString &rKeywords) { maKeywords = rKeywords; }

    const OUString& getKeywords () const { return maKeywords; }

    void setSubTitle (const OUString &rTitle) { maSubTitle = rTitle; }

    const OUString& getSubTitle () const { return maSubTitle; }

    virtual void calculateItemsPosition (const long nThumbnailHeight, const long nDisplayHeight,
                                         const long nPadding, sal_uInt32 nMaxTextLenght,
                                         const ThumbnailItemAttributes *pAttrs);

    virtual void Paint (drawinglayer::processor2d::BaseProcessor2D *pProcessor,
                        const ThumbnailItemAttributes *pAttrs);

    sal_uInt16    mnRegionId;
    sal_uInt16    mnDocId;

private:

    OUString maPath;
    OUString maAuthor;
    OUString maKeywords;
    OUString maSubTitle;

    Point maSubTitlePos;
};

#endif // TEMPLATEVIEWITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
