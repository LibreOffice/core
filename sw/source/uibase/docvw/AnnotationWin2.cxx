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

#include "SidebarWinAcc.hxx"
#include <PostItMgr.hxx>
#include <AnnotationWin.hxx>
#include <IDocumentUndoRedo.hxx>
#include <basegfx/range/b2drange.hxx>
#include "SidebarTxtControl.hxx"
#include "SidebarScrollBar.hxx"
#include "AnchorOverlayObject.hxx"
#include "ShadowOverlayObject.hxx"
#include "OverlayRanges.hxx"

#include <strings.hrc>

#include <viewopt.hxx>
#include <cmdid.h>

#include <editeng/fhgtitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/editview.hxx>
#include <editeng/outliner.hxx>
#include <editeng/editeng.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/outlobj.hxx>

#include <svl/undo.hxx>
#include <svl/stritem.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>

#include <vcl/fixed.hxx>
#include <vcl/event.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/ptrstyle.hxx>
#include <vcl/uitest/logger.hxx>
#include <vcl/uitest/eventdescription.hxx>

#include <edtwin.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <doc.hxx>
#include <swmodule.hxx>

#include <SwRewriter.hxx>
#include <txtannotationfld.hxx>
#include <ndtxt.hxx>

#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/syslocale.hxx>
#include <memory>
#include <comphelper/lok.hxx>

using namespace sw::sidebarwindows;

namespace
{

void collectUIInformation( const OUString& aevent , const OUString& aID )
{
    EventDescription aDescription;
    aDescription.aID =  aID;
    aDescription.aParameters = {{"" ,  ""}};
    aDescription.aAction = aevent;
    aDescription.aParent = "MainWindow";
    aDescription.aKeyWord = "SwEditWinUIObject";
    UITestLogger::getInstance().logEvent(aDescription);
}

#if 0
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
#endif

}

namespace sw::annotation {

#define METABUTTON_AREA_WIDTH   30
#define POSTIT_META_FIELD_HEIGHT  sal_Int32(15)
#define POSTIT_MINIMUMSIZE_WITHOUT_META     50


void SwAnnotationWin::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    InterimItemWindow::Paint(rRenderContext, rRect);

    if (!mxMetadataAuthor->get_visible())
        return;

#if 0

    //draw left over space
    if (Application::GetSettings().GetStyleSettings().GetHighContrastMode())
    {
        rRenderContext.SetFillColor(COL_BLACK);
    }
    else
    {
        rRenderContext.SetFillColor(mColorDark);
    }

    sal_uInt32 boxHeight = mxMetadataAuthor->GetSizePixel().Height() + mxMetadataDate->GetSizePixel().Height();
    boxHeight += IsResolved() ? mxMetadataResolved->GetSizePixel().Height() : 0;

    rRenderContext.SetLineColor();
    tools::Rectangle aRectangle(Point(mxMetadataAuthor->GetPosPixel().X() + mxMetadataAuthor->GetSizePixel().Width(),
                               mxMetadataAuthor->GetPosPixel().Y()),
                         Size(GetMetaButtonAreaWidth(), boxHeight));

    if (comphelper::LibreOfficeKit::isActive())
        aRectangle = rRect;
    else
        aRectangle = PixelToLogic(aRectangle);
    rRenderContext.DrawRect(aRectangle);
#endif
}

void SwAnnotationWin::PaintTile(vcl::RenderContext& /*rRenderContext*/, const tools::Rectangle& /*rRect*/)
{
#if 0
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
#endif
}

bool SwAnnotationWin::IsHitWindow(const Point& rPointLogic)
{
    tools::Rectangle aRectangleLogic(EditWin().PixelToLogic(GetPosPixel()), EditWin().PixelToLogic(GetSizePixel()));
    return aRectangleLogic.IsInside(rPointLogic);
}

void SwAnnotationWin::SetCursorLogicPosition(const Point& rPosition, bool bPoint, bool bClearMark)
{
    mxSidebarTextControl->SetCursorLogicPosition(rPosition, bPoint, bClearMark);
}

