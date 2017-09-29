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

#include <scitems.hxx>
#include <editeng/eeitem.hxx>

#include <editeng/editeng.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <osl/thread.h>
#include <svx/xtable.hxx>
#include <sfx2/linkmgr.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdobj.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/printer.hxx>
#include <svl/asiancfg.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <vcl/virdev.hxx>
#include <comphelper/processfactory.hxx>
#include <svl/PasswordHelper.hxx>
#include <svl/sharedstringpool.hxx>
#include <tools/tenccvt.hxx>
#include <tools/urlobj.hxx>
#include <rtl/crc.h>
#include <basic/basmgr.hxx>

#include <document.hxx>
#include <table.hxx>
#include <attrib.hxx>
#include <patattr.hxx>
#include <rangenam.hxx>
#include <dbdata.hxx>
#include <pivot.hxx>
#include <docpool.hxx>
#include <stlpool.hxx>
#include <stlsheet.hxx>
#include <globstr.hrc>
#include <chartarr.hxx>
#include <chartlock.hxx>
#include <rechead.hxx>
#include <global.hxx>
#include <brdcst.hxx>
#include <bcaslot.hxx>
#include <adiasync.hxx>
#include <addinlis.hxx>
#include <chartlis.hxx>
#include <markdata.hxx>
#include <conditio.hxx>
#include <colorscale.hxx>
#include <validat.hxx>
#include <progress.hxx>
#include <detdata.hxx>
#include <sc.hrc>
#include <ddelink.hxx>
#include <chgtrack.hxx>
#include <chgviset.hxx>
#include <editutil.hxx>
#include <hints.hxx>
#include <dpobject.hxx>
#include <scrdata.hxx>
#include <poolhelp.hxx>
#include <unoreflist.hxx>
#include <listenercalls.hxx>
#include <recursionhelper.hxx>
#include <lookupcache.hxx>
#include <externalrefmgr.hxx>
#include <appoptio.hxx>
#include <scmod.hxx>
#include <viewdata.hxx>
#include <viewutil.hxx>
#include <tabprotection.hxx>
#include <formulaparserpool.hxx>
#include <clipparam.hxx>
#include <macromgr.hxx>
#include <formulacell.hxx>
#include <clipcontext.hxx>
#include <refupdatecontext.hxx>
#include <refreshtimerprotector.hxx>
#include <scopetools.hxx>
#include <formulagroup.hxx>
#include <documentlinkmgr.hxx>
#include <interpre.hxx>
#include <tokenstringcontext.hxx>
#include <docsh.hxx>
#include <clipoptions.hxx>
#include <listenercontext.hxx>
#include <datamapper.hxx>

using namespace com::sun::star;

// pImpl because including lookupcache.hxx in document.hxx isn't wanted, and
// dtor plus helpers are convenient.
struct ScLookupCacheMapImpl
{
    std::unordered_map< ScRange, ScLookupCache*, ScLookupCache::Hash > aCacheMap;
    ~ScLookupCacheMapImpl()
    {
        freeCaches();
    }
    void clear()
    {
        freeCaches();
        // free mapping
        std::unordered_map< ScRange, ScLookupCache*, ScLookupCache::Hash > aTmp;
        aCacheMap.swap( aTmp);
    }
private:
    void freeCaches()
    {
        for (auto it( aCacheMap.begin()); it != aCacheMap.end(); ++it)
            delete (*it).second;
    }
};

