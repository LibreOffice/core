/*************************************************************************
 *
 *  $RCSfile: outlview.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dl $ $Date: 2000-09-22 08:21:11 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
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

#ifndef _SFX_PROGRESS_HXX
#include <sfx2/progress.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include "eetext.hxx"       // definiert ITEMID_... fuer frmitems und textitem
#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif
#ifndef _EDITSTAT_HXX
#include <svx/editstat.hxx>
#endif
#ifndef _LRSPITEM_HXX
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen
#include <svx/svdotext.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _SFXSTBMGR_HXX //autogen
#include <sfx2/stbmgr.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _SVDORECT_HXX //autogen
#include <svx/svdorect.hxx>
#endif
#ifndef _SVDUNDO_HXX //autogen
#include <svx/svdundo.hxx>
#endif
#ifndef _SFXBRDCST_HXX //autogen
#include <svtools/brdcst.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_TSPTITEM_HXX
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVX_LSPCITEM_HXX
#include <svx/lspcitem.hxx>
#endif
#ifndef _OUTLOBJ_HXX //autogen
#include <svx/outlobj.hxx>
#endif

#pragma hdrstop

#include "docshell.hxx"
#include "drawdoc.hxx"
#include "sdwindow.hxx"
#include "outlview.hxx"
#include "sdpage.hxx"
#include "pres.hxx"
#include "outlnvsh.hxx"
#include "glob.hrc"
#include "sdresid.hxx"
#include "sdoutl.hxx"
#include "strings.hrc"

#pragma code_seg("STATICS")
static USHORT SidArray[] = {
                SID_OUTLINE_UP,
                SID_OUTLINE_DOWN,
                SID_OUTLINE_LEFT,
                SID_OUTLINE_RIGHT,
                SID_OUTLINE_COLLAPSE_ALL,
                SID_OUTLINE_COLLAPSE,
                SID_OUTLINE_EXPAND_ALL,
                SID_OUTLINE_EXPAND,
                0 };
#pragma code_seg()


// Breite: DIN A 4,  zwei Raender zu je 1 cm
#define OUTLINE_PAPERWIDTH 19000

// beim Seitenmanipulation Fortschrittsanzeige, wenn mehr Seiten betroffen
// sind als:
#define PROCESS_WITH_PROGRESS_THRESHOLD  5

struct SdParaAndPos
{
    Paragraph* pPara;
    USHORT     nPos;
};

TYPEINIT1( SdOutlineView, SdView );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

SdOutlineView::SdOutlineView(SdDrawDocShell* pDocSh, Window* pWindow,
                             SdOutlineViewShell* pOutlineViewSh)
    : SdView(pDocSh->GetDoc(), pWindow, pOutlineViewSh),
      pOutlineViewShell(pOutlineViewSh),
      pOutliner( pDoc->GetOutliner(TRUE) ),
      pOldParaOrder(NULL),
      pSelectedParas(NULL),
      bFirstPaint(TRUE),
      nPagesToProcess(0),
      nPagesProcessed(0),
      mpProgress(NULL)
{
    BOOL bInitOutliner = FALSE;

    if (pOutliner->GetViewCount() == 0)
    {
        // Outliner initialisieren: Referenz-Device setzen
        bInitOutliner = TRUE;
        pOutliner->Init( OUTLINERMODE_OUTLINEVIEW );
        SfxPrinter* pPrinter = pDocSh->GetPrinter(TRUE);
        pOutliner->SetRefDevice(pPrinter);
        ULONG nWidth = OUTLINE_PAPERWIDTH;
        pOutliner->SetPaperSize(Size(nWidth, 4000000000));
    }

    // View in Outliner einfuegen
    for (USHORT nView = 0; nView < MAX_OUTLINERVIEWS; nView++)
    {
        pOutlinerView[nView] = NULL;
    }

    pOutlinerView[0] = new OutlinerView(pOutliner, pWindow);
    Color aWhiteColor( COL_WHITE );
    pOutlinerView[0]->SetBackgroundColor( aWhiteColor );
    Rectangle aNullRect;
    pOutlinerView[0]->SetOutputArea(aNullRect);
    pOutliner->SetUpdateMode(FALSE);
    pOutliner->InsertView(pOutlinerView[0], LIST_APPEND);
    pWindow->SetBackground( Wallpaper( aWhiteColor ) );

    if (bInitOutliner)
    {
        // Outliner mit Inhalt fuellen
        FillOutliner();
    }

    pWindow->GrabFocus();
}

/*************************************************************************
|*
|* Destruktor, Links restaurieren, Outliner leeren
|*
\************************************************************************/

SdOutlineView::~SdOutlineView()
{
    if( mpProgress )
        delete mpProgress;

    // OutlinerViews abmelden und zerstoeren
    for (USHORT nView = 0; nView < MAX_OUTLINERVIEWS; nView++)
    {
        if (pOutlinerView[nView] != NULL)
        {
            pOutliner->RemoveView( pOutlinerView[nView] );
            delete pOutlinerView[nView];
            pOutlinerView[nView] = NULL;
        }
    }

    if (pOutliner->GetViewCount() == 0)
    {
        // Outliner deinitialisieren: Farbdarstellung einschalten
        ResetLinks();
        ULONG nCntrl = pOutliner->GetControlWord();
        pOutliner->SetUpdateMode(FALSE); // sonst wird bei SetControlWord gezeichnet
        pOutliner->SetControlWord(nCntrl & ~EE_CNTRL_NOCOLORS);
        pOutliner->Clear();
    }

    DBG_ASSERT(!pSelectedParas, "Absatzliste nicht geloescht");
    DBG_ASSERT(!pOldParaOrder, "Absatzliste nicht geloescht");
}

/*************************************************************************
|*
|* Paint-Methode
|*
\************************************************************************/

void SdOutlineView::Paint(const Rectangle& rRect, SdWindow* pWin)
{
    OutlinerView* pOlView = GetViewByWindow(pWin);

    if (pOlView)
    {
        pOlView->HideCursor();
        pOlView->Paint(rRect);
        pOlView->ShowCursor(bFirstPaint);
        bFirstPaint = FALSE;
    }
}

/*************************************************************************
|*
|* Fenster-Groesse hat sich geaendert
|*
\************************************************************************/

void SdOutlineView::AdjustPosSizePixel(const Point &rNewPos,
                                       const Size &rNewSize, SdWindow* pWindow)
{

}

/*************************************************************************
|*
|* ein Fenster hinzufuegen
|*
\************************************************************************/

void SdOutlineView::AddWin(SdWindow* pWin)
{
    BOOL bAdded = FALSE;
    BOOL bValidArea = FALSE;
    Rectangle aOutputArea;
    const Color aWhiteColor( COL_WHITE );
    USHORT nView = 0;

    while (nView < MAX_OUTLINERVIEWS && !bAdded)
    {
        if (pOutlinerView[nView] == NULL)
        {
            pOutlinerView[nView] = new OutlinerView(pOutliner, pWin);
            pOutlinerView[nView]->SetBackgroundColor( aWhiteColor );
            pOutliner->InsertView(pOutlinerView[nView], LIST_APPEND);
            bAdded = TRUE;

            if (bValidArea)
            {
                pOutlinerView[nView]->SetOutputArea(aOutputArea);
            }
        }
        else if (!bValidArea)
        {
            aOutputArea = pOutlinerView[nView]->GetOutputArea();
            bValidArea = TRUE;
        }

        nView++;
    }

    // weisser Hintergrund im Outliner
    pWin->SetBackground( Wallpaper( aWhiteColor ) );

    SdView::AddWin(pWin);
}

/*************************************************************************
|*
|* ein Fenster entfernen
|*
\************************************************************************/

void SdOutlineView::DelWin(SdWindow* pWin)
{
    BOOL bRemoved = FALSE;
    USHORT nView = 0;
    Window* pWindow;

    while (nView < MAX_OUTLINERVIEWS && !bRemoved)
    {
        if (pOutlinerView[nView] != NULL)
        {
            pWindow = pOutlinerView[nView]->GetWindow();

            if (pWindow == pWin)
            {
                pOutliner->RemoveView( pOutlinerView[nView] );
                delete pOutlinerView[nView];
                pOutlinerView[nView] = NULL;
                bRemoved = TRUE;
            }
        }

        nView++;
    }

    SdView::DelWin(pWin);
}

