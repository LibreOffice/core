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

#include "OutlineViewShell.hxx"

#include "ViewShellImplementation.hxx"
#include "helpids.h"
#include "app.hrc"
#include <svx/hyperdlg.hxx>
#include <svx/zoomslideritem.hxx>

#include <sfx2/infobar.hxx>
#include <sfx2/objface.hxx>
#include <sot/exchange.hxx>
#include <svx/ruler.hxx>
#include <sfx2/zoomitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <sfx2/shell.hxx>
#include <sfx2/templdlg.hxx>
#include <sfx2/viewfac.hxx>
#include <sfx2/request.hxx>
#include <svx/hlnkitem.hxx>
#include <svx/svdotext.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/settings.hxx>

#include <svl/whiter.hxx>
#include <editeng/editstat.hxx>
#include <svl/itempool.hxx>
#include <sfx2/tplpitem.hxx>
#include <sfx2/sidebar/SidebarChildWindow.hxx>
#include <sfx2/sidebar/EnumContext.hxx>
#include <svx/svdorect.hxx>
#include <sot/formats.hxx>
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/i18n/TransliterationModulesExtra.hpp>
#include <editeng/unolingu.hxx>
#include <comphelper/processfactory.hxx>
#include <editeng/outlobj.hxx>
#include <svl/cjkoptions.hxx>
#include <svtools/cliplistener.hxx>
#include <svl/srchitem.hxx>
#include <editeng/editobj.hxx>
#include "fubullet.hxx"
#include "optsitem.hxx"

#include "strings.hrc"
#include "glob.hrc"
#include "res_bmp.hrc"
#include "Outliner.hxx"
#include "Window.hxx"
#include "TextObjectBar.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "sdpage.hxx"
#include "fuoltext.hxx"
#include "FrameView.hxx"
#include "zoomlist.hxx"
#include "stlsheet.hxx"
#include "slideshow.hxx"
#include "SdUnoOutlineView.hxx"
#include "SpellDialogChildWindow.hxx"

#include "AccessibleOutlineView.hxx"
#include "ViewShellBase.hxx"
#include "ViewShellManager.hxx"
#include "DrawController.hxx"
#include "framework/FrameworkHelper.hxx"

#include <boost/scoped_ptr.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

using namespace sd;

#define OutlineViewShell
#include "sdslots.hxx"

namespace sd {

#define MIN_ZOOM           10      // minimum zoom factor
#define MAX_ZOOM         1000      // maximum zoom factor

/**
 * Declare SFX-Slotmap and standard interface
 */
SFX_IMPL_INTERFACE(OutlineViewShell, SfxShell)

void OutlineViewShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterPopupMenu(SdResId(RID_OUTLINE_POPUP));

    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_TOOLS | SFX_VISIBILITY_STANDARD | SFX_VISIBILITY_FULLSCREEN | SFX_VISIBILITY_SERVER,
                                            RID_OUTLINE_TOOLBOX);
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_APPLICATION | SFX_VISIBILITY_DESKTOP | SFX_VISIBILITY_STANDARD | SFX_VISIBILITY_CLIENT | SFX_VISIBILITY_VIEWER | SFX_VISIBILITY_READONLYDOC,
                                            RID_DRAW_VIEWER_TOOLBOX);

    GetStaticInterface()->RegisterChildWindow(SfxInfoBarContainerChild::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(SvxHlinkDlgWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(::sd::SpellDialogChildWindow::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(SID_SEARCH_DLG);
    GetStaticInterface()->RegisterChildWindow(::sfx2::sidebar::SidebarChildWindow::GetChildWindowId());
}

TYPEINIT1( OutlineViewShell, ViewShell );

/**
 * common initialization part of both constructors
 */
void OutlineViewShell::Construct(DrawDocShell* )
{
    bool bModified = GetDoc()->IsChanged();

    meShellType = ST_OUTLINE;
    Size aSize(29700, 21000);
    Point aWinPos (0, 0);
    Point aViewOrigin(0, 0);
    GetActiveWindow()->SetMinZoomAutoCalc(false);
    GetActiveWindow()->SetMinZoom( MIN_ZOOM );
    GetActiveWindow()->SetMaxZoom( MAX_ZOOM );
    InitWindows(aViewOrigin, aSize, aWinPos);
    pOlView = new OutlineView(*GetDocSh(), GetActiveWindow(), *this);
    mpView = pOlView;            // Pointer of base class ViewShell

    SetPool( &GetDoc()->GetPool() );

    SetZoom(69);

    // Apply settings of FrameView
    ReadFrameViewData(mpFrameView);

    ::Outliner& rOutl = pOlView->GetOutliner();
    rOutl.SetUpdateMode(true);

    if (!bModified)
    {
        rOutl.ClearModifyFlag();
    }

    pLastPage = GetActualPage();

    SetName( OUString( "OutlineViewShell" ) );

    SetHelpId( SD_IF_SDOUTLINEVIEWSHELL );
    GetActiveWindow()->SetHelpId( HID_SDOUTLINEVIEWSHELL );
    GetActiveWindow()->SetUniqueId( HID_SDOUTLINEVIEWSHELL );
}

Reference<drawing::XDrawSubController> OutlineViewShell::CreateSubController()
{
    Reference<drawing::XDrawSubController> xSubController;

    if (IsMainViewShell())
    {
        // Create uno sub controller for the main view shell.
        xSubController = Reference<drawing::XDrawSubController>(
            new SdUnoOutlineView (
                *this));
    }

    return xSubController;
}

/**
 * Default constructor, windows must not center themselves automatically
 */
OutlineViewShell::OutlineViewShell (
    SfxViewFrame* pFrame,
    ViewShellBase& rViewShellBase,
    vcl::Window* pParentWindow,
    FrameView* pFrameViewArgument)
    : ViewShell(pFrame, pParentWindow, rViewShellBase),
      pOlView(NULL),
      pLastPage( NULL ),
      pClipEvtLstnr(NULL),
      bPastePossible(false),
      mbInitialized(false)

{
    if (pFrameViewArgument != NULL)
        mpFrameView = pFrameViewArgument;
    else
        mpFrameView = new FrameView(GetDoc());

    mpFrameView->Connect();

    Construct(GetDocSh());

    SetContextName(sfx2::sidebar::EnumContext::GetContextName(sfx2::sidebar::EnumContext::Context_OutlineText));

    m_StrOldPageName.clear();

    doShow();
}

OutlineViewShell::~OutlineViewShell()
{
    DisposeFunctions();

    delete pOlView;

    mpFrameView->Disconnect();

    if ( pClipEvtLstnr )
    {
        pClipEvtLstnr->AddRemoveListener( GetActiveWindow(), false );
        pClipEvtLstnr->ClearCallbackLink();     // prevent callback if another thread is waiting
        pClipEvtLstnr->release();
    }
}

void OutlineViewShell::Shutdown()
{
    ViewShell::Shutdown();

    PrepareClose();
}

/**
 * Paint method: the event gets forwarded from pWindow to the Viewshell
 * and the current function
 */
void OutlineViewShell::Paint(const Rectangle& rRect, ::sd::Window* pWin)
{
    if (pOlView)
    {
        pOlView->Paint(rRect, pWin);
    }
}

void OutlineViewShell::ArrangeGUIElements ()
{
    // Retrieve the current size (thickness) of the scroll bars.  That is
    // the width of the vertical and the height of the horizontal scroll
    // bar.
    int nScrollBarSize =
        GetParentWindow()->GetSettings().GetStyleSettings().GetScrollBarSize();
    maScrBarWH = Size (nScrollBarSize, nScrollBarSize);

    ViewShell::ArrangeGUIElements ();

    ::sd::Window* pWindow = mpContentWindow.get();
    if (pWindow != NULL)
    {
        pWindow->SetMinZoomAutoCalc(false);

        // change OuputArea of the OutlinerView
        OutlinerView* pOutlinerView = pOlView->GetViewByWindow(pWindow);

        Rectangle aWin(Point(0,0), pWindow->GetOutputSizePixel());

        aWin = pWindow->PixelToLogic(aWin);
        pOutlinerView->SetOutputArea(aWin);

        Rectangle aVis = pOutlinerView->GetVisArea();

        Rectangle aText = Rectangle(Point(0,0),
            Size(pOlView->GetPaperWidth(),
                pOlView->GetOutliner().GetTextHeight()));
        if (aWin.GetHeight() > aText.Bottom())
            aText.Bottom() = aWin.GetHeight();

        if (!aWin.IsEmpty())            // not when opening
        {
            InitWindows(Point(0,0), aText.GetSize(), Point(aVis.TopLeft()));
            UpdateScrollBars();
        }
    }
}

/**
 * Handle SfxRequest for the Controller
 */
void OutlineViewShell::ExecCtrl(SfxRequest &rReq)
{
    sal_uInt16 nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
        case SID_MAIL_SCROLLBODY_PAGEDOWN:
        {
            ExecReq( rReq );
            break;
        }

        case SID_OPT_LOCALE_CHANGED:
        {
            pOlView->GetOutliner().UpdateFields();
            UpdatePreview( GetActualPage() );
            rReq.Done();
            break;
        }

        default:
        break;
    }
}

/**
 * Activate(): during the first invocation the fields get updated
 */
void OutlineViewShell::Activate( bool bIsMDIActivate )
{
    if ( ! mbInitialized)
    {
        mbInitialized = true;
        SfxRequest aRequest (SID_EDIT_OUTLINER, SfxCallMode::SLOT, GetDoc()->GetItemPool());
        FuPermanent (aRequest);
    }

    ViewShell::Activate( bIsMDIActivate );
    SfxShell::BroadcastContextForActivation(true);

    pOlView->SetLinks();
    pOlView->ConnectToApplication();

    if( bIsMDIActivate )
    {
        OutlinerView* pOutlinerView = pOlView->GetViewByWindow( GetActiveWindow() );
        ::Outliner* pOutl = pOutlinerView->GetOutliner();
        pOutl->UpdateFields();
    }
}

void OutlineViewShell::Deactivate( bool bIsMDIActivate )
{
    pOlView->DisconnectFromApplication();

    // Links must be kept also on deactivated viewshell, to allow drag'n'drop
    // to function properly
    ViewShell::Deactivate( bIsMDIActivate );
}

/**
 * Set status of Controller-SfxSlots
 */
void OutlineViewShell::GetCtrlState(SfxItemSet &rSet)
{
    if (SfxItemState::DEFAULT == rSet.GetItemState(SID_HYPERLINK_GETLINK))
    {
        SvxHyperlinkItem aHLinkItem;

        OutlinerView* pOLV = pOlView->GetViewByWindow(GetActiveWindow());
        if (pOLV)
        {
            const SvxFieldItem* pFieldItem = pOLV->GetFieldAtSelection();
            if (pFieldItem)
            {
                ESelection aSel = pOLV->GetSelection();
                if ( abs( aSel.nEndPos - aSel.nStartPos ) == 1 )
                {
                    const SvxFieldData* pField = pFieldItem->GetField();
                    if ( pField->ISA(SvxURLField) )
                    {
                        aHLinkItem.SetName(static_cast<const SvxURLField*>(pField)->GetRepresentation());
                        aHLinkItem.SetURL(static_cast<const SvxURLField*>(pField)->GetURL());
                        aHLinkItem.SetTargetFrame(static_cast<const SvxURLField*>(pField)->GetTargetFrame());
                    }
                }
            }
        }
        rSet.Put(aHLinkItem);
    }
    rSet.Put( SfxBoolItem( SID_READONLY_MODE, GetDocSh()->IsReadOnly() ) );

    if ( SfxItemState::DEFAULT == rSet.GetItemState(SID_MAIL_SCROLLBODY_PAGEDOWN) )
        rSet.Put( SfxBoolItem( SID_MAIL_SCROLLBODY_PAGEDOWN, true ) );

    if ( SfxItemState::DEFAULT == rSet.GetItemState(SID_TRANSLITERATE_HALFWIDTH) ||
         SfxItemState::DEFAULT == rSet.GetItemState(SID_TRANSLITERATE_FULLWIDTH) ||
         SfxItemState::DEFAULT == rSet.GetItemState(SID_TRANSLITERATE_HIRAGANA) ||
         SfxItemState::DEFAULT == rSet.GetItemState(SID_TRANSLITERATE_KATAGANA) )
    {
        SvtCJKOptions aCJKOptions;
        if( !aCJKOptions.IsChangeCaseMapEnabled() )
        {
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_HALFWIDTH, false );
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_FULLWIDTH, false );
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_HIRAGANA, false );
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_KATAGANA, false );
            rSet.DisableItem( SID_TRANSLITERATE_HALFWIDTH );
            rSet.DisableItem( SID_TRANSLITERATE_FULLWIDTH );
            rSet.DisableItem( SID_TRANSLITERATE_HIRAGANA );
            rSet.DisableItem( SID_TRANSLITERATE_KATAGANA );
        }
        else
        {
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_HALFWIDTH, true );
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_FULLWIDTH, true );
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_HIRAGANA, true );
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_KATAGANA, true );
        }
    }
}

