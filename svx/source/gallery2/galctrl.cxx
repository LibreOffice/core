/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <config_features.h>

#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sfxsids.hrc>
#include <avmedia/mediaplayer.hxx>
#include <galbrws1.hxx>
#include <svx/galtheme.hxx>
#include <svx/galmisc.hxx>
#include <svx/galctrl.hxx>
#include <galobj.hxx>
#include <avmedia/mediawindow.hxx>
#include <vcl/event.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/graphicfilter.hxx>
#include <bitmaps.hlst>
#include <svl/itemset.hxx>

GalleryPreview::GalleryPreview(GalleryBrowser1* pParent, std::unique_ptr<weld::ScrolledWindow> xScrolledWindow)
    : mxScrolledWindow(std::move(xScrolledWindow))
    , mpParent(pParent)
    , mpTheme(nullptr)
{
}

void GalleryPreview::Show()
{
    mxScrolledWindow->show();
    weld::CustomWidgetController::Show();
}

void GalleryPreview::Hide()
{
    weld::CustomWidgetController::Hide();
    mxScrolledWindow->hide();
}

GalleryPreview::~GalleryPreview()
{
}

void GalleryPreview::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    Size aSize = pDrawingArea->get_ref_device().LogicToPixel(Size(70, 88), MapMode(MapUnit::MapAppFont));
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
    SetOutputSizePixel(aSize);

    mxDragDropTargetHelper.reset(new GalleryDragDrop(mpParent, pDrawingArea->get_drop_target()));
}

namespace
{
    bool ImplGetGraphicCenterRect(const weld::CustomWidgetController& rWidget, const Graphic& rGraphic, tools::Rectangle& rResultRect)
    {
        const Size  aWinSize(rWidget.GetOutputSizePixel());
        Size        aNewSize(rWidget.GetDrawingArea()->get_ref_device().LogicToPixel(rGraphic.GetPrefSize(), rGraphic.GetPrefMapMode()));
        bool        bRet = false;

        if( aNewSize.Width() && aNewSize.Height() )
        {
            // scale to fit window
            const double fGrfWH = static_cast<double>(aNewSize.Width()) / aNewSize.Height();
            const double fWinWH = static_cast<double>(aWinSize.Width()) / aWinSize.Height();

            if ( fGrfWH < fWinWH )
            {
                aNewSize.setWidth( static_cast<tools::Long>( aWinSize.Height() * fGrfWH ) );
                aNewSize.setHeight( aWinSize.Height() );
            }
            else
            {
                aNewSize.setWidth( aWinSize.Width() );
                aNewSize.setHeight( static_cast<tools::Long>( aWinSize.Width() / fGrfWH) );
            }

            const Point aNewPos( ( aWinSize.Width()  - aNewSize.Width() ) >> 1,
                                 ( aWinSize.Height() - aNewSize.Height() ) >> 1 );

            rResultRect = tools::Rectangle( aNewPos, aNewSize );
            bRet = true;
        }

        return bRet;
    }
}

bool GalleryPreview::ImplGetGraphicCenterRect( const Graphic& rGraphic, tools::Rectangle& rResultRect ) const
{
    return ::ImplGetGraphicCenterRect(*this, rGraphic, rResultRect);
}

void GalleryPreview::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*rRect*/)
{
    rRenderContext.SetBackground(Wallpaper(GALLERY_BG_COLOR));
    rRenderContext.Erase();

    if (ImplGetGraphicCenterRect(aGraphicObj.GetGraphic(), aPreviewRect))
    {
        const Point aPos( aPreviewRect.TopLeft() );
        const Size  aSize( aPreviewRect.GetSize() );

        if( aGraphicObj.IsAnimated() )
            aGraphicObj.StartAnimation(rRenderContext, aPos, aSize);
        else
            aGraphicObj.Draw(rRenderContext, aPos, aSize);
    }
}

bool GalleryPreview::MouseButtonDown(const MouseEvent& rMEvt)
{
    if (mpTheme && (rMEvt.GetClicks() == 2))
        mpParent->TogglePreview();
    return true;
}

