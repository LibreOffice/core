/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------

#define _ZFORLIST_DECLARE_TABLE
#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <editeng/editeng.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <sfx2/linkmgr.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdobj.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/printer.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <vcl/virdev.hxx>
#include <comphelper/processfactory.hxx>
#include <svl/PasswordHelper.hxx>
#include <tools/tenccvt.hxx>
#include <rtl/crc.h>
#include <basic/basmgr.hxx>

#include "document.hxx"
#include "table.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "rangenam.hxx"
#include "dbcolect.hxx"
#include "pivot.hxx"
#include "docpool.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"
#include "globstr.hrc"
#include "chartarr.hxx"
#include "chartlock.hxx"
#include "rechead.hxx"
#include "global.hxx"
#include "brdcst.hxx"
#include "bcaslot.hxx"
#include "adiasync.hxx"
#include "addinlis.hxx"
#include "chartlis.hxx"
#include "markdata.hxx"
#include "conditio.hxx"
#include "validat.hxx"
#include "progress.hxx"
#include "detdata.hxx"
#include "sc.hrc"               // FID_DATACHANGED
#include "ddelink.hxx"
#include "chgtrack.hxx"
#include "chgviset.hxx"
#include "editutil.hxx"
#include "hints.hxx"
#include "dpobject.hxx"
#include "scrdata.hxx"
#include "poolhelp.hxx"
#include "unoreflist.hxx"
#include "listenercalls.hxx"
#include "recursionhelper.hxx"
#include "lookupcache.hxx"
#include "externalrefmgr.hxx"
#include "tabprotection.hxx"
#include "formulaparserpool.hxx"
#include "clipparam.hxx"
#include "macromgr.hxx"

using namespace com::sun::star;

// pImpl because including lookupcache.hxx in document.hxx isn't wanted, and
// dtor plus helpers are convenient.
struct ScLookupCacheMapImpl
{
    ScLookupCacheMap aCacheMap;
    ~ScLookupCacheMapImpl()
    {
        freeCaches();
    }
    void clear()
    {
        freeCaches();
        // Zap map.
        ScLookupCacheMap aTmp;
        aCacheMap.swap( aTmp);
    }
private:
    void freeCaches()
    {
        for (ScLookupCacheMap::iterator it( aCacheMap.begin()); it != aCacheMap.end(); ++it)
            delete (*it).second;
    }
};

// STATIC DATA -----------------------------------------------------------

ScDocument::ScDocument( ScDocumentMode  eMode,
                        SfxObjectShell* pDocShell ) :
        xServiceManager( ::comphelper::getProcessServiceFactory() ),
        mpUndoManager( NULL ),
        pEditEngine( NULL ),
        pNoteEngine( NULL ),
        pNoteItemPool( NULL ),
        pShell( pDocShell ),
        pPrinter( NULL ),
        pVirtualDevice_100th_mm( NULL ),
        pDrawLayer( NULL ),
        pColorTable( NULL ),
        pCondFormList( NULL ),
        pValidationList( NULL ),
        pFormatExchangeList( NULL ),
        pDPCollection( NULL ),
        pLinkManager( NULL ),
        pFormulaTree( NULL ),
        pEOFormulaTree( NULL ),
        pFormulaTrack( NULL ),
        pEOFormulaTrack( NULL ),
        pOtherObjects( NULL ),
        pClipData( NULL ),
        pDetOpList(NULL),
        pChangeTrack( NULL ),
        pUnoBroadcaster( NULL ),
        pUnoListenerCalls( NULL ),
        pUnoRefUndoList( NULL ),
        pChangeViewSettings( NULL ),
        pScriptTypeData( NULL ),
        pCacheFieldEditEngine( NULL ),
        pDocProtection( NULL ),
        mpClipParam( NULL),
        pExternalRefMgr( NULL ),
        mpMacroMgr( NULL ),
        pViewOptions( NULL ),
        pDocOptions( NULL ),
        pExtDocOptions( NULL ),
        pConsolidateDlgData( NULL ),
        pRecursionHelper( NULL ),
        pAutoNameCache( NULL ),
        pLookupCacheMapImpl( NULL ),
        nUnoObjectId( 0 ),
        nRangeOverflowType( 0 ),
        aCurTextWidthCalcPos(MAXCOL,0,0),
        aTableOpList( 0 ),
        nFormulaCodeInTree(0),
        nXMLImportedFormulaCount( 0 ),
        nInterpretLevel(0),
        nMacroInterpretLevel(0),
        nInterpreterTableOpLevel(0),
        nMaxTableNumber( 0 ),
        nSrcVer( SC_CURRENT_VERSION ),
        nSrcMaxRow( MAXROW ),
        nFormulaTrackCount(0),
        nHardRecalcState(0),
        nVisibleTab( 0 ),
        eLinkMode(LM_UNKNOWN),
        bAutoCalc( eMode == SCDOCMODE_DOCUMENT ),
        bAutoCalcShellDisabled( FALSE ),
        bForcedFormulaPending( FALSE ),
        bCalculatingFormulaTree( FALSE ),
        bIsClip( eMode == SCDOCMODE_CLIP ),
        bIsUndo( eMode == SCDOCMODE_UNDO ),
        bIsVisible( FALSE ),
        bIsEmbedded( FALSE ),
        bNoSetDirty( FALSE ),
        bInsertingFromOtherDoc( FALSE ),
        bLoadingMedium( false ),
        bImportingXML( false ),
        bXMLFromWrapper( FALSE ),
        bCalcingAfterLoad( FALSE ),
        bNoListening( FALSE ),
        bIdleDisabled( FALSE ),
        bInLinkUpdate( FALSE ),
        bChartListenerCollectionNeedsUpdate( FALSE ),
        bHasForcedFormulas( FALSE ),
        bInDtorClear( FALSE ),
        bExpandRefs( FALSE ),
        bDetectiveDirty( FALSE ),
        nMacroCallMode( SC_MACROCALL_ALLOWED ),
        bHasMacroFunc( FALSE ),
        nVisSpellState( 0 ),
        nAsianCompression(SC_ASIANCOMPRESSION_INVALID),
        nAsianKerning(SC_ASIANKERNING_INVALID),
        bSetDrawDefaults( FALSE ),
        bPastingDrawFromOtherDoc( FALSE ),
        nInDdeLinkUpdate( 0 ),
        bInUnoBroadcast( FALSE ),
        bInUnoListenerCall( FALSE ),
        eGrammar( formula::FormulaGrammar::GRAM_NATIVE ),
        bStyleSheetUsageInvalid( TRUE ),
        mbUndoEnabled( true ),
        mbAdjustHeightEnabled( true ),
        mbExecuteLinkEnabled( true ),
        mbChangeReadOnlyEnabled( false ),
        mbStreamValidLocked( false ),
        mnNamedRangesLockCount( 0 )
{
    SetStorageGrammar( formula::FormulaGrammar::GRAM_STORAGE_DEFAULT);

    eSrcSet = gsl_getSystemTextEncoding();

    if ( eMode == SCDOCMODE_DOCUMENT )
    {
        if ( pDocShell )
            pLinkManager = new sfx2::LinkManager( pDocShell );

        xPoolHelper = new ScPoolHelper( this );

        pTab[0]  = NULL;
        pBASM = new ScBroadcastAreaSlotMachine( this );
        pChartListenerCollection = new ScChartListenerCollection( this );
        pRefreshTimerControl = new ScRefreshTimerControl;
    }
    else
    {
        pTab[0]     = NULL;
        pBASM       = NULL;
        pChartListenerCollection = NULL;
        pRefreshTimerControl = NULL;
    }

    for (SCTAB i=1; i<=MAXTAB; i++)
        pTab[i] = NULL;

    pRangeName = new ScRangeName(this);
    pDBCollection = new ScDBCollection( 4, 4, FALSE, this );
    pSelectionAttr = NULL;
    pChartCollection = new ScChartCollection;
    apTemporaryChartLock = std::auto_ptr< ScTemporaryChartLock >( new ScTemporaryChartLock(this) );
    xColNameRanges = new ScRangePairList;
    xRowNameRanges = new ScRangePairList;
    ImplCreateOptions();
    // languages for a visible document are set by docshell later (from options)
    SetLanguage( ScGlobal::eLnge, ScGlobal::eLnge, ScGlobal::eLnge );

    aTrackTimer.SetTimeoutHdl( LINK( this, ScDocument, TrackTimeHdl ) );
    aTrackTimer.SetTimeout( 100 );
}

