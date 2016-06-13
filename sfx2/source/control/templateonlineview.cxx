/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "templateonlineview.hxx"
#include "templateonlineviewitem.hxx"
#include <sfx2/templateabstractview.hxx>
#include <sfx2/sfxresid.hxx>
#include <tools/urlobj.hxx>
#include <vcl/layout.hxx>

#include <orcus/json_document_tree.hpp>
#include <orcus/config.hpp>
#include <orcus/pstring.hpp>

#include "../doc/doc.hrc"

#include <vcl/builderfactory.hxx>

#define MNI_OPEN               1
#define MNI_EDIT               2

TemplateOnlineView::TemplateOnlineView (vcl::Window *pParent, WinBits nWinStyle)
    : ThumbnailView(pParent,nWinStyle),
    msJSONQueryOutput(OUString()),
    maSelectedItem(nullptr),
    maPosition(0,0)
{
}

VCL_BUILDER_FACTORY(TemplateOnlineView)

TemplateOnlineView::~TemplateOnlineView()
{
    disposeOnce();
}

void TemplateOnlineView::Populate ()
{
    //TODO::populate view using the orcus json parser
}

void TemplateOnlineView::MouseButtonDown( const MouseEvent& rMEvt )
{
    GrabFocus();
    ThumbnailView::MouseButtonDown(rMEvt);
}

void TemplateOnlineView::KeyInput( const KeyEvent& rKEvt )
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

    ThumbnailView::KeyInput(rKEvt);
}

void TemplateOnlineView::Command( const CommandEvent& rCEvt )
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
            const TemplateOnlineViewItem *pViewItem = dynamic_cast<const TemplateOnlineViewItem*>(pItem);

            if(pViewItem)
            {
                maSelectedItem = dynamic_cast<TemplateOnlineViewItem*>(pItem);
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
                    maSelectedItem = dynamic_cast<TemplateOnlineViewItem*>(pItem);
                    maCreateContextMenuHdl.Call(pItem);
                    break;
                }
            }
        }
    }

    ThumbnailView::Command(rCEvt);
}

void TemplateOnlineView::createContextMenu()
{
    std::unique_ptr<PopupMenu> pItemMenu(new PopupMenu);
    pItemMenu->InsertItem(MNI_OPEN,SfxResId(STR_OPEN).toString());
    pItemMenu->InsertItem(MNI_EDIT,SfxResId(STR_EDIT_TEMPLATE).toString());
    maSelectedItem->setSelection(true);
    maItemStateHdl.Call(maSelectedItem);
    pItemMenu->SetSelectHdl(LINK(this, TemplateOnlineView, ContextMenuSelectHdl));
    pItemMenu->Execute(this, Rectangle(maPosition,Size(1,1)), PopupMenuFlags::ExecuteDown);
    Invalidate();
}

IMPL_LINK_TYPED(TemplateOnlineView, ContextMenuSelectHdl, Menu*, pMenu, bool)
{
    sal_uInt16 nMenuId = pMenu->GetCurItemId();

    switch(nMenuId)
    {
    case MNI_OPEN:
        maOpenTemplateHdl.Call(maSelectedItem);
        break;
    case MNI_EDIT:
        maEditTemplateHdl.Call(maSelectedItem);
        break;
    default:
        break;
    }

    return false;
}

void TemplateOnlineView::setCreateContextMenuHdl(const Link<ThumbnailViewItem*,void> &rLink)
{
    maCreateContextMenuHdl = rLink;
}

void TemplateOnlineView::setOpenTemplateHdl(const Link<ThumbnailViewItem*, void> &rLink)
{
    maOpenTemplateHdl = rLink;
}

void TemplateOnlineView::setEditTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink)
{
    maEditTemplateHdl = rLink;
}

void TemplateOnlineView::OnItemDblClicked (ThumbnailViewItem *pItem)
{
    maOpenTemplateHdl.Call(pItem);
}

void TemplateOnlineView::AppendItem(const OUString &rTitle, const OUString &rHelpText,
                                    const OUString &rPath, const OUString &rUrl,
                                    const OUString &rName, const OUString &rDesc,
                                    const BitmapEx &rImage)
{
    TemplateOnlineViewItem *pItem = new TemplateOnlineViewItem(*this, getNextItemId());

    if(!rImage.IsEmpty())
        pItem->maPreview1 = rImage;
    else
        pItem->maPreview1 = getDefaultThumbnail(rPath);

    pItem->maTitle = rTitle;
    pItem->setHelpText(rHelpText);
    pItem->setPath(rPath);
    pItem->setUrl(rUrl);
    pItem->setAuthorName(rName);
    pItem->setDescription(rDesc);

    ThumbnailView::AppendItem(pItem);

    CalculateItemPositions();
}

BitmapEx TemplateOnlineView::getDefaultThumbnail( const OUString& rPath )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
