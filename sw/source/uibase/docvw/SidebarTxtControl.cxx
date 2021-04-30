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

#include "SidebarTxtControl.hxx"

#include <docsh.hxx>
#include <doc.hxx>

#include <PostItMgr.hxx>
#include <edtwin.hxx>

#include <cmdid.h>
#include <strings.hrc>

#include <unotools/securityoptions.hxx>
#include <officecfg/Office/Common.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sfxhelp.hxx>

#include <vcl/commandevent.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <vcl/weld.hxx>
#include <vcl/gradient.hxx>
#include <vcl/settings.hxx>

#include <editeng/outliner.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/flditem.hxx>
#include <comphelper/lok.hxx>
#include <sfx2/lokhelper.hxx>

#include <uitool.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <AnnotationWin.hxx>
#include <redline.hxx>
#include <memory>

namespace sw::sidebarwindows {

SidebarTextControl::SidebarTextControl(sw::annotation::SwAnnotationWin& rSidebarWin,
                                       SwView& rDocView,
                                       SwPostItMgr& rPostItMgr)
    : mrSidebarWin(rSidebarWin)
    , mrDocView(rDocView)
    , mrPostItMgr(rPostItMgr)
    , mbMouseDownGainingFocus(false)
{
}

EditView* SidebarTextControl::GetEditView() const
{
    OutlinerView* pOutlinerView = mrSidebarWin.GetOutlinerView();
    if (!pOutlinerView)
        return nullptr;
    return &pOutlinerView->GetEditView();
}

EditEngine* SidebarTextControl::GetEditEngine() const
{
    OutlinerView* pOutlinerView = mrSidebarWin.GetOutlinerView();
    if (!pOutlinerView)
        return nullptr;
    return pOutlinerView->GetEditView().GetEditEngine();
}

void SidebarTextControl::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    Size aSize(0, 0);
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());

    SetOutputSizePixel(aSize);

    weld::CustomWidgetController::SetDrawingArea(pDrawingArea);

    EnableRTL(false);

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Color aBgColor = rStyleSettings.GetWindowColor();

    OutputDevice& rDevice = pDrawingArea->get_ref_device();

    rDevice.SetMapMode(MapMode(MapUnit::MapTwip));
    rDevice.SetBackground(aBgColor);

    Size aOutputSize(rDevice.PixelToLogic(aSize));
    aSize = aOutputSize;
    aSize.setHeight(aSize.Height());

    EditView* pEditView = GetEditView();
    pEditView->setEditViewCallbacks(this);

    EditEngine* pEditEngine = GetEditEngine();
    // For tdf#143443 note we want an 'infinite' height initially (which is the
    // editengines default). For tdf#144686 it is helpful if the initial width
    // is the "SidebarWidth" so the calculated text height is always meaningful
    // for layout in the sidebar.
    Size aPaperSize(mrPostItMgr.GetSidebarWidth(), pEditEngine->GetPaperSize().Height());
    pEditEngine->SetPaperSize(aPaperSize);
    pEditEngine->SetRefDevice(&rDevice);

    pEditView->SetOutputArea(tools::Rectangle(Point(0, 0), aOutputSize));
    pEditView->SetBackgroundColor(aBgColor);

    pDrawingArea->set_cursor(PointerStyle::Text);

#ifndef ENABLE_WASM_STRIP_ACCESSIBILITY
    InitAccessible();
#endif
}

void SidebarTextControl::SetCursorLogicPosition(const Point& rPosition, bool bPoint, bool bClearMark)
{
    Point aMousePos = EditViewOutputDevice().PixelToLogic(rPosition);
    m_xEditView->SetCursorLogicPosition(aMousePos, bPoint, bClearMark);
}

void SidebarTextControl::GetFocus()
{
    WeldEditView::GetFocus();
    if ( !mrSidebarWin.IsMouseOver() )
        Invalidate();
    mrSidebarWin.SetActiveSidebarWin();
}