sfx2::LinkManager*  ScDocument::GetLinkManager()  const
{
    if ( bAutoCalc && !pLinkManager && pShell)
    {
        pLinkManager = new sfx2::LinkManager( pShell );
    }
    return pLinkManager;
}


void ScDocument::SetStorageGrammar( formula::FormulaGrammar::Grammar eGram )
{
    DBG_ASSERT(
        eGram == formula::FormulaGrammar::GRAM_ODFF ||
            eGram == formula::FormulaGrammar::GRAM_PODF,
            "ScDocument::SetStorageGrammar: wrong storage grammar");

    eStorageGrammar = eGram;

    // FIXME: the XML import shouldn't strip brackets, the compiler should
    // digest them instead, which could also speedup reference recognition
    // during import.

    eXmlImportGrammar = formula::FormulaGrammar::mergeToGrammar( eGram,
            formula::FormulaGrammar::CONV_OOO);
}


void ScDocument::SetDocVisible( BOOL bSet )
{
    //  called from view ctor - only for a visible document,
    //  each new sheet's RTL flag is initialized from the locale
    bIsVisible = bSet;
}


sal_uInt32 ScDocument::GetDocumentID() const
{
    const ScDocument* pThis = this;
    sal_uInt32 nCrc = rtl_crc32( 0, &pThis, sizeof(ScDocument*) );
    // the this pointer only might not be sufficient
    nCrc = rtl_crc32( nCrc, &pShell, sizeof(SfxObjectShell*) );
    return nCrc;
}


void ScDocument::StartChangeTracking()
{
    if (!pChangeTrack)
        pChangeTrack = new ScChangeTrack( this );
}

void ScDocument::EndChangeTracking()
{
    delete pChangeTrack;
    pChangeTrack = NULL;
}

void ScDocument::SetChangeTrack( ScChangeTrack* pTrack )
{
    DBG_ASSERT( pTrack->GetDocument() == this, "SetChangeTrack: different documents" );
    if ( !pTrack || pTrack == pChangeTrack || pTrack->GetDocument() != this )
        return ;
    EndChangeTracking();
    pChangeTrack = pTrack;
}


IMPL_LINK( ScDocument, TrackTimeHdl, Timer*, EMPTYARG )
{
    if ( ScDdeLink::IsInUpdate() )      // nicht verschachteln
    {
        aTrackTimer.Start();            // spaeter nochmal versuchen
    }
    else if (pShell)                    // ausfuehren
    {
        TrackFormulas();
        pShell->Broadcast( SfxSimpleHint( FID_DATACHANGED ) );
        ResetChanged( ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB) );

            //  modified...

        if (!pShell->IsModified())
        {
            pShell->SetModified( TRUE );
            SfxBindings* pBindings = GetViewBindings();
            if (pBindings)
            {
                pBindings->Invalidate( SID_SAVEDOC );
                pBindings->Invalidate( SID_DOC_MODIFIED );
            }
        }
    }

    return 0;
}

void ScDocument::StartTrackTimer()
{
    if (!aTrackTimer.IsActive())        // nicht ewig aufschieben
        aTrackTimer.Start();
}

