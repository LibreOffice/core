/*************************************************************************
 *
 *  $RCSfile: docsh3.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:55 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include "rangelst.hxx"
#include <svx/flstitem.hxx>
#include <svx/pageitem.hxx>
#include <svx/paperinf.hxx>
#include <svx/postattr.hxx>
//#include <svx/postdlg.hxx>
#include <svx/sizeitem.hxx>
#include <offmgr/app.hxx>

#ifdef ONE_LINGU
#else
#include <offmgr/osplcfg.hxx>
#endif

#include <sfx2/misccfg.hxx>
#include <sfx2/printer.hxx>
#include <svtools/ctrltool.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>

#include "docsh.hxx"
#include "scmod.hxx"
#include "tabvwsh.hxx"
#include "viewdata.hxx"
#include "docpool.hxx"
#include "stlpool.hxx"
#include "patattr.hxx"
#include "uiitems.hxx"
#include "hints.hxx"
#include "docoptio.hxx"
#include "viewopti.hxx"
#include "pntlock.hxx"
#include "chgtrack.hxx"
#include "docfunc.hxx"
#include "cell.hxx"
#include "chgviset.hxx"
#include "progress.hxx"
#include "redcom.hxx"
#include "sc.hrc"

//------------------------------------------------------------------

//
//          Redraw - Benachrichtigungen
//


void ScDocShell::PostEditView( ScEditEngineDefaulter* pEditEngine, const ScAddress& rCursorPos )
{
//  Broadcast( ScEditViewHint( pEditEngine, rCursorPos ) );

        //  Test: nur aktive ViewShell

    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    if (pViewSh && pViewSh->GetViewData()->GetDocShell() == this)
    {
        ScEditViewHint aHint( pEditEngine, rCursorPos );
        pViewSh->Notify( *this, aHint );
    }
}

void ScDocShell::PostDataChanged()
{
    Broadcast( SfxSimpleHint( FID_DATACHANGED ) );
    aDocument.ResetChanged( ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB) );

    SFX_APP()->Broadcast(SfxSimpleHint( FID_ANYDATACHANGED ));      // Navigator
    //! Navigator direkt benachrichtigen!
}

void ScDocShell::PostPaint( USHORT nStartCol, USHORT nStartRow, USHORT nStartTab,
                            USHORT nEndCol, USHORT nEndRow, USHORT nEndTab, USHORT nPart,
                            USHORT nExtFlags )
{
    if (nStartCol > MAXCOL) nStartCol = MAXCOL;
    if (nStartRow > MAXROW) nStartRow = MAXROW;
    if (nEndCol > MAXCOL) nEndCol = MAXCOL;
    if (nEndRow > MAXROW) nEndRow = MAXROW;

    if ( pPaintLockData )
    {
        //! nExtFlags ???
        pPaintLockData->AddRange( ScRange( nStartCol, nStartRow, nStartTab,
                                            nEndCol, nEndRow, nEndTab ), nPart );
        return;
    }


    if (nExtFlags & SC_PF_LINES)            // Platz fuer Linien berücksichtigen
    {
                                            //! Abfrage auf versteckte Spalten/Zeilen!
        if (nStartCol>0) --nStartCol;
        if (nEndCol<MAXCOL) ++nEndCol;
        if (nStartRow>0) --nStartRow;
        if (nEndRow<MAXROW) ++nEndRow;
    }

                                            // um zusammengefasste erweitern
    if (nExtFlags & SC_PF_TESTMERGE)
        aDocument.ExtendMerge( nStartCol, nStartRow, nEndCol, nEndRow, nStartTab );

    if ( nStartCol != 0 || nEndCol != MAXCOL )      // gedreht -> ganze Zeilen
    {
        if ( aDocument.HasAttrib( 0,nStartRow,nStartTab,
                                    MAXCOL,nEndRow,nEndTab, HASATTR_ROTATE ) )
        {
            nStartCol = 0;
            nEndCol = MAXCOL;
        }
    }

    Broadcast( ScPaintHint( ScRange( nStartCol, nStartRow, nStartTab,
                                     nEndCol, nEndRow, nEndTab ), nPart ) );

    if ( nPart & PAINT_GRID )
        aDocument.ResetChanged( ScRange(nStartCol,nStartRow,nStartTab,nEndCol,nEndRow,nEndTab) );
}

void ScDocShell::PostPaint( const ScRange& rRange, USHORT nPart, USHORT nExtFlags )
{
    PostPaint( rRange.aStart.Col(), rRange.aStart.Row(), rRange.aStart.Tab(),
               rRange.aEnd.Col(),   rRange.aEnd.Row(),   rRange.aEnd.Tab(),
               nPart, nExtFlags );
}

void ScDocShell::PostPaintGridAll()
{
    PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID );
}

void ScDocShell::PostPaintCell( USHORT nCol, USHORT nRow, USHORT nTab )
{
    PostPaint( nCol,nRow,nTab, nCol,nRow,nTab, PAINT_GRID, SC_PF_TESTMERGE );
}

void ScDocShell::PostPaintExtras()
{
    PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_EXTRAS );
}

//------------------------------------------------------------------

USHORT ScDocShell::GetLockCount() const
{
    if (pPaintLockData)
        return pPaintLockData->GetLevel() + 1;      // erster Lock setzt Level auf 0
    else
        return 0;
}

void ScDocShell::SetLockCount(USHORT nNew)
{
    if (nNew)                   // setzen
    {
        if ( !pPaintLockData )
            pPaintLockData = new ScPaintLockData(0);    //! Modus...
        pPaintLockData->SetLevel(nNew-1);
    }
    else if (pPaintLockData)    // loeschen
    {
        pPaintLockData->SetLevel(0);    // bei Unlock sofort ausfuehren
        UnlockPaint();                  // jetzt
    }
}

void ScDocShell::LockPaint()
{
    if ( pPaintLockData )
        pPaintLockData->IncLevel();
    else
        pPaintLockData = new ScPaintLockData(0);    //! Modus...
}

void ScDocShell::UnlockPaint()
{
    if ( pPaintLockData )
    {
        if ( pPaintLockData->GetLevel() )
            pPaintLockData->DecLevel();
        else
        {
            //      Paint jetzt ausfuehren

            ScPaintLockData* pPaint = pPaintLockData;
            pPaintLockData = NULL;                      // nicht weitersammeln

            ScRangeListRef xRangeList = pPaint->GetRangeList();
            if (xRangeList)
            {
                USHORT nParts = pPaint->GetParts();
                ULONG nCount = xRangeList->Count();
                for ( ULONG i=0; i<nCount; i++ )
                {
                    //! nExtFlags ???
                    ScRange aRange = *xRangeList->GetObject(i);
                    PostPaint( aRange.aStart.Col(), aRange.aStart.Row(), aRange.aStart.Tab(),
                                aRange.aEnd.Col(), aRange.aEnd.Row(), aRange.aEnd.Tab(),
                                nParts );
                }
            }

            if ( pPaint->GetModified() )
                SetDocumentModified();

            delete pPaint;
        }
    }
    else
        DBG_ERROR("UnlockPaint ohne LockPaint");
}

//------------------------------------------------------------------

void ScDocShell::SetInplace( BOOL bInplace )
{
    if (bIsInplace != bInplace)
    {
        bIsInplace = bInplace;
        CalcOutputFactor();
    }
}

void ScDocShell::CalcOutputFactor()
{
    if (bIsInplace)
    {
        nPrtToScreenFactor = 1.0;           // passt sonst nicht zur inaktiven Darstellung
        return;
    }

    String aTestString = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz01234567890123456789" ));
    long nPrinterWidth = 0;
    long nWindowWidth = 0;
    const ScPatternAttr* pPattern = (const ScPatternAttr*)&aDocument.GetPool()->
                                            GetDefaultItem(ATTR_PATTERN);

    Font aDefFont;
    Printer* pPrinter = GetPrinter();
    if (pPrinter)
    {
        MapMode aOldMode = pPrinter->GetMapMode();
        Font    aOldFont = pPrinter->GetFont();

        pPrinter->SetMapMode(MAP_PIXEL);
        pPattern->GetFont(aDefFont, pPrinter);
        pPrinter->SetFont(aDefFont);
        nPrinterWidth = pPrinter->PixelToLogic( Size( pPrinter->GetTextWidth(aTestString), 0 ),
                                                    MAP_100TH_MM ).Width();
        pPrinter->SetFont(aOldFont);
        pPrinter->SetMapMode(aOldMode);
    }
    else
        DBG_ERROR("kein Drucker ?!?!?");

    VirtualDevice aVirtWindow( *Application::GetDefaultDevice() );
    aVirtWindow.SetMapMode(MAP_PIXEL);
    pPattern->GetFont(aDefFont, &aVirtWindow);
    aVirtWindow.SetFont(aDefFont);
    nWindowWidth = aVirtWindow.GetTextWidth(aTestString);
    nWindowWidth = (long) ( nWindowWidth / ScGlobal::nScreenPPTX * HMM_PER_TWIPS );

    if (nPrinterWidth && nWindowWidth)
        nPrtToScreenFactor = nPrinterWidth / (double) nWindowWidth;
    else
    {
        DBG_ERROR("GetTextSize gibt 0 ??");
        nPrtToScreenFactor = 1.0;
    }
}

double ScDocShell::GetOutputFactor() const
{
    return nPrtToScreenFactor;
}

//---------------------------------------------------------------------

void ScDocShell::InitOptions()          // Fortsetzung von InitNew (CLOOKs)
{
    //  Einstellungen aus dem SpellCheckCfg kommen in Doc- und ViewOptions

#ifdef ONE_LINGU
    USHORT nDefLang;
    BOOL bAutoSpell, bHideAuto;
    ScModule::GetSpellSettings( nDefLang, bAutoSpell, bHideAuto );
#else
    OfaCfgSpellCheck* pSpellCheckCfg = OFF_APP()->GetSpellChecker();
#endif
    ScModule* pScMod = SC_MOD();

    ScDocOptions  aDocOpt  = pScMod->GetDocOptions();
    ScViewOptions aViewOpt = pScMod->GetViewOptions();
#ifdef ONE_LINGU
    aDocOpt.SetAutoSpell( bAutoSpell );
    aViewOpt.SetHideAutoSpell( bHideAuto );
#else
    aDocOpt.SetAutoSpell( pSpellCheckCfg->IsAutoSpell() );
    aViewOpt.SetHideAutoSpell( pSpellCheckCfg->IsHideSpell() );
#endif

    // zweistellige Jahreszahleneingabe aus Extras->Optionen->Allgemein->Sonstiges
    aDocOpt.SetYear2000( SFX_APP()->GetMiscConfig()->GetYear2000() );

    aDocument.SetDocOptions( aDocOpt );
    aDocument.SetViewOptions( aViewOpt );

    //  Druck-Optionen werden jetzt direkt vor dem Drucken gesetzt

#ifdef ONE_LINGU
    aDocument.SetLanguage( (LanguageType) nDefLang );
#else
    aDocument.SetLanguage( (LanguageType) pSpellCheckCfg->GetDefaultLanguage() );
#endif
}

//---------------------------------------------------------------------

Printer* ScDocShell::GetDocumentPrinter()       // fuer OLE
{
    return aDocument.GetPrinter();
}

SfxPrinter* ScDocShell::GetPrinter()
{
    return aDocument.GetPrinter();
}

void ScDocShell::UpdateFontList()
{
    delete pFontList;
    pFontList = new FontList( GetPrinter(), Application::GetDefaultDevice() );
    SvxFontListItem aFontListItem( pFontList, SID_ATTR_CHAR_FONTLIST );
    PutItem( aFontListItem );

    CalcOutputFactor();
}

USHORT ScDocShell::SetPrinter( SfxPrinter* pNewPrinter, USHORT nDiffFlags )
{
    if (nDiffFlags & SFX_PRINTER_PRINTER)
    {
        if ( aDocument.GetPrinter() != pNewPrinter )
        {
            aDocument.SetPrinter( pNewPrinter );
            aDocument.SetPrintOptions();

            delete pFontList;
            pFontList = new FontList( pNewPrinter, Application::GetDefaultDevice() );
            SvxFontListItem aFontListItem( pFontList, SID_ATTR_CHAR_FONTLIST );
            PutItem( aFontListItem );

            CalcOutputFactor();
        }
    }
    else if (nDiffFlags & SFX_PRINTER_JOBSETUP)
    {
        SfxPrinter* pOldPrinter = aDocument.GetPrinter();
        if (pOldPrinter)
            pOldPrinter->SetJobSetup( pNewPrinter->GetJobSetup() );
    }

    if (nDiffFlags & SFX_PRINTER_OPTIONS)
    {
        aDocument.SetPrintOptions();        //! aus neuem Printer ???
    }

    if (nDiffFlags & (SFX_PRINTER_CHG_ORIENTATION | SFX_PRINTER_CHG_SIZE))
    {
        String aStyle = aDocument.GetPageStyle( GetCurTab() );
        ScStyleSheetPool* pStPl = aDocument.GetStyleSheetPool();
        SfxStyleSheet* pStyleSheet = (SfxStyleSheet*)pStPl->Find(aStyle, SFX_STYLE_FAMILY_PAGE);
        if (pStyleSheet)
        {
            SfxItemSet& rSet = pStyleSheet->GetItemSet();

            if (nDiffFlags & SFX_PRINTER_CHG_ORIENTATION)
            {
                const SvxPageItem& rOldItem = (const SvxPageItem&)rSet.Get(ATTR_PAGE);
                BOOL bWasLand = rOldItem.IsLandscape();
                BOOL bNewLand = ( pNewPrinter->GetOrientation() == ORIENTATION_LANDSCAPE );
                if (bNewLand != bWasLand)
                {
                    SvxPageItem aNewItem( rOldItem );
                    aNewItem.SetLandscape( bNewLand );
                    rSet.Put( aNewItem );

                    //  Groesse umdrehen
                    Size aOldSize = ((const SvxSizeItem&)rSet.Get(ATTR_PAGE_SIZE)).GetSize();
                    Size aNewSize(aOldSize.Height(),aOldSize.Width());
                    SvxSizeItem aNewSItem(ATTR_PAGE_SIZE,aNewSize);
                    rSet.Put( aNewSItem );
                }
            }
            if (nDiffFlags & SFX_PRINTER_CHG_SIZE)
            {
                SvxSizeItem aPaperSizeItem( ATTR_PAGE_SIZE, SvxPaperInfo::GetPaperSize(pNewPrinter) );
                rSet.Put( aPaperSizeItem );
            }
        }
    }

    PostPaint(0,0,0,MAXCOL,MAXROW,MAXTAB,PAINT_ALL);

    return 0;
}

//---------------------------------------------------------------------

ScChangeAction* ScDocShell::GetChangeAction( const ScAddress& rPos )
{
    ScChangeTrack* pTrack = GetDocument()->GetChangeTrack();
    if (!pTrack)
        return NULL;

    USHORT nTab = rPos.Tab();

    const ScChangeAction* pFound = NULL;
    const ScChangeAction* pFoundContent = NULL;
    const ScChangeAction* pFoundMove = NULL;
    long nModified = 0;
    const ScChangeAction* pAction = pTrack->GetFirst();
    while (pAction)
    {
        ScChangeActionType eType = pAction->GetType();
        //! ScViewUtil::IsActionShown( *pAction, *pSettings, *pDoc )...
        if ( pAction->IsVisible() && eType != SC_CAT_DELETE_TABS )
        {
            const ScBigRange& rBig = pAction->GetBigRange();
            if ( rBig.aStart.Tab() == nTab )
            {
                ScRange aRange = rBig.MakeRange();

                if ( eType == SC_CAT_DELETE_ROWS )
                    aRange.aEnd.SetRow( aRange.aStart.Row() );
                else if ( eType == SC_CAT_DELETE_COLS )
                    aRange.aEnd.SetCol( aRange.aStart.Col() );

                if ( aRange.In( rPos ) )
                {
                    pFound = pAction;       // der letzte gewinnt
                    switch ( pAction->GetType() )
                    {
                        case SC_CAT_CONTENT :
                            pFoundContent = pAction;
                        break;
                        case SC_CAT_MOVE :
                            pFoundMove = pAction;
                        break;
                    }
                    ++nModified;
                }
            }
            if ( pAction->GetType() == SC_CAT_MOVE )
            {
                ScRange aRange =
                    ((const ScChangeActionMove*)pAction)->
                    GetFromRange().MakeRange();
                if ( aRange.In( rPos ) )
                {
                    pFound = pAction;
                    ++nModified;
                }
            }
        }
        pAction = pAction->GetNext();
    }

    return (ScChangeAction*)pFound;
}

void ScDocShell::SetChangeComment( ScChangeAction* pAction, const String& rComment )
{
    if (pAction)
    {
        pAction->SetComment( rComment );
        //! Undo ???
        SetDocumentModified();

        //  Dialog-Notify
        ScChangeTrack* pTrack = GetDocument()->GetChangeTrack();
        if (pTrack)
        {
            ULONG nNumber = pAction->GetActionNumber();
            pTrack->NotifyModified( SC_CTM_CHANGE, nNumber, nNumber );
        }
    }
}

void ScDocShell::ExecuteChangeCommentDialog( ScChangeAction* pAction, Window* pParent,BOOL bPrevNext)
{
    if (!pAction) return;           // ohne Aktion ist nichts..

    String aComment = pAction->GetComment();
    String aAuthor = pAction->GetUser();

    International* pIntl = ScGlobal::pScInternational;
    DateTime aDT = pAction->GetDateTime();
    String aDate = pIntl->GetDate( aDT );
    aDate += ' ';
    aDate += pIntl->GetTime( aDT, FALSE, FALSE );

    SfxItemSet aSet( GetPool(),
                      SID_ATTR_POSTIT_AUTHOR, SID_ATTR_POSTIT_AUTHOR,
                      SID_ATTR_POSTIT_DATE,   SID_ATTR_POSTIT_DATE,
                      SID_ATTR_POSTIT_TEXT,   SID_ATTR_POSTIT_TEXT,
                      0 );

    aSet.Put( SvxPostItTextItem  ( aComment, SID_ATTR_POSTIT_TEXT ) );
    aSet.Put( SvxPostItAuthorItem( aAuthor,  SID_ATTR_POSTIT_AUTHOR ) );
    aSet.Put( SvxPostItDateItem  ( aDate,    SID_ATTR_POSTIT_DATE ) );

    ScRedComDialog* pDlg = new ScRedComDialog( pParent, aSet,this,pAction,bPrevNext);

    pDlg->Execute();

    delete pDlg;
}

//---------------------------------------------------------------------

void ScDocShell::CompareDocument( ScDocument& rOtherDoc )
{
    ScChangeTrack* pTrack = aDocument.GetChangeTrack();
    if ( pTrack && pTrack->GetFirst() )
    {
        //! Changes vorhanden -> Nachfrage ob geloescht werden soll
    }

    aDocument.EndChangeTracking();
    aDocument.StartChangeTracking();

    aDocument.CompareDocument( rOtherDoc );

    PostPaintGridAll();
    SetDocumentModified();
}

//---------------------------------------------------------------------
//
//              Merge (Aenderungen zusammenfuehren)
//
//---------------------------------------------------------------------

BOOL lcl_Equal( const ScChangeAction* pA, const ScChangeAction* pB )
{
    return pA && pB &&
            pA->GetUser()         == pB->GetUser() &&
            pA->GetType()         == pB->GetType() &&
            pA->GetActionNumber() == pB->GetActionNumber() &&
            pA->GetDateTimeUTC()  == pB->GetDateTimeUTC();
    //  State nicht vergleichen, falls eine alte Aenderung akzeptiert wurde
}

void ScDocShell::MergeDocument( ScDocument& rOtherDoc )
{
    ScTabViewShell* pViewSh = GetBestViewShell();   //! Funktionen an die DocShell
    if (!pViewSh)
        return;

    ScChangeTrack* pSourceTrack = rOtherDoc.GetChangeTrack();
    if (!pSourceTrack)
        return;             //! nichts zu tun - Fehlermeldung?

    ScChangeTrack* pThisTrack = aDocument.GetChangeTrack();
    if ( !pThisTrack )
    {   // anschalten
        aDocument.StartChangeTracking();
        pThisTrack = aDocument.GetChangeTrack();
        DBG_ASSERT(pThisTrack,"ChangeTracking nicht angeschaltet?");
        // #51138# visuelles RedLining einschalten
        ScChangeViewSettings aChangeViewSet;
        aChangeViewSet.SetShowChanges(TRUE);
        aDocument.SetChangeViewSettings(aChangeViewSet);
    }


    //  gemeinsame Ausgangsposition suchen
    ULONG nFirstNewNumber = 0;
    const ScChangeAction* pSourceAction = pSourceTrack->GetFirst();
    const ScChangeAction* pThisAction = pThisTrack->GetFirst();
    while ( lcl_Equal( pSourceAction, pThisAction ) )       //  gleiche Aktionen ueberspringen
    {
        nFirstNewNumber = pSourceAction->GetActionNumber() + 1;
        pSourceAction = pSourceAction->GetNext();
        pThisAction = pThisAction->GetNext();
    }
    //  pSourceAction und pThisAction zeigen jetzt auf die ersten "eigenen" Aktionen
    //  Die gemeinsamen Aktionen davor interessieren ueberhaupt nicht

    //! Abfrage, ob die Dokumente vor dem Change-Tracking gleich waren !!!


    const ScChangeAction* pFirstMergeAction = pSourceAction;
    //  MergeChangeData aus den folgenden Aktionen erzeugen
    ULONG nNewActionCount = 0;
    const ScChangeAction* pCount = pSourceAction;
    while ( pCount )
    {
        if ( !ScChangeTrack::MergeIgnore( *pCount, nFirstNewNumber ) )
            ++nNewActionCount;
        pCount = pCount->GetNext();
    }
    if (!nNewActionCount)
        return;             //! nichts zu tun - Fehlermeldung?
                            //  ab hier kein return mehr

    ScProgress aProgress( this,
                    String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("...")),
                    nNewActionCount );

    ULONG nLastMergeAction = pSourceTrack->GetLast()->GetActionNumber();
    // UpdateReference-Undo, gueltige Referenzen fuer den letzten gemeinsamen Zustand
    pSourceTrack->MergePrepare( (ScChangeAction*) pFirstMergeAction );

    //  MergeChangeData an alle noch folgenden Aktionen in diesem Dokument anpassen
    //  -> Referenzen gueltig fuer dieses Dokument
    while ( pThisAction )
    {
        ScChangeActionType eType = pThisAction->GetType();
        switch ( eType )
        {
            case SC_CAT_INSERT_COLS :
            case SC_CAT_INSERT_ROWS :
            case SC_CAT_INSERT_TABS :
                pSourceTrack->AppendInsert( pThisAction->GetBigRange().MakeRange() );
            break;
            case SC_CAT_DELETE_COLS :
            case SC_CAT_DELETE_ROWS :
            case SC_CAT_DELETE_TABS :
            {
                const ScChangeActionDel* pDel = (const ScChangeActionDel*) pThisAction;
                if ( pDel->IsTopDelete() && !pDel->IsTabDeleteCol() )
                {   // deleted Table enthaelt deleted Cols, die nicht
                    ULONG nStart, nEnd;
                    pSourceTrack->AppendDeleteRange(
                        pDel->GetOverAllRange().MakeRange(), NULL, nStart, nEnd );
                }
            }
            break;
            case SC_CAT_MOVE :
            {
                const ScChangeActionMove* pMove = (const ScChangeActionMove*) pThisAction;
                pSourceTrack->AppendMove( pMove->GetFromRange().MakeRange(),
                    pMove->GetBigRange().MakeRange(), NULL );
            }
            break;
        }
        pThisAction = pThisAction->GetNext();
    }


    //  MergeChangeData in das aktuelle Dokument uebernehmen
    BOOL bHasRejected = FALSE;
    String aOldUser = pThisTrack->GetUser();
    pThisTrack->SetUseFixDateTime( TRUE );
    ScMarkData& rMarkData = pViewSh->GetViewData()->GetMarkData();
    ScMarkData aOldMarkData( rMarkData );
    pSourceAction = pFirstMergeAction;
    while ( pSourceAction && pSourceAction->GetActionNumber() <= nLastMergeAction )
    {
        if ( !ScChangeTrack::MergeIgnore( *pSourceAction, nFirstNewNumber ) )
        {
            ScChangeActionType eSourceType = pSourceAction->GetType();
            if ( pSourceAction->IsDeletedIn() )
            {
                //! muss hier noch festgestellt werden, ob wirklich in
                //! _diesem_ Dokument geloescht?

                //  liegt in einem Bereich, der in diesem Dokument geloescht wurde
                //  -> wird weggelassen
                //! ??? Loesch-Aktion rueckgaengig machen ???
                //! ??? Aktion irgendwo anders speichern  ???
#ifndef PRODUCT
                String aValue;
                if ( eSourceType == SC_CAT_CONTENT )
                    ((const ScChangeActionContent*)pSourceAction)->GetNewString( aValue );
                ByteString aError( aValue, gsl_getSystemTextEncoding() );
                aError += " weggelassen";
                DBG_ERROR( aError.GetBuffer() );
#endif
            }
            else
            {
                //! Datum/Autor/Kommentar der Source-Aktion uebernehmen!

                pThisTrack->SetUser( pSourceAction->GetUser() );
                pThisTrack->SetFixDateTimeUTC( pSourceAction->GetDateTimeUTC() );
                ULONG nNextAction = pThisTrack->GetActionMax() + 1;

                ULONG nReject = pSourceAction->GetRejectAction();
                if (nReject)
                {
                    //  alte Aktion (aus den gemeinsamen) ablehnen
                    ScChangeAction* pOldAction = pThisTrack->GetAction( nReject );
                    if (pOldAction && pOldAction->GetState() == SC_CAS_VIRGIN)
                    {
                        //! was passiert bei Aktionen, die in diesem Dokument accepted worden sind???
                        //! Fehlermeldung oder was???
                        //! oder Reject-Aenderung normal ausfuehren

                        pThisTrack->Reject(pOldAction);
                        bHasRejected = TRUE;                // fuer Paint
                    }
                }
                else
                {
                    //  normal ausfuehren
                    ScRange aSourceRange = pSourceAction->GetBigRange().MakeRange();
                    rMarkData.SelectOneTable( aSourceRange.aStart.Tab() );
                    switch ( eSourceType )
                    {
                        case SC_CAT_CONTENT:
                        {
                            //! Test, ob es ganz unten im Dokument war, dann automatisches
                            //! Zeilen-Einfuegen ???

                            DBG_ASSERT( aSourceRange.aStart == aSourceRange.aEnd, "huch?" );
                            ScAddress aPos = aSourceRange.aStart;
                            String aValue;
                            ((const ScChangeActionContent*)pSourceAction)->GetNewString( aValue );
                            BYTE eMatrix = MM_NONE;
                            const ScBaseCell* pCell = ((const ScChangeActionContent*)pSourceAction)->GetNewCell();
                            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                                eMatrix = ((const ScFormulaCell*)pCell)->GetMatrixFlag();
                            switch ( eMatrix )
                            {
                                case MM_NONE :
                                    pViewSh->EnterData( aPos.Col(), aPos.Row(), aPos.Tab(), aValue );
                                break;
                                case MM_FORMULA :
                                {
                                    USHORT nCols, nRows;
                                    ((const ScFormulaCell*)pCell)->GetMatColsRows( nCols, nRows );
                                    aSourceRange.aEnd.SetCol( aPos.Col() + nCols - 1 );
                                    aSourceRange.aEnd.SetRow( aPos.Row() + nRows - 1 );
                                    aValue.Erase( 0, 1 );
                                    aValue.Erase( aValue.Len()-1, 1 );
                                    GetDocFunc().EnterMatrix( aSourceRange, NULL, aValue, FALSE );
                                }
                                break;
                                case MM_REFERENCE :     // do nothing
                                break;
                                case MM_FAKE :
                                    DBG_WARNING( "MergeDocument: MatrixFlag MM_FAKE" );
                                    pViewSh->EnterData( aPos.Col(), aPos.Row(), aPos.Tab(), aValue );
                                break;
                                default:
                                    DBG_ERROR( "MergeDocument: unknown MatrixFlag" );
                            }
                        }
                        break;
                        case SC_CAT_INSERT_TABS :
                        {
                            String aName;
                            aDocument.CreateValidTabName( aName );
                            GetDocFunc().InsertTable( aSourceRange.aStart.Tab(), aName, TRUE, FALSE );
                        }
                        break;
                        case SC_CAT_INSERT_ROWS:
                            GetDocFunc().InsertCells( aSourceRange, INS_INSROWS, TRUE, FALSE );
                        break;
                        case SC_CAT_INSERT_COLS:
                            GetDocFunc().InsertCells( aSourceRange, INS_INSCOLS, TRUE, FALSE );
                        break;
                        case SC_CAT_DELETE_TABS :
                            GetDocFunc().DeleteTable( aSourceRange.aStart.Tab(), TRUE, FALSE );
                        break;
                        case SC_CAT_DELETE_ROWS:
                        {
                            const ScChangeActionDel* pDel = (const ScChangeActionDel*) pSourceAction;
                            if ( pDel->IsTopDelete() )
                            {
                                aSourceRange = pDel->GetOverAllRange().MakeRange();
                                GetDocFunc().DeleteCells( aSourceRange, DEL_DELROWS, TRUE, FALSE );
                            }
                        }
                        break;
                        case SC_CAT_DELETE_COLS:
                        {
                            const ScChangeActionDel* pDel = (const ScChangeActionDel*) pSourceAction;
                            if ( pDel->IsTopDelete() && !pDel->IsTabDeleteCol() )
                            {   // deleted Table enthaelt deleted Cols, die nicht
                                aSourceRange = pDel->GetOverAllRange().MakeRange();
                                GetDocFunc().DeleteCells( aSourceRange, DEL_DELCOLS, TRUE, FALSE );
                            }
                        }
                        break;
                        case SC_CAT_MOVE :
                        {
                            const ScChangeActionMove* pMove = (const ScChangeActionMove*) pSourceAction;
                            ScRange aFromRange( pMove->GetFromRange().MakeRange() );
                            GetDocFunc().MoveBlock( aFromRange,
                                aSourceRange.aStart, TRUE, TRUE, FALSE, FALSE );
                        }
                        break;
                    }
                }
                const String& rComment = pSourceAction->GetComment();
                if ( rComment.Len() )
                {
                    ScChangeAction* pAct = pThisTrack->GetLast();
                    if ( pAct && pAct->GetActionNumber() >= nNextAction )
                        pAct->SetComment( rComment );
#ifndef PRODUCT
                    else
                        DBG_ERROR( "MergeDocument: wohin mit dem Kommentar?!?" );
#endif
                }

                // Referenzen anpassen
                pSourceTrack->MergeOwn( (ScChangeAction*) pSourceAction, nFirstNewNumber );
            }
            aProgress.SetStateCountDown( --nNewActionCount );
        }
        pSourceAction = pSourceAction->GetNext();
    }
    rMarkData = aOldMarkData;
    pThisTrack->SetUser(aOldUser);
    pThisTrack->SetUseFixDateTime( FALSE );

    pSourceTrack->Clear();      //! der ist jetzt verhunzt

    if (bHasRejected)
        PostPaintGridAll();         // Reject() paintet nicht selber
}





