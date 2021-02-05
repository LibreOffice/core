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

#include "SidebarTxtControl.hxx"

#include "SidebarTxtControlAcc.hxx"
#include <docsh.hxx>
#include <doc.hxx>

#include <PostItMgr.hxx>
#include <edtwin.hxx>

#include <cmdid.h>
#include <strings.hrc>

#include <unotools/securityoptions.hxx>

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

SidebarTextControl::SidebarTextControl( sw::annotation::SwAnnotationWin& rSidebarWin,
                                      WinBits nBits,
                                      SwView& rDocView,
                                      SwPostItMgr& rPostItMgr )
    : Control( &rSidebarWin, nBits )
    , mrSidebarWin( rSidebarWin )
    , mrDocView( rDocView )
    , mrPostItMgr( rPostItMgr )
    , mbMouseDownGainingFocus(false)
{
    AddEventListener( LINK( &mrSidebarWin, sw::annotation::SwAnnotationWin, WindowEventListener ) );
}

SidebarTextControl::~SidebarTextControl()
{
    disposeOnce();
}

void SidebarTextControl::dispose()
{
    RemoveEventListener( LINK( &mrSidebarWin, sw::annotation::SwAnnotationWin, WindowEventListener ) );
    Control::dispose();
}

OutlinerView* SidebarTextControl::GetTextView() const
{
    return mrSidebarWin.GetOutlinerView();
}

void SidebarTextControl::GetFocus()
{
    Window::GetFocus();
    if ( !mrSidebarWin.IsMouseOver() )
    {
        Invalidate();
    }
}

void SidebarTextControl::LoseFocus()
{
    // write the visible text back into the SwField
    mrSidebarWin.UpdateData();

    Window::LoseFocus();
    if ( !mrSidebarWin.IsMouseOver() )
    {
        Invalidate();
    }
}

void SidebarTextControl::RequestHelp(const HelpEvent &rEvt)
{
    const char* pResId = nullptr;
    switch( mrSidebarWin.GetLayoutStatus() )
    {
        case SwPostItHelper::INSERTED:  pResId = STR_REDLINE_INSERT; break;
        case SwPostItHelper::DELETED:   pResId = STR_REDLINE_DELETE; break;
        default: pResId = nullptr;
    }

    SwContentAtPos aContentAtPos( IsAttrAtPos::Redline );
    if ( pResId &&
         mrDocView.GetWrtShell().GetContentAtPos( mrSidebarWin.GetAnchorPos(), aContentAtPos ) )
    {
        OUString sText = SwResId(pResId) + ": " +
                        aContentAtPos.aFnd.pRedl->GetAuthorString() + " - " +
                        GetAppLangDateTimeString( aContentAtPos.aFnd.pRedl->GetTimeStamp() );
        Help::ShowQuickHelp( this,PixelToLogic(tools::Rectangle(rEvt.GetMousePosPixel(),Size(50,10))),sText);
    }
}

void SidebarTextControl::Draw(OutputDevice* pDev, const Point& rPt, DrawFlags)
{
    //Take the control's height, but overwrite the scrollbar area if there was one
    Size aSize(PixelToLogic(GetSizePixel()));

    if ( GetTextView() )
    {
        GetTextView()->GetOutliner()->Draw(pDev, tools::Rectangle(rPt, aSize));
    }

    if ( mrSidebarWin.GetLayoutStatus()==SwPostItHelper::DELETED )
    {
        pDev->Push(PushFlags::LINECOLOR);

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
}

void SidebarTextControl::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    if (!rRenderContext.GetSettings().GetStyleSettings().GetHighContrastMode())
    {
        if (mrSidebarWin.IsMouseOverSidebarWin() || HasFocus())
        {
            rRenderContext.DrawGradient(tools::Rectangle(Point(0,0), rRenderContext.PixelToLogic(GetSizePixel())),
                                        Gradient(GradientStyle::Linear, mrSidebarWin.ColorDark(), mrSidebarWin.ColorDark()));
        }
        else
        {
            rRenderContext.DrawGradient(tools::Rectangle(Point(0,0), rRenderContext.PixelToLogic(GetSizePixel())),
                           Gradient(GradientStyle::Linear, mrSidebarWin.ColorLight(), mrSidebarWin.ColorDark()));
        }
    }

    if (GetTextView())
    {
        GetTextView()->Paint(rRect, &rRenderContext);
    }

    if (mrSidebarWin.GetLayoutStatus() == SwPostItHelper::DELETED)
    {
        rRenderContext.SetLineColor(mrSidebarWin.GetChangeColor());
        rRenderContext.DrawLine(rRenderContext.PixelToLogic(GetPosPixel()),
                                rRenderContext.PixelToLogic(GetPosPixel() + Point(GetSizePixel().Width(),
                                                                                  GetSizePixel().Height())));
        rRenderContext.DrawLine(rRenderContext.PixelToLogic(GetPosPixel() + Point(GetSizePixel().Width(),
                                                                                  0)),
                                rRenderContext.PixelToLogic(GetPosPixel() + Point(0,
                                                                                  GetSizePixel().Height())));
    }
}