ScDocument::~ScDocument()
{
    DBG_ASSERT( !bInLinkUpdate, "bInLinkUpdate in dtor" );

    bInDtorClear = TRUE;

    // first of all disable all refresh timers by deleting the control
    if ( pRefreshTimerControl )
    {   // To be sure there isn't anything running do it with a protector,
        // this ensures also that nothing needs the control anymore.
        ScRefreshTimerProtector aProt( GetRefreshTimerControlAddress() );
        delete pRefreshTimerControl, pRefreshTimerControl = NULL;
    }

    // Links aufrauemen

    if ( GetLinkManager() )
    {
        // BaseLinks freigeben
        for ( USHORT n = pLinkManager->GetServers().Count(); n; )
            pLinkManager->GetServers()[ --n ]->Closed();

        if ( pLinkManager->GetLinks().Count() )
            pLinkManager->Remove( 0, pLinkManager->GetLinks().Count() );
    }

    mxFormulaParserPool.reset();
    // Destroy the external ref mgr instance here because it has a timer
    // which needs to be stopped before the app closes.
    pExternalRefMgr.reset();

    ScAddInAsync::RemoveDocument( this );
    ScAddInListener::RemoveDocument( this );
    DELETEZ( pChartListenerCollection);   // vor pBASM wg. evtl. Listener!
    DELETEZ( pLookupCacheMapImpl);  // before pBASM because of listeners
    // BroadcastAreas vor allen Zellen zerstoeren um unnoetige
    // Einzel-EndListenings der Formelzellen zu vermeiden
    delete pBASM;       // BroadcastAreaSlotMachine
    pBASM = NULL;

    if (pUnoBroadcaster)
    {
        delete pUnoBroadcaster;     // broadcasted nochmal SFX_HINT_DYING
        pUnoBroadcaster = NULL;
    }

    delete pUnoRefUndoList;
    delete pUnoListenerCalls;

    Clear( sal_True );              // TRUE = from destructor (needed for SdrModel::ClearModel)

    if (pCondFormList)
    {
        pCondFormList->DeleteAndDestroy( 0, pCondFormList->Count() );
        DELETEZ(pCondFormList);
    }
    if (pValidationList)
    {
        pValidationList->DeleteAndDestroy( 0, pValidationList->Count() );
        DELETEZ(pValidationList);
    }
    delete pRangeName;
    delete pDBCollection;
    delete pSelectionAttr;
    apTemporaryChartLock.reset();
    delete pChartCollection;
    DeleteDrawLayer();
    delete pFormatExchangeList;
    delete pPrinter;
    ImplDeleteOptions();
    delete pConsolidateDlgData;
    delete pLinkManager;
    delete pClipData;
    delete pDetOpList;                  // loescht auch die Eintraege
    delete pChangeTrack;
    delete pEditEngine;
    delete pNoteEngine;
    SfxItemPool::Free(pNoteItemPool);
    delete pChangeViewSettings;         // und weg damit
    delete pVirtualDevice_100th_mm;

    delete pDPCollection;

    // delete the EditEngine before destroying the xPoolHelper
    delete pCacheFieldEditEngine;

    if ( xPoolHelper.is() && !bIsClip )
        xPoolHelper->SourceDocumentGone();
    xPoolHelper.clear();

    DeleteColorTable();
    delete pScriptTypeData;
    delete pOtherObjects;
    delete pRecursionHelper;

    DBG_ASSERT( !pAutoNameCache, "AutoNameCache still set in dtor" );
}

void ScDocument::InitClipPtrs( ScDocument* pSourceDoc )
{
    DBG_ASSERT(bIsClip, "InitClipPtrs und nicht bIsClip");

    if (pCondFormList)
    {
        pCondFormList->DeleteAndDestroy( 0, pCondFormList->Count() );
        DELETEZ(pCondFormList);
    }
    if (pValidationList)
    {
        pValidationList->DeleteAndDestroy( 0, pValidationList->Count() );
        DELETEZ(pValidationList);
    }

    Clear();

    xPoolHelper = pSourceDoc->xPoolHelper;

    //  bedingte Formate / Gueltigkeiten
    //! Vorlagen kopieren?
    const ScConditionalFormatList* pSourceCond = pSourceDoc->pCondFormList;
    if ( pSourceCond )
        pCondFormList = new ScConditionalFormatList(this, *pSourceCond);
    const ScValidationDataList* pSourceValid = pSourceDoc->pValidationList;
    if ( pSourceValid )
        pValidationList = new ScValidationDataList(this, *pSourceValid);

                        // Links in Stream speichern
    delete pClipData;
    if (pSourceDoc->HasDdeLinks())
    {
        pClipData = new SvMemoryStream;
        pSourceDoc->SaveDdeLinks(*pClipData);
    }
    else
        pClipData = NULL;

    // Options pointers exist (ImplCreateOptions) for any document.
    // Must be copied for correct results in OLE objects (#i42666#).
    SetDocOptions( pSourceDoc->GetDocOptions() );
    SetViewOptions( pSourceDoc->GetViewOptions() );
}

SvNumberFormatter* ScDocument::GetFormatTable() const
{
    return xPoolHelper->GetFormTable();
}

SfxItemPool* ScDocument::GetEditPool() const
{
    return xPoolHelper->GetEditPool();
}

SfxItemPool* ScDocument::GetEnginePool() const
{
    return xPoolHelper->GetEnginePool();
}

ScFieldEditEngine& ScDocument::GetEditEngine()
{
    if ( !pEditEngine )
    {
        pEditEngine = new ScFieldEditEngine( GetEnginePool(), GetEditPool() );
        pEditEngine->SetUpdateMode( FALSE );
        pEditEngine->EnableUndo( FALSE );
        pEditEngine->SetRefMapMode( MAP_100TH_MM );
        pEditEngine->SetForbiddenCharsTable( xForbiddenCharacters );
    }
    return *pEditEngine;
}