void SwAnnotationWin::Draw(OutputDevice* /*pDev*/, const Point& /*rPt*/, DrawFlags /*nInFlags*/)
{
#if 0
    Size aSz = PixelToLogic(GetSizePixel());

    if (mxMetadataAuthor->get_visible() )
    {
        pDev->SetFillColor(mColorDark);
        pDev->SetLineColor();
        pDev->DrawRect( tools::Rectangle( rPt, aSz ) );
    }

    if (mxMetadataAuthor->get_visible())
    {
#if 0
        vcl::Font aOrigFont(mxMetadataAuthor->GetControlFont());
        Point aPos(PixelToLogic(mxMetadataAuthor->GetPosPixel()));
        aPos += rPt;
        vcl::Font aFont( mxMetadataAuthor->GetSettings().GetStyleSettings().GetLabelFont() );
        mxMetadataAuthor->SetControlFont( aFont );
        mxMetadataAuthor->Draw(pDev, aPos, nInFlags);
        mxMetadataAuthor->SetControlFont( aOrigFont );
#endif
    }

    if (mxMetadataDate->get_visible())
    {
        vcl::Font aOrigFont(mxMetadataDate->GetControlFont());
        Point aPos(PixelToLogic(mxMetadataDate->GetPosPixel()));
        aPos += rPt;
        vcl::Font aFont( mpMetadataDate->GetSettings().GetStyleSettings().GetLabelFont() );
        mxMetadataDate->SetControlFont( aFont );
        mxMetadataDate->Draw(pDev, aPos, nInFlags);
        mxMetadataDate->SetControlFont( aOrigFont );
    }

    if (mxMetadataResolved->IsVisible())
    {
        vcl::Font aOrigFont(mxMetadataResolved->GetControlFont());
        Point aPos(PixelToLogic(mxMetadataResolved->GetPosPixel()));
        aPos += rPt;
        vcl::Font aFont( mpMetadataResolved->GetSettings().GetStyleSettings().GetLabelFont() );
        mpMetadataResolved->SetControlFont( aFont );
        mpMetadataResolved->Draw(pDev, aPos, nInFlags);
        mpMetadataResolved->SetControlFont( aOrigFont );
    }

    mxSidebarTextControl->Draw(pDev, rPt, nInFlags);

    const drawinglayer::geometry::ViewInformation2D aNewViewInfos;
    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor(
        drawinglayer::processor2d::createBaseProcessor2DFromOutputDevice(
            *pDev, aNewViewInfos ));

    if (mpAnchor)
        pProcessor->process(mpAnchor->getOverlayObjectPrimitive2DSequence());
    if (mpTextRangeOverlay)
        pProcessor->process(mpTextRangeOverlay->getOverlayObjectPrimitive2DSequence());
    pProcessor.reset();

    if (mxVScrollbar->get_vpolicy() == VclPolicyType::NEVER)
        return;

    // if there is a scrollbar shown, draw "..." to indicate the comment isn't
    // completely shown
    vcl::Font aOrigFont(mxMetadataDate->GetControlFont());
    Color aOrigBg( mxMetadataDate->GetControlBackground() );
    OUString sOrigText(mxMetadataDate->GetText());

    Point aPos(PixelToLogic(mxMenuButton->GetPosPixel()));
    aPos += rPt;

    vcl::Font aFont( mpMetadataDate->GetSettings().GetStyleSettings().GetLabelFont() );
    mxMetadataDate->SetControlFont( aFont );
    mxMetadataDate->SetControlBackground( Color(0xFFFFFF) );
    mxMetadataDate->SetText("...");
    Size aOrigSize = mpMetadataDate->GetSizePixel();
    mxMetadataDate->SetSizePixel(mpMenuButton->GetSizePixel());
    mxMetadataDate->Draw(pDev, aPos, nInFlags);
    mxMetadataDate->SetSizePixel(aOrigSize);

    mxMetadataDate->SetText(sOrigText);
    mxMetadataDate->SetControlFont( aOrigFont );
    mxMetadataDate->SetControlBackground( aOrigBg );
#endif
}