ScDocument::ScDocument( ScDocumentMode eMode, SfxObjectShell* pDocShell ) :
        mpCellStringPool(new svl::SharedStringPool(ScGlobal::pCharClass)),
        mpFormulaGroupCxt(nullptr),
        mpDocLinkMgr(new sc::DocumentLinkManager(pDocShell)),
        maCalcConfig( ScInterpreter::GetGlobalConfig()),
        mpUndoManager( nullptr ),
        mpEditEngine( nullptr ),
        mpNoteEngine( nullptr ),
        mpShell( pDocShell ),
        mpPrinter( nullptr ),
        mpVirtualDevice_100th_mm( nullptr ),
        mpDrawLayer( nullptr ),
        pValidationList( nullptr ),
        pFormatExchangeList( nullptr ),
        pRangeName(nullptr),
        pDPCollection( nullptr ),
        pFormulaTree( nullptr ),
        pEOFormulaTree( nullptr ),
        pFormulaTrack( nullptr ),
        pEOFormulaTrack( nullptr ),
        pClipData( nullptr ),
        pDetOpList(nullptr),
        pChangeTrack( nullptr ),
        pUnoBroadcaster( nullptr ),
        pUnoListenerCalls( nullptr ),
        pUnoRefUndoList( nullptr ),
        pChangeViewSettings( nullptr ),
        pScriptTypeData( nullptr ),
        mpAnonymousDBData( nullptr ),
        pCacheFieldEditEngine( nullptr ),
        pViewOptions( nullptr ),
        pDocOptions( nullptr ),
        pExtDocOptions( nullptr ),
        pConsolidateDlgData( nullptr ),
        pAutoNameCache( nullptr ),
        pPreviewFont( nullptr ),
        pPreviewCellStyle( nullptr ),
        nUnoObjectId( 0 ),
        nRangeOverflowType( 0 ),
        aCurTextWidthCalcPos(MAXCOL,0,0),
        mbThreadedGroupCalcInProgress( false ),
        nFormulaCodeInTree(0),
        nXMLImportedFormulaCount( 0 ),
        nSrcVer( SC_CURRENT_VERSION ),
        nFormulaTrackCount(0),
        eHardRecalcState(HardRecalcState::OFF),
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
        bCalcingAfterLoad( false ),
        bNoListening( false ),
        mbIdleEnabled(true),
        bInLinkUpdate( false ),
        bChartListenerCollectionNeedsUpdate( false ),
        bHasForcedFormulas( false ),
        bInDtorClear( false ),
        bExpandRefs( false ),
        bDetectiveDirty( false ),
        bHasMacroFunc( false ),
        nAsianCompression(CharCompressType::Invalid),
        nAsianKerning(SC_ASIANKERNING_INVALID),
        bPastingDrawFromOtherDoc( false ),
        nInDdeLinkUpdate( 0 ),
        bInUnoBroadcast( false ),
        bInUnoListenerCall( false ),
        nAdjustHeightLock(0),
        eGrammar( formula::FormulaGrammar::GRAM_NATIVE ),
        bStyleSheetUsageInvalid( true ),
        mbUndoEnabled( true ),
        mbExecuteLinkEnabled( true ),
        mbChangeReadOnlyEnabled( false ),
        mbStreamValidLocked( false ),
        mbUserInteractionEnabled(true),
        mnNamedRangesLockCount(0),
        mbUseEmbedFonts(false),
        mbTrackFormulasPending(false),
        mbFinalTrackFormulas(false)
{
    SetStorageGrammar( formula::FormulaGrammar::GRAM_STORAGE_DEFAULT);

    eSrcSet = osl_getThreadTextEncoding();

    if ( eMode == SCDOCMODE_DOCUMENT )
    {
        mxPoolHelper = new ScPoolHelper( this );

        pBASM = new ScBroadcastAreaSlotMachine( this );
        pChartListenerCollection = new ScChartListenerCollection( this );
        pRefreshTimerControl = new ScRefreshTimerControl;
    }
    else
    {
        pBASM       = nullptr;
        pChartListenerCollection = nullptr;
        pRefreshTimerControl = nullptr;
    }
    pDBCollection = new ScDBCollection(this);
    pSelectionAttr = nullptr;
    pChartCollection = new ScChartCollection;
    apTemporaryChartLock.reset( new ScTemporaryChartLock(this) );
    xColNameRanges = new ScRangePairList;
    xRowNameRanges = new ScRangePairList;
    ImplCreateOptions();
    // languages for a visible document are set by docshell later (from options)
    SetLanguage( ScGlobal::eLnge, ScGlobal::eLnge, ScGlobal::eLnge );

    aTrackIdle.SetInvokeHandler( LINK( this, ScDocument, TrackTimeHdl ) );
}

sfx2::LinkManager* ScDocument::GetLinkManager()
{
    return GetDocLinkManager().getLinkManager();
}

const sfx2::LinkManager* ScDocument::GetLinkManager() const
{
    return GetDocLinkManager().getExistingLinkManager();
}

sc::DocumentLinkManager& ScDocument::GetDocLinkManager()
{
    return *mpDocLinkMgr;
}

const sc::DocumentLinkManager& ScDocument::GetDocLinkManager() const
{
    return const_cast<ScDocument*>(this)->GetDocLinkManager();
}

void ScDocument::SetStorageGrammar( formula::FormulaGrammar::Grammar eGram )
{
    OSL_PRECOND(
        eGram == formula::FormulaGrammar::GRAM_ODFF ||
            eGram == formula::FormulaGrammar::GRAM_PODF,
            "ScDocument::SetStorageGrammar: wrong storage grammar");

    eStorageGrammar = eGram;
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
    nCrc = rtl_crc32( nCrc, &mpShell, sizeof(SfxObjectShell*) );
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
    pChangeTrack = nullptr;
}

void ScDocument::SetChangeTrack( ScChangeTrack* pTrack )
{
    OSL_ENSURE( pTrack->GetDocument() == this, "SetChangeTrack: different documents" );
    if ( !pTrack || pTrack == pChangeTrack || pTrack->GetDocument() != this )
        return ;
    EndChangeTracking();
    pChangeTrack = pTrack;
}

IMPL_LINK_NOARG(ScDocument, TrackTimeHdl, Timer *, void)
{
    if ( ScDdeLink::IsInUpdate() )      // do not nest
    {
        aTrackIdle.Start();            // try again later
    }
    else if (mpShell)                    // execute
    {
        TrackFormulas();
        mpShell->Broadcast( SfxHint( SfxHintId::ScDataChanged ) );

        if (!mpShell->IsModified())
        {
            mpShell->SetModified();
            SfxBindings* pBindings = GetViewBindings();
            if (pBindings)
            {
                pBindings->Invalidate( SID_SAVEDOC );
                pBindings->Invalidate( SID_DOC_MODIFIED );
            }
        }
    }
}

void ScDocument::SetExpandRefs( bool bVal )
{
    bExpandRefs = bVal;
}

void ScDocument::StartTrackTimer()
{
    if (!aTrackIdle.IsActive())        // do not postpone for forever
        aTrackIdle.Start();
}