/*************************************************************************
|*
|* QueryDrop()
|*
\************************************************************************/

BOOL SdOutlineView::QueryDrop(DropEvent& rDEvt, SdWindow* pWindow,
                              USHORT nPage, USHORT nLayer)
{
    BOOL bResult          = FALSE;
    OutlinerView* pOlView = GetViewByWindow(pWindow);
    if (pOlView)
    {
        bResult = pOlView->QueryDrop(rDEvt);
    }
    return bResult;
}

/*************************************************************************
|*
|* Drop()
|*
\************************************************************************/

BOOL SdOutlineView::Drop(const DropEvent& rDEvt, SdWindow* pWindow,
                         USHORT nPage, USHORT nLayer)
{
    BOOL bResult          = FALSE;
    OutlinerView* pOlView = GetViewByWindow(pWindow);
    if (pOlView)
    {
        bResult = pOlView->Drop(rDEvt);
    }

    ( pOutlineViewShell ? pOutlineViewShell->GetViewFrame() : SfxViewFrame::Current() )->GetBindings().Invalidate( SidArray );

    return bResult;
}


/*************************************************************************
|*
|* Zeiger der dem Fenster entsprechenden OutlinerView zurueckgeben.
|*
\************************************************************************/

OutlinerView* SdOutlineView::GetViewByWindow(Window* pWin) const
{
    OutlinerView* pOlView = NULL;
    for (USHORT nView = 0; nView < MAX_OUTLINERVIEWS; nView++)
    {
        if (pOutlinerView[nView] != NULL)
        {
            if ( pWin == pOutlinerView[nView]->GetWindow() )
            {
                pOlView = pOutlinerView[nView];
            }
        }
    }
    return (pOlView);
}


/*************************************************************************
|*
|* Ermittelt den Titel vor einem beliebigen Absatz.
|*
\************************************************************************/

Paragraph* SdOutlineView::GetPrevTitle(const Paragraph* pPara)
{
    Paragraph* pResult = NULL;
    ULONG      nPos    = pOutliner->GetAbsPos((Paragraph*)pPara);

    if (nPos > 0)
    {
        while (nPos && !pResult)
        {
            pPara = pOutliner->GetParagraph(--nPos);
            if ( pOutliner->GetDepth( nPos ) == 0 )
            {
                pResult = (Paragraph*)pPara;
            }
        }

    }
    return pResult;
}

/*************************************************************************
|*
|* Ermittelt den Titel nach einem beliebigen Absatz.
|*
\************************************************************************/

Paragraph* SdOutlineView::GetNextTitle(const Paragraph* pPara)
{
    ULONG nPos = pOutliner->GetAbsPos((Paragraph*)pPara);

    Paragraph* pResult = (Paragraph*)pPara;
    do
    {
        pResult = pOutliner->GetParagraph(++nPos);
    }
    while (pResult && pOutliner->GetDepth( nPos ) != 0);

    if (pResult && pOutliner->GetDepth( nPos ) != 0)
    {
        pResult = NULL;
    }
    return pResult;
}

/*************************************************************************
|*
|* Handler fuer das Einfuegen von Seiten (Absaetzen)
|*
\************************************************************************/

IMPL_LINK( SdOutlineView, ParagraphInsertedHdl, Outliner *, pOutliner )
{
    Paragraph* pPara = pOutliner->GetHdlParagraph();

    if ( pOutliner->GetDepth( pOutliner->GetAbsPos( pPara ) ) == 0 )
    {
        // wieviele Titel sind vor dem neuen Titelabsatz?
        ULONG nExample = 0L;            // Position der "Vorbild"seite
        ULONG nTarget  = 0L;            // Einfuegeposition
        while(pPara)
        {
            pPara = GetPrevTitle(pPara);
            if (pPara)
                nTarget++;
        }


        // was der Outliner nicht kann, muss hier wieder wettgemacht werden:
        // wenn VOR dem ersten Absatz ein neuer Absatz mit RETURN erzeugt wird,
        // meldet der Outliner den bereits bestehenden (jetzt nach unten
        // gerutschten) Absatz als neuen Absatz; nicht darauf reinfallen!
        if (nTarget == 1)
        {
            String aTest(pOutliner->GetText( pOutliner->GetParagraph( 0 ) ));
            if (aTest.Len() == 0)
            {
                nTarget = 0;
            }
        }


        // "Vorbild"seite ist - wenn vorhanden - die Vorgaengerseite
        if (nTarget > 0)
        {
            nExample = nTarget - 1;
        }

        BegUndo();

        /**********************************************************************
        * Es wird stets zuerst eine Standardseite und dann eine
        * Notizseite erzeugt. Es ist sichergestellt, dass auf eine
        * Standardseite stets die zugehoerige Notizseite folgt.
        * Vorangestellt ist genau eine Handzettelseite
        **********************************************************************/

        // diese Seite hat Vorbildfunktion
        SdPage* pExample = (SdPage*)pDoc->GetSdPage((USHORT)nExample, PK_STANDARD);
        SdPage*    pPage = (SdPage*)pDoc->AllocPage(FALSE);

        pPage->SetLayoutName(pExample->GetLayoutName());

        // einfuegen (Seite)
        pDoc->InsertPage(pPage, (USHORT)(nTarget) * 2 + 1);
        AddUndo(new SdrUndoNewPage(*pPage));

        // der Standardseite eine Masterpage zuweisen
        USHORT nPgNum = pExample->GetMasterPageNum(0);
        pPage->InsertMasterPage(nPgNum);

        // Seitengroesse setzen
        pPage->SetSize(pExample->GetSize());
        pPage->SetBorder( pExample->GetLftBorder(),
                          pExample->GetUppBorder(),
                          pExample->GetRgtBorder(),
                          pExample->GetLwrBorder() );

        // neue Praesentationsobjekte anlegen (auf <Titel> oder
        // <Titel mit Untertitel> folgt <Titel mit Gliederung>, ansonsten
        // wird das Layout von der Vorgaengerseite uebernommen)
        AutoLayout eAutoLayout = pExample->GetAutoLayout();
        if (eAutoLayout == AUTOLAYOUT_TITLE ||
            eAutoLayout == AUTOLAYOUT_ONLY_TITLE)
        {
            pPage->SetAutoLayout(AUTOLAYOUT_ENUM, TRUE);
        }
        else
        {
            pPage->SetAutoLayout(pExample->GetAutoLayout(), TRUE);
        }

        /**********************************************************************
        |* jetzt die Notizseite
        \*********************************************************************/
        pExample = (SdPage*)pDoc->GetSdPage((USHORT)nExample, PK_NOTES);
        SdPage* pNotesPage = (SdPage*)pDoc->AllocPage(FALSE);

        pNotesPage->SetLayoutName(pExample->GetLayoutName());

        pNotesPage->SetPageKind(PK_NOTES);

        // einfuegen (Notizseite)
        pDoc->InsertPage(pNotesPage, (USHORT)(nTarget) * 2 + 2);
        AddUndo(new SdrUndoNewPage(*pNotesPage));

        // der Notizseite eine Masterpage zuweisen
        nPgNum = pExample->GetMasterPageNum(0);
        pNotesPage->InsertMasterPage(nPgNum);

        // Seitengroesse setzen, es muss bereits eine Seite vorhanden sein
        pNotesPage->SetSize(pExample->GetSize());
        pNotesPage->SetBorder( pExample->GetLftBorder(),
                               pExample->GetUppBorder(),
                               pExample->GetRgtBorder(),
                               pExample->GetLwrBorder() );

        // neue Praesentationsobjekte anlegen
        pNotesPage->SetAutoLayout(pExample->GetAutoLayout(), TRUE);

        EndUndo();

        SfxUndoManager* pDocUndoMgr = pDocSh->GetUndoManager();
        SfxLinkUndoAction* pLink = new SfxLinkUndoAction(pDocUndoMgr);
        pOutliner->GetUndoManager().AddUndoAction(pLink);

        pOutliner->UpdateFields();
    }

    return 0;
}