void SidebarTextControl::LoseFocus()
{
    // write the visible text back into the SwField
    mrSidebarWin.UpdateData();

    WeldEditView::LoseFocus();
    if ( !mrSidebarWin.IsMouseOver() )
    {
        Invalidate();
    }
    // set false for autoscroll to typing location
    mrSidebarWin.LockView(false);
}

OUString SidebarTextControl::RequestHelp(tools::Rectangle& rHelpRect)
{
    if (EditView* pEditView = GetEditView())
    {
        Point aPos = rHelpRect.TopLeft();

        const OutputDevice& rOutDev = pEditView->GetOutputDevice();
        Point aLogicClick = rOutDev.PixelToLogic(aPos);
        const SvxFieldItem* pItem = pEditView->GetField(aLogicClick);
        if (pItem)
        {
            const SvxFieldData* pField = pItem->GetField();
            const SvxURLField* pURL = dynamic_cast<const SvxURLField*>( pField  );
            if (pURL)
            {
                rHelpRect = tools::Rectangle(aPos, Size(50, 10));
                return SfxHelp::GetURLHelpText(pURL->GetURL());
            }
        }
    }

    TranslateId pResId;
    switch( mrSidebarWin.GetLayoutStatus() )
    {
        case SwPostItHelper::INSERTED:  pResId = STR_REDLINE_INSERT; break;
        case SwPostItHelper::DELETED:   pResId = STR_REDLINE_DELETE; break;
        default: break;
    }

    SwContentAtPos aContentAtPos( IsAttrAtPos::Redline );
    if ( pResId &&
         mrDocView.GetWrtShell().GetContentAtPos( mrSidebarWin.GetAnchorPos(), aContentAtPos ) )
    {
        OUString sText = SwResId(pResId) + ": " +
                        aContentAtPos.aFnd.pRedl->GetAuthorString() + " - " +
                        GetAppLangDateTimeString( aContentAtPos.aFnd.pRedl->GetTimeStamp() );
        return sText;
    }

    return OUString();
}

void SidebarTextControl::EditViewScrollStateChange()
{
    mrSidebarWin.SetScrollbar();
}

void SidebarTextControl::DrawForPage(OutputDevice* pDev, const Point& rPt)
{
    //Take the control's height, but overwrite the scrollbar area if there was one
    OutputDevice& rDevice = GetDrawingArea()->get_ref_device();
    Size aSize(rDevice.PixelToLogic(GetOutputSizePixel()));

    if (OutlinerView* pOutlinerView = mrSidebarWin.GetOutlinerView())
    {
        pOutlinerView->GetOutliner()->Draw(*pDev, tools::Rectangle(rPt, aSize));
    }

    if ( mrSidebarWin.GetLayoutStatus()!=SwPostItHelper::DELETED )
        return;

    pDev->Push(vcl::PushFlags::LINECOLOR);

    pDev->SetLineColor(mrSidebarWin.GetChangeColor());
    Point aBottomRight(rPt);
    aBottomRight.Move(aSize);
    pDev->DrawLine(rPt,  aBottomRight);

    Point aTopRight(rPt);
    aTopRight.Move(Size(aSize.Width(), 0));

    Point aBottomLeft(rPt);
    aBottomLeft.Move(Size(0, aSize.Height()));

    pDev->DrawLine(aTopRight, aBottomLeft);

    pDev->Pop();
}

