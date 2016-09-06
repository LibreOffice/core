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
#include <sfx2/templatecontaineritem.hxx>
#include <sfx2/templateviewitem.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/docfac.hxx>
#include <tools/urlobj.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/help.hxx>
#include <vcl/pngread.hxx>
#include <vcl/layout.hxx>
#include <unotools/moduleoptions.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>

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
using namespace ::com::sun::star::uno;

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

    return true;
}

TemplateAbstractView::TemplateAbstractView (vcl::Window *pParent, WinBits nWinStyle, bool bDisableTransientChildren)
    : ThumbnailView(pParent,nWinStyle,bDisableTransientChildren),
      mnCurRegionId(0),
      maSelectedItem(nullptr),
      mnThumbnailWidth(TEMPLATE_THUMBNAIL_MAX_WIDTH),
      mnThumbnailHeight(TEMPLATE_THUMBNAIL_MAX_HEIGHT),
      maPosition(0,0)
{
}

TemplateAbstractView::TemplateAbstractView(vcl::Window *pParent)
    : ThumbnailView(pParent),
      mnCurRegionId(0),
      maSelectedItem(nullptr),
      mnThumbnailWidth(TEMPLATE_THUMBNAIL_MAX_WIDTH),
      mnThumbnailHeight(TEMPLATE_THUMBNAIL_MAX_HEIGHT),
      maPosition(0,0)
{
}

TemplateAbstractView::~TemplateAbstractView()
{
    disposeOnce();
}

void TemplateAbstractView::dispose()
{
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

    AppendItem(pChild);

    CalculateItemPositions();
    Invalidate();
}

void TemplateAbstractView::insertItems(const std::vector<TemplateItemProperties> &rTemplates, bool isRegionSelected, bool bShowCategoryInTooltip)
{
    mItemList.clear();

    std::vector<ThumbnailViewItem*> aItems(rTemplates.size());
    for (size_t i = 0, n = rTemplates.size(); i < n; ++i )
    {
        const TemplateItemProperties *pCur = &rTemplates[i];

        TemplateViewItem *pChild;
        if(isRegionSelected)
            pChild = new TemplateViewItem(*this, pCur->nId);
        else
            pChild = new TemplateViewItem(*this, i+1);

        pChild->mnDocId = pCur->nDocId;
        pChild->mnRegionId = pCur->nRegionId;
        pChild->maTitle = pCur->aName;
        pChild->setPath(pCur->aPath);

        if(!bShowCategoryInTooltip)
            pChild->setHelpText(pCur->aName);
        else
        {
            OUString sHelpText = SfxResId(STR_TEMPLATE_TOOLTIP).toString();
            sHelpText = (sHelpText.replaceFirst("$1", pCur->aName)).replaceFirst("$2", pCur->aRegionName);
            pChild->setHelpText(sHelpText);
        }

        pChild->maPreview1 = pCur->aThumbnail;

        if(IsDefaultTemplate(pCur->aPath))
            pChild->showDefaultIcon(true);

        if ( pCur->aThumbnail.IsEmpty() )
        {
            // Use the default thumbnail if we have nothing else
            pChild->maPreview1 = TemplateAbstractView::getDefaultThumbnail(pCur->aPath);
        }

        aItems[i] = pChild;
    }

    updateItems(aItems);
}

void TemplateAbstractView::updateThumbnailDimensions(long itemMaxSize)
{
    mnThumbnailWidth = itemMaxSize;
    mnThumbnailHeight = itemMaxSize;
}


void TemplateAbstractView::MouseButtonDown( const MouseEvent& rMEvt )
{
    GrabFocus();
    ThumbnailView::MouseButtonDown(rMEvt);
}

void TemplateAbstractView::RequestHelp( const HelpEvent& rHEvt )
{
    if ( rHEvt.GetMode() & HelpEventMode::QUICK )
    {
        Rectangle aRect( OutputToScreenPixel( GetPosPixel() ), GetSizePixel() );
        Help::ShowQuickHelp( this, aRect, GetQuickHelpText(),
                             QuickHelpFlags::CtrlText | QuickHelpFlags::TipStyleBalloon );
        return;
    }

    ThumbnailView::RequestHelp( rHEvt );
}