/*************************************************************************
|*
|* Handler fuer das Loeschen von Seiten (Absaetzen)
|*
\************************************************************************/

IMPL_LINK( SdOutlineView, ParagraphRemovingHdl, Outliner *, pOutliner )
{
    Paragraph* pPara = pOutliner->GetHdlParagraph();
    if ( pOutliner->GetDepth( pOutliner->GetAbsPos( pPara ) ) == 0 )
    {
        // wieviele Titel sind vor dem fraglichen Titelabsatz?
        ULONG nPos = 0L;
        while(pPara)
        {
            pPara = GetPrevTitle(pPara);
            if (pPara) nPos++;
        }

        // Seite und Notizseite loeschen
        BegUndo();

        USHORT nAbsPos = (USHORT)nPos * 2 + 1;
        SdrPage* pPage = pDoc->GetPage(nAbsPos);
        AddUndo(new SdrUndoDelPage(*pPage));
        pDoc->RemovePage(nAbsPos);

        nAbsPos = (USHORT)nPos * 2 + 1;
        pPage = pDoc->GetPage(nAbsPos);
        AddUndo(new SdrUndoDelPage(*pPage));
        pDoc->RemovePage(nAbsPos);

        EndUndo();

        SfxUndoManager* pDocUndoMgr = pDocSh->GetUndoManager();
        SfxLinkUndoAction* pLink = new SfxLinkUndoAction(pDocUndoMgr);
        pOutliner->GetUndoManager().AddUndoAction(pLink);

        // ggfs. Fortschrittsanzeige
        if (nPagesToProcess)
        {
            nPagesProcessed++;

            if(mpProgress)
                mpProgress->SetState(nPagesProcessed);

            if (nPagesProcessed == nPagesToProcess)
            {
                if(mpProgress)
                {
                    delete mpProgress;
                    mpProgress = NULL;
                }
                nPagesToProcess = 0;
                nPagesProcessed = 0;
            }
        }
        pOutliner->UpdateFields();
    }
    return 0;
}

/*************************************************************************
|*
|* Handler fuer das Aendern der Einruecktiefe von Absaetzen (macht ggfs.
|* das Einfuegen oder Loeschen von Seiten notwendig)
|*
\************************************************************************/

IMPL_LINK( SdOutlineView, DepthChangedHdl, Outliner *, pOutliner )
{
    Paragraph* pPara = pOutliner->GetHdlParagraph();
    if ( pOutliner->GetDepth( pOutliner->GetAbsPos( pPara ) ) == 0 )
    {
        // werden da etwa mehrere Level-1-Absaetze auf Level 0 gebracht und
        // wir sollten eine Fortschrittsanzeige oder Eieruhr aufsetzen und
        // haben es noch nicht getan?
        if (nPagesToProcess == 0)
        {
            Window*       pActWin = pOutlineViewShell->GetActiveWindow();
            OutlinerView* pOlView = GetViewByWindow(pActWin);
            List*         pList   = pOlView->CreateSelectionList();

            Paragraph*    pParagraph   = (Paragraph*)pList->First();
            while (pParagraph)
            {
                if ( pOutliner->GetDepth( pOutliner->GetAbsPos( pParagraph ) ) == 1 )
                    nPagesToProcess++;
                pParagraph = (Paragraph*)pList->Next();
            }

            nPagesToProcess++;  // der Absatz, der jetzt schon auf Level 0
                                // steht, gehoert auch dazu
            nPagesProcessed = 0;

            if (nPagesToProcess > PROCESS_WITH_PROGRESS_THRESHOLD)
            {
                if( mpProgress )
                    delete mpProgress;

                const String aStr(SdResId(STR_CREATE_PAGES));
                mpProgress = new SfxProgress( GetDocSh(), aStr, nPagesToProcess );
            }
            else
            {
                pDocSh->SetWaitCursor( TRUE );
            }
            delete pList;
        }

        ParagraphInsertedHdl(pOutliner);

        nPagesProcessed++;

        // muss eine Fortschrittsanzeige gepflegt werden?
        if (nPagesToProcess > PROCESS_WITH_PROGRESS_THRESHOLD)
        {
            if (mpProgress)
                mpProgress->SetState(nPagesProcessed);
        }

        // war das die letzte Seite?
        if (nPagesProcessed == nPagesToProcess)
        {
            if (nPagesToProcess > PROCESS_WITH_PROGRESS_THRESHOLD && mpProgress)
            {
                delete mpProgress;
                mpProgress = NULL;
            }
            else
                pDocSh->SetWaitCursor( FALSE );

            nPagesToProcess = 0;
            nPagesProcessed = 0;
        }
        pOutliner->UpdateFields();
    }
    else if (pOutliner->GetPrevDepth() == 0)
    {
        // wieviele Titel sind vor dem fraglichen Titelabsatz?
        ULONG nPos = 0L;
        Paragraph* pParagraph = pPara;
        while(pParagraph)
        {
            pParagraph = GetPrevTitle(pParagraph);
            if (pParagraph)
                nPos++;
        }
        // Seite und Notizseite loeschen
        BegUndo();

        USHORT nAbsPos = (USHORT)nPos * 2 + 1;
        SdrPage* pPage = pDoc->GetPage(nAbsPos);
        AddUndo(new SdrUndoDelPage(*pPage));
        pDoc->RemovePage(nAbsPos);

        nAbsPos = (USHORT)nPos * 2 + 1;
        pPage = pDoc->GetPage(nAbsPos);
        AddUndo(new SdrUndoDelPage(*pPage));
        pDoc->RemovePage(nAbsPos);

        EndUndo();

        SfxUndoManager* pDocUndoMgr = pDocSh->GetUndoManager();
        SfxLinkUndoAction* pLink = new SfxLinkUndoAction(pDocUndoMgr);
        pOutliner->GetUndoManager().AddUndoAction(pLink);

        // ggfs. Fortschrittsanzeige
        if (nPagesToProcess)
        {
            nPagesProcessed++;
            if (mpProgress)
                mpProgress->SetState(nPagesProcessed);

            if (nPagesProcessed == nPagesToProcess)
            {
                if(mpProgress)
                {
                    delete mpProgress;
                    mpProgress = NULL;
                }
                nPagesToProcess = 0;
                nPagesProcessed = 0;
            }
        }
        pOutliner->UpdateFields();
    }
    else if ( (pOutliner->GetPrevDepth() == 1) && ( pOutliner->GetDepth( pOutliner->GetAbsPos( pPara ) ) == 2 ) )
    {
        // wieviele Titel sind vor dem fraglichen Titelabsatz?
        ULONG nPos = -1L;

        Paragraph* pParagraph = pPara;
        while(pParagraph)
        {
            pParagraph = GetPrevTitle(pParagraph);
            if (pParagraph)
                nPos++;
        }

        if(0 <= nPos)
        {
            SdPage*pPage = (SdPage*)pDoc->GetSdPage(nPos, PK_STANDARD);

            if(pPage && pPage->GetPresObj(PRESOBJ_TEXT))
                pOutliner->SetDepth( pPara, 1 );
        }

    }

    // wieviele Titel sind vor dem fraglichen Titelabsatz?
    ULONG nPos = -1L;

    Paragraph* pTempPara = pPara;
    while(pTempPara)
    {
        pTempPara = GetPrevTitle(pTempPara);
        if (pTempPara)
            nPos++;
    }

    if( 0 <= nPos)
    {
        SdPage* pPage = (SdPage*) pDoc->GetSdPage( nPos, PK_STANDARD );

        if( pPage )
        {
            SfxStyleSheet* pStyleSheet = NULL;
            ULONG nPara = pOutliner->GetAbsPos( pPara );
            ULONG nDepth = pOutliner->GetDepth( nPara );
            if( nDepth == 0 )
            {
                pStyleSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_TITLE );
            }
            else if( pPage->GetPresObj(PRESOBJ_TEXT) )
            {
                pStyleSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_TEXT );
            }
            else
            {
                pStyleSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_OUTLINE );

                if( nDepth > 1 )
                {
                    String aNewStyleSheetName( pStyleSheet->GetName() );
                    aNewStyleSheetName.Erase( aNewStyleSheetName.Len()-1, 1 );
                    aNewStyleSheetName += String::CreateFromInt32( nDepth );
                    SfxStyleSheetBasePool* pStylePool = pDoc->GetStyleSheetPool();
                    pStyleSheet = (SfxStyleSheet*) pStylePool->Find( aNewStyleSheetName, pStyleSheet->GetFamily() );
                }
            }

            pOutliner->SetStyleSheet( nPara, pStyleSheet );
        }
    }

    return 0;
}