void SidebarTextControl::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    Size aSize = GetOutputSizePixel();
    Point aPos;

    if (!rRenderContext.GetSettings().GetStyleSettings().GetHighContrastMode())
    {
        if (mrSidebarWin.IsMouseOverSidebarWin() || HasFocus())
        {
            rRenderContext.DrawGradient(tools::Rectangle(aPos, rRenderContext.PixelToLogic(aSize)),
                                        Gradient(GradientStyle::Linear, mrSidebarWin.ColorDark(), mrSidebarWin.ColorDark()));
        }
        else
        {
            rRenderContext.DrawGradient(tools::Rectangle(aPos, rRenderContext.PixelToLogic(aSize)),
                           Gradient(GradientStyle::Linear, mrSidebarWin.ColorLight(), mrSidebarWin.ColorDark()));
        }
    }

    DoPaint(rRenderContext, rRect);

    if (mrSidebarWin.GetLayoutStatus() != SwPostItHelper::DELETED)
        return;

    const AntialiasingFlags nFormerAntialiasing( rRenderContext.GetAntialiasing() );
    const bool bIsAntiAliasing = officecfg::Office::Common::Drawinglayer::AntiAliasing::get();
    if ( bIsAntiAliasing )
        rRenderContext.SetAntialiasing(AntialiasingFlags::Enable);
    rRenderContext.SetLineColor(mrSidebarWin.GetChangeColor());
    rRenderContext.DrawLine(rRenderContext.PixelToLogic(aPos),
                            rRenderContext.PixelToLogic(aPos + Point(aSize.Width(),
                                                                     aSize.Height() * 0.95)));
    rRenderContext.DrawLine(rRenderContext.PixelToLogic(aPos + Point(aSize.Width(),
                                                                     0)),
                            rRenderContext.PixelToLogic(aPos + Point(0,
                                                                     aSize.Height() * 0.95)));
    if ( bIsAntiAliasing )
        rRenderContext.SetAntialiasing(nFormerAntialiasing);
}

void SidebarTextControl::MakeVisible()
{
    //let's make sure we see our note
    mrPostItMgr.MakeVisible(&mrSidebarWin);
}

bool SidebarTextControl::KeyInput( const KeyEvent& rKeyEvt )
{
    if (getenv("SW_DEBUG") && rKeyEvt.GetKeyCode().GetCode() == KEY_F12)
    {
        if (rKeyEvt.GetKeyCode().IsShift())
        {
            mrDocView.GetDocShell()->GetDoc()->dumpAsXml();
            return true;
        }
    }

    bool bDone = false;

    const vcl::KeyCode& rKeyCode = rKeyEvt.GetKeyCode();
    sal_uInt16 nKey = rKeyCode.GetCode();
    if ( ( rKeyCode.IsMod1() && rKeyCode.IsMod2() ) &&
         ( (nKey == KEY_PAGEUP) || (nKey == KEY_PAGEDOWN) ) )
    {
        mrSidebarWin.SwitchToPostIt(nKey);
        bDone = true;
    }
    else if ( nKey == KEY_ESCAPE ||
              ( rKeyCode.IsMod1() &&
                ( nKey == KEY_PAGEUP ||
                  nKey == KEY_PAGEDOWN ) ) )
    {
        mrSidebarWin.SwitchToFieldPos();
        bDone = true;
    }
    else if ( rKeyCode.GetFullCode() == KEY_INSERT )
    {
        mrSidebarWin.ToggleInsMode();
        bDone = true;
    }
    else
    {
        MakeVisible();

        tools::Long aOldHeight = mrSidebarWin.GetPostItTextHeight();

        /// HACK: need to switch off processing of Undo/Redo in Outliner
        if ( !( (nKey == KEY_Z || nKey == KEY_Y) && rKeyCode.IsMod1()) )
        {
            bool bIsProtected = mrSidebarWin.IsProtected();
            if ( !bIsProtected || !EditEngine::DoesKeyChangeText(rKeyEvt) )
            {
                EditView* pEditView = GetEditView();
                bDone = pEditView && pEditView->PostKeyEvent(rKeyEvt);
            }
            else
            {
                std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetDrawingArea(), "modules/swriter/ui/inforeadonlydialog.ui"));
                std::unique_ptr<weld::MessageDialog> xQuery(xBuilder->weld_message_dialog("InfoReadonlyDialog"));
                xQuery->run();
            }
        }
        if (bDone)
            mrSidebarWin.ResizeIfNecessary( aOldHeight, mrSidebarWin.GetPostItTextHeight() );
        else
        {
            // write back data first when showing navigator
            if ( nKey==KEY_F5 )
                mrSidebarWin.UpdateData();
            bDone = mrDocView.KeyInput(rKeyEvt);
        }
    }

    mrDocView.GetViewFrame()->GetBindings().InvalidateAll(false);

    return bDone;
}