ScNoteEditEngine& ScDocument::GetNoteEngine()
{
    if ( !pNoteEngine )
    {
        pNoteEngine = new ScNoteEditEngine( GetEnginePool(), GetEditPool() );
        pNoteEngine->SetUpdateMode( FALSE );
        pNoteEngine->EnableUndo( FALSE );
        pNoteEngine->SetRefMapMode( MAP_100TH_MM );
        pNoteEngine->SetForbiddenCharsTable( xForbiddenCharacters );
                const SfxItemSet& rItemSet = GetDefPattern()->GetItemSet();
                SfxItemSet* pEEItemSet = new SfxItemSet( pNoteEngine->GetEmptyItemSet() );
                ScPatternAttr::FillToEditItemSet( *pEEItemSet, rItemSet );
                pNoteEngine->SetDefaults( pEEItemSet );      // edit engine takes ownership
    }
    return *pNoteEngine;
}


void ScDocument::ResetClip( ScDocument* pSourceDoc, const ScMarkData* pMarks )
{
    if (bIsClip)
    {
        InitClipPtrs(pSourceDoc);

        for (SCTAB i = 0; i <= MAXTAB; i++)
            if (pSourceDoc->pTab[i])
                if (!pMarks || pMarks->GetTableSelect(i))
                {
                    String aString;
                    pSourceDoc->pTab[i]->GetName(aString);
                    pTab[i] = new ScTable(this, i, aString);
                    pTab[i]->SetLayoutRTL( pSourceDoc->pTab[i]->IsLayoutRTL() );
                    nMaxTableNumber = i+1;
                }
    }
    else
    {
        OSL_FAIL("ResetClip");
    }
}

void ScDocument::ResetClip( ScDocument* pSourceDoc, SCTAB nTab )
{
    if (bIsClip)
    {
        InitClipPtrs(pSourceDoc);

        pTab[nTab] = new ScTable(this, nTab,
                            String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("baeh")));
        if (pSourceDoc->pTab[nTab])
            pTab[nTab]->SetLayoutRTL( pSourceDoc->pTab[nTab]->IsLayoutRTL() );
        nMaxTableNumber = nTab+1;
    }
    else
    {
        OSL_FAIL("ResetClip");
    }
}

void ScDocument::DeleteNumberFormat( const sal_uInt32* /* pDelKeys */, sal_uInt32 /* nCount */ )
{
/*
    for (ULONG i = 0; i < nCount; i++)
        xPoolHelper->GetFormTable()->DeleteEntry(pDelKeys[i]);
*/
}

void ScDocument::PutCell( SCCOL nCol, SCROW nRow, SCTAB nTab,
                          ScBaseCell* pCell, ULONG nFormatIndex, BOOL bForceTab )
{
    if (VALIDTAB(nTab))
    {
        if ( bForceTab && !pTab[nTab] )
        {
            BOOL bExtras = !bIsUndo;        // Spaltenbreiten, Zeilenhoehen, Flags

            pTab[nTab] = new ScTable(this, nTab,
                                String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("temp")),
                                bExtras, bExtras);
        }

        if (pTab[nTab])
            pTab[nTab]->PutCell( nCol, nRow, nFormatIndex, pCell );
    }
}


BOOL ScDocument::GetPrintArea( SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow,
                                BOOL bNotes ) const
{
    if (ValidTab(nTab) && pTab[nTab])
    {
        BOOL bAny = pTab[nTab]->GetPrintArea( rEndCol, rEndRow, bNotes );
        if (pDrawLayer)
        {
            ScRange aDrawRange(0,0,nTab, MAXCOL,MAXROW,nTab);
            if (DrawGetPrintArea( aDrawRange, TRUE, TRUE ))
            {
                if (aDrawRange.aEnd.Col()>rEndCol) rEndCol=aDrawRange.aEnd.Col();
                if (aDrawRange.aEnd.Row()>rEndRow) rEndRow=aDrawRange.aEnd.Row();
                bAny = TRUE;
            }
        }
        return bAny;
    }

    rEndCol = 0;
    rEndRow = 0;
    return FALSE;
}

BOOL ScDocument::GetPrintAreaHor( SCTAB nTab, SCROW nStartRow, SCROW nEndRow,
                                        SCCOL& rEndCol, BOOL bNotes ) const
{
    if (ValidTab(nTab) && pTab[nTab])
    {
        BOOL bAny = pTab[nTab]->GetPrintAreaHor( nStartRow, nEndRow, rEndCol, bNotes );
        if (pDrawLayer)
        {
            ScRange aDrawRange(0,nStartRow,nTab, MAXCOL,nEndRow,nTab);
            if (DrawGetPrintArea( aDrawRange, TRUE, FALSE ))
            {
                if (aDrawRange.aEnd.Col()>rEndCol) rEndCol=aDrawRange.aEnd.Col();
                bAny = TRUE;
            }
        }
        return bAny;
    }

    rEndCol = 0;
    return FALSE;
}

BOOL ScDocument::GetPrintAreaVer( SCTAB nTab, SCCOL nStartCol, SCCOL nEndCol,
                                        SCROW& rEndRow, BOOL bNotes ) const
{
    if (ValidTab(nTab) && pTab[nTab])
    {
        BOOL bAny = pTab[nTab]->GetPrintAreaVer( nStartCol, nEndCol, rEndRow, bNotes );
        if (pDrawLayer)
        {
            ScRange aDrawRange(nStartCol,0,nTab, nEndCol,MAXROW,nTab);
            if (DrawGetPrintArea( aDrawRange, FALSE, TRUE ))
            {
                if (aDrawRange.aEnd.Row()>rEndRow) rEndRow=aDrawRange.aEnd.Row();
                bAny = TRUE;
            }
        }
        return bAny;
    }

    rEndRow = 0;
    return FALSE;
}

BOOL ScDocument::GetDataStart( SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow ) const
{
    if (ValidTab(nTab) && pTab[nTab])
    {
        BOOL bAny = pTab[nTab]->GetDataStart( rStartCol, rStartRow );
        if (pDrawLayer)
        {
            ScRange aDrawRange(0,0,nTab, MAXCOL,MAXROW,nTab);
            if (DrawGetPrintArea( aDrawRange, TRUE, TRUE ))
            {
                if (aDrawRange.aStart.Col()<rStartCol) rStartCol=aDrawRange.aStart.Col();
                if (aDrawRange.aStart.Row()<rStartRow) rStartRow=aDrawRange.aStart.Row();
                bAny = TRUE;
            }
        }
        return bAny;
    }

    rStartCol = 0;
    rStartRow = 0;
    return FALSE;
}

