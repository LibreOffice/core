/*************************************************************************
 *
 *  $RCSfile: outlnvsh.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: willem.vandorp $ $Date: 2000-09-29 12:36:55 $
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
#include "app.hrc"
#define ITEMID_HYPERLINK    SID_HYPERLINK_SETLINK
#ifndef _SVX_DLG_HYPERLINK_HXX //autogen
#include <offmgr/hyprlink.hxx>
#endif
#ifndef _SVX_TAB_HYPERLINK_HXX
#include <svx/hyperdlg.hxx>
#endif

#ifndef _SOT_EXCHANGE_HXX //autogen
#include <sot/exchange.hxx>
#endif

#ifndef _SV_CLIP_HXX //autogen
#include <vcl/clip.hxx>
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
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC_XTHESAURUS_HPP_
#include <com/sun/star/linguistic/XThesaurus.hpp>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif

#ifndef _OUTLOBJ_HXX
#include <svx/outlobj.hxx>
#endif

#ifndef _SD_OPTSITEM_HXX
#include "optsitem.hxx"
#endif

#include "strings.hrc"
#include "glob.hrc"
#include "res_bmp.hrc"

#include "sdoutl.hxx"
#include "sdwindow.hxx"
#include "drtxtob.hxx"
#include "drawdoc.hxx"
#include "outlnvsh.hxx"
#include "sdresid.hxx"
#include "sdpage.hxx"
#include "fuoltext.hxx"
#include "frmview.hxx"
#include "zoomlist.hxx"
#include "stlsheet.hxx"
#include "preview.hxx"
#include "prevchld.hxx"
#include "fuslshow.hxx"

using namespace ::com::sun::star;

#define MIN_ZOOM           10       // Minimaler Zoomfaktor
#define MAX_ZOOM         1000       // Maximaler Zoomfaktor

/************************************************************************/


/*************************************************************************
|*
|* SFX-Slotmap und Standardinterface deklarieren
|*
\************************************************************************/

SFX_DECL_TYPE(13);

#define SdOutlineViewShell
#include "sdslots.hxx"

SFX_IMPL_INTERFACE(SdOutlineViewShell, SfxViewShell, SdResId(STR_OUTLINEVIEWSHELL))
{
    SFX_POPUPMENU_REGISTRATION( SdResId(RID_OUTLINE_POPUP) );
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_TOOLS | SFX_VISIBILITY_STANDARD |
                                SFX_VISIBILITY_FULLSCREEN | SFX_VISIBILITY_SERVER,
                                SdResId(RID_OUTLINE_TOOLBOX) );
    SFX_CHILDWINDOW_REGISTRATION( SfxTemplateDialogWrapper::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxHyperlinkDlgWrapper::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SdPreviewChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxHlinkDlgWrapper::GetChildWindowId() );
}

SFX_IMPL_VIEWFACTORY(SdOutlineViewShell, SdResId(STR_SPECIALVIEW) )
{
    SFX_VIEW_REGISTRATION(SdDrawDocShell);
}

TYPEINIT1( SdOutlineViewShell, SdViewShell );


/*************************************************************************
|*
|* gemeinsamer Initialiserungsanteil der beiden Konstruktoren
|*
\************************************************************************/

void SdOutlineViewShell::Construct(SdDrawDocShell* pDocSh)
{
    BOOL bModified = pDoc->IsChanged();

    Size aViewSize(Size(29700, 21000));
    Point aWinPos (0, 0);
    Point aViewOrigin(0, 0);
    aOutlineBtn.Check(TRUE);
    pWindow->SetMinZoomAutoCalc(FALSE);
    pWindow->SetMinZoom( MIN_ZOOM );
    pWindow->SetMaxZoom( MAX_ZOOM );
    InitWindows(aViewOrigin, aViewSize, aWinPos);
    pOlView = new SdOutlineView(pDocSh, pWindow, this);
    pView = pOlView;             // Pointer der Basisklasse SdViewShell

    SetPool( &pDoc->GetPool() );

    SetZoom(69);

    // Shells fuer Object Bars erzeugen
    SfxShell* pObjBarShell = new SdDrawTextObjectBar( this, pDoc->GetPool(), pOlView );
    aShellTable.Insert( RID_DRAW_TEXT_TOOLBOX, pObjBarShell );

    // ObjectBar einschalten
    SwitchObjectBar(RID_DRAW_TEXT_TOOLBOX);

    // Einstellungen der FrameView uebernehmen
    ReadFrameViewData(pFrameView);

    Outliner* pOutl = pOlView->GetOutliner();
    pOutl->SetUpdateMode(TRUE);

    if (!bModified)
    {
        pOutl->ClearModifyFlag();
    }

    pLastPage = GetActualPage();

    String aName( RTL_CONSTASCII_USTRINGPARAM( "OutlineViewShell" ));
    SetName (aName);

    SetHelpId( SD_IF_SDOUTLINEVIEWSHELL );
    pWindow->SetHelpId( SD_IF_SDOUTLINEVIEWSHELL );
    pWindow->SetUniqueId( SD_IF_SDOUTLINEVIEWSHELL );
}

/*************************************************************************
|*
|* Standard-Konstruktor, Fenster duerfen nicht automatisch zentrieren
|*
\************************************************************************/

