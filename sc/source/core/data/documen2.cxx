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

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <editeng/editeng.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <svx/xtable.hxx>
#include <sfx2/linkmgr.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdobj.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/printer.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <vcl/virdev.hxx>
#include <comphelper/processfactory.hxx>
#include <svl/PasswordHelper.hxx>
#include <tools/tenccvt.hxx>
#include <tools/urlobj.hxx>
#include <rtl/crc.h>
#include <basic/basmgr.hxx>

#include "document.hxx"
#include "table.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "rangenam.hxx"
#include "dbdata.hxx"
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
#include "colorscale.hxx"
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
#include "formulacell.hxx"
#include "clipcontext.hxx"
#include "refupdatecontext.hxx"

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
        mpUndoManager( NULL ),
        pEditEngine( NULL ),
        pNoteEngine( NULL ),
        pShell( pDocShell ),
        pPrinter( NULL ),
        pVirtualDevice_100th_mm( NULL ),
        pDrawLayer( NULL ),
        pValidationList( NULL ),
        pFormatExchangeList( NULL ),
        pRangeName(NULL),
        pDPCollection( NULL ),
        pLinkManager( NULL ),
        pFormulaTree( NULL ),
        pEOFormulaTree( NULL ),
        pFormulaTrack( NULL ),
        pEOFormulaTrack( NULL ),
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
        pPreviewFont( NULL ),
        pPreviewSelection( NULL ),
        nUnoObjectId( 0 ),
        nRangeOverflowType( 0 ),
        aCurTextWidthCalcPos(MAXCOL,0,0),
        aTableOpList( 0 ),
        nFormulaCodeInTree(0),
        nXMLImportedFormulaCount( 0 ),
        nInterpretLevel(0),
        nMacroInterpretLevel(0),
        nInterpreterTableOpLevel(0),
        nSrcVer( SC_CURRENT_VERSION ),
        nSrcMaxRow( MAXROW ),
        nFormulaTrackCount(0),
        bHardRecalcState(false),
        nVisibleTab( 0 ),
        eLinkMode(LM_UNKNOWN),
        bAutoCalc( eMode == SCDOCMODE_DOCUMENT ),
        bAutoCalcShellDisabled( false ),
        bForcedFormulaPending( false ),
        bCalculatingFormulaTree( false ),
        bIsClip( eMode == SCDOCMODE_CLIP ),
        bIsUndo( eMode == SCDOCMODE_UNDO ),
        bIsVisible( false ),
        bIsEmbedded( false ),
        bInsertingFromOtherDoc( false ),
        bLoadingMedium( false ),
        bImportingXML( false ),
        bXMLFromWrapper( false ),
        bCalcingAfterLoad( false ),
        bNoListening( false ),
        mbIdleEnabled(true),
        bInLinkUpdate( false ),
        bChartListenerCollectionNeedsUpdate( false ),
        bHasForcedFormulas( false ),
        bInDtorClear( false ),
        bExpandRefs( false ),
        bDetectiveDirty( false ),
        nMacroCallMode( SC_MACROCALL_ALLOWED ),
        bHasMacroFunc( false ),
        nVisSpellState( 0 ),
        nAsianCompression(SC_ASIANCOMPRESSION_INVALID),
        nAsianKerning(SC_ASIANKERNING_INVALID),
        bPastingDrawFromOtherDoc( false ),
        nInDdeLinkUpdate( 0 ),
        bInUnoBroadcast( false ),
        bInUnoListenerCall( false ),
        eGrammar( formula::FormulaGrammar::GRAM_NATIVE ),
        bStyleSheetUsageInvalid( true ),
        mbUndoEnabled( true ),
        mbAdjustHeightEnabled( true ),
        mbExecuteLinkEnabled( true ),
        mbChangeReadOnlyEnabled( false ),
        mbStreamValidLocked( false ),
        mbUserInteractionEnabled(true),
        mnNamedRangesLockCount(0),
        mbUseEmbedFonts(false)
{
    SetStorageGrammar( formula::FormulaGrammar::GRAM_STORAGE_DEFAULT);

    eSrcSet = osl_getThreadTextEncoding();

    if ( eMode == SCDOCMODE_DOCUMENT )
    {
        if ( pDocShell )
            pLinkManager = new sfx2::LinkManager( pDocShell );

        xPoolHelper = new ScPoolHelper( this );

        pBASM = new ScBroadcastAreaSlotMachine( this );
        pChartListenerCollection = new ScChartListenerCollection( this );
        pRefreshTimerControl = new ScRefreshTimerControl;
    }
    else
    {
        pBASM       = NULL;
        pChartListenerCollection = NULL;
        pRefreshTimerControl = NULL;
    }
    pDBCollection = new ScDBCollection(this);
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
    OSL_PRECOND(
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


void ScDocument::SetDocVisible( bool bSet )
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
    OSL_ENSURE( pTrack->GetDocument() == this, "SetChangeTrack: different documents" );
    if ( !pTrack || pTrack == pChangeTrack || pTrack->GetDocument() != this )
        return ;
    EndChangeTracking();
    pChangeTrack = pTrack;
}


IMPL_LINK_NOARG(ScDocument, TrackTimeHdl)
{
    if ( ScDdeLink::IsInUpdate() )      // nicht verschachteln
    {
        aTrackTimer.Start();            // spaeter nochmal versuchen
    }
    else if (pShell)                    // ausfuehren
    {
        TrackFormulas();
        pShell->Broadcast( SfxSimpleHint( FID_DATACHANGED ) );

            //  modified...

        if (!pShell->IsModified())
        {
            pShell->SetModified( true );
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
    OSL_PRECOND( !bInLinkUpdate, "bInLinkUpdate in dtor" );

    bInDtorClear = true;

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
        ::sfx2::SvLinkSources aTemp(pLinkManager->GetServers());
        for( ::sfx2::SvLinkSources::const_iterator it = aTemp.begin(); it != aTemp.end(); ++it )
            (*it)->Closed();

        if ( pLinkManager->GetLinks().size() )
            pLinkManager->Remove( 0, pLinkManager->GetLinks().size() );
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

    delete pUnoBroadcaster;     // broadcasted nochmal SFX_HINT_DYING
    pUnoBroadcaster = NULL;

    delete pUnoRefUndoList;
    delete pUnoListenerCalls;

    Clear( true );              // true = from destructor (needed for SdrModel::ClearModel)

    if (pValidationList)
    {
        for( ScValidationDataList::iterator it = pValidationList->begin(); it != pValidationList->end(); ++it )
            delete *it;
        pValidationList->clear();
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
    delete pChangeViewSettings;         // und weg damit
    delete pVirtualDevice_100th_mm;

    delete pDPCollection;

    // delete the EditEngine before destroying the xPoolHelper
    delete pCacheFieldEditEngine;

    if ( xPoolHelper.is() && !bIsClip )
        xPoolHelper->SourceDocumentGone();
    xPoolHelper.clear();

    delete pScriptTypeData;
    delete pRecursionHelper;

    OSL_POSTCOND( !pAutoNameCache, "AutoNameCache still set in dtor" );
}

void ScDocument::InitClipPtrs( ScDocument* pSourceDoc )
{
    OSL_ENSURE(bIsClip, "InitClipPtrs and not bIsClip");

    if (pValidationList)
    {
        for(ScValidationDataList::iterator it = pValidationList->begin(); it != pValidationList->end(); ++it )
            delete *it;
        pValidationList->clear();
        DELETEZ(pValidationList);
    }

    Clear();

    xPoolHelper = pSourceDoc->xPoolHelper;

    //  bedingte Formate / Gueltigkeiten
    //! Vorlagen kopieren?
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
        pEditEngine = new ScFieldEditEngine(this, GetEnginePool(), GetEditPool());
        pEditEngine->SetUpdateMode( false );
        pEditEngine->EnableUndo( false );
        pEditEngine->SetRefMapMode( MAP_100TH_MM );
        ApplyAsianEditSettings( *pEditEngine );
    }
    return *pEditEngine;
}

ScNoteEditEngine& ScDocument::GetNoteEngine()
{
    if ( !pNoteEngine )
    {
        pNoteEngine = new ScNoteEditEngine( GetEnginePool(), GetEditPool() );
        pNoteEngine->SetUpdateMode( false );
        pNoteEngine->EnableUndo( false );
        pNoteEngine->SetRefMapMode( MAP_100TH_MM );
        ApplyAsianEditSettings( *pNoteEngine );
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

        for (SCTAB i = 0; i < static_cast<SCTAB>(pSourceDoc->maTabs.size()); i++)
            if (pSourceDoc->maTabs[i])
                if (!pMarks || pMarks->GetTableSelect(i))
                {
                    OUString aString;
                    pSourceDoc->maTabs[i]->GetName(aString);
                    if ( i < static_cast<SCTAB>(maTabs.size()) )
                    {
                        maTabs[i] = new ScTable(this, i, aString);

                    }
                    else
                    {
                        if( i > static_cast<SCTAB>(maTabs.size()) )
                        {
                            maTabs.resize(i, NULL );
                        }
                        maTabs.push_back(new ScTable(this, i, aString));
                    }
                    maTabs[i]->SetLayoutRTL( pSourceDoc->maTabs[i]->IsLayoutRTL() );
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
        if (nTab >= static_cast<SCTAB>(maTabs.size()))
        {
            maTabs.resize(nTab+1, NULL );
        }
        maTabs[nTab] = new ScTable(this, nTab,
                            OUString("baeh"));
        if (nTab < static_cast<SCTAB>(pSourceDoc->maTabs.size()) && pSourceDoc->maTabs[nTab])
            maTabs[nTab]->SetLayoutRTL( pSourceDoc->maTabs[nTab]->IsLayoutRTL() );
    }
    else
    {
        OSL_FAIL("ResetClip");
    }
}

void ScDocument::EnsureTable( SCTAB nTab )
{
    bool bExtras = !bIsUndo;        // Spaltenbreiten, Zeilenhoehen, Flags
    if (static_cast<size_t>(nTab) >= maTabs.size())
        maTabs.resize(nTab+1, NULL);

    if (!maTabs[nTab])
        maTabs[nTab] = new ScTable(this, nTab, "temp", bExtras, bExtras);
}

ScRefCellValue ScDocument::GetRefCellValue( const ScAddress& rPos )
{
    if (!TableExists(rPos.Tab()))
        return ScRefCellValue(); // empty

    return maTabs[rPos.Tab()]->GetRefCellValue(rPos.Col(), rPos.Row());
}

bool ScDocument::GetPrintArea( SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow,
                                bool bNotes ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
    {
        bool bAny = maTabs[nTab]->GetPrintArea( rEndCol, rEndRow, bNotes );
        if (pDrawLayer)
        {
            ScRange aDrawRange(0,0,nTab, MAXCOL,MAXROW,nTab);
            if (DrawGetPrintArea( aDrawRange, true, true ))
            {
                if (aDrawRange.aEnd.Col()>rEndCol) rEndCol=aDrawRange.aEnd.Col();
                if (aDrawRange.aEnd.Row()>rEndRow) rEndRow=aDrawRange.aEnd.Row();
                bAny = true;
            }
        }
        return bAny;
    }

    rEndCol = 0;
    rEndRow = 0;
    return false;
}

bool ScDocument::GetPrintAreaHor( SCTAB nTab, SCROW nStartRow, SCROW nEndRow,
                                        SCCOL& rEndCol, bool bNotes ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
    {
        bool bAny = maTabs[nTab]->GetPrintAreaHor( nStartRow, nEndRow, rEndCol, bNotes );
        if (pDrawLayer)
        {
            ScRange aDrawRange(0,nStartRow,nTab, MAXCOL,nEndRow,nTab);
            if (DrawGetPrintArea( aDrawRange, true, false ))
            {
                if (aDrawRange.aEnd.Col()>rEndCol) rEndCol=aDrawRange.aEnd.Col();
                bAny = true;
            }
        }
        return bAny;
    }

    rEndCol = 0;
    return false;
}

bool ScDocument::GetPrintAreaVer( SCTAB nTab, SCCOL nStartCol, SCCOL nEndCol,
                                        SCROW& rEndRow, bool bNotes ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
    {
        bool bAny = maTabs[nTab]->GetPrintAreaVer( nStartCol, nEndCol, rEndRow, bNotes );
        if (pDrawLayer)
        {
            ScRange aDrawRange(nStartCol,0,nTab, nEndCol,MAXROW,nTab);
            if (DrawGetPrintArea( aDrawRange, false, true ))
            {
                if (aDrawRange.aEnd.Row()>rEndRow) rEndRow=aDrawRange.aEnd.Row();
                bAny = true;
            }
        }
        return bAny;
    }

    rEndRow = 0;
    return false;
}

bool ScDocument::GetDataStart( SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
    {
        bool bAny = maTabs[nTab]->GetDataStart( rStartCol, rStartRow );
        if (pDrawLayer)
        {
            ScRange aDrawRange(0,0,nTab, MAXCOL,MAXROW,nTab);
            if (DrawGetPrintArea( aDrawRange, true, true ))
            {
                if (aDrawRange.aStart.Col()<rStartCol) rStartCol=aDrawRange.aStart.Col();
                if (aDrawRange.aStart.Row()<rStartRow) rStartRow=aDrawRange.aStart.Row();
                bAny = true;
            }
        }
        return bAny;
    }

    rStartCol = 0;
    rStartRow = 0;
    return false;
}

bool ScDocument::MoveTab( SCTAB nOldPos, SCTAB nNewPos, ScProgress* pProgress )
{
    if (nOldPos == nNewPos)
        return false;

    SCTAB nTabCount = static_cast<SCTAB>(maTabs.size());
    if(nTabCount < 2)
        return false;

    bool bValid = false;
    if (ValidTab(nOldPos) && nOldPos < nTabCount )
    {
        if (maTabs[nOldPos])
        {
            bool bOldAutoCalc = GetAutoCalc();
            SetAutoCalc( false );   // Mehrfachberechnungen vermeiden
            SetNoListening( true );
            if (nNewPos == SC_TAB_APPEND || nNewPos >= nTabCount)
                nNewPos = nTabCount-1;

            //  Referenz-Updaterei
            //! mit UpdateReference zusammenfassen!

            SCsTAB nDz = ((SCsTAB)nNewPos) - (SCsTAB)nOldPos;
            ScRange aSourceRange( 0,0,nOldPos, MAXCOL,MAXROW,nOldPos );
            if (pRangeName)
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
            if ( pValidationList )
                pValidationList->UpdateMoveTab( nOldPos, nNewPos );
            if ( pUnoBroadcaster )
                pUnoBroadcaster->Broadcast( ScUpdateRefHint( URM_REORDER,
                            aSourceRange, 0,0,nDz ) );

            ScTable* pSaveTab = maTabs[nOldPos];
            maTabs.erase(maTabs.begin()+nOldPos);
            maTabs.insert(maTabs.begin()+nNewPos, pSaveTab);
            TableContainer::iterator it = maTabs.begin();
            for (SCTAB i = 0; i < nTabCount; i++)
                if (maTabs[i])
                    maTabs[i]->UpdateMoveTab( nOldPos, nNewPos, i, pProgress );
            it = maTabs.begin();
            for (; it != maTabs.end(); ++it)
                if (*it)
                    (*it)->UpdateCompile();
            SetNoListening( false );
            it = maTabs.begin();
            for (; it != maTabs.end(); ++it)
                if (*it)
                    (*it)->StartAllListeners();
            // sheet names of references may not be valid until sheet is moved
            pChartListenerCollection->UpdateScheduledSeriesRanges();
            SetDirty();
            SetAutoCalc( bOldAutoCalc );

            if (pDrawLayer)
                DrawMovePage( static_cast<sal_uInt16>(nOldPos), static_cast<sal_uInt16>(nNewPos) );

            bValid = true;
        }
    }
    return bValid;
}

bool ScDocument::CopyTab( SCTAB nOldPos, SCTAB nNewPos, const ScMarkData* pOnlyMarked )
{
    if (SC_TAB_APPEND == nNewPos  || nNewPos >= static_cast<SCTAB>(maTabs.size()))
        nNewPos = static_cast<SCTAB>(maTabs.size());
    OUString aName;
    GetName(nOldPos, aName);

    //  vorneweg testen, ob der Prefix als gueltig erkannt wird
    //  wenn nicht, nur doppelte vermeiden
    bool bPrefix = ValidTabName( aName );
    OSL_ENSURE(bPrefix, "invalid table name");
    SCTAB nDummy;

    CreateValidTabName(aName);

    bool bValid;
    if (bPrefix)
        bValid = ValidNewTabName(aName);
    else
        bValid = !GetTable( aName, nDummy );

    bool bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( false );   // Mehrfachberechnungen vermeiden
    if (bValid)
    {
        if (nNewPos >= static_cast<SCTAB>(maTabs.size()))
        {
            nNewPos = static_cast<SCTAB>(maTabs.size());
            maTabs.push_back(new ScTable(this, nNewPos, aName));
        }
        else
        {
            if (ValidTab(nNewPos) && (nNewPos < static_cast<SCTAB>(maTabs.size())))
            {
                SetNoListening( true );

                ScRange aRange( 0,0,nNewPos, MAXCOL,MAXROW,MAXTAB );
                xColNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,1 );
                xRowNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,1 );
                if (pRangeName)
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
                for (TableContainer::iterator it = maTabs.begin(); it != maTabs.end(); ++it)
                    if (*it && it != (maTabs.begin() + nOldPos))
                        (*it)->UpdateInsertTab(nNewPos);
                maTabs.push_back(NULL);
                for (i = static_cast<SCTAB>(maTabs.size())-1; i > nNewPos; i--)
                    maTabs[i] = maTabs[i - 1];
                if (nNewPos <= nOldPos)
                    nOldPos++;
                maTabs[nNewPos] = new ScTable(this, nNewPos, aName);
                bValid = true;
                for (TableContainer::iterator it = maTabs.begin(); it != maTabs.end(); ++it)
                    if (*it && it != maTabs.begin()+nOldPos && it != maTabs.begin() + nNewPos)
                        (*it)->UpdateCompile();
                SetNoListening( false );
                for (TableContainer::iterator it = maTabs.begin(); it != maTabs.end(); ++it)
                    if (*it && it != maTabs.begin()+nOldPos && it != maTabs.begin()+nNewPos)
                        (*it)->StartAllListeners();

                if ( pValidationList )
                    pValidationList->UpdateReference( URM_INSDEL, aRange, 0,0,1 );
                // sheet names of references may not be valid until sheet is copied
                pChartListenerCollection->UpdateScheduledSeriesRanges();
            }
            else
                bValid = false;
        }
    }
    if (bValid)
    {
        sc::CopyToDocContext aCxt(*this);

        SetNoListening( true );     // noch nicht bei CopyToTable/Insert
        maTabs[nOldPos]->CopyToTable(aCxt, 0, 0, MAXCOL, MAXROW, IDF_ALL, (pOnlyMarked != NULL),
                                        maTabs[nNewPos], pOnlyMarked );
        maTabs[nNewPos]->SetTabBgColor(maTabs[nOldPos]->GetTabBgColor());

        SCTAB nDz = nNewPos - nOldPos;
        sc::RefUpdateContext aRefCxt;
        aRefCxt.meMode = URM_COPY;
        aRefCxt.maRange = ScRange(0, 0, nNewPos, MAXCOL, MAXROW, nNewPos);
        aRefCxt.mnTabDelta = nDz;
        maTabs[nNewPos]->UpdateReference(aRefCxt, NULL);

        maTabs[nNewPos]->UpdateInsertTabAbs(nNewPos); // alle abs. um eins hoch!!
        maTabs[nOldPos]->UpdateInsertTab(nNewPos);

        maTabs[nOldPos]->UpdateCompile();
        maTabs[nNewPos]->UpdateCompile( true ); //  maybe already compiled in Clone, but used names need recompilation
        SetNoListening( false );
        maTabs[nOldPos]->StartAllListeners();
        maTabs[nNewPos]->StartAllListeners();

        ScConditionalFormatList* pNewList = new ScConditionalFormatList(*maTabs[nOldPos]->GetCondFormList());
        pNewList->UpdateReference(URM_COPY, ScRange( 0, 0, nNewPos , MAXCOL, MAXROW,
                                        nNewPos), 0, 0, nDz);
        maTabs[nNewPos]->SetCondFormList( pNewList );

        SetDirty();
        SetAutoCalc( bOldAutoCalc );

        if (pDrawLayer)
            DrawCopyPage( static_cast<sal_uInt16>(nOldPos), static_cast<sal_uInt16>(nNewPos) );

        if (pDPCollection)
            pDPCollection->CopyToTab(nOldPos, nNewPos);

        maTabs[nNewPos]->SetPageStyle( maTabs[nOldPos]->GetPageStyle() );
        maTabs[nNewPos]->SetPendingRowHeights( maTabs[nOldPos]->IsPendingRowHeights() );

        // Copy the custom print range if exists.
        maTabs[nNewPos]->CopyPrintRange(*maTabs[nOldPos]);

        // Copy the RTL settings
        maTabs[nNewPos]->SetLayoutRTL(maTabs[nOldPos]->IsLayoutRTL());
        maTabs[nNewPos]->SetLoadingRTL(maTabs[nOldPos]->IsLoadingRTL());
    }
    else
        SetAutoCalc( bOldAutoCalc );
    return bValid;
}

void VBA_InsertModule( ScDocument& rDoc, SCTAB nTab, const OUString& sModuleName, const OUString& sModuleSource );

sal_uLong ScDocument::TransferTab( ScDocument* pSrcDoc, SCTAB nSrcPos,
                                SCTAB nDestPos, bool bInsertNew,
                                bool bResultsOnly )
{
    sal_uLong nRetVal = 1;                      // 0 => Fehler 1 = ok
                                            // 3 => NameBox
                                            // 4 => beides

    if (pSrcDoc->pShell->GetMedium())
    {
        pSrcDoc->maFileURL = pSrcDoc->pShell->GetMedium()->GetURLObject().GetMainURL(INetURLObject::DECODE_TO_IURI);
        // for unsaved files use the title name and adjust during save of file
        if (pSrcDoc->maFileURL.isEmpty())
            pSrcDoc->maFileURL = pSrcDoc->pShell->GetName();
    }
    else
    {
        pSrcDoc->maFileURL = pSrcDoc->pShell->GetName();
    }

    bool bValid = true;
    if (bInsertNew)             // neu einfuegen
    {
        OUString aName;
        pSrcDoc->GetName(nSrcPos, aName);
        CreateValidTabName(aName);
        bValid = InsertTab(nDestPos, aName);

        // Copy the RTL settings
        maTabs[nDestPos]->SetLayoutRTL(pSrcDoc->maTabs[nSrcPos]->IsLayoutRTL());
        maTabs[nDestPos]->SetLoadingRTL(pSrcDoc->maTabs[nSrcPos]->IsLoadingRTL());
    }
    else                        // bestehende Tabelle ersetzen
    {
        if (ValidTab(nDestPos) && nDestPos < static_cast<SCTAB>(maTabs.size()) && maTabs[nDestPos])
        {
            maTabs[nDestPos]->DeleteArea( 0,0, MAXCOL,MAXROW, IDF_ALL );
        }
        else
            bValid = false;
    }

    if (bValid)
    {
        bool bOldAutoCalcSrc = false;
        bool bOldAutoCalc = GetAutoCalc();
        SetAutoCalc( false );   // Mehrfachberechnungen vermeiden
        SetNoListening( true );
        if ( bResultsOnly )
        {
            bOldAutoCalcSrc = pSrcDoc->GetAutoCalc();
            pSrcDoc->SetAutoCalc( true );   // falls was berechnet werden muss
        }

        {
            NumFmtMergeHandler aNumFmtMergeHdl(this, pSrcDoc);

            sc::CopyToDocContext aCxt(*this);
            nDestPos = std::min(nDestPos, (SCTAB)(GetTableCount() - 1));
            {   // scope for bulk broadcast
                ScBulkBroadcast aBulkBroadcast( pBASM);
                pSrcDoc->maTabs[nSrcPos]->CopyToTable(aCxt, 0, 0, MAXCOL, MAXROW,
                        ( bResultsOnly ? IDF_ALL & ~IDF_FORMULA : IDF_ALL),
                        false, maTabs[nDestPos] );
                maTabs[nDestPos]->CopyConditionalFormat(0, 0, MAXCOL, MAXROW,
                            0, 0, pSrcDoc->maTabs[nSrcPos]);
            }
        }
        maTabs[nDestPos]->SetTabNo(nDestPos);
        maTabs[nDestPos]->SetTabBgColor(pSrcDoc->maTabs[nSrcPos]->GetTabBgColor());

        if ( !bResultsOnly )
        {
            sc::RefUpdateContext aRefCxt;
            aRefCxt.meMode = URM_COPY;
            aRefCxt.maRange = ScRange(0, 0, nDestPos, MAXCOL, MAXROW, nDestPos);
            aRefCxt.mnTabDelta = nDestPos - nSrcPos;
            maTabs[nDestPos]->UpdateReference(aRefCxt, NULL);

            // Readjust self-contained absolute references to this sheet
            maTabs[nDestPos]->TestTabRefAbs(nSrcPos);
            maTabs[nDestPos]->CompileAll();
        }

        SetNoListening( false );
        if ( !bResultsOnly )
            maTabs[nDestPos]->StartAllListeners();
        SetDirty( ScRange( 0, 0, nDestPos, MAXCOL, MAXROW, nDestPos));

        if ( bResultsOnly )
            pSrcDoc->SetAutoCalc( bOldAutoCalcSrc );
        SetAutoCalc( bOldAutoCalc );

        //  Drawing kopieren

        if (bInsertNew)
            TransferDrawPage( pSrcDoc, nSrcPos, nDestPos );

        maTabs[nDestPos]->SetPendingRowHeights( pSrcDoc->maTabs[nSrcPos]->IsPendingRowHeights() );
    }
    if (!bValid)
        nRetVal = 0;
    bool bVbaEnabled = IsInVBAMode();

    if ( bVbaEnabled  )
    {
        SfxObjectShell* pSrcShell = pSrcDoc ? pSrcDoc->GetDocumentShell() : NULL;
        if ( pSrcShell )
        {
            OUString aLibName("Standard");
            const BasicManager *pBasicManager = pSrcShell->GetBasicManager();
            if (pBasicManager && !pBasicManager->GetName().isEmpty())
            {
                aLibName = pSrcShell->GetBasicManager()->GetName();
            }
            OUString sCodeName;
            OUString sSource;
            uno::Reference< script::XLibraryContainer > xLibContainer = pSrcShell->GetBasicContainer();
            uno::Reference< container::XNameContainer > xLib;
            if( xLibContainer.is() )
            {
                uno::Any aLibAny = xLibContainer->getByName(aLibName);
                aLibAny >>= xLib;
            }

            if( xLib.is() )
            {
                OUString sSrcCodeName;
                pSrcDoc->GetCodeName( nSrcPos, sSrcCodeName );
                OUString sRTLSource;
                xLib->getByName( sSrcCodeName ) >>= sRTLSource;
                sSource = sRTLSource;
            }
            VBA_InsertModule( *this, nDestPos, sCodeName, sSource );
        }
    }

    return nRetVal;
}

void ScDocument::SetError( SCCOL nCol, SCROW nRow, SCTAB nTab, const sal_uInt16 nError)
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
        if (maTabs[nTab])
            maTabs[nTab]->SetError( nCol, nRow, nError );
}

void ScDocument::SetFormula(
    const ScAddress& rPos, const ScTokenArray& rArray, formula::FormulaGrammar::Grammar eGram )
{
    if (!TableExists(rPos.Tab()))
        return;

    maTabs[rPos.Tab()]->SetFormula(rPos.Col(), rPos.Row(), rArray, eGram);
}

void ScDocument::SetFormula(
    const ScAddress& rPos, const OUString& rFormula, formula::FormulaGrammar::Grammar eGram )
{
    if (!TableExists(rPos.Tab()))
        return;

    maTabs[rPos.Tab()]->SetFormula(rPos.Col(), rPos.Row(), rFormula, eGram);
}

ScFormulaCell* ScDocument::SetFormulaCell( const ScAddress& rPos, ScFormulaCell* pCell )
{
    if (!TableExists(rPos.Tab()))
    {
        delete pCell;
        return NULL;
    }

    return maTabs[rPos.Tab()]->SetFormulaCell(rPos.Col(), rPos.Row(), pCell);
}

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

    OSL_ENSURE( pChangeViewSettings, "Oops. No ChangeViewSettings :-( by!" );

    *pChangeViewSettings=rNew;
}

