/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: outlnvsh.cxx,v $
 *
 *  $Revision: 1.87 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:58:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "OutlineViewShell.hxx"

#include "ViewShellImplementation.hxx"
#include <memory>

#include "app.hrc"
#ifndef _SVX_DLG_HYPERLINK_HXX //autogen
#include <svx/hyprlink.hxx>
#endif
#ifndef _SVX_TAB_HYPERLINK_HXX
#include <svx/hyperdlg.hxx>
#endif

#include <sfx2/objface.hxx>

#ifndef _SOT_EXCHANGE_HXX //autogen
#include <sot/exchange.hxx>
#endif
#ifndef _SVX_RULE_HXX //autogen
#include <svx/ruler.hxx>
#endif
#ifndef _SVX_ZOOMITEM_HXX //autogen
#include <svx/zoomitem.hxx>
#endif
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#ifndef _SVX_FLDITEM_HXX //autogen
#include <svx/flditem.hxx>
#endif
#ifndef _SFX_SHELL_HXX //autogen
#include <sfx2/shell.hxx>
#endif
#ifndef _SFX_TEMPLDLG_HXX //autogen
#include <sfx2/templdlg.hxx>
#endif
#ifndef _VIEWFAC_HXX //autogen
#include <sfx2/viewfac.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SVX_HLNKITEM_HXX //autogen
#include <svx/hlnkitem.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen
#include <svx/svdotext.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SV_SCRBAR_HXX //autogen
#include <vcl/scrbar.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _EDITSTAT_HXX //autogen
#include <svx/editstat.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _SFX_TPLPITEM_HXX //autogen
#include <sfx2/tplpitem.hxx>
#endif
#ifndef _SVDORECT_HXX //autogen
#include <svx/svdorect.hxx>
#endif
#ifndef _SOT_FORMATS_HXX //autogen
#include <sot/formats.hxx>
#endif
#ifndef _SFX_TOPFRM_HXX //autogen wg. SfxTopViewFrame
#include <sfx2/topfrm.hxx>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XTHESAURUS_HPP_
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_TRANSLITERATIONMODULES_HDL_
#include <com/sun/star/i18n/TransliterationModules.hdl>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _OUTLOBJ_HXX
#include <svx/outlobj.hxx>
#endif
#ifndef _SVTOOLS_CJKOPTIONS_HXX
#include <svtools/cjkoptions.hxx>
#endif
#ifndef _CLIPLISTENER_HXX
#include <svtools/cliplistener.hxx>
#endif
#ifndef _SFX_SRCHITEM_HXX
#include <sfx2/srchitem.hxx>
#endif

#ifndef _EDITOBJ_HXX
#include <svx/editobj.hxx>
#endif

#ifndef SD_FU_BULLET_HXX
#include "fubullet.hxx"
#endif

#ifndef _SD_OPTSITEM_HXX
#include "optsitem.hxx"
#endif

#include "strings.hrc"
#include "glob.hrc"
#include "res_bmp.hrc"

#ifndef SD_OUTLINER_HXX
#include "Outliner.hxx"
#endif
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#ifndef SD_TEXT_OBJECT_BAR_HXX
#include "TextObjectBar.hxx"
#endif
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "sdpage.hxx"
#ifndef SD_FU_OUTLINE_TEXT_HXX
#include "fuoltext.hxx"
#endif
#ifndef SD_FRAME_VIEW
#include "FrameView.hxx"
#endif
#include "zoomlist.hxx"
#include "stlsheet.hxx"
#ifndef _SD_SLIDESHOW_HXX
#include "slideshow.hxx"
#endif
#include "SdUnoOutlineView.hxx"
#include "SpellDialogChildWindow.hxx"

#ifndef _SD_ACCESSIBILITY_ACCESSIBLE_OUTLINE_VIEW_HXX
#include "AccessibleOutlineView.hxx"
#endif
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#include "ViewShellManager.hxx"
#include "DrawController.hxx"
#include "framework/FrameworkHelper.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

using namespace sd;
#define OutlineViewShell
#include "sdslots.hxx"

namespace sd {

#define MIN_ZOOM           10       // Minimaler Zoomfaktor
#define MAX_ZOOM         1000       // Maximaler Zoomfaktor

/************************************************************************/


/*************************************************************************
|*
|* SFX-Slotmap und Standardinterface deklarieren
|*
\************************************************************************/

SFX_DECL_TYPE(13);


SFX_IMPL_INTERFACE(OutlineViewShell, SfxShell, SdResId(STR_OUTLINEVIEWSHELL))
{
    SFX_POPUPMENU_REGISTRATION( SdResId(RID_OUTLINE_POPUP) );
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_TOOLS | SFX_VISIBILITY_STANDARD |
                                SFX_VISIBILITY_FULLSCREEN | SFX_VISIBILITY_SERVER,
                                SdResId(RID_OUTLINE_TOOLBOX) );
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_APPLICATION | SFX_VISIBILITY_DESKTOP | SFX_VISIBILITY_STANDARD | SFX_VISIBILITY_CLIENT | SFX_VISIBILITY_VIEWER | SFX_VISIBILITY_READONLYDOC,
                                SdResId(RID_DRAW_VIEWER_TOOLBOX) );
    SFX_CHILDWINDOW_REGISTRATION( SfxTemplateDialogWrapper::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxHyperlinkDlgWrapper::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxHlinkDlgWrapper::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( ::sd::SpellDialogChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SID_SEARCH_DLG );
}


TYPEINIT1( OutlineViewShell, ViewShell );


/*************************************************************************
|*
|* gemeinsamer Initialiserungsanteil der beiden Konstruktoren
|*
\************************************************************************/

