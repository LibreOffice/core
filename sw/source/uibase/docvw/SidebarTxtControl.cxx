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

#include <SidebarTxtControl.hxx>

#include <SidebarTxtControlAcc.hxx>
#include <docsh.hxx>
#include <doc.hxx>

#include <SidebarWin.hxx>
#include <PostItMgr.hxx>
#include <edtwin.hxx>

#include <cmdid.h>
#include <docvw.hrc>

#include <unotools/securityoptions.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/mnumgr.hxx>

#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <vcl/layout.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/gradient.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/settings.hxx>

#include <editeng/outliner.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/flditem.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>

#include <uitool.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <shellres.hxx>
#include <SwRewriter.hxx>
#include <memory>

namespace sw { namespace sidebarwindows {

SidebarTextControl::SidebarTextControl( SwSidebarWin& rSidebarWin,
                                      WinBits nBits,
                                      SwView& rDocView,
                                      SwPostItMgr& rPostItMgr )
    : Control( &rSidebarWin, nBits )
    , mrSidebarWin( rSidebarWin )
    , mrDocView( rDocView )
    , mrPostItMgr( rPostItMgr )
{
    AddEventListener( LINK( &mrSidebarWin, SwSidebarWin, WindowEventListener ) );
}

SidebarTextControl::~SidebarTextControl()
{
    disposeOnce();
}

void SidebarTextControl::dispose()
{
    RemoveEventListener( LINK( &mrSidebarWin, SwSidebarWin, WindowEventListener ) );
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
    sal_uInt16 nResId = 0;
    switch( mrSidebarWin.GetLayoutStatus() )
    {
        case SwPostItHelper::INSERTED:  nResId = STR_REDLINE_INSERT; break;
        case SwPostItHelper::DELETED:   nResId = STR_REDLINE_DELETE; break;
        default: nResId = 0;
    }

    SwContentAtPos aContentAtPos( SwContentAtPos::SW_REDLINE );
    if ( nResId &&
         mrDocView.GetWrtShell().GetContentAtPos( mrSidebarWin.GetAnchorPos(), aContentAtPos ) )
    {
        OUString sText = SW_RESSTR( nResId ) + ": " +
                        aContentAtPos.aFnd.pRedl->GetAuthorString() + " - " +
                        GetAppLangDateTimeString( aContentAtPos.aFnd.pRedl->GetTimeStamp() );
        Help::ShowQuickHelp( this,PixelToLogic(Rectangle(rEvt.GetMousePosPixel(),Size(50,10))),sText);
    }
}

void SidebarTextControl::Draw(OutputDevice* pDev, const Point& rPt, const Size& rSz, DrawFlags)
{
    //Take the control's height, but overwrite the scrollbar area if there was one
    Size aSize(PixelToLogic(GetSizePixel()));
    aSize.Width() = rSz.Width();

    if ( GetTextView() )
    {
        GetTextView()->GetOutliner()->Draw(pDev, Rectangle(rPt, aSize));
    }

    if ( mrSidebarWin.GetLayoutStatus()==SwPostItHelper::DELETED )
    {
        SetLineColor(mrSidebarWin.GetChangeColor());
        pDev->DrawLine( PixelToLogic( GetPosPixel(), pDev->GetMapMode() ),
                  PixelToLogic( GetPosPixel() +
                                Point( GetSizePixel().Width(),
                                       GetSizePixel().Height() ), pDev->GetMapMode() ) );
        pDev->DrawLine( PixelToLogic( GetPosPixel() +
                                Point( GetSizePixel().Width(),0), pDev->GetMapMode() ),
                  PixelToLogic( GetPosPixel() +
                                Point( 0, GetSizePixel().Height() ), pDev->GetMapMode() ) );
    }
}

void SidebarTextControl::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    if (!rRenderContext.GetSettings().GetStyleSettings().GetHighContrastMode())
    {
        if (mrSidebarWin.IsMouseOverSidebarWin() || HasFocus())
        {
            rRenderContext.DrawGradient(Rectangle(Point(0,0), rRenderContext.PixelToLogic(GetSizePixel())),
                                        Gradient(GradientStyle_LINEAR, mrSidebarWin.ColorDark(), mrSidebarWin.ColorDark()));
        }
        else
        {
            rRenderContext.DrawGradient(Rectangle(Point(0,0), rRenderContext.PixelToLogic(GetSizePixel())),
                           Gradient(GradientStyle_LINEAR, mrSidebarWin.ColorLight(), mrSidebarWin.ColorDark()));
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

void SidebarTextControl::LogicInvalidate(const Rectangle* pRectangle)
{
    Rectangle aRectangle;

    if (!pRectangle)
    {
        Push(PushFlags::MAPMODE);
        EnableMapMode();
        aRectangle = Rectangle(Point(0, 0), PixelToLogic(GetSizePixel()));
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
    rWrtShell.GetSfxViewShell()->libreOfficeKitViewCallback(LOK_CALLBACK_INVALIDATE_TILES, sRectangle.getStr());
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
    else if ( nKey == KEY_INSERT )
    {
        if ( !rKeyCode.IsMod1() && !rKeyCode.IsMod2() )
        {
            mrSidebarWin.ToggleInsMode();
        }
    }
    else
    {
        // MakeVisible can lose our MapMode, save it.
        auto oldMapMode = GetMapMode();
        //let's make sure we see our note
        mrPostItMgr.MakeVisible(&mrSidebarWin);
        if (comphelper::LibreOfficeKit::isActive())
            SetMapMode(oldMapMode);

        long aOldHeight = mrSidebarWin.GetPostItTextHeight();
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
                ScopedVclPtrInstance<MessageDialog>::Create(this, "InfoReadonlyDialog",
                    "modules/swriter/ui/inforeadonlydialog.ui")->Execute();
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
    if ( GetTextView() )
    {
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
                OUString sURL( pURL->GetURL() );
                SvtSecurityOptions aSecOpts;
                if ( aSecOpts.IsOptionSet( SvtSecurityOptions::E_CTRLCLICK_HYPERLINK) )
                    sURL = SwViewShell::GetShellRes()->aLinkCtrlClick + ": " + sURL;
                else
                    sURL = SwViewShell::GetShellRes()->aLinkClick + ": " + sURL;
                Help::ShowQuickHelp( this,PixelToLogic(Rectangle(GetPosPixel(),Size(50,10))),sURL);
            }
        }
    }
}

void SidebarTextControl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( GetTextView() )
    {
        SvtSecurityOptions aSecOpts;
        bool bExecuteMod = aSecOpts.IsOptionSet( SvtSecurityOptions::E_CTRLCLICK_HYPERLINK);

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
                    OUString sURL( pURL->GetURL() );
                    OUString sTarget( pURL->GetTargetFrame() );
                    ::LoadURL(rSh, sURL, URLLOAD_NOFILTER, sTarget);
                    return;
                }
            }
        }
    }

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
}