BOOL ScDocument::MoveTab( SCTAB nOldPos, SCTAB nNewPos )
{
    if (nOldPos == nNewPos) return FALSE;
    BOOL bValid = FALSE;
    if (VALIDTAB(nOldPos))
    {
        if (pTab[nOldPos])
        {
            SCTAB nTabCount = GetTableCount();
            if (nTabCount > 1)
            {
                BOOL bOldAutoCalc = GetAutoCalc();
                SetAutoCalc( FALSE );   // Mehrfachberechnungen vermeiden
                SetNoListening( TRUE );
                ScProgress* pProgress = new ScProgress( GetDocumentShell(),
                    ScGlobal::GetRscString(STR_UNDO_MOVE_TAB), GetCodeCount() );
                if (nNewPos == SC_TAB_APPEND)
                    nNewPos = nTabCount-1;

                //  Referenz-Updaterei
                //! mit UpdateReference zusammenfassen!

                SCsTAB nDz = ((SCsTAB)nNewPos) - (SCsTAB)nOldPos;
                ScRange aSourceRange( 0,0,nOldPos, MAXCOL,MAXROW,nOldPos );
                pRangeName->UpdateTabRef(nOldPos, 3, nNewPos);
                pDBCollection->UpdateMoveTab( nOldPos, nNewPos );
                xColNameRanges->UpdateReference( URM_REORDER, this, aSourceRange, 0,0,nDz );
                xRowNameRanges->UpdateReference( URM_REORDER, this, aSourceRange, 0,0,nDz );
                if (pDPCollection)
                    pDPCollection->UpdateReference( URM_REORDER, aSourceRange, 0,0,nDz );
                if (pDetOpList)
                    pDetOpList->UpdateReference( this, URM_REORDER, aSourceRange, 0,0,nDz );
                UpdateChartRef( URM_REORDER,
                                    0,0,nOldPos, MAXCOL,MAXROW,nOldPos, 0,0,nDz );
                UpdateRefAreaLinks( URM_REORDER, aSourceRange, 0,0,nDz );
                if ( pCondFormList )
                    pCondFormList->UpdateMoveTab( nOldPos, nNewPos );
                if ( pValidationList )
                    pValidationList->UpdateMoveTab( nOldPos, nNewPos );
                if ( pUnoBroadcaster )
                    pUnoBroadcaster->Broadcast( ScUpdateRefHint( URM_REORDER,
                                    aSourceRange, 0,0,nDz ) );

                ScTable* pSaveTab = pTab[nOldPos];
                SCTAB i;
                for (i = nOldPos + 1; i < nTabCount; i++)
                    pTab[i - 1] = pTab[i];
                pTab[i-1] = NULL;
                for (i = nTabCount - 1; i > nNewPos; i--)
                    pTab[i] = pTab[i - 1];
                pTab[nNewPos] = pSaveTab;
                for (i = 0; i <= MAXTAB; i++)
                    if (pTab[i])
                        pTab[i]->UpdateMoveTab( nOldPos, nNewPos, i, *pProgress );
                delete pProgress;   // freimachen fuer evtl. andere
                for (i = 0; i <= MAXTAB; i++)
                    if (pTab[i])
                        pTab[i]->UpdateCompile();
                SetNoListening( FALSE );
                for (i = 0; i <= MAXTAB; i++)
                    if (pTab[i])
                        pTab[i]->StartAllListeners();
                // sheet names of references may not be valid until sheet is moved
                pChartListenerCollection->UpdateScheduledSeriesRanges();
                SetDirty();
                SetAutoCalc( bOldAutoCalc );

                if (pDrawLayer)
                    DrawMovePage( static_cast<sal_uInt16>(nOldPos), static_cast<sal_uInt16>(nNewPos) );

                bValid = TRUE;
            }
        }
    }
    return bValid;
}