void OutlineViewShell::Construct(DrawDocShell* )
{
    BOOL bModified = GetDoc()->IsChanged();

    meShellType = ST_OUTLINE;
    Size aSize(29700, 21000);
    Point aWinPos (0, 0);
    Point aViewOrigin(0, 0);
    GetActiveWindow()->SetMinZoomAutoCalc(FALSE);
    GetActiveWindow()->SetMinZoom( MIN_ZOOM );
    GetActiveWindow()->SetMaxZoom( MAX_ZOOM );
    InitWindows(aViewOrigin, aSize, aWinPos);
    pOlView = new OutlineView(GetDocSh(), GetActiveWindow(), this);
    mpView = pOlView;            // Pointer der Basisklasse ViewShell

    SetPool( &GetDoc()->GetPool() );

    SetZoom(69);

    // Einstellungen der FrameView uebernehmen
    ReadFrameViewData(mpFrameView);

    ::Outliner* pOutl = pOlView->GetOutliner();
    pOutl->SetUpdateMode(TRUE);

    if (!bModified)
    {
        pOutl->ClearModifyFlag();
    }

    pLastPage = GetActualPage();

    String aName( RTL_CONSTASCII_USTRINGPARAM( "OutlineViewShell" ));
    SetName (aName);

    SetHelpId( SD_IF_SDOUTLINEVIEWSHELL );
    GetActiveWindow()->SetHelpId( SD_IF_SDOUTLINEVIEWSHELL );
    GetActiveWindow()->SetUniqueId( SD_IF_SDOUTLINEVIEWSHELL );
}




::std::auto_ptr<DrawSubController> OutlineViewShell::CreateSubController (void)
{
    ::std::auto_ptr<DrawSubController> pResult;

    if (IsMainViewShell())
    {
        // Create uno controller for the main view shell.  For the ones
        // displayed in the non-center panes we may later introduce
        // sub-controllers.
        ViewShellBase& rBase (GetViewShellBase());
        pResult.reset(new SdUnoOutlineView (
            rBase.GetDrawController(),
            *this,
            *GetView()));
    }

    return pResult;
}




/*************************************************************************
|*
|* Standard-Konstruktor, Fenster duerfen nicht automatisch zentrieren
|*
\************************************************************************/

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
}


/*************************************************************************
|*
|* Copy-Konstruktor
|*
\************************************************************************/

OutlineViewShell::OutlineViewShell (
    SfxViewFrame* pFrame,
    ::Window* pParentWindow,
    const OutlineViewShell& rShell)
    : ViewShell (pFrame, pParentWindow, rShell),
      pOlView(NULL),
      pLastPage( NULL ),
      pClipEvtLstnr(NULL),
      bPastePossible(FALSE),
      mbInitialized(false)

{
    mpFrameView = new FrameView(GetDoc());
    mpFrameView->Connect();

    Construct(GetDocSh());
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

OutlineViewShell::~OutlineViewShell()
{
    DisposeFunctions();

    delete pOlView;

    mpFrameView->Disconnect();

    if ( pClipEvtLstnr )
    {
        pClipEvtLstnr->AddRemoveListener( GetActiveWindow(), FALSE );
        pClipEvtLstnr->ClearCallbackLink();     // #103849# prevent callback if another thread is waiting
        pClipEvtLstnr->release();
    }
}




void OutlineViewShell::Shutdown (void)
{
    ViewShell::Shutdown();

    PrepareClose();
}




/*************************************************************************
|*
|* Paint-Methode: das Ereignis wird vom Fenster pWindow an
|* die Viewshell und die aktuelle Funktion weitergeleitet
|*
\************************************************************************/

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
        pWindow->SetMinZoomAutoCalc(FALSE);

        // pWindow->SetPosSizePixel(rNewPos, Size(nSizeX, nSizeY));

        // OutputArea der OutlinerView aendern
        OutlinerView* pOutlinerView = pOlView->GetViewByWindow(pWindow);

        Rectangle aWin(Point(0,0), pWindow->GetOutputSizePixel());
        aWin = pWindow->PixelToLogic(aWin);
        pOutlinerView->SetOutputArea(aWin);

        Rectangle aVis = pOutlinerView->GetVisArea();

        Rectangle aText = Rectangle(Point(0,0),
            Size(pOlView->GetPaperWidth(),
                pOlView->GetOutliner()->GetTextHeight()));
        aText.Bottom() += aWin.GetHeight();

        if (!aWin.IsEmpty())            // nicht beim Oeffnen
        {
            InitWindows(Point(0,0), aText.GetSize(), Point(aVis.TopLeft()));
            UpdateScrollBars();
        }
    }
}

/*************************************************************************
|*
|* SfxRequests fuer Controller bearbeiten
|*
\************************************************************************/