//  ----------------------------------------------------------------------------

ScFieldEditEngine* ScDocument::CreateFieldEditEngine()
{
    ScFieldEditEngine* pNewEditEngine = NULL;
    if (!pCacheFieldEditEngine)
    {
        pNewEditEngine = new ScFieldEditEngine(
            this, GetEnginePool(), GetEditPool(), false);
    }
    else
    {
        if ( !bImportingXML )
        {
            // #i66209# previous use might not have restored update mode,
            // ensure same state as for a new EditEngine (UpdateMode = true)
            if ( !pCacheFieldEditEngine->GetUpdateMode() )
                pCacheFieldEditEngine->SetUpdateMode(true);
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
        OSL_FAIL( "ScDocument::AddLookupCache: couldn't add to hash map");
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
        OSL_FAIL( "ScDocument::RemoveLookupCache: range not found in hash map");
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

void ScDocument::SetPreviewFont( SfxItemSet* pFont )
{
    delete pPreviewFont;
    pPreviewFont = pFont;
}

const ScMarkData& ScDocument::GetPreviewSelection()
{
    if ( !pPreviewSelection )
        pPreviewSelection = new ScMarkData();

    return *pPreviewSelection;
}

void  ScDocument::SetPreviewSelection( ScMarkData& rSel )
{
    // yeuch, why do I have a pointer here ???? ( other problems
    // to fix right now though )
    if ( !pPreviewSelection )
        pPreviewSelection = new ScMarkData();
    *pPreviewSelection = rSel;
}

SfxItemSet* ScDocument::GetPreviewFont( SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    SfxItemSet* pRet = NULL;
    if ( pPreviewFont )
    {
        if ( GetPreviewSelection().IsCellMarked( nCol, nRow ) && GetPreviewSelection().GetFirstSelected() == nTab )
            pRet = pPreviewFont;
    }
    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