void ScDocument::ClosingClipboardSource()
{
    if (!bIsClip)
        return;

    ForgetNoteCaptions( ScRangeList( ScRange( 0,0,0, MAXCOL, MAXROW, GetTableCount()-1)), true);
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
        delete pRefreshTimerControl;
        pRefreshTimerControl = nullptr;
    }

    if (IsClipboardSource())
    {
        // Notes copied to the clipboard have a raw SdrCaptionObj pointer
        // copied from this document, forget it as it references this
        // document's drawing layer pages and what not, which otherwise when
        // pasting to another document after this document was destructed would
        // attempt to access non-existing data. Preserve the text data though.
        ScDocument* pClipDoc = ScModule::GetClipDoc();
        if (pClipDoc)
            pClipDoc->ClosingClipboardSource();
    }

    mxFormulaParserPool.reset();
    // Destroy the external ref mgr instance here because it has a timer
    // which needs to be stopped before the app closes.
    pExternalRefMgr.reset();

    ScAddInAsync::RemoveDocument( this );
    ScAddInListener::RemoveDocument( this );
    DELETEZ( pChartListenerCollection);   // before pBASM because of potential Listener!

    DELETEZ(maNonThreaded.pLookupCacheMapImpl);  // before pBASM because of listeners
    DELETEZ(maThreadSpecific.pLookupCacheMapImpl);

    // destroy BroadcastAreas first to avoid un-needed Single-EndListenings of Formula-Cells
    delete pBASM;       // BroadcastAreaSlotMachine
    pBASM = nullptr;

    delete pUnoBroadcaster;     // broadcasts SfxHintId::Dying again
    pUnoBroadcaster = nullptr;

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
    mpPrinter.disposeAndClear();
    ImplDeleteOptions();
    delete pConsolidateDlgData;
    delete pClipData;
    delete pDetOpList;                  // also deletes entries
    delete pChangeTrack;
    delete mpEditEngine;
    delete mpNoteEngine;
    delete pChangeViewSettings;         // and delete
    mpVirtualDevice_100th_mm.disposeAndClear();

    delete pDPCollection;
    delete mpAnonymousDBData;

    // delete the EditEngine before destroying the mxPoolHelper
    delete pCacheFieldEditEngine;

    if ( mxPoolHelper.is() && !bIsClip && !bIsUndo)
        mxPoolHelper->SourceDocumentGone();
    mxPoolHelper.clear();

    delete pScriptTypeData;
    delete maNonThreaded.pRecursionHelper;
    delete maThreadSpecific.pRecursionHelper;

    delete pPreviewFont;
    SAL_WARN_IF( pAutoNameCache, "sc.core", "AutoNameCache still set in dtor" );

    mpFormulaGroupCxt.reset();
    mpCellStringPool.reset();
}

void ScDocument::InitClipPtrs( ScDocument* pSourceDoc )
{
    OSL_ENSURE(bIsClip, "InitClipPtrs and not bIsClip");

    ScMutationGuard aGuard(this, ScMutationGuardFlags::CORE);

    if (pValidationList)
    {
        for(ScValidationDataList::iterator it = pValidationList->begin(); it != pValidationList->end(); ++it )
            delete *it;
        pValidationList->clear();
        DELETEZ(pValidationList);
    }

    Clear();

    SharePooledResources(pSourceDoc);

    //  conditional Formats / validations
    // TODO: Copy Templates?
    const ScValidationDataList* pSourceValid = pSourceDoc->pValidationList;
    if ( pSourceValid )
        pValidationList = new ScValidationDataList(this, *pSourceValid);

                        // store Links in Stream
    delete pClipData;
    if (pSourceDoc->GetDocLinkManager().hasDdeLinks())
    {
        pClipData = new SvMemoryStream;
        pSourceDoc->SaveDdeLinks(*pClipData);
    }
    else
        pClipData = nullptr;

    // Options pointers exist (ImplCreateOptions) for any document.
    // Must be copied for correct results in OLE objects (#i42666#).
    SetDocOptions( pSourceDoc->GetDocOptions() );
    SetViewOptions( pSourceDoc->GetViewOptions() );
}

SvNumberFormatter* ScDocument::GetFormatTable() const
{
    return mxPoolHelper->GetFormTable();
}

SfxItemPool* ScDocument::GetEditPool() const
{
    return mxPoolHelper->GetEditPool();
}

SfxItemPool* ScDocument::GetEnginePool() const
{
    return mxPoolHelper->GetEnginePool();
}

ScFieldEditEngine& ScDocument::GetEditEngine()
{
    if ( !mpEditEngine )
    {
        mpEditEngine = new ScFieldEditEngine(this, GetEnginePool(), GetEditPool());
        mpEditEngine->SetUpdateMode( false );
        mpEditEngine->EnableUndo( false );
        mpEditEngine->SetRefMapMode(MapMode(MapUnit::Map100thMM));
        ApplyAsianEditSettings( *mpEditEngine );
    }
    return *mpEditEngine;
}

ScNoteEditEngine& ScDocument::GetNoteEngine()
{
    if ( !mpNoteEngine )
    {
        ScMutationGuard aGuard(this, ScMutationGuardFlags::CORE);
        mpNoteEngine = new ScNoteEditEngine( GetEnginePool(), GetEditPool() );
        mpNoteEngine->SetUpdateMode( false );
        mpNoteEngine->EnableUndo( false );
        mpNoteEngine->SetRefMapMode(MapMode(MapUnit::Map100thMM));
        ApplyAsianEditSettings( *mpNoteEngine );
        const SfxItemSet& rItemSet = GetDefPattern()->GetItemSet();
        SfxItemSet* pEEItemSet = new SfxItemSet( mpNoteEngine->GetEmptyItemSet() );
        ScPatternAttr::FillToEditItemSet( *pEEItemSet, rItemSet );
        mpNoteEngine->SetDefaults( pEEItemSet );      // edit engine takes ownership
    }
    return *mpNoteEngine;
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
                            maTabs.resize(i, nullptr );
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
            maTabs.resize(nTab+1, nullptr );
        }
        maTabs[nTab] = new ScTable(this, nTab, "baeh");
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
    bool bExtras = !bIsUndo;        // Column-Widths, Row-Heights, Flags
    if (static_cast<size_t>(nTab) >= maTabs.size())
        maTabs.resize(nTab+1, nullptr);

    if (!maTabs[nTab])
        maTabs[nTab] = new ScTable(this, nTab, "temp", bExtras, bExtras);
}

