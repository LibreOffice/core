/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/templateabstractview.hxx>

#include <comphelper/processfactory.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/templatecontaineritem.hxx>
#include <sfx2/templateviewitem.hxx>
#include <tools/urlobj.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/pngread.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/StorageFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include "../doc/doc.hrc"
#include "templateview.hrc"

using namespace basegfx;
using namespace drawinglayer::primitive2d;

bool ViewFilter_Application::isFilteredExtension(FILTER_APPLICATION filter, const OUString &rExt)
{
    bool bRet = true;

    if (filter == FILTER_APPLICATION::WRITER)
    {
        bRet = rExt == "ott" || rExt == "stw" || rExt == "oth" || rExt == "dot" || rExt == "dotx" || rExt == "otm";
    }
    else if (filter == FILTER_APPLICATION::CALC)
    {
        bRet = rExt == "ots" || rExt == "stc" || rExt == "xlt" || rExt == "xltm" || rExt == "xltx";
    }
    else if (filter == FILTER_APPLICATION::IMPRESS)
    {
        bRet = rExt == "otp" || rExt == "sti" || rExt == "pot" || rExt == "potm" || rExt == "potx";
    }
    else if (filter == FILTER_APPLICATION::DRAW)
    {
        bRet = rExt == "otg" || rExt == "std";
    }

    return bRet;
}

bool ViewFilter_Application::isValid (const OUString &rPath) const
{
    INetURLObject aUrl(rPath);
    return isFilteredExtension(mApp, aUrl.getExtension());
}

bool ViewFilter_Application::operator () (const ThumbnailViewItem *pItem)
{
    const TemplateViewItem *pTempItem = dynamic_cast<const TemplateViewItem*>(pItem);
    if (pTempItem)
        return isValid(pTempItem->getPath());

    TemplateContainerItem *pContainerItem = const_cast<TemplateContainerItem*>(dynamic_cast<const TemplateContainerItem*>(pItem));
    if (pContainerItem)
    {
        std::vector<TemplateItemProperties> &rTemplates = pContainerItem->maTemplates;

        size_t nVisCount = 0;

        // Clear thumbnails
        pContainerItem->maPreview1.Clear();
        pContainerItem->maPreview2.Clear();
        pContainerItem->maPreview3.Clear();
        pContainerItem->maPreview4.Clear();

        for (size_t i = 0, n = rTemplates.size(); i < n && pContainerItem->HasMissingPreview(); ++i)
        {
            if (isValid(rTemplates[i].aPath))
            {
                ++nVisCount;
                if ( pContainerItem->maPreview1.IsEmpty( ) )
                {
                    pContainerItem->maPreview1 = TemplateAbstractView::scaleImg(rTemplates[i].aThumbnail,
                                                                       TEMPLATE_THUMBNAIL_MAX_WIDTH*0.75,
                                                                       TEMPLATE_THUMBNAIL_MAX_HEIGHT*0.75);
                }
                else if ( pContainerItem->maPreview2.IsEmpty() )
                {
                    pContainerItem->maPreview2 = TemplateAbstractView::scaleImg(rTemplates[i].aThumbnail,
                                                                       TEMPLATE_THUMBNAIL_MAX_WIDTH*0.75,
                                                                       TEMPLATE_THUMBNAIL_MAX_HEIGHT*0.75);
                }
                else if ( pContainerItem->maPreview3.IsEmpty() )
                {
                    pContainerItem->maPreview3 = TemplateAbstractView::scaleImg(rTemplates[i].aThumbnail,
                                                                       TEMPLATE_THUMBNAIL_MAX_WIDTH*0.75,
                                                                       TEMPLATE_THUMBNAIL_MAX_HEIGHT*0.75);
                }
                else if ( pContainerItem->maPreview4.IsEmpty() )
                {
                    pContainerItem->maPreview4 = TemplateAbstractView::scaleImg(rTemplates[i].aThumbnail,
                                                                       TEMPLATE_THUMBNAIL_MAX_WIDTH*0.75,
                                                                       TEMPLATE_THUMBNAIL_MAX_HEIGHT*0.75);
                }
            }
        }
    }
    return true;
}

