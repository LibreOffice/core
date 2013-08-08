/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/recentdocsviewitem.hxx>

#include <sfx2/sfxresid.hxx>
#include "../doc/doc.hrc"

#include <sfx2/templateabstractview.hxx>
#include <tools/urlobj.hxx>

RecentDocsViewItem::RecentDocsViewItem(ThumbnailView &rView, const OUString &rURL, const OUString &rTitle)
    : ThumbnailViewItem(rView)
{
    OUString aTitle = rTitle;
    if( !aTitle.getLength() )
    {
        // If we have no title, get filename from the URL
        INetURLObject aURLObj(rURL);
        aTitle = aURLObj.GetName(INetURLObject::DECODE_WITH_CHARSET);
    }

    BitmapEx aThumbnail = ThumbnailView::readThumbnail(rURL);
    if( aThumbnail.IsEmpty() )
    {
        // Use the default thumbnail if we have nothing else
        aThumbnail = TemplateAbstractView::getDefaultThumbnail(rURL);
    }
    if( aThumbnail.IsEmpty() )
    {
        // Last fallback
        aThumbnail = BitmapEx ( SfxResId( SFX_THUMBNAIL_TEXT ) );
    }

    maURL = rURL;
    maTitle = aTitle;
    maPreview1 = TemplateAbstractView::scaleImg(aThumbnail, 150, 150);
}

void RecentDocsViewItem::setEditTitle (bool edit, bool bChangeFocus)
{
    // Unused parameters.
    (void)edit;
    (void)bChangeFocus;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
