/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "OutlineViewShell.hxx"

#include "ViewShellImplementation.hxx"
#include <memory>
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

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

using namespace sd;
#define OutlineViewShell
#include "sdslots.hxx"

namespace sd {

#define MIN_ZOOM           10      
#define MAX_ZOOM         1000      

/************************************************************************/


/**
 * Declare SFX-Slotmap and standard interface
 */
SFX_IMPL_INTERFACE(OutlineViewShell, SfxShell, SdResId(STR_OUTLINEVIEWSHELL))
{
    SFX_POPUPMENU_REGISTRATION( SdResId(RID_OUTLINE_POPUP) );
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_TOOLS | SFX_VISIBILITY_STANDARD |
                                SFX_VISIBILITY_FULLSCREEN | SFX_VISIBILITY_SERVER,
                                SdResId(RID_OUTLINE_TOOLBOX) );
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_APPLICATION | SFX_VISIBILITY_DESKTOP | SFX_VISIBILITY_STANDARD | SFX_VISIBILITY_CLIENT | SFX_VISIBILITY_VIEWER | SFX_VISIBILITY_READONLYDOC,
                                SdResId(RID_DRAW_VIEWER_TOOLBOX) );
    SFX_CHILDWINDOW_REGISTRATION( SfxTemplateDialogWrapper::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SfxInfoBarContainerChild::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxHlinkDlgWrapper::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( ::sd::SpellDialogChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SID_SEARCH_DLG );
    SFX_CHILDWINDOW_REGISTRATION(::sfx2::sidebar::SidebarChildWindow::GetChildWindowId());
}


TYPEINIT1( OutlineViewShell, ViewShell );


/**
 * common initialization part of both constructors
 */
void OutlineViewShell::Construct(DrawDocShell* )
{
    sal_Bool bModified = GetDoc()->IsChanged();

    meShellType = ST_OUTLINE;
    Size aSize(29700, 21000);
    Point aWinPos (0, 0);
    Point aViewOrigin(0, 0);
    GetActiveWindow()->SetMinZoomAutoCalc(false);
    GetActiveWindow()->SetMinZoom( MIN_ZOOM );
    GetActiveWindow()->SetMaxZoom( MAX_ZOOM );
    InitWindows(aViewOrigin, aSize, aWinPos);
    pOlView = new OutlineView(*GetDocSh(), GetActiveWindow(), *this);
    mpView = pOlView;            

    SetPool( &GetDoc()->GetPool() );

    SetZoom(69);

    
    ReadFrameViewData(mpFrameView);

    ::Outliner* pOutl = pOlView->GetOutliner();
    pOutl->SetUpdateMode(sal_True);

    if (!bModified)
    {
        pOutl->ClearModifyFlag();
    }

    pLastPage = GetActualPage();

    SetName( OUString( "OutlineViewShell" ) );

    SetHelpId( SD_IF_SDOUTLINEVIEWSHELL );
    GetActiveWindow()->SetHelpId( HID_SDOUTLINEVIEWSHELL );
    GetActiveWindow()->SetUniqueId( HID_SDOUTLINEVIEWSHELL );
}




Reference<drawing::XDrawSubController> OutlineViewShell::CreateSubController (void)
{
    Reference<drawing::XDrawSubController> xSubController;

    if (IsMainViewShell())
    {
        
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
    ::Window* pParentWindow,
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

    m_StrOldPageName = OUString();

    doShow();
}


OutlineViewShell::~OutlineViewShell()
{
    DisposeFunctions();

    delete pOlView;

    mpFrameView->Disconnect();

    if ( pClipEvtLstnr )
    {
        pClipEvtLstnr->AddRemoveListener( GetActiveWindow(), sal_False );
        pClipEvtLstnr->ClearCallbackLink();     
        pClipEvtLstnr->release();
    }
}




void OutlineViewShell::Shutdown (void)
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

    if(HasCurrentFunction())
    {
        GetCurrentFunction()->Paint(rRect, pWin);
    }
}

