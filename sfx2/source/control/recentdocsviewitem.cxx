/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <drawinglayer/primitive2d/discretebitmapprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <i18nutil/paper.hxx>
#include <officecfg/Office/Common.hxx>
#include <recentdocsview.hxx>
#include <sfx2/templatelocalview.hxx>
#include <tools/urlobj.hxx>
#include <unotools/historyoptions.hxx>
#include <vcl/event.hxx>
#include <vcl/ptrstyle.hxx>

#include <bitmaps.hlst>
#include "recentdocsviewitem.hxx"

using namespace basegfx;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace drawinglayer::primitive2d;
using namespace drawinglayer::processor2d;

RecentDocsViewItem::RecentDocsViewItem(sfx2::RecentDocsView &rView, const OUString &rURL,
    const OUString &rTitle, const BitmapEx &rThumbnail, sal_uInt16 nId, tools::Long nThumbnailSize)
    : ThumbnailViewItem(rView, nId),
      mrParentView(rView),
      maURL(rURL),
      m_bRemoveIconHighlighted(false),
      m_aRemoveRecentBitmap(BMP_RECENTDOC_REMOVE),
      m_aRemoveRecentBitmapHighlighted(BMP_RECENTDOC_REMOVE_HIGHLIGHTED)
{
    OUString aTitle(rTitle);
    INetURLObject aURLObj(rURL);

    if( aURLObj.GetProtocol() == INetProtocol::File )
        m_sHelpText = aURLObj.getFSysPath(FSysStyle::Detect);
    if( m_sHelpText.isEmpty() )
        m_sHelpText = aURLObj.GetURLNoPass();

    if (aTitle.isEmpty())
        aTitle = aURLObj.GetLastName(INetURLObject::DecodeMechanism::WithCharset);

    BitmapEx aThumbnail(rThumbnail);
    //fdo#74834: only load thumbnail if the corresponding option is not disabled in the configuration
    if (aThumbnail.IsEmpty() && aURLObj.GetProtocol() == INetProtocol::File &&
            officecfg::Office::Common::History::RecentDocsThumbnail::get())
        aThumbnail = ThumbnailView::readThumbnail(rURL);

    if (aThumbnail.IsEmpty())
    {
        // Use the default thumbnail if we have nothing else
        BitmapEx aExt(sfx2::RecentDocsView::getDefaultThumbnail(rURL));
        Size aExtSize(aExt.GetSizePixel());

        // attempt to make it appear as if it is on a piece of paper
        tools::Long nPaperHeight;
        tools::Long nPaperWidth;
        if (sfx2::RecentDocsView::typeMatchesExtension(
                sfx2::ApplicationType::TYPE_IMPRESS, aURLObj.getExtension()))
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
            aExt = TemplateLocalView::scaleImg(aExt, aThumbnailSize.Width(), aThumbnailSize.Height());
            aExtSize = aExt.GetSizePixel();
        }

        // create empty, and copy the default thumbnail in
        sal_uInt8 nAlpha = 255;
        aThumbnail = BitmapEx(Bitmap(aThumbnailSize, vcl::PixelFormat::N24_BPP), AlphaMask(aThumbnailSize, &nAlpha));

        aThumbnail.CopyPixel(
                ::tools::Rectangle(Point((aThumbnailSize.Width() - aExtSize.Width()) / 2, (aThumbnailSize.Height() - aExtSize.Height()) / 2), aExtSize),
                ::tools::Rectangle(Point(0, 0), aExtSize),
                &aExt);
    }

    maTitle = aTitle;
    maPreview1 = TemplateLocalView::scaleImg(aThumbnail, nThumbnailSize, nThumbnailSize);
}