/*************************************************************************
|*
|* Handler fuer StatusEvents
|*
\************************************************************************/

IMPL_LINK( SdOutlineView, StatusEventHdl, EditStatus *, pEditStatus )
{
    SdWindow*     pWin          = pOutlineViewShell->GetActiveWindow();
    OutlinerView* pOutlinerView = GetViewByWindow(pWin);
    Rectangle     aVis          = pOutlinerView->GetVisArea();

//    ULONG nWidth = ((SdPage*)pDoc->GetSdPage(0, PK_STANDARD))->GetSize().Width();
    ULONG nWidth = OUTLINE_PAPERWIDTH;
    Rectangle aText = Rectangle(Point(0,0),
                                   Size(nWidth,
                                        pOutliner->GetTextHeight()));
    Rectangle aWin(Point(0,0), pWin->GetOutputSizePixel());
    aWin = pWin->PixelToLogic(aWin);

    if (!aVis.IsEmpty())        // nicht beim Oeffnen
    {
        aText.Bottom() += aWin.GetHeight();

        pOutlineViewShell->InitWindows(Point(0,0), aText.GetSize(),
                                       Point(aVis.TopLeft()));
        pOutlineViewShell->UpdateScrollBars();
    }
    return 0;
}

/*************************************************************************
|*
|* Handler fuer den Beginn einer Absatzverschiebung
|*
\************************************************************************/

IMPL_LINK( SdOutlineView, BeginMovingHdl, Outliner *, pOutliner )
{
    DBG_ASSERT(!pSelectedParas, "Absatzliste nicht geloescht");
    DBG_ASSERT(!pOldParaOrder, "Absatzliste nicht geloescht");
    pOldParaOrder = new List;

    // Liste der selektierten Titelabsaetze
    pSelectedParas = pOutlinerView[0]->CreateSelectionList();
    Paragraph* pPara = (Paragraph*)pSelectedParas->First();
    while (pPara)
    {
        if ( pOutliner->GetDepth( pOutliner->GetAbsPos( pPara ) ) > 0 )
        {
            pSelectedParas->Remove();
            pPara = (Paragraph*)pSelectedParas->GetCurObject();
        }
        else
        {
            pPara = (Paragraph*)pSelectedParas->Next();
        }
    }

    // Die zu den selektierten Absaetzen auf Ebene 0 gehoerenden Seiten
    // selektieren
    USHORT nPos = 0;
    ULONG nParaPos = 0;
    pPara = pOutliner->GetParagraph( 0 );

    while (pPara)
    {
        if ( pOutliner->GetDepth( nParaPos ) == 0 )                     // eine Seite?
        {
            pOldParaOrder->Insert(pPara, LIST_APPEND);
            SdPage* pPage = pDoc->GetSdPage(nPos, PK_STANDARD);
            pPage->SetSelected(FALSE);
            if (pSelectedParas->Seek(pPara))            // selektiert?
            {
                pPage->SetSelected(TRUE);
            }
            nPos++;
        }
        pPara = pOutliner->GetParagraph( ++nParaPos );
    }

    return 0;
}

/*************************************************************************
|*
|* Handler fuer das Ende einer Absatzverschiebung
|*
\************************************************************************/

IMPL_LINK( SdOutlineView, EndMovingHdl, Outliner *, pOutliner )
{
    DBG_ASSERT(pSelectedParas, "keine Absatzliste");
    DBG_ASSERT(pOldParaOrder, "keine Absatzliste");

    // Einfuegeposition anhand des ersten Absatzes suchen
    Paragraph* pSearchIt = (Paragraph*)pSelectedParas->First();

    // den ersten der selektierten Paragraphen in der neuen Ordnung suchen
    USHORT nPosNewOrder = 0;
    ULONG nParaPos = 0;
    Paragraph*  pPara = pOutliner->GetParagraph( 0 );
    Paragraph*  pPrev = NULL;
    while (pPara && pPara != pSearchIt)
    {
        if (pOutliner->GetDepth( nParaPos ) == 0)
        {
            nPosNewOrder++;
            pPrev = pPara;
        }
        pPara = pOutliner->GetParagraph( ++nParaPos );
    }

    USHORT nPos = nPosNewOrder;     // nPosNewOrder nicht veraendern
    if (nPos == 0)
    {
        nPos = (USHORT)-1;          // vor der ersten Seite einfuegen
    }
    else
    {
        // den Vorgaenger in der alten Ordnung suchen
        nPos = (USHORT)pOldParaOrder->GetPos(pPrev);
        DBG_ASSERT(nPos != LIST_ENTRY_NOTFOUND, "Absatz nicht gefunden");
    }

    BOOL bTest = pDoc->MovePages(nPos);

    // die Seiten wieder deselektieren
    USHORT nPageCount = (USHORT)pSelectedParas->Count();
    while (nPageCount)
    {
        SdPage* pPage = pDoc->GetSdPage(nPosNewOrder, PK_STANDARD);
        pPage->SetSelected(FALSE);
        nPosNewOrder++;
        nPageCount--;
    }

    pOutliner->UpdateFields();

    delete pSelectedParas;
    pSelectedParas = NULL;
    delete pOldParaOrder;
    pOldParaOrder = NULL;
    return 0;
}

/*************************************************************************
|*
|* Eine Seite des Models nach dem Titeltextobjekt durchsuchen
|*
\************************************************************************/

SdrTextObj* SdOutlineView::GetTitleTextObject(SdrPage* pPage)
{
    ULONG           nObjectCount = pPage->GetObjCount();
    SdrObject*      pObject      = NULL;
    SdrTextObj*     pResult      = NULL;

    for (ULONG nObject = 0; nObject < nObjectCount; nObject++)
    {
        pObject = pPage->GetObj(nObject);
        if (pObject->GetObjInventor() == SdrInventor &&
            pObject->GetObjIdentifier() == OBJ_TITLETEXT)
        {
            pResult = (SdrTextObj*)pObject;
            break;
        }
    }
    return pResult;
}


/*************************************************************************
|*
|* Eine Seite des Models nach dem Gliederungstextobjekt durchsuchen
|*
\************************************************************************/

SdrTextObj* SdOutlineView::GetLayoutTextObject(SdrPage* pPage)
{
    ULONG           nObjectCount = pPage->GetObjCount();
    SdrObject*      pObject      = NULL;
    SdrTextObj*     pResult      = NULL;

    for (ULONG nObject = 0; nObject < nObjectCount; nObject++)
    {
        pObject = pPage->GetObj(nObject);
        if (pObject->GetObjInventor() == SdrInventor &&
            pObject->GetObjIdentifier() == OBJ_OUTLINETEXT)
        {
            pResult = (SdrTextObj*)pObject;
            break;
        }
    }
    return pResult;
}

/*************************************************************************
|*
|* PrepareClose(), Daten und Selektion ins Model uebertragen
|*
\************************************************************************/