void OutlineViewShell::ArrangeGUIElements ()
{
    
    
    
    int nScrollBarSize =
        GetParentWindow()->GetSettings().GetStyleSettings().GetScrollBarSize();
    maScrBarWH = Size (nScrollBarSize, nScrollBarSize);

    ViewShell::ArrangeGUIElements ();

    ::sd::Window* pWindow = mpContentWindow.get();
    if (pWindow != NULL)
    {
        pWindow->SetMinZoomAutoCalc(false);


        
        OutlinerView* pOutlinerView = pOlView->GetViewByWindow(pWindow);

        Rectangle aWin(Point(0,0), pWindow->GetOutputSizePixel());

        aWin = pWindow->PixelToLogic(aWin);
        pOutlinerView->SetOutputArea(aWin);

        Rectangle aVis = pOutlinerView->GetVisArea();

        Rectangle aText = Rectangle(Point(0,0),
            Size(pOlView->GetPaperWidth(),
                pOlView->GetOutliner()->GetTextHeight()));
        aText.Bottom() += aWin.GetHeight();

        if (!aWin.IsEmpty())            
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
            pOlView->GetOutliner()->UpdateFields();
            UpdatePreview( GetActualPage() );
            rReq.Done();
            break;
        }

        default:
        break;
    }
}




void OutlineViewShell::AddWindow (::sd::Window* pWin)
{
    pOlView->AddWindowToPaintView(pWin);
}




void OutlineViewShell::RemoveWindow (::sd::Window* pWin)
{
    pOlView->DeleteWindowFromPaintView(pWin);
}




/**
 * Activate(): during the first invocation the fields get updated
 */
void OutlineViewShell::Activate( sal_Bool bIsMDIActivate )
{
    if ( ! mbInitialized)
    {
        mbInitialized = true;
        SfxRequest aRequest (SID_EDIT_OUTLINER, 0, GetDoc()->GetItemPool());
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

void OutlineViewShell::Deactivate( sal_Bool bIsMDIActivate )
{
    pOlView->DisconnectFromApplication();

    
    
    ViewShell::Deactivate( bIsMDIActivate );
}

/**
 * Set status of Controller-SfxSlots
 */
void OutlineViewShell::GetCtrlState(SfxItemSet &rSet)
{
    if (SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_HYPERLINK_GETLINK))
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
                        aHLinkItem.SetName(((const SvxURLField*) pField)->GetRepresentation());
                        aHLinkItem.SetURL(((const SvxURLField*) pField)->GetURL());
                        aHLinkItem.SetTargetFrame(((const SvxURLField*) pField)->GetTargetFrame());
                    }
                }
            }
        }
        rSet.Put(aHLinkItem);
    }
    rSet.Put( SfxBoolItem( SID_READONLY_MODE, GetDocSh()->IsReadOnly() ) );

    if ( SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_MAIL_SCROLLBODY_PAGEDOWN) )
        rSet.Put( SfxBoolItem( SID_MAIL_SCROLLBODY_PAGEDOWN, true ) );

    if ( SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_TRANSLITERATE_HALFWIDTH) ||
         SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_TRANSLITERATE_FULLWIDTH) ||
         SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_TRANSLITERATE_HIRAGANA) ||
         SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_TRANSLITERATE_KATAGANA) )
    {
        SvtCJKOptions aCJKOptions;
        if( !aCJKOptions.IsChangeCaseMapEnabled() )
        {
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_HALFWIDTH, sal_False );
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_FULLWIDTH, sal_False );
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_HIRAGANA, sal_False );
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_KATAGANA, sal_False );
            rSet.DisableItem( SID_TRANSLITERATE_HALFWIDTH );
            rSet.DisableItem( SID_TRANSLITERATE_FULLWIDTH );
            rSet.DisableItem( SID_TRANSLITERATE_HIRAGANA );
            rSet.DisableItem( SID_TRANSLITERATE_KATAGANA );
        }
        else
        {
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_HALFWIDTH, sal_True );
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_FULLWIDTH, sal_True );
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_HIRAGANA, sal_True );
            GetViewFrame()->GetBindings().SetVisibleState( SID_TRANSLITERATE_KATAGANA, sal_True );
        }
    }
}

/**
 * SfxRequests for support functions
 */