::tools::Rectangle RecentDocsViewItem::updateHighlight(bool bVisible, const Point& rPoint)
{
    ::tools::Rectangle aRect(ThumbnailViewItem::updateHighlight(bVisible, rPoint));

    if (bVisible && getRemoveIconArea().IsInside(rPoint))
    {
        if (!m_bRemoveIconHighlighted)
            aRect.Union(getRemoveIconArea());

        m_bRemoveIconHighlighted = true;
    }
    else
    {
        if (m_bRemoveIconHighlighted)
            aRect.Union(getRemoveIconArea());

        m_bRemoveIconHighlighted = false;
    }

    return aRect;
}

::tools::Rectangle RecentDocsViewItem::getRemoveIconArea() const
{
    ::tools::Rectangle aArea(getDrawArea());
    Size aSize(m_aRemoveRecentBitmap.GetSizePixel());

    return ::tools::Rectangle(
            Point(aArea.Right() - aSize.Width() - THUMBNAILVIEW_ITEM_CORNER, aArea.Top() + THUMBNAILVIEW_ITEM_CORNER),
            aSize);
}

OUString RecentDocsViewItem::getHelpText() const
{
    return m_sHelpText;
}

void RecentDocsViewItem::Paint(drawinglayer::processor2d::BaseProcessor2D *pProcessor, const ThumbnailItemAttributes *pAttrs)
{
    ThumbnailViewItem::Paint(pProcessor, pAttrs);

    // paint the remove icon when highlighted
    if (isHighlighted())
    {
        drawinglayer::primitive2d::Primitive2DContainer aSeq(1);

        Point aIconPos(getRemoveIconArea().TopLeft());

        aSeq[0] = drawinglayer::primitive2d::Primitive2DReference(new DiscreteBitmapPrimitive2D(
                    m_bRemoveIconHighlighted ? m_aRemoveRecentBitmapHighlighted : m_aRemoveRecentBitmap,
                    B2DPoint(aIconPos.X(), aIconPos.Y())));

        pProcessor->process(aSeq);
    }
}

void RecentDocsViewItem::MouseButtonUp(const MouseEvent& rMEvt)
{
    if (rMEvt.IsLeft())
    {
        if (getRemoveIconArea().IsInside(rMEvt.GetPosPixel()))
        {
            SvtHistoryOptions().DeleteItem(EHistoryType::PickList, maURL);
            mrParent.Reload();
            return;
        }

        OpenDocument();
        return;
    }
}

void RecentDocsViewItem::OpenDocument()
{
    // show busy mouse pointer
    mrParentView.SetPointer(PointerStyle::Wait);

    Reference<frame::XDispatch> xDispatch;
    css::util::URL aTargetURL;
    Sequence<beans::PropertyValue> aArgsList;

    uno::Reference<frame::XDesktop2> xDesktop = frame::Desktop::create(::comphelper::getProcessComponentContext());

    aTargetURL.Complete = maURL;
    Reference<util::XURLTransformer> xTrans(util::URLTransformer::create(::comphelper::getProcessComponentContext()));
    xTrans->parseStrict(aTargetURL);

    aArgsList.realloc(2);
    aArgsList[0].Name = "Referer";
    aArgsList[0].Value <<= OUString("private:user");

    // documents will never be opened as templates
    aArgsList[1].Name = "AsTemplate";
    aArgsList[1].Value <<= false;

    xDispatch = xDesktop->queryDispatch(aTargetURL, "_default", 0);

    if (!xDispatch.is())
        return;

    // Call dispatch asynchronously as we can be destroyed while dispatch is
    // executed. VCL is not able to survive this as it wants to call listeners
    // after select!!!
    sfx2::LoadRecentFile *const pLoadRecentFile = new sfx2::LoadRecentFile;
    pLoadRecentFile->xDispatch = xDispatch;
    pLoadRecentFile->aTargetURL = aTargetURL;
    pLoadRecentFile->aArgSeq = aArgsList;
    pLoadRecentFile->pView = &mrParentView;

    mrParentView.PostLoadRecentUsedFile(pLoadRecentFile);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