BOOL SdOutlineView::PrepareClose(BOOL bUI)
{
    if( pOutliner->IsModified() )
    {
        // die Seiten des Models mit den Inhalten des Outliners fuellen
        USHORT              nPageCount  = pDoc->GetSdPageCount(PK_STANDARD);
        Paragraph*          pPara       = NULL;
        SdrTextObj*         pTO         = NULL;
        OutlinerParaObject* pOPO        = NULL;
        SfxItemPool* pPool = pDoc->GetDrawOutliner().GetEmptyItemSet().GetPool();

        // den Outliner in den OutlinerStyle setzen; dann wird ein von ihm
        // erzeugter Titeltext bei der Anzeige durch einen Outliner im
        // EditEngine-Modus mit der Titelvorlage formatiert
        SdrOutliner* pTempLiner = new SdrOutliner( pPool, OUTLINERMODE_OUTLINEOBJECT );
        pTempLiner->SetStyleSheetPool((SfxStyleSheetPool*)pDoc->GetStyleSheetPool());
        pTempLiner->SetEditTextObjectPool(pPool);

        // Referenz-Device setzen
        SfxPrinter* pPrinter = pDocSh->GetPrinter(TRUE);
        pTempLiner->SetRefDevice(pPrinter);
        SfxStyleSheetBasePool* pSPool   = pDoc->GetStyleSheetPool();
        pTempLiner->SetMinDepth(0);

        BOOL bBegUndoDone = FALSE;      // wurde schon BegUndo() gerufen?
        String aUndoStr;
        aUndoStr = String(SdResId(STR_UNDO_CHANGE_TITLE_AND_LAYOUT));


        /**************************************************************************
        |* jetzt die Seiten fuellen
        \*************************************************************************/
        pPara = pOutliner->GetParagraph( 0 );
        for (USHORT nPage = 0; nPage < nPageCount; nPage++)
        {
            SdPage* pPage = pDoc->GetSdPage(nPage, PK_STANDARD);
            SdPage* pMasterPage = (SdPage*) pPage->GetMasterPage(0);

            Point aPos;
            Rectangle aTitleRect ( aPos, pPage->GetSize() );
            Rectangle aLayoutRect ( aTitleRect );

            SdrObject* pTitleObj = pMasterPage->GetPresObj(PRESOBJ_TITLE);
            SdrObject* pLayoutObj = pMasterPage->GetPresObj(PRESOBJ_OUTLINE);

            if (pTitleObj)
            {
                aTitleRect = pTitleObj->GetLogicRect();
            }

            if (pLayoutObj)
            {
                aLayoutRect = pLayoutObj->GetLogicRect();
            }

            pDoc->SetSelected(pPage, FALSE);          // deselektieren

            // Liste der Gliederungsvorlagen fuer Anmeldung als Listener
            String aName = pPage->GetLayoutName();

            // erst fuer die Gliederungsebenen
            List* pOutlineStyleList = new List; // StyleSheet-Liste zum Anmelden
            pOutlineStyleList->Insert(NULL, LIST_APPEND);
            String aFullName;
            SfxStyleSheetBase* pSheet = NULL;
            for (USHORT i = 1; i < 10; i++)
            {
                aFullName  = aName;
                aFullName += sal_Unicode(' ');
                aFullName += String::CreateFromInt32( (sal_Int32)i );;
                pSheet = pSPool->Find(aFullName, SD_LT_FAMILY);
                pOutlineStyleList->Insert(pSheet, LIST_APPEND);
            }

            // Titelvorlage
            aFullName = aName;
            String aSep( RTL_CONSTASCII_USTRINGPARAM( SD_LT_SEPARATOR ));
            USHORT n = aFullName.Search(aSep);
            n += aSep.Len();
            aFullName.Erase(n);
            aFullName += String (SdResId(STR_LAYOUT_TITLE));
            SfxStyleSheet* pTitleSheet = (SfxStyleSheet*)pSPool->
                Find(aFullName, SD_LT_FAMILY);
            DBG_ASSERT(pTitleSheet, "Titelvorlage nicht gefunden");

            /*********************************************************************
            |* Titeltextobjekt
            \********************************************************************/
            pTO = GetTitleTextObject(pPage);
            String aTest(pOutliner->GetText(pPara));
            BOOL bText = aTest.Len() > 0;

            if (!pTO && bText)
            {
                // Kein Seitenobjekt, Text im Outliner:
                // Entspr. Seitenobjekt erzeugen und einfuegen
                if (!bBegUndoDone)
                {
                    BegUndo(aUndoStr);
                    bBegUndoDone = TRUE;
                }

                pTO  = new SdrRectObj(OBJ_TITLETEXT);
                pOPO = pOutliner->CreateParaObject( pOutliner->GetAbsPos( pPara ), 1 );
                pOPO->SetOutlinerMode( OUTLINERMODE_TITLEOBJECT );
                pTO->SetOutlinerParaObject(pOPO);
                pTO->SetEmptyPresObj(FALSE);

                // als Praesentationsobjekt anmelden
                pTO->SetUserCall(pPage);
                List* pPresObjList = pPage->GetPresObjList();
                pPresObjList->Insert(pTO, LIST_APPEND);

                pPage->InsertObject(pTO);
                            // TRUE: DontRemoveHardAttr
                pTO->SetStyleSheet(pTitleSheet, TRUE);
                AddUndo(new SdrUndoNewObj(*pTO));
            }
            else if (pTO && bText)
            {
                // Seitenobjekt, Text im Outliner:
                // Titeltext uebernehmen

                if (!bBegUndoDone)
                {
                    BegUndo(aUndoStr);
                    bBegUndoDone = TRUE;
                }

                pOPO = pOutliner->CreateParaObject( pOutliner->GetAbsPos( pPara ), 1 );
                pOPO->SetOutlinerMode( OUTLINERMODE_TITLEOBJECT );
                // loescht altes OutlinerParaObject
                AddUndo(new SdrUndoObjSetText(*pTO));
                pTO->SetOutlinerParaObject(pOPO);
                pTO->SetEmptyPresObj(FALSE);
            }
            else if (!pTO && !bText)
            {
                // Kein Seitenobjekt, kein Text im Outliner: nix tun
            }
            else if(pTO && !bText)
            {
                // Seitenobjekt, kein Text im Outliner:
                // wenn Objekt in Praesentationsliste der Seite ist -> Defaulttext,
                // sonst Objekt loeschen
                List* pPresObjList = pPage->GetPresObjList();
                if (pPresObjList->GetPos(pTO) != LIST_ENTRY_NOTFOUND)
                {
                    if( !pTO->IsEmptyPresObj() )
                    {
                        if (!bBegUndoDone)
                        {
                            BegUndo(aUndoStr);
                            bBegUndoDone = TRUE;
                        }

                        pTempLiner->Clear();
                        pTempLiner->SetMinDepth(0);
                        String aString (SdResId(STR_PRESOBJ_TITLE));
                        pTempLiner->Insert(aString, LIST_APPEND, 0);
                        pOPO = pTempLiner->CreateParaObject( 0, 1 );
                        pOPO->SetOutlinerMode( OUTLINERMODE_TITLEOBJECT );

                        // loescht auch altes OutlinerParaobject
                        AddUndo(new SdrUndoObjSetText(*pTO));
                        pTO->SetOutlinerParaObject(pOPO);
                        pTO->SetEmptyPresObj(TRUE);
                        pTempLiner->SetMinDepth(0);
                    }
                }
                else
                {
                    // Grafik: Seitenobjekt loeschen
                    if (!bBegUndoDone)
                    {
                        BegUndo(aUndoStr);
                        bBegUndoDone = TRUE;
                    }

                    AddUndo(new SdrUndoRemoveObj(*pTO));
                    pPage->RemoveObject(pTO->GetOrdNum());
                }
            }

            /*********************************************************************
            |* Gliederungstextobjekt
            \********************************************************************/
            BOOL bSubTitle = FALSE;
            pTO = (SdrTextObj*) pPage->GetPresObj(PRESOBJ_TEXT);

            if (pTO)
            {
                bSubTitle = TRUE;
            }
            else
            {
                pTO = GetLayoutTextObject(pPage);
            }

            pOPO = NULL;

            // Wenn moeglich, OutlinerParaObject fuer Gliederung erzeugen
            if ( pPara && pOutliner->HasChilds(pPara) )
            {
                // wieviele Absaetze in der Gliederung?
                ULONG nTitlePara = pOutliner->GetAbsPos(pPara);
                ULONG nParaPos = nTitlePara;
                ULONG nParasInLayout = 0L;
                pPara = pOutliner->GetParagraph( ++nParaPos );
                while (pPara && pOutliner->GetDepth( nParaPos ) != 0)
                {
                    nParasInLayout++;
                    pPara = pOutliner->GetParagraph( ++nParaPos );
                }

                // ein OutlinerParaObject erzeugen
                pPara = pOutliner->GetParagraph(nTitlePara + 1);
                pOPO  = pOutliner->CreateParaObject( nTitlePara + 1, nParasInLayout );
                pOPO->SetOutlinerMode( OUTLINERMODE_OUTLINEOBJECT );
            }

            if (!pTO && pOPO)
            {
                // Kein Seitenobjekt, Gliederungstext im Outliner
                if (!bBegUndoDone)
                {
                    BegUndo(aUndoStr);
                    bBegUndoDone = TRUE;
                }

                pTO = new SdrRectObj(OBJ_OUTLINETEXT);
                pTO->SetOutlinerParaObject(pOPO);
                pTO->SetEmptyPresObj(FALSE);

                // als Praesentationsobjekt anmelden
                pTO->SetUserCall(pPage);
                List* pPresObjList = pPage->GetPresObjList();
                pPresObjList->Insert(pTO, LIST_APPEND);

                pPage->InsertObject(pTO);

                // Linien- und Fuellattribute der Standardvorlage hart
                // ueberschreiben
                SfxItemSet aTempAttr(pDoc->GetPool());
                aTempAttr.Put(XLineStyleItem(XLINE_NONE));
                aTempAttr.Put(XFillStyleItem(XFILL_NONE));
                pTO->SetAttributes(aTempAttr, FALSE);

                // als Listener anmelden
                for (USHORT i = 1; i < 10; i++)
                {
                    SfxStyleSheet* pS = (SfxStyleSheet*)pOutlineStyleList->
                                                                    GetObject(i);
                    if (pS)
                        pTO->StartListening(*pS);
                }
                AddUndo(new SdrUndoNewObj(*pTO));
            }
            else if (pTO && pOPO)
            {
                // Seitenobjekt, Gliederungstext im Outliner:
                // Text uebernehmen
                if (!bBegUndoDone)
                {
                    BegUndo(aUndoStr);
                    bBegUndoDone = TRUE;
                }

                // loescht auch altes OutlinerParaObject
                AddUndo(new SdrUndoObjSetText(*pTO));

                if (bSubTitle)
                {
                    // Fuer alle Absaetze im TextObjekt schauen ob eine Ausrichtung
                    // gesetzt ist, wenn nicht => zentrieren
                    Outliner* pOutl = pDoc->GetInternalOutliner();
                    pOutl->Clear();
                    pOutl->SetText( *pOPO );

                    ULONG nCount = pOutl->GetParagraphCount();
                    for( ULONG nPara = 0; nPara < nCount; nPara++ )
                    {
                        SfxItemSet aSet( pOutl->GetParaAttribs( nPara ) );
                        if (aSet.GetItemState(EE_PARA_JUST) != SFX_ITEM_SET)
                        {
                            aSet.Put(SvxAdjustItem( SVX_ADJUST_CENTER ));
                            pOutl->SetParaAttribs(nPara, aSet);
                        }
                    }

                    delete pOPO;
                    pOPO = pOutl->CreateParaObject( 0, nCount );
                    pOPO->SetOutlinerMode( OUTLINERMODE_TEXTOBJECT );
                    pOutl->Clear();
                }

                pTO->SetOutlinerParaObject(pOPO);
                pTO->SetEmptyPresObj(FALSE);

                if (bSubTitle)
                {
                    // Subtitle-Vorlage setzen
                    SfxStyleSheet* pSheet = pPage->GetStyleSheetForPresObj(PRESOBJ_TEXT);

                    if( pTO->GetStyleSheet() != pSheet )
                        pTO->SetStyleSheet(pSheet, TRUE);

                    // Untertitel-Objekt statt Gliederung -> Linken Einzug zuruecksetzen
                    SfxItemSet aSet(pDoc->GetPool(), EE_PARA_LRSPACE, EE_PARA_LRSPACE);
                    pTO->TakeAttributes(aSet, TRUE, FALSE);

                    if (aSet.GetItemState(EE_PARA_LRSPACE) != SFX_ITEM_SET)
                    {
                        const SvxLRSpaceItem& rLRItem = (const SvxLRSpaceItem&) aSet.Get(EE_PARA_LRSPACE);
                        SvxLRSpaceItem aNewLRItem(rLRItem);
                        aNewLRItem.SetTxtLeft(0);
                        aSet.Put(aNewLRItem);
                        pTO->NbcSetAttributes(aSet, FALSE);
                    }
                }
            }
            else if (!pTO && !pOPO)
            {
                // kein Seitenobjekt, kein Gliederungstext: nix tun
            }
            else if (pTO && !pOPO)
            {
                // Seitenobjekt, aber kein Gliederungstext:
                // wenn Objekt in Praesentationsliste der Seite ist -> Defaulttext,
                // sonst Objekt loeschen
                List* pPresObjList = pPage->GetPresObjList();
                if (pPresObjList->GetPos(pTO) != LIST_ENTRY_NOTFOUND)
                {
                    if( !pTO->IsEmptyPresObj() )
                    {
                        if (!bBegUndoDone)
                        {
                            BegUndo(aUndoStr);
                            bBegUndoDone = TRUE;
                        }

                        // loescht auch altes OutlinerParaObject
                        AddUndo(new SdrUndoObjSetText(*pTO));

                        pTempLiner->Clear();
                        pTempLiner->SetMinDepth(0);

                        String aEmptyStr;
                        PresObjKind ePresObjKind = pPage->GetPresObjKind(pTO);
                        String aString = pPage->GetPresObjText(ePresObjKind);
                        pPage->SetObjText( pTO, pTempLiner, ePresObjKind, aString );
                        pTO->NbcSetStyleSheet( pPage->GetStyleSheetForPresObj( ePresObjKind ), TRUE );

                        if (ePresObjKind == PRESOBJ_TEXT)
                        {
                            // Vorlage setzen
                            pTO->SetStyleSheet(pPage->GetStyleSheetForPresObj(ePresObjKind), TRUE);

                            // Linken Einzug zuruecksetzen
                            SfxItemSet aSet(pDoc->GetPool(), EE_PARA_LRSPACE, EE_PARA_LRSPACE);
                            pTO->TakeAttributes(aSet, TRUE, FALSE);
                            const SvxLRSpaceItem& rLRItem = (const SvxLRSpaceItem&) aSet.Get(EE_PARA_LRSPACE);
                            SvxLRSpaceItem aNewLRItem(rLRItem);
                            aNewLRItem.SetTxtLeft(0);
                            aSet.Put(aNewLRItem);
                            pTO->NbcSetAttributes(aSet, FALSE);
                        }

                        pTO->SetEmptyPresObj(TRUE);
                        pTempLiner->SetMinDepth(0);
                    }
                }
                else
                {
                    if (!bBegUndoDone)
                    {
                        BegUndo(aUndoStr);
                        bBegUndoDone = TRUE;
                    }

                    AddUndo(new SdrUndoRemoveObj(*pTO));
                    pPage->RemoveObject(pTO->GetOrdNum());
                }
            }

            delete pOutlineStyleList;

            // das AutoLayout nochmal anwenden, damit neu eingefuegte Textobjekte
            // die richtige Position/Groesse bekommen
            pPage->SetAutoLayout(pPage->GetAutoLayout());

            // zum naechsten Absatz der 0-ten Ebene
            pPara = GetNextTitle(pPara);
        }

        if (bBegUndoDone)
        {
            // Actions im Redo-Anteil des Outliner-UndoManagers loeschen, denn die
            // referenzieren Actions im Redo-Anteil des Doc-UndoManagers (und die
            // werden bei EndUndo() geloescht --> Referenzen ins Leere --> Absturz)
            pOutliner->GetUndoManager().ClearRedo();

            EndUndo();
            SfxUndoManager* pDocUndoMgr = pDocSh->GetUndoManager();
            SfxLinkUndoAction* pLink = new SfxLinkUndoAction(pDocUndoMgr);
            pOutliner->GetUndoManager().AddUndoAction(pLink);
        }


        // die Seite selektieren
        SdPage* pPage = GetActualPage();
        pDoc->SetSelected(pPage, TRUE);

        // temporaeren Outliner loeschen
        delete pTempLiner;
    }

    return TRUE;    // einverstanden mit der eigenen Zerstoerung
}


