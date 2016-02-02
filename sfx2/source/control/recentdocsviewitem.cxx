/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/recentdocsviewitem.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/primitive2d/discretebitmapprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <i18nutil/paper.hxx>
#include <sfx2/recentdocsview.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/templateabstractview.hxx>
#include <tools/urlobj.hxx>
#include <unotools/historyoptions.hxx>
#include <vcl/svapp.hxx>

#include <templateview.hrc>

using namespace basegfx;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace drawinglayer::primitive2d;
using namespace drawinglayer::processor2d;

RecentDocsViewItem::RecentDocsViewItem(ThumbnailView &rView, const OUString &rURL,
    const OUString &rTitle, const BitmapEx &rThumbnail, sal_uInt16 nId, long nThumbnailSize)
    : ThumbnailViewItem(rView, nId),
      maURL(rURL),
      m_bRemoveIconHighlighted(false),
      m_aRemoveRecentBitmap(SfxResId(IMG_RECENTDOC_REMOVE)),
      m_aRemoveRecentBitmapHighlighted(SfxResId(IMG_RECENTDOC_REMOVE_HIGHLIGHTED))
{
    OUString aTitle(rTitle);
    INetURLObject aURLObj(rURL);

    if( aURLObj.GetProtocol() == INetProtocol::File )
        m_sHelpText = aURLObj.getFSysPath(INetURLObject::FSYS_DETECT);
    if( m_sHelpText.isEmpty() )
        m_sHelpText = aURLObj.GetURLNoPass();

    if (aTitle.isEmpty())
        aTitle = aURLObj.GetName(INetURLObject::DECODE_WITH_CHARSET);

    BitmapEx aThumbnail(rThumbnail);
    if (aThumbnail.IsEmpty() && aURLObj.GetProtocol() == INetProtocol::File)
        aThumbnail = ThumbnailView::readThumbnail(rURL);

    if (aThumbnail.IsEmpty())
    {
        // Use the default thumbnail if we have nothing else
        BitmapEx aExt(RecentDocsView::getDefaultThumbnail(rURL));
        Size aExtSize(aExt.GetSizePixel());

        // attempt to make it appear as if it is on a piece of paper
        long nPaperHeight;
        long nPaperWidth;
        if( RecentDocsView::typeMatchesExtension(TYPE_IMPRESS, aURLObj.getExtension()) )
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

Rectangle RecentDocsViewItem::updateHighlight(bool bVisible, const Point& rPoint)
{
    Rectangle aRect(ThumbnailViewItem::updateHighlight(bVisible, rPoint));

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

Rectangle RecentDocsViewItem::getRemoveIconArea() const
{
    Rectangle aArea(getDrawArea());
    Size aSize(m_aRemoveRecentBitmap.GetSizePixel());

    return Rectangle(
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
        drawinglayer::primitive2d::Primitive2DSequence aSeq(1);

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
            SvtHistoryOptions().DeleteItem(ePICKLIST, maURL);
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
    mrParent.SetPointer(Pointer(PointerStyle::Wait));

    Reference<frame::XDispatch> xDispatch;
    Reference<frame::XDispatchProvider> xDispatchProvider;
    css::util::URL aTargetURL;
    Sequence<beans::PropertyValue> aArgsList;

    uno::Reference<frame::XDesktop2> xDesktop = frame::Desktop::create(::comphelper::getProcessComponentContext());
    uno::Reference<frame::XFrame> xActiveFrame = xDesktop->getActiveFrame();

    //osl::ClearableMutexGuard aLock(m_aMutex);
    xDispatchProvider.set(xActiveFrame, UNO_QUERY);
    //aLock.clear();

    aTargetURL.Complete = maURL;
    Reference<util::XURLTransformer> xTrans(util::URLTransformer::create(::comphelper::getProcessComponentContext()));
    xTrans->parseStrict(aTargetURL);

    sal_Int32 nSize = 2;
    aArgsList.realloc(nSize);
    aArgsList[0].Name = "Referer";
    aArgsList[0].Value = makeAny(OUString("private:user"));

    // documents will never be opened as templates
    aArgsList[1].Name = "AsTemplate";
    aArgsList[1].Value = makeAny(false);

    xDispatch = xDispatchProvider->queryDispatch(aTargetURL, "_default", 0);

    if (xDispatch.is())
    {
        // Call dispatch asynchronously as we can be destroyed while dispatch is
        // executed. VCL is not able to survive this as it wants to call listeners
        // after select!!!
        LoadRecentFile* pLoadRecentFile = new LoadRecentFile;
        pLoadRecentFile->xDispatch = xDispatch;
        pLoadRecentFile->aTargetURL = aTargetURL;
        pLoadRecentFile->aArgSeq = aArgsList;
        pLoadRecentFile->pView.set(&mrParent);

        Application::PostUserEvent(LINK(nullptr, RecentDocsView, ExecuteHdl_Impl), pLoadRecentFile, true);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