SdOutlineViewShell::SdOutlineViewShell(SfxViewFrame* pFrame, SfxViewShell* pOldShell) :
    SdViewShell(pFrame, &pFrame->GetWindow(), FALSE),
    pOlView(NULL),
    pLastPage( NULL )
{
    if (pOldShell)
    {
        pFrameView = ( (SdViewShell*) pOldShell)->GetFrameView();
    }
    else
    {
        pFrameView = new FrameView(pDoc);
    }

    pFrameView->Connect();

    Construct(pDocSh);

    // Editierfunktion starten
    SfxRequest aReq( SID_EDIT_OUTLINER, 0, pDoc->GetItemPool() );
    FuPermanent( aReq );
}


/*************************************************************************
|*
|* Copy-Konstruktor
|*
\************************************************************************/

SdOutlineViewShell::SdOutlineViewShell(SfxViewFrame* pFrame,
                                 const SdOutlineViewShell& rShell) :
    SdViewShell(pFrame, rShell),
    pOlView(NULL),
    pLastPage( NULL )
{
    pFrameView = new FrameView(pDoc);
    pFrameView->Connect();

    Construct(pDocSh);
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

__EXPORT SdOutlineViewShell::~SdOutlineViewShell()
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

    RemoveSubShell();

    // Umschaltung der ObjectBarShells unterdruecken, sonst versucht die
    // sterbende Funktion eventuell die ObjectBars zu wechseln. Die
    // entsprechende Shell ist aber schon vom SFX vom Dispatcher-Stack
    // genommen worden.
    bObjectBarSwitchEnabled = FALSE;

    delete pOlView;

    pFrameView->Disconnect();
}

/*************************************************************************
|*
|* Paint-Methode: das Ereignis wird vom Fenster pWindow an
|* die Viewshell und die aktuelle Funktion weitergeleitet
|*
\************************************************************************/

void __EXPORT SdOutlineViewShell::Paint(const Rectangle& rRect, SdWindow* pWin)
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

/*************************************************************************
|*
|* Ersatz fuer AdjustPosSizePixel ab Sfx 248a
|*
\************************************************************************/

void __EXPORT SdOutlineViewShell::InnerResizePixel(const Point &rPos, const Size &rSize)
{
    SdViewShell::InnerResizePixel(rPos, rSize);
}

/*************************************************************************
|*
|* Ersatz fuer AdjustPosSizePixel ab Sfx 248a
|*
\************************************************************************/

void __EXPORT SdOutlineViewShell::OuterResizePixel(const Point &rPos, const Size &rSize)
{
    SdViewShell::OuterResizePixel(rPos, rSize);
}

/*************************************************************************
|*
|* View-Groesse (und Position bei OLE) aendern
|*
\************************************************************************/