ScRefCellValue ScDocument::GetRefCellValue( const ScAddress& rPos )
{
    if (!TableExists(rPos.Tab()))
        return ScRefCellValue(); // empty

    return maTabs[rPos.Tab()]->GetRefCellValue(rPos.Col(), rPos.Row());
}

svl::SharedStringPool& ScDocument::GetSharedStringPool()
{
    return *mpCellStringPool;
}

const svl::SharedStringPool& ScDocument::GetSharedStringPool() const
{
    return *mpCellStringPool;
}

bool ScDocument::GetPrintArea( SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow,
                                bool bNotes ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
    {
        bool bAny = maTabs[nTab]->GetPrintArea( rEndCol, rEndRow, bNotes );
        if (mpDrawLayer)
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
                                        SCCOL& rEndCol ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
    {
        bool bAny = maTabs[nTab]->GetPrintAreaHor( nStartRow, nEndRow, rEndCol );
        if (mpDrawLayer)
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
        if (mpDrawLayer)
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
        if (mpDrawLayer)
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

void ScDocument::GetTiledRenderingArea(SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow) const
{
    bool bHasPrintArea = GetPrintArea(nTab, rEndCol, rEndRow, false);

    // we need some reasonable minimal document size
    ScViewData* pViewData = ScDocShell::GetViewData();
    if (!pViewData)
    {
        if (!bHasPrintArea)
        {
            rEndCol = 20;
            rEndRow = 50;
        }
        else
        {
            rEndCol += 20;
            rEndRow += 50;
        }
    }
    else if (!bHasPrintArea)
    {
        rEndCol = pViewData->GetMaxTiledCol();
        rEndRow = pViewData->GetMaxTiledRow();
    }
    else
    {
        rEndCol = std::max(rEndCol, pViewData->GetMaxTiledCol());
        rEndRow = std::max(rEndRow, pViewData->GetMaxTiledRow());
    }
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
            sc::AutoCalcSwitch aACSwitch(*this, false);

            SetNoListening( true );
            if (nNewPos == SC_TAB_APPEND || nNewPos >= nTabCount)
                nNewPos = nTabCount-1;

            // Update Reference
            // TODO: combine with UpdateReference!

            sc::RefUpdateMoveTabContext aCxt( *this, nOldPos, nNewPos);

            SCTAB nDz = nNewPos - nOldPos;
            ScRange aSourceRange( 0,0,nOldPos, MAXCOL,MAXROW,nOldPos );
            if (pRangeName)
                pRangeName->UpdateMoveTab(aCxt);

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
                pValidationList->UpdateMoveTab(aCxt);
            if ( pUnoBroadcaster )
                pUnoBroadcaster->Broadcast( ScUpdateRefHint( URM_REORDER,
                            aSourceRange, 0,0,nDz ) );

            ScTable* pSaveTab = maTabs[nOldPos];
            maTabs.erase(maTabs.begin()+nOldPos);
            maTabs.insert(maTabs.begin()+nNewPos, pSaveTab);
            TableContainer::iterator it = maTabs.begin();
            for (SCTAB i = 0; i < nTabCount; i++)
                if (maTabs[i])
                    maTabs[i]->UpdateMoveTab(aCxt, i, pProgress);
            it = maTabs.begin();
            for (; it != maTabs.end(); ++it)
                if (*it)
                    (*it)->UpdateCompile();
            SetNoListening( false );
            StartAllListeners();

            // sheet names of references may not be valid until sheet is moved
            pChartListenerCollection->UpdateScheduledSeriesRanges();

            sc::SetFormulaDirtyContext aFormulaDirtyCxt;
            SetAllFormulasDirty(aFormulaDirtyCxt);

            if (mpDrawLayer)
                mpDrawLayer->ScMovePage( static_cast<sal_uInt16>(nOldPos), static_cast<sal_uInt16>(nNewPos) );

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

    //  check first if Prefix is valid; if not, then only avoid duplicates
    bool bPrefix = ValidTabName( aName );
    OSL_ENSURE(bPrefix, "invalid table name");
    SCTAB nDummy;

    CreateValidTabName(aName);

    bool bValid;
    if (bPrefix)
        bValid = ValidNewTabName(aName);
    else
        bValid = !GetTable( aName, nDummy );

    sc::AutoCalcSwitch aACSwitch(*this, false);
    sc::RefUpdateInsertTabContext aCxt( *this, nNewPos, 1);

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
                    pRangeName->UpdateInsertTab(aCxt);

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
                        (*it)->UpdateInsertTab(aCxt);
                maTabs.push_back(nullptr);
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
                sc::StartListeningContext aSLCxt(*this);
                for (TableContainer::iterator it = maTabs.begin(); it != maTabs.end(); ++it)
                    if (*it && it != maTabs.begin()+nOldPos && it != maTabs.begin()+nNewPos)
                        (*it)->StartListeners(aSLCxt, true);

                if (pValidationList)
                    pValidationList->UpdateInsertTab(aCxt);

                // sheet names of references may not be valid until sheet is copied
                pChartListenerCollection->UpdateScheduledSeriesRanges();
            }
            else
                bValid = false;
        }
    }

    if (bValid)
    {
        SetNoListening( true );     // not yet at CopyToTable/Insert

        const bool bGlobalNamesToLocal = true;
        const SCTAB nRealOldPos = (nNewPos < nOldPos) ? nOldPos - 1 : nOldPos;
        const ScRangeName* pNames = GetRangeName( nOldPos);
        if (pNames)
            pNames->CopyUsedNames( nOldPos, nRealOldPos, nNewPos, *this, *this, bGlobalNamesToLocal);
        GetRangeName()->CopyUsedNames( -1, nRealOldPos, nNewPos, *this, *this, bGlobalNamesToLocal);

        sc::CopyToDocContext aCopyDocCxt(*this);
        maTabs[nOldPos]->CopyToTable(aCopyDocCxt, 0, 0, MAXCOL, MAXROW, InsertDeleteFlags::ALL,
                (pOnlyMarked != nullptr), maTabs[nNewPos], pOnlyMarked,
                false /*bAsLink*/, true /*bColRowFlags*/, bGlobalNamesToLocal, false /*bCopyCaptions*/ );
        maTabs[nNewPos]->SetTabBgColor(maTabs[nOldPos]->GetTabBgColor());

        SCTAB nDz = nNewPos - nOldPos;
        sc::RefUpdateContext aRefCxt(*this);
        aRefCxt.meMode = URM_COPY;
        aRefCxt.maRange = ScRange(0, 0, nNewPos, MAXCOL, MAXROW, nNewPos);
        aRefCxt.mnTabDelta = nDz;
        maTabs[nNewPos]->UpdateReference(aRefCxt);

        maTabs[nNewPos]->UpdateInsertTabAbs(nNewPos); // move all paragraphs up by one!!
        maTabs[nOldPos]->UpdateInsertTab(aCxt);

        maTabs[nOldPos]->UpdateCompile();
        maTabs[nNewPos]->UpdateCompile( true ); //  maybe already compiled in Clone, but used names need recompilation
        SetNoListening( false );
        sc::StartListeningContext aSLCxt(*this);
        maTabs[nOldPos]->StartListeners(aSLCxt, true);
        maTabs[nNewPos]->StartListeners(aSLCxt, true);

        sc::SetFormulaDirtyContext aFormulaDirtyCxt;
        SetAllFormulasDirty(aFormulaDirtyCxt);

        if (mpDrawLayer) //  Skip cloning Note caption object
            // page is already created in ScTable ctor
            mpDrawLayer->ScCopyPage( static_cast<sal_uInt16>(nOldPos), static_cast<sal_uInt16>(nNewPos) );

        if (pDPCollection)
            pDPCollection->CopyToTab(nOldPos, nNewPos);

        maTabs[nNewPos]->SetPageStyle( maTabs[nOldPos]->GetPageStyle() );
        maTabs[nNewPos]->SetPendingRowHeights( maTabs[nOldPos]->IsPendingRowHeights() );

        // Copy the custom print range if exists.
        maTabs[nNewPos]->CopyPrintRange(*maTabs[nOldPos]);

        // Copy the RTL settings
        maTabs[nNewPos]->SetLayoutRTL(maTabs[nOldPos]->IsLayoutRTL());
        maTabs[nNewPos]->SetLoadingRTL(maTabs[nOldPos]->IsLoadingRTL());

        // Finally copy the note captions, which need
        // 1. the updated source ScColumn::nTab members if nNewPos <= nOldPos
        // 2. row heights and column widths of the destination
        // 3. RTL settings of the destination
        maTabs[nOldPos]->CopyCaptionsToTable( 0, 0, MAXCOL, MAXROW, maTabs[nNewPos], true /*bCloneCaption*/);
    }

    return bValid;
}

