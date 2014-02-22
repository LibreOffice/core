/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include <sfx2/recentdocsviewitem.hxx>

#include <i18nutil/paper.hxx>
#include <sfx2/templateabstractview.hxx>
#include <sfx2/recentdocsview.hxx>
#include <tools/urlobj.hxx>

RecentDocsViewItem::RecentDocsViewItem(ThumbnailView &rView, const OUString &rURL,
    const OUString &rTitle, const BitmapEx &rThumbnail, sal_uInt16 nId)
    : ThumbnailViewItem(rView, nId),
      maURL(rURL)
{
    OUString aTitle(rTitle);
    INetURLObject aURLObj(rURL);

    if( aURLObj.GetProtocol() == INET_PROT_FILE )
        m_sHelpText = aURLObj.getFSysPath(INetURLObject::FSYS_DETECT);
    if( m_sHelpText.isEmpty() )
        m_sHelpText = aURLObj.GetURLNoPass();

    RecentDocsView& rRecentView = dynamic_cast<RecentDocsView&>(rView);
    long nThumbnailSize = rRecentView.GetThumbnailSize();

    if (aTitle.isEmpty())
        aTitle = aURLObj.GetName(INetURLObject::DECODE_WITH_CHARSET);

    BitmapEx aThumbnail(rThumbnail);
    if (aThumbnail.IsEmpty() && aURLObj.GetProtocol() == INET_PROT_FILE)
        aThumbnail = ThumbnailView::readThumbnail(rURL);

    if (aThumbnail.IsEmpty())
    {
        
        BitmapEx aExt(RecentDocsView::getDefaultThumbnail(rURL));
        Size aExtSize(aExt.GetSizePixel());

        
        long nPaperHeight;
        long nPaperWidth;
        if( RecentDocsView::typeMatchesExtension(TYPE_IMPRESS, aURLObj.getExtension()) )
        {
            
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
    
    (void)edit;
    (void)bChangeFocus;
}

OUString RecentDocsViewItem::getHelpText() const
{
    return m_sHelpText;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