bool ViewFilter_Keyword::operator ()(const ThumbnailViewItem *pItem)
{
    assert(pItem);

    return pItem->maTitle.matchIgnoreAsciiCase(maKeyword);
}

TemplateAbstractView::TemplateAbstractView (vcl::Window *pParent, WinBits nWinStyle, bool bDisableTransientChildren)
    : ThumbnailView(pParent,nWinStyle,bDisableTransientChildren),
      mnCurRegionId(0),
      maAllButton(VclPtr<PushButton>::Create(this, SfxResId(BTN_ALL_TEMPLATES))),
      maFTName(VclPtr<FixedText>::Create(this, SfxResId(FT_NAME)))
{
    maAllButton->Hide();
    maAllButton->SetStyle(maAllButton->GetStyle() | WB_FLATBUTTON);
    maAllButton->SetClickHdl(LINK(this,TemplateAbstractView,ShowRootRegionHdl));
    maFTName->Hide();
}

TemplateAbstractView::TemplateAbstractView(vcl::Window *pParent)
    : ThumbnailView(pParent),
      mnCurRegionId(0),
      maAllButton(VclPtr<PushButton>::Create(this, SfxResId(BTN_ALL_TEMPLATES))),
      maFTName(VclPtr<FixedText>::Create(this, SfxResId(FT_NAME)))
{
    maAllButton->Hide();
    maAllButton->SetStyle(maAllButton->GetStyle() | WB_FLATBUTTON);
    maAllButton->SetClickHdl(LINK(this,TemplateAbstractView,ShowRootRegionHdl));
    maFTName->Hide();
}

TemplateAbstractView::~TemplateAbstractView()
{
    disposeOnce();
}

void TemplateAbstractView::dispose()
{
    maAllButton.disposeAndClear();
    maFTName.disposeAndClear();
    ThumbnailView::dispose();
}

void TemplateAbstractView::insertItem(const TemplateItemProperties &rTemplate)
{
    const TemplateItemProperties *pCur = &rTemplate;

    TemplateViewItem *pChild = new TemplateViewItem(*this, pCur->nId);
    pChild->mnDocId = pCur->nDocId;
    pChild->mnRegionId = pCur->nRegionId;
    pChild->maTitle = pCur->aName;
    pChild->setPath(pCur->aPath);
    pChild->maPreview1 = pCur->aThumbnail;

    if ( pCur->aThumbnail.IsEmpty() )
    {
        // Use the default thumbnail if we have nothing else
        pChild->maPreview1 = TemplateAbstractView::getDefaultThumbnail(pCur->aPath);
    }

    pChild->setSelectClickHdl(LINK(this,ThumbnailView,OnItemSelected));

    AppendItem(pChild);

    CalculateItemPositions();
    Invalidate();
}

void TemplateAbstractView::insertItems(const std::vector<TemplateItemProperties> &rTemplates)
{
    std::vector<ThumbnailViewItem*> aItems(rTemplates.size());
    for (size_t i = 0, n = rTemplates.size(); i < n; ++i )
    {
        //TODO: CHECK IF THE ITEM IS A FOLDER OR NOT
        const TemplateItemProperties *pCur = &rTemplates[i];

        TemplateViewItem *pChild = new TemplateViewItem(*this, pCur->nId);
        pChild->mnDocId = pCur->nDocId;
        pChild->mnRegionId = pCur->nRegionId;
        pChild->maTitle = pCur->aName;
        pChild->setPath(pCur->aPath);
        pChild->maPreview1 = pCur->aThumbnail;

        if ( pCur->aThumbnail.IsEmpty() )
        {
            // Use the default thumbnail if we have nothing else
            pChild->maPreview1 = TemplateAbstractView::getDefaultThumbnail(pCur->aPath);
        }

        pChild->setSelectClickHdl(LINK(this,ThumbnailView,OnItemSelected));

        aItems[i] = pChild;
    }

    updateItems(aItems);
}