void SdOutlineViewShell::AdjustPosSizePixel(const Point &rNewPos,
                                            const Size &rNewSize)
{
    SdViewShell::AdjustPosSizePixel(rNewPos, rNewSize);

    long nSizeX = rNewSize.Width() - aScrBarWH.Width();
    long nSizeY = rNewSize.Height() - aScrBarWH.Height();

    for (short nX = 0; nX < MAX_HSPLIT_CNT; nX++)
    {
        for (short nY = 0; nY < MAX_VSPLIT_CNT; nY++)
        {
            SdWindow* pWindow = pWinArray[nX][nY];

            if (pWindow)
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
    }
}

/*************************************************************************
|*
|* Horizontales Lineal erzeugen
|*
\************************************************************************/

SvxRuler* SdOutlineViewShell::CreateHRuler(SdWindow* pWin, BOOL bIsFirst)
{
    WinBits aWBits = WB_HSCROLL | WB_SVLOOK | WB_BORDER;
    USHORT  nFlags = SVXRULER_SUPPORT_TABS | SVXRULER_SUPPORT_BORDERS |
                     SVXRULER_SUPPORT_PARAGRAPH_MARGINS;

    if ( bIsFirst )
        aWBits |= WB_EXTRAFIELD;

    SvxRuler* pRuler = new SvxRuler(&GetViewFrame()->GetWindow(), pWin,
                            nFlags, GetViewFrame()->GetBindings(), aWBits);
    pRuler->SetExtraType(RULER_EXTRA_TAB, RULER_TAB_LEFT);
    return pRuler;
}

/*************************************************************************
|*
|* Horizontales Lineal aktualisieren
|*
\************************************************************************/

void SdOutlineViewShell::UpdateHRuler()
{
}

/*************************************************************************
|*
|* SfxRequests fuer Controller bearbeiten
|*
\************************************************************************/

void SdOutlineViewShell::ExecCtrl(SfxRequest &rReq)
{
    USHORT nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
        case SID_MAIL_SCROLLBODY_PAGEDOWN:
        {
            if (pFuActual)
            {
                pFuActual->ScrollStart();
                ScrollLines(0, -1);
                pFuActual->ScrollEnd();
            }
            rReq.Done();
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

        //#49150# macht auch hier sinn, die Preview Qualitaet zu aendern
        case SID_PREVIEW_QUALITY_COLOR:
        case SID_PREVIEW_QUALITY_GRAYSCALE:
        case SID_PREVIEW_QUALITY_BLACKWHITE:
        {
            ULONG nMode;
            switch(nSlot)
            {
            case SID_PREVIEW_QUALITY_COLOR:
                nMode = PREVIEW_DRAWMODE_COLOR;
                break;
            case SID_PREVIEW_QUALITY_GRAYSCALE:
                nMode = PREVIEW_DRAWMODE_GRAYSCALE;
                break;
            default: //case SID_PREVIEW_QUALITY_BLACKWHITE:
                nMode = PREVIEW_DRAWMODE_BLACKWHITE;
                break;
            }

            pFrameView->SetPreviewDrawMode( nMode );
            SdOptions* pOptions = SD_MOD()->GetSdOptions( pDoc->GetDocumentType() );
            pOptions->SetPreviewQuality( nMode );

            Invalidate();
            rReq.Done();
            break;
        }

        default:
        break;
    }
}

/*************************************************************************
|*
|* Activate(), beim ersten Aufruf erfolgt ein Update der Felder
|*
\************************************************************************/
void SdOutlineViewShell::Activate( BOOL bIsMDIActivate )
{
    SdViewShell::Activate( bIsMDIActivate );
    pOlView->SetLinks();

    if( bIsMDIActivate )
    {
        OutlinerView* pOutlinerView = pOlView->GetViewByWindow( pWindow );
        Outliner* pOutl = pOutlinerView->GetOutliner();
        pOutl->UpdateFields();

        SfxBoolItem aItem(SID_PREVIEW_WIN, pFrameView->IsShowPreviewInOutlineMode());
        GetViewFrame()->GetDispatcher()->Execute(
            SID_PREVIEW_WIN, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L);
    }
}

/*************************************************************************
|*
|* Deactivate()
|*
\************************************************************************/
void SdOutlineViewShell::Deactivate( BOOL bIsMDIActivate )
{
    pOlView->ResetLinks();
    SdViewShell::Deactivate( bIsMDIActivate );
}

/*************************************************************************
|*
|* Status von Controller-SfxSlots setzen
|*
\************************************************************************/
void __EXPORT SdOutlineViewShell::GetCtrlState(SfxItemSet &rSet)
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

        OutlinerView* pOLV = pOlView->GetViewByWindow(pWindow);

        if (pOLV)
        {
            const SvxFieldItem* pFieldItem = pOLV->GetFieldAtSelection();

            if (pFieldItem)
            {
                const SvxFieldData* pField = pFieldItem->GetField();

                if (pField->ISA(SvxURLField))
                {
                    aHLinkItem.SetName(((const SvxURLField*) pField)->GetRepresentation());
                    aHLinkItem.SetURL(((const SvxURLField*) pField)->GetURL());
                    aHLinkItem.SetTargetFrame(((const SvxURLField*) pField)->GetTargetFrame());
                }
            }
        }

        rSet.Put(aHLinkItem);
    }

    // #49150#: Qualitaet des Previewfensters aendern, falls vorhanden
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PREVIEW_QUALITY_COLOR ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PREVIEW_QUALITY_GRAYSCALE ) ||
        SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PREVIEW_QUALITY_BLACKWHITE ) )
    {
        USHORT nId = SdPreviewChildWindow::GetChildWindowId();
        if( GetViewFrame()->GetChildWindow( nId ) )
        {
            ULONG nMode = pFrameView->GetPreviewDrawMode();
            rSet.Put( SfxBoolItem( SID_PREVIEW_QUALITY_COLOR, (BOOL)(nMode == PREVIEW_DRAWMODE_COLOR) ) );
            rSet.Put( SfxBoolItem( SID_PREVIEW_QUALITY_GRAYSCALE, (BOOL)(nMode == PREVIEW_DRAWMODE_GRAYSCALE) ) );
            rSet.Put( SfxBoolItem( SID_PREVIEW_QUALITY_BLACKWHITE, (BOOL)(nMode == PREVIEW_DRAWMODE_BLACKWHITE) ) );
        }
        else
        {
            rSet.DisableItem( SID_PREVIEW_QUALITY_COLOR );
            rSet.DisableItem( SID_PREVIEW_QUALITY_GRAYSCALE );
            rSet.DisableItem( SID_PREVIEW_QUALITY_BLACKWHITE );
        }
    }

    if ( SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_MAIL_SCROLLBODY_PAGEDOWN) )
    {
        rSet.Put( SfxBoolItem( SID_MAIL_SCROLLBODY_PAGEDOWN, TRUE ) );
    }
}

/*************************************************************************
|*
|* SfxRequests fuer Support-Funktionen
|*
\************************************************************************/