/**
 * SfxRequests for support functions
 */
void OutlineViewShell::FuSupport(SfxRequest &rReq)
{
    if( rReq.GetSlot() == SID_STYLE_FAMILY && rReq.GetArgs())
        GetDocSh()->SetStyleFamily(static_cast<const SfxUInt16Item&>(rReq.GetArgs()->Get( SID_STYLE_FAMILY )).GetValue());

    bool bPreviewState = false;
    sal_uLong nSlot = rReq.GetSlot();

    std::unique_ptr< OutlineViewModelChangeGuard > aGuard;
    if( pOlView && (
        (nSlot == SID_TRANSLITERATE_SENTENCE_CASE) ||
        (nSlot == SID_TRANSLITERATE_TITLE_CASE) ||
        (nSlot == SID_TRANSLITERATE_TOGGLE_CASE) ||
        (nSlot == SID_TRANSLITERATE_UPPER) ||
        (nSlot == SID_TRANSLITERATE_LOWER) ||
        (nSlot == SID_TRANSLITERATE_HALFWIDTH) ||
        (nSlot == SID_TRANSLITERATE_FULLWIDTH) ||
        (nSlot == SID_TRANSLITERATE_HIRAGANA) ||
        (nSlot == SID_TRANSLITERATE_KATAGANA) ||
        (nSlot == SID_CUT) ||
        (nSlot == SID_PASTE) ||
        (nSlot == SID_PASTE_UNFORMATTED) ||
        (nSlot == SID_DELETE)))
    {
        aGuard.reset( new OutlineViewModelChangeGuard( *pOlView ) );
    }

    switch ( nSlot )
    {
        case SID_CUT:
        {
            if(HasCurrentFunction())
            {
                GetCurrentFunction()->DoCut();
            }
            else if (pOlView)
            {
                pOlView->DoCut();
            }
            rReq.Done();
            bPreviewState = true;
        }
        break;

        case SID_COPY:
        {
            if(HasCurrentFunction())
            {
                GetCurrentFunction()->DoCopy();
            }
            else if (pOlView)
            {
                pOlView->DoCopy();
            }
            rReq.Done();
            bPreviewState = true;
        }
        break;

        case SID_PASTE:
        {
            OutlineViewPageChangesGuard aGuard2(pOlView);

            if(HasCurrentFunction())
            {
                GetCurrentFunction()->DoPaste();
            }
            else if (pOlView)
            {
                pOlView->DoPaste();
            }
            rReq.Done();
            bPreviewState = true;
        }
        break;

        case SID_PASTE_UNFORMATTED:
        {
            OutlineViewPageChangesGuard aGuard2(pOlView);

            if(HasCurrentFunction())
            {
                GetCurrentFunction()->DoPasteUnformatted();
            }
            else if(pOlView)
            {
                sal_Int8 nAction = DND_ACTION_COPY;
                TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( GetActiveWindow() ) );
                if (aDataHelper.GetTransferable().is())
                {
                    pOlView->InsertData( aDataHelper,
                                         GetActiveWindow()->PixelToLogic( Rectangle( Point(), GetActiveWindow()->GetOutputSizePixel() ).Center() ),
                                         nAction, false, SotClipboardFormatId::STRING);
                }
            }

            rReq.Ignore ();
        }
        break;
        case SID_DELETE:
        {
            if( pOlView )
            {
                OutlinerView* pOutlView = pOlView->GetViewByWindow(GetActiveWindow());
                if (pOutlView)
                {
                    OutlineViewPageChangesGuard aGuard2(pOlView);

                    vcl::KeyCode aKCode(KEY_DELETE);
                    KeyEvent aKEvt( 0, aKCode );
                    pOutlView->PostKeyEvent(aKEvt);

                    rtl::Reference<FuPoor> xFunc( GetCurrentFunction() );
                    FuOutlineText* pFuOutlineText = dynamic_cast< FuOutlineText* >( xFunc.get() );
                    if( pFuOutlineText )
                        pFuOutlineText->UpdateForKeyPress (aKEvt);
                }
            }
            rReq.Done();
            bPreviewState = true;
        }
        break;

        case SID_DRAWINGMODE:
        case SID_NOTESMODE:
        case SID_HANDOUTMODE:
        case SID_DIAMODE:
        case SID_OUTLINEMODE:
            framework::FrameworkHelper::Instance(GetViewShellBase())->HandleModeChangeSlot(
                nSlot,
                rReq);
            rReq.Done();
            break;

        case SID_RULER:
            SetRuler( !HasRuler() );
            Invalidate( SID_RULER );
            rReq.Done();
        break;

        case SID_ZOOM_PREV:
        {
            if (mpZoomList->IsPreviousPossible())
            {
                SetZoomRect(mpZoomList->GetPreviousZoomRect());
            }
            rReq.Done ();
        }
        break;

        case SID_ZOOM_NEXT:
        {
            if (mpZoomList->IsNextPossible())
            {
                SetZoomRect(mpZoomList->GetNextZoomRect());
            }
            rReq.Done ();
        }
        break;

        case SID_AUTOSPELL_CHECK:
        {
            GetDoc()->SetOnlineSpell(!GetDoc()->GetOnlineSpell());
            rReq.Done ();
        }
        break;

        case SID_TRANSLITERATE_SENTENCE_CASE:
        case SID_TRANSLITERATE_TITLE_CASE:
        case SID_TRANSLITERATE_TOGGLE_CASE:
        case SID_TRANSLITERATE_UPPER:
        case SID_TRANSLITERATE_LOWER:
        case SID_TRANSLITERATE_HALFWIDTH:
        case SID_TRANSLITERATE_FULLWIDTH:
        case SID_TRANSLITERATE_HIRAGANA:
        case SID_TRANSLITERATE_KATAGANA:
        {
            OutlinerView* pOLV = pOlView ? pOlView->GetViewByWindow( GetActiveWindow() ) : 0;
            if( pOLV )
            {
                using namespace ::com::sun::star::i18n;
                sal_Int32 nType = 0;

                switch( nSlot )
                {
                    case SID_TRANSLITERATE_SENTENCE_CASE:
                        nType = TransliterationModulesExtra::SENTENCE_CASE;
                        break;
                    case SID_TRANSLITERATE_TITLE_CASE:
                        nType = TransliterationModulesExtra::TITLE_CASE;
                        break;
                    case SID_TRANSLITERATE_TOGGLE_CASE:
                        nType = TransliterationModulesExtra::TOGGLE_CASE;
                        break;
                    case SID_TRANSLITERATE_UPPER:
                        nType = TransliterationModules_LOWERCASE_UPPERCASE;
                        break;
                    case SID_TRANSLITERATE_LOWER:
                        nType = TransliterationModules_UPPERCASE_LOWERCASE;
                        break;
                    case SID_TRANSLITERATE_HALFWIDTH:
                        nType = TransliterationModules_FULLWIDTH_HALFWIDTH;
                        break;
                    case SID_TRANSLITERATE_FULLWIDTH:
                        nType = TransliterationModules_HALFWIDTH_FULLWIDTH;
                        break;
                    case SID_TRANSLITERATE_HIRAGANA:
                        nType = TransliterationModules_KATAKANA_HIRAGANA;
                        break;
                    case SID_TRANSLITERATE_KATAGANA:
                        nType = TransliterationModules_HIRAGANA_KATAKANA;
                        break;
                }

                pOLV->TransliterateText( nType );
            }

            rReq.Done();
            bPreviewState = true;
        }
        break;

        // added Undo/Redo handling
        case SID_UNDO :
        {
            OutlineViewPageChangesGuard aGuard2(pOlView);
            ImpSidUndo(false, rReq);
        }
        break;
        case SID_REDO :
        {
            OutlineViewPageChangesGuard aGuard2(pOlView);
            ImpSidRedo(false, rReq);
        }
        break;

        default:
        break;
    }

    if( bPreviewState )
        Invalidate( SID_PREVIEW_STATE );

    Invalidate(SID_CUT);
    Invalidate(SID_COPY);
    Invalidate(SID_PASTE);
}

