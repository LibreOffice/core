/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef TEMPLATEFOLDERVIEWITEM_HXX
#define TEMPLATEFOLDERVIEWITEM_HXX

#include <sfx2/templateproperties.hxx>
#include <sfx2/thumbnailviewitem.hxx>

class TemplateViewItem;

class TemplateLocalViewItem : public ThumbnailViewItem
{
public:

    BitmapEx maPreview2;
    std::vector<TemplateItemProperties> maTemplates;

    TemplateLocalViewItem (ThumbnailView &rView, Window *pParent);

    virtual ~TemplateLocalViewItem ();

    const Point& getPrev2Pos () const { return maPrev2Pos; }

    virtual void Paint (drawinglayer::processor2d::BaseProcessor2D *pProcessor,
                        const ThumbnailItemAttributes *pAttrs);
private:

    Point maPrev2Pos;
};

#endif // TEMPLATEFOLDERVIEWITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