void OutlineViewShell::FuSupport(SfxRequest &rReq)
{
    if( rReq.GetSlot() == SID_STYLE_FAMILY && rReq.GetArgs())
        GetDocSh()->SetStyleFamily(((SfxUInt16Item&)rReq.GetArgs()->Get( SID_STYLE_FAMILY )).GetValue());

    sal_Bool bPreviewState = sal_False;
    sal_uLong nSlot = rReq.GetSlot();

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr< OutlineViewModelChangeGuard > aGuard;
    SAL_WNODEPRECATED_DECLARATIONS_POP
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
            bPreviewState = sal_True;
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
            bPreviewState = sal_True;
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
            bPreviewState = sal_True;
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
                                         nAction, sal_False, FORMAT_STRING);
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

                    KeyCode  aKCode(KEY_DELETE);
                    KeyEvent aKEvt( 0, aKCode );
                    pOutlView->PostKeyEvent(aKEvt);

                    rtl::Reference<FuPoor> xFunc( GetCurrentFunction() );
                    FuOutlineText* pFuOutlineText = dynamic_cast< FuOutlineText* >( xFunc.get() );
                    if( pFuOutlineText )
                        pFuOutlineText->UpdateForKeyPress (aKEvt);
                }
            }
            rReq.Done();
            bPreviewState = sal_True;
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
            OutlinerView* pOLV = pOlView->GetViewByWindow( GetActiveWindow() );
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
            bPreviewState = sal_True;
        }
        break;

        
        case SID_UNDO :
        {
            OutlineViewPageChangesGuard aGuard2(pOlView);
            ImpSidUndo(sal_False, rReq);
        }
        break;
        case SID_REDO :
        {
            OutlineViewPageChangesGuard aGuard2(pOlView);
            ImpSidRedo(sal_False, rReq);
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
            ::Outliner* pOutl = pOlView->GetOutliner();
            if( pOutl )
            {
                pOutl->GetUndoManager().Clear();
                pOutl->UpdateFields();
            }

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


IMPL_LINK( OutlineViewShell, ClipboardChanged, TransferableDataHelper*, pDataHelper )
{
    if ( pDataHelper )
    {
        bPastePossible = ( pDataHelper->GetFormatCount() != 0 &&
                            ( pDataHelper->HasFormat( FORMAT_STRING ) ||
                              pDataHelper->HasFormat( FORMAT_RTF ) ||
                              pDataHelper->HasFormat( SOT_FORMATSTR_ID_HTML ) ) );

        SfxBindings& rBindings = GetViewFrame()->GetBindings();
        rBindings.Invalidate( SID_PASTE );
        rBindings.Invalidate( SID_PASTE_SPECIAL );
        rBindings.Invalidate( SID_PASTE_UNFORMATTED );
        rBindings.Invalidate( SID_CLIPBOARD_FORMAT_ITEMS );
    }
    return 0;
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

    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_ZOOM_IN ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_ZOOM_OUT ) )
    {
        if( GetActiveWindow()->GetZoom() <= GetActiveWindow()->GetMinZoom() || GetDocSh()->IsUIActive() )
            rSet.DisableItem( SID_ZOOM_IN );
        if( GetActiveWindow()->GetZoom() >= GetActiveWindow()->GetMaxZoom() || GetDocSh()->IsUIActive() )
            rSet.DisableItem( SID_ZOOM_OUT );
    }

    ::Outliner* pOutl = pOlView->GetOutliner();
    DBG_ASSERT(pOutl, "OutlineViewShell::GetMenuState(), no outliner? Fatality!");
    if( !pOutl )
        return;

    
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_SELECTALL ) )
    {
        sal_Int32 nParaCount = pOutl->GetParagraphCount();
        sal_Bool bDisable = nParaCount == 0;
        if (!bDisable && nParaCount == 1)
        {
            OUString aTest = pOutl->GetText(pOutl->GetParagraph(0));
            if (aTest.isEmpty())
            {
                bDisable = sal_True;
            }
        }
        if (bDisable)
            rSet.DisableItem(SID_SELECTALL);
    }

    
    rSet.Put( SfxBoolItem( SID_RULER, HasRuler() ) );

    
    rSet.Put( SfxBoolItem( SID_OUTLINE_FORMAT, !pOutl->IsFlatMode() ) );

    if( pOutl->IsFlatMode() )
        rSet.DisableItem( SID_COLORVIEW );
    else
    {
        
        sal_uLong nCntrl = pOutl->GetControlWord();
        sal_Bool bNoColor = sal_False;
        if (nCntrl & EE_CNTRL_NOCOLORS)
            bNoColor = sal_True;

        rSet.Put( SfxBoolItem( SID_COLORVIEW, bNoColor ) );
    }

    
    
    sal_Bool bDisableCollapse = sal_True;
    sal_Bool bDisableExpand   = sal_True;
    sal_Bool bUnique          = sal_True;
    OutlinerView* pOutlinerView = pOlView->GetViewByWindow(GetActiveWindow());

    std::vector<Paragraph*> aSelList;
    pOutlinerView->CreateSelectionList(aSelList);

    if (!aSelList.empty())
    {
        std::vector<Paragraph*>::const_iterator iter = aSelList.begin();
        Paragraph* pPara = *iter;

        sal_Int16 nDepth;
        sal_Int16 nTmpDepth = pOutl->GetDepth( pOutl->GetAbsPos( pPara ) );
        bool bPage = pOutl->HasParaFlag( pPara, PARAFLAG_ISPAGE );

        while (iter != aSelList.begin())
        {
            pPara = *iter;

            nDepth = pOutl->GetDepth( pOutl->GetAbsPos( pPara ) );

            if( nDepth != nTmpDepth || bPage != pOutl->HasParaFlag( pPara, PARAFLAG_ISPAGE ))
                bUnique = sal_False;

            if (pOutl->HasChildren(pPara))
            {
                if (!pOutl->IsExpanded(pPara))
                    bDisableExpand = sal_False;
                else
                    bDisableCollapse = sal_False;
            }

            ++iter;
        }
    }

    if (bDisableExpand)
        rSet.DisableItem(SID_OUTLINE_EXPAND);
    if (bDisableCollapse)
        rSet.DisableItem(SID_OUTLINE_COLLAPSE);

    
    
    SfxItemSet aSet(*rSet.GetPool(), SID_STATUS_LAYOUT, SID_STATUS_LAYOUT);
    GetStatusBarState(aSet);
    OUString aTest = ((SfxStringItem&)aSet.Get(SID_STATUS_LAYOUT)).GetValue();
    if (aTest.isEmpty())
    {
        bUnique = sal_False;
        rSet.DisableItem(SID_PRESENTATION_TEMPLATES);
    }

    if (!bUnique)
        rSet.DisableItem( SID_PRESENTATIONOBJECT );

    
    sal_Bool bDisableCollapseAll = sal_True;
    sal_Bool bDisableExpandAll   = sal_True;

    
    if (!bDisableCollapse)
        bDisableCollapseAll = sal_False;
    if (!bDisableExpand)
        bDisableExpandAll = sal_False;

    
    if (bDisableCollapseAll || bDisableExpandAll)
    {
        sal_Int32 nParaPos = 0;
        Paragraph* pPara = pOutl->GetParagraph( nParaPos );
        while (pPara && (bDisableCollapseAll || bDisableExpandAll))
        {
            if (!pOutl->IsExpanded(pPara) && pOutl->HasChildren(pPara))
                bDisableExpandAll = sal_False;

            if (pOutl->IsExpanded(pPara) && pOutl->HasChildren(pPara))
                bDisableCollapseAll = sal_False;

            pPara = pOutl->GetParagraph( ++nParaPos );
        }
    }

    if (bDisableExpandAll)
        rSet.DisableItem(SID_OUTLINE_EXPAND_ALL);
    if (bDisableCollapseAll)
        rSet.DisableItem(SID_OUTLINE_COLLAPSE_ALL);

    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PASTE ) )
    {
        if ( !pClipEvtLstnr )
        {
            
            pClipEvtLstnr = new TransferableClipboardListener( LINK( this, OutlineViewShell, ClipboardChanged ) );
            pClipEvtLstnr->acquire();
            pClipEvtLstnr->AddRemoveListener( GetActiveWindow(), sal_True );

            
            TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( GetActiveWindow() ) );
            bPastePossible = ( aDataHelper.GetFormatCount() != 0 &&
                                ( aDataHelper.HasFormat( FORMAT_STRING ) ||
                                  aDataHelper.HasFormat( FORMAT_RTF ) ||
                                  aDataHelper.HasFormat( SOT_FORMATSTR_ID_HTML ) ) );
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

    if (pOlView->GetOutliner()->IsModified())
    {
        GetDoc()->SetChanged(true);
    }

    
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

    
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_MODIFY_FIELD ) )
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

    if (SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_EXPAND_PAGE))
    {
        sal_Bool bDisable = sal_True;
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

    if (SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_SUMMARY_PAGE))
    {
        sal_Bool bDisable = sal_True;
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
                    bDisable = sal_False;
                }
            }

            i++;
        }

        if (bDisable)
        {
            rSet.DisableItem(SID_SUMMARY_PAGE);
        }
    }

    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_THESAURUS ) )
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

    
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PRESENTATION ) )
    {
        sal_Bool bDisable = sal_True;
        sal_uInt16 nCount = GetDoc()->GetSdPageCount( PK_STANDARD );

        for( sal_uInt16 i = 0; i < nCount && bDisable; i++ )
        {
            SdPage* pPage = GetDoc()->GetSdPage(i, PK_STANDARD);

            if( !pPage->IsExcluded() )
                bDisable = sal_False;
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
    pOutlinerView->ShowCursor(sal_False);

    pOlView->InvalidateSlideNumberArea();
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
    long          nTextHeight   = pOlView->GetOutliner()->GetTextHeight();
    nViewHeight                += nTextHeight;
    long          nCurrentPos   = pOutlinerView->GetVisArea().Top();
    long          nTargetPos    = (long)(fY * nViewHeight);
    long          nDelta        = nTargetPos - nCurrentPos;

    pOutlinerView->HideCursor();
    pOutlinerView->Scroll(0, -nDelta);
    pOutlinerView->ShowCursor(sal_False);

    pOlView->InvalidateSlideNumberArea();

    return 0;
}

/**
 * PrepareClose, gets called when the Shell shall be destroyed.
 * Forwards the invocation to the View
 */
bool OutlineViewShell::PrepareClose( sal_Bool bUI )
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
            
            
            GetDocSh()->Execute (rReq);
            bForwardCall = false;
            break;

        case SID_SPELL_DIALOG:
        {
            SfxViewFrame* pViewFrame = GetViewFrame();
            if (rReq.GetArgs() != NULL)
                pViewFrame->SetChildWindow (SID_SPELL_DIALOG,
                    ((const SfxBoolItem&) (rReq.GetArgs()->
                        Get(SID_SPELL_DIALOG))).GetValue());
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
        ((DrawDocShell*)GetViewFrame()->GetObjectShell())->ExecuteSlot( rReq );
}

/**
 * Read FrameViews data and set actual views data
 */
void OutlineViewShell::ReadFrameViewData(FrameView* pView)
{
    ::Outliner* pOutl = pOlView->GetOutliner();

    pOutl->SetFlatMode( pView->IsNoAttribs() );

    sal_uLong nCntrl = pOutl->GetControlWord();

    if ( pView->IsNoColors() )
        pOutl->SetControlWord(nCntrl | EE_CNTRL_NOCOLORS);
    else
        pOutl->SetControlWord(nCntrl & ~EE_CNTRL_NOCOLORS);

    sal_uInt16 nPage = mpFrameView->GetSelectedPage();
    pLastPage = GetDoc()->GetSdPage( nPage, PK_STANDARD );
    pOlView->SetActualPage(pLastPage);
}



/**
 * Write actual views data to FrameView
 */
void OutlineViewShell::WriteFrameViewData()
{
    ::Outliner* pOutl = pOlView->GetOutliner();

    sal_uLong nCntrl = pOutl->GetControlWord();
    sal_Bool bNoColor = sal_False;
    if (nCntrl & EE_CNTRL_NOCOLORS)
        bNoColor = sal_True;
    mpFrameView->SetNoColors(bNoColor);
    mpFrameView->SetNoAttribs( pOutl->IsFlatMode() );
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
    
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_ATTR_ZOOM ) )
    {
        SvxZoomItem* pZoomItem;
        sal_uInt16 nZoom = (sal_uInt16) GetActiveWindow()->GetZoom();

        pZoomItem = new SvxZoomItem( SVX_ZOOM_PERCENT, nZoom );

        
        sal_uInt16 nZoomValues = SVX_ZOOM_ENABLE_ALL;
        nZoomValues &= ~SVX_ZOOM_ENABLE_OPTIMAL;
        nZoomValues &= ~SVX_ZOOM_ENABLE_WHOLEPAGE;
        nZoomValues &= ~SVX_ZOOM_ENABLE_PAGEWIDTH;

        pZoomItem->SetValueSet( nZoomValues );
        rSet.Put( *pZoomItem );
        delete pZoomItem;
    }

    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_ATTR_ZOOMSLIDER ) )
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


    

    sal_uInt16  nPageCount = GetDoc()->GetSdPageCount( PK_STANDARD );
    OUString  aPageStr, aLayoutStr;

    ::sd::Window*       pWin        = GetActiveWindow();
    OutlinerView*   pActiveView = pOlView->GetViewByWindow( pWin );
    ::Outliner*     pOutliner   = pOlView->GetOutliner();

    std::vector<Paragraph*> aSelList;
    pActiveView->CreateSelectionList(aSelList);

    Paragraph *pFirstPara = NULL;
    Paragraph *pLastPara = NULL;

    if (!aSelList.empty())
    {
        pFirstPara = *(aSelList.begin());
        pLastPara = *(aSelList.rbegin());
    }

    if( !pOutliner->HasParaFlag(pFirstPara,PARAFLAG_ISPAGE) )
        pFirstPara = pOlView->GetPrevTitle( pFirstPara );

    if( !pOutliner->HasParaFlag(pLastPara, PARAFLAG_ISPAGE) )
        pLastPara = pOlView->GetPrevTitle( pLastPara );

    
    if( pFirstPara == pLastPara )
    {
        
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

        aPageStr = SD_RESSTR(STR_SD_PAGE);
        aPageStr += " ";
        aPageStr += OUString::number( (sal_Int32)(nPos + 1) );   
        aPageStr += " / ";
        aPageStr += OUString::number( nPageCount );

        aLayoutStr = pPage->GetLayoutName();
        sal_Int32 nIndex = aLayoutStr.indexOf(SD_LT_SEPARATOR);
        if (nIndex != -1)
            aLayoutStr = aLayoutStr.copy(0, nIndex);
        
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
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU )
    {
        GetActiveWindow()->ReleaseMouse();

        OutlinerView* pOLV = pOlView->GetViewByWindow(GetActiveWindow());
        Point aPos(rCEvt.GetMousePosPixel());

        if (pOLV && pOLV->IsWrongSpelledWordAtPos(aPos))
        {
            
            Link aLink = LINK(GetDocSh(), DrawDocShell, OnlineSpellCallback);

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

        
        Invalidate( SID_PREVIEW_STATE );

    }
}