/*************************************************************************
|*
|* Attribute des selektierten Textes setzen
|*
\************************************************************************/

BOOL SdOutlineView::SetAttributes(const SfxItemSet& rSet, BOOL bReplaceAll)
{
    FASTBOOL bOk = FALSE;

    OutlinerView* pOlView = GetViewByWindow(pOutlineViewShell->GetActiveWindow());

    if (pOlView)
    {
        pOlView->SetAttribs(rSet);
        bOk = TRUE;
    }

    return (bOk);
}

/*************************************************************************
|*
|* Attribute des selektierten Textes erfragen
|*
\************************************************************************/

BOOL SdOutlineView::GetAttributes( SfxItemSet& rTargetSet, BOOL bOnlyHardAttr ) const
{
    OutlinerView* pOlView = GetViewByWindow(
                                pOutlineViewShell->GetActiveWindow());
    DBG_ASSERT(pOlView, "keine OutlinerView gefunden");

    rTargetSet.Put( pOlView->GetAttribs(), FALSE );
    return TRUE;
}

/*************************************************************************
|*
|* gibt's selektierten Text ?
|*
\************************************************************************/

BOOL SdOutlineView::HasMarkedObj() const
{
    BOOL bResult = FALSE;
    OutlinerView* pOlView = GetViewByWindow(
                                pOutlineViewShell->GetActiveWindow());
    DBG_ASSERT(pOlView, "keine OutlinerView gefunden");
    List* pSelList = (List*)pOlView->CreateSelectionList();
    if (pSelList->Count() > 0)
        bResult = TRUE;

    delete pSelList;
    return bResult;
}

