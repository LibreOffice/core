/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/StorageFactory.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

#include <comphelper/base64.hxx>
#include <comphelper/propertyvalue.hxx>
#include <drawinglayer/primitive2d/discretebitmapprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <i18nutil/paper.hxx>
#include <officecfg/Office/Common.hxx>
#include <recentdocsview.hxx>
#include <sfx2/templatelocalview.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <unotools/historyoptions.hxx>
#include <vcl/event.hxx>
#include <vcl/filter/PngImageReader.hxx>
#include <vcl/ptrstyle.hxx>
#include <vcl/virdev.hxx>

#include <map>

#include <bitmaps.hlst>
#include "recentdocsviewitem.hxx"

using namespace basegfx;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace drawinglayer::primitive2d;
using namespace drawinglayer::processor2d;

namespace
{
bool IsDocEncrypted(const OUString& rURL)
{
    bool bIsEncrypted = false;

    try
    {
        auto xFactory = embed::StorageFactory::create(comphelper::getProcessComponentContext());
        auto xStorage(xFactory->createInstanceWithArguments(
            { uno::Any(rURL), uno::Any(embed::ElementModes::READ) }));
        if (uno::Reference<beans::XPropertySet> xStorageProps{ xStorage, uno::UNO_QUERY })
        {
            try
            {
                xStorageProps->getPropertyValue(u"HasEncryptedEntries"_ustr) >>= bIsEncrypted;
            }
            catch (uno::Exception&)
            {
            }
        }
    }
    catch (const uno::Exception&)
    {
        TOOLS_INFO_EXCEPTION("sfx", "caught exception trying to find out if doc <"
                                        << rURL << "> is encrypted:");
    }

    return bIsEncrypted;
}

using Ext2IconMap = std::map<sfx2::ApplicationType, OUString>;
BitmapEx Url2Icon(std::u16string_view rURL, const Ext2IconMap& rExtToIcon, const OUString& sDefault)
{
    auto it = std::find_if(rExtToIcon.begin(), rExtToIcon.end(),
                           [aExt = INetURLObject(rURL).getExtension()](const auto& r)
                           { return sfx2::RecentDocsView::typeMatchesExtension(r.first, aExt); });

    return BitmapEx(it != rExtToIcon.end() ? it->second : sDefault);
};

BitmapEx getDefaultThumbnail(const OUString& rURL)
{
    static const Ext2IconMap BitmapForExtension
        = { { sfx2::ApplicationType::TYPE_WRITER, SFX_FILE_THUMBNAIL_TEXT },
            { sfx2::ApplicationType::TYPE_CALC, SFX_FILE_THUMBNAIL_SHEET },
            { sfx2::ApplicationType::TYPE_IMPRESS, SFX_FILE_THUMBNAIL_PRESENTATION },
            { sfx2::ApplicationType::TYPE_DRAW, SFX_FILE_THUMBNAIL_DRAWING },
            { sfx2::ApplicationType::TYPE_DATABASE, SFX_FILE_THUMBNAIL_DATABASE },
            { sfx2::ApplicationType::TYPE_MATH, SFX_FILE_THUMBNAIL_MATH } };

    static const Ext2IconMap EncryptedBitmapForExtension
        = { { sfx2::ApplicationType::TYPE_WRITER, BMP_128X128_WRITER_DOC },
            { sfx2::ApplicationType::TYPE_CALC, BMP_128X128_CALC_DOC },
            { sfx2::ApplicationType::TYPE_IMPRESS, BMP_128X128_IMPRESS_DOC },
            { sfx2::ApplicationType::TYPE_DRAW, BMP_128X128_DRAW_DOC },
            // You can't save a database file with encryption -> no respective icon
            { sfx2::ApplicationType::TYPE_MATH, BMP_128X128_MATH_DOC } };

    const std::map<sfx2::ApplicationType, OUString>& rWhichMap
        = IsDocEncrypted(rURL) ? EncryptedBitmapForExtension : BitmapForExtension;

    return Url2Icon(rURL, rWhichMap, SFX_FILE_THUMBNAIL_DEFAULT);
}

BitmapEx getModuleOverlay(std::u16string_view rURL)
{
    static const Ext2IconMap OverlayBitmapForExtension
        = { { sfx2::ApplicationType::TYPE_WRITER, SFX_FILE_OVERLAY_TEXT },
            { sfx2::ApplicationType::TYPE_CALC, SFX_FILE_OVERLAY_SHEET },
            { sfx2::ApplicationType::TYPE_IMPRESS, SFX_FILE_OVERLAY_PRESENTATION },
            { sfx2::ApplicationType::TYPE_DRAW, SFX_FILE_OVERLAY_DRAWING },
            { sfx2::ApplicationType::TYPE_DATABASE, SFX_FILE_OVERLAY_DATABASE },
            { sfx2::ApplicationType::TYPE_MATH, SFX_FILE_OVERLAY_MATH } };

    return Url2Icon(rURL, OverlayBitmapForExtension, SFX_FILE_OVERLAY_DEFAULT);
}
};

