/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_SOURCE_INC_TEMPLATEONLINEVIEWITEM_HXX
#define INCLUDED_SFX2_SOURCE_INC_TEMPLATEONLINEVIEWITEM_HXX

#include <sfx2/thumbnailviewitem.hxx>

struct TemplateOnlineViewItem : public ThumbnailViewItem
{
    TemplateOnlineViewItem (ThumbnailView &rView, sal_uInt16 nId);

    virtual ~TemplateOnlineViewItem ();

    void setPath (const OUString &rPath) { msPath = rPath; }

    const OUString& getPath () const { return msPath; }

    void setUrl (const OUString &rUrl) { msUrl = rUrl; }

    const OUString& getUrl () const { return msUrl; }

    void setAuthorName (const OUString &rName) { msAuthorName = rName; }

    const OUString& getAuthorName () const { return msAuthorName; }

    void setDescription (const OUString &rDesc) { msDescription = rDesc; }

    const OUString& getDescription () const { return msDescription; }

    virtual void Paint (drawinglayer::processor2d::BaseProcessor2D *pProcessor,
                        const ThumbnailItemAttributes *pAttrs) override;

    OUString msAuthorName;
    OUString msUrl;
    OUString msPath;
    OUString msDescription;
};

#endif // INCLUDED_SFX2_SOURCE_INC_TEMPLATEONLINEVIEWITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
