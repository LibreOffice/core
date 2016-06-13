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
    TemplateOnlineViewItem (ThumbnailView &rView, sal_uInt16 nId)
        : ThumbnailViewItem(rView, nId)
    {}

    virtual ~TemplateOnlineViewItem ()
    {}

    OUString msAuthorName;
    OUString msUrl;
    OUString maPath;
    OUString maDescription;
};

#endif // INCLUDED_SFX2_SOURCE_INC_TEMPLATEONLINEVIEWITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