/*************************************************************************
|*
|* Outliner mit Daten aus dem SdDrawDocument fuellen
|*
\************************************************************************/

void SdOutlineView::FillOutliner()
{
    ResetLinks();
    pOutliner->SetMinDepth(0);

    Paragraph* pTitleToSelect = NULL;
    ULONG nPageCount = pDoc->GetSdPageCount(PK_STANDARD);

    // Outliner mit Text versorgen
    for (USHORT nPage = 0; nPage < nPageCount; nPage++)
    {
        SdPage*     pPage = (SdPage*)pDoc->GetSdPage(nPage, PK_STANDARD);
        Paragraph * pPara = NULL;

        // Titeltext uebernehmen
        SdrTextObj*     pTO   = GetTitleTextObject(pPage);
        if (pTO)
        {
            if (!(pTO->IsEmptyPresObj()))   // gesetzten Titeltext gefunden
            {
                OutlinerParaObject* pOPO = pTO->GetOutlinerParaObject();
                if (pOPO)
                {
                    pOutliner->AddText(*pOPO);
                    pPara = pOutliner->GetParagraph( pOutliner->GetParagraphCount() - 1 );
                    // Man sollte meinen, dass folgendes Statement unnoetig sei,
                    // aber ueber das #39788#-Bugdoc ist eine Inkonsistenz im
                    // Doc aufgefallen, wo Titel die Ebene 1 (nicht 0) hat.
                    // Die Ursache ist bisher ungeklaert.
                    pOutliner->SetDepth( pPara, 0 );
                }
                else
                {
                    pPara = pOutliner->Insert(String());

                    // Keine harten Attribute vom vorherigen Absatz uebernehmen
                    pOutliner->SetParaAttribs( pOutliner->GetAbsPos(pPara),
                                               pOutliner->GetEmptyItemSet() );
                    pPara = pOutliner->GetParagraph( pOutliner->GetParagraphCount() - 1 );
                }
            }
            else                            // leerer Praesentationstitel
            {
                pPara = pOutliner->Insert(String());

                // Keine harten Attribute vom vorherigen Absatz uebernehmen
                pOutliner->SetParaAttribs( pOutliner->GetAbsPos(pPara),
                                           pOutliner->GetEmptyItemSet() );

                pOutliner->SetStyleSheet( pOutliner->GetAbsPos( pPara ), pPage->GetStyleSheetForPresObj( PRESOBJ_TITLE ) );
            }
        }
        else                // kein Titeltext, also leerer Titel
        {
            pPara = pOutliner->Insert(String());

            // Keine harten Attribute vom vorherigen Absatz uebernehmen
            pOutliner->SetParaAttribs( pOutliner->GetAbsPos(pPara),
                                       pOutliner->GetEmptyItemSet() );

            pOutliner->SetStyleSheet( pOutliner->GetAbsPos( pPara ), pPage->GetStyleSheetForPresObj( PRESOBJ_TITLE ) );
        }

        // selektierte Seite merken
        if (pPage->IsSelected())
        {
            pTitleToSelect = pPara;
        }


        // Gliederungstext uebernehmen
        pTO = (SdrTextObj*)pPage->GetPresObj(PRESOBJ_TEXT);
        BOOL   bSubTitle = pTO != NULL;
        if (!pTO)
        {
            pTO = GetLayoutTextObject(pPage);
        }
        if (pTO)
        {
            if(!(pTO->IsEmptyPresObj())) // gesetzten Gl.text gefunden
            {
                OutlinerParaObject* pOPO = pTO->GetOutlinerParaObject();
                if (pOPO)
                {
                    ULONG nParaCount1 = pOutliner->GetParagraphCount();
                    pOutliner->AddText(*pOPO);
                    if (bSubTitle)
                    {
                        ULONG nParaCount2 = pOutliner->GetParagraphCount();
                        Paragraph* pPara = NULL;
                        for (ULONG nPara = nParaCount1; nPara < nParaCount2; nPara++)
                        {
                            pPara = pOutliner->GetParagraph(nPara);
                            if(pPara && pOutliner->GetDepth( nPara ) != 1 )
                                pOutliner->SetDepth(pPara, 1);
                        }
                    }
                }
            }
        }
        else                // kein Gliederungstext, also nichts einfuegen
        {
        }
    }

    // Cursor an den Anfang stellen
    Paragraph* pFirstPara = pOutliner->GetParagraph( 0 );
    pOutlinerView[0]->Select( pFirstPara, TRUE, FALSE );
    pOutlinerView[0]->Select( pFirstPara, FALSE, FALSE );

    // Titel der selektierten Seite des Models im Outliner selektieren
    if (pTitleToSelect)
    {
        pOutlinerView[0]->Select(pTitleToSelect, TRUE, FALSE);
    }

    SetLinks();
}

/*************************************************************************
|*
|* Handler fuer das Loeschen von Level-0-Absaetzen (Seiten): Warnung
|*
\************************************************************************/