void OutlineViewShell::ExecCtrl(SfxRequest &rReq)
{
    USHORT nSlot = rReq.GetSlot();
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




/*************************************************************************
|*
|* Activate(), beim ersten Aufruf erfolgt ein Update der Felder
|*
\************************************************************************/
void OutlineViewShell::Activate( BOOL bIsMDIActivate )
{
    if ( ! mbInitialized)
    {
        mbInitialized = true;
        SfxRequest aRequest (SID_EDIT_OUTLINER, 0, GetDoc()->GetItemPool());
        FuPermanent (aRequest);
    }

    ViewShell::Activate( bIsMDIActivate );
    pOlView->SetLinks();
    pOlView->ConnectToApplication();

    if( bIsMDIActivate )
    {
        OutlinerView* pOutlinerView = pOlView->GetViewByWindow( GetActiveWindow() );
        ::Outliner* pOutl = pOutlinerView->GetOutliner();
        pOutl->UpdateFields();
    }
}

/*************************************************************************
|*
|* Deactivate()
|*
\************************************************************************/
void OutlineViewShell::Deactivate( BOOL bIsMDIActivate )
{
    pOlView->DisconnectFromApplication();

    // #96416# Links must be kept also on deactivated viewshell, to allow drag'n'drop
    // to function properly
    // pOlView->ResetLinks();

    ViewShell::Deactivate( bIsMDIActivate );
}

/*************************************************************************
|*
|* Status von Controller-SfxSlots setzen
|*
\************************************************************************/
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
        rSet.Put( SfxBoolItem( SID_MAIL_SCROLLBODY_PAGEDOWN, TRUE ) );

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

/*************************************************************************
|*
|* SfxRequests fuer Support-Funktionen
|*
\************************************************************************/

void OutlineViewShell::FuSupport(SfxRequest &rReq)
{
    if( rReq.GetSlot() == SID_STYLE_FAMILY && rReq.GetArgs())
        GetDocSh()->SetStyleFamily(((SfxUInt16Item&)rReq.GetArgs()->Get( SID_STYLE_FAMILY )).GetValue());

    BOOL bPreviewState = FALSE;
    ULONG nSlot = rReq.GetSlot();

    std::auto_ptr< OutlineViewModelChangeGuard > aGuard;
    if( pOlView && (
        (nSlot == SID_TRANSLITERATE_UPPER) ||
        (nSlot == SID_TRANSLITERATE_LOWER) ||
        (nSlot == SID_TRANSLITERATE_HALFWIDTH) ||
        (nSlot == SID_TRANSLITERATE_FULLWIDTH) ||
        (nSlot == SID_TRANSLITERATE_HIRAGANA) ||
        (nSlot == SID_TRANSLITERATE_KATAGANA) ||
        (nSlot == SID_CUT) ||
//      (nSlot == SID_COPY) ||
        (nSlot == SID_PASTE) ||
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
            bPreviewState = TRUE;
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
            bPreviewState = TRUE;
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
            bPreviewState = TRUE;
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

                    FunctionReference xFunc( GetCurrentFunction() );
                    FuOutlineText* pFuOutlineText = dynamic_cast< FuOutlineText* >( xFunc.get() );
                    if( pFuOutlineText )
                        pFuOutlineText->UpdateForKeyPress (aKEvt);
                }
            }
            rReq.Done();
            bPreviewState = TRUE;
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
                // Vorheriges ZoomRect einstellen
                SetZoomRect(mpZoomList->GetPreviousZoomRect());
            }
            rReq.Done ();
        }
        break;

        case SID_ZOOM_NEXT:
        {
            if (mpZoomList->IsNextPossible())
            {
                // Naechstes ZoomRect einstellen
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
            bPreviewState = TRUE;
        }
        break;

        // #96090# added Undo/Redo handling
        case SID_UNDO :
        {
            OutlineViewPageChangesGuard aGuard2(pOlView);
            ImpSidUndo(FALSE, rReq);
        }
        break;
        case SID_REDO :
        {
            OutlineViewPageChangesGuard aGuard2(pOlView);
            ImpSidRedo(FALSE, rReq);
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

/*************************************************************************
|*
|* SfxRequests fuer permanente Funktionen
|*
\************************************************************************/

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
        rBindings.Invalidate( SID_PASTE2 );
        rBindings.Invalidate( SID_CLIPBOARD_FORMAT_ITEMS );
    }
    return 0;
}

/*************************************************************************
|*
|* Status (Enabled/Disabled) von Menue-SfxSlots setzen
|*
\************************************************************************/