RecentDocsViewItem::RecentDocsViewItem(sfx2::RecentDocsView &rView, const OUString &rURL,
    const OUString &rTitle, std::u16string_view const sThumbnailBase64,
        sal_uInt16 const nId, tools::Long const nThumbnailSize,
        bool const isReadOnly, bool const isPinned)
    : ThumbnailViewItem(rView, nId),
      mrParentView(rView),
      maURL(rURL),
      m_isReadOnly(isReadOnly),
      m_bRemoveIconHighlighted(false),
      m_aRemoveRecentBitmap(BMP_RECENTDOC_REMOVE),
      m_aRemoveRecentBitmapHighlighted(BMP_RECENTDOC_REMOVE_HIGHLIGHTED),
      m_bPinned(isPinned),
      m_bPinnedIconHighlighted(false),
      m_aPinnedDocumentBitmap(BMP_PIN_DOC),
      m_aPinnedDocumentBitmapHiglighted(BMP_PIN_DOC_HIGHLIGHTED)
{
    OUString aTitle(rTitle);
    INetURLObject aURLObj(rURL);

    if( aURLObj.GetProtocol() == INetProtocol::File )
        m_sHelpText = aURLObj.getFSysPath(FSysStyle::Detect);
    if( m_sHelpText.isEmpty() )
        m_sHelpText = aURLObj.GetURLNoPass();

    if (aTitle.isEmpty())
        aTitle = aURLObj.GetLastName(INetURLObject::DecodeMechanism::WithCharset);

    BitmapEx aThumbnail;

    //fdo#74834: only load thumbnail if the corresponding option is not disabled in the configuration
    if (officecfg::Office::Common::History::RecentDocsThumbnail::get())
    {
        if (!sThumbnailBase64.empty())
        {
            Sequence<sal_Int8> aDecoded;
            comphelper::Base64::decode(aDecoded, sThumbnailBase64);

            SvMemoryStream aStream(aDecoded.getArray(), aDecoded.getLength(), StreamMode::READ);
            vcl::PngImageReader aReader(aStream);
            aThumbnail = aReader.read();
        }
        else if (sfx2::RecentDocsView::typeMatchesExtension(sfx2::ApplicationType::TYPE_DATABASE,
                                                            aURLObj.getExtension()))
        {
            aThumbnail
                = BitmapEx(nThumbnailSize > 192 ? SFX_THUMBNAIL_BASE_256 : SFX_THUMBNAIL_BASE_192);
        }
    }

    if (aThumbnail.IsEmpty())
    {
        // 1. Thumbnail absent: get the default thumbnail, checking for encryption.
        BitmapEx aExt(getDefaultThumbnail(rURL));
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
        Size aThumbnailSize(std::round(nPaperWidth * ratio), std::round(nPaperHeight * ratio));

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
                aExt);
    }
    else
    {
        // 2. Thumbnail present: it's unencrypted document -> add a module overlay.
        // Pre-scale the thumbnail to the final size before applying the overlay
        aThumbnail = TemplateLocalView::scaleImg(aThumbnail, nThumbnailSize, nThumbnailSize);

        BitmapEx aModule = getModuleOverlay(rURL);
        aModule.Scale(Size(48,48)); //tdf#155200: Thumbnails don't change their size so overlay must not too
        if (!aModule.IsEmpty())
        {
            const Size aSize(aThumbnail.GetSizePixel());
            const Size aOverlaySize(aModule.GetSizePixel());
            ScopedVclPtr<VirtualDevice> pVirDev(VclPtr<VirtualDevice>::Create());
            pVirDev->SetOutputSizePixel(aSize);
            pVirDev->DrawBitmapEx(Point(), aThumbnail);
            pVirDev->DrawBitmapEx(Point(aSize.Width() - aOverlaySize.Width() - 5,
                                        aSize.Height() - aOverlaySize.Height() - 5),
                                  aModule);
            aThumbnail = pVirDev->GetBitmapEx(Point(), aSize);
        }
    }

    maTitle = aTitle;
    maPreview1 = aThumbnail;
}