/**
 * SfxRequests for permanent functions
 */
void OutlineViewShell::FuPermanent(SfxRequest &rReq)
{
    if(HasCurrentFunction())
    {
        DeactivateCurrentFunction(true);
    }

    switch ( rReq.GetSlot() )
    {
        case SID_EDIT_OUTLINER:
        {
            ::Outliner& rOutl = pOlView->GetOutliner();
            rOutl.GetUndoManager().Clear();
            rOutl.UpdateFields();

            SetCurrentFunction( FuOutlineText::Create(this,GetActiveWindow(),pOlView,GetDoc(),rReq) );

            rReq.Done();
        }
        break;

      default:
      break;
    }

    if(HasOldFunction())
    {
        GetOldFunction()->Deactivate();
        SetOldFunction(0);
    }

    if(HasCurrentFunction())
    {
        GetCurrentFunction()->Activate();
        SetOldFunction(GetCurrentFunction());
    }
}

IMPL_LINK_TYPED( OutlineViewShell, ClipboardChanged, TransferableDataHelper*, pDataHelper, void )
{
    bPastePossible = pDataHelper->GetFormatCount() != 0 &&
                     ( pDataHelper->HasFormat( SotClipboardFormatId::STRING ) ||
                       pDataHelper->HasFormat( SotClipboardFormatId::RTF ) ||
                       pDataHelper->HasFormat( SotClipboardFormatId::HTML ) );

    SfxBindings& rBindings = GetViewFrame()->GetBindings();
    rBindings.Invalidate( SID_PASTE );
    rBindings.Invalidate( SID_PASTE_SPECIAL );
    rBindings.Invalidate( SID_PASTE_UNFORMATTED );
    rBindings.Invalidate( SID_CLIPBOARD_FORMAT_ITEMS );
}

/**
 * Set Status (Enabled/Disabled) of Menu-SfxSlots
 */