BOOL ScDocument::CopyTab( SCTAB nOldPos, SCTAB nNewPos, const ScMarkData* pOnlyMarked )
{
    if (SC_TAB_APPEND == nNewPos ) nNewPos = nMaxTableNumber;
    String aName;
    GetName(nOldPos, aName);

    //  vorneweg testen, ob der Prefix als gueltig erkannt wird
    //  wenn nicht, nur doppelte vermeiden
    BOOL bPrefix = ValidTabName( aName );
    DBG_ASSERT(bPrefix, "ungueltiger Tabellenname");
    SCTAB nDummy;

    CreateValidTabName(aName);

    BOOL bValid;
    if (bPrefix)
        bValid = ( ValidNewTabName(aName) && (nMaxTableNumber <= MAXTAB) );
    else
        bValid = ( !GetTable( aName, nDummy ) && (nMaxTableNumber <= MAXTAB) );

    BOOL bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( FALSE );   // Mehrfachberechnungen vermeiden
    if (bValid)
    {
        if (nNewPos == nMaxTableNumber)
        {
            pTab[nMaxTableNumber] = new ScTable(this, nMaxTableNumber, aName);
            ++nMaxTableNumber;
        }
        else
        {
            if (VALIDTAB(nNewPos) && (nNewPos < nMaxTableNumber))
            {
                SetNoListening( TRUE );

                ScRange aRange( 0,0,nNewPos, MAXCOL,MAXROW,MAXTAB );
                xColNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,1 );
                xRowNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,1 );
                pRangeName->UpdateTabRef(nNewPos, 1);
                pDBCollection->UpdateReference(
                                    URM_INSDEL, 0,0,nNewPos, MAXCOL,MAXROW,MAXTAB, 0,0,1 );
                if (pDPCollection)
                    pDPCollection->UpdateReference( URM_INSDEL, aRange, 0,0,1 );
                if (pDetOpList)
                    pDetOpList->UpdateReference( this, URM_INSDEL, aRange, 0,0,1 );
                UpdateChartRef( URM_INSDEL, 0,0,nNewPos, MAXCOL,MAXROW,MAXTAB, 0,0,1 );
                UpdateRefAreaLinks( URM_INSDEL, aRange, 0,0,1 );
                if ( pUnoBroadcaster )
                    pUnoBroadcaster->Broadcast( ScUpdateRefHint( URM_INSDEL, aRange, 0,0,1 ) );

                SCTAB i;
                for (i = 0; i <= MAXTAB; i++)
                    if (pTab[i] && i != nOldPos)
                        pTab[i]->UpdateInsertTab(nNewPos);
                for (i = nMaxTableNumber; i > nNewPos; i--)
                    pTab[i] = pTab[i - 1];
                if (nNewPos <= nOldPos)
                    nOldPos++;
                pTab[nNewPos] = new ScTable(this, nNewPos, aName);
                ++nMaxTableNumber;
                bValid = TRUE;
                for (i = 0; i <= MAXTAB; i++)
                    if (pTab[i] && i != nOldPos && i != nNewPos)
                        pTab[i]->UpdateCompile();
                SetNoListening( FALSE );
                for (i = 0; i <= MAXTAB; i++)
                    if (pTab[i] && i != nOldPos && i != nNewPos)
                        pTab[i]->StartAllListeners();

                //  update conditional formats after table is inserted
                if ( pCondFormList )
                    pCondFormList->UpdateReference( URM_INSDEL, aRange, 0,0,1 );
                if ( pValidationList )
                    pValidationList->UpdateReference( URM_INSDEL, aRange, 0,0,1 );
                // sheet names of references may not be valid until sheet is copied
                pChartListenerCollection->UpdateScheduledSeriesRanges();
            }
            else
                bValid = FALSE;
        }
    }
    if (bValid)
    {
        SetNoListening( TRUE );     // noch nicht bei CopyToTable/Insert
        pTab[nOldPos]->CopyToTable(0, 0, MAXCOL, MAXROW, IDF_ALL, (pOnlyMarked != NULL),
                                        pTab[nNewPos], pOnlyMarked );
        pTab[nNewPos]->SetTabBgColor(pTab[nOldPos]->GetTabBgColor());

        SCsTAB nDz;
        nDz = ((short)nNewPos) - (short)nOldPos;
        pTab[nNewPos]->UpdateReference(URM_COPY, 0, 0, nNewPos , MAXCOL, MAXROW,
                                        nNewPos, 0, 0, nDz, NULL);

        pTab[nNewPos]->UpdateInsertTabAbs(nNewPos); // alle abs. um eins hoch!!
        pTab[nOldPos]->UpdateInsertTab(nNewPos);

        pTab[nOldPos]->UpdateCompile();
        pTab[nNewPos]->UpdateCompile( TRUE );   //  maybe already compiled in Clone, but used names need recompilation
        SetNoListening( FALSE );
        pTab[nOldPos]->StartAllListeners();
        pTab[nNewPos]->StartAllListeners();
        SetDirty();
        SetAutoCalc( bOldAutoCalc );

        if (pDrawLayer)
            DrawCopyPage( static_cast<sal_uInt16>(nOldPos), static_cast<sal_uInt16>(nNewPos) );

        pTab[nNewPos]->SetPageStyle( pTab[nOldPos]->GetPageStyle() );
        pTab[nNewPos]->SetPendingRowHeights( pTab[nOldPos]->IsPendingRowHeights() );
    }
    else
        SetAutoCalc( bOldAutoCalc );
    return bValid;
}

void VBA_InsertModule( ScDocument& rDoc, SCTAB nTab, String& sModuleName, String& sModuleSource );