void SdOutlineViewShell::FuSupport(SfxRequest &rReq)
{
    if( rReq.GetSlot() == SID_STYLE_FAMILY && rReq.GetArgs())
        pDocSh->SetStyleFamily(((SfxUInt16Item&)rReq.GetArgs()->Get( SID_STYLE_FAMILY )).GetValue());

    BOOL bPreviewState = FALSE;
    switch ( rReq.GetSlot() )
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
                OutlinerView* pOutlView = pOlView->GetViewByWindow(pWindow);
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
        {
            if (rReq.GetSlot()==SID_DRAWINGMODE)
            {
                pFrameView->SetPageKind(PK_STANDARD);
                GetViewFrame()->GetDispatcher()->Execute(SID_VIEWSHELL0,
                            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
            }
            else if (rReq.GetSlot()==SID_NOTESMODE)
            {
                pFrameView->SetPageKind(PK_NOTES);
                GetViewFrame()->GetDispatcher()->Execute(SID_VIEWSHELL0,
                            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
            }
            else if (rReq.GetSlot()==SID_HANDOUTMODE)
            {
                pFrameView->SetPageKind(PK_HANDOUT);
                GetViewFrame()->GetDispatcher()->Execute(SID_VIEWSHELL0,
                            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
            }
            rReq.Done();
        }
        break;

        case SID_DIAMODE:
            GetViewFrame()->GetDispatcher()->Execute(SID_VIEWSHELL1,
                        SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
            rReq.Done();
            break;

        case SID_OUTLINEMODE:
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
            pDoc->SetOnlineSpell(!pDoc->GetOnlineSpell());
            rReq.Done ();
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

void SdOutlineViewShell::FuPermanent(SfxRequest &rReq)
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
            // den an der Shell der TextObjectBar gesetzten UndoManager leeren
            SfxUndoManager* pUndoMgr = GetSubShell(0)->GetUndoManager();
            DBG_ASSERT(pUndoMgr, "UndoManager nicht gefunden");
            pUndoMgr->Clear();

            Outliner* pOutl = pOlView->GetOutliner();
            pOutl->UpdateFields();

            pFuActual = new FuOutlineText(this,pWindow,pOlView,pDoc,rReq);
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



/*************************************************************************
|*
|* Status (Enabled/Disabled) von Menue-SfxSlots setzen
|*
\************************************************************************/

void SdOutlineViewShell::GetMenuState( SfxItemSet &rSet )
{
    SdViewShell::GetMenuState(rSet);

    if (pDocSh->IsPreview())
    {
        aPresentationBtn.Disable();
    }
    else
    {
        aPresentationBtn.Enable();
    }

    if ( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PRESENTATION ) )
    {
        SfxChildWindow* pPreviewChildWindow = GetViewFrame()->GetChildWindow(SdPreviewChildWindow::GetChildWindowId());
        SdPreviewWin*   pPreviewWin = (SdPreviewWin*) ( pPreviewChildWindow ? pPreviewChildWindow->GetWindow() : NULL );
        FuSlideShow*    pShow = pPreviewWin ? pPreviewWin->GetSlideShow() : NULL;

        if ( (pShow && pShow->IsInputLocked()) ||
             pDocSh->IsPreview() )
        {
            rSet.DisableItem( SID_PRESENTATION );
            aPresentationBtn.Disable();
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
        if( pWindow->GetZoom() <= pWindow->GetMinZoom() || pDocSh->IsUIActive() )
            rSet.DisableItem( SID_ZOOM_IN );
        if( pWindow->GetZoom() >= pWindow->GetMaxZoom() || pDocSh->IsUIActive() )
            rSet.DisableItem( SID_ZOOM_OUT );
    }

    // 'Alles auswaehlen' zulassen?

    Outliner* pOutl = pOlView->GetOutliner();
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
    OutlinerView* pOutlinerView = pOlView->GetViewByWindow(pWindow);
    List* pList = pOutlinerView->CreateSelectionList();
    Paragraph* pPara = (Paragraph*)pList->First();

    USHORT nDepth;
    USHORT nTmpDepth = pOutl->GetDepth( pOutl->GetAbsPos( pPara ) );
    while (pPara)
    {
        nDepth = pOutl->GetDepth( pOutl->GetAbsPos( pPara ) );

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


    if ( pDocSh->GetActualFunction() )
    {
        rSet.ClearItem( SID_DRAWINGMODE );
        rSet.DisableItem( SID_DRAWINGMODE );
        aDrawBtn.Disable();

        rSet.ClearItem( SID_NOTESMODE );
        rSet.DisableItem( SID_NOTESMODE );
        aNotesBtn.Disable();

        rSet.ClearItem( SID_HANDOUTMODE );
        rSet.DisableItem( SID_HANDOUTMODE );
        aHandoutBtn.Disable();

        rSet.ClearItem( SID_OUTLINEMODE );
        rSet.DisableItem( SID_OUTLINEMODE );
        aOutlineBtn.Disable();

        rSet.ClearItem( SID_DIAMODE );
        rSet.DisableItem( SID_DIAMODE );
        aSlideBtn.Disable();
    }
    else
    {
        aDrawBtn.Enable();
        aNotesBtn.Enable();
        aHandoutBtn.Enable();
        aOutlineBtn.Enable();
        aSlideBtn.Enable();
    }

    if (Clipboard::GetFormatCount() == 0 ||
        (!Clipboard::HasFormat(FORMAT_STRING) &&
         !Clipboard::HasFormat(FORMAT_RTF)    &&
         !Clipboard::HasFormat(SOT_FORMATSTR_ID_HTML)))
    {
        rSet.DisableItem(SID_PASTE);
    }

    if (!pOlView->GetViewByWindow(pWindow)->HasSelection())
    {
        rSet.DisableItem(SID_CUT);
        rSet.DisableItem(SID_COPY);
    }

    if (pOlView->GetOutliner()->IsModified())
    {
        pDoc->SetChanged(TRUE);
    }

    // Da šberladen, muss hier der Status gesetzt werden
    if( !pDocSh->IsModified() )
    {
        rSet.DisableItem( SID_SAVEDOC );
    }

    if ( pDocSh->IsReadOnly() )
    {
        rSet.DisableItem( SID_AUTOSPELL_CHECK );
    }
    else
    {
        if (pDoc->GetOnlineSpell())
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
        USHORT nId = SdPreviewChildWindow::GetChildWindowId();
        rSet.Put( SfxBoolItem( SID_PREVIEW_WIN, GetViewFrame()->HasChildWindow( nId ) ) );
    }
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PREVIEW_STATE ) )
    {
        BOOL bModified = pDoc->IsChanged();
        UpdatePreview();
        pDoc->SetChanged( bModified );
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
        USHORT nCount = pDoc->GetSdPageCount(PK_STANDARD);
        pOlView->SetSelectedPages();

        while (i < nCount && bDisable)
        {
            SdPage* pPage = pDoc->GetSdPage(i, PK_STANDARD);

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
        USHORT nCount = pDoc->GetSdPageCount(PK_STANDARD);
        pOlView->SetSelectedPages();

        while (i < nCount && bDisable)
        {
            SdPage* pPage = pDoc->GetSdPage(i, PK_STANDARD);

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
            LanguageType eLang = pDoc->GetLanguage();
             uno::Reference< lang::XMultiServiceFactory > xMgr( ::utl::getProcessServiceFactory() );
             uno::Reference< linguistic::XThesaurus > xThes( xMgr->createInstance(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.linguistic.Thesaurus" ))),
                                                            uno::UNO_QUERY );

            lang::Locale aLocale;
            SvxLanguageToLocale( aLocale, eLang );
            if (!xThes.is() || eLang == LANGUAGE_NONE || !xThes->hasLocale(aLocale))
                rSet.DisableItem( SID_THESAURUS );
        }
    }

    // Starten der Praesentation moeglich?
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_PRESENTATION ) )
    {
        BOOL bDisable = TRUE;
        USHORT nCount = pDoc->GetSdPageCount( PK_STANDARD );

        for( USHORT i = 0; i < nCount && bDisable; i++ )
        {
            SdPage* pPage = pDoc->GetSdPage(i, PK_STANDARD);

            if( !pPage->IsExcluded() )
                bDisable = FALSE;
        }
        if( bDisable || pDocSh->IsPreview())
        {
            rSet.DisableItem( SID_PRESENTATION );
            aPresentationBtn.Disable();
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

long SdOutlineViewShell::VirtHScrollHdl(ScrollBar* pHScroll)
{
    short nX = 0, nY = 0;

    // Spalte der zu scrollenden Fenster bestimmen
    while ( nX < MAX_HSPLIT_CNT &&  pHScroll != pHScrlArray[nX] )
        ++nX;

    long   nThumb = pHScroll->GetThumbPos();
    long   nRange = pHScroll->GetRange().Len();
    double fX     = (double) nThumb / nRange;

    // alle Fenster der Spalte scrollen
    while ( nY < MAX_VSPLIT_CNT && pWinArray[nX][nY] )
    {
        Window*       pWin          = pWinArray[nX][nY++];
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
    }
    return 0;
}

/*************************************************************************
|*
|* wird gerufen, wenn ScrollBar benutzt wird
|*
\************************************************************************/

long SdOutlineViewShell::VirtVScrollHdl(ScrollBar* pVScroll)
{
    short nX = 0, nY = 0;

    // Zeile der zu scrollenden Fenster bestimmen
    while ( nY < MAX_VSPLIT_CNT &&  pVScroll != pVScrlArray[nY] )
        ++nY;

    long nThumb = pVScroll->GetThumbPos();
    long nRange = pVScroll->GetRange().Len();
    double fY = (double) nThumb / nRange;

    // alle Fenster der Zeile scrollen
    while ( nX < MAX_HSPLIT_CNT && pWinArray[nX][nY] )
    {
        Window*       pWin          = pWinArray[nX++][nY];
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
    }
    return 0;
}

/*************************************************************************
|*
|* PrepareClose, wird gerufen, wenn die Shell zestoert werden soll,
|* leitet den Aufruf an die View weiter
|*
\************************************************************************/

USHORT SdOutlineViewShell::PrepareClose( BOOL bUI, BOOL bForBrowsing )
{
    if( SdViewShell::PrepareClose(bUI, bForBrowsing) != TRUE )
        return FALSE;

    return pOlView == NULL || pOlView->PrepareClose(bUI);
}


/*************************************************************************
|*
|* Zoomen mit Zoomfaktor, OutlinerView informieren
|*
\************************************************************************/

void SdOutlineViewShell::SetZoom(long nZoom)
{
    SdViewShell::SetZoom(nZoom);

    for (short nX = 0; nX < MAX_HSPLIT_CNT; nX++)
    {
        for (short nY = 0; nY < MAX_VSPLIT_CNT; nY++)
        {
            SdWindow* pWindow = pWinArray[nX][nY];
            if (pWindow)
            {
                // OutputArea der OutlinerView aendern
                OutlinerView* pOutlinerView = pOlView->GetViewByWindow(pWindow);
                Rectangle aWin(Point(0,0), pWindow->GetOutputSizePixel());
                aWin = pWindow->PixelToLogic(aWin);
                pOutlinerView->SetOutputArea(aWin);
            }
        }
    }
}

/*************************************************************************
|*
|* Zoomen mit Zoomrechteck, OutlinerView informieren
|*
\************************************************************************/

void SdOutlineViewShell::SetZoomRect(const Rectangle& rZoomRect)
{
    SdViewShell::SetZoomRect(rZoomRect);

    for (short nX = 0; nX < MAX_HSPLIT_CNT; nX++)
    {
        for (short nY = 0; nY < MAX_VSPLIT_CNT; nY++)
        {
            SdWindow* pWindow = pWinArray[nX][nY];
            if (pWindow)
            {
                // OutputArea der OutlinerView aendern
                OutlinerView* pOutlinerView = pOlView->GetViewByWindow(pWindow);
                Rectangle aWin(Point(0,0), pWindow->GetOutputSizePixel());
                aWin = pWindow->PixelToLogic(aWin);
                pOutlinerView->SetOutputArea(aWin);
            }
        }
    }
}


/*************************************************************************
|*
|* Vorm Speichern das Model der Drawing Engine aktualisieren, dann den
|* Call weiterleiten an die ObjectShell.
|*
\************************************************************************/

void SdOutlineViewShell::Execute(SfxRequest& rReq)
{
    switch(rReq.GetSlot())
    {
        case SID_SAVEDOC:
        case SID_SAVEASDOC:
        {
            PrepareClose();
        }
    }

    ((SdDrawDocShell*)GetViewFrame()->GetObjectShell())->ExecuteSlot( rReq );
}

/*************************************************************************
|*
|* Read FrameViews data and set actual views data
|*
\************************************************************************/

void SdOutlineViewShell::ReadFrameViewData(FrameView* pView)
{
    Outliner* pOutl = pOlView->GetOutliner();

    if ( pView->IsNoAttribs() )
        pOutl->SetFlatMode( TRUE );   // Attribut-Darstellung ausschalten
    else
        pOutl->SetFlatMode( FALSE );  // Attribut-Darstellung einschalten

    ULONG nCntrl = pOutl->GetControlWord();

    if ( pView->IsNoColors() )
        pOutl->SetControlWord(nCntrl | EE_CNTRL_NOCOLORS);   // Farbansicht ausschalten
    else
        pOutl->SetControlWord(nCntrl & ~EE_CNTRL_NOCOLORS);  // Farbansicht einschalten
}



/*************************************************************************
|*
|* Write actual views data to FrameView
|*
\************************************************************************/

void SdOutlineViewShell::WriteFrameViewData()
{
    Outliner* pOutl = pOlView->GetOutliner();

    ULONG nCntrl = pOutl->GetControlWord();
    BOOL bNoColor = FALSE;
    if (nCntrl & EE_CNTRL_NOCOLORS)
        bNoColor = TRUE;
    pFrameView->SetNoColors(bNoColor);
    pFrameView->SetNoAttribs( pOutl->IsFlatMode() );
    pFrameView->SetSelectedPage((pOlView->GetActualPage()->GetPageNum() - 1) / 2);
}


/*************************************************************************
|*
|* SfxRequests fuer StatusBar bearbeiten
|*
\************************************************************************/

void SdOutlineViewShell::ExecStatusBar(SfxRequest& rReq)
{
}

/*************************************************************************
|*
|* Statuswerte der Statusbar zurueckgeben
|*
\************************************************************************/

void SdOutlineViewShell::GetStatusBarState(SfxItemSet& rSet)
{
    // Zoom-Item
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_ATTR_ZOOM ) )
    {
        SvxZoomItem* pZoomItem;
        UINT16 nZoom = (UINT16) pWindow->GetZoom();

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
    USHORT  nPageCount = pDoc->GetSdPageCount( PK_STANDARD );
    String  aPageStr, aLayoutStr;

    SdWindow*       pWin        = GetActiveWindow();
    OutlinerView*   pActiveView = pOlView->GetViewByWindow( pWin );
    Outliner*       pOutliner   = pOlView->GetOutliner();
    List*           pSelList    = (List*)pActiveView->CreateSelectionList();
    Paragraph*      pFirstPara  = (Paragraph*)pSelList->First();
    Paragraph*      pLastPara   = (Paragraph*)pSelList->Last();

    if( pOutliner->GetDepth( pOutliner->GetAbsPos( pFirstPara ) ) > 0 )
        pFirstPara = pOlView->GetPrevTitle( pFirstPara );

    if( pOutliner->GetDepth( pOutliner->GetAbsPos( pLastPara ) ) > 0 )
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
        SdrPage* pPage = pDoc->GetSdPage( (USHORT) nPos, PK_STANDARD );

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

void __EXPORT SdOutlineViewShell::Command( const CommandEvent& rCEvt, SdWindow* pWin )
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU )
    {
        pWindow->ReleaseMouse();

        OutlinerView* pOLV = pOlView->GetViewByWindow(pWindow);
        Point aPos(rCEvt.GetMousePosPixel());

        if (pOLV && pOLV->IsWrongSpelledWordAtPos(aPos))
        {
            // Popup fuer Online-Spelling
            Link aLink = LINK(pDoc, SdDrawDocument, OnlineSpellCallback);
            pOLV->ExecuteSpellPopup(aPos, &aLink);
        }
        else
        {
           GetViewFrame()->GetDispatcher()->ExecutePopup(SdResId(RID_OUTLINE_POPUP));
        }
    }
    else
    {
        SdViewShell::Command( rCEvt, pWin );

        // ggfs. Preview den neuen Kontext mitteilen
        Invalidate( SID_PREVIEW_STATE );

    }
}


/*************************************************************************
|*
|* Keyboard event
|*
\************************************************************************/

BOOL SdOutlineViewShell::KeyInput(const KeyEvent& rKEvt, SdWindow* pWin)
{
    BOOL bReturn = FALSE;

    if (pWin == NULL && pFuActual)
    {
        bReturn = pFuActual->KeyInput(rKEvt);
    }

    // nein, weiterleiten an Basisklasse
    else
    {
        bReturn = SdViewShell::KeyInput(rKEvt, pWin);
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

Size __EXPORT SdOutlineViewShell::GetOptimalSizePixel() const
{
    Size aResult(200, 200);
    if (pOlView)
    {
        Outliner* pOutliner = pOlView->GetOutliner();
        if (pOutliner)
        {
            Size aTemp = pOutliner->CalcTextSize();
            aTemp = pWindow->LogicToPixel(aTemp);
            aResult.Width() = Max(aResult.Width(), aTemp.Width());
            aResult.Height() = Max(aResult.Height(), aTemp.Height());
            if (4 * aResult.Height() > 3 * aResult.Width())
            {
                aResult.Height() = 3 * aResult.Width() / 4;
            }
        }
    }

    // und jetzt jetzt das Standardgelumpe draufaddieren
    aResult.Width()  += pVScrlArray[0]->GetSizePixel().Width();
    aResult.Height() += pHScrlArray[0]->GetSizePixel().Height();
        //!!! + System::GetMenuBarHeightPixel();  // statt Titlebar
    return aResult;
}


/*************************************************************************
|*
|* Text der Selektion zurueckgeben
|*
\************************************************************************/

String __EXPORT SdOutlineViewShell::GetSelectionText(BOOL bCompleteWords)
{
    String aStrSelection;
    Outliner* pOl = pOlView->GetOutliner();
    OutlinerView* pOutlinerView = pOlView->GetViewByWindow( pWindow );

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

BOOL SdOutlineViewShell::HasSelection(BOOL bText) const
{
    BOOL bReturn = FALSE;

    if (bText)
    {
        OutlinerView* pOutlinerView = pOlView->GetViewByWindow( pWindow );

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

void __EXPORT SdOutlineViewShell::GetAttrState( SfxItemSet& rSet )
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
                SfxStyleSheet* pStyleSheet = pOlView->GetViewByWindow(pWindow)->GetStyleSheet();

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
                SdWindow*     pActWin = GetActiveWindow();
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

void SdOutlineViewShell::MouseButtonUp(const MouseEvent& rMEvt, SdWindow* pWin)
{
    // Zuerst die Basisklasse
    SdViewShell::MouseButtonUp(rMEvt, pWin);

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

                pPreviewWin->SetContext( pDoc, nPage, pFrameView );
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

SdPage* SdOutlineViewShell::GetActualPage()
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

    SdPage* pPage = pDoc->GetSdPage( nPara, PK_STANDARD );
    */
    SdPage* pPage = pOlView->GetActualPage();

    return( pPage );
}


/*************************************************************************
|*
|* Retrieve range of marked pages
|*
\************************************************************************/

String SdOutlineViewShell::GetPageRangeString()
{
    SdWindow*      pWin             = GetActiveWindow();
    OutlinerView*  pActiveView      = pOlView->GetViewByWindow(pWin);
    Outliner*      pOutl            = pActiveView->GetOutliner();
    List*          pSelList         = (List*)pActiveView->CreateSelectionList();
    Paragraph*     pPara            = (Paragraph*)pSelList->First();

    String aStrPageRange;
    BOOL bFirstPageNo = TRUE;
    BOOL bOpenRange = FALSE;
    USHORT nLastPage;
    USHORT nLastUsedPage = -1;

    USHORT nPageCount = 0;
    for( USHORT n = 0; n< pDoc->GetPageCount(); n++ )
        if( ( (SdPage*)pDoc->GetPage( n ) )->GetPageKind() == PK_STANDARD )
            nPageCount++;

    while ( pPara )
    {
        if ( pOutl->GetDepth( pOutl->GetAbsPos( pPara ) ) > 0 )
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

void SdOutlineViewShell::UpdatePreview()
{
    // vom ShowWindow der DiaShow?
    // ggfs. Preview den neuen Kontext mitteilen
    SfxChildWindow* pPreviewChildWindow =
        GetViewFrame()->GetChildWindow(SdPreviewChildWindow::GetChildWindowId());
    if (pPreviewChildWindow)
    {
        SdPreviewWin* pPreviewWin =
            (SdPreviewWin*)pPreviewChildWindow->GetWindow();
        if (pPreviewWin && pPreviewWin->GetDoc() == pDoc)
        {
            SdPage* pPage = GetActualPage();
            BOOL bNewObject = FALSE;

            OutlinerView* pOutlinerView = pOlView->GetViewByWindow( pWindow );
            Outliner* pOutliner = pOutlinerView->GetOutliner();
            List* pList = pOutlinerView->CreateSelectionList();
            Paragraph* pPara = (Paragraph*)pList->First();
            delete pList;

            BOOL bNewPage = pPage != pLastPage;
            BOOL bTitleObject = pOutliner->GetDepth( pOutliner->GetAbsPos( pPara ) ) == 0;
            if( !bTitleObject )
                pPara = pOlView->GetPrevTitle( pPara );

            if( bTitleObject || bNewPage )
            {
                /*********************************************************************
                |* Titeltextobjekt
                \********************************************************************/
                bNewObject = UpdateTitleObject( pPage, pPara );
            }
            if( !bTitleObject || bNewPage )
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
            if( bNewPage || bNewObject )
            {
                pLastPage = pPage;
                USHORT  nPage = (pPage->GetPageNum() - 1) / 2; // Sdr --> Sd
                pPreviewWin->SetContext( pDoc, nPage, pFrameView );
            }
        }
    }
}

/*************************************************************************
|*
|* Update Title
|*
\************************************************************************/

BOOL SdOutlineViewShell::UpdateTitleObject( SdPage* pPage, Paragraph* pPara )
{
    Outliner*           pOutliner = pOlView->GetOutliner();
    SdrTextObj*         pTO  = pOlView->GetTitleTextObject( pPage );
    OutlinerParaObject* pOPO = NULL;

    String  aTest( pOutliner->GetText( pPara ) );
    BOOL    bText = aTest.Len() > 0;
    BOOL    bNewObject = FALSE;

    // kein Seitenobjekt, Text im Outliner:
    // entspr. Seitenobjekt erzeugen und einfuegen
    if( !pTO && bText )
    {
        SfxStyleSheetBasePool* pSPool = pDoc->GetStyleSheetPool();

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
        pOPO = pOutliner->CreateParaObject( pOutliner->GetAbsPos( pPara ), 1 );
        pOPO->SetOutlinerMode( OUTLINERMODE_TITLEOBJECT );
        pTO->SetOutlinerParaObject( pOPO );
        pTO->SetEmptyPresObj( FALSE );

        // als Praesentationsobjekt anmelden
        pTO->SetUserCall( pPage );
        List* pPresObjList = pPage->GetPresObjList();
        pPresObjList->Insert( pTO, LIST_APPEND );

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
        pOPO = pOutliner->CreateParaObject( pOutliner->GetAbsPos( pPara ), 1 );
        pOPO->SetOutlinerMode( OUTLINERMODE_TITLEOBJECT );
        pTO->SetOutlinerParaObject( pOPO );
        pTO->SetEmptyPresObj( FALSE );

        // Nur Objekt painten
        pTO->SendRepaintBroadcast();
    }
    return( bNewObject );
}

/*************************************************************************
|*
|* Update LayoutObject
|*
\************************************************************************/

BOOL SdOutlineViewShell::UpdateLayoutObject( SdPage* pPage, Paragraph* pPara )
{
    Outliner*           pOutliner = pOlView->GetOutliner();
    OutlinerParaObject* pOPO = NULL;
    SdrTextObj*         pTO  = NULL;

    BOOL bNewObject = FALSE;

    pTO = (SdrTextObj*)pPage->GetPresObj( PRESOBJ_TEXT );
    if( !pTO )
        pTO = pOlView->GetLayoutTextObject( pPage );

    // wieviele Absaetze in der Gliederung?
    ULONG nTitlePara     = pOutliner->GetAbsPos( pPara );
    ULONG nPara          = nTitlePara + 1;
    ULONG nParasInLayout = 0L;
    pPara = pOutliner->GetParagraph( nPara );
    while( pPara && pOutliner->GetDepth( pOutliner->GetAbsPos( pPara ) ) != 0 )
    {
        nParasInLayout++;
        pPara = pOutliner->GetParagraph( ++nPara );
    }
    if( nParasInLayout == 0 )
        return( FALSE );

    // ein OutlinerParaObject erzeugen
    pPara = pOutliner->GetParagraph( nTitlePara + 1 );
    pOPO  = pOutliner->CreateParaObject( nTitlePara + 1, nParasInLayout );

    // kein Seitenobjekt, aber Gliederung im Outliner
    if( !pTO && pOPO )
    {
        pTO = new SdrRectObj( OBJ_OUTLINETEXT );
        pTO->SetOutlinerParaObject( pOPO );
        pTO->SetEmptyPresObj( FALSE );

        // als Praesentationsobjekt anmelden
        pTO->SetUserCall( pPage );
        List* pPresObjList = pPage->GetPresObjList();
        pPresObjList->Insert( pTO, LIST_APPEND );

        pPage->InsertObject( pTO );

        // Linien- und Fuellattribute der Standardvorlage hart
        // ueberschreiben
        SfxItemSet aTempAttr( pDoc->GetPool() );
        aTempAttr.Put( XLineStyleItem( XLINE_NONE ) );
        aTempAttr.Put( XFillStyleItem( XFILL_NONE ) );
        pTO->SetAttributes( aTempAttr, FALSE );

        // als Listener anmelden
        /*
        for (USHORT i = 1; i < 10; i++)
        {
            SfxStyleSheet* pS = (SfxStyleSheet*)pOutlineStyleList->
                                                    GetObject( i );
            if (pS)
                pTO->StartListening( *pS );
        }*/
        bNewObject = TRUE;
    }
    // Seitenobjekt, Gliederungstext im Outliner:
    // Text uebernehmen
    else if( pTO && pOPO )
    {
        pTO->SetOutlinerParaObject( pOPO );
        pTO->SetEmptyPresObj( FALSE );

        // Nur Objekt painten
        pTO->SendRepaintBroadcast();
    }
    return( bNewObject );
}


/*************************************************************************
|*
|* Outliner aus Stream fuellen
|*
\************************************************************************/

ULONG SdOutlineViewShell::Read(SvStream& rInput, USHORT eFormat)
{
    Outliner* pOutl = pOlView->GetOutliner();
    BOOL bRet = pOutl->Read( rInput, eFormat, pDocSh->GetHeaderAttributes() );

    SdPage* pPage = pDoc->GetSdPage( pDoc->GetSdPageCount(PK_STANDARD) - 1, PK_STANDARD );;
    SfxStyleSheet* pTitleSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_TITLE );
    SfxStyleSheet* pOutlSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_OUTLINE );
    ULONG nParaCount = pOutl->GetParagraphCount();

    if ( nParaCount > 0 )
    {
        for ( ULONG nPara = 0; nPara < nParaCount; nPara++ )
        {
            USHORT nDepth = pOutl->GetDepth( nPara );

            if( nDepth == 0 )
            {
                pOutl->SetStyleSheet( nPara, pTitleSheet );
            }
            else
            {
                String aStyleSheetName( pOutlSheet->GetName() );
                aStyleSheetName.Erase( aStyleSheetName.Len() - 1, 1 );
                aStyleSheetName += String::CreateFromInt32( nDepth );
                SfxStyleSheetBasePool* pStylePool = pDoc->GetStyleSheetPool();
                SfxStyleSheet* pStyle = (SfxStyleSheet*) pStylePool->Find( aStyleSheetName, pOutlSheet->GetFamily() );
                DBG_ASSERT( pStyle, "AutoStyleSheetName - Style not found!" );
                if ( pStyle )
                    pOutl->SetStyleSheet( nPara, pStyle );
            }
        }
    }

    return( bRet );
}