::tools::Rectangle RecentDocsViewItem::updateHighlight(bool bVisible, const Point& rPoint)
{
    ::tools::Rectangle aRect(ThumbnailViewItem::updateHighlight(bVisible, rPoint));

    if (bVisible && getRemoveIconArea().Contains(rPoint))
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

    if (bVisible && getPinnedIconArea().Contains(rPoint))
    {
        if (!m_bPinnedIconHighlighted)
            aRect.Union(getPinnedIconArea());

        m_bPinnedIconHighlighted = true;
    }
    else
    {
        if (m_bPinnedIconHighlighted)
            aRect.Union(getPinnedIconArea());

        m_bPinnedIconHighlighted = false;
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

::tools::Rectangle RecentDocsViewItem::getPinnedIconArea() const
{
    return ::tools::Rectangle(maPinPos, m_aPinnedDocumentBitmap.GetSizePixel());
}

OUString RecentDocsViewItem::getHelpText() const
{
    return m_sHelpText;
}

void RecentDocsViewItem::Paint(drawinglayer::processor2d::BaseProcessor2D *pProcessor, const ThumbnailItemAttributes *pAttrs)
{
    ThumbnailViewItem::Paint(pProcessor, pAttrs);

    // paint the remove/pinned icon when hovered
    if (isHighlighted())
    {
        drawinglayer::primitive2d::Primitive2DContainer aSeq;

        Point aIconPos(getRemoveIconArea().TopLeft());

        aSeq.push_back(new DiscreteBitmapPrimitive2D(
                    m_bRemoveIconHighlighted ? m_aRemoveRecentBitmapHighlighted : m_aRemoveRecentBitmap,
                    B2DPoint(aIconPos.X(), aIconPos.Y())));

        // tdf#38742 - draw pinned icon
        const Point aPinnedIconPos(getPinnedIconArea().TopLeft());
        aSeq.push_back(new DiscreteBitmapPrimitive2D(
            m_aPinnedDocumentBitmap, B2DPoint(aPinnedIconPos.X(), aPinnedIconPos.Y())));

        pProcessor->process(aSeq);
    }
    // tdf#38742 - draw pinned icon if item is pinned
    else if (m_bPinned)
    {
        const Point aPinnedIconPos(getPinnedIconArea().TopLeft());
        drawinglayer::primitive2d::Primitive2DContainer aSeq {
            new DiscreteBitmapPrimitive2D(
                m_bPinnedIconHighlighted ? m_aPinnedDocumentBitmapHiglighted : m_aPinnedDocumentBitmap,
                B2DPoint(aPinnedIconPos.X(), aPinnedIconPos.Y())) };

        pProcessor->process(aSeq);
    }
}

void RecentDocsViewItem::MouseButtonUp(const MouseEvent& rMEvt)
{
    if (rMEvt.IsLeft())
    {
        if (getRemoveIconArea().Contains(rMEvt.GetPosPixel()))
        {
            SvtHistoryOptions::DeleteItem(EHistoryType::PickList, maURL);
            mrParent.Reload();
            return;
        }

        if (getPinnedIconArea().Contains(rMEvt.GetPosPixel()))
        {
            SvtHistoryOptions::TogglePinItem(EHistoryType::PickList, maURL);
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
    mrParentView.Disable();

    Reference<frame::XDispatch> xDispatch;
    css::util::URL aTargetURL;
    Sequence<beans::PropertyValue> aArgsList;

    uno::Reference<frame::XDesktop2> xDesktop = frame::Desktop::create(::comphelper::getProcessComponentContext());

    aTargetURL.Complete = maURL;
    Reference<util::XURLTransformer> xTrans(util::URLTransformer::create(::comphelper::getProcessComponentContext()));
    xTrans->parseStrict(aTargetURL);

    aArgsList = { comphelper::makePropertyValue(u"Referer"_ustr, u"private:user"_ustr),
                  // documents will never be opened as templates
                  comphelper::makePropertyValue(u"AsTemplate"_ustr, false) };
    if (m_isReadOnly) // tdf#149170 only add if true
    {
        aArgsList.realloc(aArgsList.size()+1);
        aArgsList.getArray()[aArgsList.size()-1] = comphelper::makePropertyValue(u"ReadOnly"_ustr, true);
    }

    xDispatch = xDesktop->queryDispatch(aTargetURL, u"_default"_ustr, 0);

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
