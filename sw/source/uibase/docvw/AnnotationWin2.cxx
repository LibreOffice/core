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

#include <sal/config.h>

#include <cstddef>
#include <cstdlib>

#include "SidebarWinAcc.hxx"
#include <PostItMgr.hxx>
#include <AnnotationWin.hxx>
#include <basegfx/range/b2drange.hxx>
#include "SidebarTxtControl.hxx"
#include "SidebarScrollBar.hxx"
#include "AnchorOverlayObject.hxx"
#include "ShadowOverlayObject.hxx"
#include "OverlayRanges.hxx"

#include <strings.hrc>
#include <app.hrc>

#include <viewopt.hxx>
#include <cmdid.h>

#include <editeng/editobj.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/editview.hxx>
#include <editeng/outliner.hxx>
#include <editeng/editeng.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/outlobj.hxx>

#include <svl/zforlist.hxx>
#include <svl/undo.hxx>
#include <svl/stritem.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>

#include <vcl/scrbar.hxx>
#include <vcl/svapp.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/settings.hxx>
#include <vcl/ptrstyle.hxx>

#include <edtwin.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <doc.hxx>
#include <swmodule.hxx>
#include <langhelper.hxx>

#include <txtannotationfld.hxx>
#include <ndtxt.hxx>

#include <sw_primitivetypes2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/primitive2d/primitivetools2d.hxx>
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>
#include <drawinglayer/primitive2d/shadowprimitive2d.hxx>
#include <memory>
#include <comphelper/lok.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <drawdoc.hxx>

using namespace sw::sidebarwindows;

namespace
{

/// Translate absolute <-> relative twips: LOK wants absolute coordinates as output and gives absolute coordinates as input.
void lcl_translateTwips(vcl::Window const & rParent, vcl::Window& rChild, MouseEvent* pMouseEvent)
{
    // Set map mode, so that callback payloads will contain absolute coordinates instead of relative ones.
    Point aOffset(rChild.GetOutOffXPixel() - rParent.GetOutOffXPixel(), rChild.GetOutOffYPixel() - rParent.GetOutOffYPixel());
    if (!rChild.IsMapModeEnabled())
    {
        MapMode aMapMode(rChild.GetMapMode());
        aMapMode.SetMapUnit(MapUnit::MapTwip);
        aMapMode.SetScaleX(rParent.GetMapMode().GetScaleX());
        aMapMode.SetScaleY(rParent.GetMapMode().GetScaleY());
        rChild.SetMapMode(aMapMode);
        rChild.EnableMapMode();
    }
    aOffset = rChild.PixelToLogic(aOffset);
    MapMode aMapMode(rChild.GetMapMode());
    aMapMode.SetOrigin(aOffset);
    aMapMode.SetMapUnit(rParent.GetMapMode().GetMapUnit());
    rChild.SetMapMode(aMapMode);
    rChild.EnableMapMode(false);

    if (pMouseEvent)
    {
        // Set event coordinates, so they contain relative coordinates instead of absolute ones.
        Point aPos = pMouseEvent->GetPosPixel();
        aPos.Move(-aOffset.getX(), -aOffset.getY());
        MouseEvent aMouseEvent(aPos, pMouseEvent->GetClicks(), pMouseEvent->GetMode(), pMouseEvent->GetButtons(), pMouseEvent->GetModifier());
        *pMouseEvent = aMouseEvent;
    }
}

/// Decide which one from the children of rParent should get rMouseEvent.
vcl::Window* lcl_getHitWindow(sw::annotation::SwAnnotationWin& rParent, const MouseEvent& rMouseEvent)
{
    vcl::Window* pRet = nullptr;

    rParent.EditWin().Push(PushFlags::MAPMODE);
    rParent.EditWin().EnableMapMode();
    for (sal_Int16 i = rParent.GetChildCount() - 1; i >= 0; --i)
    {
        vcl::Window* pChild = rParent.GetChild(i);

        Point aPosition(rParent.GetPosPixel());
        aPosition.Move(pChild->GetPosPixel().getX(), pChild->GetPosPixel().getY());
        Size aSize(rParent.GetSizePixel());
        tools::Rectangle aRectangleLogic(rParent.EditWin().PixelToLogic(aPosition), rParent.EditWin().PixelToLogic(aSize));
        if (aRectangleLogic.IsInside(rMouseEvent.GetPosPixel()))
        {
            pRet = pChild;
            break;
        }
    }
    rParent.EditWin().Pop();
    return pRet;
}

}

namespace sw { namespace annotation {

#define METABUTTON_WIDTH        16
#define METABUTTON_HEIGHT       18
#define METABUTTON_AREA_WIDTH   30
#define POSTIT_META_HEIGHT  sal_Int32(30)
#define POSTIT_MINIMUMSIZE_WITHOUT_META     50


void SwAnnotationWin::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    Window::Paint(rRenderContext, rRect);