void OutlineViewShell::GetMenuState( SfxItemSet &rSet )
{
    ViewShell::GetMenuState(rSet);

    rSet.Put(SfxBoolItem(SID_DIAMODE, false));
    rSet.Put(SfxBoolItem(SID_DRAWINGMODE, false));
    rSet.Put(SfxBoolItem(SID_OUTLINEMODE, true));
    rSet.Put(SfxBoolItem(SID_NOTESMODE, false));
    rSet.Put(SfxBoolItem(SID_HANDOUTMODE, false));

    if (!mpZoomList->IsNextPossible())
    {
       rSet.DisableItem(SID_ZOOM_NEXT);
    }
    if (!mpZoomList->IsPreviousPossible())
    {
       rSet.DisableItem(SID_ZOOM_PREV);
    }

    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_ZOOM_IN ) ||
        SfxItemState::DEFAULT == rSet.GetItemState( SID_ZOOM_OUT ) )
    {
        if( GetActiveWindow()->GetZoom() <= GetActiveWindow()->GetMinZoom() || GetDocSh()->IsUIActive() )
            rSet.DisableItem( SID_ZOOM_IN );
        if( GetActiveWindow()->GetZoom() >= GetActiveWindow()->GetMaxZoom() || GetDocSh()->IsUIActive() )
            rSet.DisableItem( SID_ZOOM_OUT );
    }

    ::Outliner& rOutl = pOlView->GetOutliner();

    // allow 'Select All'?
    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_SELECTALL ) )
    {
        sal_Int32 nParaCount = rOutl.GetParagraphCount();
        bool bDisable = nParaCount == 0;
        if (!bDisable && nParaCount == 1)
        {
            OUString aTest = rOutl.GetText(rOutl.GetParagraph(0));
            if (aTest.isEmpty())
            {
                bDisable = true;
            }
        }
        if (bDisable)
            rSet.DisableItem(SID_SELECTALL);
    }

    // set status of Ruler
    rSet.Put( SfxBoolItem( SID_RULER, HasRuler() ) );

    // Enable formatting?
    rSet.Put( SfxBoolItem( SID_OUTLINE_FORMAT, !rOutl.IsFlatMode() ) );

    if( rOutl.IsFlatMode() )
        rSet.DisableItem( SID_COLORVIEW );
    else
    {
        // Enable color view?
        EEControlBits nCntrl = rOutl.GetControlWord();
        bool bNoColor = false;
        if (nCntrl & EEControlBits::NOCOLORS)
            bNoColor = true;

        rSet.Put( SfxBoolItem( SID_COLORVIEW, bNoColor ) );
    }

    // Buttons of toolbar
    // first the selection dependent ones: COLLAPSE, EXPAND
    bool bDisableCollapse = true;
    bool bDisableExpand   = true;
    bool bUnique          = true;
    OutlinerView* pOutlinerView = pOlView->GetViewByWindow(GetActiveWindow());

    std::vector<Paragraph*> aSelList;
    pOutlinerView->CreateSelectionList(aSelList);

    if (!aSelList.empty())
    {
        std::vector<Paragraph*>::const_iterator iter = aSelList.begin();
        Paragraph* pPara = *iter;

        sal_Int16 nDepth;
        sal_Int16 nTmpDepth = rOutl.GetDepth( rOutl.GetAbsPos( pPara ) );
        bool bPage = ::Outliner::HasParaFlag( pPara, ParaFlag::ISPAGE );

        while (iter != aSelList.begin())
        {
            pPara = *iter;

            nDepth = rOutl.GetDepth( rOutl.GetAbsPos( pPara ) );

            if( nDepth != nTmpDepth || bPage != ::Outliner::HasParaFlag( pPara, ParaFlag::ISPAGE ))
                bUnique = false;

            if (rOutl.HasChildren(pPara))
            {
                if (!rOutl.IsExpanded(pPara))
                    bDisableExpand = false;
                else
                    bDisableCollapse = false;
            }

            ++iter;
        }
    }

    if (bDisableExpand)
        rSet.DisableItem(SID_OUTLINE_EXPAND);
    if (bDisableCollapse)
        rSet.DisableItem(SID_OUTLINE_COLLAPSE);

    // does the selection provide a unique presentation layout?
    // if not, the templates must not be edited
    SfxItemSet aSet(*rSet.GetPool(), SID_STATUS_LAYOUT, SID_STATUS_LAYOUT);
    GetStatusBarState(aSet);
    OUString aTest = static_cast<const SfxStringItem&>(aSet.Get(SID_STATUS_LAYOUT)).GetValue();
    if (aTest.isEmpty())
    {
        bUnique = false;
        rSet.DisableItem(SID_PRESENTATION_TEMPLATES);
    }

    if (!bUnique)
        rSet.DisableItem( SID_PRESENTATIONOBJECT );

    // now the selection independent ones: COLLAPSE_ALL, EXPAND_ALL
    bool bDisableCollapseAll = true;
    bool bDisableExpandAll   = true;

    // does the selection contain something collapsable/expandable?
    if (!bDisableCollapse)
        bDisableCollapseAll = false;
    if (!bDisableExpand)
        bDisableExpandAll = false;

    // otherwise look through all paragraphs
    if (bDisableCollapseAll || bDisableExpandAll)
    {
        sal_Int32 nParaPos = 0;
        Paragraph* pPara = rOutl.GetParagraph( nParaPos );
        while (pPara && (bDisableCollapseAll || bDisableExpandAll))
        {
            if (!rOutl.IsExpanded(pPara) && rOutl.HasChildren(pPara))
                bDisableExpandAll = false;

            if (rOutl.IsExpanded(pPara) && rOutl.HasChildren(pPara))
                bDisableCollapseAll = false;

            pPara = rOutl.GetParagraph( ++nParaPos );
        }
    }

    if (bDisableExpandAll)
        rSet.DisableItem(SID_OUTLINE_EXPAND_ALL);
    if (bDisableCollapseAll)
        rSet.DisableItem(SID_OUTLINE_COLLAPSE_ALL);

    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_PASTE ) )
    {
        if ( !pClipEvtLstnr )
        {
            // create listener
            pClipEvtLstnr = new TransferableClipboardListener( LINK( this, OutlineViewShell, ClipboardChanged ) );
            pClipEvtLstnr->acquire();
            pClipEvtLstnr->AddRemoveListener( GetActiveWindow(), true );

            // get initial state
            TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( GetActiveWindow() ) );
            bPastePossible = ( aDataHelper.GetFormatCount() != 0 &&
                                ( aDataHelper.HasFormat( SotClipboardFormatId::STRING ) ||
                                  aDataHelper.HasFormat( SotClipboardFormatId::RTF ) ||
                                  aDataHelper.HasFormat( SotClipboardFormatId::HTML ) ) );
        }

        if( !bPastePossible )
        {
            rSet.DisableItem( SID_PASTE );
        }
    }

    if (!pOlView->GetViewByWindow(GetActiveWindow())->HasSelection())
    {
        rSet.DisableItem(SID_CUT);
        rSet.DisableItem(SID_COPY);
    }

    if (pOlView->GetOutliner().IsModified())
    {
        GetDoc()->SetChanged();
    }

    // the status has to be set here because of overriding
    if( !GetDocSh()->IsModified() )
    {
        rSet.DisableItem( SID_SAVEDOC );
    }

    if ( GetDocSh()->IsReadOnly() )
    {
        rSet.DisableItem( SID_AUTOSPELL_CHECK );
    }
    else
    {
        if (GetDoc()->GetOnlineSpell())
        {
            rSet.Put(SfxBoolItem(SID_AUTOSPELL_CHECK, true));
        }
        else
        {
            rSet.Put(SfxBoolItem(SID_AUTOSPELL_CHECK, false));
        }
    }

    // field commands
    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_MODIFY_FIELD ) )
    {
        const SvxFieldItem* pFldItem = pOutlinerView->GetFieldAtSelection();

        if( !( pFldItem && (pFldItem->GetField()->ISA( SvxDateField ) ||
                            pFldItem->GetField()->ISA( SvxAuthorField ) ||
                            pFldItem->GetField()->ISA( SvxExtFileField ) ||
                            pFldItem->GetField()->ISA( SvxExtTimeField ) ) ) )
        {
            rSet.DisableItem( SID_MODIFY_FIELD );
        }
    }

    if (SfxItemState::DEFAULT == rSet.GetItemState(SID_EXPAND_PAGE))
    {
        bool bDisable = true;
        sal_uInt16 i = 0;
        sal_uInt16 nCount = GetDoc()->GetSdPageCount(PK_STANDARD);
        pOlView->SetSelectedPages();

        while (i < nCount && bDisable)
        {
            SdPage* pPage = GetDoc()->GetSdPage(i, PK_STANDARD);

            if (pPage->IsSelected())
            {
                SdrObject* pObj = pPage->GetPresObj(PRESOBJ_OUTLINE);

                if (pObj!=NULL )
                {
                    if( !pObj->IsEmptyPresObj() )
                    {
                        bDisable = false;
                    }
                    else
                    {
                        // check if the object is in edit, than its temporarely not empty
                        SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( pObj );
                        if( pTextObj )
                        {
                            OutlinerParaObject* pParaObj = pTextObj->GetEditOutlinerParaObject();
                            if( pParaObj )
                            {
                                delete pParaObj;
                                bDisable = false;
                            }
                        }
                    }
                }
            }

            i++;
        }

        if (bDisable)
        {
            rSet.DisableItem(SID_EXPAND_PAGE);
        }
    }

    if (SfxItemState::DEFAULT == rSet.GetItemState(SID_SUMMARY_PAGE))
    {
        bool bDisable = true;
        sal_uInt16 i = 0;
        sal_uInt16 nCount = GetDoc()->GetSdPageCount(PK_STANDARD);
        pOlView->SetSelectedPages();

        while (i < nCount && bDisable)
        {
            SdPage* pPage = GetDoc()->GetSdPage(i, PK_STANDARD);

            if (pPage->IsSelected())
            {
                SdrObject* pObj = pPage->GetPresObj(PRESOBJ_TITLE);

                if (pObj && !pObj->IsEmptyPresObj())
                {
                    bDisable = false;
                }
            }

            i++;
        }

        if (bDisable)
        {
            rSet.DisableItem(SID_SUMMARY_PAGE);
        }
    }

    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_THESAURUS ) )
    {
        if ( !pOlView->IsTextEdit() )
        {
            rSet.DisableItem( SID_THESAURUS );
        }
        else
        {
            LanguageType            eLang = GetDoc()->GetLanguage( EE_CHAR_LANGUAGE );
            Reference< XThesaurus > xThesaurus( LinguMgr::GetThesaurus() );

            if (!xThesaurus.is() || eLang == LANGUAGE_NONE || !xThesaurus->hasLocale( LanguageTag::convertToLocale( eLang)))
                rSet.DisableItem( SID_THESAURUS );
        }
    }

    // is starting the presentation possible?
    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_PRESENTATION ) )
    {
        bool bDisable = true;
        sal_uInt16 nCount = GetDoc()->GetSdPageCount( PK_STANDARD );

        for( sal_uInt16 i = 0; i < nCount && bDisable; i++ )
        {
            SdPage* pPage = GetDoc()->GetSdPage(i, PK_STANDARD);

            if( !pPage->IsExcluded() )
                bDisable = false;
        }
        if( bDisable || GetDocSh()->IsPreview())
        {
            rSet.DisableItem( SID_PRESENTATION );
        }
    }

    FuBullet::GetSlotState( rSet, this, GetViewFrame() );

}

