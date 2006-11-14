/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: outlview.cxx,v $
 *
 *  $Revision: 1.44 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 14:45:20 $
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

#include "OutlineView.hxx"
#include <memory>
#ifndef _FORBIDDENCHARACTERSTABLE_HXX
#include <svx/forbiddencharacterstable.hxx>
#endif
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
#ifndef _SVX_NUMITEM_HXX
#include <svx/numitem.hxx>
#endif
#ifndef _OUTLOBJ_HXX //autogen
#include <svx/outlobj.hxx>
#endif
#ifndef _SVX_NUMITEM_HXX
#include <svx/numitem.hxx>
#endif
#ifndef _MyEDITENG_HXX
#include <svx/editeng.hxx>
#endif

// #97766#
#ifndef _EDITOBJ_HXX
#include <svx/editobj.hxx>
#endif
#ifndef _EDITUND2_HXX
#include <svx/editund2.hxx>
#endif


#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#include "sdpage.hxx"
#include "pres.hxx"
#ifndef SD_OUTLINE_VIEW_SHELL_HXX
#include "OutlineViewShell.hxx"
#endif
#include "app.hrc"
#include "glob.hrc"
#include "sdresid.hxx"
#ifndef SD_OUTLINER_HXX
#include "Outliner.hxx"
#endif
#include "strings.hrc"
#include "EventMultiplexer.hxx"
#include "ViewShellBase.hxx"
#include "undo/undoobjects.hxx"
#include "undo/undomanager.hxx"

namespace sd {

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

TYPEINIT1( OutlineView, ::sd::View );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

OutlineView::OutlineView (
    DrawDocShell* pDocSh,
    ::Window* pWindow,
    OutlineViewShell* pOutlineViewSh)
    : ::sd::View(pDocSh->GetDoc(), pWindow, pOutlineViewSh),
      pOutlineViewShell(pOutlineViewSh),
      pOutliner( pDoc->GetOutliner(TRUE) ),
      pOldParaOrder(NULL),
      pSelectedParas(NULL),
      bFirstPaint(TRUE),
      nPagesToProcess(0),
      nPagesProcessed(0),
      mbHighContrastMode( false ),
      maDocColor( COL_WHITE ),
      mpProgress(NULL),
      mnIgnoreCurrentPageChangesLevel(0)
{
    BOOL bInitOutliner = FALSE;

    if (pOutliner->GetViewCount() == 0)
    {
        // Outliner initialisieren: Referenz-Device setzen
        bInitOutliner = TRUE;
        pOutliner->Init( OUTLINERMODE_OUTLINEVIEW );

        SfxStyleSheet* pTitleSheet = pDoc->GetSdPage( 0, PK_STANDARD )->GetStyleSheetForPresObj( PRESOBJ_TITLE );

        if ( pTitleSheet )
        {
            // set title symbol (level 0)
            SvxNumBulletItem aNumBulletItem( (const SvxNumBulletItem&) pTitleSheet->GetItemSet().Get(EE_PARA_NUMBULLET) );
            SvxNumRule aNumRule(* aNumBulletItem.GetNumRule());
            SvxNumberFormat aFormat( aNumRule.GetLevel(0));
            Font    aBulletFont;
            const Font* pFont = aFormat.GetBulletFont();
            if ( pFont )                                        // if available take font size and color from style
                aBulletFont = *pFont;
            else
            {
                aBulletFont.SetColor( COL_AUTO );
                aBulletFont.SetHeight( 1552 );
            }
            aBulletFont.SetCharSet(RTL_TEXTENCODING_MS_1252);   // and replacing other values by standard
            aBulletFont.SetName( String( RTL_CONSTASCII_USTRINGPARAM( "StarSymbol" )) );
            aBulletFont.SetWeight(WEIGHT_NORMAL);
            aBulletFont.SetUnderline(UNDERLINE_NONE);
            aBulletFont.SetStrikeout(STRIKEOUT_NONE);
            aBulletFont.SetItalic(ITALIC_NONE);
            aBulletFont.SetOutline(FALSE);
            aBulletFont.SetShadow(FALSE);
            aFormat.SetBulletFont( &aBulletFont );
            aFormat.SetBulletChar( 0xE011 );  // StarBats: 0xF000 + 114
            pOutliner->OverwriteLevel0Bullet( aFormat );
        }

        pOutliner->SetRefDevice( SD_MOD()->GetRefDevice( *pDocSh ) );
        ULONG nWidth = OUTLINE_PAPERWIDTH;
        pOutliner->SetPaperSize(Size(nWidth, 4000000000));
    }

    // View in Outliner einfuegen
    for (USHORT nView = 0; nView < MAX_OUTLINERVIEWS; nView++)
    {
        pOutlinerView[nView] = NULL;
    }

    pOutlinerView[0] = new OutlinerView(pOutliner, pWindow);
    Rectangle aNullRect;
    pOutlinerView[0]->SetOutputArea(aNullRect);
    pOutliner->SetUpdateMode(FALSE);
    pOutliner->InsertView(pOutlinerView[0], LIST_APPEND);

    onUpdateStyleSettings( true );

    if (bInitOutliner)
    {
        // Outliner mit Inhalt fuellen
        FillOutliner();
    }

    Link aLink( LINK(this,OutlineView,EventMultiplexerListener) );
    pOutlineViewShell->GetViewShellBase().GetEventMultiplexer().AddEventListener( aLink,
        tools::EventMultiplexerEvent::EID_CURRENT_PAGE
        | tools::EventMultiplexerEvent::EID_PAGE_ORDER);
}

/*************************************************************************
|*
|* Destruktor, Links restaurieren, Outliner leeren
|*
\************************************************************************/

OutlineView::~OutlineView()
{
    DBG_ASSERT(maDragAndDropModelGuard.get() == 0, "sd::OutlineView::~OutlineView(), prior drag operation not finished correctly!" );

    Link aLink( LINK(this,OutlineView,EventMultiplexerListener) );
    pOutlineViewShell->GetViewShellBase().GetEventMultiplexer().RemoveEventListener( aLink );
    DisconnectFromApplication();

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
        SvtAccessibilityOptions aOptions;
        pOutliner->ForceAutoColor( aOptions.GetIsAutomaticFontColor() );
        pOutliner->Clear();
    }