    if (mpMetadataAuthor->IsVisible())
    {
        //draw left over space
        if (Application::GetSettings().GetStyleSettings().GetHighContrastMode())
        {
            rRenderContext.SetFillColor(COL_BLACK);
        }
        else
        {
            rRenderContext.SetFillColor(mColorDark);
        }

        rRenderContext.SetLineColor();
        tools::Rectangle aRectangle(Point(mpMetadataAuthor->GetPosPixel().X() + mpMetadataAuthor->GetSizePixel().Width(),
                                   mpMetadataAuthor->GetPosPixel().Y()),
                             Size(GetMetaButtonAreaWidth(),
                                  mpMetadataAuthor->GetSizePixel().Height() + mpMetadataDate->GetSizePixel().Height()));

        if (comphelper::LibreOfficeKit::isActive())
            aRectangle = rRect;
        else
            aRectangle = PixelToLogic(aRectangle);
        rRenderContext.DrawRect(aRectangle);
    }
}

void SwAnnotationWin::PaintTile(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    Paint(rRenderContext, rRect);

    for (sal_uInt16 i = 0; i < GetChildCount(); ++i)
    {
        vcl::Window* pChild = GetChild(i);

        // No point in showing this button till click on it are not handled.
        if (pChild == mpMenuButton.get())
            continue;

        if (!pChild->IsVisible())
            continue;

        rRenderContext.Push(PushFlags::MAPMODE);
        Point aOffset(PixelToLogic(pChild->GetPosPixel()));
        MapMode aMapMode(rRenderContext.GetMapMode());
        aMapMode.SetOrigin(aMapMode.GetOrigin() + aOffset);
        rRenderContext.SetMapMode(aMapMode);

        bool bPopChild = false;
        if (pChild->GetMapMode().GetMapUnit() != rRenderContext.GetMapMode().GetMapUnit())
        {
            // This is needed for the scrollbar that has its map unit in pixels.
            pChild->Push(PushFlags::MAPMODE);
            bPopChild = true;
            pChild->EnableMapMode();
            aMapMode = pChild->GetMapMode();
            aMapMode.SetMapUnit(rRenderContext.GetMapMode().GetMapUnit());
            aMapMode.SetScaleX(rRenderContext.GetMapMode().GetScaleX());
            aMapMode.SetScaleY(rRenderContext.GetMapMode().GetScaleY());
            pChild->SetMapMode(aMapMode);
        }

        pChild->Paint(rRenderContext, rRect);

        if (bPopChild)
            pChild->Pop();
        rRenderContext.Pop();
    }

    const drawinglayer::geometry::ViewInformation2D aViewInformation;
    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor(drawinglayer::processor2d::createBaseProcessor2DFromOutputDevice(rRenderContext, aViewInformation));

    // drawinglayer sets the map mode to pixels, not needed here.
    rRenderContext.Pop();
    // Work in document-global twips.
    rRenderContext.Pop();
    if (mpAnchor)
        pProcessor->process(mpAnchor->getOverlayObjectPrimitive2DSequence());
    if (mpTextRangeOverlay)
        pProcessor->process(mpTextRangeOverlay->getOverlayObjectPrimitive2DSequence());

    rRenderContext.Push(PushFlags::NONE);
    pProcessor.reset();
    rRenderContext.Push(PushFlags::NONE);
}

bool SwAnnotationWin::IsHitWindow(const Point& rPointLogic)
{
    tools::Rectangle aRectangleLogic(EditWin().PixelToLogic(GetPosPixel()), EditWin().PixelToLogic(GetSizePixel()));
    return aRectangleLogic.IsInside(rPointLogic);
}

void SwAnnotationWin::SetCursorLogicPosition(const Point& rPosition, bool bPoint, bool bClearMark)
{
    mpSidebarTextControl->Push(PushFlags::MAPMODE);
    MouseEvent aMouseEvent(rPosition);
    lcl_translateTwips(EditWin(), *mpSidebarTextControl, &aMouseEvent);
    Point aPosition(aMouseEvent.GetPosPixel());

    EditView& rEditView = GetOutlinerView()->GetEditView();
    rEditView.SetCursorLogicPosition(aPosition, bPoint, bClearMark);

    mpSidebarTextControl->Pop();
}

void SwAnnotationWin::Draw(OutputDevice* pDev, const Point& rPt, const Size& rSz, DrawFlags nInFlags)
{
    if (mpMetadataAuthor->IsVisible() )
    {
        pDev->SetFillColor(mColorDark);
        pDev->SetLineColor();
        pDev->DrawRect( tools::Rectangle( rPt, rSz ) );
    }

    if (mpMetadataAuthor->IsVisible())
    {
        vcl::Font aOrigFont(mpMetadataAuthor->GetControlFont());
        Size aSize(PixelToLogic(mpMetadataAuthor->GetSizePixel()));
        Point aPos(PixelToLogic(mpMetadataAuthor->GetPosPixel()));
        aPos += rPt;
        vcl::Font aFont( mpMetadataAuthor->GetSettings().GetStyleSettings().GetFieldFont() );
        mpMetadataAuthor->SetControlFont( aFont );
        mpMetadataAuthor->Draw(pDev, aPos, aSize, nInFlags);
        mpMetadataAuthor->SetControlFont( aOrigFont );
    }

    if (mpMetadataDate->IsVisible())
    {
        vcl::Font aOrigFont(mpMetadataDate->GetControlFont());
        Size aSize(PixelToLogic(mpMetadataDate->GetSizePixel()));
        Point aPos(PixelToLogic(mpMetadataDate->GetPosPixel()));
        aPos += rPt;
        vcl::Font aFont( mpMetadataDate->GetSettings().GetStyleSettings().GetFieldFont() );
        mpMetadataDate->SetControlFont( aFont );
        mpMetadataDate->Draw(pDev, aPos, aSize, nInFlags);
        mpMetadataDate->SetControlFont( aOrigFont );
    }

    mpSidebarTextControl->Draw(pDev, rPt, rSz, nInFlags);

    const drawinglayer::geometry::ViewInformation2D aNewViewInfos;
    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor(
        drawinglayer::processor2d::createBaseProcessor2DFromOutputDevice(
            *pDev, aNewViewInfos ));

    if (mpAnchor)
        pProcessor->process(mpAnchor->getOverlayObjectPrimitive2DSequence());
    if (mpTextRangeOverlay)
        pProcessor->process(mpTextRangeOverlay->getOverlayObjectPrimitive2DSequence());
    pProcessor.reset();

    if (!mpVScrollbar->IsVisible())
        return;

    vcl::Font aOrigFont(mpMetadataDate->GetControlFont());
    Color aOrigBg( mpMetadataDate->GetControlBackground() );
    OUString sOrigText(mpMetadataDate->GetText());

    Size aSize(PixelToLogic(mpMenuButton->GetSizePixel()));
    Point aPos(PixelToLogic(mpMenuButton->GetPosPixel()));
    aPos += rPt;

    vcl::Font aFont( mpMetadataDate->GetSettings().GetStyleSettings().GetFieldFont() );
    mpMetadataDate->SetControlFont( aFont );
    mpMetadataDate->SetControlBackground( Color(0xFFFFFF) );
    mpMetadataDate->SetText("...");
    mpMetadataDate->Draw(pDev, aPos, aSize, nInFlags);

    mpMetadataDate->SetText(sOrigText);
    mpMetadataDate->SetControlFont( aOrigFont );
    mpMetadataDate->SetControlBackground( aOrigBg );

}

void SwAnnotationWin::KeyInput(const KeyEvent& rKeyEvent)
{
    if (mpSidebarTextControl)
    {
        mpSidebarTextControl->Push(PushFlags::MAPMODE);
        lcl_translateTwips(EditWin(), *mpSidebarTextControl, nullptr);

        mpSidebarTextControl->KeyInput(rKeyEvent);

        mpSidebarTextControl->Pop();
    }
}

void SwAnnotationWin::MouseMove(const MouseEvent& rMouseEvent)
{
    if (vcl::Window* pHit = lcl_getHitWindow(*this, rMouseEvent))
    {
        pHit->Push(PushFlags::MAPMODE);
        MouseEvent aMouseEvent(rMouseEvent);
        lcl_translateTwips(EditWin(), *pHit, &aMouseEvent);

        pHit->MouseMove(aMouseEvent);

        pHit->Pop();
    }
}

void SwAnnotationWin::MouseButtonDown(const MouseEvent& rMouseEvent)
{
    if (vcl::Window* pHit = lcl_getHitWindow(*this, rMouseEvent))
    {
        pHit->Push(PushFlags::MAPMODE);
        MouseEvent aMouseEvent(rMouseEvent);
        lcl_translateTwips(EditWin(), *pHit, &aMouseEvent);

        pHit->MouseButtonDown(aMouseEvent);

        pHit->Pop();
    }
}

void SwAnnotationWin::MouseButtonUp(const MouseEvent& rMouseEvent)
{
    if (vcl::Window* pHit = lcl_getHitWindow(*this, rMouseEvent))
    {
        pHit->Push(PushFlags::MAPMODE);
        MouseEvent aMouseEvent(rMouseEvent);
        lcl_translateTwips(EditWin(), *pHit, &aMouseEvent);

        pHit->MouseButtonUp(aMouseEvent);

        pHit->Pop();
    }
}

void SwAnnotationWin::SetPosSizePixelRect(long nX, long nY, long nWidth, long nHeight,
                                       const SwRect& aAnchorRect, const long aPageBorder)
{
    mPosSize = tools::Rectangle(Point(nX,nY),Size(nWidth,nHeight));
    if (!mAnchorRect.IsEmpty() && mAnchorRect != aAnchorRect)
        mbAnchorRectChanged = true;
    mAnchorRect = aAnchorRect;
    mPageBorder = aPageBorder;
}

void SwAnnotationWin::SetSize( const Size& rNewSize )
{
    mPosSize.SetSize(rNewSize);
}

void SwAnnotationWin::SetVirtualPosSize( const Point& aPoint, const Size& aSize)
{
    mPosSize = tools::Rectangle(aPoint,aSize);
}

void SwAnnotationWin::TranslateTopPosition(const long aAmount)
{
    mPosSize.Move(0,aAmount);
}

void SwAnnotationWin::ShowAnchorOnly(const Point &aPoint)
{
    HideNote();
    SetPosAndSize();
    if (mpAnchor)
    {
        mpAnchor->SetSixthPosition(basegfx::B2DPoint(aPoint.X(),aPoint.Y()));
        mpAnchor->SetSeventhPosition(basegfx::B2DPoint(aPoint.X(),aPoint.Y()));
        mpAnchor->SetAnchorState(AnchorState::All);
        mpAnchor->setVisible(true);
    }
    if (mpShadow)
        mpShadow->setVisible(false);
}

SfxItemSet SwAnnotationWin::DefaultItem()
{
    SfxItemSet aItem( mrView.GetDocShell()->GetPool() );
    aItem.Put(SvxFontHeightItem(200,100,EE_CHAR_FONTHEIGHT));
    return aItem;
}

void SwAnnotationWin::InitControls()
{
    AddEventListener( LINK( this, SwAnnotationWin, WindowEventListener ) );

    // actual window which holds the user text
    mpSidebarTextControl = VclPtr<SidebarTextControl>::Create( *this,
                                                 WB_NODIALOGCONTROL,
                                                 mrView, mrMgr );
    mpSidebarTextControl->SetPointer(PointerStyle::Text);

    // window controls for author and date
    mpMetadataAuthor = VclPtr<Edit>::Create( this, 0 );
    mpMetadataAuthor->SetAccessibleName( SwResId( STR_ACCESS_ANNOTATION_AUTHOR_NAME ) );
    mpMetadataAuthor->EnableRTL(AllSettings::GetLayoutRTL());
    mpMetadataAuthor->SetReadOnly();
    mpMetadataAuthor->AlwaysDisableInput(true);
    mpMetadataAuthor->SetCallHandlersOnInputDisabled(true);
    mpMetadataAuthor->AddEventListener( LINK( this, SwAnnotationWin, WindowEventListener ) );
    // we should leave this setting alone, but for this we need a better layout algo
    // with variable meta size height
    {
        AllSettings aSettings = mpMetadataAuthor->GetSettings();
        StyleSettings aStyleSettings = aSettings.GetStyleSettings();
        vcl::Font aFont = aStyleSettings.GetFieldFont();
        aFont.SetFontHeight(8);
        aStyleSettings.SetFieldFont(aFont);
        aSettings.SetStyleSettings(aStyleSettings);
        mpMetadataAuthor->SetSettings(aSettings);
    }

    mpMetadataDate = VclPtr<Edit>::Create( this, 0 );
    mpMetadataDate->SetAccessibleName( SwResId( STR_ACCESS_ANNOTATION_DATE_NAME ) );
    mpMetadataDate->EnableRTL(AllSettings::GetLayoutRTL());
    mpMetadataDate->SetReadOnly();
    mpMetadataDate->AlwaysDisableInput(true);
    mpMetadataDate->SetCallHandlersOnInputDisabled(true);
    mpMetadataDate->AddEventListener( LINK( this, SwAnnotationWin, WindowEventListener ) );
    // we should leave this setting alone, but for this we need a better layout algo
    // with variable meta size height
    {
        AllSettings aSettings = mpMetadataDate->GetSettings();
        StyleSettings aStyleSettings = aSettings.GetStyleSettings();
        vcl::Font aFont = aStyleSettings.GetFieldFont();
        aFont.SetFontHeight(8);
        aStyleSettings.SetFieldFont(aFont);
        aSettings.SetStyleSettings(aStyleSettings);
        mpMetadataDate->SetSettings(aSettings);
    }

    SwDocShell* aShell = mrView.GetDocShell();
    mpOutliner.reset(new Outliner(&aShell->GetPool(),OutlinerMode::TextObject));
    aShell->GetDoc()->SetCalcFieldValueHdl( mpOutliner.get() );
    mpOutliner->SetUpdateMode( true );
    Rescale();

    mpSidebarTextControl->EnableRTL( false );
    mpOutlinerView.reset(new OutlinerView ( mpOutliner.get(), mpSidebarTextControl ));
    mpOutlinerView->SetBackgroundColor(COL_TRANSPARENT);
    mpOutliner->InsertView(mpOutlinerView.get() );
    mpOutlinerView->SetOutputArea( PixelToLogic( tools::Rectangle(0,0,1,1) ) );

    mpOutlinerView->SetAttribs(DefaultItem());

    if (comphelper::LibreOfficeKit::isActive())
    {
        // If there is a callback already registered, inform the new outliner view about it.
        mpOutlinerView->RegisterViewShell(&mrView);
    }

    //create Scrollbars
    mpVScrollbar = VclPtr<SidebarScrollBar>::Create(*this, WB_3DLOOK |WB_VSCROLL|WB_DRAG, mrView);
    mpVScrollbar->EnableNativeWidget(false);
    mpVScrollbar->EnableRTL( false );
    mpVScrollbar->SetScrollHdl(LINK(this, SwAnnotationWin, ScrollHdl));
    mpVScrollbar->EnableDrag();
    mpVScrollbar->AddEventListener( LINK( this, SwAnnotationWin, WindowEventListener ) );

    const SwViewOption* pVOpt = mrView.GetWrtShellPtr()->GetViewOptions();
    EEControlBits nCntrl = mpOutliner->GetControlWord();
    // TODO: crash when AUTOCOMPLETE enabled
    nCntrl |= EEControlBits::MARKFIELDS | EEControlBits::PASTESPECIAL | EEControlBits::AUTOCORRECT | EEControlBits::USECHARATTRIBS; // | EEControlBits::AUTOCOMPLETE;
    if (SwViewOption::IsFieldShadings())
        nCntrl |= EEControlBits::MARKFIELDS;
    else
        nCntrl &= ~EEControlBits::MARKFIELDS;
    if (pVOpt->IsOnlineSpell())
        nCntrl |= EEControlBits::ONLINESPELLING;
    else
        nCntrl &= ~EEControlBits::ONLINESPELLING;
    mpOutliner->SetControlWord(nCntrl);

    std::size_t aIndex = SW_MOD()->InsertRedlineAuthor(GetAuthor());
    SetColor( SwPostItMgr::GetColorDark(aIndex),
              SwPostItMgr::GetColorLight(aIndex),
              SwPostItMgr::GetColorAnchor(aIndex));

    CheckMetaText();

    mpMenuButton = CreateMenuButton();

    SetLanguage(GetLanguage());
    GetOutlinerView()->StartSpeller();
    SetPostItText();
    mpOutliner->CompleteOnlineSpelling();

    mpSidebarTextControl->Show();
    mpMetadataAuthor->Show();
    mpMetadataDate->Show();
    mpVScrollbar->Show();
}

void SwAnnotationWin::CheckMetaText()
{
    const SvtSysLocale aSysLocale;
    const LocaleDataWrapper& rLocalData = aSysLocale.GetLocaleData();
    OUString sMeta = GetAuthor();
    if (sMeta.isEmpty())
    {
        sMeta = SwResId(STR_NOAUTHOR);
    }
    else if (sMeta.getLength() > 23)
    {
        sMeta = sMeta.copy(0, 20) + "...";
    }
    if ( mpMetadataAuthor->GetText() != sMeta )
    {
        mpMetadataAuthor->SetText(sMeta);
    }

    Date aDate = GetDate();
    if (aDate.IsValidAndGregorian() )
    {
        sMeta = rLocalData.getDate(aDate);
    }
    else
    {
        sMeta = SwResId(STR_NODATE);
    }
    if (GetTime().GetTime()!=0)
    {
        sMeta += " " + rLocalData.getTime( GetTime(),false );
    }
    if ( mpMetadataDate->GetText() != sMeta )
    {
        mpMetadataDate->SetText(sMeta);
    }
}

void SwAnnotationWin::Rescale()
{
    MapMode aMode = GetParent()->GetMapMode();
    aMode.SetOrigin( Point() );
    mpOutliner->SetRefMapMode( aMode );
    SetMapMode( aMode );
    mpSidebarTextControl->SetMapMode( aMode );
    const Fraction& rFraction = mrView.GetWrtShellPtr()->GetOut()->GetMapMode().GetScaleY();
    if ( mpMetadataAuthor )
    {
        vcl::Font aFont( mpMetadataAuthor->GetSettings().GetStyleSettings().GetFieldFont() );
        sal_Int32 nHeight = long(aFont.GetFontHeight() * rFraction);
        aFont.SetFontHeight( nHeight );
        mpMetadataAuthor->SetControlFont( aFont );
    }
    if ( mpMetadataDate )
    {
        vcl::Font aFont( mpMetadataDate->GetSettings().GetStyleSettings().GetFieldFont() );
        sal_Int32 nHeight = long(aFont.GetFontHeight() * rFraction);
        aFont.SetFontHeight( nHeight );
        mpMetadataDate->SetControlFont( aFont );
    }
}

void SwAnnotationWin::SetPosAndSize()
{
    bool bChange = false;

    if (GetSizePixel() != mPosSize.GetSize())
    {
        bChange = true;
        SetSizePixel(mPosSize.GetSize());

        if (comphelper::LibreOfficeKit::isActive())
        {
            // Position is not yet set at VCL level, but the map mode should
            // contain the right origin to emit the correct cursor position.
            mpSidebarTextControl->Push(PushFlags::MAPMODE);
            Point aOffset(mPosSize.Left(), mPosSize.Top());
            aOffset = PixelToLogic(aOffset);
            MapMode aMapMode(mpSidebarTextControl->GetMapMode());
            aMapMode.SetOrigin(aOffset);
            mpSidebarTextControl->SetMapMode(aMapMode);
            mpSidebarTextControl->EnableMapMode(false);
        }

        DoResize();

        if (comphelper::LibreOfficeKit::isActive())
            mpSidebarTextControl->Pop();
    }

    if (GetPosPixel().X() != mPosSize.TopLeft().X() || (std::abs(GetPosPixel().Y() - mPosSize.TopLeft().Y()) > 5) )
    {
        bChange = true;
        SetPosPixel(mPosSize.TopLeft());

        Point aLineStart;
        Point aLineEnd ;
        switch ( meSidebarPosition )
        {
            case sw::sidebarwindows::SidebarPosition::LEFT:
            {
                aLineStart = EditWin().PixelToLogic( Point(GetPosPixel().X()+GetSizePixel().Width(),GetPosPixel().Y()-1) );
                aLineEnd = EditWin().PixelToLogic( Point(GetPosPixel().X(),GetPosPixel().Y()-1) );
            }
            break;
            case sw::sidebarwindows::SidebarPosition::RIGHT:
            {
                aLineStart = EditWin().PixelToLogic( Point(GetPosPixel().X(),GetPosPixel().Y()-1) );
                aLineEnd = EditWin().PixelToLogic( Point(GetPosPixel().X()+GetSizePixel().Width(),GetPosPixel().Y()-1) );
            }
            break;
            default:
                OSL_FAIL( "<SwAnnotationWin::SetPosAndSize()> - unexpected position of sidebar" );
            break;
        }

        // LOK has map mode disabled, and we still want to perform pixel ->
        // twips conversion for the size of the line above the note.
        if (comphelper::LibreOfficeKit::isActive() && !EditWin().IsMapModeEnabled())
        {
            EditWin().EnableMapMode();
            Size aSize(aLineEnd.getX() - aLineStart.getX(), aLineEnd.getY() - aLineStart.getY());
            aSize = EditWin().PixelToLogic(aSize);
            aLineEnd = aLineStart;
            aLineEnd.Move(aSize.getWidth(), aSize.getHeight());
            EditWin().EnableMapMode(false);
        }

        if (mpAnchor)
        {
            mpAnchor->SetAllPosition( basegfx::B2DPoint( mAnchorRect.Left() , mAnchorRect.Bottom() - 5* 15),
                                      basegfx::B2DPoint( mAnchorRect.Left()-5*15 , mAnchorRect.Bottom()+5*15),
                                      basegfx::B2DPoint( mAnchorRect.Left()+5*15 , mAnchorRect.Bottom()+5*15),
                                      basegfx::B2DPoint( mAnchorRect.Left(), mAnchorRect.Bottom()+2*15),
                                      basegfx::B2DPoint( mPageBorder ,mAnchorRect.Bottom()+2*15),
                                      basegfx::B2DPoint( aLineStart.X(),aLineStart.Y()),
                                      basegfx::B2DPoint( aLineEnd.X(),aLineEnd.Y()));
        }
        else
        {
            mpAnchor = AnchorOverlayObject::CreateAnchorOverlayObject( mrView,
                                                                       mAnchorRect,
                                                                       mPageBorder,
                                                                       aLineStart,
                                                                       aLineEnd,
                                                                       mColorAnchor );
            if ( mpAnchor )
            {
                mpAnchor->setVisible(true);
                mpAnchor->SetAnchorState(AnchorState::Tri);
                if (HasChildPathFocus())
                {
                    mpAnchor->setLineSolid(true);
                }
            }
        }
    }
    else
    {
        if ( mpAnchor &&
             ( mpAnchor->getBasePosition() != basegfx::B2DPoint( mAnchorRect.Left() , mAnchorRect.Bottom()-5*15) ) )
        {
            mpAnchor->SetTriPosition( basegfx::B2DPoint( mAnchorRect.Left() , mAnchorRect.Bottom() - 5* 15),
                                      basegfx::B2DPoint( mAnchorRect.Left()-5*15 , mAnchorRect.Bottom()+5*15),
                                      basegfx::B2DPoint( mAnchorRect.Left()+5*15 , mAnchorRect.Bottom()+5*15),
                                      basegfx::B2DPoint( mAnchorRect.Left(), mAnchorRect.Bottom()+2*15),
                                      basegfx::B2DPoint( mPageBorder , mAnchorRect.Bottom()+2*15));
        }
    }

    if (bChange)
    {
        Point aStart = EditWin().PixelToLogic(GetPosPixel()+Point(0,GetSizePixel().Height()));
        Point aEnd = EditWin().PixelToLogic(GetPosPixel()+Point(GetSizePixel().Width()-1,GetSizePixel().Height()));
        mpShadow->SetPosition(basegfx::B2DPoint(aStart.X(),aStart.Y()), basegfx::B2DPoint(aEnd.X(),aEnd.Y()));
    }

    if (mrMgr.ShowNotes())
    {
        if (IsFollow() && !HasChildPathFocus())
        {
            // #i111964#
            if ( mpAnchor )
            {
                mpAnchor->SetAnchorState(AnchorState::End);
            }
        }
        else
        {
            // #i111964#
            if ( mpAnchor )
            {
                mpAnchor->SetAnchorState(AnchorState::All);
            }
            SwAnnotationWin* pWin = GetTopReplyNote();
            // #i111964#
            if ( pWin && pWin->Anchor() )
            {
                pWin->Anchor()->SetAnchorState(AnchorState::End);
            }
        }
    }


    // text range overlay
    maAnnotationTextRanges.clear();
    if ( mrSidebarItem.maLayoutInfo.mnStartNodeIdx != 0
         && mrSidebarItem.maLayoutInfo.mnStartContent != -1 )
    {
        const SwTextAnnotationField* pTextAnnotationField =
            dynamic_cast< const SwTextAnnotationField* >( mrSidebarItem.GetFormatField().GetTextField() );
        SwTextNode* pTextNode = pTextAnnotationField ? pTextAnnotationField->GetpTextNode() : nullptr;
        SwContentNode* pContentNd = nullptr;
        if (pTextNode)
        {
            SwNodes& rNds = pTextNode->GetDoc()->GetNodes();
            pContentNd = rNds[mrSidebarItem.maLayoutInfo.mnStartNodeIdx]->GetContentNode();
        }
        if (pContentNd)
        {
            SwPosition aStartPos( *pContentNd, mrSidebarItem.maLayoutInfo.mnStartContent );
            SwShellCursor* pTmpCursor = nullptr;
            const bool bTableCursorNeeded = pTextNode->FindTableBoxStartNode() != pContentNd->FindTableBoxStartNode();
            if ( bTableCursorNeeded )
            {
                SwShellTableCursor* pTableCursor = new SwShellTableCursor( mrView.GetWrtShell(), aStartPos );
                pTableCursor->SetMark();
                pTableCursor->GetMark()->nNode = *pTextNode;
                pTableCursor->GetMark()->nContent.Assign( pTextNode, pTextAnnotationField->GetStart()+1 );
                pTableCursor->NewTableSelection();
                pTmpCursor = pTableCursor;
            }
            else
            {
                SwShellCursor* pCursor = new SwShellCursor( mrView.GetWrtShell(), aStartPos );
                pCursor->SetMark();
                pCursor->GetMark()->nNode = *pTextNode;
                pCursor->GetMark()->nContent.Assign( pTextNode, pTextAnnotationField->GetStart()+1 );
                pTmpCursor = pCursor;
            }
            std::unique_ptr<SwShellCursor> pTmpCursorForAnnotationTextRange( pTmpCursor );

            // For annotation text range rectangles to be calculated correctly,
            // we need the map mode disabled
            bool bDisableMapMode = comphelper::LibreOfficeKit::isActive() && EditWin().IsMapModeEnabled();
            if (bDisableMapMode)
                EditWin().EnableMapMode(false);

            pTmpCursorForAnnotationTextRange->FillRects();

            if (bDisableMapMode)
                EditWin().EnableMapMode();

            SwRects* pRects(pTmpCursorForAnnotationTextRange.get());
            for(SwRect & rNextRect : *pRects)
            {
                const tools::Rectangle aPntRect(rNextRect.SVRect());
                maAnnotationTextRanges.emplace_back(
                    aPntRect.Left(), aPntRect.Top(),
                    aPntRect.Right() + 1, aPntRect.Bottom() + 1);
            }
        }
    }

    if (mrMgr.ShowNotes() && !maAnnotationTextRanges.empty())
    {
        if ( mpTextRangeOverlay != nullptr )
        {
            mpTextRangeOverlay->setRanges( maAnnotationTextRanges );
            if ( mpAnchor != nullptr && mpAnchor->getLineSolid() )
            {
                mpTextRangeOverlay->ShowSolidBorder();
            }
            else
            {
                mpTextRangeOverlay->HideSolidBorder();
            }
        }
        else if (!IsFollow())
        {
            // This window is not a reply, then draw its range overlay.
            mpTextRangeOverlay =
                sw::overlay::OverlayRanges::CreateOverlayRange(
                    mrView,
                    mColorAnchor,
                    maAnnotationTextRanges,
                    mpAnchor && mpAnchor->getLineSolid() );
        }
    }
    else
    {
        mpTextRangeOverlay.reset();
    }
}

void SwAnnotationWin::DoResize()
{
    long aTextHeight    =  LogicToPixel( mpOutliner->CalcTextSize()).Height();
    long aHeight        =  GetSizePixel().Height();
    unsigned long aWidth    =  GetSizePixel().Width();

    aHeight -= GetMetaHeight();
    mpMetadataAuthor->Show();
    mpMetadataDate->Show();
    mpSidebarTextControl->SetQuickHelpText(OUString());

    if (aTextHeight > aHeight)
    {   // we need vertical scrollbars and have to reduce the width
        aWidth -= GetScrollbarWidth();
        mpVScrollbar->Show();
    }
    else
    {
        mpVScrollbar->Hide();
    }

    {
        const Size aSizeOfMetadataControls( GetSizePixel().Width() - GetMetaButtonAreaWidth(),
                                            GetMetaHeight()/2 );
        mpMetadataAuthor->setPosSizePixel( 0,
                                           aHeight,
                                           aSizeOfMetadataControls.Width(),
                                           aSizeOfMetadataControls.Height() );
        mpMetadataDate->setPosSizePixel( 0,
                                         aHeight + aSizeOfMetadataControls.Height(),
                                         aSizeOfMetadataControls.Width(),
                                         aSizeOfMetadataControls.Height() );
    }

    mpOutliner->SetPaperSize( PixelToLogic( Size(aWidth,aHeight) ) ) ;
    if (!mpVScrollbar->IsVisible())
    {   // if we do not have a scrollbar anymore, we want to see the complete text
        mpOutlinerView->SetVisArea( PixelToLogic( tools::Rectangle(0,0,aWidth,aHeight) ) );
    }
    mpOutlinerView->SetOutputArea( PixelToLogic( tools::Rectangle(0,0,aWidth,aHeight) ) );

    if (!AllSettings::GetLayoutRTL())
    {
        mpSidebarTextControl->setPosSizePixel(0, 0, aWidth, aHeight);
        mpVScrollbar->setPosSizePixel( aWidth, 0, GetScrollbarWidth(), aHeight);
    }
    else
    {
        mpSidebarTextControl->setPosSizePixel( ( aTextHeight > aHeight ? GetScrollbarWidth() : 0 ), 0,
                                      aWidth, aHeight);
        mpVScrollbar->setPosSizePixel( 0, 0, GetScrollbarWidth(), aHeight);
    }

    mpVScrollbar->SetVisibleSize( PixelToLogic(Size(0,aHeight)).Height() );
    mpVScrollbar->SetPageSize( PixelToLogic(Size(0,aHeight)).Height() * 8 / 10 );
    mpVScrollbar->SetLineSize( mpOutliner->GetTextHeight() / 10 );
    SetScrollbar();
    mpVScrollbar->SetRange( Range(0, mpOutliner->GetTextHeight()));

    //calculate rects for meta- button
    const Fraction& fx( GetMapMode().GetScaleX() );
    const Fraction& fy( GetMapMode().GetScaleY() );

    const Point aPos( mpMetadataAuthor->GetPosPixel());
    mpMenuButton->setPosSizePixel( long(aPos.X()+GetSizePixel().Width()-(METABUTTON_WIDTH+10)*fx),
                                   long(aPos.Y()+5*fy),
                                   long(METABUTTON_WIDTH*fx),
                                   long(METABUTTON_HEIGHT*fy) );
}

void SwAnnotationWin::SetSizePixel( const Size& rNewSize )
{
    Window::SetSizePixel(rNewSize);

    if (mpShadow)
    {
        Point aStart = EditWin().PixelToLogic(GetPosPixel()+Point(0,GetSizePixel().Height()));
        Point aEnd = EditWin().PixelToLogic(GetPosPixel()+Point(GetSizePixel().Width()-1,GetSizePixel().Height()));
        mpShadow->SetPosition(basegfx::B2DPoint(aStart.X(),aStart.Y()), basegfx::B2DPoint(aEnd.X(),aEnd.Y()));
    }
}

void SwAnnotationWin::SetScrollbar()
{
    mpVScrollbar->SetThumbPos(mpOutlinerView->GetVisArea().Top());
}

void SwAnnotationWin::ResizeIfNecessary(long aOldHeight, long aNewHeight)
{
    if (aOldHeight != aNewHeight)
    {
        //check for lower border or next note
        long aBorder = mrMgr.GetNextBorder();
        if (aBorder != -1)
        {
            if (aNewHeight > GetMinimumSizeWithoutMeta())
            {
                long aNewLowerValue = GetPosPixel().Y() + aNewHeight + GetMetaHeight();
                if (aNewLowerValue < aBorder)
                    SetSizePixel(Size(GetSizePixel().Width(),aNewHeight+GetMetaHeight()));
                else
                    SetSizePixel(Size(GetSizePixel().Width(),aBorder - GetPosPixel().Y()));
                DoResize();
                Invalidate();
            }
            else
            {
                if (GetSizePixel().Height() != GetMinimumSizeWithoutMeta() + GetMetaHeight())
                    SetSizePixel(Size(GetSizePixel().Width(),GetMinimumSizeWithoutMeta() + GetMetaHeight()));
                DoResize();
                Invalidate();
            }
        }
        else
        {
            DoResize();
            Invalidate();
        }
    }
    else
    {
        SetScrollbar();
    }
}

void SwAnnotationWin::SetColor(Color aColorDark,Color aColorLight, Color aColorAnchor)
{
    mColorDark =  aColorDark;
    mColorLight = aColorLight;
    mColorAnchor = aColorAnchor;

    if ( !Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
    {
        {
            mpMetadataAuthor->SetControlBackground(mColorDark);
            AllSettings aSettings = mpMetadataAuthor->GetSettings();
            StyleSettings aStyleSettings = aSettings.GetStyleSettings();
            aStyleSettings.SetFieldTextColor(aColorAnchor);
            aSettings.SetStyleSettings(aStyleSettings);
            mpMetadataAuthor->SetSettings(aSettings);
        }

        {
            mpMetadataDate->SetControlBackground(mColorDark);
            AllSettings aSettings = mpMetadataDate->GetSettings();
            StyleSettings aStyleSettings = aSettings.GetStyleSettings();
            aStyleSettings.SetFieldTextColor(aColorAnchor);
            aSettings.SetStyleSettings(aStyleSettings);
            mpMetadataDate->SetSettings(aSettings);
        }

        AllSettings aSettings2 = mpVScrollbar->GetSettings();
        StyleSettings aStyleSettings2 = aSettings2.GetStyleSettings();
        aStyleSettings2.SetButtonTextColor(Color(0,0,0));
        aStyleSettings2.SetCheckedColor(mColorLight); // background
        aStyleSettings2.SetShadowColor(mColorAnchor);
        aStyleSettings2.SetFaceColor(mColorDark);
        aSettings2.SetStyleSettings(aStyleSettings2);
        mpVScrollbar->SetSettings(aSettings2);
    }
}

void SwAnnotationWin::SetSidebarPosition(sw::sidebarwindows::SidebarPosition eSidebarPosition)
{
    meSidebarPosition = eSidebarPosition;
}

void SwAnnotationWin::SetReadonly(bool bSet)
{
    mbReadonly = bSet;
    GetOutlinerView()->SetReadOnly(bSet);
}

void SwAnnotationWin::SetLanguage(const SvxLanguageItem& rNewItem)
{
    Link<LinkParamNone*,void> aLink = mpOutliner->GetModifyHdl();
    mpOutliner->SetModifyHdl( Link<LinkParamNone*,void>() );
    ESelection aOld = GetOutlinerView()->GetSelection();

    ESelection aNewSelection( 0, 0, mpOutliner->GetParagraphCount()-1, EE_TEXTPOS_ALL );
    GetOutlinerView()->SetSelection( aNewSelection );
    SfxItemSet aEditAttr(GetOutlinerView()->GetAttribs());
    aEditAttr.Put(rNewItem);
    GetOutlinerView()->SetAttribs( aEditAttr );

    GetOutlinerView()->SetSelection(aOld);
    mpOutliner->SetModifyHdl( aLink );

    const SwViewOption* pVOpt = mrView.GetWrtShellPtr()->GetViewOptions();
    EEControlBits nCntrl = mpOutliner->GetControlWord();
    // turn off
    nCntrl &= ~EEControlBits::ONLINESPELLING;
    mpOutliner->SetControlWord(nCntrl);

    //turn back on
    if (pVOpt->IsOnlineSpell())
        nCntrl |= EEControlBits::ONLINESPELLING;
    else
        nCntrl &= ~EEControlBits::ONLINESPELLING;
    mpOutliner->SetControlWord(nCntrl);

    mpOutliner->CompleteOnlineSpelling();
    Invalidate();
}

void SwAnnotationWin::GetFocus()
{
    if (mpSidebarTextControl)
        mpSidebarTextControl->GrabFocus();
}

void SwAnnotationWin::LoseFocus()
{
}

void SwAnnotationWin::ShowNote()
{
    SetPosAndSize();
    if (!IsVisible())
        Window::Show();
    if (mpShadow && !mpShadow->isVisible())
        mpShadow->setVisible(true);
    if (mpAnchor && !mpAnchor->isVisible())
        mpAnchor->setVisible(true);

    // Invalidate.
    InvalidateControl();
}

void SwAnnotationWin::HideNote()
{
    if (IsVisible())
        Window::Hide();
    if (mpAnchor)
    {
        if (mrMgr.IsShowAnchor())
            mpAnchor->SetAnchorState(AnchorState::Tri);
        else
            mpAnchor->setVisible(false);
    }
    if (mpShadow && mpShadow->isVisible())
        mpShadow->setVisible(false);
}

void SwAnnotationWin::InvalidateControl()
{
    // Invalidate.
    mpSidebarTextControl->Push(PushFlags::MAPMODE);
    lcl_translateTwips(EditWin(), *mpSidebarTextControl, nullptr);
    mpSidebarTextControl->Invalidate();
    mpSidebarTextControl->Pop();
}

void SwAnnotationWin::ActivatePostIt()
{
    mrMgr.AssureStdModeAtShell();

    mpOutliner->ClearModifyFlag();
    mpOutliner->GetUndoManager().Clear();

    CheckMetaText();
    SetViewState(ViewState::EDIT);
    GetOutlinerView()->ShowCursor();

    mpOutlinerView->GetEditView().SetInsertMode(mrView.GetWrtShellPtr()->IsInsMode());

    if ( !Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
        GetOutlinerView()->SetBackgroundColor(mColorDark);

    //tdf#119130 only have the active postit as a dialog control in which pressing
    //ctrl+tab cycles between text and button so we don't waste time searching
    //thousands of SwAnnotationWins
    SetStyle(GetStyle() | WB_DIALOGCONTROL);
}

void SwAnnotationWin::DeactivatePostIt()
{
    //tdf#119130 only have the active postit as a dialog control in which pressing
    //ctrl+tab cycles between text and button so we don't waste time searching
    //thousands of SwAnnotationWins
    SetStyle(GetStyle() & ~WB_DIALOGCONTROL);

    // remove selection, #i87073#
    if (GetOutlinerView()->GetEditView().HasSelection())
    {
        ESelection aSelection = GetOutlinerView()->GetEditView().GetSelection();
        aSelection.nEndPara = aSelection.nStartPara;
        aSelection.nEndPos = aSelection.nStartPos;
        GetOutlinerView()->GetEditView().SetSelection(aSelection);
    }

    mpOutliner->CompleteOnlineSpelling();

    SetViewState(ViewState::NORMAL);
    // Make sure this view doesn't emit LOK callbacks during the update, as the
    // sidebar window's SidebarTextControl doesn't have a valid twip offset
    // (map mode origin) during that operation.
    bool bTiledPainting = comphelper::LibreOfficeKit::isTiledPainting();
    comphelper::LibreOfficeKit::setTiledPainting(true);
    // write the visible text back into the SwField
    UpdateData();
    comphelper::LibreOfficeKit::setTiledPainting(bTiledPainting);

    if ( !Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
        GetOutlinerView()->SetBackgroundColor(COL_TRANSPARENT);

    if ( !IsProtected() && mpOutliner->GetEditEngine().GetText().isEmpty() )
    {
        mnEventId = Application::PostUserEvent( LINK( this, SwAnnotationWin, DeleteHdl), nullptr, true );
    }
}

void SwAnnotationWin::ToggleInsMode()
{
    if (!mrView.GetWrtShell().IsRedlineOn())
    {
        //change outliner
        mpOutlinerView->GetEditView().SetInsertMode(!mpOutlinerView->GetEditView().IsInsertMode());
        //change document
        mrView.GetWrtShell().ToggleInsMode();
        //update statusbar
        SfxBindings &rBnd = mrView.GetViewFrame()->GetBindings();
        rBnd.Invalidate(SID_ATTR_INSERT);
        rBnd.Update(SID_ATTR_INSERT);
    }
}

void SwAnnotationWin::ExecuteCommand(sal_uInt16 nSlot)
{
    mrMgr.AssureStdModeAtShell();

    switch (nSlot)
    {
        case FN_POSTIT:
        case FN_REPLY:
        {
            // if this note is empty, it will be deleted once losing the focus, so no reply, but only a new note
            // will be created
            if (!mpOutliner->GetEditEngine().GetText().isEmpty())
            {
                OutlinerParaObject* pPara = new OutlinerParaObject(*GetOutlinerView()->GetEditView().CreateTextObject());
                mrMgr.RegisterAnswer(pPara);
            }
            if (mrMgr.HasActiveSidebarWin())
                mrMgr.SetActiveSidebarWin(nullptr);
            SwitchToFieldPos();
            mrView.GetViewFrame()->GetDispatcher()->Execute(FN_POSTIT);
            break;
        }
        case FN_DELETE_COMMENT:

            //Delete(); // do not kill the parent of our open popup menu
            mnEventId = Application::PostUserEvent( LINK( this, SwAnnotationWin, DeleteHdl), nullptr, true );
            break;
        case FN_FORMAT_ALL_NOTES:
        case FN_DELETE_ALL_NOTES:
        case FN_HIDE_ALL_NOTES:
            // not possible as slot as this would require that "this" is the active postit
            mrView.GetViewFrame()->GetBindings().Execute( nSlot, nullptr, SfxCallMode::ASYNCHRON );
            break;
        case FN_DELETE_NOTE_AUTHOR:
        case FN_HIDE_NOTE_AUTHOR:
        {
            // not possible as slot as this would require that "this" is the active postit
            SfxStringItem aItem( nSlot, GetAuthor() );
            const SfxPoolItem* aItems[2];
            aItems[0] = &aItem;
            aItems[1] = nullptr;
            mrView.GetViewFrame()->GetBindings().Execute( nSlot, aItems, SfxCallMode::ASYNCHRON );
        }
            break;
        default:
            mrView.GetViewFrame()->GetBindings().Execute( nSlot );
            break;
    }
}

SwEditWin&  SwAnnotationWin::EditWin()
{
    return mrView.GetEditWin();
}

long SwAnnotationWin::GetPostItTextHeight()
{
    return mpOutliner ? LogicToPixel(mpOutliner->CalcTextSize()).Height() : 0;
}

void SwAnnotationWin::SwitchToPostIt(sal_uInt16 aDirection)
{
    SwAnnotationWin* pPostIt = mrMgr.GetNextPostIt(aDirection, this);
    if (pPostIt)
        pPostIt->GrabFocus();
}

IMPL_LINK( SwAnnotationWin, WindowEventListener, VclWindowEvent&, rEvent, void )
{
    if ( rEvent.GetId() == VclEventId::WindowMouseMove )
    {
        MouseEvent* pMouseEvt = static_cast<MouseEvent*>(rEvent.GetData());
        if ( pMouseEvt->IsEnterWindow() )
        {
            mbMouseOver = true;
            if ( !HasFocus() )
            {
                SetViewState(ViewState::VIEW);
                Invalidate();
            }
        }
        else if ( pMouseEvt->IsLeaveWindow())
        {
            mbMouseOver = false;
            if ( !HasFocus() )
            {
                SetViewState(ViewState::NORMAL);
                Invalidate();
            }
        }
    }
    else if ( rEvent.GetId() == VclEventId::WindowActivate &&
              rEvent.GetWindow() == mpSidebarTextControl )
    {
        const bool bLockView = mrView.GetWrtShell().IsViewLocked();
        mrView.GetWrtShell().LockView( true );
        mrMgr.SetActiveSidebarWin( this );
        mrView.GetWrtShell().LockView( bLockView );
        mrMgr.MakeVisible( this );
    }
}

IMPL_LINK(SwAnnotationWin, ScrollHdl, ScrollBar*, pScroll, void)
{
    long nDiff = GetOutlinerView()->GetEditView().GetVisArea().Top() - pScroll->GetThumbPos();
    GetOutlinerView()->Scroll( 0, nDiff );
}

IMPL_LINK_NOARG(SwAnnotationWin, ModifyHdl, LinkParamNone*, void)
{
    mrView.GetDocShell()->SetModified();
}

IMPL_LINK_NOARG(SwAnnotationWin, DeleteHdl, void*, void)
{
    mnEventId = nullptr;
    Delete();
}

void SwAnnotationWin::ResetAttributes()
{
    mpOutlinerView->RemoveAttribsKeepLanguages(true);
    mpOutliner->RemoveFields();
    mpOutlinerView->SetAttribs(DefaultItem());
}

sal_Int32 SwAnnotationWin::GetScrollbarWidth()
{
    return mrView.GetWrtShell().GetViewOptions()->GetZoom() / 10;
}

sal_Int32 SwAnnotationWin::GetMetaButtonAreaWidth()
{
    const Fraction& f( GetMapMode().GetScaleX() );
    return long(METABUTTON_AREA_WIDTH * f);
}

sal_Int32 SwAnnotationWin::GetMetaHeight()
{
    const Fraction& f(mrView.GetWrtShellPtr()->GetOut()->GetMapMode().GetScaleY());
    return long(POSTIT_META_HEIGHT * f);
}

sal_Int32 SwAnnotationWin::GetMinimumSizeWithMeta()
{
    return mrMgr.GetMinimumSizeWithMeta();
}

sal_Int32 SwAnnotationWin::GetMinimumSizeWithoutMeta()
{
    const Fraction& f(mrView.GetWrtShellPtr()->GetOut()->GetMapMode().GetScaleY());
    return long(POSTIT_MINIMUMSIZE_WITHOUT_META * f);
}

void SwAnnotationWin::SetSpellChecking()
{
    const SwViewOption* pVOpt = mrView.GetWrtShellPtr()->GetViewOptions();
    EEControlBits nCntrl = mpOutliner->GetControlWord();
    if (pVOpt->IsOnlineSpell())
        nCntrl |= EEControlBits::ONLINESPELLING;
    else
        nCntrl &= ~EEControlBits::ONLINESPELLING;
    mpOutliner->SetControlWord(nCntrl);

    mpOutliner->CompleteOnlineSpelling();
    Invalidate();
}

void SwAnnotationWin::SetViewState(ViewState bViewState)
{
    switch (bViewState)
    {
        case ViewState::EDIT:
        {
            if (mpAnchor)
            {
                mpAnchor->SetAnchorState(AnchorState::All);
                SwAnnotationWin* pWin = GetTopReplyNote();
                // #i111964#
                if ( pWin && pWin->Anchor() )
                {
                    pWin->Anchor()->SetAnchorState(AnchorState::End);
                }
                mpAnchor->setLineSolid(true);
                if ( mpTextRangeOverlay != nullptr )
                {
                    mpTextRangeOverlay->ShowSolidBorder();
                }
            }
            if (mpShadow)
                mpShadow->SetShadowState(SS_EDIT);
            break;
        }
        case ViewState::VIEW:
        {
            if (mpAnchor)
            {
                mpAnchor->setLineSolid(true);
                if ( mpTextRangeOverlay != nullptr )
                {
                    mpTextRangeOverlay->ShowSolidBorder();
                }
            }
            if (mpShadow)
                mpShadow->SetShadowState(SS_VIEW);
            break;
        }
        case ViewState::NORMAL:
        {
            if (mpAnchor)
            {
                if (IsFollow())
                {
                    // if there is no visible parent note, we want to see the complete anchor ??
                    //if (IsAnyStackParentVisible())
                    mpAnchor->SetAnchorState(AnchorState::End);
                    SwAnnotationWin* pTopWinSelf = GetTopReplyNote();
                    SwAnnotationWin* pTopWinActive = mrMgr.HasActiveSidebarWin()
                                                  ? mrMgr.GetActiveSidebarWin()->GetTopReplyNote()
                                                  : nullptr;
                    // #i111964#
                    if ( pTopWinSelf && ( pTopWinSelf != pTopWinActive ) &&
                         pTopWinSelf->Anchor() )
                    {
                        if ( pTopWinSelf != mrMgr.GetActiveSidebarWin() )
                        {
                            pTopWinSelf->Anchor()->setLineSolid(false);
                            if ( pTopWinSelf->TextRange() != nullptr )
                            {
                                pTopWinSelf->TextRange()->HideSolidBorder();
                            }
                        }
                        pTopWinSelf->Anchor()->SetAnchorState(AnchorState::All);
                    }
                }
                mpAnchor->setLineSolid(false);
                if ( mpTextRangeOverlay != nullptr )
                {
                    mpTextRangeOverlay->HideSolidBorder();
                }
            }
            if ( mpShadow )
            {
                mpShadow->SetShadowState(SS_NORMAL);
            }
            break;
        }
    }
}

SwAnnotationWin* SwAnnotationWin::GetTopReplyNote()
{
    SwAnnotationWin* pTopNote = nullptr;
    SwAnnotationWin* pSidebarWin = IsFollow() ? mrMgr.GetNextPostIt(KEY_PAGEUP, this) : nullptr;
    while (pSidebarWin)
    {
        pTopNote = pSidebarWin;
        pSidebarWin = pSidebarWin->IsFollow() ? mrMgr.GetNextPostIt(KEY_PAGEUP, pSidebarWin) : nullptr;
    }
    return pTopNote;
}

void SwAnnotationWin::SwitchToFieldPos()
{
    if ( mrMgr.GetActiveSidebarWin() == this )
            mrMgr.SetActiveSidebarWin(nullptr);
    GotoPos();
    sal_uInt32 aCount = MoveCaret();
    if (aCount)
        mrView.GetDocShell()->GetWrtShell()->SwCursorShell::Right(aCount, 0);
    GrabFocusToDocument();
}

void SwAnnotationWin::SetChangeTracking( const SwPostItHelper::SwLayoutStatus aLayoutStatus,
                                      const Color& aChangeColor )
{
    if ( (mLayoutStatus != aLayoutStatus) ||
         (mChangeColor != aChangeColor) )
    {
        mLayoutStatus = aLayoutStatus;
        mChangeColor = aChangeColor;
        Invalidate();
    }
}

bool SwAnnotationWin::HasScrollbar() const
{
    return mpVScrollbar != nullptr;
}

bool SwAnnotationWin::IsScrollbarVisible() const
{
    return HasScrollbar() && mpVScrollbar->IsVisible();
}

void SwAnnotationWin::ChangeSidebarItem( SwSidebarItem const & rSidebarItem )
{
    const bool bAnchorChanged = mpAnchorFrame != rSidebarItem.maLayoutInfo.mpAnchorFrame;
    if ( bAnchorChanged )
    {
        mrMgr.DisconnectSidebarWinFromFrame( *mpAnchorFrame, *this );
    }

    mrSidebarItem = rSidebarItem;
    mpAnchorFrame = mrSidebarItem.maLayoutInfo.mpAnchorFrame;

    if ( GetWindowPeer() )
    {
        SidebarWinAccessible* pAcc =
                        static_cast<SidebarWinAccessible*>( GetWindowPeer() );
        OSL_ENSURE( dynamic_cast<SidebarWinAccessible*>( GetWindowPeer() ),
                "<SwAnnotationWin::ChangeSidebarItem(..)> - unexpected type of window peer -> crash possible!" );
        pAcc->ChangeSidebarItem( mrSidebarItem );
    }

    if ( bAnchorChanged )
    {
        mrMgr.ConnectSidebarWinToFrame( *(mrSidebarItem.maLayoutInfo.mpAnchorFrame),
                                      mrSidebarItem.GetFormatField(),
                                      *this );
    }
}

css::uno::Reference< css::accessibility::XAccessible > SwAnnotationWin::CreateAccessible()
{
    SidebarWinAccessible* pAcc( new SidebarWinAccessible( *this,
                                                          mrView.GetWrtShell(),
                                                          mrSidebarItem ) );
    css::uno::Reference< css::awt::XWindowPeer > xWinPeer( pAcc );
    SetWindowPeer( xWinPeer, pAcc );

    css::uno::Reference< css::accessibility::XAccessible > xAcc( xWinPeer, css::uno::UNO_QUERY );
    return xAcc;
}

} } // eof of namespace sw::sidebarwindows

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