/**
 * gets invoked when ScrollBar is used
 */
long OutlineViewShell::VirtHScrollHdl(ScrollBar* pHScroll)
{
    long   nThumb = pHScroll->GetThumbPos();
    long   nRange = pHScroll->GetRange().Len();
    double fX     = (double) nThumb / nRange;

    Window*       pWin          = mpContentWindow.get();
    OutlinerView* pOutlinerView = pOlView->GetViewByWindow(pWin);
    long          nViewWidth    = pWin->PixelToLogic(
        pWin->GetSizePixel()).Width();
    long          nTextWidth    = pOlView->GetPaperWidth();
    nViewWidth                  = std::max(nViewWidth, nTextWidth);
    long          nCurrentPos   = pOutlinerView->GetVisArea().Left();
    long          nTargetPos    = (long)(fX * nViewWidth);
    long          nDelta        = nTargetPos - nCurrentPos;

    pOutlinerView->HideCursor();
    pOutlinerView->Scroll(-nDelta, 0);
    pOutlinerView->ShowCursor(false);

    return 0;
}

long OutlineViewShell::VirtVScrollHdl(ScrollBar* pVScroll)
{
    long nThumb = pVScroll->GetThumbPos();
    long nRange = pVScroll->GetRange().Len();
    double fY = (double) nThumb / nRange;

    Window*       pWin          = mpContentWindow.get();
    OutlinerView* pOutlinerView = pOlView->GetViewByWindow(pWin);
    long          nViewHeight   = pWin->PixelToLogic(
        pWin->GetSizePixel()).Height();
    long          nTextHeight   = pOlView->GetOutliner().GetTextHeight();
    nViewHeight                += nTextHeight;
    long          nCurrentPos   = pOutlinerView->GetVisArea().Top();
    long          nTargetPos    = (long)(fY * nViewHeight);
    long          nDelta        = nTargetPos - nCurrentPos;

    pOutlinerView->HideCursor();
    pOutlinerView->Scroll(0, -nDelta);
    pOutlinerView->ShowCursor(false);

    return 0;
}

/**
 * PrepareClose, gets called when the Shell shall be destroyed.
 * Forwards the invocation to the View
 */
bool OutlineViewShell::PrepareClose( bool bUI )
{
    if( !ViewShell::PrepareClose(bUI) )
        return false;

    return pOlView == NULL || pOlView->PrepareClose(bUI);
}

/**
 * Zoom with zoom factor. Inform OutlinerView
 */
void OutlineViewShell::SetZoom(long nZoom)
{
    ViewShell::SetZoom(nZoom);

    ::sd::Window* pWindow = mpContentWindow.get();
    if (pWindow)
    {
        // change OutputArea of OutlinerView
        OutlinerView* pOutlinerView = pOlView->GetViewByWindow(pWindow);
        Rectangle aWin(Point(0,0), pWindow->GetOutputSizePixel());
        aWin = pWindow->PixelToLogic(aWin);
        pOutlinerView->SetOutputArea(aWin);
    }

    GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOM );
    GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOMSLIDER );
}

/**
 * Zoom with zoom rectangle. Inform OutlinerView
 */
void OutlineViewShell::SetZoomRect(const Rectangle& rZoomRect)
{
    ViewShell::SetZoomRect(rZoomRect);

    ::sd::Window* pWindow = mpContentWindow.get();
    if (pWindow)
    {
        // change OutputArea of OutlinerView
        OutlinerView* pOutlinerView = pOlView->GetViewByWindow(pWindow);
        Rectangle aWin(Point(0,0), pWindow->GetOutputSizePixel());
        aWin = pWindow->PixelToLogic(aWin);
        pOutlinerView->SetOutputArea(aWin);
    }

    GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOM );
    GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOMSLIDER );
}

/**
 * Before saving: Update Model of the Drawing Engine, then forward the
 * invocation to the ObjectShell.
 */
void OutlineViewShell::Execute(SfxRequest& rReq)
{
    bool bForwardCall = true;

    switch(rReq.GetSlot())
    {
        case SID_SAVEDOC:
        case SID_SAVEASDOC:
            PrepareClose();
            break;

        case SID_SEARCH_ITEM:
            // Forward this request to the common (old) code of the
            // document shell.
            GetDocSh()->Execute (rReq);
            bForwardCall = false;
            break;

        case SID_SPELL_DIALOG:
        {
            SfxViewFrame* pViewFrame = GetViewFrame();
            if (rReq.GetArgs() != NULL)
                pViewFrame->SetChildWindow (SID_SPELL_DIALOG,
                    static_cast<const SfxBoolItem&>(rReq.GetArgs()->
                        Get(SID_SPELL_DIALOG)).GetValue());
            else
                pViewFrame->ToggleChildWindow(SID_SPELL_DIALOG);

            pViewFrame->GetBindings().Invalidate(SID_SPELL_DIALOG);
            rReq.Done ();

            bForwardCall = false;
        }
        break;

        default:
            OSL_TRACE ("OutlineViewShell::Execute(): can not handle slot %d", rReq.GetSlot());
            break;

    }

    if (bForwardCall)
        static_cast<DrawDocShell*>(GetViewFrame()->GetObjectShell())->ExecuteSlot( rReq );
}

/**
 * Read FrameViews data and set actual views data
 */
void OutlineViewShell::ReadFrameViewData(FrameView* pView)
{
    ::Outliner& rOutl = pOlView->GetOutliner();

    rOutl.SetFlatMode( pView->IsNoAttribs() );

    EEControlBits nCntrl = rOutl.GetControlWord();

    if ( pView->IsNoColors() )
        rOutl.SetControlWord(nCntrl | EEControlBits::NOCOLORS);
    else
        rOutl.SetControlWord(nCntrl & ~EEControlBits::NOCOLORS);

    sal_uInt16 nPage = mpFrameView->GetSelectedPage();
    pLastPage = GetDoc()->GetSdPage( nPage, PK_STANDARD );
    pOlView->SetActualPage(pLastPage);
}

/**
 * Write actual views data to FrameView
 */
void OutlineViewShell::WriteFrameViewData()
{
    ::Outliner& rOutl = pOlView->GetOutliner();

    EEControlBits nCntrl = rOutl.GetControlWord();
    bool bNoColor = false;
    if (nCntrl & EEControlBits::NOCOLORS)
        bNoColor = true;
    mpFrameView->SetNoColors(bNoColor);
    mpFrameView->SetNoAttribs( rOutl.IsFlatMode() );
    SdPage* pActualPage = pOlView->GetActualPage();
    DBG_ASSERT(pActualPage, "No current page");
    if( pActualPage )
        mpFrameView->SetSelectedPage((pActualPage->GetPageNum() - 1) / 2);
}