    DBG_ASSERT(!pSelectedParas, "Absatzliste nicht geloescht");
    DBG_ASSERT(!pOldParaOrder, "Absatzliste nicht geloescht");
}




void OutlineView::ConnectToApplication (void)
{
    pOutlineViewShell->GetActiveWindow()->GrabFocus();
    Application::AddEventListener(LINK(this, OutlineView, AppEventListenerHdl));
}




void OutlineView::DisconnectFromApplication (void)
{
    Application::RemoveEventListener(LINK(this, OutlineView, AppEventListenerHdl));
}




/*************************************************************************
|*
|* Paint-Methode
|*
\************************************************************************/

void OutlineView::Paint(const Rectangle& rRect, ::sd::Window* pWin)
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

void OutlineView::AdjustPosSizePixel(
    const Point &rNewPos,
    const Size &rNewSize,
    ::sd::Window* pWindow)
{

}

/*************************************************************************
|*
|* ein Fenster hinzufuegen
|*
\************************************************************************/

void OutlineView::AddWindowToPaintView(::sd::Window* pWin)
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

    ::sd::View::AddWindowToPaintView(pWin);
}

/*************************************************************************
|*
|* ein Fenster entfernen
|*
\************************************************************************/

void OutlineView::DeleteWindowFromPaintView(::sd::Window* pWin)
{
    BOOL bRemoved = FALSE;
    USHORT nView = 0;
    ::Window* pWindow;

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

    ::sd::View::DeleteWindowFromPaintView(pWin);
}

/*************************************************************************
|*
|* Zeiger der dem Fenster entsprechenden OutlinerView zurueckgeben.
|*
\************************************************************************/

OutlinerView* OutlineView::GetViewByWindow (::Window* pWin) const
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