ULONG ScDocument::TransferTab( ScDocument* pSrcDoc, SCTAB nSrcPos,
                                SCTAB nDestPos, BOOL bInsertNew,
                                BOOL bResultsOnly )
{
    ULONG nRetVal = 1;                      // 0 => Fehler 1 = ok
                                            // 2 => RefBox, 3 => NameBox
                                            // 4 => beides
    BOOL bValid = TRUE;
    if (bInsertNew)             // neu einfuegen
    {
        String aName;
        pSrcDoc->GetName(nSrcPos, aName);
        CreateValidTabName(aName);
        bValid = InsertTab(nDestPos, aName);
    }
    else                        // bestehende Tabelle ersetzen
    {
        if (VALIDTAB(nDestPos) && pTab[nDestPos])
        {
            pTab[nDestPos]->DeleteArea( 0,0, MAXCOL,MAXROW, IDF_ALL );
        }
        else
            bValid = FALSE;
    }

    if (bValid)
    {
        BOOL bOldAutoCalcSrc = FALSE;
        BOOL bOldAutoCalc = GetAutoCalc();
        SetAutoCalc( FALSE );   // Mehrfachberechnungen vermeiden
        SetNoListening( TRUE );
        if ( bResultsOnly )
        {
            bOldAutoCalcSrc = pSrcDoc->GetAutoCalc();
            pSrcDoc->SetAutoCalc( TRUE );   // falls was berechnet werden muss
        }

        {
            NumFmtMergeHandler aNumFmtMergeHdl(this, pSrcDoc);

            nDestPos = Min(nDestPos, (SCTAB)(GetTableCount() - 1));
            {   // scope for bulk broadcast
                ScBulkBroadcast aBulkBroadcast( pBASM);
                pSrcDoc->pTab[nSrcPos]->CopyToTable(0, 0, MAXCOL, MAXROW,
                        ( bResultsOnly ? IDF_ALL & ~IDF_FORMULA : IDF_ALL),
                        FALSE, pTab[nDestPos] );
            }
        }
        pTab[nDestPos]->SetTabNo(nDestPos);

        if ( !bResultsOnly )
        {
#if NEW_RANGE_NAME
#else
            BOOL bNamesLost = FALSE;
            size_t nSrcRangeNames = pSrcDoc->pRangeName->size();
            // array containing range names which might need update of indices
            ScRangeData** pSrcRangeNames = nSrcRangeNames ? new ScRangeData* [nSrcRangeNames] : NULL;
            // the index mapping thereof
            ScRangeData::IndexMap aSrcRangeMap;
            BOOL bRangeNameReplace = FALSE;

            // find named ranges that are used in the source sheet
            std::set<USHORT> aUsedNames;
            pSrcDoc->pTab[nSrcPos]->FindRangeNamesInUse( 0, 0, MAXCOL, MAXROW, aUsedNames );

            for (size_t i = 0; i < nSrcRangeNames; i++)     //! DB-Bereiche Pivot-Bereiche auch !!!
            {
                ScRangeData* pSrcData = (*pSrcDoc->pRangeName)[i];
                USHORT nOldIndex = pSrcData->GetIndex();
                bool bInUse = ( aUsedNames.find(nOldIndex) != aUsedNames.end() );
                if (bInUse)
                {
                    const ScRangeData* pExistingData = pRangeName->findByName(pSrcData->GetName());
                    if (pExistingData)
                    {
                        // the name exists already in the destination document
                        // -> use the existing name, but show a warning
                        // (when refreshing links, the existing name is used and the warning not shown)

                        USHORT nExistingIndex = pExistingData->GetIndex();

                        pSrcRangeNames[i] = NULL;       // don't modify the named range
                        aSrcRangeMap.insert(
                            ScRangeData::IndexMap::value_type(nOldIndex, nExistingIndex));
                        bRangeNameReplace = TRUE;
                        bNamesLost = TRUE;
                    }
                    else
                    {
                        ScRangeData* pData = new ScRangeData( *pSrcData );
                        pData->SetDocument(this);
                        if ( pRangeName->FindIndex( pData->GetIndex() ) )
                            pData->SetIndex(0);     // need new index, done in Insert
                        if (!pRangeName->Insert(pData))
                        {
                            OSL_FAIL("can't insert name");     // shouldn't happen
                            delete pData;
                        }
                        else
                        {
                            pData->TransferTabRef( nSrcPos, nDestPos );
                            pSrcRangeNames[i] = pData;
                            USHORT nNewIndex = pData->GetIndex();
                            aSrcRangeMap.insert(
                                ScRangeData::IndexMap::value_type(nOldIndex, nNewIndex));
                            if ( !bRangeNameReplace )
                                bRangeNameReplace = ( nOldIndex != nNewIndex );
                        }
                    }
                }
                else
                {
                    pSrcRangeNames[i] = NULL;
                }
            }
            if ( bRangeNameReplace )
            {
                // first update all inserted named formulas if they contain other
                // range names and used indices changed
                for (USHORT i = 0; i < nSrcRangeNames; i++)     //! DB-Bereiche Pivot-Bereiche auch
                {
                    if ( pSrcRangeNames[i] )
                        pSrcRangeNames[i]->ReplaceRangeNamesInUse( aSrcRangeMap );
                }
                // then update the formulas, they might need the just updated range names
                pTab[nDestPos]->ReplaceRangeNamesInUse( 0, 0, MAXCOL, MAXROW, aSrcRangeMap );
            }
            if ( pSrcRangeNames )
                delete [] pSrcRangeNames;

            SCsTAB nDz = ((SCsTAB)nDestPos) - (SCsTAB)nSrcPos;
            pTab[nDestPos]->UpdateReference(URM_COPY, 0, 0, nDestPos,
                                                     MAXCOL, MAXROW, nDestPos,
                                                     0, 0, nDz, NULL);
            // Test for outside absolute references for info box
            BOOL bIsAbsRef = pSrcDoc->pTab[nSrcPos]->TestTabRefAbs(nSrcPos);
            // Readjust self-contained absolute references to this sheet
            pTab[nDestPos]->TestTabRefAbs(nSrcPos);
            if (bIsAbsRef)
            {
                nRetVal += 1;
                    // InfoBox AbsoluteRefs sind moeglicherweise nicht mehr korrekt!!
            }
            if (bNamesLost)
            {
                nRetVal += 2;
                // message: duplicate names
            }
            pTab[nDestPos]->CompileAll();
#endif
        }

        SetNoListening( FALSE );
        if ( !bResultsOnly )
            pTab[nDestPos]->StartAllListeners();
        SetDirty( ScRange( 0, 0, nDestPos, MAXCOL, MAXROW, nDestPos));

        if ( bResultsOnly )
            pSrcDoc->SetAutoCalc( bOldAutoCalcSrc );
        SetAutoCalc( bOldAutoCalc );

        //  Drawing kopieren

        if (bInsertNew)
            TransferDrawPage( pSrcDoc, nSrcPos, nDestPos );

        pTab[nDestPos]->SetPendingRowHeights( pSrcDoc->pTab[nSrcPos]->IsPendingRowHeights() );
    }
    if (!bValid)
        nRetVal = 0;
    BOOL bVbaEnabled = IsInVBAMode();

    if ( bVbaEnabled  )
    {
        SfxObjectShell* pSrcShell = pSrcDoc ? pSrcDoc->GetDocumentShell() : NULL;
        if ( pSrcShell )
        {
            StarBASIC* pStarBASIC = pSrcShell ? pSrcShell->GetBasic() : NULL;
            String aLibName( RTL_CONSTASCII_USTRINGPARAM( "Standard" ) );
            if ( pSrcShell && pSrcShell->GetBasicManager()->GetName().Len() > 0 )
            {
                aLibName = pSrcShell->GetBasicManager()->GetName();
                pStarBASIC = pSrcShell->GetBasicManager()->GetLib( aLibName );
            }

            String sCodeName;
            String sSource;
            uno::Reference< script::XLibraryContainer > xLibContainer = pSrcShell->GetBasicContainer();
            uno::Reference< container::XNameContainer > xLib;
            if( xLibContainer.is() )
            {
                uno::Any aLibAny = xLibContainer->getByName( aLibName );
                aLibAny >>= xLib;
            }

            if( xLib.is() )
            {
                String sSrcCodeName;
                pSrcDoc->GetCodeName( nSrcPos, sSrcCodeName );
                rtl::OUString sRTLSource;
                xLib->getByName( sSrcCodeName ) >>= sRTLSource;
                sSource = sRTLSource;
            }
            VBA_InsertModule( *this, nDestPos, sCodeName, sSource );
        }
    }

    return nRetVal;
}