IMPL_LINK_TYPED( SidebarTextControl, OnlineSpellCallback, SpellCallbackInfo&, rInfo, void )
{
    if ( rInfo.nCommand == SpellCallbackCommand::STARTSPELLDLG )
    {
        mrDocView.GetViewFrame()->GetDispatcher()->Execute( FN_SPELL_GRAMMAR_DIALOG, SfxCallMode::ASYNCHRON);
    }
}

IMPL_LINK_TYPED( SidebarTextControl, Select, Menu*, pSelMenu, bool )
{
    mrSidebarWin.ExecuteCommand( pSelMenu->GetCurItemId() );
    return false;
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
            std::unique_ptr<SfxPopupMenuManager> pMgr(SfxDispatcher::Popup(0, this,&rCEvt.GetMousePosPixel()));
            static_cast<PopupMenu*>(pMgr->GetSVMenu())->SetSelectHdl( LINK(this, SidebarTextControl, Select) );

            {
                OUString aText = static_cast<PopupMenu*>(pMgr->GetSVMenu())->GetItemText( FN_DELETE_NOTE_AUTHOR );
                SwRewriter aRewriter;
                aRewriter.AddRule(UndoArg1, mrSidebarWin.GetAuthor());
                aText = aRewriter.Apply(aText);
                static_cast<PopupMenu*>(pMgr->GetSVMenu())->SetItemText(FN_DELETE_NOTE_AUTHOR,aText);
            }

            Point aPos;
            if (rCEvt.IsMouseEvent())
                aPos = rCEvt.GetMousePosPixel();
            else
            {
                const Size aSize = GetSizePixel();
                aPos = Point( aSize.getWidth()/2, aSize.getHeight()/2 );
            }

            //!! call different Execute function to get rid of the new thesaurus sub menu
            //!! pointer created in the call to Popup.
            //!! Otherwise we would have a memory leak (see also #i107205#)
            pMgr->Execute( aPos, this );
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
    if (GetTextView())
        return GetTextView()->GetSurroundingText();
    return OUString();
}

Selection SidebarTextControl::GetSurroundingTextSelection() const
{
    if( GetTextView() )
        return GetTextView()->GetSurroundingTextSelection();
    else
        return Selection( 0, 0 );
}

css::uno::Reference< css::accessibility::XAccessible > SidebarTextControl::CreateAccessible()
{

    SidebarTextControlAccessible* pAcc( new SidebarTextControlAccessible( *this ) );
    css::uno::Reference< css::awt::XWindowPeer > xWinPeer( pAcc );
    SetWindowPeer( xWinPeer, pAcc );

    css::uno::Reference< css::accessibility::XAccessible > xAcc( xWinPeer, css::uno::UNO_QUERY );
    return xAcc;
}

} } // end of namespace sw::sidebarwindows

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