void OutlineViewShell::GetMenuState( SfxItemSet &rSet )
{
    ViewShell::GetMenuState(rSet);

    // Vorlagenkatalog darf nicht aufgerufen werden
    rSet.DisableItem( SID_STYLE_CATALOG );

    rSet.Put(SfxBoolItem(SID_DIAMODE, FALSE));
    rSet.Put(SfxBoolItem(SID_DRAWINGMODE, FALSE));
    rSet.Put(SfxBoolItem(SID_OUTLINEMODE, TRUE));
    rSet.Put(SfxBoolItem(SID_NOTESMODE, FALSE));
    rSet.Put(SfxBoolItem(SID_HANDOUTMODE, FALSE));

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

    // 'Alles auswaehlen' zulassen?
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_SELECTALL ) )
    {
        ULONG nParaCount = pOutl->GetParagraphCount();
        BOOL bDisable = nParaCount == 0;
        if (!bDisable && nParaCount == 1)
        {
            String aTest( pOutl->GetText( pOutl->GetParagraph( 0 ) ) );
            if (aTest.Len() == 0)
            {
                bDisable = TRUE;
            }
        }
        if (bDisable)
            rSet.DisableItem(SID_SELECTALL);
    }

    // Status des Lineals setzen
    rSet.Put( SfxBoolItem( SID_RULER, HasRuler() ) );

    // Formatierung ein oder aus?
    rSet.Put( SfxBoolItem( SID_OUTLINE_FORMAT, !pOutl->IsFlatMode() ) );

    if( pOutl->IsFlatMode() )
        rSet.DisableItem( SID_COLORVIEW );
    else
    {
        // Farbansicht ein/aus
        ULONG nCntrl = pOutl->GetControlWord();
        BOOL bNoColor = FALSE;
        if (nCntrl & EE_CNTRL_NOCOLORS)
            bNoColor = TRUE;

        rSet.Put( SfxBoolItem( SID_COLORVIEW, bNoColor ) );
    }

    // Buttons der Werkzeugleiste
    // zunaechst selektionsabhaengige: COLLAPSE, EXPAND
    BOOL bDisableCollapse = TRUE;
    BOOL bDisableExpand   = TRUE;
    BOOL bUnique          = TRUE;
    OutlinerView* pOutlinerView = pOlView->GetViewByWindow(GetActiveWindow());
    List* pList = pOutlinerView->CreateSelectionList();
    Paragraph* pPara = (Paragraph*)pList->First();

    USHORT nDepth;
    USHORT nTmpDepth = pOutl->GetDepth( (USHORT) pOutl->GetAbsPos( pPara ) );
    while (pPara)
    {
        nDepth = pOutl->GetDepth( (USHORT) pOutl->GetAbsPos( pPara ) );

        if( nDepth != nTmpDepth )
            bUnique = FALSE;
        if (!pOutl->IsExpanded(pPara) && pOutl->HasChilds(pPara))
            bDisableExpand = FALSE;
        if (pOutl->IsExpanded(pPara) && pOutl->HasChilds(pPara))
            bDisableCollapse = FALSE;

        pPara = (Paragraph*)pList->Next();
    }

    delete pList;

    if (bDisableExpand)
        rSet.DisableItem(SID_OUTLINE_EXPAND);
    if (bDisableCollapse)
        rSet.DisableItem(SID_OUTLINE_COLLAPSE);

    // ergibt die Selektion ein eindeutiges Praesentationslayout?
    // wenn nicht, duerfen die Vorlagen nicht bearbeitet werden
    SfxItemSet aSet(*rSet.GetPool(), SID_STATUS_LAYOUT, SID_STATUS_LAYOUT);
    GetStatusBarState(aSet);
    String aTest(((SfxStringItem&)aSet.Get(SID_STATUS_LAYOUT)).GetValue());
    if (aTest.Len() == 0)
    {
        bUnique = FALSE;
        rSet.DisableItem(SID_PRESENTATION_TEMPLATES);
    }

    if (!bUnique)
        rSet.DisableItem( SID_PRESENTATIONOBJECT );

    // jetzt die selektionsunabhaengigen: COLLAPSE_ALL, EXPAND_ALL
    BOOL bDisableCollapseAll = TRUE;
    BOOL bDisableExpandAll   = TRUE;

    // wenn schon die Selektion etwas kollabierbares/expandierbares enthaelt
    if (!bDisableCollapse)
        bDisableCollapseAll = FALSE;
    if (!bDisableExpand)
        bDisableExpandAll = FALSE;

    // schade, so billig kommen wir nicht davon; alle Absaetze durchsuchen
    if (bDisableCollapseAll || bDisableExpandAll)
    {
        ULONG nParaPos = 0;
        pPara = pOutl->GetParagraph( nParaPos );
        while (pPara && (bDisableCollapseAll || bDisableExpandAll))
        {
            if (!pOutl->IsExpanded(pPara) && pOutl->HasChilds(pPara))
                bDisableExpandAll = FALSE;

            if (pOutl->IsExpanded(pPara) && pOutl->HasChilds(pPara))
                bDisableCollapseAll = FALSE;

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
            // create listener
            pClipEvtLstnr = new TransferableClipboardListener( LINK( this, OutlineViewShell, ClipboardChanged ) );
            pClipEvtLstnr->acquire();
            pClipEvtLstnr->AddRemoveListener( GetActiveWindow(), TRUE );

            // get initial state
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
        GetDoc()->SetChanged(TRUE);
    }

    // Da šberladen, muss hier der Status gesetzt werden
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
            rSet.Put(SfxBoolItem(SID_AUTOSPELL_CHECK, TRUE));
        }
        else
        {
            rSet.Put(SfxBoolItem(SID_AUTOSPELL_CHECK, FALSE));
        }
    }

    // Feldbefehle
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
        BOOL bDisable = TRUE;
        USHORT i = 0;
        USHORT nCount = GetDoc()->GetSdPageCount(PK_STANDARD);
        pOlView->SetSelectedPages();

        while (i < nCount && bDisable)
        {
            SdPage* pPage = GetDoc()->GetSdPage(i, PK_STANDARD);

            if (pPage->IsSelected())
            {
                SdrObject* pObj = pPage->GetPresObj(PRESOBJ_OUTLINE);

                if (pObj && !pObj->IsEmptyPresObj())
                {
                    bDisable = FALSE;
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
        BOOL bDisable = TRUE;
        USHORT i = 0;
        USHORT nCount = GetDoc()->GetSdPageCount(PK_STANDARD);
        pOlView->SetSelectedPages();

        while (i < nCount && bDisable)
        {
            SdPage* pPage = GetDoc()->GetSdPage(i, PK_STANDARD);

            if (pPage->IsSelected())
            {
                SdrObject* pObj = pPage->GetPresObj(PRESOBJ_TITLE);

                if (pObj && !pObj->IsEmptyPresObj())
                {
                    bDisable = FALSE;
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
            Locale                  aLocale;

            SvxLanguageToLocale( aLocale, eLang );

            if (!xThesaurus.is() || eLang == LANGUAGE_NONE || !xThesaurus->hasLocale(aLocale))
                rSet.DisableItem( SID_THESAURUS );
        }
    }

    // Starten der Praesentation moeglich?
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PRESENTATION ) )
    {
        BOOL bDisable = TRUE;
        USHORT nCount = GetDoc()->GetSdPageCount( PK_STANDARD );

        for( USHORT i = 0; i < nCount && bDisable; i++ )
        {
            SdPage* pPage = GetDoc()->GetSdPage(i, PK_STANDARD);

            if( !pPage->IsExcluded() )
                bDisable = FALSE;
        }
        if( bDisable || GetDocSh()->IsPreview())
        {
            rSet.DisableItem( SID_PRESENTATION );
        }
    }

    FuBullet::GetSlotState( rSet, this, GetViewFrame() );

    //rSet.DisableItem( SID_PRINTDOC );
    //rSet.DisableItem( SID_PRINTDOCDIRECT );
    //rSet.DisableItem( SID_SETUPPRINTER );
}

/*************************************************************************
|*
|* wird gerufen, wenn ScrollBar benutzt wird
|*
\************************************************************************/

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
    nViewWidth                  = Max(nViewWidth, nTextWidth);
    long          nCurrentPos   = pOutlinerView->GetVisArea().Left();
    long          nTargetPos    = (long)(fX * nViewWidth);
    long          nDelta        = nTargetPos - nCurrentPos;

    pOutlinerView->HideCursor();
    pOutlinerView->Scroll(-nDelta, 0);
    pOutlinerView->ShowCursor(FALSE);

    return 0;
}

/*************************************************************************
|*
|* wird gerufen, wenn ScrollBar benutzt wird
|*
\************************************************************************/

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
    pOutlinerView->ShowCursor(FALSE);

    return 0;
}

/*************************************************************************
|*
|* PrepareClose, wird gerufen, wenn die Shell zestoert werden soll,
|* leitet den Aufruf an die View weiter
|*
\************************************************************************/

USHORT OutlineViewShell::PrepareClose( BOOL bUI, BOOL bForBrowsing )
{
    if( ViewShell::PrepareClose(bUI, bForBrowsing) != TRUE )
        return FALSE;

    return pOlView == NULL || pOlView->PrepareClose(bUI);
}


/*************************************************************************
|*
|* Zoomen mit Zoomfaktor, OutlinerView informieren
|*
\************************************************************************/

void OutlineViewShell::SetZoom(long nZoom)
{
    ViewShell::SetZoom(nZoom);

    ::sd::Window* pWindow = mpContentWindow.get();
    if (pWindow)
    {
        // OutputArea der OutlinerView aendern
        OutlinerView* pOutlinerView = pOlView->GetViewByWindow(pWindow);
        Rectangle aWin(Point(0,0), pWindow->GetOutputSizePixel());
        aWin = pWindow->PixelToLogic(aWin);
        pOutlinerView->SetOutputArea(aWin);
    }

    // #106268#
    GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOM );
}

/*************************************************************************
|*
|* Zoomen mit Zoomrechteck, OutlinerView informieren
|*
\************************************************************************/