//  ----------------------------------------------------------------------------

void ScDocument::SetError( SCCOL nCol, SCROW nRow, SCTAB nTab, const USHORT nError)
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->SetError( nCol, nRow, nError );
}

namespace {

bool eraseUnusedSharedName(ScRangeName* pRangeName, ScTable* pTab[], USHORT nLevel)
{
    ScRangeName::const_iterator itr = pRangeName->begin(), itrEnd = pRangeName->end();
    for (; itr != itrEnd; ++itr)
    {
        if (!itr->HasType(RT_SHARED))
            continue;

        String aName;
        itr->GetName(aName);
        aName.Erase(0, 6);                      // !!! vgl. Table4, FillFormula !!
        USHORT nInd = static_cast<USHORT>(aName.ToInt32());
        if (nInd > nLevel)
            continue;

        USHORT nIndex = itr->GetIndex();

        bool bInUse = false;
        for (SCTAB j = 0; !bInUse && (j <= MAXTAB); ++j)
        {
            if (pTab[j])
                bInUse = pTab[j]->IsRangeNameInUse(0, 0, MAXCOL-1, MAXROW-1, nIndex);
        }
        if (!bInUse)
        {
            pRangeName->erase(itr);
            return true;
        }
    }
    return false;
}

}

void ScDocument::EraseNonUsedSharedNames(USHORT nLevel)
{
    while (eraseUnusedSharedName(pRangeName, pTab, nLevel))
        ;
}

//  ----------------------------------------------------------------------------

void ScDocument::SetConsolidateDlgData( const ScConsolidateParam* pData )
{
    delete pConsolidateDlgData;

    if ( pData )
        pConsolidateDlgData = new ScConsolidateParam( *pData );
    else
        pConsolidateDlgData = NULL;
}

void ScDocument::SetChangeViewSettings(const ScChangeViewSettings& rNew)
{
    if (pChangeViewSettings==NULL)
        pChangeViewSettings = new ScChangeViewSettings;

    DBG_ASSERT( pChangeViewSettings, "Oops. No ChangeViewSettings :-( by!" );

    *pChangeViewSettings=rNew;
}

//  ----------------------------------------------------------------------------

ScFieldEditEngine* ScDocument::CreateFieldEditEngine()
{
    ScFieldEditEngine* pNewEditEngine = NULL;
    if (!pCacheFieldEditEngine)
    {
        pNewEditEngine = new ScFieldEditEngine( GetEnginePool(),
            GetEditPool(), FALSE );
    }
    else
    {
        if ( !bImportingXML )
        {
            // #i66209# previous use might not have restored update mode,
            // ensure same state as for a new EditEngine (UpdateMode = TRUE)
            if ( !pCacheFieldEditEngine->GetUpdateMode() )
                pCacheFieldEditEngine->SetUpdateMode(TRUE);
        }

        pNewEditEngine = pCacheFieldEditEngine;
        pCacheFieldEditEngine = NULL;
    }
    return pNewEditEngine;
}

void ScDocument::DisposeFieldEditEngine(ScFieldEditEngine*& rpEditEngine)
{
    if (!pCacheFieldEditEngine && rpEditEngine)
    {
        pCacheFieldEditEngine = rpEditEngine;
        pCacheFieldEditEngine->Clear();
    }
    else
        delete rpEditEngine;
    rpEditEngine = NULL;
}

//  ----------------------------------------------------------------------------

ScRecursionHelper* ScDocument::CreateRecursionHelperInstance()
{
    return new ScRecursionHelper;
}

//  ----------------------------------------------------------------------------

ScLookupCache & ScDocument::GetLookupCache( const ScRange & rRange )
{
    ScLookupCache* pCache = 0;
    if (!pLookupCacheMapImpl)
        pLookupCacheMapImpl = new ScLookupCacheMapImpl;
    ScLookupCacheMap::iterator it( pLookupCacheMapImpl->aCacheMap.find( rRange));
    if (it == pLookupCacheMapImpl->aCacheMap.end())
    {
        pCache = new ScLookupCache( this, rRange);
        AddLookupCache( *pCache);
    }
    else
        pCache = (*it).second;
    return *pCache;
}

void ScDocument::AddLookupCache( ScLookupCache & rCache )
{
    if (!pLookupCacheMapImpl->aCacheMap.insert( ::std::pair< const ScRange,
                ScLookupCache*>( rCache.getRange(), &rCache)).second)
    {
        DBG_ERRORFILE( "ScDocument::AddLookupCache: couldn't add to hash map");
    }
    else
        StartListeningArea( rCache.getRange(), &rCache);
}

void ScDocument::RemoveLookupCache( ScLookupCache & rCache )
{
    ScLookupCacheMap::iterator it( pLookupCacheMapImpl->aCacheMap.find(
                rCache.getRange()));
    if (it == pLookupCacheMapImpl->aCacheMap.end())
    {
        DBG_ERRORFILE( "ScDocument::RemoveLookupCache: range not found in hash map");
    }
    else
    {
        ScLookupCache* pCache = (*it).second;
        pLookupCacheMapImpl->aCacheMap.erase( it);
        EndListeningArea( pCache->getRange(), &rCache);
    }
}

void ScDocument::ClearLookupCaches()
{
    if( pLookupCacheMapImpl )
        pLookupCacheMapImpl->clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