sal_uLong ScDocument::TransferTab( ScDocument* pSrcDoc, SCTAB nSrcPos,
                                SCTAB nDestPos, bool bInsertNew,
                                bool bResultsOnly )
{
    sal_uLong nRetVal = 1;                  // 0 => error 1 = ok
                                            // 3 => NameBox
                                            // 4 => both

    if (pSrcDoc->mpShell->GetMedium())
    {
        pSrcDoc->maFileURL = pSrcDoc->mpShell->GetMedium()->GetURLObject().GetMainURL(INetURLObject::DecodeMechanism::ToIUri);
        // for unsaved files use the title name and adjust during save of file
        if (pSrcDoc->maFileURL.isEmpty())
            pSrcDoc->maFileURL = pSrcDoc->mpShell->GetName();
    }
    else
    {
        pSrcDoc->maFileURL = pSrcDoc->mpShell->GetName();
    }

    bool bValid = true;
    if (bInsertNew)             // re-insert
    {
        OUString aName;
        pSrcDoc->GetName(nSrcPos, aName);
        CreateValidTabName(aName);
        bValid = InsertTab(nDestPos, aName);

        // Copy the RTL settings
        maTabs[nDestPos]->SetLayoutRTL(pSrcDoc->maTabs[nSrcPos]->IsLayoutRTL());
        maTabs[nDestPos]->SetLoadingRTL(pSrcDoc->maTabs[nSrcPos]->IsLoadingRTL());
    }
    else                        // replace existing tables
    {
        if (ValidTab(nDestPos) && nDestPos < static_cast<SCTAB>(maTabs.size()) && maTabs[nDestPos])
        {
            maTabs[nDestPos]->DeleteArea( 0,0, MAXCOL,MAXROW, InsertDeleteFlags::ALL );
        }
        else
            bValid = false;
    }

    if (bValid)
    {
        bool bOldAutoCalcSrc = false;
        bool bOldAutoCalc = GetAutoCalc();
        SetAutoCalc( false );   // avoid repeated calculations
        SetNoListening( true );
        if ( bResultsOnly )
        {
            bOldAutoCalcSrc = pSrcDoc->GetAutoCalc();
            pSrcDoc->SetAutoCalc( true );   // in case something needs calculation
        }

        {
            NumFmtMergeHandler aNumFmtMergeHdl(this, pSrcDoc);

            sc::CopyToDocContext aCxt(*this);
            nDestPos = std::min(nDestPos, (SCTAB)(GetTableCount() - 1));
            {   // scope for bulk broadcast
                ScBulkBroadcast aBulkBroadcast( pBASM, SfxHintId::ScDataChanged);
                if (!bResultsOnly)
                {
                    const bool bGlobalNamesToLocal = false;
                    const ScRangeName* pNames = pSrcDoc->GetRangeName( nSrcPos);
                    if (pNames)
                        pNames->CopyUsedNames( nSrcPos, nSrcPos, nDestPos, *pSrcDoc, *this, bGlobalNamesToLocal);
                    pSrcDoc->GetRangeName()->CopyUsedNames( -1, nSrcPos, nDestPos, *pSrcDoc, *this, bGlobalNamesToLocal);
                }
                pSrcDoc->maTabs[nSrcPos]->CopyToTable(aCxt, 0, 0, MAXCOL, MAXROW,
                        ( bResultsOnly ? InsertDeleteFlags::ALL & ~InsertDeleteFlags::FORMULA : InsertDeleteFlags::ALL),
                        false, maTabs[nDestPos] );
            }
        }
        maTabs[nDestPos]->SetTabNo(nDestPos);
        maTabs[nDestPos]->SetTabBgColor(pSrcDoc->maTabs[nSrcPos]->GetTabBgColor());

        if ( !bResultsOnly )
        {
            sc::RefUpdateContext aRefCxt(*this);
            aRefCxt.meMode = URM_COPY;
            aRefCxt.maRange = ScRange(0, 0, nDestPos, MAXCOL, MAXROW, nDestPos);
            aRefCxt.mnTabDelta = nDestPos - nSrcPos;
            maTabs[nDestPos]->UpdateReference(aRefCxt);

            // Readjust self-contained absolute references to this sheet
            maTabs[nDestPos]->TestTabRefAbs(nSrcPos);
            sc::CompileFormulaContext aFormulaCxt(this);
            maTabs[nDestPos]->CompileAll(aFormulaCxt);
        }

        SetNoListening( false );
        if ( !bResultsOnly )
        {
            sc::StartListeningContext aSLCxt(*this);
            maTabs[nDestPos]->StartListeners(aSLCxt, true);
        }
        SetDirty( ScRange( 0, 0, nDestPos, MAXCOL, MAXROW, nDestPos), false);

        if ( bResultsOnly )
            pSrcDoc->SetAutoCalc( bOldAutoCalcSrc );
        SetAutoCalc( bOldAutoCalc );

        //  copy Drawing

        if (bInsertNew)
            TransferDrawPage( pSrcDoc, nSrcPos, nDestPos );

        maTabs[nDestPos]->SetPendingRowHeights( pSrcDoc->maTabs[nSrcPos]->IsPendingRowHeights() );
    }
    if (!bValid)
        nRetVal = 0;
    bool bVbaEnabled = IsInVBAMode();

    if ( bVbaEnabled  )
    {
        SfxObjectShell* pSrcShell = pSrcDoc->GetDocumentShell();
        if ( pSrcShell )
        {
            OUString aLibName("Standard");
            const BasicManager *pBasicManager = pSrcShell->GetBasicManager();
            if (pBasicManager && !pBasicManager->GetName().isEmpty())
            {
                aLibName = pSrcShell->GetBasicManager()->GetName();
            }
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
            VBA_InsertModule( *this, nDestPos, OUString(), sSource );
        }
    }

    return nRetVal;
}