void OutlineViewShell::SetZoomRect(const Rectangle& rZoomRect)
{
    ViewShell::SetZoomRect(rZoomRect);

    ::sd::Window* pWindow = mpContentWindow.get();
    if (pWindow)
    {
        // OutputArea der OutlinerView aendern
        OutlinerView* pOutlinerView = pOlView->GetViewByWindow(pWindow);
        Rectangle aWin(Point(0,0), pWindow->GetOutputSizePixel());
        aWin = pWindow->PixelToLogic(aWin);
        pOutlinerView->SetOutputArea(aWin);
    }

    // #106268#
    GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOM );
}


/*************************************************************************
|*
|* Vorm Speichern das Model der Drawing Engine aktualisieren, dann den
|* Call weiterleiten an die ObjectShell.
|*
\************************************************************************/

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
            // Forward this request to the the common (old) code of the
            // document shell.
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

/*************************************************************************
|*
|* Read FrameViews data and set actual views data
|*
\************************************************************************/

void OutlineViewShell::ReadFrameViewData(FrameView* pView)
{
    ::Outliner* pOutl = pOlView->GetOutliner();

    if ( pView->IsNoAttribs() )
        pOutl->SetFlatMode( TRUE );   // Attribut-Darstellung ausschalten
    else
        pOutl->SetFlatMode( FALSE );  // Attribut-Darstellung einschalten

    ULONG nCntrl = pOutl->GetControlWord();

    if ( pView->IsNoColors() )
        pOutl->SetControlWord(nCntrl | EE_CNTRL_NOCOLORS);   // Farbansicht ausschalten
    else
        pOutl->SetControlWord(nCntrl & ~EE_CNTRL_NOCOLORS);  // Farbansicht einschalten

    USHORT nPage = mpFrameView->GetSelectedPage();
    pLastPage = GetDoc()->GetSdPage( nPage, PK_STANDARD );
    pOlView->SetActualPage(pLastPage);
}



/*************************************************************************
|*
|* Write actual views data to FrameView
|*
\************************************************************************/

void OutlineViewShell::WriteFrameViewData()
{
    ::Outliner* pOutl = pOlView->GetOutliner();

    ULONG nCntrl = pOutl->GetControlWord();
    BOOL bNoColor = FALSE;
    if (nCntrl & EE_CNTRL_NOCOLORS)
        bNoColor = TRUE;
    mpFrameView->SetNoColors(bNoColor);
    mpFrameView->SetNoAttribs( pOutl->IsFlatMode() );
    SdPage* pActualPage = pOlView->GetActualPage();
    DBG_ASSERT(pActualPage, "No current page");
    if( pActualPage )
        mpFrameView->SetSelectedPage((pActualPage->GetPageNum() - 1) / 2);
}


/*************************************************************************
|*
|* SfxRequests fuer StatusBar bearbeiten
|*
\************************************************************************/

void OutlineViewShell::ExecStatusBar(SfxRequest&)
{
}

/*************************************************************************
|*
|* Statuswerte der Statusbar zurueckgeben
|*
\************************************************************************/

void OutlineViewShell::GetStatusBarState(SfxItemSet& rSet)
{
    // Zoom-Item
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_ATTR_ZOOM ) )
    {
        SvxZoomItem* pZoomItem;
        UINT16 nZoom = (UINT16) GetActiveWindow()->GetZoom();

        pZoomItem = new SvxZoomItem( SVX_ZOOM_PERCENT, nZoom );

        // Bereich einschraenken
        USHORT nZoomValues = SVX_ZOOM_ENABLE_ALL;
        nZoomValues &= ~SVX_ZOOM_ENABLE_OPTIMAL;
        nZoomValues &= ~SVX_ZOOM_ENABLE_WHOLEPAGE;
        nZoomValues &= ~SVX_ZOOM_ENABLE_PAGEWIDTH;

        pZoomItem->SetValueSet( nZoomValues );
        rSet.Put( *pZoomItem );
        delete pZoomItem;
    }

    // Seitenanzeige und Layout
    /*
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_STATUS_PAGE ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_STATUS_LAYOUT ) )
    */
    USHORT  nPageCount = GetDoc()->GetSdPageCount( PK_STANDARD );
    String  aPageStr, aLayoutStr;

    ::sd::Window*       pWin        = GetActiveWindow();
    OutlinerView*   pActiveView = pOlView->GetViewByWindow( pWin );
    ::Outliner*       pOutliner   = pOlView->GetOutliner();
    List*           pSelList    = (List*)pActiveView->CreateSelectionList();
    Paragraph*      pFirstPara  = (Paragraph*)pSelList->First();
    Paragraph*      pLastPara   = (Paragraph*)pSelList->Last();

    if( pOutliner->GetDepth( (USHORT) pOutliner->GetAbsPos( pFirstPara ) ) > 0 )
        pFirstPara = pOlView->GetPrevTitle( pFirstPara );

    if( pOutliner->GetDepth( (USHORT) pOutliner->GetAbsPos( pLastPara ) ) > 0 )
        pLastPara = pOlView->GetPrevTitle( pLastPara );

    delete pSelList;                // die wurde extra fuer uns erzeugt

    // nur eine Seite selektiert?
    if( pFirstPara == pLastPara )
    {
        // wieviele Seiten sind vor der selektierten Seite?
        ULONG nPos = 0L;
        while( pFirstPara )
        {
            pFirstPara = pOlView->GetPrevTitle( pFirstPara );
            if( pFirstPara )
                nPos++;
        }

        if( nPos >= GetDoc()->GetSdPageCount( PK_STANDARD ) )
            nPos = 0;

        SdrPage* pPage = GetDoc()->GetSdPage( (USHORT) nPos, PK_STANDARD );

        aPageStr = String(SdResId( STR_SD_PAGE ));
        aPageStr += sal_Unicode(' ');
        aPageStr += String::CreateFromInt32( (sal_Int32)(nPos + 1) );   // ULONG -> sal_Int32
        aPageStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " / " ));
        aPageStr += String::CreateFromInt32( nPageCount );

        aLayoutStr = pPage->GetLayoutName();
        aLayoutStr.Erase( aLayoutStr.SearchAscii( SD_LT_SEPARATOR ) );
    }
    rSet.Put( SfxStringItem( SID_STATUS_PAGE, aPageStr ) );
    rSet.Put( SfxStringItem( SID_STATUS_LAYOUT, aLayoutStr ) );
}

