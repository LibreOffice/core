/*************************************************************************
 *
 *  $RCSfile: outlnvsh.cxx,v $
 *
 *  $Revision: 1.55 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 15:00:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "OutlineViewShell.hxx"

#include "app.hrc"
#define ITEMID_HYPERLINK    SID_HYPERLINK_SETLINK
#ifndef _SVX_DLG_HYPERLINK_HXX //autogen
#include <svx/hyprlink.hxx>
#endif
#ifndef _SVX_TAB_HYPERLINK_HXX
#include <svx/hyperdlg.hxx>
#endif

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
#define ITEMID_FIELD    EE_FEATURE_FIELD
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
#ifndef _EEITEMID_HXX
#include <svx/eeitemid.hxx>
#endif
#ifndef _SFX_SRCHITEM_HXX
#include <sfx2/srchitem.hxx>
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
#ifndef SD_PREVIEW_WINDOW_HXX
#include "PreviewWindow.hxx"
#endif
#ifndef SD_PREVIEW_CHILD_WINDOW_HXX
#include "PreviewChildWindow.hxx"
#endif
#ifndef SD_FU_SLIDE_SHOW_HXX
#include "fuslshow.hxx"
#endif
#include "SdUnoOutlineView.hxx"
#ifndef SD_OBJECT_BAR_MANAGER_HXX
#include "ObjectBarManager.hxx"
#endif
#ifndef _SD_ACCESSIBILITY_ACCESSIBLE_OUTLINE_VIEW_HXX
#include "AccessibleOutlineView.hxx"
#endif
#include "PaneManager.hxx"
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif

using namespace ::rtl;
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
    SFX_CHILDWINDOW_REGISTRATION( PreviewChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxHlinkDlgWrapper::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SID_SEARCH_DLG );
}


TYPEINIT1( OutlineViewShell, ViewShell );


/*************************************************************************
|*
|* gemeinsamer Initialiserungsanteil der beiden Konstruktoren
|*
\************************************************************************/