void ScDocument::SetError( SCCOL nCol, SCROW nRow, SCTAB nTab, const FormulaError nError)
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
        if (maTabs[nTab])
            maTabs[nTab]->SetError( nCol, nRow, nError );
}

void ScDocument::SetFormula(
    const ScAddress& rPos, const ScTokenArray& rArray )
{
    if (!TableExists(rPos.Tab()))
        return;

    maTabs[rPos.Tab()]->SetFormula(rPos.Col(), rPos.Row(), rArray, formula::FormulaGrammar::GRAM_DEFAULT);
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
        return nullptr;
    }

    return maTabs[rPos.Tab()]->SetFormulaCell(rPos.Col(), rPos.Row(), pCell);
}

bool ScDocument::SetFormulaCells( const ScAddress& rPos, std::vector<ScFormulaCell*>& rCells )
{
    if (rCells.empty())
        return false;

    ScTable* pTab = FetchTable(rPos.Tab());
    if (!pTab)
        return false;

    return pTab->SetFormulaCells(rPos.Col(), rPos.Row(), rCells);
}

void ScDocument::SetConsolidateDlgData( const ScConsolidateParam* pData )
{
    delete pConsolidateDlgData;

    if ( pData )
        pConsolidateDlgData = new ScConsolidateParam( *pData );
    else
        pConsolidateDlgData = nullptr;
}

void ScDocument::SetChangeViewSettings(const ScChangeViewSettings& rNew)
{
    if (pChangeViewSettings==nullptr)
        pChangeViewSettings = new ScChangeViewSettings;

    OSL_ENSURE( pChangeViewSettings, "Oops. No ChangeViewSettings :-( by!" );

    *pChangeViewSettings=rNew;
}

