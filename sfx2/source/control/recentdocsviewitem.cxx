/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/recentdocsviewitem.hxx>

#include <i18nutil/paper.hxx>
#include <sfx2/templateabstractview.hxx>
#include <sfx2/recentdocsview.hxx>
#include <tools/urlobj.hxx>

RecentDocsViewItem::RecentDocsViewItem(ThumbnailView &rView, const OUString &rURL,
    const OUString &rTitle, sal_uInt16 nId)
    : ThumbnailViewItem(rView, nId),
      maURL(rURL)
{
    OUString aTitle(rTitle);
    INetURLObject aURLObj(rURL);
    RecentDocsView& rRecentView = dynamic_cast<RecentDocsView&>(rView);
    long nThumbnailSize = rRecentView.GetThumbnailSize();

    if (aTitle.isEmpty())
        aTitle = aURLObj.GetName(INetURLObject::DECODE_WITH_CHARSET);

    BitmapEx aThumbnail;
    if (aURLObj.GetProtocol() == INET_PROT_FILE)
        aThumbnail = ThumbnailView::readThumbnail(rURL);

    if (aThumbnail.IsEmpty())
    {
        // Use the default thumbnail if we have nothing else
        BitmapEx aExt(RecentDocsView::getDefaultThumbnail(rURL));
        Size aExtSize(aExt.GetSizePixel());

        // attempt to make it appear as if it is on a piece of paper
        INetURLObject aUrl(rURL);
        long nPaperHeight;
        long nPaperWidth;
        if( RecentDocsView::typeMatchesExtension(TYPE_IMPRESS, aUrl.getExtension()) )
        {
            // Swap width and height (PAPER_SCREEN_4_3 definition make it needed)
            PaperInfo aInfo(PAPER_SCREEN_4_3);
            nPaperHeight = aInfo.getWidth();
            nPaperWidth = aInfo.getHeight();
        }
        else
        {
            PaperInfo aInfo(PaperInfo::getSystemDefaultPaper());
            nPaperHeight = aInfo.getHeight();
            nPaperWidth = aInfo.getWidth();
        }
        double ratio = double(nThumbnailSize) / double(std::max(nPaperHeight, nPaperWidth));
        Size aThumbnailSize(nPaperWidth * ratio, nPaperHeight * ratio);

        if (aExtSize.Width() > aThumbnailSize.Width() || aExtSize.Height() > aThumbnailSize.Height())
        {
            aExt = TemplateAbstractView::scaleImg(aExt, aThumbnailSize.Width(), aThumbnailSize.Height());
            aExtSize = aExt.GetSizePixel();
        }

        // create empty, and copy the default thumbnail in
        sal_uInt8 nAlpha = 255;
        aThumbnail = BitmapEx(Bitmap(aThumbnailSize, 24), AlphaMask(aThumbnailSize, &nAlpha));

        aThumbnail.CopyPixel(
                Rectangle(Point((aThumbnailSize.Width() - aExtSize.Width()) / 2, (aThumbnailSize.Height() - aExtSize.Height()) / 2), aExtSize),
                Rectangle(Point(0, 0), aExtSize),
                &aExt);
    }

    maTitle = aTitle;
    maPreview1 = TemplateAbstractView::scaleImg(aThumbnail, nThumbnailSize, nThumbnailSize);
}

void RecentDocsViewItem::setEditTitle (bool edit, bool bChangeFocus)
{
    // Unused parameters.
    (void)edit;
    (void)bChangeFocus;
}

OUString RecentDocsViewItem::getHelpText() const
{
    return maURL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