sal_Bool OutlineViewShell::KeyInput(const KeyEvent& rKEvt, ::sd::Window* pWin)
{
    sal_Bool bReturn = sal_False;
    OutlineViewPageChangesGuard aGuard(pOlView);

    if (pWin == NULL && HasCurrentFunction())
    {
        bReturn = GetCurrentFunction()->KeyInput(rKEvt);
    }

    
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

    
    KeyCode aKeyGroup( rKEvt.GetKeyCode().GetGroup() );
    if( (aKeyGroup != KEYGROUP_CURSOR && aKeyGroup != KEYGROUP_FKEYS) ||
        (GetActualPage() != pLastPage) )
    {
        Invalidate( SID_PREVIEW_STATE );
    }

    return(bReturn);
}


/**
 * Return text of the selection
 */
OUString OutlineViewShell::GetSelectionText(bool bCompleteWords)
{
    OUString aStrSelection;
    ::Outliner* pOl = pOlView->GetOutliner();
    OutlinerView* pOutlinerView = pOlView->GetViewByWindow( GetActiveWindow() );

    if (pOl)
    {
        if (bCompleteWords)
        {
            ESelection aSel = pOutlinerView->GetSelection();
            OUString aStrCurrentDelimiters = pOl->GetWordDelimiters();

            pOl->SetWordDelimiters(" .,;\"'");
            aStrSelection = pOl->GetWord( aSel.nEndPara, aSel.nEndPos );
            pOl->SetWordDelimiters( aStrCurrentDelimiters );
        }
        else
        {
            aStrSelection = pOutlinerView->GetSelected();
        }
    }

    return (aStrSelection);
}