bool GalleryPreview::Command(const CommandEvent& rCEvt)
{
    if (mpTheme && (rCEvt.GetCommand() == CommandEventId::ContextMenu))
    {
        mpParent->ShowContextMenu(rCEvt);
        return true;
    }
    return false;
}

bool GalleryPreview::KeyInput(const KeyEvent& rKEvt)
{
    if(mpTheme)
    {
        GalleryBrowser1* pBrowser = mpParent;

        switch( rKEvt.GetKeyCode().GetCode() )
        {
            case KEY_BACKSPACE:
                pBrowser->TogglePreview();
            break;

            case KEY_HOME:
                pBrowser->Travel( GalleryBrowserTravel::First );
            break;

            case KEY_END:
                pBrowser->Travel( GalleryBrowserTravel::Last );
            break;

            case KEY_LEFT:
            case KEY_UP:
                pBrowser->Travel( GalleryBrowserTravel::Previous );
            break;

            case KEY_RIGHT:
            case KEY_DOWN:
                pBrowser->Travel( GalleryBrowserTravel::Next );
            break;

            default:
            {
                if (!pBrowser->KeyInput(rKEvt))
                    return false;
            }
            break;
        }

        return true;
    }
    return false;
}

bool GalleryPreview::StartDrag()
{
    if (mpTheme)
        return mpParent->StartDrag();
    return true;
}

void GalleryPreview::PreviewMedia( const INetURLObject& rURL )
{
#if HAVE_FEATURE_AVMEDIA
    if (rURL.GetProtocol() == INetProtocol::NotValid)
        return;

    ::avmedia::MediaFloater* pFloater = avmedia::getMediaFloater();

    if (!pFloater)
    {
        if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
            pViewFrm->GetBindings().GetDispatcher()->Execute( SID_AVMEDIA_PLAYER, SfxCallMode::SYNCHRON );
        pFloater = avmedia::getMediaFloater();
    }

    if (pFloater)
        pFloater->setURL( rURL.GetMainURL( INetURLObject::DecodeMechanism::Unambiguous ), "", true );
#else
    (void) rURL;
#endif
}

DialogGalleryPreview::DialogGalleryPreview()
{
}

void DialogGalleryPreview::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    Size aSize(pDrawingArea->get_ref_device().LogicToPixel(Size(70, 88), MapMode(MapUnit::MapAppFont)));
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
}

bool DialogGalleryPreview::SetGraphic( const INetURLObject& _aURL )
{
    bool bRet = true;
    Graphic aGraphic;
#if HAVE_FEATURE_AVMEDIA
    if( ::avmedia::MediaWindow::isMediaURL( _aURL.GetMainURL( INetURLObject::DecodeMechanism::Unambiguous ), "" ) )
    {
        aGraphic = BitmapEx(RID_SVXBMP_GALLERY_MEDIA);
    }
    else
#endif
    {
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        GalleryProgress aProgress( &rFilter );
        if( rFilter.ImportGraphic( aGraphic, _aURL ) )
            bRet = false;
    }

    SetGraphic( aGraphic );
    Invalidate();
    return bRet;
}

bool DialogGalleryPreview::ImplGetGraphicCenterRect( const Graphic& rGraphic, tools::Rectangle& rResultRect ) const
{
    return ::ImplGetGraphicCenterRect(*this, rGraphic, rResultRect);
}

void DialogGalleryPreview::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    rRenderContext.SetBackground(Wallpaper(GALLERY_BG_COLOR));

    if (ImplGetGraphicCenterRect(aGraphicObj.GetGraphic(), aPreviewRect))
    {
        const Point aPos( aPreviewRect.TopLeft() );
        const Size  aSize( aPreviewRect.GetSize() );

        if( aGraphicObj.IsAnimated() )
            aGraphicObj.StartAnimation(rRenderContext, aPos, aSize);
        else
            aGraphicObj.Draw(rRenderContext, aPos, aSize);
    }
}

void GalleryIconView::drawTransparenceBackground(vcl::RenderContext& rOut, const Point& rPos, const Size& rSize)
{
    // draw checkered background
    static const sal_uInt32 nLen(8);
    static const Color aW(COL_WHITE);
    static const Color aG(0xef, 0xef, 0xef);

    rOut.DrawCheckered(rPos, rSize, nLen, aW, aG);
}