void OutlineViewShell::Construct(DrawDocShell* pDocSh)
{
    BOOL bModified = GetDoc()->IsChanged();

    meShellType = ST_OUTLINE;
    Size aViewSize(Size(29700, 21000));
    Point aWinPos (0, 0);
    Point aViewOrigin(0, 0);
    GetActiveWindow()->SetMinZoomAutoCalc(FALSE);
    GetActiveWindow()->SetMinZoom( MIN_ZOOM );
    GetActiveWindow()->SetMaxZoom( MAX_ZOOM );
    InitWindows(aViewOrigin, aViewSize, aWinPos);
    pOlView = new OutlineView(GetDocSh(), GetActiveWindow(), this);
    mpView = pOlView;            // Pointer der Basisklasse ViewShell

    SetPool( &GetDoc()->GetPool() );

    SetZoom(69);

    // Activate the object bar.
    GetObjectBarManager().EnableObjectBarSwitching();
    GetObjectBarManager().SwitchObjectBar (RID_DRAW_TEXT_TOOLBOX);

    // Einstellungen der FrameView uebernehmen
    ReadFrameViewData(pFrameView);

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




DrawController* OutlineViewShell::GetController (void)
{
    if ( ! mpController.is() && IsMainViewShell())
    {
        // Create uno controller for the main view shell.  For the ones
        // displayed in the non-center panes we may later introduce
        // sub-controllers.
        DrawController* pController = new SdUnoOutlineView (
            GetViewShellBase(),
            *this,
            *GetView());
        mpController = ::comphelper::ImplementationReference<
        DrawController,
            ::com::sun::star::uno::XInterface,
            ::com::sun::star::uno::XWeak> (pController);
    }
    return mpController.get();
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
      bPastePossible(FALSE)
{
    if (pFrameViewArgument != NULL)
        pFrameView = pFrameViewArgument;
    else
        pFrameView = new FrameView(GetDoc());

    pFrameView->Connect();

    Construct(GetDocSh());

    // Editierfunktion starten
    SfxRequest aReq( SID_EDIT_OUTLINER, 0, GetDoc()->GetItemPool() );
    FuPermanent( aReq );
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
      bPastePossible(FALSE)
{
    pFrameView = new FrameView(GetDoc());
    pFrameView->Connect();

    Construct(GetDocSh());
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

OutlineViewShell::~OutlineViewShell()
{
    if (pFuActual)
    {
        if (pFuOld == pFuActual)
            pFuOld = NULL;

        pFuActual->Deactivate();
        delete pFuActual;
        pFuActual = NULL;
    }

    if (pFuOld)
    {
        delete pFuOld;
        pFuOld = NULL;
    }

    // The sub shell manager will be destroyed in a short time.
    // Disable the switching of object bars now anyway just in case
    // the object bars would access invalid data when switched.
    GetObjectBarManager().DisableObjectBarSwitching();

    delete pOlView;

    pFrameView->Disconnect();

    if ( pClipEvtLstnr )
    {
        pClipEvtLstnr->AddRemoveListener( GetActiveWindow(), FALSE );
        pClipEvtLstnr->ClearCallbackLink();     // #103849# prevent callback if another thread is waiting
        pClipEvtLstnr->release();
    }
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

    if (pFuActual)
    {
        pFuActual->Paint(rRect, pWin);
    }
}

void OutlineViewShell::ArrangeGUIElements ()
{
    // Retrieve the current size (thickness) of the scroll bars.  That is
    // the width of the vertical and the height of the horizontal scroll
    // bar.
    int nScrollBarSize =
        GetParentWindow()->GetSettings().GetStyleSettings().GetScrollBarSize();
    aScrBarWH = Size (nScrollBarSize, nScrollBarSize);

    ViewShell::ArrangeGUIElements ();

    long nSizeX = aViewSize.Width() - aScrBarWH.Width();
    long nSizeY = aViewSize.Height() - aScrBarWH.Height();

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
        break;

        case SID_RELOAD:
        {
            // Normale Weiterleitung an ViewFrame zur Ausfuehrung
            GetViewFrame()->ExecuteSlot(rReq);

            // Muss sofort beendet werden
            return;
        }
        break;

        case SID_PREVIEW_QUALITY_COLOR:
        case SID_PREVIEW_QUALITY_GRAYSCALE:
        case SID_PREVIEW_QUALITY_BLACKWHITE:
        case SID_PREVIEW_QUALITY_CONTRAST:
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
    pOlView->AddWin(pWin);
}




void OutlineViewShell::RemoveWindow (::sd::Window* pWin)
{
    pOlView->DelWin(pWin);
}




/*************************************************************************
|*
|* Activate(), beim ersten Aufruf erfolgt ein Update der Felder
|*
\************************************************************************/
void OutlineViewShell::Activate( BOOL bIsMDIActivate )
{
    ViewShell::Activate( bIsMDIActivate );
    pOlView->SetLinks();

    if( bIsMDIActivate )
    {
        OutlinerView* pOutlinerView = pOlView->GetViewByWindow( GetActiveWindow() );
        ::Outliner* pOutl = pOutlinerView->GetOutliner();
        pOutl->UpdateFields();

        SfxBoolItem aItem(SID_PREVIEW_WIN, pFrameView->IsShowPreviewInOutlineMode() != 0 );
        GetViewFrame()->GetDispatcher()->Execute(
            SID_PREVIEW_WIN, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L);
    }
}

/*************************************************************************
|*
|* Deactivate()
|*
\************************************************************************/
void OutlineViewShell::Deactivate( BOOL bIsMDIActivate )
{
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
    if (rSet.GetItemState(SID_RELOAD) != SFX_ITEM_UNKNOWN)
    {
        // "Letzte Version" vom SFx en/disablen lassen
        SfxViewFrame* pViewFrame = SFX_APP()->GetViewFrame();
        if (pViewFrame->ISA(SfxTopViewFrame))
        {
            pViewFrame->GetSlotState (SID_RELOAD, NULL, &rSet);
        }
        else        // MI sagt: kein MDIFrame --> disablen
        {
            rSet.DisableItem(SID_RELOAD);
        }
    }

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

    // #49150#: Qualitaet des Previewfensters aendern, falls vorhanden
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PREVIEW_QUALITY_COLOR ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PREVIEW_QUALITY_GRAYSCALE ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PREVIEW_QUALITY_BLACKWHITE ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PREVIEW_QUALITY_CONTRAST ) )
    {
        USHORT nId = PreviewChildWindow::GetChildWindowId();
        if( GetViewFrame()->GetChildWindow( nId ) )
        {
            ULONG nMode = pFrameView->GetPreviewDrawMode();
            rSet.Put( SfxBoolItem( SID_PREVIEW_QUALITY_COLOR, (BOOL)(nMode == PREVIEW_DRAWMODE_COLOR) ) );
            rSet.Put( SfxBoolItem( SID_PREVIEW_QUALITY_GRAYSCALE, (BOOL)(nMode == PREVIEW_DRAWMODE_GRAYSCALE) ) );
            rSet.Put( SfxBoolItem( SID_PREVIEW_QUALITY_BLACKWHITE, (BOOL)(nMode == PREVIEW_DRAWMODE_BLACKWHITE) ) );
            rSet.Put( SfxBoolItem( SID_PREVIEW_QUALITY_CONTRAST, (BOOL)(nMode == PREVIEW_DRAWMODE_CONTRAST) ) );
        }
        else
        {
            rSet.DisableItem( SID_PREVIEW_QUALITY_COLOR );
            rSet.DisableItem( SID_PREVIEW_QUALITY_GRAYSCALE );
            rSet.DisableItem( SID_PREVIEW_QUALITY_BLACKWHITE );
            rSet.DisableItem( SID_PREVIEW_QUALITY_CONTRAST );
        }
    }

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
            rSet.DisableItem( SID_TRANSLITERATE_HALFWIDTH );
            rSet.DisableItem( SID_TRANSLITERATE_FULLWIDTH );
            rSet.DisableItem( SID_TRANSLITERATE_HIRAGANA );
            rSet.DisableItem( SID_TRANSLITERATE_KATAGANA );
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
    switch ( nSlot )
    {
        case SID_CUT:
        {
            if (pFuActual)
            {
                pFuActual->DoCut();
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
            if (pFuActual)
            {
                pFuActual->DoCopy();
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
            if (pFuActual)
            {
                pFuActual->DoPaste();
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
                    KeyCode  aKCode(KEY_DELETE);
                    KeyEvent aKEvt( 0, aKCode );
                    pOutlView->PostKeyEvent(aKEvt);
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
            GetViewShellBase().GetPaneManager().HandleModeChangeSlot (
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
            if (pZoomList->IsPreviousPossible())
            {
                // Vorheriges ZoomRect einstellen
                SetZoomRect(pZoomList->GetPreviousZoomRect());
            }
            rReq.Done ();
        }
        break;

        case SID_ZOOM_NEXT:
        {
            if (pZoomList->IsNextPossible())
            {
                // Naechstes ZoomRect einstellen
                SetZoomRect(pZoomList->GetNextZoomRect());
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
            ImpSidUndo(FALSE, rReq);
        }
        break;
        case SID_REDO :
        {
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
    if (pFuActual)
    {
        if (pFuOld == pFuActual)
        {
            pFuOld = NULL;
        }

        pFuActual->Deactivate();
        delete pFuActual;
        pFuActual = NULL;
    }

    switch ( rReq.GetSlot() )
    {
        case SID_EDIT_OUTLINER:
        {
            // Empty the undo manager of the text object bar.
            SfxUndoManager* pUndoManager = NULL;
            SfxShell* pTextObjectBar =
                GetObjectBarManager().GetObjectBar (RID_DRAW_TEXT_TOOLBOX);
            if (pTextObjectBar != NULL)
                pUndoManager = pTextObjectBar->GetUndoManager();

            DBG_ASSERT(pUndoManager,
                "UndoManager of text object bar not found");
            if (pUndoManager != NULL)
                pUndoManager->Clear();

            ::Outliner* pOutl = pOlView->GetOutliner();
            pOutl->UpdateFields();

            pFuActual = new FuOutlineText(this,GetActiveWindow(),pOlView,GetDoc(),rReq);

            rReq.Done();
        }
        break;

      default:
      break;
    }

    if (pFuOld)
    {
        pFuOld->Deactivate();
        delete pFuOld;
        pFuOld = NULL;
    }

    if (pFuActual)
    {
        pFuActual->Activate();
        pFuOld = pFuActual;
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

    if ( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PRESENTATION ) )
    {
        SfxChildWindow* pPreviewChildWindow = GetViewFrame()->GetChildWindow(
            PreviewChildWindow::GetChildWindowId());
        PreviewWindow*  pPreviewWin = static_cast<PreviewWindow*>(
            pPreviewChildWindow ? pPreviewChildWindow->GetWindow() : NULL );
        FuSlideShow* pShow = pPreviewWin ? pPreviewWin->GetSlideShow() : NULL;

        if ( (pShow && pShow->IsInputLocked()) ||
             GetDocSh()->IsPreview() )
        {
            rSet.DisableItem( SID_PRESENTATION );
        }
    }

    // Vorlagenkatalog darf nicht aufgerufen werden
    rSet.DisableItem( SID_STYLE_CATALOG );

    rSet.Put(SfxBoolItem(SID_DIAMODE, FALSE));
    rSet.Put(SfxBoolItem(SID_DRAWINGMODE, FALSE));
    rSet.Put(SfxBoolItem(SID_OUTLINEMODE, TRUE));
    rSet.Put(SfxBoolItem(SID_NOTESMODE, FALSE));
    rSet.Put(SfxBoolItem(SID_HANDOUTMODE, FALSE));

    if (!pZoomList->IsNextPossible())
    {
       rSet.DisableItem(SID_ZOOM_NEXT);
    }
    if (!pZoomList->IsPreviousPossible())
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

    // 'Alles auswaehlen' zulassen?

    ::Outliner* pOutl = pOlView->GetOutliner();
    DBG_ASSERT(pOutl, "kein Outliner");
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

    // PreviewWindow
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PREVIEW_WIN ) )
    {
        USHORT nId = PreviewChildWindow::GetChildWindowId();
        rSet.Put( SfxBoolItem( SID_PREVIEW_WIN, GetViewFrame()->HasChildWindow( nId ) ) );
    }
    /*af This should not be called here but probably when the current
    pages changes.  This leads eventually to an Invalidate() call
    while in Update().
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PREVIEW_STATE ) )
    {
        BOOL bModified = GetDoc()->IsChanged();
        UpdatePreview( GetActualPage() );
        GetDoc()->SetChanged( bModified );
    }
    */

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

    USHORT nPage = pFrameView->GetSelectedPage();
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
    pFrameView->SetNoColors(bNoColor);
    pFrameView->SetNoAttribs( pOutl->IsFlatMode() );
    SdPage* pActualPage = pOlView->GetActualPage();
    DBG_ASSERT(pActualPage, "No current page");
    if( pActualPage )
        pFrameView->SetSelectedPage((pActualPage->GetPageNum() - 1) / 2);
}


/*************************************************************************
|*
|* SfxRequests fuer StatusBar bearbeiten
|*
\************************************************************************/

void OutlineViewShell::ExecStatusBar(SfxRequest& rReq)
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

    if (pWin == NULL && pFuActual)
    {
        bReturn = pFuActual->KeyInput(rKEvt);
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

                if (pTmplCommon && pTmplCommon->GetActualFamily() == SFX_STYLE_FAMILY_PSEUDO)
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

    /*
    SfxChildWindow* pPreviewChildWindow =
        SFX_APP()->GetChildWindow(SdPreviewChildWindow::GetChildWindowId());
    if (pPreviewChildWindow)
    {
        SdPreviewWin* pPreviewWin =
            (SdPreviewWin*)pPreviewChildWindow->GetWindow();
        if (pPreviewWin)
        {
            SdPage* pPage = GetActualPage();
            if( pPage != pLastPage )
            {
                pLastPage = pPage;
                USHORT  nPage = (pPage->GetPageNum() - 1) / 2; // Sdr --> Sd

                pPreviewWin->SetContext( GetDoc(), nPage, pFrameView );
            }
        }
    }
    */
}

/*************************************************************************
|*
|* Liefert die erste selektierte Seite zurueck.
|* Wenn nichts selektiert ist, wird die erste Seite zurueckgeliefert.
|*
\************************************************************************/

SdPage* OutlineViewShell::GetActualPage()
{
    /* Code, der das gleiche Ergebnis liefert, gibt es schon in der
       SdOutlinerView ! */
    /*
    Outliner* pOutl = pOlView->GetOutliner();
    DBG_ASSERT(pOutl, "kein Outliner");
    OutlinerView* pOutlinerView = pOlView->GetViewByWindow( pWindow );
    List* pList = pOutlinerView->CreateSelectionList();

    Paragraph* pSelPara = (Paragraph*)pList->First();
    USHORT nPara = 0;

    Paragraph* pPara = pOutl->First();
    BOOL bFound = pPara == pSelPara;
    while( !bFound )
    {
        BOOL bHasChilds = pOutl->HasChilds(pPara);
        if( bHasChilds )
        {
            pPara = pOutl->Next();
            while( pPara && pPara->GetDepth() != 0 && !bFound )
            {
                bFound = pPara == pSelPara;
                if( !bFound )
                    pPara = pOutl->Next();
            }
        }
        if( !bFound )
        {
            if( !bHasChilds )
                pPara = pOutl->Next();
            bFound = pPara == pSelPara;
            nPara++;
        }
    }
    delete pList;

    SdPage* pPage = GetDoc()->GetSdPage( nPara, PK_STANDARD );
    */
    SdPage* pPage = pOlView->GetActualPage();

    return( pPage );
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
    USHORT nLastPage;
    USHORT nLastUsedPage = -1;

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

/*************************************************************************
|*
|* Update Preview
|*
\************************************************************************/

void OutlineViewShell::UpdatePreview( SdPage* pPage, BOOL bInit )
{
    // vom ShowWindow der DiaShow?
    // ggfs. Preview den neuen Kontext mitteilen
    /*  SfxChildWindow* pPreviewChildWindow =
        GetViewFrame()->GetChildWindow(PreviewChildWindow::GetChildWindowId());
    if (pPreviewChildWindow)
    {
        PreviewWindow* pPreviewWin =
            static_cast<PreviewWindow*>(pPreviewChildWindow->GetWindow());
        if (pPreviewWin && ( bInit || pPreviewWin->GetDoc() == GetDoc() ) )
    */      {
            BOOL bNewObject = FALSE;

            OutlinerView* pOutlinerView = pOlView->GetViewByWindow( GetActiveWindow() );
            ::Outliner* pOutliner = pOutlinerView->GetOutliner();
            List* pList = pOutlinerView->CreateSelectionList();
            Paragraph* pPara = (Paragraph*)pList->First();
            delete pList;

            BOOL bNewPage = pPage != pLastPage;
            BOOL bTitleObject = pOutliner->GetDepth( (USHORT) pOutliner->GetAbsPos( pPara ) ) == 0;
            if( !bTitleObject )
                pPara = pOlView->GetPrevTitle( pPara );

            // #96551# handle both updates when its an OutlineView
            BOOL bOutlineView(FALSE);
            if(OUTLINERMODE_OUTLINEVIEW == pOutliner->GetMode())
                bOutlineView = TRUE;

            if( bTitleObject || bNewPage || bOutlineView )
            {
                /*********************************************************************
                |* Titeltextobjekt
                \********************************************************************/
                bNewObject = UpdateTitleObject( pPage, pPara );
            }
            if( !bTitleObject || bNewPage || bOutlineView )
            {
                /*********************************************************************
                |* Gliederungstextobjekt
                \********************************************************************/
                bNewObject |= UpdateLayoutObject( pPage, pPara );
            }

            if( bNewObject )
            {
                // das AutoLayout nochmal anwenden, damit neu eingefuegte Textobjekte
                // die richtige Position/Groesse bekommen
                pPage->SetAutoLayout(pPage->GetAutoLayout());
            }
            // In Preview neu darstellen (nur bei neuer Seite):
            if( bNewPage || bNewObject || bInit )
            {
                pLastPage = pPage;
                ViewShell::UpdatePreview( pPage, TRUE );
            }
            if (bNewPage)
                SetCurrentPage (pPage);
        }
    //  }
}

/*************************************************************************
|*
|* Update Title
|*
\************************************************************************/

BOOL OutlineViewShell::UpdateTitleObject( SdPage*   pPage, Paragraph* pPara )
{
    ::Outliner*             pOutliner = pOlView->GetOutliner();
    SdrTextObj*         pTO  = pOlView->GetTitleTextObject( pPage );
    OutlinerParaObject* pOPO = NULL;

    String  aTest( pOutliner->GetText( pPara ) );
    BOOL    bText = aTest.Len() > 0;
    BOOL    bNewObject = FALSE;

    // kein Seitenobjekt, Text im Outliner:
    // entspr. Seitenobjekt erzeugen und einfuegen
    if( !pTO && bText )
    {
        SfxStyleSheetBasePool* pSPool = GetDoc()->GetStyleSheetPool();

        // Titelvorlage
        String aFullName = pPage->GetLayoutName();
        String aSep( RTL_CONSTASCII_USTRINGPARAM( SD_LT_SEPARATOR ));
        USHORT n = aFullName.Search(aSep);
        n += aSep.Len();
        aFullName.Erase(n);
        aFullName += String (SdResId(STR_LAYOUT_TITLE));
        SfxStyleSheet* pTitleSheet = (SfxStyleSheet*)pSPool->
                                        Find(aFullName, SD_LT_FAMILY);
        DBG_ASSERT(pTitleSheet, "Titelvorlage nicht gefunden");


        pTO  = new SdrRectObj( OBJ_TITLETEXT );
        pOPO = pOutliner->CreateParaObject( (USHORT) pOutliner->GetAbsPos( pPara ), 1 );
        pOPO->SetOutlinerMode( OUTLINERMODE_TITLEOBJECT );
        pTO->SetOutlinerParaObject( pOPO );
        pTO->SetEmptyPresObj( FALSE );

        AutoLayout eLayout = pPage->GetAutoLayout();
        if( eLayout == AUTOLAYOUT_VERTICAL_TITLE_TEXT_CHART       ||
            eLayout == AUTOLAYOUT_VERTICAL_TITLE_VERTICAL_OUTLINE )
            pTO->SetVerticalWriting( TRUE );

        // als Praesentationsobjekt anmelden
        pTO->SetUserCall( pPage );
        pPage->InsertPresObj( pTO, PRESOBJ_TITLE );

        pPage->InsertObject( pTO );
                    // TRUE: DontRemoveHardAttr
        pTO->SetStyleSheet( pTitleSheet, TRUE );

        // Nur Objekt painten
        //pTO->SendRepaintBroadcast();
        bNewObject = TRUE;
    }
    // Seitenobjekt, Text im Outliner:
    // Titeltext uebernehmen
    else if( pTO && bText )
    {
        pOPO = pOutliner->CreateParaObject( (USHORT) pOutliner->GetAbsPos( pPara ), 1 );
        pOPO->SetOutlinerMode( OUTLINERMODE_TITLEOBJECT );
        pOPO->SetVertical( pTO->IsVerticalWriting() );
        pTO->SetOutlinerParaObject( pOPO );
        pTO->SetEmptyPresObj( FALSE );

        // Nur Objekt painten
        pTO->ActionChanged();
        // pTO->SendRepaintBroadcast();
    }

    return( bNewObject );
}

/*************************************************************************
|*
|* Update LayoutObject
|*
\************************************************************************/

BOOL OutlineViewShell::UpdateLayoutObject( SdPage* pPage, Paragraph* pPara )
{
    ::Outliner*             pOutliner = pOlView->GetOutliner();
    OutlinerParaObject* pOPO = NULL;
    SdrTextObj*         pTO  = NULL;

    BOOL bNewObject = FALSE;

    sal_uInt16 eOutlinerMode = OUTLINERMODE_TITLEOBJECT;
    pTO = (SdrTextObj*)pPage->GetPresObj( PRESOBJ_TEXT );
    if( !pTO )
    {
        eOutlinerMode = OUTLINERMODE_OUTLINEOBJECT;
        pTO = pOlView->GetLayoutTextObject( pPage );
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
    if( nParasInLayout == 0 )
        return( FALSE );

    // ein OutlinerParaObject erzeugen
    pPara = pOutliner->GetParagraph( nTitlePara + 1 );
    pOPO  = pOutliner->CreateParaObject( (USHORT) nTitlePara + 1, (USHORT) nParasInLayout );

    // kein Seitenobjekt, aber Gliederung im Outliner
    if( !pTO && pOPO )
    {
        pTO = new SdrRectObj( OBJ_OUTLINETEXT );
        pTO->SetEmptyPresObj( FALSE );

        // als Praesentationsobjekt anmelden
        pTO->SetUserCall( pPage );
        pPage->InsertPresObj( pTO, PRESOBJ_OUTLINE );

        pPage->InsertObject( pTO );
        pOPO->SetOutlinerMode( eOutlinerMode );
        pTO->SetOutlinerParaObject( pOPO );

        AutoLayout eLayout = pPage->GetAutoLayout();
        if( eLayout == AUTOLAYOUT_TITLE_VERTICAL_OUTLINE       ||
            eLayout == AUTOLAYOUT_VERTICAL_TITLE_VERTICAL_OUTLINE ||
            eLayout == AUTOLAYOUT_TITLE_VERTICAL_OUTLINE_CLIPART )
            pTO->SetVerticalWriting( TRUE );

        // Linien- und Fuellattribute der Standardvorlage hart
        // ueberschreiben
        SfxItemSet aTempAttr( GetDoc()->GetPool() );
        aTempAttr.Put( XLineStyleItem( XLINE_NONE ) );
        aTempAttr.Put( XFillStyleItem( XFILL_NONE ) );
        pTO->SetMergedItemSetAndBroadcast(aTempAttr);

        // Liste der Gliederungsvorlagen fuer Anmeldung als Listener
        String aName = pPage->GetLayoutName();
        String aFullName;
        SfxStyleSheet* pSheet = NULL;
        SfxStyleSheetBasePool* pStyleSheetPool = GetDoc()->GetStyleSheetPool();
        for (USHORT i = 1; i < 10; i++)
        {
            aFullName  = aName;
            aFullName += sal_Unicode(' ');
            aFullName += String::CreateFromInt32( (sal_Int32)i );
            pSheet = (SfxStyleSheet*) pStyleSheetPool->Find(aFullName, SD_LT_FAMILY);
            pTO->StartListening( *pSheet );

            if( i == 1 )
                pTO->SetStyleSheet( pSheet, TRUE );
        }

        bNewObject = TRUE;
    }
    // Seitenobjekt, Gliederungstext im Outliner:
    // Text uebernehmen
    else if( pTO && pOPO )
    {
        pOPO->SetVertical( pTO->IsVerticalWriting() );
        pOPO->SetOutlinerMode( eOutlinerMode );
        pTO->SetOutlinerParaObject( pOPO );
        pTO->SetEmptyPresObj( FALSE );

        // Nur Objekt painten
        pTO->ActionChanged();
        // pTO->SendRepaintBroadcast();
    }
    return( bNewObject );
}


/*************************************************************************
|*
|* Outliner aus Stream fuellen
|*
\************************************************************************/

ULONG OutlineViewShell::Read(SvStream& rInput, USHORT eFormat)
{
    ::Outliner* pOutl = pOlView->GetOutliner();

    ULONG bRet = pOutl->Read( rInput, eFormat, GetDocSh()->GetHeaderAttributes() );

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

    ReadFrameViewData( pFrameView );
}

void OutlineViewShell::VisAreaChanged(const Rectangle& rRect)
{
    ViewShell::VisAreaChanged( rRect );

    if (mpController.is() != NULL)
    {
        mpController->FireVisAreaChanged( rRect );
    }
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
    SdUnoOutlineView* pController (
        static_cast<SdUnoOutlineView*>(GetController()));

    // Adapt the selection of the model.
    for (USHORT i=0; i<GetDoc()->GetSdPageCount(PK_STANDARD); i++)
        GetDoc()->SetSelected(
            GetDoc()->GetSdPage(i, PK_STANDARD),
            FALSE);
    GetDoc()->SetSelected (pPage, TRUE);
    if (pController != NULL)
        pController->FireSelectionChangeListener();

    // Tell the controller to notify its listeners.
    if (pController != NULL)
        pController->FireSwitchCurrentPage (pPage);
}


} // end of namespace sd
