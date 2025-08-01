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

#include <config_wasm_strip.h>

#include <sal/config.h>

#include <cstddef>

#include "SidebarWinAcc.hxx"
#include <PostItMgr.hxx>
#include <AnnotationWin.hxx>
#include <IDocumentUndoRedo.hxx>
#include <basegfx/range/b2drange.hxx>
#include "SidebarTxtControl.hxx"
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
#include <editeng/editund2.hxx>

#include <svl/undo.hxx>
#include <svl/stritem.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>

#include <vcl/decoview.hxx>
#include <vcl/event.hxx>
#include <vcl/gradient.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/ptrstyle.hxx>
#include <vcl/uitest/logger.hxx>
#include <vcl/uitest/eventdescription.hxx>

#include <svx/postattr.hxx>

#include <edtwin.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <doc.hxx>
#include <docstyle.hxx>
#include <docufld.hxx>
#include <swmodule.hxx>

#include <SwRewriter.hxx>
#include <txtannotationfld.hxx>
#include <ndtxt.hxx>
#include <svtools/colorcfg.hxx>

#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <officecfg/Office/Writer.hxx>
#include <osl/diagnose.h>
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

}

namespace sw::annotation {

#define METABUTTON_WIDTH        16
#define METABUTTON_HEIGHT       18
#define POSTIT_MINIMUMSIZE_WITHOUT_META     50

void SwAnnotationWin::PaintTile(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    bool bMenuButtonVisible = mxMenuButton->get_visible();
    // No point in showing this button till click on it are not handled.
    if (bMenuButtonVisible)
        mxMenuButton->hide();

    // draw left over space
    if (Application::GetSettings().GetStyleSettings().GetHighContrastMode())
        rRenderContext.SetFillColor(COL_BLACK);
    else
        rRenderContext.SetFillColor(mColorDark);
    rRenderContext.SetLineColor();
    rRenderContext.DrawRect(rRect);

    m_xContainer->draw(rRenderContext, rRect.TopLeft(), GetSizePixel());

    const drawinglayer::geometry::ViewInformation2D aViewInformation;
    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor(drawinglayer::processor2d::createProcessor2DFromOutputDevice(rRenderContext, aViewInformation));

    // drawinglayer sets the map mode to pixels, not needed here.
    rRenderContext.Pop();
    // Work in document-global twips.
    rRenderContext.Pop();
    if (mpAnchor)
        pProcessor->process(mpAnchor->getOverlayObjectPrimitive2DSequence());
    if (mpTextRangeOverlay)
        pProcessor->process(mpTextRangeOverlay->getOverlayObjectPrimitive2DSequence());

    rRenderContext.Push(vcl::PushFlags::NONE);
    pProcessor.reset();
    rRenderContext.Push(vcl::PushFlags::NONE);

    if (bMenuButtonVisible)
        mxMenuButton->show();
}

bool SwAnnotationWin::IsHitWindow(const Point& rPointLogic)
{
    tools::Rectangle aRectangleLogic(EditWin().PixelToLogic(tools::Rectangle(GetPosPixel(),GetSizePixel())));
    return aRectangleLogic.Contains(rPointLogic);
}

void SwAnnotationWin::SetCursorLogicPosition(const Point& rPosition, bool bPoint, bool bClearMark)
{
    mxSidebarTextControl->SetCursorLogicPosition(rPosition, bPoint, bClearMark);
}

void SwAnnotationWin::DrawForPage(OutputDevice* pDev, const Point& rPt)
{
    // tdf#143511 unclip SysObj so get_extents_relative_to of children
    // of the SysObj can provide meaningful results
    UnclipVisibleSysObj();

    vcl::PDFExtOutDevData *const pPDFExtOutDevData(
        dynamic_cast<vcl::PDFExtOutDevData*>(pDev->GetExtOutDevData()));
    if (pPDFExtOutDevData && pPDFExtOutDevData->GetIsExportTaggedPDF())
    {
        pPDFExtOutDevData->WrapBeginStructureElement(vcl::pdf::StructElement::NonStructElement, OUString());
    }

    pDev->Push();

    pDev->SetFillColor(mColorDark);
    pDev->SetLineColor();

    pDev->SetTextColor(mColorDark.IsDark() ? COL_WHITE : COL_BLACK);
    vcl::Font aFont = maLabelFont;
    aFont.SetFontHeight(aFont.GetFontHeight() * 20);
    pDev->SetFont(aFont);

    Size aSz = PixelToLogic(GetSizePixel());

    pDev->DrawRect(tools::Rectangle(rPt, aSz));

    if (mxMetadataAuthor->get_visible())
    {
        int x, y, width, height;
        mxMetadataAuthor->get_extents_relative_to(*m_xContainer, x, y, width, height);
        Point aPos(rPt + PixelToLogic(Point(x, y)));
        Size aSize(PixelToLogic(Size(width, height)));

        pDev->Push(vcl::PushFlags::CLIPREGION);
        pDev->IntersectClipRegion(tools::Rectangle(aPos, aSize));
        pDev->DrawText(aPos, mxMetadataAuthor->get_label());
        pDev->Pop();
    }

    if (mxMetadataDate->get_visible())
    {
        int x, y, width, height;
        mxMetadataDate->get_extents_relative_to(*m_xContainer, x, y, width, height);
        Point aPos(rPt + PixelToLogic(Point(x, y)));
        Size aSize(PixelToLogic(Size(width, height)));

        pDev->Push(vcl::PushFlags::CLIPREGION);
        pDev->IntersectClipRegion(tools::Rectangle(aPos, aSize));
        pDev->DrawText(aPos, mxMetadataDate->get_label());
        pDev->Pop();
    }

    if (mxMetadataResolved->get_visible())
    {
        int x, y, width, height;
        mxMetadataResolved->get_extents_relative_to(*m_xContainer, x, y, width, height);
        Point aPos(rPt + PixelToLogic(Point(x, y)));
        Size aSize(PixelToLogic(Size(width, height)));

        pDev->Push(vcl::PushFlags::CLIPREGION);
        pDev->IntersectClipRegion(tools::Rectangle(aPos, aSize));
        pDev->DrawText(aPos, mxMetadataResolved->get_label());
        pDev->Pop();
    }

    mxSidebarTextControl->DrawForPage(pDev, rPt);

    const drawinglayer::geometry::ViewInformation2D aNewViewInfos;
    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor(
        drawinglayer::processor2d::createProcessor2DFromOutputDevice(
            *pDev, aNewViewInfos ));

    if (mpAnchor)
        pProcessor->process(mpAnchor->getOverlayObjectPrimitive2DSequence());
    if (mpTextRangeOverlay)
        pProcessor->process(mpTextRangeOverlay->getOverlayObjectPrimitive2DSequence());
    pProcessor.reset();

    if (mxVScrollbar->get_vpolicy() != VclPolicyType::NEVER)
    {
        // if there is a scrollbar shown, draw "..." to indicate the comment isn't
        // completely shown
        int x, y, width, height;
        mxMenuButton->get_extents_relative_to(*m_xContainer, x, y, width, height);
        Point aPos(rPt + PixelToLogic(Point(x, y)));
        pDev->DrawText(aPos, u"..."_ustr);
    }

    pDev->Pop();

    if (pPDFExtOutDevData && pPDFExtOutDevData->GetIsExportTaggedPDF())
    {
        pPDFExtOutDevData->EndStructureElement();
    }
}

void SwAnnotationWin::SetPosSizePixelRect(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight,
                                          const tools::Long aPageBorder)
{
    mPosSize = tools::Rectangle(Point(nX,nY),Size(nWidth,nHeight));
    mPageBorder = aPageBorder;
}

void SwAnnotationWin::SetAnchorRect(const SwRect& aAnchorRect)
{
    if (!mAnchorRect.IsEmpty() && mAnchorRect != aAnchorRect)
        mbAnchorRectChanged = true;
    mAnchorRect = aAnchorRect;
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

void SwAnnotationWin::InitControls()
{
    // window controls for author and date
    mxMetadataAuthor = m_xBuilder->weld_label(u"author"_ustr);
    mxMetadataAuthor->set_accessible_name( SwResId( STR_ACCESS_ANNOTATION_AUTHOR_NAME ) );
    mxMetadataAuthor->set_direction(AllSettings::GetLayoutRTL());

    maLabelFont = Application::GetSettings().GetStyleSettings().GetLabelFont();
    maLabelFont.SetFontHeight(8);

    // we should leave this setting alone, but for this we need a better layout algo
    // with variable meta size height
    mxMetadataAuthor->set_font(maLabelFont);

    mxMetadataDate = m_xBuilder->weld_label(u"date"_ustr);
    mxMetadataDate->set_accessible_name( SwResId( STR_ACCESS_ANNOTATION_DATE_NAME ) );
    mxMetadataDate->set_direction(AllSettings::GetLayoutRTL());
    mxMetadataDate->connect_mouse_move(LINK(this, SwAnnotationWin, MouseMoveHdl));

    // we should leave this setting alone, but for this we need a better layout algo
    // with variable meta size height
    mxMetadataDate->set_font(maLabelFont);

    mxMetadataResolved = m_xBuilder->weld_label(u"resolved"_ustr);
    mxMetadataResolved->set_accessible_name( SwResId( STR_ACCESS_ANNOTATION_RESOLVED_NAME ) );
    mxMetadataResolved->set_direction(AllSettings::GetLayoutRTL());
    mxMetadataResolved->connect_mouse_move(LINK(this, SwAnnotationWin, MouseMoveHdl));

    // we should leave this setting alone, but for this we need a better layout algo
    // with variable meta size height
    mxMetadataResolved->set_font(maLabelFont);
    mxMetadataResolved->set_label(SwResId(STR_ACCESS_ANNOTATION_RESOLVED_NAME));

    SwDocShell* aShell = mrView.GetDocShell();
    mpOutliner.reset(new Outliner(&aShell->GetPool(),OutlinerMode::TextObject));
    mpOutliner->SetStyleSheetPool(static_cast<SwDocStyleSheetPool*>(aShell->GetStyleSheetPool())->GetEEStyleSheetPool());
    aShell->GetDoc()->SetCalcFieldValueHdl( mpOutliner.get() );
    mpOutliner->SetUpdateLayout( true );

    mpOutlinerView.reset(new OutlinerView(*mpOutliner, nullptr));
    mpOutliner->InsertView(mpOutlinerView.get());

    //create Scrollbars
    mxVScrollbar = m_xBuilder->weld_scrolled_window(u"scrolledwindow"_ustr, true);

    mxMenuButton = m_xBuilder->weld_menu_button(u"menubutton"_ustr);
    mxMenuButton->set_size_request(METABUTTON_WIDTH, METABUTTON_HEIGHT);

    // actual window which holds the user text
    mxSidebarTextControl.reset(new SidebarTextControl(*this, mrView, mrMgr));
    mxSidebarTextControlWin.reset(new weld::CustomWeld(*m_xBuilder, u"editview"_ustr, *mxSidebarTextControl));
    mxSidebarTextControl->SetPointer(PointerStyle::Text);

    Rescale();

    mpOutlinerView->SetBackgroundColor(COL_TRANSPARENT);
    mpOutlinerView->SetOutputArea( PixelToLogic( tools::Rectangle(0,0,1,1) ) );

    mxVScrollbar->set_direction(false);
    mxVScrollbar->connect_vadjustment_value_changed(LINK(this, SwAnnotationWin, ScrollHdl));
    mxVScrollbar->connect_mouse_move(LINK(this, SwAnnotationWin, MouseMoveHdl));

    EEControlBits nCntrl = mpOutliner->GetControlWord();
    // TODO: crash when AUTOCOMPLETE enabled
    nCntrl |= EEControlBits::MARKFIELDS | EEControlBits::PASTESPECIAL | EEControlBits::AUTOCORRECT | EEControlBits::USECHARATTRIBS; // | EEControlBits::AUTOCOMPLETE;
    // Our stylesheet pool follows closely the core paragraph styles.
    // We don't want the rtf import (during paste) to mess with that.
    nCntrl &= ~EEControlBits::RTFSTYLESHEETS;

    if (SwWrtShell* pWrtShell = mrView.GetWrtShellPtr())
    {
        const SwViewOption* pVOpt = pWrtShell->GetViewOptions();
        if (pVOpt->IsFieldShadings())
            nCntrl |= EEControlBits::MARKFIELDS;
        else
            nCntrl &= ~EEControlBits::MARKFIELDS;
        if (pVOpt->IsOnlineSpell())
            nCntrl |= EEControlBits::ONLINESPELLING;
        else
            nCntrl &= ~EEControlBits::ONLINESPELLING;
    }
    mpOutliner->SetControlWord(nCntrl);

    std::size_t aIndex = SwModule::get()->InsertRedlineAuthor(GetAuthor());
    SetColor( SwPostItMgr::GetColorDark(aIndex),
              SwPostItMgr::GetColorLight(aIndex),
              SwPostItMgr::GetColorAnchor(aIndex));

    CheckMetaText();

    // expand %1 "Author"
    OUString aText = mxMenuButton->get_item_label(u"deleteby"_ustr);
    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1, GetAuthor());
    aText = aRewriter.Apply(aText);
    mxMenuButton->set_item_label(u"deleteby"_ustr, aText);

    mxMenuButton->set_accessible_name(SwResId(STR_ACCESS_ANNOTATION_BUTTON_NAME));
    mxMenuButton->set_accessible_description(SwResId(STR_ACCESS_ANNOTATION_BUTTON_DESC));
    mxMenuButton->set_tooltip_text(SwResId(STR_ACCESS_ANNOTATION_BUTTON_DESC));

    mxMenuButton->connect_toggled(LINK(this, SwAnnotationWin, ToggleHdl));
    mxMenuButton->connect_selected(LINK(this, SwAnnotationWin, SelectHdl));
    mxMenuButton->connect_key_press(LINK(this, SwAnnotationWin, KeyInputHdl));
    mxMenuButton->connect_mouse_move(LINK(this, SwAnnotationWin, MouseMoveHdl));

    GetOutlinerView()->StartSpeller(mxSidebarTextControl->GetDrawingArea());
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
    UpdateColors();
}

void SwAnnotationWin::UpdateColors()
{
    std::size_t aIndex = SwModule::get()->InsertRedlineAuthor(GetAuthor());
    SetColor( SwPostItMgr::GetColorDark(aIndex),
              SwPostItMgr::GetColorLight(aIndex),
              SwPostItMgr::GetColorAnchor(aIndex));
    // draw comments either black or white depending on the document background
    // TODO: make editeng depend on the actual note background
    mpOutlinerView->SetBackgroundColor(svtools::ColorConfig().GetColorValue(svtools::DOCCOLOR).nColor);
}

void SwAnnotationWin::SetMenuButtonColors()
{
    if (!mxMenuButton)
        return;

    SwWrtShell* pWrtShell = mrView.GetWrtShellPtr();
    if (!pWrtShell)
        return;
    const Fraction& rFraction = pWrtShell->GetOut()->GetMapMode().GetScaleY();

    ScopedVclPtrInstance<VirtualDevice> xVirDev;
    Size aSize(tools::Long(METABUTTON_WIDTH * rFraction),
               tools::Long(METABUTTON_HEIGHT * rFraction));
    tools::Rectangle aRect(Point(0, 0), aSize);
    xVirDev->SetOutputSizePixel(aSize);

    Gradient aGradient(css::awt::GradientStyle_LINEAR,
                       mColorLight,
                       mColorDark);
    xVirDev->DrawGradient(aRect, aGradient);

    //draw rect around button
    xVirDev->SetFillColor();
    xVirDev->SetLineColor(mColorDark.IsDark() ? mColorLight : mColorDark);
    xVirDev->DrawRect(aRect);

    tools::Rectangle aSymbolRect(aRect);
    // 25% distance to the left and right button border
    const tools::Long nBorderDistanceLeftAndRight = ((aSymbolRect.GetWidth() * 250) + 500) / 1000;
    aSymbolRect.AdjustLeft(nBorderDistanceLeftAndRight );
    aSymbolRect.AdjustRight( -nBorderDistanceLeftAndRight );
    // 30% distance to the top button border
    const tools::Long nBorderDistanceTop = ((aSymbolRect.GetHeight() * 300) + 500) / 1000;
    aSymbolRect.AdjustTop(nBorderDistanceTop );
    // 25% distance to the bottom button border
    const tools::Long nBorderDistanceBottom = ((aSymbolRect.GetHeight() * 250) + 500) / 1000;
    aSymbolRect.AdjustBottom( -nBorderDistanceBottom );

    DecorationView aDecoView(xVirDev.get());
    aDecoView.DrawSymbol(aSymbolRect, SymbolType::SPIN_DOWN, mColorDark.IsDark() ? COL_WHITE : COL_BLACK,
                         DrawSymbolFlags::NONE);
    mxMenuButton->set_image(xVirDev);
    mxMenuButton->set_size_request(aSize.Width() + 4, aSize.Height() + 4);
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
    SetMapMode( aMode );
    mxSidebarTextControl->SetMapMode( aMode );

    SwWrtShell* pWrtShell = mrView.GetWrtShellPtr();
    if (!pWrtShell)
        return;
    const Fraction& rFraction = pWrtShell->GetOut()->GetMapMode().GetScaleY();

    vcl::Font aFont = maLabelFont;
    sal_Int32 nHeight = tools::Long(aFont.GetFontHeight() * rFraction);
    aFont.SetFontHeight( nHeight );

    if (mxMetadataAuthor)
        mxMetadataAuthor->set_font(aFont);
    if (mxMetadataDate)
        mxMetadataDate->set_font(aFont);
    if (mxMetadataResolved)
        mxMetadataResolved->set_font(aFont);
    SetMenuButtonColors();
    if (mxVScrollbar)
        mxVScrollbar->set_scroll_thickness(GetPrefScrollbarWidth());
}

void SwAnnotationWin::SetPosAndSize()
{
    const bool bShowNotes = mrMgr.ShowNotes();
    if (bShowNotes)
    {
        bool bChange = false;

        if (GetSizePixel() != mPosSize.GetSize())
        {
            bChange = true;
            SetSizePixel(mPosSize.GetSize());

            DoResize();
        }

        if (GetPosPixel().X() != mPosSize.Left() || (std::abs(GetPosPixel().Y() - mPosSize.Top()) > 5) )
        {
            bChange = true;
            SetPosPixel(mPosSize.TopLeft());
        }

        if (bChange)
        {
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
    if ( mpSidebarItem->maLayoutInfo.mnStartNodeIdx != SwNodeOffset(0)
         && mpSidebarItem->maLayoutInfo.mnStartContent != -1 )
    {
        const SwTextAnnotationField* pTextAnnotationField =
            dynamic_cast< const SwTextAnnotationField* >( mpSidebarItem->GetFormatField().GetTextField() );
        SwTextNode* pTextNode = pTextAnnotationField ? pTextAnnotationField->GetpTextNode() : nullptr;
        SwContentNode* pContentNd = nullptr;
        if (pTextNode)
        {
            SwNodes& rNds = pTextNode->GetDoc().GetNodes();
            pContentNd = rNds[mpSidebarItem->maLayoutInfo.mnStartNodeIdx]->GetContentNode();
        }
        if (pContentNd)
        {
            SwPosition aStartPos( *pContentNd, mpSidebarItem->maLayoutInfo.mnStartContent );
            SwShellCursor* pTmpCursor = nullptr;
            const bool bTableCursorNeeded = pTextNode->FindTableBoxStartNode() != pContentNd->FindTableBoxStartNode();
            if ( bTableCursorNeeded )
            {
                SwShellTableCursor* pTableCursor = new SwShellTableCursor( mrView.GetWrtShell(), aStartPos );
                pTableCursor->SetMark();
                pTableCursor->GetMark()->Assign( *pTextNode, pTextAnnotationField->GetStart()+1 );
                pTableCursor->NewTableSelection();
                pTmpCursor = pTableCursor;
            }
            else
            {
                SwShellCursor* pCursor = new SwShellCursor( mrView.GetWrtShell(), aStartPos );
                pCursor->SetMark();
                pCursor->GetMark()->Assign(*pTextNode, pTextAnnotationField->GetStart()+1 );
                pTmpCursor = pCursor;
            }
            std::unique_ptr<SwShellCursor> pTmpCursorForAnnotationTextRange( pTmpCursor );

            // For annotation text range rectangles to be calculated correctly,
            // we need the map mode disabled
            bool bDisableMapMode = comphelper::LibreOfficeKit::isActive() && EditWin().IsMapModeEnabled();
            if (bDisableMapMode)
                EditWin().EnableMapMode(false);

            if (mpSidebarItem->maLayoutInfo.mPositionFromCommentAnchor)
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

    if (bShowNotes && !maAnnotationTextRanges.empty())
    {
        if ( mpTextRangeOverlay != nullptr )
        {
            mpTextRangeOverlay->setRanges( std::vector(maAnnotationTextRanges) );
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
                    std::vector(maAnnotationTextRanges),
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
    tools::Long aHeight = GetSizePixel().Height();
    tools::ULong aWidth = GetSizePixel().Width();

    aHeight -= GetMetaHeight();

    mpOutliner->SetPaperSize( PixelToLogic( Size(aWidth, aHeight) ) ) ;
    tools::Long aTextHeight = LogicToPixel( mpOutliner->CalcTextSize()).Height();

    mxMetadataAuthor->show();
    if(IsResolved()) { mxMetadataResolved->show(); }
    mxMetadataDate->show();

    if (aTextHeight > aHeight)
    {
        const int nThickness = mxVScrollbar->get_scroll_thickness();
        if (nThickness)
        {
            // we need vertical scrollbars and have to reduce the width
            aWidth -= nThickness;
            mpOutliner->SetPaperSize(PixelToLogic(Size(aWidth, aHeight)));
        }
        mxVScrollbar->set_vpolicy(VclPolicyType::ALWAYS);
    }
    else
    {
        mxVScrollbar->set_vpolicy(VclPolicyType::NEVER);
    }

    tools::Rectangle aOutputArea = PixelToLogic(tools::Rectangle(0, 0, aWidth, aHeight));
    if (mxVScrollbar->get_vpolicy() == VclPolicyType::NEVER)
    {
        // if we do not have a scrollbar anymore, we want to see the complete text
        mpOutlinerView->SetVisArea(aOutputArea);
    }
    mpOutlinerView->SetOutputArea(aOutputArea);
    mpOutlinerView->ShowCursor(true, true);

    // Don't leave an empty area at the bottom if we can move the text down.
    tools::Long nMaxVisAreaTop = mpOutliner->GetTextHeight() - aOutputArea.GetHeight();
    if (mpOutlinerView->GetVisArea().Top() > nMaxVisAreaTop)
    {
        GetOutlinerView()->Scroll(0, mpOutlinerView->GetVisArea().Top() - nMaxVisAreaTop);
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

    mxVScrollbar->vadjustment_configure(nCurrentDocPos, nUpper, nStepIncrement, nPageIncrement,
                                        nPageSize);
}

void SwAnnotationWin::SetSizePixel( const Size& rNewSize )
{
    if (comphelper::LibreOfficeKit::isActive())
        return;

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
    SetMenuButtonColors();

    Color aColor(mColorDark.IsDark() ? COL_WHITE : COL_BLACK);
    mxMetadataAuthor->set_font_color(aColor);

    mxMetadataDate->set_font_color(aColor);

    mxMetadataResolved->set_font_color(aColor);

    mxVScrollbar->customize_scrollbars(mColorLight,
                                       mColorAnchor,
                                       mColorDark);
}

void SwAnnotationWin::SetSidebarPosition(sw::sidebarwindows::SidebarPosition eSidebarPosition)
{
    meSidebarPosition = eSidebarPosition;
}

void SwAnnotationWin::SetReadonly(bool bSet)
{
    // the OutlinerView determines if the comment window accepts key input
    mbReadonly = bSet && getenv("EDIT_COMMENT_IN_READONLY_MODE") == nullptr;
    GetOutlinerView()->SetReadOnly(mbReadonly);
}

void SwAnnotationWin::GetFocus()
{
    if (mxSidebarTextControl)
        mxSidebarTextControl->GrabFocus();
}

void SwAnnotationWin::LoseFocus()
{
}

void SwAnnotationWin::queue_draw()
{
    if (mxSidebarTextControlWin)
    {
        mxSidebarTextControlWin->queue_draw();
    }
}

void SwAnnotationWin::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    if (mxSidebarTextControl)
    {
        mxSidebarTextControl->Paint(rRenderContext, rRect);
    }
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

    collectUIInformation(u"SHOW"_ustr,get_id());
}

void SwAnnotationWin::HideNote()
{
    if (IsVisible())
        Window::Hide();
    if (mpAnchor)
    {
        if (officecfg::Office::Writer::Notes::ShowAnkor::get())
            mpAnchor->SetAnchorState(AnchorState::Tri);
        else
            mpAnchor->setVisible(false);
    }
    if (mpShadow && mpShadow->isVisible())
        mpShadow->setVisible(false);
    if (mpTextRangeOverlay && mpTextRangeOverlay->isVisible())
        mpTextRangeOverlay->setVisible(false);
    collectUIInformation(u"HIDE"_ustr,get_id());
}

void SwAnnotationWin::ActivatePostIt()
{
    mrMgr.AssureStdModeAtShell();

    mpOutliner->ClearModifyFlag();
    mpOutliner->GetUndoManager().Clear();

    CheckMetaText();
    SetViewState(ViewState::EDIT);

    // prevent autoscroll to the old cursor location
    // when cursor out of visible area
    GetOutlinerView()->ShowCursor(false);

    if (SwWrtShell* pWrtShell = mrView.GetWrtShellPtr())
        mpOutlinerView->GetEditView().SetInsertMode(pWrtShell->IsInsMode());

    if ( !Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
        GetOutlinerView()->SetBackgroundColor(mColorDark);

    //tdf#119130 only have the active postit as a dialog control in which pressing
    //ctrl+tab cycles between text and button so we don't waste time searching
    //thousands of SwAnnotationWins
    SetStyle(GetStyle() | WB_DIALOGCONTROL);

    mrView.GetDocShell()->Broadcast(SfxHint(SfxHintId::SwNavigatorUpdateTracking));
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
        aSelection.CollapseToStart();
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

    if (!mnDeleteEventId && !IsReadOnlyOrProtected() && !mpOutliner->GetEditEngine().HasText())
    {
        mnDeleteEventId = Application::PostUserEvent( LINK( this, SwAnnotationWin, DeleteHdl), nullptr, true );
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
        SfxBindings &rBnd = mrView.GetViewFrame().GetBindings();
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
            const bool bReply = nSlot == FN_REPLY;
            // if this note is empty, it will be deleted once losing the focus, so no reply, but only a new note
            // will be created
            if (!mrMgr.IsAnswer() && mpOutliner->GetEditEngine().HasText())
            {
                OutlinerParaObject aPara(GetOutlinerView()->GetEditView().CreateTextObject());
                mrMgr.RegisterAnswer(aPara);
            }
            if (mrMgr.HasActiveSidebarWin())
                mrMgr.SetActiveSidebarWin(nullptr);
            SwitchToFieldPos();

            SwDocShell* pShell = mrView.GetDocShell();
            if (!bReply)
            {
                // synchronous dispatch
                mrView.GetViewFrame().GetDispatcher()->Execute(FN_POSTIT);
            }
            else
            {
                pShell->GetDoc()->GetIDocumentUndoRedo().StartUndo(SwUndoId::INSERT, nullptr);
                SvxPostItIdItem parentParaId{SID_ATTR_POSTIT_PARENTPARAID};
                parentParaId.SetValue(OUString::number(GetParaId()));
                SvxPostItIdItem parentPostitId{SID_ATTR_POSTIT_PARENTPOSTITID};
                parentPostitId.SetValue(OUString::number(GetPostItField()->GetPostItId()));
                this->GeneratePostItName();
                SfxStringItem const parentName{SID_ATTR_POSTIT_PARENTNAME,
                    GetPostItField()->GetName().toString()};
                // transport parent ids to SwWrtShell::InsertPostIt()
                SfxPoolItem const* items[]{ &parentParaId, &parentPostitId, &parentName, nullptr };
                mrView.GetViewFrame().GetDispatcher()->Execute(FN_POSTIT, SfxCallMode::SLOT, items);

                auto pPostItField = mrMgr.GetLatestPostItField();

                // In this case, force generating the associated window
                // synchronously so we can bundle its use of the registered
                // "Answer" into the same undo group that the synchronous
                // FN_POSTIT was put in
                mrMgr.GetOrCreateAnnotationWindowForLatestPostItField();

                SwRewriter aRewriter;
                aRewriter.AddRule(UndoArg1, pPostItField->GetDescription());
                pShell->GetDoc()->GetIDocumentUndoRedo().EndUndo(SwUndoId::INSERT, &aRewriter);
            }
            break;
        }
        case FN_DELETE_COMMENT:
            //Delete(); // do not kill the parent of our open popup menu
            mnDeleteEventId = Application::PostUserEvent( LINK( this, SwAnnotationWin, DeleteHdl), nullptr, true );
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
            mrView.GetViewFrame().GetBindings().Execute( nSlot, nullptr, SfxCallMode::ASYNCHRON );
            break;
        case FN_DELETE_NOTE_AUTHOR:
        case FN_HIDE_NOTE_AUTHOR:
        {
            // not possible as slot as this would require that "this" is the active postit
            SfxStringItem aItem( nSlot, GetAuthor() );
            const SfxPoolItem* aItems[2];
            aItems[0] = &aItem;
            aItems[1] = nullptr;
            mrView.GetViewFrame().GetBindings().Execute( nSlot, aItems, SfxCallMode::ASYNCHRON );
        }
            break;
        case FN_PROMOTE_COMMENT:
            SetAsRoot();
            DoResize();
            Invalidate();
            mrMgr.LayoutPostIts();
            break;
        default:
            mrView.GetViewFrame().GetBindings().Execute( nSlot );
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
    mrView.GetWrtShell().LockView( true );
    mrMgr.SetActiveSidebarWin(this);
    mrView.GetWrtShell().LockView( true );

    return true;
}

void SwAnnotationWin::UnsetActiveSidebarWin()
{
    if (mrMgr.GetActiveSidebarWin() != this)
        return;
    mrView.GetWrtShell().LockView( true );
    mrMgr.SetActiveSidebarWin(nullptr);
    mrView.GetWrtShell().LockView( false );
}

void SwAnnotationWin::LockView(bool bLock)
{
    mrView.GetWrtShell().LockView( bLock );
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
    mnDeleteEventId = nullptr;
    Delete();
}

void SwAnnotationWin::ResetAttributes()
{
    mpOutlinerView->RemoveAttribsKeepLanguages(true);
    mpOutliner->RemoveFields();
}

int SwAnnotationWin::GetPrefScrollbarWidth() const
{
    if (SwWrtShell* pWrtShell = mrView.GetWrtShellPtr())
    {
        const Fraction& f(pWrtShell->GetOut()->GetMapMode().GetScaleY());
        return tools::Long(Application::GetSettings().GetStyleSettings().GetScrollBarSize() * f);
    }
    else
        return tools::Long(Application::GetSettings().GetStyleSettings().GetScrollBarSize());
}

sal_Int32 SwAnnotationWin::GetMetaHeight() const
{
    const int fields = GetNumFields();

    sal_Int32 nRequiredHeight = 0;
    weld::Label* aLabels[3] = { mxMetadataAuthor.get(), mxMetadataDate.get(), mxMetadataResolved.get() };
    for (int i = 0; i < fields; ++i)
        nRequiredHeight += aLabels[i]->get_preferred_size().Height();

    return nRequiredHeight;
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
    if (SwWrtShell* pWrtShell = mrView.GetWrtShellPtr())
    {
        const Fraction& f(pWrtShell->GetOut()->GetMapMode().GetScaleY());
        return tools::Long(POSTIT_MINIMUMSIZE_WITHOUT_META * f);
    }
    else
        return tools::Long(POSTIT_MINIMUMSIZE_WITHOUT_META);
}

void SwAnnotationWin::SetSpellChecking()
{
    if (SwWrtShell* pWrtShell = mrView.GetWrtShellPtr())
    {
        const SwViewOption* pVOpt = pWrtShell->GetViewOptions();
        EEControlBits nCntrl = mpOutliner->GetControlWord();
        mpOutliner->SetControlWord(nCntrl & ~EEControlBits::ONLINESPELLING);
        if (pVOpt->IsOnlineSpell())
            mpOutliner->SetControlWord(nCntrl | EEControlBits::ONLINESPELLING);

        mpOutliner->CompleteOnlineSpelling();
        Invalidate();
    }
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
    for (SwAnnotationWin* pTopNote = this;;)
    {
        if (!pTopNote->IsFollow())
            return pTopNote;
        SwAnnotationWin* pPrev = mrMgr.GetNextPostIt(KEY_PAGEUP, pTopNote);
        if (!pPrev)
            return pTopNote;
        pTopNote = pPrev;
    }
}

void SwAnnotationWin::SwitchToFieldPos()
{
    if ( mrMgr.GetActiveSidebarWin() == this )
            mrMgr.SetActiveSidebarWin(nullptr);
    GotoPos();
    sal_uInt32 aCount = MoveCaret();
    if (aCount)
        mrView.GetDocShell()->GetWrtShell()->SwCursorShell::Right(aCount, SwCursorSkipMode::Chars);
    GrabFocusToDocument();
    collectUIInformation(u"LEAVE"_ustr,get_id());
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

void SwAnnotationWin::ChangeSidebarItem( SwAnnotationItem & rSidebarItem )
{
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
    const bool bAnchorChanged = mpAnchorFrame != rSidebarItem.maLayoutInfo.mpAnchorFrame;
    if (bAnchorChanged && mpAnchorFrame)
    {
        mrMgr.DisconnectSidebarWinFromFrame( *mpAnchorFrame, *this );
    }
#endif

    mpSidebarItem = &rSidebarItem;
    mpAnchorFrame = mpSidebarItem->maLayoutInfo.mpAnchorFrame;
    assert(mpAnchorFrame);

#if !ENABLE_WASM_STRIP_ACCESSIBILITY
    if (mxSidebarWinAccessible)
        mxSidebarWinAccessible->ChangeSidebarItem( *mpSidebarItem );

    if ( bAnchorChanged )
    {
        mrMgr.ConnectSidebarWinToFrame( *(mpSidebarItem->maLayoutInfo.mpAnchorFrame),
                                      mpSidebarItem->GetFormatField(),
                                      *this );
    }
#endif
}

rtl::Reference<comphelper::OAccessible> SwAnnotationWin::CreateAccessible()
{
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
    if (!mxSidebarWinAccessible)
        mxSidebarWinAccessible = new SidebarWinAccessible( *this,
                                                          mrView.GetWrtShell(),
                                                          *mpSidebarItem );
#endif
    return mxSidebarWinAccessible;
}

} // eof of namespace sw::sidebarwindows

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