void TemplateAbstractView::setOpenRegionHdl(const Link<> &rLink)
{
    maOpenRegionHdl = rLink;
}

void TemplateAbstractView::setOpenTemplateHdl(const Link<> &rLink)
{
    maOpenTemplateHdl = rLink;
}

BitmapEx TemplateAbstractView::scaleImg (const BitmapEx &rImg, long width, long height)
{
    BitmapEx aImg = rImg;

    if (!rImg.IsEmpty())
    {
        Size aSize = rImg.GetSizePixel();

        if (aSize.Width() == 0)
            aSize.Width() = 1;

        if (aSize.Height() == 0)
            aSize.Height() = 1;

        // make the picture fit the given width/height constraints
        double nRatio = std::min(double(width)/double(aSize.Width()), double(height)/double(aSize.Height()));

        aImg.Scale(Size(aSize.Width() * nRatio, aSize.Height() * nRatio));
    }

    return aImg;
}

BitmapEx TemplateAbstractView::getDefaultThumbnail( const OUString& rPath )
{
    BitmapEx aImg;
    INetURLObject aUrl(rPath);
    OUString aExt = aUrl.getExtension();

    if ( ViewFilter_Application::isFilteredExtension( FILTER_APPLICATION::WRITER, aExt) )
        aImg = BitmapEx ( SfxResId( SFX_THUMBNAIL_TEXT ) );
    else if ( ViewFilter_Application::isFilteredExtension( FILTER_APPLICATION::CALC, aExt) )
        aImg = BitmapEx ( SfxResId( SFX_THUMBNAIL_SHEET ) );
    else if ( ViewFilter_Application::isFilteredExtension( FILTER_APPLICATION::IMPRESS, aExt) )
        aImg = BitmapEx ( SfxResId( SFX_THUMBNAIL_PRESENTATION ) );
    else if ( ViewFilter_Application::isFilteredExtension( FILTER_APPLICATION::DRAW, aExt) )
        aImg = BitmapEx ( SfxResId( SFX_THUMBNAIL_DRAWING ) );

    return aImg;
}

BitmapEx TemplateAbstractView::fetchThumbnail (const OUString &msURL, long width, long height)
{
    return TemplateAbstractView::scaleImg(ThumbnailView::readThumbnail(msURL), width, height);
}

IMPL_LINK_NOARG(TemplateAbstractView, ShowRootRegionHdl)
{
    showRootRegion();
    return 0;
}

void TemplateAbstractView::OnItemDblClicked (ThumbnailViewItem *pItem)
{
    //Check if the item is a TemplateContainerItem (Folder) or a TemplateViewItem (File)

    TemplateContainerItem* pContainerItem = dynamic_cast<TemplateContainerItem*>(pItem);
    if ( pContainerItem )
    {
        // Fill templates

        mnCurRegionId = pContainerItem->mnRegionId+1;
        maCurRegionName = pContainerItem->maTitle;
        maFTName->SetText(maCurRegionName);
        showRegion(pItem);
    }
    else
    {
        maOpenTemplateHdl.Call(pItem);
    }
}

void TemplateAbstractView::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    ThumbnailView::Paint(rRenderContext, rRect);

    Rectangle aRect(rRect.TopLeft(),
                    Point(rRect.BottomRight().X(),
                          mnHeaderHeight));

    drawinglayer::primitive2d::Primitive2DSequence aSeq(1);
    aSeq[0] = drawinglayer::primitive2d::Primitive2DReference(
        new PolyPolygonColorPrimitive2D(B2DPolyPolygon(Polygon(aRect).getB2DPolygon()),
        BColor(1.0, 1.0, 1.0)));

    mpProcessor->process(aSeq);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