void SidebarTextControl::LogicInvalidate(const tools::Rectangle* pRectangle)
{
    tools::Rectangle aRectangle;

    if (!pRectangle)
    {
        Push(PushFlags::MAPMODE);
        EnableMapMode();
        aRectangle = tools::Rectangle(Point(0, 0), PixelToLogic(GetSizePixel()));
        Pop();
    }
    else
        aRectangle = *pRectangle;

    // Convert from relative twips to absolute ones.
    vcl::Window& rParent = mrSidebarWin.EditWin();
    Point aOffset(GetOutOffXPixel() - rParent.GetOutOffXPixel(), GetOutOffYPixel() - rParent.GetOutOffYPixel());
    rParent.Push(PushFlags::MAPMODE);
    rParent.EnableMapMode();
    aOffset = rParent.PixelToLogic(aOffset);
    rParent.Pop();
    aRectangle.Move(aOffset.getX(), aOffset.getY());

    OString sRectangle = aRectangle.toString();
    SwWrtShell& rWrtShell = mrDocView.GetWrtShell();
    SfxLokHelper::notifyInvalidation(rWrtShell.GetSfxViewShell(), sRectangle);
}

void SidebarTextControl::MakeVisible()
{
    // PostItMgr::MakeVisible can lose our MapMode, save it.
    auto oldMapMode = GetMapMode();
    //let's make sure we see our note
    mrPostItMgr.MakeVisible(&mrSidebarWin);
    if (comphelper::LibreOfficeKit::isActive())
        SetMapMode(oldMapMode);
}

void SidebarTextControl::KeyInput( const KeyEvent& rKeyEvt )
{
    if (getenv("SW_DEBUG") && rKeyEvt.GetKeyCode().GetCode() == KEY_F12)
    {
        if (rKeyEvt.GetKeyCode().IsShift())
        {
            mrDocView.GetDocShell()->GetDoc()->dumpAsXml();
            return;
        }
    }

    const vcl::KeyCode& rKeyCode = rKeyEvt.GetKeyCode();
    sal_uInt16 nKey = rKeyCode.GetCode();
    if ( ( rKeyCode.IsMod1() && rKeyCode.IsMod2() ) &&
         ( (nKey == KEY_PAGEUP) || (nKey == KEY_PAGEDOWN) ) )
    {
        mrSidebarWin.SwitchToPostIt(nKey);
    }
    else if ( nKey == KEY_ESCAPE ||
              ( rKeyCode.IsMod1() &&
                ( nKey == KEY_PAGEUP ||
                  nKey == KEY_PAGEDOWN ) ) )
    {
        mrSidebarWin.SwitchToFieldPos();
    }
    else if ( rKeyCode.GetFullCode() == KEY_INSERT )
    {
        mrSidebarWin.ToggleInsMode();
    }
    else
    {
        MakeVisible();

        tools::Long aOldHeight = mrSidebarWin.GetPostItTextHeight();
        bool bDone = false;

        /// HACK: need to switch off processing of Undo/Redo in Outliner
        if ( !( (nKey == KEY_Z || nKey == KEY_Y) && rKeyCode.IsMod1()) )
        {
            bool bIsProtected = mrSidebarWin.IsProtected();
            if ( !bIsProtected || !EditEngine::DoesKeyChangeText(rKeyEvt) )
            {
                bDone = GetTextView() && GetTextView()->PostKeyEvent( rKeyEvt );
            }
            else
            {
                std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), "modules/swriter/ui/inforeadonlydialog.ui"));
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
            if (!mrDocView.KeyInput(rKeyEvt))
                Window::KeyInput(rKeyEvt);
        }
    }

    mrDocView.GetViewFrame()->GetBindings().InvalidateAll(false);
}

void SidebarTextControl::MouseMove( const MouseEvent& rMEvt )
{
    if ( !GetTextView() )
        return;

    OutlinerView* pOutlinerView( GetTextView() );
    pOutlinerView->MouseMove( rMEvt );
    // mba: why does OutlinerView not handle the modifier setting?!
    // this forces the postit to handle *all* pointer types
    SetPointer( pOutlinerView->GetPointer( rMEvt.GetPosPixel() ) );

    const EditView& aEV = pOutlinerView->GetEditView();
    const SvxFieldItem* pItem = aEV.GetFieldUnderMousePointer();
    if ( pItem )
    {
        const SvxFieldData* pField = pItem->GetField();
        const SvxURLField* pURL = dynamic_cast<const SvxURLField*>( pField  );
        if ( pURL )
        {
            OUString sText(SfxHelp::GetURLHelpText(pURL->GetURL()));
            Help::ShowQuickHelp(
                this, PixelToLogic(tools::Rectangle(GetPosPixel(), Size(50, 10))), sText);
        }
    }
}