Paragraph* OutlineView::GetPrevTitle(const Paragraph* pPara)
{
    Paragraph* pResult = NULL;
    ULONG      nPos    = pOutliner->GetAbsPos((Paragraph*)pPara);

    if (nPos > 0)
    {
        while (nPos && !pResult)
        {
            pPara = pOutliner->GetParagraph(--nPos);
            if ( pOutliner->GetDepth( (USHORT) nPos ) == 0 )
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

Paragraph* OutlineView::GetNextTitle(const Paragraph* pPara)
{
    ULONG nPos = pOutliner->GetAbsPos((Paragraph*)pPara);

    Paragraph* pResult = (Paragraph*)pPara;
    do
    {
        pResult = pOutliner->GetParagraph(++nPos);
    }
    while (pResult && pOutliner->GetDepth( (USHORT) nPos ) != 0);

    if (pResult && pOutliner->GetDepth( (USHORT) nPos ) != 0)
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

IMPL_LINK( OutlineView, ParagraphInsertedHdl, ::Outliner *, pOutliner )
{
    InsertSlideForParagraph( pOutliner->GetHdlParagraph() );
    return 0;
}

/** creates and inserts an empty slide for the given paragraph */
SdPage* OutlineView::InsertSlideForParagraph( Paragraph* pPara )
{
    DBG_ASSERT( isRecordingUndo(), "sd::OutlineView::ParagraphInsertedHdl(), model change without undo?!" );

    OutlineViewPageChangesGuard aGuard(this);

    SdPage* pPage = 0;

    if ( pOutliner->GetDepth( (USHORT) pOutliner->GetAbsPos( pPara ) ) == 0 )
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

            USHORT nPageCount = pDoc->GetSdPageCount( PK_STANDARD );
            if( nExample >= nPageCount )
                nExample = nPageCount - 1;
        }

        /**********************************************************************
        * Es wird stets zuerst eine Standardseite und dann eine
        * Notizseite erzeugt. Es ist sichergestellt, dass auf eine
        * Standardseite stets die zugehoerige Notizseite folgt.
        * Vorangestellt ist genau eine Handzettelseite
        **********************************************************************/

        // diese Seite hat Vorbildfunktion
        SdPage* pExample = (SdPage*)pDoc->GetSdPage((USHORT)nExample, PK_STANDARD);
        pPage = (SdPage*)pDoc->AllocPage(FALSE);

        pPage->SetLayoutName(pExample->GetLayoutName());

        // einfuegen (Seite)
        pDoc->InsertPage(pPage, (USHORT)(nTarget) * 2 + 1);
        if( isRecordingUndo() )
            AddUndo(pDoc->GetSdrUndoFactory().CreateUndoNewPage(*pPage));

        // der Standardseite eine Masterpage zuweisen
        pPage->TRG_SetMasterPage(pExample->TRG_GetMasterPage());

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
        if( isRecordingUndo() )
            AddUndo(pDoc->GetSdrUndoFactory().CreateUndoNewPage(*pNotesPage));

        // der Notizseite eine Masterpage zuweisen
        pNotesPage->TRG_SetMasterPage(pExample->TRG_GetMasterPage());

        // Seitengroesse setzen, es muss bereits eine Seite vorhanden sein
        pNotesPage->SetSize(pExample->GetSize());
        pNotesPage->SetBorder( pExample->GetLftBorder(),
                               pExample->GetUppBorder(),
                               pExample->GetRgtBorder(),
                               pExample->GetLwrBorder() );

        // neue Praesentationsobjekte anlegen
        pNotesPage->SetAutoLayout(pExample->GetAutoLayout(), TRUE);

        pOutliner->UpdateFields();
    }

    return pPage;
}

/*************************************************************************
|*
|* Handler fuer das Loeschen von Seiten (Absaetzen)
|*
\************************************************************************/

IMPL_LINK( OutlineView, ParagraphRemovingHdl, ::Outliner *, pOutliner )
{
    DBG_ASSERT( isRecordingUndo(), "sd::OutlineView::ParagraphRemovingHdl(), model change without undo?!" );

    OutlineViewPageChangesGuard aGuard(this);

    Paragraph* pPara = pOutliner->GetHdlParagraph();
    if ( pOutliner->GetDepth( (USHORT) pOutliner->GetAbsPos( pPara ) ) == 0 )
    {
        // wieviele Titel sind vor dem fraglichen Titelabsatz?
        ULONG nPos = 0L;
        while(pPara)
        {
            pPara = GetPrevTitle(pPara);
            if (pPara) nPos++;
        }

        // Seite und Notizseite loeschen
        USHORT nAbsPos = (USHORT)nPos * 2 + 1;
        SdrPage* pPage = pDoc->GetPage(nAbsPos);
        if( isRecordingUndo() )
            AddUndo(pDoc->GetSdrUndoFactory().CreateUndoDeletePage(*pPage));
        pDoc->RemovePage(nAbsPos);

        nAbsPos = (USHORT)nPos * 2 + 1;
        pPage = pDoc->GetPage(nAbsPos);
        if( isRecordingUndo() )
            AddUndo(pDoc->GetSdrUndoFactory().CreateUndoDeletePage(*pPage));
        pDoc->RemovePage(nAbsPos);

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

IMPL_LINK( OutlineView, DepthChangedHdl, ::Outliner *, pOutliner )
{
    DBG_ASSERT( isRecordingUndo(), "sd::OutlineView::DepthChangedHdl(), no undo for model change?!" );

    OutlineViewPageChangesGuard aGuard(this);

    Paragraph* pPara = pOutliner->GetHdlParagraph();
    if ( pOutliner->GetDepth( (USHORT) pOutliner->GetAbsPos( pPara ) ) == 0 )
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
                if ( pOutliner->GetDepth( (USHORT) pOutliner->GetAbsPos( pParagraph ) ) == 1 )
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

        USHORT nAbsPos = (USHORT)nPos * 2 + 1;
        SdrPage* pPage = pDoc->GetPage(nAbsPos);
        if( isRecordingUndo() )
            AddUndo(pDoc->GetSdrUndoFactory().CreateUndoDeletePage(*pPage));
        pDoc->RemovePage(nAbsPos);

        nAbsPos = (USHORT)nPos * 2 + 1;
        pPage = pDoc->GetPage(nAbsPos);
        if( isRecordingUndo() )
            AddUndo(pDoc->GetSdrUndoFactory().CreateUndoDeletePage(*pPage));
        pDoc->RemovePage(nAbsPos);

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
    else if ( (pOutliner->GetPrevDepth() == 1) && ( pOutliner->GetDepth( (USHORT) pOutliner->GetAbsPos( pPara ) ) == 2 ) )
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
            SdPage*pPage = (SdPage*)pDoc->GetSdPage( (USHORT) nPos, PK_STANDARD);

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
        SdPage* pPage = (SdPage*) pDoc->GetSdPage( (USHORT) nPos, PK_STANDARD );

        if( pPage )
        {
            SfxStyleSheet* pStyleSheet = NULL;
            ULONG nPara = pOutliner->GetAbsPos( pPara );
            ULONG nDepth = pOutliner->GetDepth( (USHORT) nPara );
            BOOL bSubTitle = pPage->GetPresObj(PRESOBJ_TEXT) != NULL;

            if( nDepth == 0 )
            {
                pStyleSheet = pPage->GetStyleSheetForPresObj( PRESOBJ_TITLE );
            }
            else if( bSubTitle )
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

            // before we set the style sheet we need to preserve the bullet item
            // since all items will be deleted while setting a new style sheet
             SfxItemSet aOldAttrs( pOutliner->GetParaAttribs( nPara ) );

            pOutliner->SetStyleSheet( nPara, pStyleSheet );

            // restore the old bullet item but not if the style changed
            if ( pOutliner->GetPrevDepth() != 0 &&
                 nDepth != 0 &&
                 aOldAttrs.GetItemState( EE_PARA_NUMBULLET ) == SFX_ITEM_ON )
            {
                SfxItemSet aAttrs( pOutliner->GetParaAttribs( nPara ) );
                aAttrs.Put( *aOldAttrs.GetItem( EE_PARA_NUMBULLET ) );
                pOutliner->SetParaAttribs( nPara, aAttrs );
            }
        }
    }

    return 0;
}

/*************************************************************************
|*
|* Handler fuer StatusEvents
|*
\************************************************************************/

IMPL_LINK( OutlineView, StatusEventHdl, EditStatus *, pEditStatus )
{
    ::sd::Window*   pWin = pOutlineViewShell->GetActiveWindow();
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

IMPL_LINK( OutlineView, BeginDropHdl, void *, EMPTYARG )
{
    DBG_ASSERT(maDragAndDropModelGuard.get() == 0, "sd::OutlineView::BeginDropHdl(), prior drag operation not finished correctly!" );

    maDragAndDropModelGuard.reset( new OutlineViewModelChangeGuard( *this ) );
    return 0;
}

IMPL_LINK( OutlineView, EndDropHdl, void *, EMPTYARG )
{
    maDragAndDropModelGuard.reset(0);
    return 0;
}

/*************************************************************************
|*
|* Handler fuer den Beginn einer Absatzverschiebung
|*
\************************************************************************/

IMPL_LINK( OutlineView, BeginMovingHdl, ::Outliner *, pOutliner )
{
    DBG_ASSERT(!pSelectedParas, "Absatzliste nicht geloescht");
    DBG_ASSERT(!pOldParaOrder, "Absatzliste nicht geloescht");

    OutlineViewPageChangesGuard aGuard(this);

    pOldParaOrder = new List;

    // Liste der selektierten Titelabsaetze
    pSelectedParas = pOutlinerView[0]->CreateSelectionList();
    Paragraph* pPara = (Paragraph*)pSelectedParas->First();
    while (pPara)
    {
        if ( pOutliner->GetDepth( (USHORT) pOutliner->GetAbsPos( pPara ) ) > 0 )
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
        if ( pOutliner->GetDepth( (USHORT) nParaPos ) == 0 )                     // eine Seite?
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

IMPL_LINK( OutlineView, EndMovingHdl, ::Outliner *, pOutliner )
{
    OutlineViewPageChangesGuard aGuard(this);

    DBG_ASSERT(pSelectedParas, "keine Absatzliste");
    DBG_ASSERT(pOldParaOrder, "keine Absatzliste");
    DBG_ASSERT( isRecordingUndo(), "sd::OutlineView::EndMovingHdl(), model change without undo?!" );

    // Einfuegeposition anhand des ersten Absatzes suchen
    Paragraph* pSearchIt = (Paragraph*)pSelectedParas->First();

    // den ersten der selektierten Paragraphen in der neuen Ordnung suchen
    USHORT nPosNewOrder = 0;
    ULONG nParaPos = 0;
    Paragraph*  pPara = pOutliner->GetParagraph( 0 );
    Paragraph*  pPrev = NULL;
    while (pPara && pPara != pSearchIt)
    {
        if (pOutliner->GetDepth( (USHORT) nParaPos ) == 0)
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

SdrTextObj* OutlineView::GetTitleTextObject(SdrPage* pPage)
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

SdrTextObj* OutlineView::GetOutlineTextObject(SdrPage* pPage)
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

SdrTextObj* OutlineView::CreateTitleTextObject(SdPage* pPage)
{
    DBG_ASSERT( GetTitleTextObject(pPage) == 0, "sd::OutlineView::CreateTitleTextObject(), there is already a title text object!" );

    if( pPage->GetAutoLayout() == AUTOLAYOUT_NONE )
    {
        // simple case
        pPage->SetAutoLayout( AUTOLAYOUT_ONLY_TITLE, true );
    }
    else
    {
        // we already have a layout with a title but the title
        // object was deleted, create a new one
        pPage->InsertAutoLayoutShape( 0, PRESOBJ_TITLE, false, pPage->GetTitleRect(), true );
    }

    return GetTitleTextObject(pPage);
}

SdrTextObj* OutlineView::CreateOutlineTextObject(SdPage* pPage)
{
    DBG_ASSERT( GetOutlineTextObject(pPage) == 0, "sd::OutlineView::CreateOutlineTextObject(), there is already a layout text object!" );


    AutoLayout eNewLayout = pPage->GetAutoLayout();
    switch( eNewLayout )
    {
    case AUTOLAYOUT_NONE:
    case AUTOLAYOUT_ONLY_TITLE:
    case AUTOLAYOUT_TITLE:  eNewLayout = AUTOLAYOUT_ENUM; break;

    case AUTOLAYOUT_CHART:  eNewLayout = AUTOLAYOUT_CHARTTEXT; break;

    case AUTOLAYOUT_ORG:
    case AUTOLAYOUT_TAB:
    case AUTOLAYOUT_OBJ:    eNewLayout = AUTOLAYOUT_OBJTEXT; break;
    }

    if( eNewLayout != pPage->GetAutoLayout() )
    {
        pPage->SetAutoLayout( eNewLayout, true );
    }
    else
    {
        // we already have a layout with a text but the text
        // object was deleted, create a new one
        pPage->InsertAutoLayoutShape( 0,
                                      (eNewLayout == AUTOLAYOUT_TITLE) ? PRESOBJ_TEXT : PRESOBJ_OUTLINE,
                                      false, pPage->GetLayoutRect(), true );
    }

    return GetOutlineTextObject(pPage);
}

/** updates draw model with all changes from outliner model */
BOOL OutlineView::PrepareClose(BOOL bUI)
{
    pOutliner->GetUndoManager().Clear();

    const String aUndoStr(SdResId(STR_UNDO_CHANGE_TITLE_AND_LAYOUT));
    BegUndo(aUndoStr);
    UpdateDocument();
    EndUndo();
    pDoc->SetSelected(GetActualPage(), TRUE);
    return TRUE;
}


/*************************************************************************
|*
|* Attribute des selektierten Textes setzen
|*
\************************************************************************/

BOOL OutlineView::SetAttributes(const SfxItemSet& rSet, BOOL bReplaceAll)
{
    FASTBOOL bOk = FALSE;

    OutlinerView* pOlView = GetViewByWindow(pOutlineViewShell->GetActiveWindow());

    if (pOlView)
    {
        pOlView->SetAttribs(rSet);
        bOk = TRUE;
    }

    pOutlineViewShell->Invalidate (SID_PREVIEW_STATE);

    return (bOk);
}

/*************************************************************************
|*
|* Attribute des selektierten Textes erfragen
|*
\************************************************************************/

BOOL OutlineView::GetAttributes( SfxItemSet& rTargetSet, BOOL bOnlyHardAttr ) const
{
    OutlinerView* pOlView = GetViewByWindow(
                                pOutlineViewShell->GetActiveWindow());
    DBG_ASSERT(pOlView, "keine OutlinerView gefunden");

    rTargetSet.Put( pOlView->GetAttribs(), FALSE );
    return TRUE;
}

/** creates outliner model from draw model */
void OutlineView::FillOutliner()
{
    pOutliner->GetUndoManager().Clear();
    pOutliner->EnableUndo(FALSE);
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
                    BOOL bVertical = pOPO->IsVertical();
                    pOPO->SetVertical( FALSE );
                    pOutliner->AddText(*pOPO);
                    pOPO->SetVertical( bVertical );
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
            pTO = GetOutlineTextObject(pPage);
        }
        if (pTO)
        {
            if(!(pTO->IsEmptyPresObj())) // gesetzten Gl.text gefunden
            {
                OutlinerParaObject* pOPO = pTO->GetOutlinerParaObject();
                if (pOPO)
                {
                    ULONG nParaCount1 = pOutliner->GetParagraphCount();
                    BOOL bVertical = pOPO->IsVertical();
                    pOPO->SetVertical( FALSE );
                    pOutliner->AddText(*pOPO);
                    pOPO->SetVertical( bVertical );
                    if (bSubTitle)
                    {
                        ULONG nParaCount2 = pOutliner->GetParagraphCount();
                        Paragraph* pPara = NULL;
                        for (ULONG nPara = nParaCount1; nPara < nParaCount2; nPara++)
                        {
                            pPara = pOutliner->GetParagraph(nPara);
                            if(pPara && pOutliner->GetDepth( (USHORT) nPara ) != 1 )
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

    pOutliner->EnableUndo(TRUE);
}

/*************************************************************************
|*
|* Handler fuer das Loeschen von Level-0-Absaetzen (Seiten): Warnung
|*
\************************************************************************/

IMPL_LINK( OutlineView, RemovingPagesHdl, OutlinerView *, pOutlinerView )
{
    USHORT nNumOfPages = pOutliner->GetSelPageCount();

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

    return 1;
}

/*************************************************************************
|*
|* Handler fuer das Einruecken von Level-0-Absaetzen (Seiten): Warnung
|*
\************************************************************************/

IMPL_LINK_INLINE_START( OutlineView, IndentingPagesHdl, OutlinerView *, pOutlinerView )
{
    return RemovingPagesHdl(pOutlinerView);
}
IMPL_LINK_INLINE_END( OutlineView, IndentingPagesHdl, OutlinerView *, pOutlinerView )


/** returns the first slide that is selected in the outliner or where
    the cursor is located */
SdPage* OutlineView::GetActualPage()
{
    ::sd::Window* pWin = pOutlineViewShell->GetActiveWindow();
    OutlinerView* pActiveView = GetViewByWindow(pWin);
    ::Outliner* pOutl = pActiveView->GetOutliner();
    std::auto_ptr<List> pSelList( static_cast< List* >(pActiveView->CreateSelectionList()) );

    SdPage* pCurrent = GetPageForParagraph(pOutl, static_cast<Paragraph*>(pSelList->First()) );
    DBG_ASSERT( pCurrent ||
                (pDocSh->GetUndoManager() && static_cast< sd::UndoManager *>(pDocSh->GetUndoManager())->isInUndo()) ||
                maDragAndDropModelGuard.get(),
                "sd::OutlineView::GetActualPage(), no current page?" );
    if( pCurrent )
        return pCurrent;
    else
        return pDoc->GetSdPage( 0, PK_STANDARD );
}

SdPage* OutlineView::GetPageForParagraph( ::Outliner* pOutl, Paragraph* pPara )
{
    if( pOutl->GetDepth( (USHORT) pOutl->GetAbsPos( pPara ) ) > 0 )
    {
        pPara = GetPrevTitle(pPara);
    }
    sal_uInt32 nPageToSelect = 0;
    while(pPara)
    {
        pPara = GetPrevTitle(pPara);
        if(pPara)
            nPageToSelect++;
    }

    if( nPageToSelect < (sal_uInt32)pDoc->GetSdPageCount( PK_STANDARD ) )
        return static_cast< SdPage* >( pDoc->GetSdPage( (USHORT)nPageToSelect, PK_STANDARD) );
    else
        return 0;
}

Paragraph* OutlineView::GetParagraphForPage( ::Outliner* pOutl, SdPage* pPage )
{
    // get the number of paragraphs with ident 0 we need to skip before
    // we finde the actual page
    sal_uInt32 nPagesToSkip = (pPage->GetPageNum() - 1) >> 1;

    sal_uInt32 nParaPos = 0;
    Paragraph* pPara = pOutl->GetParagraph( 0 );
    while( pPara )
    {
        // if this paragraph is a page ...
        if ( pOutl->GetDepth( (USHORT) nParaPos ) == 0 )
        {
            // see if we already skiped enough pages
            if( 0 == nPagesToSkip )
                break;  // and if so, end the loop

            // we skiped another page
            nPagesToSkip--;
        }

        // get next paragraph
        pPara = pOutliner->GetParagraph( ++nParaPos );
    }

    return pPara;
}

/** selects the paragraph for the given page at the outliner view*/
void OutlineView::SetActualPage( SdPage* pActual )
{
    if( pActual && mnIgnoreCurrentPageChangesLevel==0 && !bFirstPaint)
    {
        // if we found a paragraph, select its text at the outliner view
        Paragraph* pPara = GetParagraphForPage( pOutliner, pActual );
        if( pPara )
            pOutlinerView[0]->Select( pPara, TRUE, FALSE );
    }
}

/*************************************************************************
|*
|* StyleSheet aus der Selektion besorgen
|*
\************************************************************************/

SfxStyleSheet* OutlineView::GetStyleSheet() const
{
     ::sd::Window* pActWin = pOutlineViewShell->GetActiveWindow();
    OutlinerView* pOlView = GetViewByWindow(pActWin);
    SfxStyleSheet* pResult = pOlView->GetStyleSheet();
    return pResult;
}



/*************************************************************************
|*
|* Seiten als selektiert / nicht selektiert setzen
|*
\************************************************************************/

void OutlineView::SetSelectedPages()
{
    // Liste der selektierten Titelabsaetze
    List* pSelParas = pOutlinerView[0]->CreateSelectionList();
    Paragraph* pPara = (Paragraph*) pSelParas->First();

    while (pPara)
    {
        if ( pOutliner->GetDepth( (USHORT) pOutliner->GetAbsPos( pPara ) ) > 0 )
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
        if ( pOutliner->GetDepth( (USHORT) nParaPos ) == 0 )                     // eine Seite?
        {
            SdPage* pPage = pDoc->GetSdPage(nPos, PK_STANDARD);
            DBG_ASSERT(pPage!=NULL,
                "Trying to select non-existing page OutlineView::SetSelectedPages()");
            if (pPage != NULL)
            {
                pPage->SetSelected(FALSE);

                if (pSelParas->Seek(pPara))            // selektiert?
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

void OutlineView::SetLinks()
{
    // Benachrichtigungs-Links setzen
    pOutliner->SetParaInsertedHdl(LINK(this, OutlineView, ParagraphInsertedHdl));
    pOutliner->SetParaRemovingHdl(LINK(this, OutlineView, ParagraphRemovingHdl));
    pOutliner->SetDepthChangedHdl(LINK(this, OutlineView, DepthChangedHdl));
    pOutliner->SetBeginMovingHdl(LINK(this, OutlineView, BeginMovingHdl));
    pOutliner->SetEndMovingHdl(LINK(this, OutlineView, EndMovingHdl));
    pOutliner->SetRemovingPagesHdl(LINK(this, OutlineView, RemovingPagesHdl));
    pOutliner->SetIndentingPagesHdl(LINK(this, OutlineView, IndentingPagesHdl));
    pOutliner->SetMinDepth(0);
    pOutliner->SetStatusEventHdl(LINK(this, OutlineView, StatusEventHdl));
    pOutliner->SetBeginDropHdl(LINK(this,OutlineView, BeginDropHdl));
    pOutliner->SetEndDropHdl(LINK(this,OutlineView, EndDropHdl));
}



/*************************************************************************
|*
|* Alte Links restaurieren
|*
\************************************************************************/

void OutlineView::ResetLinks() const
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

/*************************************************************************
|*
|* AcceptDrop
|*
\************************************************************************/

sal_Int8 OutlineView::AcceptDrop (
    const AcceptDropEvent& rEvt,
    DropTargetHelper& rTargetHelper,
    ::sd::Window* pTargetWindow,
    USHORT nPage,
    USHORT nLayer)
{
    return DND_ACTION_NONE;
}

/*************************************************************************
|*
|* ExecuteDrop
|*
\************************************************************************/

sal_Int8 OutlineView::ExecuteDrop (
    const ExecuteDropEvent& rEvt,
    DropTargetHelper& rTargetHelper,
    ::sd::Window* pTargetWindow,
    USHORT nPage,
    USHORT nLayer)
{
    return DND_ACTION_NONE;
}

// #97766# Re-implement GetScriptType for this view to get correct results
sal_uInt16 OutlineView::GetScriptType() const
{
    sal_uInt16 nScriptType = ::sd::View::GetScriptType();

    if(pOutliner)
    {
        OutlinerParaObject* pTempOPObj = pOutliner->CreateParaObject();

        if(pTempOPObj)
        {
            nScriptType = pTempOPObj->GetTextObject().GetScriptType();
            delete pTempOPObj;
        }
    }

    return nScriptType;
}

void OutlineView::onUpdateStyleSettings( bool bForceUpdate /* = false */ )
{
    const bool bHighContrastMode = Application::GetSettings().GetStyleSettings().GetHighContrastMode() != 0;
    if( bForceUpdate || (mbHighContrastMode != bHighContrastMode) )
    {
        if( pOutliner )
        {
            pOutliner->ForceAutoColor( bHighContrastMode );
        }
        mbHighContrastMode = bHighContrastMode;

    }

    svtools::ColorConfig aColorConfig;
    const Color aDocColor( aColorConfig.GetColorValue( svtools::DOCCOLOR ).nColor );
    if( bForceUpdate || (maDocColor != aDocColor) )
    {
        sal_uInt16 nView;
        for( nView = 0; nView < MAX_OUTLINERVIEWS; nView++ )
        {
            if (pOutlinerView[nView] != NULL)
            {
                pOutlinerView[nView]->SetBackgroundColor( aDocColor );

                ::Window* pWindow = pOutlinerView[nView]->GetWindow();

                if( pWindow )
                    pWindow->SetBackground( Wallpaper( aDocColor ) );

            }
        }

        if( pOutliner )
            pOutliner->SetBackgroundColor( aDocColor );

        maDocColor = aDocColor;
    }
}

IMPL_LINK( OutlineView, AppEventListenerHdl, void *, EMPTYARG )
{
    onUpdateStyleSettings();
    return 0;
}




IMPL_LINK(OutlineView, EventMultiplexerListener, ::sd::tools::EventMultiplexerEvent*, pEvent)
{
    if (pEvent != NULL)
    {
        switch (pEvent->meEventId)
        {
            case tools::EventMultiplexerEvent::EID_CURRENT_PAGE:
                SetActualPage(pOutlineViewShell->GetActualPage());
                break;

            case tools::EventMultiplexerEvent::EID_PAGE_ORDER:
                if (pOutliner != NULL && pDoc!=NULL && mnIgnoreCurrentPageChangesLevel==0)
                {
                    if (((pDoc->GetPageCount()-1)%2) == 0)
                    {
                        pOutliner->Clear();
                        FillOutliner();
                        ::sd::Window* pWindow = pOutlineViewShell->GetActiveWindow();
                        if (pWindow != NULL)
                            pWindow->Invalidate();
               }
                }
                break;
        }
    }
    return 0;
}

void OutlineView::IgnoreCurrentPageChanges (bool bIgnoreChanges)
{
    if (bIgnoreChanges)
        mnIgnoreCurrentPageChangesLevel++;
    else
        mnIgnoreCurrentPageChangesLevel--;
}

/** call this method before you do anything that can modify the outliner
    and or the drawing document model. It will create needed undo actions */
void OutlineView::BeginModelChange()
{
    const String aEmpty;
    pOutliner->GetUndoManager().EnterListAction(aEmpty,aEmpty);
    const String aUndoStr(SdResId(STR_UNDO_CHANGE_TITLE_AND_LAYOUT));
    BegUndo(aUndoStr);
}

/** call this method after BeginModelChange(), when all possible model
    changes are done. */
void OutlineView::EndModelChange()
{
    UpdateDocument();

    SfxUndoManager* pDocUndoMgr = pDocSh->GetUndoManager();

    bool bHasUndoActions = pDocUndoMgr->GetUndoActionCount() != 0;

    EndUndo();

    DBG_ASSERT( bHasUndoActions == (pOutliner->GetUndoManager().GetUndoActionCount() != 0), "sd::OutlineView::EndModelChange(), undo actions not in sync!" );

    if( bHasUndoActions )
    {
        SfxLinkUndoAction* pLink = new SfxLinkUndoAction(pDocUndoMgr);
        pOutliner->GetUndoManager().AddUndoAction(pLink);
    }

    pOutliner->GetUndoManager().LeaveListAction();

    if( bHasUndoActions && pOutliner->GetEditEngine().HasTriedMergeOnLastAddUndo() )
        TryToMergeUndoActions();
}

/** updates all changes in the outliner model to the draw model */
void OutlineView::UpdateDocument()
{
    const sal_uInt32 nPageCount = pDoc->GetSdPageCount(PK_STANDARD);
    Paragraph* pPara = pOutliner->GetParagraph( 0 );
    sal_uInt32 nPage;
    for (nPage = 0; nPage < nPageCount; nPage++)
    {
        SdPage* pPage = pDoc->GetSdPage( (USHORT)nPage, PK_STANDARD);
        pDoc->SetSelected(pPage, FALSE);

        pOutlineViewShell->UpdateTitleObject( pPage, pPara );
        pOutlineViewShell->UpdateOutlineObject( pPage, pPara );

        if( pPara )
            pPara = GetNextTitle(pPara);
    }

    DBG_ASSERT( pPara == 0, "sd::OutlineView::UpdateDocument(), slides are out of sync, creating missing ones" );
    while( pPara )
    {
        SdPage* pPage = InsertSlideForParagraph( pPara );
        pDoc->SetSelected(pPage, FALSE);

        pOutlineViewShell->UpdateTitleObject( pPage, pPara );
        pOutlineViewShell->UpdateOutlineObject( pPage, pPara );

        if( pPara )
            pPara = GetNextTitle(pPara);
    }
}

/** merge edit engine undo actions if possible */
void OutlineView::TryToMergeUndoActions()
{
    SfxUndoManager& rOutlineUndo = pOutliner->GetUndoManager();
    if( rOutlineUndo.GetUndoActionCount() > 1 )
    {
        SfxListUndoAction* pListAction = dynamic_cast< SfxListUndoAction* >( rOutlineUndo.GetUndoAction(0) );
        SfxListUndoAction* pPrevListAction = dynamic_cast< SfxListUndoAction* >( rOutlineUndo.GetUndoAction(1) );
        if( pListAction && pPrevListAction )
        {
            // find the top EditUndo action in the top undo action list
            USHORT nAction = pListAction->aUndoActions.Count();
            EditUndo* pEditUndo = 0;
            while( !pEditUndo && nAction )
            {
                pEditUndo = dynamic_cast< EditUndo* >(pListAction->aUndoActions[--nAction]);
            }

            USHORT nEditPos = nAction; // we need this later to remove the merged undo actions

            // make sure it is the only EditUndo action in the top undo list
            while( pEditUndo && nAction )
            {
                if( dynamic_cast< EditUndo* >(pListAction->aUndoActions[--nAction]) )
                    pEditUndo = 0;
            }

            // do we have one and only one EditUndo action in the top undo list?
            if( pEditUndo )
            {
                // yes, see if we can merge it with the prev undo list

                USHORT nAction = pPrevListAction->aUndoActions.Count();
                EditUndo* pPrevEditUndo = 0;
                while( !pPrevEditUndo && nAction )
                    pPrevEditUndo = dynamic_cast< EditUndo* >(pPrevListAction->aUndoActions[--nAction]);

                if( pPrevEditUndo && pPrevEditUndo->Merge( pEditUndo ) )
                {
                    // ok we merged the only EditUndo of the top undo list with
                    // the top EditUndo of the previous undo list

                    // first remove the merged undo action
                    DBG_ASSERT( pListAction->aUndoActions[nEditPos] == pEditUndo, "sd::OutlineView::TryToMergeUndoActions(), wrong edit pos!" );
                    pListAction->aUndoActions.Remove(nEditPos);
                    delete pEditUndo;

                    // now check if we also can merge the draw undo actions
                    SfxUndoManager* pDocUndoManager = pDocSh->GetUndoManager();
                    if( pDocUndoManager && ( pListAction->aUndoActions.Count() == 1 ))
                    {
                        SfxLinkUndoAction* pLinkAction = dynamic_cast< SfxLinkUndoAction* >( pListAction->aUndoActions[0] );
                        SfxLinkUndoAction* pPrevLinkAction = 0;

                        if( pLinkAction )
                        {
                            nAction = pPrevListAction->aUndoActions.Count();
                            while( !pPrevLinkAction && nAction )
                                pPrevLinkAction = dynamic_cast< SfxLinkUndoAction* >(pPrevListAction->aUndoActions[--nAction]);
                        }

                        if( pLinkAction && pPrevLinkAction &&
                            ( pLinkAction->GetAction() == pDocUndoManager->GetUndoAction(0) ) &&
                            ( pPrevLinkAction->GetAction() == pDocUndoManager->GetUndoAction(1) ) )
                        {
                            SfxListUndoAction* pSourceList = dynamic_cast< SfxListUndoAction* >(pLinkAction->GetAction());
                            SfxListUndoAction* pDestinationList = dynamic_cast< SfxListUndoAction* >(pPrevLinkAction->GetAction());

                            if( pSourceList && pDestinationList )
                            {
                                USHORT nCount = pSourceList->aUndoActions.Count();
                                USHORT nDestAction = pDestinationList->aUndoActions.Count();
                                while( nCount-- )
                                {
                                    const SfxUndoAction* pTemp = pSourceList->aUndoActions.GetObject(0);
                                    pSourceList->aUndoActions.Remove(0);
                                    pDestinationList->aUndoActions.Insert( pTemp, nDestAction++ );
                                }
                                pDestinationList->nCurUndoAction = pDestinationList->aUndoActions.Count();

                                pListAction->aUndoActions.Remove(0);
                                delete pLinkAction;

                                pDocUndoManager->RemoveLastUndoAction();
                            }
                        }
                    }

                    if( pListAction->aUndoActions.Count() )
                    {
                        // now we have to move all remaining doc undo actions from the top undo
                        // list to the previous undo list and remove the top undo list

                        USHORT nCount = pListAction->aUndoActions.Count();
                        USHORT nDestAction = pPrevListAction->aUndoActions.Count();
                        while( nCount-- )
                        {
                            const SfxUndoAction* pTemp = pListAction->aUndoActions.GetObject(0);
                            pListAction->aUndoActions.Remove(0);
                            if( pTemp )
                                pPrevListAction->aUndoActions.Insert( pTemp, nDestAction++ );
                        }
                        pPrevListAction->nCurUndoAction = pPrevListAction->aUndoActions.Count();
                    }

                    rOutlineUndo.RemoveLastUndoAction();
                }
            }
        }
    }
}

OutlineViewModelChangeGuard::OutlineViewModelChangeGuard( OutlineView& rView )
: mrView( rView )
{
    mrView.BeginModelChange();
}

OutlineViewModelChangeGuard::~OutlineViewModelChangeGuard()
{
    mrView.EndModelChange();
}

OutlineViewPageChangesGuard::OutlineViewPageChangesGuard( OutlineView* pView )
: mpView( pView )
{
    if( mpView )
        mpView->IgnoreCurrentPageChanges( true );
}

OutlineViewPageChangesGuard::~OutlineViewPageChangesGuard()
{
    if( mpView )
        mpView->IgnoreCurrentPageChanges( false );
}

} // end of namespace sd