void TemplateAbstractView::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == CommandEventId::ContextMenu )
    {
        if(rCEvt.IsMouseEvent())
        {
            deselectItems();
            size_t nPos = ImplGetItem(rCEvt.GetMousePosPixel());
            Point aPosition (rCEvt.GetMousePosPixel());
            maPosition = aPosition;
            ThumbnailViewItem* pItem = ImplGetItem(nPos);
            const TemplateViewItem *pViewItem = dynamic_cast<const TemplateViewItem*>(pItem);

            if(pViewItem)
            {
                maSelectedItem = dynamic_cast<TemplateViewItem*>(pItem);
                maCreateContextMenuHdl.Call(pItem);
            }
        }
        else
        {
            for (ThumbnailViewItem* pItem : mFilteredItemList)
            {
                //create context menu for the first selected item
                if (pItem->isSelected())
                {
                    deselectItems();
                    pItem->setSelection(true);
                    maItemStateHdl.Call(pItem);
                    Rectangle aRect = pItem->getDrawArea();
                    maPosition = aRect.Center();
                    maSelectedItem = dynamic_cast<TemplateViewItem*>(pItem);
                    maCreateContextMenuHdl.Call(pItem);
                    break;
                }
            }
        }
    }

    ThumbnailView::Command(rCEvt);
}

void TemplateAbstractView::KeyInput( const KeyEvent& rKEvt )
{
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();

    if(aKeyCode == ( KEY_MOD1 | KEY_A ) )
    {
        for (ThumbnailViewItem* pItem : mFilteredItemList)
        {
            if (!pItem->isSelected())
            {
                pItem->setSelection(true);
                maItemStateHdl.Call(pItem);
            }
        }

        if (IsReallyVisible() && IsUpdateMode())
            Invalidate();
        return;
    }
    else if( aKeyCode == KEY_DELETE && !mFilteredItemList.empty())
    {
        ScopedVclPtrInstance< MessageDialog > aQueryDlg(this, SfxResId(STR_QMSG_SEL_TEMPLATE_DELETE), VclMessageType::VCL_MESSAGE_QUESTION, VCL_BUTTONS_YES_NO);

        if ( aQueryDlg->Execute() != RET_YES )
            return;

        //copy to avoid changing filtered item list during deletion
        ThumbnailValueItemList mFilteredItemListCopy = mFilteredItemList;

        for (ThumbnailViewItem* pItem : mFilteredItemListCopy)
        {
            if (pItem->isSelected())
            {
                maDeleteTemplateHdl.Call(pItem);
            }
        }
        reload();
    }

    ThumbnailView::KeyInput(rKEvt);
}


void TemplateAbstractView::setOpenRegionHdl(const Link<void*,void> &rLink)
{
    maOpenRegionHdl = rLink;
}

void TemplateAbstractView::setCreateContextMenuHdl(const Link<ThumbnailViewItem*,void> &rLink)
{
    maCreateContextMenuHdl = rLink;
}

void TemplateAbstractView::setOpenTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink)
{
    maOpenTemplateHdl = rLink;
}

void TemplateAbstractView::setEditTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink)
{
    maEditTemplateHdl = rLink;
}

void TemplateAbstractView::setDeleteTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink)
{
    maDeleteTemplateHdl = rLink;
}

void TemplateAbstractView::setDefaultTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink)
{
    maDefaultTemplateHdl = rLink;
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

bool TemplateAbstractView::IsDefaultTemplate(const OUString& rPath)
{
    SvtModuleOptions aModOpt;
    const css::uno::Sequence<OUString> &aServiceNames = aModOpt.GetAllServiceNames();

    for( sal_Int32 i=0, nCount = aServiceNames.getLength(); i < nCount; ++i )
    {
        const OUString defaultPath = SfxObjectFactory::GetStandardTemplate( aServiceNames[i] );
        if(defaultPath.match(rPath))
            return true;
    }

    return false;
}

void TemplateAbstractView::RemoveDefaultTemplateIcon(const OUString& rPath)
{
    for (ThumbnailViewItem* pItem : mItemList)
    {
        TemplateViewItem* pViewItem = dynamic_cast<TemplateViewItem*>(pItem);
        if(pViewItem->getPath().match(rPath))
        {
            pViewItem->showDefaultIcon(false);
            Invalidate();
            return;
        }
    }
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

void TemplateAbstractView::OnItemDblClicked (ThumbnailViewItem *pItem)
{
    TemplateViewItem* pViewItem = dynamic_cast<TemplateViewItem*>(pItem);

    if( pViewItem )
        maOpenTemplateHdl.Call(pViewItem);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