bool SidebarTextControl::MouseButtonDown(const MouseEvent& rMEvt)
{
    if (EditView* pEditView = GetEditView())
    {
        bool bExecuteMod = SvtSecurityOptions::IsOptionSet( SvtSecurityOptions::EOption::CtrlClickHyperlink);

        if ( !bExecuteMod || (rMEvt.GetModifier() == KEY_MOD1))
        {
            const OutputDevice& rOutDev = pEditView->GetOutputDevice();
            Point aLogicClick = rOutDev.PixelToLogic(rMEvt.GetPosPixel());
            if (const SvxFieldItem* pItem = pEditView->GetField(aLogicClick))
            {
                const SvxFieldData* pField = pItem->GetField();
                const SvxURLField* pURL = dynamic_cast<const SvxURLField*>( pField  );
                if ( pURL )
                {
                    pEditView->MouseButtonDown( rMEvt );
                    SwWrtShell &rSh = mrDocView.GetWrtShell();
                    const OUString& sURL( pURL->GetURL() );
                    const OUString& sTarget( pURL->GetTargetFrame() );
                    ::LoadURL(rSh, sURL, LoadUrlFlags::NONE, sTarget);
                    return true;
                }
            }
        }
    }

    mbMouseDownGainingFocus = !HasFocus();
    GrabFocus();

    bool bRet = WeldEditView::MouseButtonDown(rMEvt);

    mrDocView.GetViewFrame()->GetBindings().InvalidateAll(false);

    return bRet;
}

bool SidebarTextControl::MouseButtonUp(const MouseEvent& rMEvt)
{
    bool bRet = WeldEditView::MouseButtonUp(rMEvt);

    if (mbMouseDownGainingFocus)
    {
        MakeVisible();
        mbMouseDownGainingFocus = false;
    }

    return bRet;
}

IMPL_LINK( SidebarTextControl, OnlineSpellCallback, SpellCallbackInfo&, rInfo, void )
{
    if ( rInfo.nCommand == SpellCallbackCommand::STARTSPELLDLG )
    {
        mrDocView.GetViewFrame()->GetDispatcher()->Execute( FN_SPELL_GRAMMAR_DIALOG, SfxCallMode::ASYNCHRON);
    }
}

bool SidebarTextControl::Command( const CommandEvent& rCEvt )
{
    EditView* pEditView = GetEditView();

    if ( rCEvt.GetCommand() == CommandEventId::ContextMenu )
    {
        if (IsMouseCaptured())
            ReleaseMouse();
        if ( !mrSidebarWin.IsProtected() &&
             pEditView &&
             pEditView->IsWrongSpelledWordAtPos( rCEvt.GetMousePosPixel(), true ))
        {
            Link<SpellCallbackInfo&,void> aLink = LINK(this, SidebarTextControl, OnlineSpellCallback);
            pEditView->ExecuteSpellPopup(rCEvt.GetMousePosPixel(), aLink);
        }
        else
        {
            Point aPos;
            if (rCEvt.IsMouseEvent())
                aPos = rCEvt.GetMousePosPixel();
            else
            {
                const Size aSize = GetOutputSizePixel();
                aPos = Point( aSize.getWidth()/2, aSize.getHeight()/2 );
            }
            SfxDispatcher::ExecutePopup(&mrSidebarWin, &aPos);
        }
        return true;
    }
    else if (rCEvt.GetCommand() == CommandEventId::Wheel)
    {
        // if no scrollbar, or extra keys held scroll the document and consume
        // this event, otherwise don't consume and let the event get to the
        // surrounding scrolled window
        if (!mrSidebarWin.IsScrollbarVisible())
        {
            mrDocView.HandleWheelCommands(rCEvt);
            return true;
        }
        else
        {
            const CommandWheelData* pData = rCEvt.GetWheelData();
            if (pData->IsShift() || pData->IsMod1() || pData->IsMod2())
            {
                mrDocView.HandleWheelCommands(rCEvt);
                return true;
            }
        }
    }

    return WeldEditView::Command(rCEvt);
}

} // end of namespace sw::sidebarwindows

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