#if 0
void SwAnnotationWin::KeyInput(const KeyEvent& rKeyEvent)
{
    if (mxSidebarTextControl)
    {
        mxSidebarTextControl->Push(PushFlags::MAPMODE);
        lcl_translateTwips(EditWin(), *mxSxidebarTextControl, nullptr);

        mxSidebarTextControl->KeyInput(rKeyEvent);

        mxSidebarTextControl->Pop();
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
#endif

void SwAnnotationWin::SetPosSizePixelRect(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight,
                                       const SwRect& aAnchorRect, const tools::Long aPageBorder)
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

void SwAnnotationWin::TranslateTopPosition(const tools::Long aAmount)
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

    // window controls for author and date
    mxMetadataAuthor = m_xBuilder->weld_label("author");
    mxMetadataAuthor->set_accessible_name( SwResId( STR_ACCESS_ANNOTATION_AUTHOR_NAME ) );
    mxMetadataAuthor->set_direction(AllSettings::GetLayoutRTL());
    mxMetadataAuthor->connect_mouse_move(LINK(this, SwAnnotationWin, MouseMoveHdl));

    maLabelFont = Application::GetSettings().GetStyleSettings().GetLabelFont();
    maLabelFont.SetFontHeight(8);

    // we should leave this setting alone, but for this we need a better layout algo
    // with variable meta size height
    mxMetadataAuthor->set_font(maLabelFont);

    mxMetadataDate = m_xBuilder->weld_label("date");
    mxMetadataDate->set_accessible_name( SwResId( STR_ACCESS_ANNOTATION_DATE_NAME ) );
    mxMetadataDate->set_direction(AllSettings::GetLayoutRTL());
    mxMetadataDate->connect_mouse_move(LINK(this, SwAnnotationWin, MouseMoveHdl));

    // we should leave this setting alone, but for this we need a better layout algo
    // with variable meta size height
    mxMetadataDate->set_font(maLabelFont);

    mxMetadataResolved = m_xBuilder->weld_label("resolved");
    mxMetadataResolved->set_accessible_name( SwResId( STR_ACCESS_ANNOTATION_RESOLVED_NAME ) );
    mxMetadataResolved->set_direction(AllSettings::GetLayoutRTL());
    mxMetadataResolved->connect_mouse_move(LINK(this, SwAnnotationWin, MouseMoveHdl));

    // we should leave this setting alone, but for this we need a better layout algo
    // with variable meta size height
    mxMetadataResolved->set_font(maLabelFont);
    mxMetadataResolved->set_label(SwResId(STR_ACCESS_ANNOTATION_RESOLVED_NAME));

    SwDocShell* aShell = mrView.GetDocShell();
    mpOutliner.reset(new Outliner(&aShell->GetPool(),OutlinerMode::TextObject));
    aShell->GetDoc()->SetCalcFieldValueHdl( mpOutliner.get() );
    mpOutliner->SetUpdateMode( true );
    Rescale();

    mpOutlinerView.reset(new OutlinerView(mpOutliner.get(), nullptr));
    mpOutliner->InsertView(mpOutlinerView.get());

    //create Scrollbars
    mxVScrollbar = m_xBuilder->weld_scrolled_window("scrolledwindow", true);

    // actual window which holds the user text
    mxSidebarTextControl.reset(new SidebarTextControl(*this, mrView, mrMgr));
    mxSidebarTextControlWin.reset(new weld::CustomWeld(*m_xBuilder, "editview", *mxSidebarTextControl));
    mxSidebarTextControl->SetPointer(PointerStyle::Text);

    mpOutlinerView->SetBackgroundColor(COL_TRANSPARENT);
    mpOutlinerView->SetOutputArea( PixelToLogic( tools::Rectangle(0,0,1,1) ) );

    mpOutlinerView->SetAttribs(DefaultItem());

//TODO    mxVScrollbar->EnableRTL( false );
    mxVScrollbar->connect_vadjustment_changed(LINK(this, SwAnnotationWin, ScrollHdl));
//TODO    mxVScrollbar->EnableDrag();
    mxVScrollbar->connect_mouse_move(LINK(this, SwAnnotationWin, MouseMoveHdl));

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

    mxMenuButton = m_xBuilder->weld_menu_button("menubutton");

    // expand %1 "Author"
    OUString aText = mxMenuButton->get_item_label("deleteby");
    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1, GetAuthor());
    aText = aRewriter.Apply(aText);
    mxMenuButton->set_item_label("deleteby", aText);

    mxMenuButton->set_accessible_name(SwResId(STR_ACCESS_ANNOTATION_BUTTON_NAME));
    mxMenuButton->set_accessible_description(SwResId(STR_ACCESS_ANNOTATION_BUTTON_DESC));
    mxMenuButton->set_tooltip_text(SwResId(STR_ACCESS_ANNOTATION_BUTTON_DESC));

    mxMenuButton->connect_toggled(LINK(this, SwAnnotationWin, ToggleHdl));
    mxMenuButton->connect_selected(LINK(this, SwAnnotationWin, SelectHdl));
    mxMenuButton->connect_key_press(LINK(this, SwAnnotationWin, KeyInputHdl));
    mxMenuButton->connect_mouse_move(LINK(this, SwAnnotationWin, MouseMoveHdl));

    SetLanguage(GetLanguage());
    GetOutlinerView()->StartSpeller();
    SetPostItText();
    mpOutliner->CompleteOnlineSpelling();

    mxSidebarTextControl->Show();
    mxMetadataAuthor->show();
    mxMetadataDate->show();
    mxMetadataResolved->set_visible(IsResolved());
    mxVScrollbar->set_vpolicy(VclPolicyType::ALWAYS);
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
        sMeta = OUString::Concat(sMeta.subView(0, 20)) + "...";
    }
    if ( mxMetadataAuthor->get_label() != sMeta )
    {
        mxMetadataAuthor->set_label(sMeta);
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
    if ( mxMetadataDate->get_label() != sMeta )
    {
        mxMetadataDate->set_label(sMeta);
    }

    std::size_t aIndex = SW_MOD()->InsertRedlineAuthor(GetAuthor());
    SetColor( SwPostItMgr::GetColorDark(aIndex),
              SwPostItMgr::GetColorLight(aIndex),
              SwPostItMgr::GetColorAnchor(aIndex));
}