IMPL_LINK( SdOutlineView, RemovingPagesHdl, OutlinerView *, pOutlinerView )
{
    long nResult = 0;
    USHORT nNumOfPages = 0;
    Paragraph* pTitlePara = NULL;  // der Titel (falls nur eine Seite betroffen)
    Paragraph* pPara      = NULL;

    /*************************************************************************
    |* Befinden sich Objekte auf der/den Seite(n), die der Benutzer nicht
    |* so einfach neu erzeugen kann?
    |* Kriterium: <mehr Objekte als Praesentationsobjekte>   oder
    |*            <wenigstens ein nicht-leeres Praesentationsobjekt>
    \**********************************************************************/
    BOOL bValuableObjectsFound = FALSE;
    List* pList = pOutlinerView->CreateSelectionList();
    pPara = (Paragraph*)pList->First();
    while (pPara)
    {
        if ( pOutliner->GetDepth( pOutlinerView->GetOutliner()->GetAbsPos( pPara ) ) == 0 )
        {
            // welche Seite?
            USHORT nPage = 0;
            Paragraph* pScan = pPara;
            while (pScan)
            {
                pScan = GetPrevTitle(pScan);
                if (pScan)
                    nPage++;
            }

            SdPage* pPage = pDoc->GetSdPage(nPage, PK_STANDARD);
            List* pPresObjs = pPage->GetPresObjList();
            if (pPage->GetObjCount() > pPresObjs->Count())
            {
                bValuableObjectsFound = TRUE;
            }
            else
            {
                SdrObject* pObject = (SdrObject*)pPresObjs->First();
                while (pObject && !bValuableObjectsFound)
                {
                    bValuableObjectsFound = !pObject->IsEmptyPresObj();
                    pObject = (SdrObject*)pPresObjs->Next();

                }
            }
        }
        pPara = (Paragraph*)pList->Next();
    }
    delete pList;

    /*************************************************************************
    |* nichts "wertvolles" gefunden: es darf ohne Rueckfrage geloescht werden
    \************************************************************************/
    if (!bValuableObjectsFound)
    {
        nResult = 1;
    }
    /*************************************************************************
    |* lieber vorher nachfragen
    \************************************************************************/
    else
    {
        nNumOfPages = pOutliner->GetSelPageCount();
        DBG_ASSERT(nNumOfPages > 0, "0 Seiten werden geloescht ???");

        String aWarnStr;
        if (nNumOfPages == 1)
        {
            // wie heisst die betroffene Seite?
            USHORT nPage = 0;
            ULONG nPara = pOutliner->GetFirstSelPage();
            pPara = pOutliner->GetParagraph(nPara);
            while (pPara)
            {
                pPara = GetPrevTitle(pPara);
                if (pPara)
                    nPage++;
            }

            String aPageName = ((SdPage*)pDoc->GetSdPage(nPage, PK_STANDARD))->
                                                 GetName();

            // dynamische Seitentitel beachten
            if (aPageName.Len() == 0)
            {
                aPageName += String(SdResId(STR_PAGE));
                aPageName += String::CreateFromInt32( nPage + 1 );      // an der UI beginnen Seiten bei 1
            }

            aWarnStr = String(SdResId(STR_WARN_DEL_PAGE));

            // Platzhalter durch Seitennamen ersetzen
            xub_StrLen nPos = aWarnStr.Search(sal_Unicode('$'));
            aWarnStr.Erase(nPos, 1);
            aWarnStr.Insert(aPageName, nPos);
        }
        else if (nNumOfPages > 1)
        {
            aWarnStr = String(SdResId(STR_WARN_DEL_SEL_PAGES));

        }

        // Warnung ausgeben
        WarningBox aWarningBox( pOutlineViewShell->GetActiveWindow(),
                               (WinBits)WB_OK_CANCEL | WB_DEF_CANCEL, aWarnStr);
        short nDlgResult = aWarningBox.Execute();
        if (nDlgResult == RET_OK)
        {
            nResult = 1;
        }
    }

    /*************************************************************************
    |* Es soll geloescht werden: ggfs. Fortschrittsanzeige vorbereiten
    \************************************************************************/
    if (nResult == 1)
    {
        if (nNumOfPages > PROCESS_WITH_PROGRESS_THRESHOLD)
        {
            nPagesToProcess = nNumOfPages;
            nPagesProcessed  = 0;
        }

        if (nPagesToProcess)
        {
            if( mpProgress )
                delete mpProgress;

            String aStr(SdResId(STR_DELETE_PAGES));
            mpProgress = new SfxProgress( GetDocSh(), aStr, nPagesToProcess );
        }
        pOutliner->UpdateFields();
    }

    return nResult;
}

/*************************************************************************
|*
|* Handler fuer das Einruecken von Level-0-Absaetzen (Seiten): Warnung
|*
\************************************************************************/

IMPL_LINK_INLINE_START( SdOutlineView, IndentingPagesHdl, OutlinerView *, pOutlinerView )
{
    return RemovingPagesHdl(pOutlinerView);
}
IMPL_LINK_INLINE_END( SdOutlineView, IndentingPagesHdl, OutlinerView *, pOutlinerView )


/*************************************************************************
|*
|* Liefert die aktuellen Seite
|*
\************************************************************************/

SdPage* SdOutlineView::GetActualPage()
{
    /**************************************************************************
    |* Die Seite, in deren Titel oder Gliederung der Cursor (bzw. der
    |* Selektionsbeginn) steht, soll selektiert werden.
    \*************************************************************************/
    SdWindow*      pWin             = pOutlineViewShell->GetActiveWindow();
    OutlinerView*  pActiveView      = GetViewByWindow(pWin);
    Outliner*      pOutl            = pActiveView->GetOutliner();
    List*          pSelList         = (List*)pActiveView->CreateSelectionList();
    Paragraph*     pPara            = (Paragraph*)pSelList->First();
    if ( pOutl->GetDepth( pOutl->GetAbsPos( pPara ) ) > 0 )
    {
        pPara = GetPrevTitle(pPara);
    }
    USHORT nPageToSelect = 0;
    while(pPara)
    {
        pPara = GetPrevTitle(pPara);
        if (pPara) nPageToSelect++;
    }

    delete pSelList;                // die wurde extra fuer uns erzeugt

    SdPage* pPage = (SdPage*)pDoc->GetSdPage(nPageToSelect, PK_STANDARD);

    return(pPage);
}

/*************************************************************************
|*
|* StyleSheet aus der Selektion besorgen
|*
\************************************************************************/

SfxStyleSheet* SdOutlineView::GetStyleSheet() const
{
    SdWindow*     pActWin = pOutlineViewShell->GetActiveWindow();
    OutlinerView* pOlView = GetViewByWindow(pActWin);
    SfxStyleSheet* pResult = pOlView->GetStyleSheet();
    return pResult;
}



/*************************************************************************
|*
|* Seiten als selektiert / nicht selektiert setzen
|*
\************************************************************************/

void SdOutlineView::SetSelectedPages()
{
    // Liste der selektierten Titelabsaetze
    List* pSelParas = pOutlinerView[0]->CreateSelectionList();
    Paragraph* pPara = (Paragraph*) pSelParas->First();

    while (pPara)
    {
        if ( pOutliner->GetDepth( pOutliner->GetAbsPos( pPara ) ) > 0 )
        {
            pSelParas->Remove();
            pPara = (Paragraph*) pSelParas->GetCurObject();
        }
        else
        {
            pPara = (Paragraph*) pSelParas->Next();
        }
    }

    // Die zu den selektierten Absaetzen auf Ebene 0 gehoerenden Seiten
    // selektieren
    USHORT nPos = 0;
    ULONG nParaPos = 0;
    pPara = pOutliner->GetParagraph( 0 );

    while (pPara)
    {
        if ( pOutliner->GetDepth( nParaPos ) == 0 )                     // eine Seite?
        {
            SdPage* pPage = pDoc->GetSdPage(nPos, PK_STANDARD);
            pPage->SetSelected(FALSE);

            if (pSelParas->Seek(pPara))            // selektiert?
            {
                pPage->SetSelected(TRUE);
            }

            nPos++;
        }

        pPara = pOutliner->GetParagraph( ++nParaPos );
    }
}


/*************************************************************************
|*
|* Neue Links setzen
|*
\************************************************************************/

void SdOutlineView::SetLinks()
{
    // Benachrichtigungs-Links setzen
    pOutliner->SetParaInsertedHdl(LINK(this, SdOutlineView, ParagraphInsertedHdl));
    pOutliner->SetParaRemovingHdl(LINK(this, SdOutlineView, ParagraphRemovingHdl));
    pOutliner->SetDepthChangedHdl(LINK(this, SdOutlineView, DepthChangedHdl));
    pOutliner->SetBeginMovingHdl(LINK(this, SdOutlineView, BeginMovingHdl));
    pOutliner->SetEndMovingHdl(LINK(this, SdOutlineView, EndMovingHdl));
    pOutliner->SetRemovingPagesHdl(LINK(this, SdOutlineView, RemovingPagesHdl));
    pOutliner->SetIndentingPagesHdl(LINK(this, SdOutlineView, IndentingPagesHdl));
    pOutliner->SetMinDepth(0);
    pOutliner->SetStatusEventHdl(LINK(this, SdOutlineView, StatusEventHdl));
}



/*************************************************************************
|*
|* Alte Links restaurieren
|*
\************************************************************************/

void SdOutlineView::ResetLinks() const
{
    // alte Links restaurieren
    Link aEmptyLink;
    pOutliner->SetParaInsertedHdl(aEmptyLink);
    pOutliner->SetParaRemovingHdl(aEmptyLink);
    pOutliner->SetDepthChangedHdl(aEmptyLink);
    pOutliner->SetBeginMovingHdl(aEmptyLink);
    pOutliner->SetEndMovingHdl(aEmptyLink);
    pOutliner->SetStatusEventHdl(aEmptyLink);
    pOutliner->SetRemovingPagesHdl(aEmptyLink);
    pOutliner->SetIndentingPagesHdl(aEmptyLink);
    pOutliner->SetMinDepth(0);
}