/**
 * Handle SfxRequests for the StatusBar
 */
void OutlineViewShell::ExecStatusBar(SfxRequest&)
{
}

void OutlineViewShell::GetStatusBarState(SfxItemSet& rSet)
{
    // Zoom-Item
    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_ATTR_ZOOM ) )
    {
        sal_uInt16 nZoom = (sal_uInt16) GetActiveWindow()->GetZoom();

        std::unique_ptr<SvxZoomItem> pZoomItem(new SvxZoomItem( SvxZoomType::PERCENT, nZoom ));

        // limit area
        SvxZoomEnableFlags nZoomValues = SvxZoomEnableFlags::ALL;
        nZoomValues &= ~SvxZoomEnableFlags::OPTIMAL;
        nZoomValues &= ~SvxZoomEnableFlags::WHOLEPAGE;
        nZoomValues &= ~SvxZoomEnableFlags::PAGEWIDTH;

        pZoomItem->SetValueSet( nZoomValues );
        rSet.Put( *pZoomItem );
    }

    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_ATTR_ZOOMSLIDER ) )
    {
        if (GetDocSh()->IsUIActive() || !GetActiveWindow() )
        {
            rSet.DisableItem( SID_ATTR_ZOOMSLIDER );
        }
        else
        {
            sd::Window * pActiveWindow = GetActiveWindow();
            SvxZoomSliderItem aZoomItem( (sal_uInt16) pActiveWindow->GetZoom(), (sal_uInt16)pActiveWindow->GetMinZoom(), (sal_uInt16)pActiveWindow->GetMaxZoom() ) ;
            aZoomItem.AddSnappingPoint(100);
            rSet.Put( aZoomItem );
        }
    }

    // page view and layout

    sal_uInt16  nPageCount = GetDoc()->GetSdPageCount( PK_STANDARD );
    OUString  aPageStr, aLayoutStr;

    ::sd::Window*   pWin        = GetActiveWindow();
    OutlinerView*   pActiveView = pOlView->GetViewByWindow( pWin );

    std::vector<Paragraph*> aSelList;
    pActiveView->CreateSelectionList(aSelList);

    Paragraph *pFirstPara = NULL;
    Paragraph *pLastPara = NULL;

    if (!aSelList.empty())
    {
        pFirstPara = *(aSelList.begin());
        pLastPara = *(aSelList.rbegin());
    }

    if( !::Outliner::HasParaFlag(pFirstPara,ParaFlag::ISPAGE) )
        pFirstPara = pOlView->GetPrevTitle( pFirstPara );

    if( !::Outliner::HasParaFlag(pLastPara, ParaFlag::ISPAGE) )
        pLastPara = pOlView->GetPrevTitle( pLastPara );

    // only one page selected?
    if( pFirstPara == pLastPara )
    {
        // how many pages are we before the selected page?
        sal_uLong nPos = 0L;
        while( pFirstPara )
        {
            pFirstPara = pOlView->GetPrevTitle( pFirstPara );
            if( pFirstPara )
                nPos++;
        }

        if( nPos >= GetDoc()->GetSdPageCount( PK_STANDARD ) )
            nPos = 0;

        SdrPage* pPage = GetDoc()->GetSdPage( (sal_uInt16) nPos, PK_STANDARD );

        aPageStr = SD_RESSTR(STR_SD_PAGE_COUNT);

        aPageStr = aPageStr.replaceFirst("%1", OUString::number((sal_Int32)(nPos + 1)));
        aPageStr = aPageStr.replaceFirst("%2", OUString::number(nPageCount));

        aLayoutStr = pPage->GetLayoutName();
        sal_Int32 nIndex = aLayoutStr.indexOf(SD_LT_SEPARATOR);
        if (nIndex != -1)
            aLayoutStr = aLayoutStr.copy(0, nIndex);
        //Now, CurrentPage property change is already sent for DrawView and OutlineView, so it is not necessary to send again here
        if(m_StrOldPageName!=aPageStr)
        {
            GetViewShellBase().GetDrawController().fireSwitchCurrentPage(nPos);
            m_StrOldPageName = aPageStr;
        }
    }
    rSet.Put( SfxStringItem( SID_STATUS_PAGE, aPageStr ) );
    rSet.Put( SfxStringItem( SID_STATUS_LAYOUT, aLayoutStr ) );
}

void OutlineViewShell::Command( const CommandEvent& rCEvt, ::sd::Window* pWin )
{
    if ( rCEvt.GetCommand() == CommandEventId::ContextMenu )
    {
        GetActiveWindow()->ReleaseMouse();

        OutlinerView* pOLV = pOlView->GetViewByWindow(GetActiveWindow());
        Point aPos(rCEvt.GetMousePosPixel());

        if (pOLV && pOLV->IsWrongSpelledWordAtPos(aPos))
        {
            // Popup for Online-Spelling now handled by DrawDocShell
            Link<> aLink = LINK(GetDocSh(), DrawDocShell, OnlineSpellCallback);

            pOLV->ExecuteSpellPopup(aPos, &aLink);
        }
        else
        {
           GetViewFrame()->GetDispatcher()->ExecutePopup(SdResId(RID_OUTLINE_POPUP));
        }
    }
    else
    {
        ViewShell::Command( rCEvt, pWin );

        // if necessary communicate the new context to the Preview
        Invalidate( SID_PREVIEW_STATE );

    }
}

bool OutlineViewShell::KeyInput(const KeyEvent& rKEvt, ::sd::Window* pWin)
{
    bool bReturn = false;
    OutlineViewPageChangesGuard aGuard(pOlView);

    if (pWin == NULL && HasCurrentFunction())
    {
        bReturn = GetCurrentFunction()->KeyInput(rKEvt);
    }

    // no, forward to base class
    else
    {
        bReturn = ViewShell::KeyInput(rKEvt, pWin);
    }

    Invalidate(SID_STYLE_EDIT);
    Invalidate(SID_STYLE_NEW);
    Invalidate(SID_STYLE_DELETE);
    Invalidate(SID_STYLE_HIDE);
    Invalidate(SID_STYLE_SHOW);
    Invalidate(SID_STYLE_UPDATE_BY_EXAMPLE);
    Invalidate(SID_STYLE_NEW_BY_EXAMPLE);
    Invalidate(SID_STYLE_WATERCAN);
    Invalidate(SID_STYLE_FAMILY5);

    // check and distinguish cursor movements- or input-keys
    vcl::KeyCode aKeyGroup( rKEvt.GetKeyCode().GetGroup() );
    if( (aKeyGroup != KEYGROUP_CURSOR && aKeyGroup != KEYGROUP_FKEYS) ||
        (GetActualPage() != pLastPage) )
    {
        Invalidate( SID_PREVIEW_STATE );
    }

    return bReturn;
}

/**
 * Status of Attribute-Items
 */