GalleryIconView::GalleryIconView(GalleryBrowser1* pParent, std::unique_ptr<weld::ScrolledWindow> xScrolledWindow)
    : ValueSet(std::move(xScrolledWindow))
    , mpParent(pParent)
    , mpTheme(nullptr)
{
}

GalleryIconView::~GalleryIconView()
{
}

void GalleryIconView::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    ValueSet::SetDrawingArea(pDrawingArea);

    SetStyle(GetStyle() | WB_TABSTOP | WB_3DLOOK | WB_BORDER | WB_ITEMBORDER | WB_DOUBLEBORDER | WB_VSCROLL | WB_FLATVALUESET);
    EnableFullItemMode( false );

    SetExtraSpacing( 2 );
    SetItemWidth( S_THUMB + 6 );
    SetItemHeight( S_THUMB + 6 );

    mxDragDropTargetHelper.reset(new GalleryDragDrop(mpParent, pDrawingArea->get_drop_target()));
}

void GalleryIconView::UserDraw(const UserDrawEvent& rUDEvt)
{
    const sal_uInt16 nId = rUDEvt.GetItemId();

    if (!nId || !mpTheme)
        return;

    const tools::Rectangle& rRect = rUDEvt.GetRect();
    const Size aSize(rRect.GetWidth(), rRect.GetHeight());
    BitmapEx aBitmapEx;
    Size aPreparedSize;
    OUString aItemTextTitle;
    OUString aItemTextPath;

    mpTheme->GetPreviewBitmapExAndStrings(nId - 1, aBitmapEx, aPreparedSize, aItemTextTitle, aItemTextPath);

    bool bNeedToCreate(aBitmapEx.IsEmpty());

    if (!bNeedToCreate && aItemTextTitle.isEmpty())
    {
        bNeedToCreate = true;
    }

    if (!bNeedToCreate && aPreparedSize != aSize)
    {
        bNeedToCreate = true;
    }

    if (bNeedToCreate)
    {
        std::unique_ptr<SgaObject> pObj = mpTheme->AcquireObject(nId - 1);

        if(pObj)
        {
            aBitmapEx = pObj->createPreviewBitmapEx(aSize);
            aItemTextTitle = GalleryBrowser1::GetItemText(*pObj, GalleryItemFlags::Title);

            mpTheme->SetPreviewBitmapExAndStrings(nId - 1, aBitmapEx, aSize, aItemTextTitle, aItemTextPath);
        }
    }

    if (!aBitmapEx.IsEmpty())
    {
        const Size aBitmapExSizePixel(aBitmapEx.GetSizePixel());
        const Point aPos(
            ((aSize.Width() - aBitmapExSizePixel.Width()) >> 1) + rRect.Left(),
            ((aSize.Height() - aBitmapExSizePixel.Height()) >> 1) + rRect.Top());
        OutputDevice* pDev = rUDEvt.GetRenderContext();

        if(aBitmapEx.IsAlpha())
        {
            // draw checkered background for full rectangle.
            GalleryIconView::drawTransparenceBackground(*pDev, rRect.TopLeft(), rRect.GetSize());
        }

        pDev->DrawBitmapEx(aPos, aBitmapEx);
    }

    SetItemText(nId, aItemTextTitle);
}

bool GalleryIconView::MouseButtonDown(const MouseEvent& rMEvt)
{
    bool bRet = ValueSet::MouseButtonDown(rMEvt);

    if (rMEvt.GetClicks() == 2)
        mpParent->TogglePreview();

    return bRet;
}

bool GalleryIconView::Command(const CommandEvent& rCEvt)
{
    bool bRet = ValueSet::Command(rCEvt);

    if (!bRet && rCEvt.GetCommand() == CommandEventId::ContextMenu)
    {
        bRet = mpParent->ShowContextMenu(rCEvt);
    }

    return bRet;
}

bool GalleryIconView::KeyInput(const KeyEvent& rKEvt)
{
    if (!mpTheme || !mpParent->KeyInput(rKEvt))
        return ValueSet::KeyInput(rKEvt);
    return true;
}

bool GalleryIconView::StartDrag()
{
    Select();
    return mpParent->StartDrag();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