/**
 * Is something selected?
 */
sal_Bool OutlineViewShell::HasSelection(sal_Bool bText) const
{
    sal_Bool bReturn = sal_False;

    if (bText)
    {
        OutlinerView* pOutlinerView = pOlView->GetViewByWindow( GetActiveWindow() );

        if (pOutlinerView && !pOutlinerView->GetSelected().isEmpty())
        {
            bReturn = sal_True;
        }
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
                    pStyleSheet = ((SdStyleSheet*)pStyleSheet)->GetPseudoStyleSheet();

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
                    
                }
            }
            break;

            case SID_STYLE_EDIT:
            {
                ISfxTemplateCommon* pTmplCommon = SFX_APP()->GetCurrentTemplateCommon(GetViewFrame()->GetBindings());

                if (pTmplCommon && pTmplCommon->GetActualFamily() == SD_STYLE_FAMILY_PSEUDO)
                {
                    SfxItemSet aSet(*rSet.GetPool(), SID_STATUS_LAYOUT, SID_STATUS_LAYOUT);
                    GetStatusBarState(aSet);
                    OUString aRealStyle = ((SfxStringItem&) aSet.Get(SID_STATUS_LAYOUT)).GetValue();
                    if (aRealStyle.isEmpty())
                    {
                        
                        rSet.DisableItem(nWhich);
                    }
                }
            }
            break;

            case SID_STYLE_UPDATE_BY_EXAMPLE:
            {
                ::sd::Window*     pActWin = GetActiveWindow();
                OutlinerView* pOV = pOlView->GetViewByWindow(pActWin);
                ESelection aESel(pOV->GetSelection());

                if (aESel.nStartPara != aESel.nEndPara ||
                    aESel.nStartPos  != aESel.nEndPos)
                    
                    
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

    
    if( GetActualPage() != pLastPage )
        Invalidate( SID_PREVIEW_STATE );
}