void OutlineViewShell::GetAttrState( SfxItemSet& rSet )
{
    SfxWhichIter  aIter( rSet );
    sal_uInt16        nWhich = aIter.FirstWhich();
    SfxAllItemSet aAllSet( *rSet.GetPool() );

    while ( nWhich )
    {
        sal_uInt16 nSlotId = SfxItemPool::IsWhich(nWhich)
            ? GetPool().GetSlotId(nWhich)
            : nWhich;

        switch ( nSlotId )
        {
            case SID_STYLE_FAMILY2:
            case SID_STYLE_FAMILY3:
            {
                rSet.DisableItem( nWhich );
            }
            break;

            case SID_STYLE_FAMILY5:
            {
                SfxStyleSheet* pStyleSheet = pOlView->GetViewByWindow(GetActiveWindow())->GetStyleSheet();

                if( pStyleSheet )
                {
                    pStyleSheet = static_cast<SdStyleSheet*>(pStyleSheet)->GetPseudoStyleSheet();

                    if (pStyleSheet)
                    {
                        SfxTemplateItem aItem( nWhich, pStyleSheet->GetName() );
                        aAllSet.Put( aItem, aItem.Which()  );
                    }
                }

                if( !pStyleSheet )
                {
                    SfxTemplateItem aItem( nWhich, OUString() );
                    aAllSet.Put( aItem, aItem.Which() );
                    // rSet.DisableItem( nWhich );
                }
            }
            break;

            case SID_STYLE_EDIT:
            {
                SfxPoolItem* pItem = NULL;
                GetViewFrame()->GetBindings().QueryState(SID_STYLE_FAMILY, pItem);
                SfxUInt16Item* pFamilyItem = dynamic_cast<SfxUInt16Item*>(pItem);
                if (pFamilyItem && SfxTemplate::NIdToSfxFamilyId(pFamilyItem->GetValue()) == SD_STYLE_FAMILY_PSEUDO)
                {
                    SfxItemSet aSet(*rSet.GetPool(), SID_STATUS_LAYOUT, SID_STATUS_LAYOUT);
                    GetStatusBarState(aSet);
                    OUString aRealStyle = static_cast<const SfxStringItem&>(aSet.Get(SID_STATUS_LAYOUT)).GetValue();
                    if (aRealStyle.isEmpty())
                    {
                        // no unique layout name found
                        rSet.DisableItem(nWhich);
                    }
                }
                delete pItem;
            }
            break;

            case SID_STYLE_UPDATE_BY_EXAMPLE:
            {
                ::sd::Window*     pActWin = GetActiveWindow();
                OutlinerView* pOV = pOlView->GetViewByWindow(pActWin);
                ESelection aESel(pOV->GetSelection());

                if (aESel.nStartPara != aESel.nEndPara ||
                    aESel.nStartPos  != aESel.nEndPos)
                    // spanned selection, i.e. StyleSheet and/or
                    // attribution not necessarily unqiue
                    rSet.DisableItem(nWhich);
            }
            break;

            case SID_STYLE_NEW:
            case SID_STYLE_DELETE:
            case SID_STYLE_HIDE:
            case SID_STYLE_SHOW:
            case SID_STYLE_NEW_BY_EXAMPLE:
            case SID_STYLE_WATERCAN:
            {
                rSet.DisableItem(nWhich);
            }
            break;
        }

        nWhich = aIter.NextWhich();
    }

    rSet.Put( aAllSet, false );
}

void OutlineViewShell::MouseButtonUp(const MouseEvent& rMEvt, ::sd::Window* pWin)
{
    // first the base classes
    ViewShell::MouseButtonUp(rMEvt, pWin);

    Invalidate(SID_STYLE_EDIT);
    Invalidate(SID_STYLE_NEW);
    Invalidate(SID_STYLE_DELETE);
    Invalidate(SID_STYLE_HIDE);
    Invalidate(SID_STYLE_SHOW);
    Invalidate(SID_STYLE_UPDATE_BY_EXAMPLE);
    Invalidate(SID_STYLE_NEW_BY_EXAMPLE);
    Invalidate(SID_STYLE_WATERCAN);
    Invalidate(SID_STYLE_FAMILY5);

    // if necessary communicate the new context to the Preview
    if( GetActualPage() != pLastPage )
        Invalidate( SID_PREVIEW_STATE );
}

SdPage* OutlineViewShell::getCurrentPage() const
{
    // since there are no master pages in outline view, we can
    // for now use the GetActualPage method
    return const_cast<OutlineViewShell*>(this)->GetActualPage();
}

/**
 * Returns the first selected page.
 * If nothing is selected, the first page is returned.
 */
SdPage* OutlineViewShell::GetActualPage()
{
    return pOlView->GetActualPage();
}

void OutlineViewShell::UpdatePreview( SdPage* pPage, bool )
{
    const bool bNewPage = pPage != pLastPage;
    pLastPage = pPage;
    if (bNewPage)
    {
        OutlineViewPageChangesGuard aGuard(pOlView);
        SetCurrentPage(pPage);
    }
}

bool OutlineViewShell::UpdateTitleObject( SdPage* pPage, Paragraph* pPara )
{
    DBG_ASSERT( pPage, "sd::OutlineViewShell::UpdateTitleObject(), pPage == 0?" );
    DBG_ASSERT( pPara, "sd::OutlineViewShell::UpdateTitleObject(), pPara == 0?" );

    if( !pPage || !pPara )
        return false;

    ::Outliner&         rOutliner = pOlView->GetOutliner();
    SdrTextObj*         pTO  = OutlineView::GetTitleTextObject( pPage );

    OUString aTest = rOutliner.GetText(pPara);
    bool    bText = !aTest.isEmpty();
    bool    bNewObject = false;

    if( bText )
    {
        // create a title object if we don't have one but have text
        if( !pTO )
        {
            DBG_ASSERT( pOlView->isRecordingUndo(), "sd::OutlineViewShell::UpdateTitleObject(), no undo for model change!?" );
            pTO = OutlineView::CreateTitleTextObject(pPage);
            bNewObject = true;
        }

        // if we have a title object and a text, set the text
        OutlinerParaObject* pOPO = pTO ? rOutliner.CreateParaObject(rOutliner.GetAbsPos(pPara), 1) : NULL;
        if (pOPO)
        {
            pOPO->SetOutlinerMode( OUTLINERMODE_TITLEOBJECT );
            pOPO->SetVertical( pTO->IsVerticalWriting() );
            if( pTO->GetOutlinerParaObject() && (pOPO->GetTextObject() == pTO->GetOutlinerParaObject()->GetTextObject()) )
            {
                // do nothing, same text already set
                delete pOPO;
            }
            else
            {
                DBG_ASSERT( pOlView->isRecordingUndo(), "sd::OutlineViewShell::UpdateTitleObject(), no undo for model change!?" );
                if( !bNewObject && pOlView->isRecordingUndo() )
                    pOlView->AddUndo(GetDoc()->GetSdrUndoFactory().CreateUndoObjectSetText(*pTO,0));

                pTO->SetOutlinerParaObject( pOPO );
                pTO->SetEmptyPresObj( false );
                pTO->ActionChanged();
            }
        }
    }
    else if( pTO )
    {
        // no text but object available?
        // outline object available, but we have no text
        if(pPage->IsPresObj(pTO))
        {
            // if it is not already empty
            if( !pTO->IsEmptyPresObj() )
            {
                DBG_ASSERT( pOlView->isRecordingUndo(), "sd::OutlineViewShell::UpdateTitleObject(), no undo for model change!?" );

                // make it empty
                if( pOlView->isRecordingUndo() )
                    pOlView->AddUndo(GetDoc()->GetSdrUndoFactory().CreateUndoObjectSetText(*pTO,0));
                pPage->RestoreDefaultText( pTO );
                pTO->SetEmptyPresObj(true);
                pTO->ActionChanged();
            }
        }
        else
        {
            DBG_ASSERT( pOlView->isRecordingUndo(), "sd::OutlineViewShell::UpdateTitleObject(), no undo for model change!?" );
            // outline object is not part of the layout, delete it
            if( pOlView->isRecordingUndo() )
                pOlView->AddUndo(GetDoc()->GetSdrUndoFactory().CreateUndoRemoveObject(*pTO));
            pPage->RemoveObject(pTO->GetOrdNum());
        }
    }

    return bNewObject;
}