/*************************************************************************
|*
|* Command event
|*
\************************************************************************/

void OutlineViewShell::Command( const CommandEvent& rCEvt, ::sd::Window* pWin )
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU )
    {
        GetActiveWindow()->ReleaseMouse();

        OutlinerView* pOLV = pOlView->GetViewByWindow(GetActiveWindow());
        Point aPos(rCEvt.GetMousePosPixel());

        if (pOLV && pOLV->IsWrongSpelledWordAtPos(aPos))
        {
            // #91457# Popup for Online-Spelling now handled by DrawDocShell
            // Link aLink = LINK(GetDoc(), SdDrawDocument, OnlineSpellCallback);
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

        // ggfs. Preview den neuen Kontext mitteilen
        Invalidate( SID_PREVIEW_STATE );

    }
}


/*************************************************************************
|*
|* Keyboard event
|*
\************************************************************************/

BOOL OutlineViewShell::KeyInput(const KeyEvent& rKEvt, ::sd::Window* pWin)
{
    BOOL bReturn = FALSE;
    OutlineViewPageChangesGuard aGuard(pOlView);

    if (pWin == NULL && HasCurrentFunction())
    {
        bReturn = GetCurrentFunction()->KeyInput(rKEvt);
    }

    // nein, weiterleiten an Basisklasse
    else
    {
        bReturn = ViewShell::KeyInput(rKEvt, pWin);
    }

    Invalidate(SID_STYLE_EDIT);
    Invalidate(SID_STYLE_NEW);
    Invalidate(SID_STYLE_DELETE);
    Invalidate(SID_STYLE_UPDATE_BY_EXAMPLE);
    Invalidate(SID_STYLE_NEW_BY_EXAMPLE);
    Invalidate(SID_STYLE_WATERCAN);
    Invalidate(SID_STYLE_FAMILY5);

    // Pruefen und Unterscheiden von CursorBewegungs- oder Eingabe-Keys
    KeyCode aKeyGroup( rKEvt.GetKeyCode().GetGroup() );
    if( aKeyGroup != KEYGROUP_CURSOR && aKeyGroup != KEYGROUP_FKEYS ||
        GetActualPage() != pLastPage )
    {
        Invalidate( SID_PREVIEW_STATE );
    }

    return(bReturn);
}


/*************************************************************************
|*
|* Optimale Groesse zurueckgeben
|*
\************************************************************************/

Size OutlineViewShell::GetOptimalSizePixel() const
{
    Size aResult(200, 200);
    if (pOlView)
    {
        ::Outliner* pOutliner = pOlView->GetOutliner();
        if (pOutliner)
        {
            Size aTemp = pOutliner->CalcTextSize();
            aTemp = GetActiveWindow()->LogicToPixel(aTemp);
            aResult.Width() = Max(aResult.Width(), aTemp.Width());
            aResult.Height() = Max(aResult.Height(), aTemp.Height());
            if (4 * aResult.Height() > 3 * aResult.Width())
            {
                aResult.Height() = 3 * aResult.Width() / 4;
            }
        }
    }

    // und jetzt jetzt das Standardgelumpe draufaddieren
    aResult.Width()  += mpVerticalScrollBar->GetSizePixel().Width();
    aResult.Height() += mpHorizontalScrollBar->GetSizePixel().Height();
        //!!! + System::GetMenuBarHeightPixel();  // statt Titlebar
    return aResult;
}


/*************************************************************************
|*
|* Text der Selektion zurueckgeben
|*
\************************************************************************/

