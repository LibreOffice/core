/*************************************************************************
 *
 *  $RCSfile: documen8.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:14 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include "scitems.hxx"

#include <tools/string.hxx>
#include <svx/editobj.hxx>
#include <svx/editstat.hxx>
#include <svx/langitem.hxx>
#include <svx/linkmgr.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/viewsh.hxx>
#include <svtools/flagitem.hxx>
#include <svtools/intitem.hxx>
#include <svtools/zforlist.hxx>
#include <svtools/zformat.hxx>
#include <offmgr/app.hxx>
#include <sfx2/misccfg.hxx>

#ifdef ONE_LINGU
#else
#include <offmgr/osplcfg.hxx>
#endif

#include <vcl/msgbox.hxx>
#include <vcl/system.hxx>

#include "global.hxx"
#include "table.hxx"
#include "column.hxx"
#include "cell.hxx"
#include "docpool.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"
#include "docoptio.hxx"
#include "viewopti.hxx"
#include "scextopt.hxx"
#include "scdebug.hxx"
#include "rechead.hxx"
#include "ddelink.hxx"
#include "arealink.hxx"
#include "dociter.hxx"
#include "patattr.hxx"
#include "hints.hxx"
#include "editutil.hxx"
#include "progress.hxx"
#include "document.hxx"
#include "chartlis.hxx"
#include "validat.hxx"      // fuer HasMacroCalls
#include "globstr.hrc"
#include "sc.hrc"

#define GET_SCALEVALUE(set,id)  ((const SfxUInt16Item&)(set.Get( id ))).GetValue()

//  states for online spelling in the visible range (0 is set initially)
#define VSPL_START  0
#define VSPL_DONE   1


// STATIC DATA -----------------------------------------------------------



//------------------------------------------------------------------------

void ScDocument::ImplLoadDocOptions( SvStream& rStream )
{
    USHORT d,m,y;

    DBG_ASSERT( pDocOptions, "No DocOptions to load! :-(" );
    DBG_ASSERT( pFormTable,  "Missing NumberFormatter :-(" );

    pDocOptions->Load( rStream );

    if ( pDocOptions->GetStdPrecision() > 20 ) //!!! ist 20 als Maximum konstant ???
    {
        DBG_ERROR( "Document options corrupted. Setting to defaults." );
        pDocOptions->ResetDocOptions();
    }

    pDocOptions->GetDate( d,m,y );
    pFormTable->ChangeNullDate( d,m,y );
    pFormTable->ChangeStandardPrec( pDocOptions->GetStdPrecision() );
    pFormTable->SetYear2000( pDocOptions->GetYear2000() );
}

//------------------------------------------------------------------------

void ScDocument::ImplLoadViewOptions( SvStream& rStream )
{
    DBG_ASSERT( pViewOptions, "No ViewOptions to load! :-(" );
    rStream >> *pViewOptions;
}

//------------------------------------------------------------------------

void ScDocument::ImplSaveDocOptions( SvStream& rStream ) const
{
    DBG_ASSERT( pDocOptions, "No DocOptions to save! :-(" );
    pDocOptions->Save( rStream );
}

//------------------------------------------------------------------------

void ScDocument::ImplSaveViewOptions( SvStream& rStream ) const
{
    DBG_ASSERT( pViewOptions, "No ViewOptions to save! :-(" );
    rStream << *pViewOptions;
}

//------------------------------------------------------------------------

void ScDocument::ImplCreateOptions()
{
    pDocOptions  = new ScDocOptions();
    pViewOptions = new ScViewOptions();
}

//------------------------------------------------------------------------

void ScDocument::ImplDeleteOptions()
{
    delete pDocOptions;
    delete pViewOptions;
    delete pExtDocOptions;
}

//------------------------------------------------------------------------

SfxPrinter* ScDocument::GetPrinter()
{
    if ( !pPrinter )
    {
        SfxItemSet* pSet =
            new SfxItemSet( *pDocPool,
                            SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                            SID_PRINTER_CHANGESTODOC,  SID_PRINTER_CHANGESTODOC,
                            NULL );

        OfaMiscCfg* pOffCfg = SFX_APP()->GetMiscConfig();
        if ( pOffCfg )
        {
            USHORT nFlags = 0;
            if ( pOffCfg->IsPaperBinWarning() )
                nFlags |= SFX_PRINTER_CHG_ORIENTATION;
            if ( pOffCfg->IsPaperSizeWarning() )
                nFlags |= SFX_PRINTER_CHG_SIZE;
            pSet->Put( SfxFlagItem( SID_PRINTER_CHANGESTODOC, nFlags ) );
            pSet->Put( SfxBoolItem( SID_PRINTER_NOTFOUND_WARN, pOffCfg->IsNotFoundWarning() ) );
        }

        pPrinter = new SfxPrinter( pSet );
        UpdateDrawPrinter();
    }

    return pPrinter;
}

//------------------------------------------------------------------------

void ScDocument::SetPrinter( SfxPrinter* pNewPrinter )
{
    SfxPrinter* pOld = pPrinter;
    pPrinter = pNewPrinter;
    UpdateDrawPrinter();
    delete pOld;
    InvalidateTextWidth();
}

//------------------------------------------------------------------------

void ScDocument::SetPrintOptions()
{
    if ( !pPrinter ) GetPrinter(); // setzt pPrinter
    DBG_ASSERT( pPrinter, "Error in printer creation :-/" );

    if ( pPrinter )
    {
        OfaMiscCfg* pOffCfg = SFX_APP()->GetMiscConfig();
        if ( pOffCfg )
        {
            SfxItemSet aOptSet( pPrinter->GetOptions() );

            USHORT nFlags = 0;
            if ( pOffCfg->IsPaperBinWarning() )
                nFlags |= SFX_PRINTER_CHG_ORIENTATION;
            if ( pOffCfg->IsPaperSizeWarning() )
                nFlags |= SFX_PRINTER_CHG_SIZE;
            aOptSet.Put( SfxFlagItem( SID_PRINTER_CHANGESTODOC, nFlags ) );
            aOptSet.Put( SfxBoolItem( SID_PRINTER_NOTFOUND_WARN, pOffCfg->IsNotFoundWarning() ) );

            pPrinter->SetOptions( aOptSet );
        }
    }
}

//------------------------------------------------------------------------

void ScDocument::ModifyStyleSheet( SfxStyleSheetBase& rStyleSheet,
                                   const SfxItemSet&  rChanges )
{
    SfxItemSet& rSet = rStyleSheet.GetItemSet();

    switch ( rStyleSheet.GetFamily() )
    {
        case SFX_STYLE_FAMILY_PAGE:
            {
                const USHORT nOldScale        = GET_SCALEVALUE(rSet,ATTR_PAGE_SCALE);
                const USHORT nOldScaleToPages = GET_SCALEVALUE(rSet,ATTR_PAGE_SCALETOPAGES);
                rSet.Put( rChanges );
                const USHORT nNewScale        = GET_SCALEVALUE(rSet,ATTR_PAGE_SCALE);
                const USHORT nNewScaleToPages = GET_SCALEVALUE(rSet,ATTR_PAGE_SCALETOPAGES);

                if ( (nOldScale != nNewScale) || (nOldScaleToPages != nNewScaleToPages) )
                    InvalidateTextWidth( rStyleSheet.GetName() );
            }
            break;

        case SFX_STYLE_FAMILY_PARA:
            {
                BOOL bNumFormatChanged;
                if ( ScGlobal::CheckWidthInvalidate( bNumFormatChanged,
                        rSet, rChanges ) )
                    InvalidateTextWidth( NULL, NULL, bNumFormatChanged );
                ULONG nOldFormat =
                    ((const SfxUInt32Item*)&rSet.Get(
                    ATTR_VALUE_FORMAT ))->GetValue();
                ULONG nNewFormat =
                    ((const SfxUInt32Item*)&rChanges.Get(
                    ATTR_VALUE_FORMAT ))->GetValue();
                LanguageType eNewLang, eOldLang;
                eNewLang = eOldLang = LANGUAGE_DONTKNOW;
                if ( nNewFormat != nOldFormat )
                {
                    SvNumberFormatter* pFormatter = GetFormatTable();
                    eOldLang = pFormatter->GetEntry( nOldFormat )->GetLanguage();
                    eNewLang = pFormatter->GetEntry( nNewFormat )->GetLanguage();
                }

                // Bedeutung der Items in rChanges:
                //  Item gesetzt    - Aenderung uebernehmen
                //  Dontcare        - Default setzen
                //  Default         - keine Aenderung
                // ("keine Aenderung" geht nicht mit PutExtended, darum Schleife)
                for (USHORT nWhich = ATTR_PATTERN_START; nWhich <= ATTR_PATTERN_END; nWhich++)
                {
                    const SfxPoolItem* pItem;
                    SfxItemState eState = rChanges.GetItemState( nWhich, FALSE, &pItem );
                    if ( eState == SFX_ITEM_SET )
                        rSet.Put( *pItem );
                    else if ( eState == SFX_ITEM_DONTCARE )
                        rSet.ClearItem( nWhich );
                    // bei Default nichts
                }

                if ( eNewLang != eOldLang )
                    rSet.Put(
                        SvxLanguageItem( eNewLang, ATTR_LANGUAGE_FORMAT ) );
            }
            break;
    }
}

//------------------------------------------------------------------------

void ScDocument::CopyStdStylesFrom( ScDocument* pSrcDoc )
{
    pStylePool->CopyStdStylesFrom( pSrcDoc->pStylePool );
}

//------------------------------------------------------------------------

void ScDocument::InvalidateTextWidth( const String& rStyleName )
{
    const USHORT nCount = GetTableCount();
    for ( USHORT i=0; i<nCount && pTab[i]; i++ )
        if ( pTab[i]->GetPageStyle() == rStyleName )
            InvalidateTextWidth( i );
}

//------------------------------------------------------------------------

void ScDocument::InvalidateTextWidth( USHORT nTab )
{
    ScAddress aAdrFrom( 0,    0,        nTab );
    ScAddress aAdrTo  ( MAXCOL, MAXROW, nTab );
    InvalidateTextWidth( &aAdrFrom, &aAdrTo );
}

//------------------------------------------------------------------------

BOOL ScDocument::IsPageStyleInUse( const String& rStrPageStyle, USHORT* pInTab )
{
    BOOL         bInUse = FALSE;
    const USHORT nCount = GetTableCount();

    for ( USHORT i=0; !bInUse && i<nCount && pTab[i]; i++ )
        bInUse = ( pTab[i]->GetPageStyle() == rStrPageStyle );

    if ( pInTab )
        *pInTab = i-1;

    return bInUse;
}

//------------------------------------------------------------------------

BOOL ScDocument::RemovePageStyleInUse( const String& rStyle )
{
    BOOL bWasInUse = FALSE;
    const USHORT nCount = GetTableCount();

    for ( USHORT i=0; i<nCount && pTab[i]; i++ )
        if ( pTab[i]->GetPageStyle() == rStyle )
        {
            bWasInUse = TRUE;
            pTab[i]->SetPageStyle( STR_STANDARD );
        }

    return bWasInUse;
}

BOOL ScDocument::RenamePageStyleInUse( const String& rOld, const String& rNew )
{
    BOOL bWasInUse = FALSE;
    const USHORT nCount = GetTableCount();

    for ( USHORT i=0; i<nCount && pTab[i]; i++ )
        if ( pTab[i]->GetPageStyle() == rOld )
        {
            bWasInUse = TRUE;
            pTab[i]->SetPageStyle( rNew );
        }

    return bWasInUse;
}

//------------------------------------------------------------------------

void ScDocument::InvalidateTextWidth( const ScAddress* pAdrFrom,
                                      const ScAddress* pAdrTo,
                                      BOOL bBroadcast )
{
    bBroadcast = (bBroadcast && GetDocOptions().IsCalcAsShown());
    if ( pAdrFrom && !pAdrTo )
    {
        const USHORT nTab = pAdrFrom->Tab();

        if ( pTab[nTab] )
            pTab[nTab]->InvalidateTextWidth( pAdrFrom, NULL, bBroadcast );
    }
    else
    {
        const USHORT nTabStart = pAdrFrom ? pAdrFrom->Tab() : 0;
        const USHORT nTabEnd   = pAdrTo   ? pAdrTo->Tab()   : MAXTAB;

        for ( USHORT nTab=nTabStart; nTab<=nTabEnd; nTab++ )
            if ( pTab[nTab] )
                pTab[nTab]->InvalidateTextWidth( pAdrFrom, pAdrTo, bBroadcast );
    }
}

//------------------------------------------------------------------------

#define CALCMAX                 1000    // Berechnungen
#define ABORT_EVENTS            (INPUT_ANY & ~INPUT_TIMER & ~INPUT_OTHER)

BOOL ScDocument::IdleCalcTextWidth()            // TRUE = demnaechst wieder versuchen
{
    if ( bIdleDisabled || IsInLinkUpdate() )
        return FALSE;
    bIdleDisabled = TRUE;

// ULONG nMs = 0;
// USHORT nIter = 0;

    const ULONG         nStart   = GetSysTicks();
    double              nPPTX    = 0.0;
    double              nPPTY    = 0.0;
    OutputDevice*       pDev     = NULL;
    MapMode             aOldMap;
    ScStyleSheet*       pStyle   = NULL;
    ScColumnIterator*   pColIter = NULL;
    ScTable*            pTable   = NULL;
    ScColumn*           pColumn  = NULL;
    ScBaseCell*         pCell    = NULL;
    USHORT              nTab     = aCurTextWidthCalcPos.Tab();
    USHORT              nRow     = aCurTextWidthCalcPos.Row();
    short               nCol     = aCurTextWidthCalcPos.Col();
    USHORT              nRestart = 0;
    USHORT              nZoom    = 0;
    BOOL                bNeedMore= FALSE;

    if ( nRow > MAXROW )
        nRow = 0, nCol--;
    if ( nCol < 0 )
        nCol = MAXCOL, nTab++;
    if ( nTab > MAXTAB || !pTab[nTab] )
        nTab = 0;

//  DBG_ERROR( String("Start = ") + String(nTab) + String(',') + String(nCol) + String(',') + String(nRow)  );

    //  SearchMask/Family muss gemerkt werden,
    //  damit z.B. der Organizer nicht durcheinanderkommt, wenn zwischendurch eine
    //  Query-Box aufgemacht wird !!!

    USHORT nOldMask = pStylePool->GetSearchMask();
    SfxStyleFamily eOldFam = pStylePool->GetSearchFamily();

    pTable = pTab[nTab];
    pStylePool->SetSearchMask( SFX_STYLE_FAMILY_PAGE, SFXSTYLEBIT_ALL );
    pStyle = (ScStyleSheet*)pStylePool->Find( pTable->aPageStyle,
                                              SFX_STYLE_FAMILY_PAGE );

    if ( !pStyle )
        DBG_ERROR( "Missing StyleSheet :-/" );


    BOOL bProgress = FALSE;
    if ( pStyle && 0 == GET_SCALEVALUE(pStyle->GetItemSet(),ATTR_PAGE_SCALETOPAGES) )
    {
        USHORT nCount = 0;

        nZoom    = GET_SCALEVALUE(pStyle->GetItemSet(),ATTR_PAGE_SCALE);
        Fraction aZoomFract( nZoom, 100 );
        pColumn  = &pTable->aCol[nCol];
        pColIter = new ScColumnIterator( pColumn, nRow, MAXROW );

        while ( (nZoom > 0) && (nCount < CALCMAX) && (nRestart < 2) )
        {
            if ( pColIter->Next( nRow, pCell ) )
            {
                if ( TEXTWIDTH_DIRTY == pCell->GetTextWidth() )
                {
                    if ( !pDev )
                    {
                        pDev = GetPrinter();
                        aOldMap = pDev->GetMapMode();
                        pDev->SetMapMode( MAP_PIXEL );  // wichtig fuer GetNeededSize

                        Point aPix1000 = pDev->LogicToPixel( Point(1000,1000), MAP_TWIP );
                        nPPTX = aPix1000.X() / 1000.0;
                        nPPTY = aPix1000.Y() / 1000.0;
                    }
                    if ( !bProgress && pCell->GetCellType() == CELLTYPE_FORMULA
                      && ((ScFormulaCell*)pCell)->GetDirty() )
                    {
                        ScProgress::CreateInterpretProgress( this, FALSE );
                        bProgress = TRUE;
                    }

//                  DBG_ERROR( String("t,c,r = ") + String(nTab) + String(',') + String(nCol) + String(',') + String(nRow)  );
//                  DBG_ERROR( String("nOldWidth = ") + String(pCell->GetTextWidth()) );

                    USHORT nNewWidth = (USHORT)GetNeededSize( nCol, nRow, nTab,
                                                              pDev, nPPTX, nPPTY,
                                                              aZoomFract,aZoomFract, TRUE,
                                                              TRUE );   // bTotalSize

//                  DBG_ERROR( String("nNewWidth = ") + String(nNewWidth) );

                    pCell->SetTextWidth( nNewWidth );

                    bNeedMore = TRUE;
                }
            }
            else
            {
                BOOL bNewTab = FALSE;

                nRow = 0;
                nCol--;

                if ( nCol < 0 )
                {
                    nCol = MAXCOL;
                    nTab++;
                    bNewTab = TRUE;
                }

                if ( nTab > MAXTAB || !pTab[nTab] )
                {
                    nTab = 0;
                    nRestart++;
                    bNewTab = TRUE;
                }

                if ( nRestart < 2 )
                {
                    if ( bNewTab )
                    {
                        pTable = pTab[nTab];
                        pStyle = (ScStyleSheet*)pStylePool->Find( pTable->aPageStyle,
                                                                  SFX_STYLE_FAMILY_PAGE );

                        if ( pStyle )
                        {
                            SfxItemSet& rSet = pStyle->GetItemSet();
                            if ( GET_SCALEVALUE( rSet, ATTR_PAGE_SCALETOPAGES ) == 0 )
                                nZoom = GET_SCALEVALUE(rSet, ATTR_PAGE_SCALE );
                            else
                                nZoom = 0;
                        }
                        else
                            DBG_ERROR( "Missing StyleSheet :-/" );
                    }

                    if ( nZoom > 0 )
                    {
                        delete pColIter;

                        pColumn  = &pTable->aCol[nCol];
                        pColIter = new ScColumnIterator( pColumn, nRow, MAXROW );
                    }
                    else
                        nTab++; // Tabelle nicht mit absolutem Zoom -> naechste
                }
            }

// nIter = nCount;

            nCount++;

            // Idle Berechnung abbrechen, wenn Berechnungen laenger als
            // 50ms dauern, oder nach 32 Berechnungen mal nachschauen, ob
            // bestimmte Events anstehen, die Beachtung wuenschen:

// nMs = SysTicksToMs( GetSysTicks() - nStart );

            if (   ( 50L < SysTicksToMs( GetSysTicks() - nStart ) )
                || ( !(nCount&31) && Application::AnyInput( ABORT_EVENTS ) ) )
                nCount = CALCMAX;
        }
    }
    else
        nTab++; // Tabelle nicht mit absolutem Zoom -> naechste

    if ( bProgress )
        ScProgress::DeleteInterpretProgress();

    delete pColIter;

//  DBG_ERROR( String(nCount) + String(" End = ") + String(nTab) + String(',') + String(nCol) + String(',') + String(nRow)  );

    if (pDev)
        pDev->SetMapMode(aOldMap);

    aCurTextWidthCalcPos.SetTab( nTab );
    aCurTextWidthCalcPos.SetRow( nRow );
    aCurTextWidthCalcPos.SetCol( (USHORT)nCol );

// DBG_ERROR( String(nMs) + String(" ms (") + String(nIter) + String(')') );

    pStylePool->SetSearchMask( eOldFam, nOldMask );
    bIdleDisabled = FALSE;

    return bNeedMore;
}

//------------------------------------------------------------------------

class ScSpellStatus
{
public:
    BOOL    bModified;

    ScSpellStatus() : bModified(FALSE) {};

    DECL_LINK (EventHdl, EditStatus*);
};

IMPL_LINK( ScSpellStatus, EventHdl, EditStatus *, pStatus )
{
    ULONG nStatus = pStatus->GetStatusWord();
    if ( nStatus & EE_STAT_WRONGWORDCHANGED )
        bModified = TRUE;

    return 0;
}

//  SPELL_MAXCELLS muss mindestens 256 sein, solange am Iterator keine
//  Start-Spalte gesetzt werden kann

//! SPELL_MAXTEST fuer Timer und Idle unterschiedlich ???

//  SPELL_MAXTEST now divided between visible and rest of document

#define SPELL_MAXTEST_VIS   1
#define SPELL_MAXTEST_ALL   3
#define SPELL_MAXCELLS      256

BOOL ScDocument::OnlineSpellInRange( const ScRange& rSpellRange, ScAddress& rSpellPos,
                                     USHORT nMaxTest )
{
    ScEditEngineDefaulter* pEngine = NULL;              //! am Dokument speichern
    SfxItemSet* pDefaults = NULL;
    ScSpellStatus aStatus;

    USHORT nCellCount = 0;          // Zellen insgesamt
    USHORT nTestCount = 0;          // Aufrufe Spelling
    BOOL bChanged = FALSE;          // Aenderungen?

    USHORT nCol = rSpellRange.aStart.Col();     // iterator always starts on the left edge
    USHORT nRow = rSpellPos.Row();
    USHORT nTab = rSpellPos.Tab();
    if ( !pTab[nTab] )                          // sheet deleted?
    {
        nTab = rSpellRange.aStart.Tab();
        nRow = rSpellRange.aStart.Row();
        if ( !pTab[nTab] )
        {
            //  may happen for visible range
            return FALSE;
        }
    }
    ScHorizontalCellIterator aIter( this, nTab,
                                    rSpellRange.aStart.Col(), nRow,
                                    rSpellRange.aEnd.Col(), rSpellRange.aEnd.Row() );
    ScBaseCell* pCell = aIter.GetNext( nCol, nRow );
    //  skip everything left of rSpellPos:
    while ( pCell && nRow == rSpellPos.Row() && nCol < rSpellPos.Col() )
        pCell = aIter.GetNext( nCol, nRow );
    while ( pCell )
    {
        CellType eType = pCell->GetCellType();
        if ( eType == CELLTYPE_STRING || eType == CELLTYPE_EDIT )
        {
            if (!pEngine)
            {
                //  #71154# ScTabEditEngine is needed
                //  because MapMode must be set for some old documents
                pEngine = new ScTabEditEngine( this );
                pEngine->SetControlWord( pEngine->GetControlWord() |
                            ( EE_CNTRL_ONLINESPELLING | EE_CNTRL_ALLOWBIGOBJS ) );
                pEngine->SetStatusEventHdl( LINK( &aStatus, ScSpellStatus, EventHdl ) );
                //  Delimiters hier wie in inputhdl.cxx !!!
                pEngine->SetWordDelimiters(
                            ScEditUtil::ModifyDelimiters( pEngine->GetWordDelimiters() ) );
                pDefaults = new SfxItemSet( pEngine->GetEmptyItemSet() );

#ifdef ONE_LINGU
                pEngine->SetSpeller( OFF_APP()->GetSpellChecker() );
#endif
            }

            const ScPatternAttr* pPattern = GetPattern( nCol, nRow, nTab );
            pPattern->FillEditItemSet( pDefaults );
            pEngine->SetDefaults( pDefaults, FALSE );               //! noetig ?

#ifdef ONE_LINGU
            USHORT nCellLang = ((const SvxLanguageItem&)
                                    pPattern->GetItem(ATTR_FONT_LANGUAGE)).GetValue();
            if ( nCellLang == LANGUAGE_SYSTEM )
                nCellLang = System::GetLanguage();          // never use SYSTEM for spelling
            pEngine->SetDefaultLanguage( nCellLang );
#else
            SpellCheck& rSpeller = *OFF_APP()->GetSpellChecker();
            USHORT nCellLang = ((const SvxLanguageItem&)
                                    pPattern->GetItem(ATTR_FONT_LANGUAGE)).GetValue();
            if ( nCellLang == LANGUAGE_SYSTEM )
                nCellLang = System::GetLanguage();          // Spelling nie mit SYSTEM
            rSpeller.SetActualLanguage( nCellLang );
            pEngine->SetSpeller(&rSpeller);
#endif

            if ( eType == CELLTYPE_STRING )
            {
                String aText;
                ((ScStringCell*)pCell)->GetString(aText);
                pEngine->SetText( aText );
            }
            else
                pEngine->SetText( *((ScEditCell*)pCell)->GetData() );

            aStatus.bModified = FALSE;
            pEngine->CompleteOnlineSpelling();
            if ( aStatus.bModified )                // Fehler dazu oder weggekommen?
            {
                BOOL bNeedEdit = TRUE;                      //  Test auf einfachen Text
                if ( !pEngine->HasOnlineSpellErrors() )
                {
                    ScEditAttrTester aTester( pEngine );
                    bNeedEdit = aTester.NeedsObject();
                }

                if ( bNeedEdit )
                {
                    EditTextObject* pNewData = pEngine->CreateTextObject();
                    if ( eType == CELLTYPE_EDIT )
                        ((ScEditCell*)pCell)->SetData( pNewData,
                            pEngine->GetEditTextObjectPool() );
                    else
                        PutCell( nCol, nRow, nTab, new ScEditCell( pNewData,
                            this, pEngine->GetEditTextObjectPool() ) );
                    delete pNewData;
                }
                else                    // einfacher String
                    PutCell( nCol, nRow, nTab, new ScStringCell( pEngine->GetText() ) );

                //  Paint
                if (pShell)
                {
                    //  #47751# Seitenvorschau ist davon nicht betroffen
                    //  (sollte jedenfalls nicht)
                    ScPaintHint aHint( ScRange( nCol, nRow, nTab ), PAINT_GRID );
                    aHint.SetPrintFlag( FALSE );
                    pShell->Broadcast( aHint );
                }

                bChanged = TRUE;
            }

            if ( ++nTestCount >= nMaxTest )             // checked enough text?
                break;
        }

        if ( ++nCellCount >= SPELL_MAXCELLS )           // seen enough cells?
            break;

        pCell = aIter.GetNext( nCol, nRow );
    }

    if ( pCell )
    {
        ++nCol;                                         // continue after last cell
        if ( nCol > rSpellRange.aEnd.Col() )
        {
            nCol = rSpellRange.aStart.Col();
            ++nRow;
            if ( nRow > rSpellRange.aEnd.Row() )
                pCell = NULL;
        }
    }

    if (!pCell)         // end of range reached -> next sheet
    {
        ++nTab;
        if ( nTab > rSpellRange.aEnd.Tab() || !pTab[nTab] )
            nTab = rSpellRange.aStart.Tab();
        nCol = rSpellRange.aStart.Col();
        nRow = rSpellRange.aStart.Row();

        nVisSpellState = VSPL_DONE;     //! only if this is for the visible range
    }
    rSpellPos.Set( nCol, nRow, nTab );

    delete pDefaults;
    delete pEngine;         // bevor aStatus out of scope geht

    return bChanged;
}


BOOL ScDocument::ContinueOnlineSpelling()
{
    if ( bIdleDisabled || !pDocOptions->IsAutoSpell() || (pShell && pShell->IsReadOnly()) )
        return FALSE;

    //! use one EditEngine for both calls

    //  #41504# first check visible range
    BOOL bResult = OnlineSpellInRange( aVisSpellRange, aVisSpellPos, SPELL_MAXTEST_VIS );

    //  during first pass through visible range, always continue
    if ( nVisSpellState == VSPL_START )
        bResult = TRUE;

    if (bResult)
    {
        //  if errors found, continue there
        OnlineSpellInRange( aVisSpellRange, aVisSpellPos, SPELL_MAXTEST_ALL );
    }
    else
    {
        //  if nothing found there, continue with rest of document
        ScRange aTotalRange( 0,0,0, MAXCOL,MAXROW,MAXTAB );
        bResult = OnlineSpellInRange( aTotalRange, aOnlineSpellPos, SPELL_MAXTEST_ALL );
    }
    return bResult;
}


void ScDocument::SetOnlineSpellPos( const ScAddress& rPos )
{
    aOnlineSpellPos = rPos;

    //  skip visible area for aOnlineSpellPos
    if ( aVisSpellRange.In( aOnlineSpellPos ) )
        aOnlineSpellPos = aVisSpellRange.aEnd;
}

BOOL ScDocument::SetVisibleSpellRange( const ScRange& rNewRange )
{
    BOOL bChange = ( aVisSpellRange != rNewRange );
    if (bChange)
    {
        //  continue spelling through visible range when scrolling down
        BOOL bContDown = ( nVisSpellState == VSPL_START && rNewRange.In( aVisSpellPos ) &&
                            rNewRange.aStart.Row() >  aVisSpellRange.aStart.Row() &&
                            rNewRange.aStart.Col() == aVisSpellRange.aStart.Col() &&
                            rNewRange.aEnd.Col()   == aVisSpellRange.aEnd.Col() );

        aVisSpellRange = rNewRange;

        if ( !bContDown )
        {
            aVisSpellPos = aVisSpellRange.aStart;
            nVisSpellState = VSPL_START;
        }

        //  skip visible area for aOnlineSpellPos
        if ( aVisSpellRange.In( aOnlineSpellPos ) )
            aOnlineSpellPos = aVisSpellRange.aEnd;
    }
    return bChange;
}

void ScDocument::RemoveAutoSpellObj()
{
    //  alle Spelling-Informationen entfernen

    for (USHORT nTab=0; nTab<=MAXTAB && pTab[nTab]; nTab++)
        pTab[nTab]->RemoveAutoSpellObj();
}

//------------------------------------------------------------------------

BOOL ScDocument::IdleCheckLinks()           // TRUE = demnaechst wieder versuchen
{
    BOOL bAnyLeft = FALSE;

    const SvBaseLinks& rLinks = pLinkManager->GetLinks();
    USHORT nCount = rLinks.Count();
    for (USHORT i=0; i<nCount; i++)
    {
        SvBaseLink* pBase = *rLinks[i];
        if (pBase->ISA(ScDdeLink))
        {
            ScDdeLink* pDdeLink = (ScDdeLink*)pBase;
            if (pDdeLink->NeedsUpdate())
            {
                pDdeLink->TryUpdate();
                if (pDdeLink->NeedsUpdate())        // war nix?
                    bAnyLeft = TRUE;
            }
        }
    }

    return bAnyLeft;
}

void ScDocument::SaveDdeLinks(SvStream& rStream) const
{
    //  bei 4.0-Export alle mit Modus != DEFAULT weglassen
    BOOL bExport40 = ( rStream.GetVersion() <= SOFFICE_FILEFORMAT_40 );

    const SvBaseLinks& rLinks = pLinkManager->GetLinks();
    USHORT nCount = rLinks.Count();

    //  erstmal zaehlen...

    USHORT nDdeCount = 0;
    USHORT i;
    for (i=0; i<nCount; i++)
    {
        SvBaseLink* pBase = *rLinks[i];
        if (pBase->ISA(ScDdeLink))
            if ( !bExport40 || ((ScDdeLink*)pBase)->GetMode() == SC_DDE_DEFAULT )
                ++nDdeCount;
    }

    //  Header

    ScMultipleWriteHeader aHdr( rStream );
    rStream << nDdeCount;

    //  Links speichern

    for (i=0; i<nCount; i++)
    {
        SvBaseLink* pBase = *rLinks[i];
        if (pBase->ISA(ScDdeLink))
        {
            ScDdeLink* pLink = (ScDdeLink*)pBase;
            if ( !bExport40 || pLink->GetMode() == SC_DDE_DEFAULT )
                pLink->Store( rStream, aHdr );
        }
    }
}

void ScDocument::LoadDdeLinks(SvStream& rStream)
{
    ScMultipleReadHeader aHdr( rStream );

    USHORT nCount;
    rStream >> nCount;
    for (USHORT i=0; i<nCount; i++)
    {
        ScDdeLink* pLink = new ScDdeLink( this, rStream, aHdr );
        pLinkManager->InsertDDELink( *pLink,
                            pLink->GetAppl(), pLink->GetTopic(), pLink->GetItem() );
    }
}

BOOL ScDocument::HasDdeLinks() const
{
    if (pLinkManager)           // Clipboard z.B. hat keinen LinkManager
    {
        const SvBaseLinks& rLinks = pLinkManager->GetLinks();
        USHORT nCount = rLinks.Count();
        for (USHORT i=0; i<nCount; i++)
            if ((*rLinks[i])->ISA(ScDdeLink))
                return TRUE;
    }

    return FALSE;
}

void ScDocument::SetInLinkUpdate(BOOL bSet)
{
    //  called from TableLink and AreaLink

    DBG_ASSERT( bInLinkUpdate != bSet, "SetInLinkUpdate twice" );
    bInLinkUpdate = bSet;
}

BOOL ScDocument::IsInLinkUpdate() const
{
    //! set flag in document from dde link update
    return bInLinkUpdate || ScDdeLink::IsInUpdate();
}

void ScDocument::UpdateDdeLinks()
{
    const SvBaseLinks& rLinks = pLinkManager->GetLinks();
    USHORT nCount = rLinks.Count();
    USHORT i;

    //  #49226# falls das Updaten laenger dauert, erstmal alle Werte
    //  zuruecksetzen, damit nichts altes (falsches) stehen bleibt
    BOOL bAny = FALSE;
    for (i=0; i<nCount; i++)
    {
        SvBaseLink* pBase = *rLinks[i];
        if (pBase->ISA(ScDdeLink))
        {
            ((ScDdeLink*)pBase)->ResetValue();
            bAny = TRUE;
        }
    }
    if (bAny)
    {
        //  Formeln berechnen und painten wie im TrackTimeHdl
        TrackFormulas();
        pShell->Broadcast( SfxSimpleHint( FID_DATACHANGED ) );
        ResetChanged( ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB) );

        //  wenn FID_DATACHANGED irgendwann mal asynchron werden sollte
        //  (z.B. mit Invalidate am Window), muss hier ein Update erzwungen werden.
    }

    //  nun wirklich updaten...
    for (i=0; i<nCount; i++)
    {
        SvBaseLink* pBase = *rLinks[i];
        if (pBase->ISA(ScDdeLink))
            ((ScDdeLink*)pBase)->TryUpdate();       // bei DDE-Links TryUpdate statt Update
    }
}

BOOL ScDocument::UpdateDdeLink( const String& rAppl, const String& rTopic, const String& rItem )
{
    //  fuer refresh() per StarOne Api
    //  ResetValue() fuer einzelnen Link nicht noetig
    //! wenn's mal alles asynchron wird, aber auch hier

    BOOL bFound = FALSE;
    const SvBaseLinks& rLinks = pLinkManager->GetLinks();
    USHORT nCount = rLinks.Count();
    for (USHORT i=0; i<nCount; i++)
    {
        SvBaseLink* pBase = *rLinks[i];
        if (pBase->ISA(ScDdeLink))
        {
            ScDdeLink* pDdeLink = (ScDdeLink*)pBase;
            if ( pDdeLink->GetAppl() == rAppl &&
                 pDdeLink->GetTopic() == rTopic &&
                 pDdeLink->GetItem() == rItem )
            {
                pDdeLink->TryUpdate();
                bFound = TRUE;          // koennen theoretisch mehrere sein (Mode), darum weitersuchen
            }
        }
    }
    return bFound;
}

void ScDocument::DisconnectDdeLinks()
{
    if (pLinkManager)
    {
        const SvBaseLinks& rLinks = pLinkManager->GetLinks();
        USHORT nCount = rLinks.Count();
        for (USHORT i=0; i<nCount; i++)
        {
            SvBaseLink* pBase = *rLinks[i];
            if (pBase->ISA(ScDdeLink))
                pBase->Disconnect();            // bleibt im LinkManager eingetragen
        }
    }
}

void ScDocument::CopyDdeLinks( ScDocument* pDestDoc ) const
{
    if (bIsClip)        // aus Stream erzeugen
    {
        if (pClipData)
        {
            pClipData->Seek(0);
            pDestDoc->LoadDdeLinks(*pClipData);
        }
    }
    else                // Links direkt kopieren
    {
        const SvBaseLinks& rLinks = pLinkManager->GetLinks();
        USHORT nCount = rLinks.Count();
        for (USHORT i=0; i<nCount; i++)
        {
            SvBaseLink* pBase = *rLinks[i];
            if (pBase->ISA(ScDdeLink))
            {
                ScDdeLink* pNew = new ScDdeLink( pDestDoc, *(ScDdeLink*)pBase );

                pDestDoc->pLinkManager->InsertDDELink( *pNew,
                                pNew->GetAppl(), pNew->GetTopic(), pNew->GetItem() );
            }
        }
    }
}

void ScDocument::CreateDdeLink( const String& rAppl, const String& rTopic, const String& rItem )
{
    //  DDE-Link anlegen und nicht updaten (z.B. fuer Excel-Import,
    //  damit nicht ohne Nachfrage Verbindungen aufgebaut werden)

    BYTE nMode = SC_DDE_DEFAULT;

    //  zuerst suchen, ob schon vorhanden
    //! Dde-Links (zusaetzlich) effizienter am Dokument speichern?
    const SvBaseLinks& rLinks = pLinkManager->GetLinks();
    USHORT nCount = rLinks.Count();
    for (USHORT i=0; i<nCount; i++)
    {
        SvBaseLink* pBase = *rLinks[i];
        if (pBase->ISA(ScDdeLink))
        {
            ScDdeLink* pLink = (ScDdeLink*)pBase;
            if ( pLink->GetAppl() == rAppl &&
                 pLink->GetTopic() == rTopic &&
                 pLink->GetItem() == rItem &&
                 pLink->GetMode() == nMode )
                return;                                     // dann nichts tun
        }
    }

    //  neu anlegen, aber kein TryUpdate
    ScDdeLink* pNew = new ScDdeLink( this, rAppl, rTopic, rItem, nMode );
    pLinkManager->InsertDDELink( *pNew, rAppl, rTopic, rItem );
}

USHORT ScDocument::GetDdeLinkCount() const
{
    USHORT nDdeCount = 0;
    if (pLinkManager)
    {
        const SvBaseLinks& rLinks = pLinkManager->GetLinks();
        USHORT nCount = rLinks.Count();
        for (USHORT i=0; i<nCount; i++)
            if ((*rLinks[i])->ISA(ScDdeLink))
                ++nDdeCount;
    }
    return nDdeCount;
}

BOOL ScDocument::GetDdeLinkData( USHORT nPos, String& rAppl, String& rTopic, String& rItem ) const
{
    USHORT nDdeCount = 0;
    if (pLinkManager)
    {
        const SvBaseLinks& rLinks = pLinkManager->GetLinks();
        USHORT nCount = rLinks.Count();
        for (USHORT i=0; i<nCount; i++)
        {
            SvBaseLink* pBase = *rLinks[i];
            if (pBase->ISA(ScDdeLink))
            {
                if ( nDdeCount == nPos )
                {
                    ScDdeLink* pDde = (ScDdeLink*)pBase;
                    rAppl  = pDde->GetAppl();
                    rTopic = pDde->GetTopic();
                    rItem  = pDde->GetItem();
                    return TRUE;
                }
                ++nDdeCount;
            }
        }
    }
    return FALSE;
}

//------------------------------------------------------------------------

BOOL ScDocument::HasAreaLinks() const
{
    if (pLinkManager)           // Clipboard z.B. hat keinen LinkManager
    {
        const SvBaseLinks& rLinks = pLinkManager->GetLinks();
        USHORT nCount = rLinks.Count();
        for (USHORT i=0; i<nCount; i++)
            if ((*rLinks[i])->ISA(ScAreaLink))
                return TRUE;
    }

    return FALSE;
}

void ScDocument::UpdateAreaLinks()
{
    const SvBaseLinks& rLinks = pLinkManager->GetLinks();
    USHORT nCount = rLinks.Count();
    for (USHORT i=0; i<nCount; i++)
    {
        SvBaseLink* pBase = *rLinks[i];
        if (pBase->ISA(ScAreaLink))
            pBase->Update();
    }
}

void ScDocument::SaveAreaLinks(SvStream& rStream) const
{
    const SvBaseLinks& rLinks = pLinkManager->GetLinks();
    USHORT nCount = rLinks.Count();

    //  erstmal zaehlen...

    USHORT nAreaCount = 0;
    USHORT i;
    for (i=0; i<nCount; i++)
        if ((*rLinks[i])->ISA(ScAreaLink))      // rLinks[i] = Pointer auf Ref
            ++nAreaCount;

    //  Header

    ScMultipleWriteHeader aHdr( rStream );
    rStream << nAreaCount;

    //  Links speichern

    for (i=0; i<nCount; i++)
    {
        SvBaseLink* pBase = *rLinks[i];
        if (pBase->ISA(ScAreaLink))
        {
            ScAreaLink* pLink = (ScAreaLink*)pBase;

            aHdr.StartEntry();

            rStream.WriteByteString( pLink->GetFile(), rStream.GetStreamCharSet() );
            rStream.WriteByteString( pLink->GetFilter(), rStream.GetStreamCharSet() );
            rStream.WriteByteString( pLink->GetSource(), rStream.GetStreamCharSet() );
            rStream << pLink->GetDestArea();                // ScRange
            rStream.WriteByteString( pLink->GetOptions(), rStream.GetStreamCharSet() );
            //  filter options starting from 336

            aHdr.EndEntry();
        }
    }
}

void ScDocument::LoadAreaLinks(SvStream& rStream)
{
    ScMultipleReadHeader aHdr( rStream );

    if (!pShell)
    {
        DBG_ERROR("AreaLinks koennen nicht ohne Shell geladen werden");
        return;
    }

    String aFile, aFilter, aOptions, aSource;
    ScRange aDestArea;

    USHORT nCount;
    rStream >> nCount;
    for (USHORT i=0; i<nCount; i++)
    {
        aHdr.StartEntry();

        rStream.ReadByteString( aFile,   rStream.GetStreamCharSet() );
        rStream.ReadByteString( aFilter, rStream.GetStreamCharSet() );
        rStream.ReadByteString( aSource, rStream.GetStreamCharSet() );
        rStream >> aDestArea;
        if ( aHdr.BytesLeft() )         // Filter-Optionen ab 336
            rStream.ReadByteString( aOptions, rStream.GetStreamCharSet() );
        else
            aOptions.Erase();
        aHdr.EndEntry();

        ScAreaLink* pLink = new ScAreaLink( pShell, aFile, aFilter, aOptions,
                                            aSource, aDestArea.aStart );
        pLink->SetInCreate( TRUE );
        pLink->SetDestArea( aDestArea );
        pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, aFile, &aFilter, &aSource );
        pLink->Update();
        pLink->SetInCreate( FALSE );
    }
}


//------------------------------------------------------------------------

// TimerDelays etc.
void ScDocument::KeyInput( const KeyEvent& rKEvt )
{
    if ( pChartListenerCollection->GetCount() )
        pChartListenerCollection->StartTimer();
}

//  ----------------------------------------------------------------------------
//
//  Makro-Warnung:
//  Nur beim Laden von Dokumenten wird der Modus vorher auf ASK gesetzt, wenn
//  es in der EventConfig so eingestellt ist. Beim Ausfuehren waehrend des Ladens
//  wird dann nachgefragt. Nach dem Laden wird nachgeschaut, ob Makro-Aufrufe enthalten
//  sind (Formeln oder Gueltigkeit), und ggf. nachgefragt, sonst auf ALLOWED gesetzt.
//

BOOL ScDocument::CheckMacroWarn()
{
    if ( nMacroCallMode == SC_MACROCALL_ASK )
    {
        //  Wenn wegen !IsSecure nichts ausgefuehrt wird, braucht auch nicht gefragt zu werden
        if ( !pShell || !pShell->IsSecure() )
            return FALSE;                           // nicht ausfuehren, aber nicht umstellen

        QueryBox aBox( NULL, WinBits(WB_YES_NO | WB_DEF_YES),
                        ScGlobal::GetRscString(STR_MACRO_WARNING) );
        USHORT nRet = aBox.Execute();
        if ( nRet == RET_YES )
            nMacroCallMode = SC_MACROCALL_ALLOWED;
        else
            nMacroCallMode = SC_MACROCALL_NOTALLOWED;
    }

    return ( nMacroCallMode == SC_MACROCALL_ALLOWED );
}

BOOL ScDocument::HasMacroCallsAfterLoad()   // wird direkt nach dem Laden abgefragt
{
    //  1. ocMacro in formulas
    //  bHasMacroFunc is set when a formula with ocMacro is loaded

    if ( bHasMacroFunc )
        return TRUE;

    //  2. Gueltigkeit mit Makro-Aufruf
    //  (direkt nach dem Laden sind nur Eintraege in der Liste, die auch verwendet werden)

    if (pValidationList)
    {
        USHORT nCount = pValidationList->Count();
        for (USHORT i=0; i<nCount; i++)
        {
            const ScValidationData* pData = (*pValidationList)[i];
            if ( pData->HasErrMsg() )
            {
                String aTitle, aMsg;
                ScValidErrorStyle eStyle;
                if ( pData->GetErrMsg( aTitle, aMsg, eStyle ) && eStyle == SC_VALERR_MACRO )
                    return TRUE;
            }
        }
    }

    return FALSE;       // nichts gefunden
}




