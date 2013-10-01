/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SFX2_RECENTDOCSVIEWITEM_HXX__
#define __SFX2_RECENTDOCSVIEWITEM_HXX__

#include <sfx2/thumbnailview.hxx>

struct RecentThumbnailInfo
{
    RecentThumbnailInfo(const OUString &rURL, const OUString &rTitle, sal_uInt16 nId_)
        :sURL(rURL), sTitle(rTitle), nId(nId_){}
    OUString    sURL;
    OUString    sTitle;
    sal_uInt16  nId;
};

class RecentDocsViewItem : public ThumbnailViewItem
{
public:
    RecentDocsViewItem(ThumbnailView &rView, const RecentThumbnailInfo&);
    virtual void setEditTitle (bool edit, bool bChangeFocus = true);

    OUString maURL;
};

#endif  // __SFX2_RECENTDOCSVIEWITEM_HXX__

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