void SidebarTextControl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( GetTextView() )
    {
        SvtSecurityOptions aSecOpts;
        bool bExecuteMod = aSecOpts.IsOptionSet( SvtSecurityOptions::EOption::CtrlClickHyperlink);

        if ( !bExecuteMod || (rMEvt.GetModifier() == KEY_MOD1))
        {
            const EditView& aEV = GetTextView()->GetEditView();
            const SvxFieldItem* pItem = aEV.GetFieldUnderMousePointer();
            if ( pItem )
            {
                const SvxFieldData* pField = pItem->GetField();
                const SvxURLField* pURL = dynamic_cast<const SvxURLField*>( pField  );
                if ( pURL )
                {
                    GetTextView()->MouseButtonDown( rMEvt );
                    SwWrtShell &rSh = mrDocView.GetWrtShell();
                    const OUString& sURL( pURL->GetURL() );
                    const OUString& sTarget( pURL->GetTargetFrame() );
                    ::LoadURL(rSh, sURL, LoadUrlFlags::NONE, sTarget);
                    return;
                }
            }
        }
    }

    mbMouseDownGainingFocus = !HasFocus();
    GrabFocus();

    if ( GetTextView() )
    {
        GetTextView()->MouseButtonDown( rMEvt );
    }
    mrDocView.GetViewFrame()->GetBindings().InvalidateAll(false);
}

void SidebarTextControl::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( GetTextView() )
        GetTextView()->MouseButtonUp( rMEvt );
    if (mbMouseDownGainingFocus)
    {
        MakeVisible();
        mbMouseDownGainingFocus = false;
    }
}

IMPL_LINK( SidebarTextControl, OnlineSpellCallback, SpellCallbackInfo&, rInfo, void )
{
    if ( rInfo.nCommand == SpellCallbackCommand::STARTSPELLDLG )
    {
        mrDocView.GetViewFrame()->GetDispatcher()->Execute( FN_SPELL_GRAMMAR_DIALOG, SfxCallMode::ASYNCHRON);
    }
}

void SidebarTextControl::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == CommandEventId::ContextMenu )
    {
        if ( !mrSidebarWin.IsProtected() &&
             GetTextView() &&
             GetTextView()->IsWrongSpelledWordAtPos( rCEvt.GetMousePosPixel(), true ))
        {
            Link<SpellCallbackInfo&,void> aLink = LINK(this, SidebarTextControl, OnlineSpellCallback);
            GetTextView()->ExecuteSpellPopup(rCEvt.GetMousePosPixel(),&aLink);
        }
        else
        {
            Point aPos;
            if (rCEvt.IsMouseEvent())
                aPos = rCEvt.GetMousePosPixel();
            else
            {
                const Size aSize = GetSizePixel();
                aPos = Point( aSize.getWidth()/2, aSize.getHeight()/2 );
            }
            SfxDispatcher::ExecutePopup(this, &aPos);
        }
    }
    else
    if (rCEvt.GetCommand() == CommandEventId::Wheel)
    {
        if (mrSidebarWin.IsScrollbarVisible())
        {
            const CommandWheelData* pData = rCEvt.GetWheelData();
            if (pData->IsShift() || pData->IsMod1() || pData->IsMod2())
            {
                mrDocView.HandleWheelCommands(rCEvt);
            }
            else
            {
                HandleScrollCommand( rCEvt, nullptr , mrSidebarWin.Scrollbar());
            }
        }
        else
        {
            mrDocView.HandleWheelCommands(rCEvt);
        }
    }
    else
    {
        if ( GetTextView() )
            GetTextView()->Command( rCEvt );
        else
            Window::Command(rCEvt);
    }
}

OUString SidebarTextControl::GetSurroundingText() const
{
    if (OutlinerView* pTextView = GetTextView())
        return pTextView->GetSurroundingText();
    return OUString();
}

Selection SidebarTextControl::GetSurroundingTextSelection() const
{
    if (OutlinerView* pTextView = GetTextView())
        return pTextView->GetSurroundingTextSelection();
    return Selection( 0, 0 );
}

bool SidebarTextControl::DeleteSurroundingText(const Selection& rSelection)
{
    if (OutlinerView* pTextView = GetTextView())
        return pTextView->DeleteSurroundingText(rSelection);
    return false;
}

css::uno::Reference< css::accessibility::XAccessible > SidebarTextControl::CreateAccessible()
{

    SidebarTextControlAccessible* pAcc( new SidebarTextControlAccessible( *this ) );
    css::uno::Reference< css::awt::XWindowPeer > xWinPeer( pAcc );
    SetWindowPeer( xWinPeer, pAcc );

    css::uno::Reference< css::accessibility::XAccessible > xAcc( xWinPeer, css::uno::UNO_QUERY );
    return xAcc;
}

} // end of namespace sw::sidebarwindows

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
