/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef TEMPLATEFOLDERVIEWITEM_HXX
#define TEMPLATEFOLDERVIEWITEM_HXX

#include <sfx2/templateproperties.hxx>
#include <sfx2/thumbnailviewitem.hxx>

class TemplateContainerItem : public ThumbnailViewItem
{
public:

    sal_uInt16 mnRegionId;
    BitmapEx maPreview2;
    BitmapEx maPreview3;
    BitmapEx maPreview4;
    std::vector<TemplateItemProperties> maTemplates;

    TemplateContainerItem (ThumbnailView &rView, sal_uInt16 nId);

    virtual ~TemplateContainerItem ();

    virtual void Paint (drawinglayer::processor2d::BaseProcessor2D *pProcessor,
                        const ThumbnailItemAttributes *pAttrs);

    virtual void calculateItemsPosition (const long nThumbnailHeight, const long nDisplayHeight,
                                         const long nPadding, sal_uInt32 nMaxTextLenght,
                                         const ThumbnailItemAttributes *pAttrs);

    bool HasMissingPreview( );

private:

    Rectangle maThumbnailArea;
};

#endif // TEMPLATEFOLDERVIEWITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