String OutlineViewShell::GetSelectionText(BOOL bCompleteWords)
{
    String aStrSelection;
    ::Outliner* pOl = pOlView->GetOutliner();
    OutlinerView* pOutlinerView = pOlView->GetViewByWindow( GetActiveWindow() );

    if (pOl && pOlView)
    {
        if (bCompleteWords)
        {
            ESelection aSel = pOutlinerView->GetSelection();
            String aStrCurrentDelimiters = pOl->GetWordDelimiters();

            pOl->SetWordDelimiters( String( RTL_CONSTASCII_USTRINGPARAM( " .,;\"'" )));
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


/*************************************************************************
|*
|* Ist etwas selektiert?
|*
\************************************************************************/

BOOL OutlineViewShell::HasSelection(BOOL bText) const
{
    BOOL bReturn = FALSE;

    if (bText)
    {
        OutlinerView* pOutlinerView = pOlView->GetViewByWindow( GetActiveWindow() );

        if (pOutlinerView && pOutlinerView->GetSelected().Len() != 0)
        {
            bReturn = TRUE;
        }
    }

    return bReturn;
}


/*************************************************************************
|*
|* Status der Attribut-Items
|*
\************************************************************************/

void OutlineViewShell::GetAttrState( SfxItemSet& rSet )
{
    SfxWhichIter  aIter( rSet );
    USHORT        nWhich = aIter.FirstWhich();
    SfxAllItemSet aAllSet( *rSet.GetPool() );

    while ( nWhich )
    {
        USHORT nSlotId = SfxItemPool::IsWhich(nWhich)
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
                    SfxTemplateItem aItem( nWhich, String() );
                    aAllSet.Put( aItem, aItem.Which() );
                    // rSet.DisableItem( nWhich );
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
                    String aRealStyle(((SfxStringItem&) aSet.Get(SID_STATUS_LAYOUT)).GetValue());

                    if (!aRealStyle.Len())
                    {
                        // Kein eindeutiger Layoutname gefunden
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
                    // aufgespannte Selektion, also StyleSheet und/oder
                    // Attributierung nicht zwingend eindeutig
                    rSet.DisableItem(nWhich);
            }
            break;

            case SID_STYLE_NEW:
            case SID_STYLE_DELETE:
            case SID_STYLE_NEW_BY_EXAMPLE:
            case SID_STYLE_WATERCAN:
            {
                rSet.DisableItem(nWhich);
            }
            break;
        }

        nWhich = aIter.NextWhich();
    }

    rSet.Put( aAllSet, FALSE );
}



/*************************************************************************
|*
|* MouseButtonUp event
|*
\************************************************************************/

void OutlineViewShell::MouseButtonUp(const MouseEvent& rMEvt, ::sd::Window* pWin)
{
    // Zuerst die Basisklasse
    ViewShell::MouseButtonUp(rMEvt, pWin);

    Invalidate(SID_STYLE_EDIT);
    Invalidate(SID_STYLE_NEW);
    Invalidate(SID_STYLE_DELETE);
    Invalidate(SID_STYLE_UPDATE_BY_EXAMPLE);
    Invalidate(SID_STYLE_NEW_BY_EXAMPLE);
    Invalidate(SID_STYLE_WATERCAN);
    Invalidate(SID_STYLE_FAMILY5);

    // ggfs. Preview den neuen Kontext mitteilen
    if( GetActualPage() != pLastPage )
        Invalidate( SID_PREVIEW_STATE );
}



SdPage* OutlineViewShell::getCurrentPage() const
{
    // since there are no master pages in outline view, we can
    // for now use the GetActualPage method
    return const_cast<OutlineViewShell*>(this)->GetActualPage();
}

/*************************************************************************
|*
|* Liefert die erste selektierte Seite zurueck.
|* Wenn nichts selektiert ist, wird die erste Seite zurueckgeliefert.
|*
\************************************************************************/
SdPage* OutlineViewShell::GetActualPage()
{
    return pOlView->GetActualPage();
}


/*************************************************************************
|*
|* Retrieve range of marked pages
|*
\************************************************************************/

String OutlineViewShell::GetPageRangeString()
{
    ::sd::Window*      pWin             = GetActiveWindow();
    OutlinerView*  pActiveView      = pOlView->GetViewByWindow(pWin);
    ::Outliner*      pOutl            = pActiveView->GetOutliner();
    List*          pSelList         = (List*)pActiveView->CreateSelectionList();
    Paragraph*     pPara            = (Paragraph*)pSelList->First();

    String aStrPageRange;
    BOOL bFirstPageNo = TRUE;
    BOOL bOpenRange = FALSE;
    USHORT nLastPage = 0;
    USHORT nLastUsedPage = (USHORT)-1;

    USHORT nPageCount = 0;
    for( USHORT n = 0; n< GetDoc()->GetPageCount(); n++ )
        if( ( (SdPage*)GetDoc()->GetPage( n ) )->GetPageKind() == PK_STANDARD )
            nPageCount++;

    while ( pPara )
    {
        if ( pOutl->GetDepth( (USHORT) pOutl->GetAbsPos( pPara ) ) > 0 )
        {
            pPara = pOlView->GetPrevTitle(pPara);
        }
        USHORT nPageToSelect = 0;
        while(pPara)
        {
            pPara = pOlView->GetPrevTitle(pPara);
            if (pPara)
                nPageToSelect++;
        }

        if( bFirstPageNo )
        {
            bFirstPageNo = FALSE;
            aStrPageRange = String::CreateFromInt32( sal_Int32( nPageToSelect+1 ) );
            nLastUsedPage = nPageToSelect;
            nPageCount--;
        }
        else
        {
            if( nPageToSelect != nLastPage )
            {
                if( nPageToSelect == nLastPage+1 )
                {
                    bOpenRange = TRUE;
                    nPageCount--;
                }
                else
                {
                    if( bOpenRange )
                    {
                        if( nLastPage == nLastUsedPage+1 )
                            aStrPageRange.Append( sal_Unicode(',') );
                        else
                            aStrPageRange.Append( sal_Unicode('-') );

                        aStrPageRange.Append( String::CreateFromInt32( sal_Int32( nLastPage+1 ) ) );
                    }
                    aStrPageRange.Append( sal_Unicode(',') );
                    aStrPageRange.Append( String::CreateFromInt32( sal_Int32( nPageToSelect+1 ) ) );
                    nLastUsedPage = nPageToSelect;
                    bOpenRange = FALSE;
                    nPageCount--;
                }
            }
        }

        nLastPage = nPageToSelect;
        pPara = (Paragraph*)pSelList->Next();
    }

    if( bOpenRange )
    {
        if( nLastPage == nLastUsedPage+1 )
            aStrPageRange.Append( sal_Unicode(',') );
        else
            aStrPageRange.Append( sal_Unicode('-') );

        aStrPageRange.Append( String::CreateFromInt32( sal_Int32( nLastPage+1 ) ) );
    }

    if( nPageCount == 0 )
        aStrPageRange.Erase();

    delete pSelList;                // die wurde extra fuer uns erzeugt

    return aStrPageRange;
}

void OutlineViewShell::UpdatePreview( SdPage* pPage, BOOL )
{
    const bool bNewPage = pPage != pLastPage;
    pLastPage = pPage;
    if (bNewPage)
    {
        OutlineViewPageChangesGuard aGuard(pOlView);
        SetCurrentPage(pPage);
    }
}

/*************************************************************************
|*
|* Update Title
|*
\************************************************************************/

bool OutlineViewShell::UpdateTitleObject( SdPage* pPage, Paragraph* pPara )
{
    DBG_ASSERT( pPage, "sd::OutlineViewShell::UpdateTitleObject(), pPage == 0?" );
    DBG_ASSERT( pPara, "sd::OutlineViewShell::UpdateTitleObject(), pPara == 0?" );

    if( !pPage || !pPara )
        return false;

    ::Outliner*             pOutliner = pOlView->GetOutliner();
    SdrTextObj*         pTO  = pOlView->GetTitleTextObject( pPage );
    OutlinerParaObject* pOPO = NULL;

    String  aTest( pOutliner->GetText( pPara ) );
    bool    bText = aTest.Len() > 0;
    bool    bNewObject = false;

    if( bText )
    {
        // create a title object if we don't have one but have text
        if( !pTO )
        {
            DBG_ASSERT( pOlView->isRecordingUndo(), "sd::OutlineViewShell::UpdateTitleObject(), no undo for model change!?" );
            pTO = pOlView->CreateTitleTextObject(pPage);
            bNewObject = TRUE;
        }

        // if we have a title object and a text, set the text
        if( pTO )
        {
            pOPO = pOutliner->CreateParaObject( (USHORT) pOutliner->GetAbsPos( pPara ), 1 );
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
                pTO->SetEmptyPresObj( FALSE );
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
                pTO->SetEmptyPresObj(TRUE);
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

/*************************************************************************
|*
|* Update LayoutObject
|*
\************************************************************************/

bool OutlineViewShell::UpdateOutlineObject( SdPage* pPage, Paragraph* pPara )
{
    DBG_ASSERT( pPage, "sd::OutlineViewShell::UpdateOutlineObject(), pPage == 0?" );
    DBG_ASSERT( pPara, "sd::OutlineViewShell::UpdateOutlineObject(), pPara == 0?" );

    if( !pPage || !pPara )
        return false;

    ::Outliner*         pOutliner = pOlView->GetOutliner();
    OutlinerParaObject* pOPO = NULL;
    SdrTextObj*         pTO  = NULL;

    BOOL bNewObject = FALSE;

    sal_uInt16 eOutlinerMode = OUTLINERMODE_TITLEOBJECT;
    pTO = (SdrTextObj*)pPage->GetPresObj( PRESOBJ_TEXT );
    if( !pTO )
    {
        eOutlinerMode = OUTLINERMODE_OUTLINEOBJECT;
        pTO = pOlView->GetOutlineTextObject( pPage );
    }

    // wieviele Absaetze in der Gliederung?
    ULONG nTitlePara     = pOutliner->GetAbsPos( pPara );
    ULONG nPara          = nTitlePara + 1;
    ULONG nParasInLayout = 0L;
    pPara = pOutliner->GetParagraph( nPara );
    while( pPara && pOutliner->GetDepth( (USHORT) pOutliner->GetAbsPos( pPara ) ) != 0 )
    {
        nParasInLayout++;
        pPara = pOutliner->GetParagraph( ++nPara );
    }
    if( nParasInLayout )
    {
        // ein OutlinerParaObject erzeugen
        pPara = pOutliner->GetParagraph( nTitlePara + 1 );
        pOPO  = pOutliner->CreateParaObject( (USHORT) nTitlePara + 1, (USHORT) nParasInLayout );
    }

    if( pOPO )
    {
        DBG_ASSERT( pOlView->isRecordingUndo(), "sd::OutlineViewShell::UpdateOutlineObject(), no undo for model change!?" );

        // do we need an outline text object?
        if( !pTO )
        {
            pTO = pOlView->CreateOutlineTextObject( pPage );
            bNewObject = TRUE;
        }

        // Seitenobjekt, Gliederungstext im Outliner:
        // Text uebernehmen
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
                pTO->SetEmptyPresObj( FALSE );
                pTO->ActionChanged();
            }
        }
    }
    else if( pTO )
    {
        // Seitenobjekt, aber kein Gliederungstext:
        // wenn Objekt in Praesentationsliste der Seite ist -> Defaulttext,
        // sonst Objekt loeschen
        if( pPage->IsPresObj(pTO) )
        {
            if( !pTO->IsEmptyPresObj() )
            {
                DBG_ASSERT( pOlView->isRecordingUndo(), "sd::OutlineViewShell::UpdateOutlineObject(), no undo for model change!?" );

                // loescht auch altes OutlinerParaObject
                if( pOlView->isRecordingUndo() )
                    pOlView->AddUndo(GetDoc()->GetSdrUndoFactory().CreateUndoObjectSetText(*pTO,0));
                pPage->RestoreDefaultText( pTO );
                pTO->SetEmptyPresObj(TRUE);
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


/*************************************************************************
|*
|* Outliner aus Stream fuellen
|*
\************************************************************************/

ULONG OutlineViewShell::Read(SvStream& rInput, const String& rBaseURL, USHORT eFormat)
{
    OutlineViewPageChangesGuard aGuard( pOlView );

    ::Outliner* pOutl = pOlView->GetOutliner();

    ULONG bRet = pOutl->Read( rInput, rBaseURL, eFormat, GetDocSh()->GetHeaderAttributes() );

    SdPage* pPage = GetDoc()->GetSdPage( GetDoc()->GetSdPageCount(PK_STANDARD) - 1, PK_STANDARD );;
    SfxStyleSheet* pTitleSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_TITLE );
    SfxStyleSheet* pOutlSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_OUTLINE );
    ULONG nParaCount = pOutl->GetParagraphCount();

    if ( nParaCount > 0 )
    {
        for ( ULONG nPara = 0; nPara < nParaCount; nPara++ )
        {
            USHORT nDepth = pOutl->GetDepth( (USHORT) nPara );

            if( nDepth == 0 )
            {
                pOutl->SetStyleSheet( nPara, pTitleSheet );
            }
            else
            {
                String aStyleSheetName( pOutlSheet->GetName() );
                aStyleSheetName.Erase( aStyleSheetName.Len() - 1, 1 );
                aStyleSheetName += String::CreateFromInt32( nDepth );
                SfxStyleSheetBasePool* pStylePool = GetDoc()->GetStyleSheetPool();
                SfxStyleSheet* pStyle = (SfxStyleSheet*) pStylePool->Find( aStyleSheetName, pOutlSheet->GetFamily() );
                DBG_ASSERT( pStyle, "AutoStyleSheetName - Style not found!" );
                if ( pStyle )
                    pOutl->SetStyleSheet( nPara, pStyle );
            }
        }
    }

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
    <type>AccessibleDrawDocumentView</type>.  Otherwise delegate this call
    to the base class to return a default object (probably an empty
    reference).
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
    else
    {
        OSL_TRACE ("OutlineViewShell::CreateAccessibleDocumentView: no controller");
        return ViewShell::CreateAccessibleDocumentView (pWindow);
    }
}




void OutlineViewShell::GetState (SfxItemSet& rSet)
{
    // Iterate over all requested items in the set.
    SfxWhichIter aIter( rSet );
    USHORT nWhich = aIter.FirstWhich();
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
    for (USHORT i=0; i<GetDoc()->GetSdPageCount(PK_STANDARD); i++)
        GetDoc()->SetSelected(
            GetDoc()->GetSdPage(i, PK_STANDARD),
            FALSE);
    GetDoc()->SetSelected (pPage, TRUE);

    DrawController& rController(GetViewShellBase().GetDrawController());
    rController.FireSelectionChangeListener();
    rController.FireSwitchCurrentPage (pPage);

    pOlView->SetActualPage(pPage);
}


} // end of namespace sd