ScFieldEditEngine* ScDocument::CreateFieldEditEngine()
{
    ScFieldEditEngine* pNewEditEngine = nullptr;
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
        pCacheFieldEditEngine = nullptr;
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
    rpEditEngine = nullptr;
}

ScRecursionHelper* ScDocument::CreateRecursionHelperInstance()
{
    return new ScRecursionHelper;
}

ScLookupCache & ScDocument::GetLookupCache( const ScRange & rRange )
{
    ScLookupCache* pCache = nullptr;
    if (!mbThreadedGroupCalcInProgress)
    {
        if (!maNonThreaded.pLookupCacheMapImpl)
            maNonThreaded.pLookupCacheMapImpl = new ScLookupCacheMapImpl;
        auto it(maNonThreaded.pLookupCacheMapImpl->aCacheMap.find(rRange));
        if (it == maNonThreaded.pLookupCacheMapImpl->aCacheMap.end())
        {
            pCache = new ScLookupCache(this, rRange);
            AddLookupCache(*pCache);
        }
        else
            pCache = (*it).second;
    }
    else
    {
        if (!maThreadSpecific.pLookupCacheMapImpl)
            maThreadSpecific.pLookupCacheMapImpl = new ScLookupCacheMapImpl;
        auto it(maThreadSpecific.pLookupCacheMapImpl->aCacheMap.find(rRange));
        if (it == maThreadSpecific.pLookupCacheMapImpl->aCacheMap.end())
        {
            pCache = new ScLookupCache(this, rRange);
            AddLookupCache(*pCache);
        }
        else
            pCache = (*it).second;
    }
    return *pCache;
}

void ScDocument::AddLookupCache( ScLookupCache & rCache )
{
    if (!mbThreadedGroupCalcInProgress)
    {
        if (!maNonThreaded.pLookupCacheMapImpl->aCacheMap.insert( ::std::pair< const ScRange,
                ScLookupCache*>(rCache.getRange(), &rCache)).second)
        {
            OSL_FAIL( "ScDocument::AddLookupCache: couldn't add to hash map");
        }
        else
            StartListeningArea(rCache.getRange(), false, &rCache);
    }
    else
    {
        if (!maThreadSpecific.pLookupCacheMapImpl->aCacheMap.insert( ::std::pair< const ScRange,
                ScLookupCache*>(rCache.getRange(), &rCache)).second)
        {
            OSL_FAIL( "ScDocument::AddLookupCache: couldn't add to hash map");
        }
        else
            StartListeningArea(rCache.getRange(), false, &rCache);
    }
}

void ScDocument::RemoveLookupCache( ScLookupCache & rCache )
{
    if (!mbThreadedGroupCalcInProgress)
    {
        auto it(maNonThreaded.pLookupCacheMapImpl->aCacheMap.find(rCache.getRange()));
        if (it == maNonThreaded.pLookupCacheMapImpl->aCacheMap.end())
        {
            OSL_FAIL( "ScDocument::RemoveLookupCache: range not found in hash map");
        }
        else
        {
            ScLookupCache* pCache = (*it).second;
            maNonThreaded.pLookupCacheMapImpl->aCacheMap.erase(it);
            EndListeningArea(pCache->getRange(), false, &rCache);
        }
    }
    else
    {
        auto it( maThreadSpecific.pLookupCacheMapImpl->aCacheMap.find(rCache.getRange()));
        if (it == maThreadSpecific.pLookupCacheMapImpl->aCacheMap.end())
        {
            OSL_FAIL( "ScDocument::RemoveLookupCache: range not found in hash map");
        }
        else
        {
            ScLookupCache* pCache = (*it).second;
            maThreadSpecific.pLookupCacheMapImpl->aCacheMap.erase(it);
            EndListeningArea(pCache->getRange(), false, &rCache);
        }
    }
}

void ScDocument::ClearLookupCaches()
{
    if (!mbThreadedGroupCalcInProgress)
    {
        if (maNonThreaded.pLookupCacheMapImpl )
            maNonThreaded.pLookupCacheMapImpl->clear();
    }
    else
    {
        if( maThreadSpecific.pLookupCacheMapImpl )
            maThreadSpecific.pLookupCacheMapImpl->clear();
    }
}

bool ScDocument::IsCellInChangeTrack(const ScAddress &cell,Color *pColCellBorder)
{
    ScChangeTrack* pTrack = GetChangeTrack();
    ScChangeViewSettings* pSettings = GetChangeViewSettings();
    if ( !pTrack || !pTrack->GetFirst() || !pSettings || !pSettings->ShowChanges() )
        return false;           // missing or turned-off
    ScActionColorChanger aColorChanger(*pTrack);
    //  Clipping happens from outside
    //! TODO: without Clipping; only paint affected cells ??!??!?
    const ScChangeAction* pAction = pTrack->GetFirst();
    while (pAction)
    {
        ScChangeActionType eType;
        if ( pAction->IsVisible() )
        {
            eType = pAction->GetType();
            const ScBigRange& rBig = pAction->GetBigRange();
            if ( rBig.aStart.Tab() == cell.Tab())
            {
                ScRange aRange = rBig.MakeRange();
                if ( eType == SC_CAT_DELETE_ROWS )
                    aRange.aEnd.SetRow( aRange.aStart.Row() );
                else if ( eType == SC_CAT_DELETE_COLS )
                    aRange.aEnd.SetCol( aRange.aStart.Col() );
                if (ScViewUtil::IsActionShown( *pAction, *pSettings, *this ) )
                {
                    if (aRange.In(cell))
                    {
                        if (pColCellBorder != nullptr)
                        {
                            aColorChanger.Update( *pAction );
                            Color aColor( aColorChanger.GetColor() );
                            *pColCellBorder = aColor;
                        }
                        return true;
                    }
                }
            }
            if ( eType == SC_CAT_MOVE &&
                static_cast<const ScChangeActionMove*>(pAction)->
                GetFromRange().aStart.Tab() == cell.Col() )
            {
                ScRange aRange = static_cast<const ScChangeActionMove*>(pAction)->
                    GetFromRange().MakeRange();
                if (ScViewUtil::IsActionShown( *pAction, *pSettings, *this ) )
                {
                    if (aRange.In(cell))
                    {
                        if (pColCellBorder != nullptr)
                        {
                            aColorChanger.Update( *pAction );
                            Color aColor( aColorChanger.GetColor() );
                            *pColCellBorder = aColor;
                        }
                        return true;
                    }
                }
            }
        }
        pAction = pAction->GetNext();
    }
    return false;
}