void SwAnnotationWin::Rescale()
{
    // On Android, this method leads to invoke ImpEditEngine::UpdateViews
    // which hides the text cursor. Moreover it causes sudden document scroll
    // when modifying a commented text. Not clear the root cause,
    // anyway skipping this method fixes the problem, and there should be
    // no side effect, since the client has disabled annotations rendering.
    if (comphelper::LibreOfficeKit::isActive() && !comphelper::LibreOfficeKit::isTiledAnnotations())
        return;

    MapMode aMode = GetParent()->GetMapMode();
    aMode.SetOrigin( Point() );
    mpOutliner->SetRefMapMode( aMode );
    SetMapMode( aMode );
//TODO    mxSidebarTextControl->SetMapMode( aMode );
    const Fraction& rFraction = mrView.GetWrtShellPtr()->GetOut()->GetMapMode().GetScaleY();

    vcl::Font aFont = maLabelFont;
    sal_Int32 nHeight = tools::Long(aFont.GetFontHeight() * rFraction);
    aFont.SetFontHeight( nHeight );

#if 0
    if ( mxMetadataAuthor )
        mxMetadataAuthor->set_font(aFont);
    if ( mxMetadataDate )
        mxMetadataDate->set_font(aFont);
    if ( mxMetadataResolved )
        mxMetadataResolved->set_font(aFont);
#endif
}