bool OutlineViewShell::UpdateOutlineObject( SdPage* pPage, Paragraph* pPara )
{
    DBG_ASSERT( pPage, "sd::OutlineViewShell::UpdateOutlineObject(), pPage == 0?" );
    DBG_ASSERT( pPara, "sd::OutlineViewShell::UpdateOutlineObject(), pPara == 0?" );

    if( !pPage || !pPara )
        return false;

    ::Outliner&         rOutliner = pOlView->GetOutliner();
    OutlinerParaObject* pOPO = NULL;
    SdrTextObj*         pTO  = NULL;

    bool bNewObject = false;

    sal_uInt16 eOutlinerMode = OUTLINERMODE_TITLEOBJECT;
    pTO = static_cast<SdrTextObj*>(pPage->GetPresObj( PRESOBJ_TEXT ));
    if( !pTO )
    {
        eOutlinerMode = OUTLINERMODE_OUTLINEOBJECT;
        pTO = OutlineView::GetOutlineTextObject( pPage );
    }

    // how many paragraphs in the outline?
    sal_Int32 nTitlePara     = rOutliner.GetAbsPos( pPara );
    sal_Int32 nPara          = nTitlePara + 1;
    sal_Int32 nParasInLayout = 0L;
    pPara = rOutliner.GetParagraph( nPara );
    while( pPara && !::Outliner::HasParaFlag(pPara, ParaFlag::ISPAGE) )
    {
        nParasInLayout++;
        pPara = rOutliner.GetParagraph( ++nPara );
    }
    if( nParasInLayout )
    {
        // create an OutlinerParaObject
        pOPO  = rOutliner.CreateParaObject( nTitlePara + 1, nParasInLayout );
    }

    if( pOPO )
    {
        DBG_ASSERT( pOlView->isRecordingUndo(), "sd::OutlineViewShell::UpdateOutlineObject(), no undo for model change!?" );

        // do we need an outline text object?
        if( !pTO )
        {
            pTO = OutlineView::CreateOutlineTextObject( pPage );
            bNewObject = true;
        }

        // page object, outline text in Outliner:
        // apply text
        if( pTO )
        {
            pOPO->SetVertical( pTO->IsVerticalWriting() );
            pOPO->SetOutlinerMode( eOutlinerMode );
            if( pTO->GetOutlinerParaObject() && (pOPO->GetTextObject() == pTO->GetOutlinerParaObject()->GetTextObject()) )
            {
                // do nothing, same text already set
                delete pOPO;
            }
            else
            {
                if( !bNewObject && pOlView->isRecordingUndo() )
                    pOlView->AddUndo(GetDoc()->GetSdrUndoFactory().CreateUndoObjectSetText(*pTO,0));

                pTO->SetOutlinerParaObject( pOPO );
                pTO->SetEmptyPresObj( false );
                pTO->ActionChanged();
            }
        }
        else
            delete pOPO;
    }
    else if( pTO )
    {
        // page object but no outline text:
        // if the object is in the outline of the page -> default text

        // otherwise delete object
        if( pPage->IsPresObj(pTO) )
        {
            if( !pTO->IsEmptyPresObj() )
            {
                DBG_ASSERT( pOlView->isRecordingUndo(), "sd::OutlineViewShell::UpdateOutlineObject(), no undo for model change!?" );

                // delete old OutlinerParaObject, too
                if( pOlView->isRecordingUndo() )
                    pOlView->AddUndo(GetDoc()->GetSdrUndoFactory().CreateUndoObjectSetText(*pTO,0));
                pPage->RestoreDefaultText( pTO );
                pTO->SetEmptyPresObj(true);
                pTO->ActionChanged();
            }
        }
        else
        {
            DBG_ASSERT( pOlView->isRecordingUndo(), "sd::OutlineViewShell::UpdateOutlineObject(), no undo for model change!?" );
            if( pOlView->isRecordingUndo() )
                pOlView->AddUndo(GetDoc()->GetSdrUndoFactory().CreateUndoRemoveObject(*pTO));
            pPage->RemoveObject(pTO->GetOrdNum());
        }
    }

    return bNewObject;
}

/**
 * Fill Outliner from Stream
 */
sal_uLong OutlineViewShell::Read(SvStream& rInput, const OUString& rBaseURL, sal_uInt16 eFormat)
{
    sal_uLong bRet = 0;

    ::Outliner& rOutl = pOlView->GetOutliner();

    OutlineViewPageChangesGuard aGuard( pOlView );
    OutlineViewModelChangeGuard aGuard2( *pOlView );

    bRet = rOutl.Read( rInput, rBaseURL, eFormat, GetDocSh()->GetHeaderAttributes() );

    SdPage* pPage = GetDoc()->GetSdPage( GetDoc()->GetSdPageCount(PK_STANDARD) - 1, PK_STANDARD );;
    SfxStyleSheet* pTitleSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_TITLE );
    SfxStyleSheet* pOutlSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_OUTLINE );

    sal_Int32 nParaCount = rOutl.GetParagraphCount();
    if ( nParaCount > 0 )
    {
        for ( sal_Int32 nPara = 0; nPara < nParaCount; nPara++ )
        {
            pOlView->UpdateParagraph( nPara );

            sal_Int16 nDepth = rOutl.GetDepth( nPara );

            if( (nDepth == 0) || !nPara )
            {
                Paragraph* pPara = rOutl.GetParagraph( nPara );
                rOutl.SetDepth(pPara, -1);
                rOutl.SetParaFlag(pPara, ParaFlag::ISPAGE);

                rOutl.SetStyleSheet( nPara, pTitleSheet );

                if( nPara ) // first slide already exists
                    pOlView->InsertSlideForParagraph( pPara );
            }
            else
            {
                rOutl.SetDepth( rOutl.GetParagraph( nPara ), nDepth - 1 );
                OUString aStyleSheetName = pOutlSheet->GetName();
                if (!aStyleSheetName.isEmpty())
                    aStyleSheetName = aStyleSheetName.copy(0, aStyleSheetName.getLength() - 1);
                aStyleSheetName += OUString::number( nDepth );
                SfxStyleSheetBasePool* pStylePool = GetDoc()->GetStyleSheetPool();
                SfxStyleSheet* pStyle = static_cast<SfxStyleSheet*>( pStylePool->Find( aStyleSheetName, pOutlSheet->GetFamily() ) );
                DBG_ASSERT( pStyle, "AutoStyleSheetName - Style not found!" );
                if ( pStyle )
                    rOutl.SetStyleSheet( nPara, pStyle );
            }
        }
    }

    rOutl.GetUndoManager().Clear();

    return bRet;
}

void OutlineViewShell::WriteUserDataSequence ( ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& rSequence, bool bBrowse )
{
    WriteFrameViewData();

    ViewShell::WriteUserDataSequence( rSequence, bBrowse );
}

void OutlineViewShell::ReadUserDataSequence ( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& rSequence, bool bBrowse )
{
    WriteFrameViewData();

    ViewShell::ReadUserDataSequence( rSequence, bBrowse );

    ReadFrameViewData( mpFrameView );
}

void OutlineViewShell::VisAreaChanged(const Rectangle& rRect)
{
    ViewShell::VisAreaChanged( rRect );

    GetViewShellBase().GetDrawController().FireVisAreaChanged(rRect);
}

/** If there is a valid controller then create a new instance of
    <type>AccessibleDrawDocumentView</type>.  Otherwise return an empty
    reference.
*/
::com::sun::star::uno::Reference<
    ::com::sun::star::accessibility::XAccessible>
    OutlineViewShell::CreateAccessibleDocumentView (::sd::Window* pWindow)
{
    OSL_ASSERT (GetViewShell()!=NULL);
    if (GetViewShell()->GetController() != NULL)
    {
        ::accessibility::AccessibleOutlineView* pDocumentView =
            new ::accessibility::AccessibleOutlineView (
                pWindow,
                this,
                GetViewShell()->GetController(),
                pWindow->GetAccessibleParentWindow()->GetAccessible());
        pDocumentView->Init();
        return ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>
            (static_cast< ::com::sun::star::uno::XWeak*>(pDocumentView),
                ::com::sun::star::uno::UNO_QUERY);
    }

    OSL_TRACE ("OutlineViewShell::CreateAccessibleDocumentView: no controller");
    return ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >();
}

void OutlineViewShell::GetState (SfxItemSet& rSet)
{
    // Iterate over all requested items in the set.
    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        switch (nWhich)
        {
            case SID_SEARCH_ITEM:
            case SID_SEARCH_OPTIONS:
                // Call common (old) implementation in the document shell.
                GetDocSh()->GetState (rSet);
                break;
            default:
                OSL_TRACE ("OutlineViewShell::GetState(): can not handle which id %d", nWhich);
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

void OutlineViewShell::SetCurrentPage (SdPage* pPage)
{
    // Adapt the selection of the model.
    for (sal_uInt16 i=0; i<GetDoc()->GetSdPageCount(PK_STANDARD); i++)
        GetDoc()->SetSelected(
            GetDoc()->GetSdPage(i, PK_STANDARD),
            false);
    GetDoc()->SetSelected (pPage, true);

    DrawController& rController(GetViewShellBase().GetDrawController());
    rController.FireSelectionChangeListener();
    rController.FireSwitchCurrentPage (pPage);

    pOlView->SetActualPage(pPage);
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