void ScDocument::GetCellChangeTrackNote( const ScAddress &aCellPos, OUString &aTrackText,bool &bLeftEdge)
{
    aTrackText.clear();
    //  Change-Tracking
    ScChangeTrack* pTrack = GetChangeTrack();
    ScChangeViewSettings* pSettings = GetChangeViewSettings();
    if ( pTrack && pTrack->GetFirst() && pSettings && pSettings->ShowChanges())
    {
        const ScChangeAction* pFound = nullptr;
        const ScChangeAction* pFoundContent = nullptr;
        const ScChangeAction* pFoundMove = nullptr;
        long nModified = 0;
        const ScChangeAction* pAction = pTrack->GetFirst();
        while (pAction)
        {
            if ( pAction->IsVisible() &&
                 ScViewUtil::IsActionShown( *pAction, *pSettings, *this ) )
            {
                ScChangeActionType eType = pAction->GetType();
                const ScBigRange& rBig = pAction->GetBigRange();
                if ( rBig.aStart.Tab() == aCellPos.Tab())
                {
                    ScRange aRange = rBig.MakeRange();
                    if ( eType == SC_CAT_DELETE_ROWS )
                        aRange.aEnd.SetRow( aRange.aStart.Row() );
                    else if ( eType == SC_CAT_DELETE_COLS )
                        aRange.aEnd.SetCol( aRange.aStart.Col() );
                    if ( aRange.In( aCellPos ) )
                    {
                        pFound = pAction;       // the last wins
                        switch ( eType )
                        {
                            case SC_CAT_CONTENT :
                                pFoundContent = pAction;
                            break;
                            case SC_CAT_MOVE :
                                pFoundMove = pAction;
                            break;
                            default:
                                break;
                        }
                        ++nModified;
                    }
                }
                if ( eType == SC_CAT_MOVE )
                {
                    ScRange aRange =
                        static_cast<const ScChangeActionMove*>(pAction)->
                        GetFromRange().MakeRange();
                    if ( aRange.In( aCellPos ) )
                    {
                        pFound = pAction;
                        ++nModified;
                    }
                }
            }
            pAction = pAction->GetNext();
        }
        if ( pFound )
        {
            if ( pFoundContent && pFound->GetType() != SC_CAT_CONTENT )
                pFound = pFoundContent;     // Content wins
            if ( pFoundMove && pFound->GetType() != SC_CAT_MOVE &&
                    pFoundMove->GetActionNumber() >
                    pFound->GetActionNumber() )
                pFound = pFoundMove;        // Move wins
            //  for deleted columns: arrow on left side of row
            if ( pFound->GetType() == SC_CAT_DELETE_COLS )
                bLeftEdge = true;
            DateTime aDT = pFound->GetDateTime();
            aTrackText  = pFound->GetUser();
            aTrackText += ", ";
            aTrackText += ScGlobal::pLocaleData->getDate(aDT);
            aTrackText += " ";
            aTrackText += ScGlobal::pLocaleData->getTime(aDT);
            aTrackText += ":\n";
            OUString aComStr = pFound->GetComment();
            if(!aComStr.isEmpty())
            {
                aTrackText += aComStr;
                aTrackText += "\n( ";
            }
            pFound->GetDescription( aTrackText, this );
            if (!aComStr.isEmpty())
            {
                aTrackText += ")";
            }
        }
    }
}

void ScDocument::SetPreviewFont( SfxItemSet* pFont )
{
    delete pPreviewFont;
    pPreviewFont = pFont;
}

void  ScDocument::SetPreviewSelection( const ScMarkData& rSel )
{
    maPreviewSelection = rSel;
}

SfxItemSet* ScDocument::GetPreviewFont( SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    SfxItemSet* pRet = nullptr;
    if ( pPreviewFont )
    {
        ScMarkData aSel = GetPreviewSelection();
        if ( aSel.IsCellMarked( nCol, nRow ) && aSel.GetFirstSelected() == nTab )
            pRet = pPreviewFont;
    }
    return pRet;
}

ScStyleSheet* ScDocument::GetPreviewCellStyle( SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    ScStyleSheet* pRet = nullptr;
    ScMarkData aSel = GetPreviewSelection();
    if ( pPreviewCellStyle && aSel.IsCellMarked( nCol, nRow ) && aSel.GetFirstSelected() == nTab  )
        pRet = pPreviewCellStyle;
    return pRet;
}

sc::IconSetBitmapMap& ScDocument::GetIconSetBitmapMap()
{
    if (!m_pIconSetBitmapMap)
    {
        m_pIconSetBitmapMap.reset(new sc::IconSetBitmapMap);
    }
    return *m_pIconSetBitmapMap;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