void SwAnnotationWin::SetPosAndSize()
{
    bool bChange = false;

    if (GetSizePixel() != mPosSize.GetSize())
    {
        bChange = true;
        SetSizePixel(mPosSize.GetSize());

#if 0
        if (comphelper::LibreOfficeKit::isActive())
        {
            // Position is not yet set at VCL level, but the map mode should
            // contain the right origin to emit the correct cursor position.
            mxSidebarTextControl->Push(PushFlags::MAPMODE);
            Point aOffset(mPosSize.Left(), mPosSize.Top());
            aOffset = PixelToLogic(aOffset);
            MapMode aMapMode(mxSidebarTextControl->GetMapMode());
            aMapMode.SetOrigin(aOffset);
            mxSidebarTextControl->SetMapMode(aMapMode);
            mxSidebarTextControl->EnableMapMode(false);
        }
#endif

        DoResize();

#if 0
        if (comphelper::LibreOfficeKit::isActive())
            mxSidebarTextControl->Pop();
#endif
    }

    if (GetPosPixel().X() != mPosSize.Left() || (std::abs(GetPosPixel().Y() - mPosSize.Top()) > 5) )
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

    if (mpShadow && bChange)
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
            if ( pWin != this && pWin->Anchor() )
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
            SwNodes& rNds = pTextNode->GetDoc().GetNodes();
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

            if (mrSidebarItem.maLayoutInfo.mPositionFromCommentAnchor)
                pTmpCursorForAnnotationTextRange->FillRects();

            if (bDisableMapMode)
                EditWin().EnableMapMode();

            SwRects* pRects(pTmpCursorForAnnotationTextRange.get());
            for(const SwRect & rNextRect : *pRects)
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
    tools::Long aTextHeight    =  LogicToPixel( mpOutliner->CalcTextSize()).Height();
    tools::Long aHeight        =  GetSizePixel().Height();
    tools::ULong aWidth    =  GetSizePixel().Width();

    aHeight -= GetMetaHeight();
    mxMetadataAuthor->show();
    if(IsResolved()) { mxMetadataResolved->show(); }
    mxMetadataDate->show();
//TODO    mxSidebarTextControl->SetQuickHelpText(OUString());
#if 0
    unsigned int numFields = GetNumFields();
#endif
    if (aTextHeight > aHeight)
    {
        // we need vertical scrollbars and have to reduce the width
        aWidth -= mxVScrollbar->get_vscroll_width();
        mxVScrollbar->set_vpolicy(VclPolicyType::ALWAYS);
    }
    else
    {
        mxVScrollbar->set_vpolicy(VclPolicyType::NEVER);
    }

#if 0
    {
        const Size aSizeOfMetadataControls( GetSizePixel().Width() - GetMetaButtonAreaWidth(),
                                            GetMetaHeight()/numFields );
        mxMetadataAuthor->setPosSizePixel( 0,
                                           aHeight,
                                           aSizeOfMetadataControls.Width(),
                                           aSizeOfMetadataControls.Height() );
        mxMetadataDate->setPosSizePixel( 0,
                                         aHeight + aSizeOfMetadataControls.Height(),
                                         aSizeOfMetadataControls.Width(),
                                         aSizeOfMetadataControls.Height() );
        if(IsResolved()) {
            mxMetadataResolved->setPosSizePixel( 0,
                                                 aHeight + aSizeOfMetadataControls.Height()*2,
                                                 aSizeOfMetadataControls.Width(),
                                                 aSizeOfMetadataControls.Height() );
        }
    }
#endif

    mpOutliner->SetPaperSize( PixelToLogic( Size(aWidth,aHeight) ) ) ;

    if (comphelper::LibreOfficeKit::isActive() && !mpOutlinerView->GetViewShell())
    {
        mpOutlinerView->RegisterViewShell(&mrView);
    }

    if (mxVScrollbar->get_vpolicy() == VclPolicyType::NEVER)
    {   // if we do not have a scrollbar anymore, we want to see the complete text
        mpOutlinerView->SetVisArea( PixelToLogic( tools::Rectangle(0,0,aWidth,aHeight) ) );
    }
    tools::Rectangle aOutputArea = PixelToLogic(tools::Rectangle(0, 0, aWidth, aHeight));
    mpOutlinerView->SetOutputArea(aOutputArea);

    // Don't leave an empty area at the bottom if we can move the text down.
    tools::Long nMaxVisAreaTop = mpOutliner->GetTextHeight() - aOutputArea.GetHeight();
    if (mpOutlinerView->GetVisArea().Top() > nMaxVisAreaTop)
    {
        GetOutlinerView()->Scroll(0, mpOutlinerView->GetVisArea().Top() - nMaxVisAreaTop);
    }

    if (!AllSettings::GetLayoutRTL())
    {
//TODO        mxSidebarTextControl->setPosSizePixel(0, 0, aWidth, aHeight);
//TODO        mpVScrollbar->setPosSizePixel( aWidth, 0, GetPrefScrollbarWidth(), aHeight);
    }
    else
    {
//TODO        mxSidebarTextControl->setPosSizePixel( ( aTextHeight > aHeight ? GetPrefScrollbarWidth() : 0 ), 0,
//TODO                                      aWidth, aHeight);
//TODO        mpVScrollbar->setPosSizePixel( 0, 0, GetPrefScrollbarWidth(), aHeight);
    }

    int nUpper = mpOutliner->GetTextHeight();
    int nCurrentDocPos = mpOutlinerView->GetVisArea().Top();
    int nStepIncrement = mpOutliner->GetTextHeight() / 10;
    int nPageIncrement = PixelToLogic(Size(0,aHeight)).Height() * 8 / 10;
    int nPageSize = PixelToLogic(Size(0,aHeight)).Height();

    /* limit the page size to below nUpper because gtk's gtk_scrolled_window_start_deceleration has
       effectively...

       lower = gtk_adjustment_get_lower
       upper = gtk_adjustment_get_upper - gtk_adjustment_get_page_size

       and requires that upper > lower or the deceleration animation never ends
    */
    nPageSize = std::min(nPageSize, nUpper);

    mxVScrollbar->vadjustment_configure(nCurrentDocPos, 0, nUpper,
                                        nStepIncrement, nPageIncrement, nPageSize);

#if 0
    //calculate rects for meta- button
    const Fraction& fx( GetMapMode().GetScaleX() );
    const Fraction& fy( GetMapMode().GetScaleY() );

    const Point aPos( mxMetadataAuthor->GetPosPixel());
    mpMenuButton->setPosSizePixel( tools::Long(aPos.X()+GetSizePixel().Width()-(METABUTTON_WIDTH+10)*fx),
                                   tools::Long(aPos.Y()+5*fy),
                                   tools::Long(METABUTTON_WIDTH*fx),
                                   tools::Long(METABUTTON_HEIGHT*fy) );
#endif
}