SdPage* OutlineViewShell::getCurrentPage() const
{
    
    
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

void OutlineViewShell::UpdatePreview( SdPage* pPage, sal_Bool )
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

    ::Outliner*             pOutliner = pOlView->GetOutliner();
    SdrTextObj*         pTO  = pOlView->GetTitleTextObject( pPage );
    OutlinerParaObject* pOPO = NULL;

    OUString aTest = pOutliner->GetText(pPara);
    bool    bText = !aTest.isEmpty();
    bool    bNewObject = false;

    if( bText )
    {
        
        if( !pTO )
        {
            DBG_ASSERT( pOlView->isRecordingUndo(), "sd::OutlineViewShell::UpdateTitleObject(), no undo for model change!?" );
            pTO = pOlView->CreateTitleTextObject(pPage);
            bNewObject = true;
        }

        
        if( pTO )
        {
            pOPO = pOutliner->CreateParaObject( pOutliner->GetAbsPos( pPara ), 1 );
            pOPO->SetOutlinerMode( OUTLINERMODE_TITLEOBJECT );
            pOPO->SetVertical( pTO->IsVerticalWriting() );
            if( pTO->GetOutlinerParaObject() && (pOPO->GetTextObject() == pTO->GetOutlinerParaObject()->GetTextObject()) )
            {
                
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
        
        
        if(pPage->IsPresObj(pTO))
        {
            
            if( !pTO->IsEmptyPresObj() )
            {
                DBG_ASSERT( pOlView->isRecordingUndo(), "sd::OutlineViewShell::UpdateTitleObject(), no undo for model change!?" );

                
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

    ::Outliner*         pOutliner = pOlView->GetOutliner();
    OutlinerParaObject* pOPO = NULL;
    SdrTextObj*         pTO  = NULL;

    sal_Bool bNewObject = sal_False;

    sal_uInt16 eOutlinerMode = OUTLINERMODE_TITLEOBJECT;
    pTO = (SdrTextObj*)pPage->GetPresObj( PRESOBJ_TEXT );
    if( !pTO )
    {
        eOutlinerMode = OUTLINERMODE_OUTLINEOBJECT;
        pTO = pOlView->GetOutlineTextObject( pPage );
    }

    
    sal_Int32 nTitlePara     = pOutliner->GetAbsPos( pPara );
    sal_Int32 nPara          = nTitlePara + 1;
    sal_Int32 nParasInLayout = 0L;
    pPara = pOutliner->GetParagraph( nPara );
    while( pPara && !pOutliner->HasParaFlag(pPara, PARAFLAG_ISPAGE) )
    {
        nParasInLayout++;
        pPara = pOutliner->GetParagraph( ++nPara );
    }
    if( nParasInLayout )
    {
        
        pPara = pOutliner->GetParagraph( nTitlePara + 1 );
        pOPO  = pOutliner->CreateParaObject( nTitlePara + 1, nParasInLayout );
    }

    if( pOPO )
    {
        DBG_ASSERT( pOlView->isRecordingUndo(), "sd::OutlineViewShell::UpdateOutlineObject(), no undo for model change!?" );

        
        if( !pTO )
        {
            pTO = pOlView->CreateOutlineTextObject( pPage );
            bNewObject = sal_True;
        }

        
        
        if( pTO )
        {
            pOPO->SetVertical( pTO->IsVerticalWriting() );
            pOPO->SetOutlinerMode( eOutlinerMode );
            if( pTO->GetOutlinerParaObject() && (pOPO->GetTextObject() == pTO->GetOutlinerParaObject()->GetTextObject()) )
            {
                
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
        
        

        
        if( pPage->IsPresObj(pTO) )
        {
            if( !pTO->IsEmptyPresObj() )
            {
                DBG_ASSERT( pOlView->isRecordingUndo(), "sd::OutlineViewShell::UpdateOutlineObject(), no undo for model change!?" );

                
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

    ::Outliner* pOutl = pOlView->GetOutliner();

    {
    OutlineViewPageChangesGuard aGuard( pOlView );
    OutlineViewModelChangeGuard aGuard2( *pOlView );

    bRet = pOutl->Read( rInput, rBaseURL, eFormat, GetDocSh()->GetHeaderAttributes() );

    SdPage* pPage = GetDoc()->GetSdPage( GetDoc()->GetSdPageCount(PK_STANDARD) - 1, PK_STANDARD );;
    SfxStyleSheet* pTitleSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_TITLE );
    SfxStyleSheet* pOutlSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_OUTLINE );

    sal_Int32 nParaCount = pOutl->GetParagraphCount();
    if ( nParaCount > 0 )
    {
        for ( sal_Int32 nPara = 0; nPara < nParaCount; nPara++ )
        {
            pOlView->UpdateParagraph( nPara );

            sal_Int16 nDepth = pOutl->GetDepth( nPara );

            if( (nDepth == 0) || !nPara )
            {
                Paragraph* pPara = pOutl->GetParagraph( nPara );
                pOutl->SetDepth(pPara, -1);
                pOutl->SetParaFlag(pPara, PARAFLAG_ISPAGE);

                pOutl->SetStyleSheet( nPara, pTitleSheet );

                if( nPara ) 
                    pOlView->InsertSlideForParagraph( pPara );
            }
            else
            {
                pOutl->SetDepth( pOutl->GetParagraph( nPara ), nDepth - 1 );
                OUString aStyleSheetName = pOutlSheet->GetName();
                if (!aStyleSheetName.isEmpty())
                    aStyleSheetName = aStyleSheetName.copy(0, aStyleSheetName.getLength() - 1);
                aStyleSheetName += OUString::number( nDepth );
                SfxStyleSheetBasePool* pStylePool = GetDoc()->GetStyleSheetPool();
                SfxStyleSheet* pStyle = (SfxStyleSheet*) pStylePool->Find( aStyleSheetName, pOutlSheet->GetFamily() );
                DBG_ASSERT( pStyle, "AutoStyleSheetName - Style not found!" );
                if ( pStyle )
                    pOutl->SetStyleSheet( nPara, pStyle );
            }
        }
    }
    }

    pOutl->GetUndoManager().Clear();

    return( bRet );
}

void OutlineViewShell::WriteUserDataSequence ( ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& rSequence, sal_Bool bBrowse )
{
    WriteFrameViewData();

    ViewShell::WriteUserDataSequence( rSequence, bBrowse );
}

void OutlineViewShell::ReadUserDataSequence ( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& rSequence, sal_Bool bBrowse )
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
    
    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        switch (nWhich)
        {
            case SID_SEARCH_ITEM:
            case SID_SEARCH_OPTIONS:
                
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
    
    for (sal_uInt16 i=0; i<GetDoc()->GetSdPageCount(PK_STANDARD); i++)
        GetDoc()->SetSelected(
            GetDoc()->GetSdPage(i, PK_STANDARD),
            sal_False);
    GetDoc()->SetSelected (pPage, sal_True);

    DrawController& rController(GetViewShellBase().GetDrawController());
    rController.FireSelectionChangeListener();
    rController.FireSwitchCurrentPage (pPage);

    pOlView->SetActualPage(pPage);
}


} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
