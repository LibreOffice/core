/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
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

    TemplateViewItem (ThumbnailView &rView);

    virtual ~TemplateViewItem ();

    void setPath (const rtl::OUString &rPath) { maPath = rPath; }

    const rtl::OUString& getPath () const { return maPath; }

    void setAuthor (const rtl::OUString &rAuthor) { maAuthor = rAuthor; }

    const rtl::OUString& getAuthor () const { return maAuthor; }

    void setKeywords (const rtl::OUString &rKeywords) { maKeywords = rKeywords; }

    const rtl::OUString& getKeywords () const { return maKeywords; }

    void setSubTitle (const rtl::OUString &rTitle) { maSubTitle = rTitle; }

    const rtl::OUString& getSubTitle () const { return maSubTitle; }

    virtual void calculateItemsPosition (const long nThumbnailHeight, const long nDisplayHeight,
                                         const long nPadding, sal_uInt32 nMaxTextLenght,
                                         const ThumbnailItemAttributes *pAttrs);

    virtual void Paint (drawinglayer::processor2d::BaseProcessor2D *pProcessor,
                        const ThumbnailItemAttributes *pAttrs);

    sal_uInt16    mnRegionId;
    sal_uInt16    mnDocId;

private:

    rtl::OUString maPath;
    rtl::OUString maAuthor;
    rtl::OUString maKeywords;
    rtl::OUString maSubTitle;

    Point maSubTitlePos;
};

#endif // TEMPLATEVIEWITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