void SwAnnotationWin::SetSizePixel( const Size& rNewSize )
{
    InterimItemWindow::SetSizePixel(rNewSize);

    if (mpShadow)
    {
        Point aStart = EditWin().PixelToLogic(GetPosPixel()+Point(0,GetSizePixel().Height()));
        Point aEnd = EditWin().PixelToLogic(GetPosPixel()+Point(GetSizePixel().Width()-1,GetSizePixel().Height()));
        mpShadow->SetPosition(basegfx::B2DPoint(aStart.X(),aStart.Y()), basegfx::B2DPoint(aEnd.X(),aEnd.Y()));
    }
}

void SwAnnotationWin::SetScrollbar()
{
    mxVScrollbar->vadjustment_set_value(mpOutlinerView->GetVisArea().Top());
}

void SwAnnotationWin::ResizeIfNecessary(tools::Long aOldHeight, tools::Long aNewHeight)
{
    if (aOldHeight != aNewHeight)
    {
        //check for lower border or next note
        tools::Long aBorder = mrMgr.GetNextBorder();
        if (aBorder != -1)
        {
            if (aNewHeight > GetMinimumSizeWithoutMeta())
            {
                tools::Long aNewLowerValue = GetPosPixel().Y() + aNewHeight + GetMetaHeight();
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

    if ( Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
        return;

    m_xContainer->set_background(mColorDark);

    mxMetadataAuthor->set_font_color(aColorAnchor);

    mxMetadataDate->set_font_color(aColorAnchor);

    mxMetadataResolved->set_font_color(aColorAnchor);

    mxVScrollbar->customize_scrollbars(mColorLight,
                                       mColorAnchor,
                                       mColorDark,
                                       GetPrefScrollbarWidth());
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
    IDocumentUndoRedo& rUndoRedo(
        mrView.GetDocShell()->GetDoc()->GetIDocumentUndoRedo());
    const bool bDocUndoEnabled = rUndoRedo.DoesUndo();
    const bool bOutlinerUndoEnabled = mpOutliner->IsUndoEnabled();
    const bool bOutlinerModified = mpOutliner->IsModified();
    const bool bDisableAndRestoreUndoMode = !bDocUndoEnabled && bOutlinerUndoEnabled;

    if (bDisableAndRestoreUndoMode)
    {
        // doc undo is disabled, but outliner was enabled, turn outliner undo off
        // for the duration of this function
        mpOutliner->EnableUndo(false);
    }

    Link<LinkParamNone*,void> aLink = mpOutliner->GetModifyHdl();
    mpOutliner->SetModifyHdl( Link<LinkParamNone*,void>() );
    ESelection aOld = GetOutlinerView()->GetSelection();

    ESelection aNewSelection( 0, 0, mpOutliner->GetParagraphCount()-1, EE_TEXTPOS_ALL );
    GetOutlinerView()->SetSelection( aNewSelection );
    SfxItemSet aEditAttr(GetOutlinerView()->GetAttribs());
    aEditAttr.Put(rNewItem);
    GetOutlinerView()->SetAttribs( aEditAttr );

    if (!mpOutliner->IsUndoEnabled() && !bOutlinerModified)
    {
        // if undo was disabled (e.g. this is a redo action) and we were
        // originally 'unmodified' keep it that way
        mpOutliner->ClearModifyFlag();
    }

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

    // restore original mode
    if (bDisableAndRestoreUndoMode)
        mpOutliner->EnableUndo(true);

    Invalidate();
}

void SwAnnotationWin::GetFocus()
{
    if (mxSidebarTextControl)
        mxSidebarTextControl->GrabFocus();
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
    if (mpTextRangeOverlay && !mpTextRangeOverlay->isVisible())
        mpTextRangeOverlay->setVisible(true);

    // Invalidate.
    InvalidateControl();
    collectUIInformation("SHOW",get_id());
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
    if (mpTextRangeOverlay && mpTextRangeOverlay->isVisible())
        mpTextRangeOverlay->setVisible(false);
    collectUIInformation("HIDE",get_id());
}

void SwAnnotationWin::InvalidateControl()
{
#if 0
    // Invalidate.
    mxSidebarTextControl->Push(PushFlags::MAPMODE);
    lcl_translateTwips(EditWin(), *mxSidebarTextControl, nullptr);
    mxSidebarTextControl->Invalidate();
    mxSidebarTextControl->Pop();
#endif
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
                OutlinerParaObject* pPara = new OutlinerParaObject(GetOutlinerView()->GetEditView().CreateTextObject());
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
        case FN_DELETE_COMMENT_THREAD:
            DeleteThread();
            break;
        case FN_RESOLVE_NOTE:
            ToggleResolved();
            DoResize();
            Invalidate();
            mrMgr.LayoutPostIts();
            break;
        case FN_RESOLVE_NOTE_THREAD:
            GetTopReplyNote()->SetResolved(!IsThreadResolved());
            mrMgr.UpdateResolvedStatus(GetTopReplyNote());
            DoResize();
            Invalidate();
            mrMgr.LayoutPostIts();
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

tools::Long SwAnnotationWin::GetPostItTextHeight()
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
        MouseMoveHdl(*pMouseEvt);
    }
}

IMPL_LINK(SwAnnotationWin, MouseMoveHdl, const MouseEvent&, rMEvt, bool)
{
    if (rMEvt.IsEnterWindow())
    {
        mbMouseOver = true;
        if ( !HasFocus() )
        {
            SetViewState(ViewState::VIEW);
            Invalidate();
        }
    }
    else if (rMEvt.IsLeaveWindow())
    {
        mbMouseOver = false;
        if ( !HasFocus() )
        {
            SetViewState(ViewState::NORMAL);
            Invalidate();
        }
    }
    return false;
}

bool SwAnnotationWin::SetActiveSidebarWin()
{
    if (mrMgr.GetActiveSidebarWin() == this)
        return false;
    const bool bLockView = mrView.GetWrtShell().IsViewLocked();
    mrView.GetWrtShell().LockView( true );
    mrMgr.SetActiveSidebarWin(this);
    mrView.GetWrtShell().LockView( bLockView );
    return true;
}

void SwAnnotationWin::UnsetActiveSidebarWin()
{
    if (mrMgr.GetActiveSidebarWin() != this)
        return;
    const bool bLockView = mrView.GetWrtShell().IsViewLocked();
    mrView.GetWrtShell().LockView( true );
    mrMgr.SetActiveSidebarWin(nullptr);
    mrView.GetWrtShell().LockView( bLockView );
}

IMPL_LINK(SwAnnotationWin, ScrollHdl, weld::ScrolledWindow&, rScrolledWindow, void)
{
    tools::Long nDiff = GetOutlinerView()->GetEditView().GetVisArea().Top() - rScrolledWindow.vadjustment_get_value();
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

sal_Int32 SwAnnotationWin::GetPrefScrollbarWidth() const
{
    return mrView.GetWrtShell().GetViewOptions()->GetZoom() / 10;
}

sal_Int32 SwAnnotationWin::GetMetaButtonAreaWidth() const
{
    const Fraction& f( GetMapMode().GetScaleX() );
    return tools::Long(METABUTTON_AREA_WIDTH * f);
}

sal_Int32 SwAnnotationWin::GetMetaHeight() const
{
    const Fraction& f(mrView.GetWrtShellPtr()->GetOut()->GetMapMode().GetScaleY());
    const int fields = GetNumFields();
    return tools::Long(fields*POSTIT_META_FIELD_HEIGHT*f);
}

sal_Int32 SwAnnotationWin::GetNumFields() const
{
    return IsResolved() ? 3 : 2;
}

sal_Int32 SwAnnotationWin::GetMinimumSizeWithMeta() const
{
    return mrMgr.GetMinimumSizeWithMeta();
}

sal_Int32 SwAnnotationWin::GetMinimumSizeWithoutMeta() const
{
    const Fraction& f(mrView.GetWrtShellPtr()->GetOut()->GetMapMode().GetScaleY());
    return tools::Long(POSTIT_MINIMUMSIZE_WITHOUT_META * f);
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
                if ( pWin != this && pWin->Anchor() )
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
                    if ( ( pTopWinSelf != this ) &&
                         ( pTopWinSelf != pTopWinActive ) &&
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
    SwAnnotationWin* pTopNote = this;
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
    collectUIInformation("LEAVE",get_id());
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
    return static_cast<bool>(mxVScrollbar);
}

bool SwAnnotationWin::IsScrollbarVisible() const
{
    return HasScrollbar() && mxVScrollbar->get_vpolicy() == VclPolicyType::ALWAYS;
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

} // eof of namespace sw::sidebarwindows

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
