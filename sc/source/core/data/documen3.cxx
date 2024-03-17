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

#include <sal/types.h>
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>
#include <com/sun/star/sheet/TableValidationVisibility.hpp>
#include <scitems.hxx>
#include <editeng/langitem.hxx>
#include <svl/srchitem.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewsh.hxx>
#include <vcl/svapp.hxx>
#include <osl/thread.hxx>
#include <osl/diagnose.h>
#include <tools/duration.hxx>
#include <document.hxx>
#include <attrib.hxx>
#include <table.hxx>
#include <rangenam.hxx>
#include <dbdata.hxx>
#include <docpool.hxx>
#include <docsh.hxx>
#include <poolhelp.hxx>
#include <rangelst.hxx>
#include <chartlock.hxx>
#include <refupdat.hxx>
#include <docoptio.hxx>
#include <scmod.hxx>
#include <clipoptions.hxx>
#include <viewopti.hxx>
#include <scextopt.hxx>
#include <tablink.hxx>
#include <externalrefmgr.hxx>
#include <markdata.hxx>
#include <validat.hxx>
#include <dociter.hxx>
#include <detdata.hxx>
#include <inputopt.hxx>
#include <chartlis.hxx>
#include <sc.hrc>
#include <hints.hxx>
#include <dpobject.hxx>
#include <drwlayer.hxx>
#include <unoreflist.hxx>
#include <listenercalls.hxx>
#include <tabprotection.hxx>
#include <formulaparserpool.hxx>
#include <clipparam.hxx>
#include <sheetevents.hxx>
#include <queryentry.hxx>
#include <formulacell.hxx>
#include <refupdatecontext.hxx>
#include <scopetools.hxx>
#include <filterentries.hxx>
#include <queryparam.hxx>

#include <globalnames.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <config_fuzzers.h>
#include <memory>

using namespace com::sun::star;

namespace {

void sortAndRemoveDuplicates(std::vector<ScTypedStrData>& rStrings, bool bCaseSens)
{
    if (bCaseSens)
    {
        std::stable_sort(rStrings.begin(), rStrings.end(), ScTypedStrData::LessCaseSensitive());
        std::vector<ScTypedStrData>::iterator it =
            std::unique(rStrings.begin(), rStrings.end(), ScTypedStrData::EqualCaseSensitive());
        rStrings.erase(it, rStrings.end());
        std::stable_sort(rStrings.begin(), rStrings.end(), ScTypedStrData::LessSortCaseSensitive());
    }
    else
    {
        std::stable_sort(rStrings.begin(), rStrings.end(), ScTypedStrData::LessCaseInsensitive());
        std::vector<ScTypedStrData>::iterator it =
            std::unique(rStrings.begin(), rStrings.end(), ScTypedStrData::EqualCaseInsensitive());
        rStrings.erase(it, rStrings.end());
        std::stable_sort(rStrings.begin(), rStrings.end(), ScTypedStrData::LessSortCaseInsensitive());
    }
    if (std::any_of(rStrings.begin(), rStrings.end(),
        [](ScTypedStrData& rString) { return rString.IsHiddenByFilter(); })) {
        std::stable_sort(rStrings.begin(), rStrings.end(), ScTypedStrData::LessHiddenRows());
    }
}

}

void ScDocument::GetAllTabRangeNames(ScRangeName::TabNameCopyMap& rNames) const
{
    ScRangeName::TabNameCopyMap aNames;
    for (SCTAB i = 0; i < GetTableCount(); ++i)
    {
        if (!maTabs[i])
            // no more tables to iterate through.
            break;

        const ScRangeName* p = maTabs[i]->mpRangeName.get();
        if (!p || p->empty())
            // ignore empty ones.
            continue;

        aNames.emplace(i, p);
    }
    rNames.swap(aNames);
}

void ScDocument::SetAllRangeNames(const std::map<OUString, ScRangeName>& rRangeMap)
{
    for (const auto& [rName, rRangeName] : rRangeMap)
    {
        if (rName == STR_GLOBAL_RANGE_NAME)
        {
            pRangeName.reset();
            if (!rRangeName.empty())
                pRangeName.reset( new ScRangeName( rRangeName ) );
        }
        else
        {
            SCTAB nTab;
            bool bFound = GetTable(rName, nTab);
            assert(bFound); (void)bFound;   // fouled up?
            if (rRangeName.empty())
                SetRangeName( nTab, nullptr );
            else
                SetRangeName( nTab, std::unique_ptr<ScRangeName>(new ScRangeName( rRangeName )) );
        }
    }
}

void ScDocument::GetRangeNameMap(std::map<OUString, ScRangeName*>& aRangeNameMap)
{
    for (SCTAB i = 0; i < GetTableCount(); ++i)
    {
        if (!maTabs[i])
            continue;
        ScRangeName* p = maTabs[i]->GetRangeName();
        if (!p )
        {
            p = new ScRangeName();
            SetRangeName(i, std::unique_ptr<ScRangeName>(p));
        }
        OUString aTableName = maTabs[i]->GetName();
        aRangeNameMap.insert(std::pair<OUString, ScRangeName*>(aTableName,p));
    }
    if (!pRangeName)
    {
        pRangeName.reset(new ScRangeName());
    }
    aRangeNameMap.insert(std::pair<OUString, ScRangeName*>(STR_GLOBAL_RANGE_NAME, pRangeName.get()));
}

ScRangeName* ScDocument::GetRangeName(SCTAB nTab) const
{
    if (const ScTable* pTable = FetchTable(nTab))
        return pTable->GetRangeName();
    return nullptr;
}

ScRangeName* ScDocument::GetRangeName() const
{
    if (!pRangeName)
        pRangeName.reset(new ScRangeName);
    return pRangeName.get();
}

void ScDocument::SetRangeName(SCTAB nTab, std::unique_ptr<ScRangeName> pNew)
{
    if (ScTable* pTable = FetchTable(nTab))
        pTable->SetRangeName(std::move(pNew));
}

void ScDocument::SetRangeName( std::unique_ptr<ScRangeName> pNewRangeName )
{
    pRangeName = std::move(pNewRangeName);
}

bool ScDocument::IsAddressInRangeName( RangeNameScope eScope, const ScAddress& rAddress )
{
    ScRangeName* pRangeNames;
    ScRange aNameRange;

    if (eScope == RangeNameScope::GLOBAL)
        pRangeNames= GetRangeName();
    else
        pRangeNames= GetRangeName(rAddress.Tab());

    for (const auto& rEntry : *pRangeNames)
    {
        if (rEntry.second->IsValidReference(aNameRange))
        {
            if (aNameRange.Contains(rAddress))
                return true;
        }
    }

    return false;
}

bool ScDocument::InsertNewRangeName( const OUString& rName, const ScAddress& rPos, const OUString& rExpr )
{
    ScRangeName* pGlobalNames = GetRangeName();
    if (!pGlobalNames)
        return false;

    ScRangeData* pName = new ScRangeData(*this, rName, rExpr, rPos, ScRangeData::Type::Name, GetGrammar());
    return pGlobalNames->insert(pName);
}

bool ScDocument::InsertNewRangeName( SCTAB nTab, const OUString& rName, const ScAddress& rPos, const OUString& rExpr )
{
    ScRangeName* pLocalNames = GetRangeName(nTab);
    if (!pLocalNames)
        return false;

    ScRangeData* pName = new ScRangeData(*this, rName, rExpr, rPos, ScRangeData::Type::Name, GetGrammar());
    return pLocalNames->insert(pName);
}

const ScRangeData* ScDocument::GetRangeAtBlock( const ScRange& rBlock, OUString& rName, bool* pSheetLocal ) const
{
    const ScRangeData* pData = nullptr;
    if (rBlock.aStart.Tab() == rBlock.aEnd.Tab())
    {
        const ScRangeName* pLocalNames = GetRangeName(rBlock.aStart.Tab());
        if (pLocalNames)
        {
            pData = pLocalNames->findByRange( rBlock );
            if (pData)
            {
                rName = pData->GetName();
                if (pSheetLocal)
                    *pSheetLocal = true;
                return pData;
            }
        }
    }
    if ( pRangeName )
    {
        pData = pRangeName->findByRange( rBlock );
        if (pData)
        {
            rName = pData->GetName();
            if (pSheetLocal)
                *pSheetLocal = false;
        }
    }
    return pData;
}

ScRangeData* ScDocument::FindRangeNameBySheetAndIndex( SCTAB nTab, sal_uInt16 nIndex ) const
{
    const ScRangeName* pRN = (nTab < 0 ? GetRangeName() : GetRangeName(nTab));
    return (pRN ? pRN->findByIndex( nIndex) : nullptr);
}

void ScDocument::SetDBCollection( std::unique_ptr<ScDBCollection> pNewDBCollection, bool bRemoveAutoFilter )
{
    if (pDBCollection && bRemoveAutoFilter)
    {
        //  remove auto filter attribute if new db data don't contain auto filter flag
        //  start position is also compared, so bRemoveAutoFilter must not be set from ref-undo!

        ScDBCollection::NamedDBs& rNamedDBs = pDBCollection->getNamedDBs();
        for (const auto& rxNamedDB : rNamedDBs)
        {
            const ScDBData& rOldData = *rxNamedDB;
            if (!rOldData.HasAutoFilter())
                continue;

            ScRange aOldRange;
            rOldData.GetArea(aOldRange);

            bool bFound = false;
            if (pNewDBCollection)
            {
                ScDBData* pNewData = pNewDBCollection->getNamedDBs().findByUpperName(rOldData.GetUpperName());
                if (pNewData)
                {
                    if (pNewData->HasAutoFilter())
                    {
                        ScRange aNewRange;
                        pNewData->GetArea(aNewRange);
                        if (aOldRange.aStart == aNewRange.aStart)
                            bFound = true;
                    }
                }
            }

            if (!bFound)
            {
                aOldRange.aEnd.SetRow(aOldRange.aStart.Row());
                RemoveFlagsTab( aOldRange.aStart.Col(), aOldRange.aStart.Row(),
                                aOldRange.aEnd.Col(),   aOldRange.aEnd.Row(),
                                aOldRange.aStart.Tab(), ScMF::Auto );
                RepaintRange( aOldRange );
            }
        }
    }

    pDBCollection = std::move(pNewDBCollection);
}

const ScDBData* ScDocument::GetDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, ScDBDataPortion ePortion) const
{
    if (pDBCollection)
        return pDBCollection->GetDBAtCursor(nCol, nRow, nTab, ePortion);
    else
        return nullptr;
}

ScDBData* ScDocument::GetDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, ScDBDataPortion ePortion)
{
    if (pDBCollection)
        return pDBCollection->GetDBAtCursor(nCol, nRow, nTab, ePortion);
    else
        return nullptr;
}

const ScDBData* ScDocument::GetDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2) const
{
    if (pDBCollection)
        return pDBCollection->GetDBAtArea(nTab, nCol1, nRow1, nCol2, nRow2);
    else
        return nullptr;
}

ScDBData* ScDocument::GetDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2)
{
    if (pDBCollection)
        return pDBCollection->GetDBAtArea(nTab, nCol1, nRow1, nCol2, nRow2);
    else
        return nullptr;
}

void ScDocument::RefreshDirtyTableColumnNames()
{
    if (pDBCollection)
        pDBCollection->RefreshDirtyTableColumnNames();
}

bool ScDocument::HasPivotTable() const
{
    return pDPCollection && pDPCollection->GetCount();
}

ScDPCollection* ScDocument::GetDPCollection()
{
    if (!pDPCollection)
        pDPCollection.reset( new ScDPCollection(*this) );
    return pDPCollection.get();
}

const ScDPCollection* ScDocument::GetDPCollection() const
{
    return pDPCollection.get();
}

ScDPObject* ScDocument::GetDPAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab) const
{
    if (!pDPCollection)
        return nullptr;

    sal_uInt16 nCount = pDPCollection->GetCount();
    ScAddress aPos( nCol, nRow, nTab );
    for (sal_uInt16 i=0; i<nCount; i++)
        if ( (*pDPCollection)[i].GetOutRange().Contains( aPos ) )
            return &(*pDPCollection)[i];

    return nullptr;
}

ScDPObject* ScDocument::GetDPAtBlock( const ScRange & rBlock ) const
{
    if (!pDPCollection)
        return nullptr;

    /* Walk the collection in reverse order to get something of an
     * approximation of MS Excels 'most recent' effect. */
    sal_uInt16 i = pDPCollection->GetCount();
    while ( i-- > 0 )
        if ( (*pDPCollection)[i].GetOutRange().Contains( rBlock ) )
            return &(*pDPCollection)[i];

    return nullptr;
}

void ScDocument::StopTemporaryChartLock()
{
    if (apTemporaryChartLock)
        apTemporaryChartLock->StopLocking();
}

void ScDocument::SetChartListenerCollection(
            std::unique_ptr<ScChartListenerCollection> pNewChartListenerCollection,
            bool bSetChartRangeLists )
{
    std::unique_ptr<ScChartListenerCollection> pOld = std::move(pChartListenerCollection);
    pChartListenerCollection = std::move(pNewChartListenerCollection);
    if ( pChartListenerCollection )
    {
        if ( pOld )
            pChartListenerCollection->SetDiffDirty( *pOld, bSetChartRangeLists );
        pChartListenerCollection->StartAllListeners();
    }
}

void ScDocument::SetScenario( SCTAB nTab, bool bFlag )
{
    if (ScTable* pTable = FetchTable(nTab))
        pTable->SetScenario(bFlag);
}

bool ScDocument::IsScenario( SCTAB nTab ) const
{
    const ScTable* pTable = FetchTable(nTab);
    return pTable && pTable->IsScenario();
}

void ScDocument::SetScenarioData( SCTAB nTab, const OUString& rComment,
                                        const Color& rColor, ScScenarioFlags nFlags )
{
    if (ScTable* pTable = FetchTable(nTab); pTable && pTable->IsScenario())
    {
        pTable->SetScenarioComment( rComment );
        pTable->SetScenarioColor( rColor );
        pTable->SetScenarioFlags( nFlags );
    }
}

Color ScDocument::GetTabBgColor( SCTAB nTab ) const
{
    if (const ScTable* pTable = FetchTable(nTab))
        return pTable->GetTabBgColor();
    return COL_AUTO;
}

void ScDocument::SetTabBgColor( SCTAB nTab, const Color& rColor )
{
    if (ScTable* pTable = FetchTable(nTab))
        pTable->SetTabBgColor(rColor);
}

bool ScDocument::IsDefaultTabBgColor( SCTAB nTab ) const
{
    if (const ScTable* pTable = FetchTable(nTab))
        return pTable->GetTabBgColor() == COL_AUTO;
    return true;
}

void ScDocument::GetScenarioData( SCTAB nTab, OUString& rComment,
                                        Color& rColor, ScScenarioFlags& rFlags ) const
{
    if (const ScTable* pTable = FetchTable(nTab); pTable && pTable->IsScenario())
    {
        pTable->GetScenarioComment( rComment );
        rColor = pTable->GetScenarioColor();
        rFlags = pTable->GetScenarioFlags();
    }
}

void ScDocument::GetScenarioFlags( SCTAB nTab, ScScenarioFlags& rFlags ) const
{
    if (const ScTable* pTable = FetchTable(nTab); pTable && pTable->IsScenario())
        rFlags = pTable->GetScenarioFlags();
}

bool ScDocument::IsLinked( SCTAB nTab ) const
{
    const ScTable* pTable = FetchTable(nTab);
    return pTable && pTable->IsLinked();
}

formula::FormulaGrammar::AddressConvention ScDocument::GetAddressConvention() const
{
    return formula::FormulaGrammar::extractRefConvention(eGrammar);
}

void ScDocument::SetGrammar( formula::FormulaGrammar::Grammar eGram )
{
    eGrammar = eGram;
}

ScLinkMode ScDocument::GetLinkMode( SCTAB nTab ) const
{
    if (const ScTable* pTable = FetchTable(nTab))
        return pTable->GetLinkMode();
    return ScLinkMode::NONE;
}

OUString ScDocument::GetLinkDoc( SCTAB nTab ) const
{
    if (const ScTable* pTable = FetchTable(nTab))
        return pTable->GetLinkDoc();
    return OUString();
}

OUString ScDocument::GetLinkFlt( SCTAB nTab ) const
{
    if (const ScTable* pTable = FetchTable(nTab))
        return pTable->GetLinkFlt();
    return OUString();
}

OUString ScDocument::GetLinkOpt( SCTAB nTab ) const
{
    if (const ScTable* pTable = FetchTable(nTab))
        return pTable->GetLinkOpt();
    return OUString();
}

OUString ScDocument::GetLinkTab( SCTAB nTab ) const
{
    if (const ScTable* pTable = FetchTable(nTab))
        return pTable->GetLinkTab();
    return OUString();
}

sal_Int32 ScDocument::GetLinkRefreshDelay( SCTAB nTab ) const
{
    if (const ScTable* pTable = FetchTable(nTab))
        return pTable->GetLinkRefreshDelay();
    return 0;
}

void ScDocument::SetLink( SCTAB nTab, ScLinkMode nMode, const OUString& rDoc,
                            const OUString& rFilter, const OUString& rOptions,
                            const OUString& rTabName, sal_Int32 nRefreshDelay )
{
    if (ScTable* pTable = FetchTable(nTab))
        pTable->SetLink(nMode, rDoc, rFilter, rOptions, rTabName, nRefreshDelay);
}

bool ScDocument::HasLink( std::u16string_view rDoc,
                            std::u16string_view rFilter, std::u16string_view rOptions ) const
{
    SCTAB nCount = GetTableCount();
    for (SCTAB i=0; i<nCount; i++)
        if (maTabs[i]->IsLinked()
                && maTabs[i]->GetLinkDoc() == rDoc
                && maTabs[i]->GetLinkFlt() == rFilter
                && maTabs[i]->GetLinkOpt() == rOptions)
            return true;

    return false;
}

bool ScDocument::LinkExternalTab( SCTAB& rTab, const OUString& aDocTab,
        const OUString& aFileName, const OUString& aTabName )
{
    if ( IsClipboard() )
    {
        OSL_FAIL( "LinkExternalTab in Clipboard" );
        return false;
    }
    rTab = 0;
#if ENABLE_FUZZERS
    (void)aDocTab;
    (void)aFileName;
    (void)aTabName;
    return false;
#else
    OUString  aFilterName; // Is filled by the Loader
    OUString  aOptions; // Filter options
    sal_uInt32 nLinkCnt = pExtDocOptions ? pExtDocOptions->GetDocSettings().mnLinkCnt : 0;
    ScDocumentLoader aLoader( aFileName, aFilterName, aOptions, nLinkCnt + 1 );
    if ( aLoader.IsError() )
        return false;
    ScDocument* pSrcDoc = aLoader.GetDocument();

    // Copy table
    SCTAB nSrcTab;
    if ( pSrcDoc->GetTable( aTabName, nSrcTab ) )
    {
        if ( !InsertTab( SC_TAB_APPEND, aDocTab, true ) )
        {
            OSL_FAIL("can't insert external document table");
            return false;
        }
        rTab = GetTableCount() - 1;
        // Don't insert anew, just the results
        TransferTab( *pSrcDoc, nSrcTab, rTab, false, true );
    }
    else
        return false;

    sal_Int32 nRefreshDelay = 0;

    bool bWasThere = HasLink( aFileName, aFilterName, aOptions );
    SetLink( rTab, ScLinkMode::VALUE, aFileName, aFilterName, aOptions, aTabName, nRefreshDelay );
    if ( !bWasThere ) // Add link only once per source document
    {
        ScTableLink* pLink = new ScTableLink( mpShell, aFileName, aFilterName, aOptions, nRefreshDelay );
        pLink->SetInCreate( true );
        OUString aFilName = aFilterName;
        GetLinkManager()->InsertFileLink( *pLink, sfx2::SvBaseLinkObjectType::ClientFile, aFileName, &aFilName );
        pLink->Update();
        pLink->SetInCreate( false );
        SfxBindings* pBindings = GetViewBindings();
        if (pBindings)
            pBindings->Invalidate( SID_LINKS );
    }
    return true;
#endif
}

ScExternalRefManager* ScDocument::GetExternalRefManager() const
{
    ScDocument* pThis = const_cast<ScDocument*>(this);
    if (!pExternalRefMgr)
        pThis->pExternalRefMgr.reset( new ScExternalRefManager(*pThis));

    return pExternalRefMgr.get();
}

bool ScDocument::IsInExternalReferenceMarking() const
{
    return pExternalRefMgr && pExternalRefMgr->isInReferenceMarking();
}

void ScDocument::MarkUsedExternalReferences()
{
    if (!pExternalRefMgr)
        return;
    if (!pExternalRefMgr->hasExternalData())
        return;
    // Charts.
    pExternalRefMgr->markUsedByLinkListeners();
    // Formula cells.
    pExternalRefMgr->markUsedExternalRefCells();

    /* NOTE: Conditional formats and validation objects are marked when
     * collecting them during export. */
}

ScFormulaParserPool& ScDocument::GetFormulaParserPool() const
{
    if (!mxFormulaParserPool)
        mxFormulaParserPool.reset( new ScFormulaParserPool( *this ) );
    return *mxFormulaParserPool;
}

const ScSheetEvents* ScDocument::GetSheetEvents( SCTAB nTab ) const
{
    if (const ScTable* pTable = FetchTable(nTab))
        return pTable->GetSheetEvents();
    return nullptr;
}

void ScDocument::SetSheetEvents( SCTAB nTab, std::unique_ptr<ScSheetEvents> pNew )
{
    if (ScTable* pTable = FetchTable(nTab))
        pTable->SetSheetEvents( std::move(pNew) );
}

bool ScDocument::HasSheetEventScript( SCTAB nTab, ScSheetEventId nEvent, bool bWithVbaEvents ) const
{
    if (const ScTable* pTable = FetchTable(nTab))
    {
        // check if any event handler script has been configured
        const ScSheetEvents* pEvents = pTable->GetSheetEvents();
        if ( pEvents && pEvents->GetScript( nEvent ) )
            return true;
        // check if VBA event handlers exist
        if (bWithVbaEvents && mxVbaEvents.is()) try
        {
            uno::Sequence< uno::Any > aArgs{ uno::Any(nTab) };
            if (mxVbaEvents->hasVbaEventHandler( ScSheetEvents::GetVbaSheetEventId( nEvent ), aArgs ) ||
                mxVbaEvents->hasVbaEventHandler( ScSheetEvents::GetVbaDocumentEventId( nEvent ), uno::Sequence< uno::Any >() ))
                return true;
        }
        catch( uno::Exception& )
        {
        }
    }
    return false;
}

bool ScDocument::HasAnySheetEventScript( ScSheetEventId nEvent, bool bWithVbaEvents ) const
{
    SCTAB nSize = GetTableCount();
    for (SCTAB nTab = 0; nTab < nSize; nTab++)
        if (HasSheetEventScript( nTab, nEvent, bWithVbaEvents ))
            return true;
    return false;
}

bool ScDocument::HasAnyCalcNotification() const
{
    SCTAB nSize = GetTableCount();
    for (SCTAB nTab = 0; nTab < nSize; nTab++)
        if (maTabs[nTab] && maTabs[nTab]->GetCalcNotification())
            return true;
    return false;
}

bool ScDocument::HasCalcNotification( SCTAB nTab ) const
{
    if (const ScTable* pTable = FetchTable(nTab))
        return pTable->GetCalcNotification();
    return false;
}

void ScDocument::SetCalcNotification( SCTAB nTab )
{
    // set only if not set before
    if (ScTable* pTable = FetchTable(nTab) ; pTable && !pTable->GetCalcNotification())
        pTable->SetCalcNotification(true);
}

void ScDocument::ResetCalcNotifications()
{
    SCTAB nSize = GetTableCount();
    for (SCTAB nTab = 0; nTab < nSize; nTab++)
        if (maTabs[nTab] && maTabs[nTab]->GetCalcNotification())
            maTabs[nTab]->SetCalcNotification(false);
}

ScOutlineTable* ScDocument::GetOutlineTable( SCTAB nTab, bool bCreate )
{
    ScOutlineTable* pVal = nullptr;

    if (ScTable* pTable = FetchTable(nTab))
    {
        pVal = pTable->GetOutlineTable();
        if (!pVal && bCreate)
        {
            pTable->StartOutlineTable();
            pVal = pTable->GetOutlineTable();
        }
    }

    return pVal;
}

bool ScDocument::SetOutlineTable( SCTAB nTab, const ScOutlineTable* pNewOutline )
{
    ScTable* pTable = FetchTable(nTab);
    return pTable && pTable->SetOutlineTable(pNewOutline);
}

void ScDocument::DoAutoOutline( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow, SCTAB nTab )
{
    if (ScTable* pTable = FetchTable(nTab))
        pTable->DoAutoOutline( nStartCol, nStartRow, nEndCol, nEndRow );
}

bool ScDocument::TestRemoveSubTotals( SCTAB nTab, const ScSubTotalParam& rParam )
{
    ScTable* pTable = FetchTable(nTab);
    return pTable && pTable->TestRemoveSubTotals(rParam);
}

void ScDocument::RemoveSubTotals( SCTAB nTab, ScSubTotalParam& rParam )
{
    if (ScTable* pTable = FetchTable(nTab))
        pTable->RemoveSubTotals( rParam );
}

bool ScDocument::DoSubTotals( SCTAB nTab, ScSubTotalParam& rParam )
{
    ScTable* pTable = FetchTable(nTab);
    return pTable && pTable->DoSubTotals(rParam);
}

bool ScDocument::HasSubTotalCells( const ScRange& rRange )
{
    ScCellIterator aIter(*this, rRange);
    for (bool bHas = aIter.first(); bHas; bHas = aIter.next())
    {
        if (aIter.getType() != CELLTYPE_FORMULA)
            continue;

        if (aIter.getFormulaCell()->IsSubTotal())
            return true;
    }
    return false;   // none found
}

/**
 * From this document this method copies the cells of positions at which
 * there are also cells in pPosDoc to pDestDoc
 */
void ScDocument::CopyUpdated( ScDocument* pPosDoc, ScDocument* pDestDoc )
{
    SCTAB nCount = GetTableCount();
    for (SCTAB nTab=0; nTab<nCount; nTab++)
        if (maTabs[nTab] && pPosDoc->maTabs[nTab] && pDestDoc->maTabs[nTab])
            maTabs[nTab]->CopyUpdated( pPosDoc->maTabs[nTab].get(), pDestDoc->maTabs[nTab].get() );
}

void ScDocument::CopyScenario( SCTAB nSrcTab, SCTAB nDestTab, bool bNewScenario )
{
    if (!HasTable(nSrcTab) || !HasTable(nDestTab))
        return;

    // Set flags correctly for active scenarios
    // and write current values back to recently active scenarios
    ScRangeList aRanges = *maTabs[nSrcTab]->GetScenarioRanges();

    // nDestTab is the target table
    for ( SCTAB nTab = nDestTab+1;
            nTab < GetTableCount() && maTabs[nTab] && maTabs[nTab]->IsScenario();
            nTab++ )
    {
        if ( maTabs[nTab]->IsActiveScenario() ) // Even if it's the same scenario
        {
            bool bTouched = false;
            for ( size_t nR=0, nRangeCount = aRanges.size(); nR < nRangeCount && !bTouched; nR++ )
            {
                const ScRange& rRange = aRanges[ nR ];
                if ( maTabs[nTab]->HasScenarioRange( rRange ) )
                    bTouched = true;
            }
            if (bTouched)
            {
                maTabs[nTab]->SetActiveScenario(false);
                if ( maTabs[nTab]->GetScenarioFlags() & ScScenarioFlags::TwoWay )
                    maTabs[nTab]->CopyScenarioFrom( maTabs[nDestTab].get() );
            }
        }
    }

    maTabs[nSrcTab]->SetActiveScenario(true); // This is where it's from ...
    if (!bNewScenario) // Copy data from the selected scenario
    {
        sc::AutoCalcSwitch aACSwitch(*this, false);
        maTabs[nSrcTab]->CopyScenarioTo( maTabs[nDestTab].get() );

        sc::SetFormulaDirtyContext aCxt;
        SetAllFormulasDirty(aCxt);
    }
}

void ScDocument::MarkScenario( SCTAB nSrcTab, SCTAB nDestTab, ScMarkData& rDestMark,
                                bool bResetMark, ScScenarioFlags nNeededBits ) const
{
    if (bResetMark)
        rDestMark.ResetMark();

    if (const ScTable* pTable = FetchTable(nSrcTab))
        pTable->MarkScenarioIn(rDestMark, nNeededBits);

    rDestMark.SetAreaTab( nDestTab);
}

bool ScDocument::HasScenarioRange( SCTAB nTab, const ScRange& rRange ) const
{
    const ScTable* pTable = FetchTable(nTab);
    return pTable && pTable->HasScenarioRange(rRange);
}

const ScRangeList* ScDocument::GetScenarioRanges( SCTAB nTab ) const
{
    if (const ScTable* pTable = FetchTable(nTab))
        return pTable->GetScenarioRanges();

    return nullptr;
}

bool ScDocument::IsActiveScenario( SCTAB nTab ) const
{
    const ScTable* pTable = FetchTable(nTab);
    return pTable && pTable->IsActiveScenario();
}

void ScDocument::SetActiveScenario( SCTAB nTab, bool bActive )
{
    if (ScTable* pTable = FetchTable(nTab))
        pTable->SetActiveScenario( bActive );
}

bool ScDocument::TestCopyScenario( SCTAB nSrcTab, SCTAB nDestTab ) const
{
    if (HasTable(nSrcTab) && HasTable(nDestTab))
        return maTabs[nSrcTab]->TestCopyScenarioTo(maTabs[nDestTab].get());

    OSL_FAIL("wrong table at TestCopyScenario");
    return false;
}

void ScDocument::AddUnoObject( SfxListener& rObject )
{
    if (!pUnoBroadcaster)
        pUnoBroadcaster.reset( new SfxBroadcaster );

    rObject.StartListening( *pUnoBroadcaster );
}

void ScDocument::RemoveUnoObject( SfxListener& rObject )
{
    if (pUnoBroadcaster)
    {
        rObject.EndListening( *pUnoBroadcaster );

        if ( bInUnoBroadcast )
        {
            // Broadcasts from ScDocument::BroadcastUno are the only way that
            // uno object methods are called without holding a reference.
            //
            // If RemoveUnoObject is called from an object dtor in the finalizer thread
            // while the main thread is calling BroadcastUno, the dtor thread must wait
            // (or the object's Notify might try to access a deleted object).
            // The SolarMutex can't be locked here because if a component is called from
            // a VCL event, the main thread has the SolarMutex locked all the time.
            //
            // This check is done after calling EndListening, so a later BroadcastUno call
            // won't touch this object.

            vcl::SolarMutexTryAndBuyGuard g;
            if (g.isAcquired())
            {
                // BroadcastUno is always called with the SolarMutex locked, so if it
                // can be acquired, this is within the same thread (should not happen)
                OSL_FAIL( "RemoveUnoObject called from BroadcastUno" );
            }
            else
            {
                // Let the thread that called BroadcastUno continue
                while ( bInUnoBroadcast )
                {
                    osl::Thread::yield();
                }
            }
        }
    }
    else
    {
        OSL_FAIL("No Uno broadcaster");
    }
}

void ScDocument::BroadcastUno( const SfxHint &rHint )
{
    if (!pUnoBroadcaster)
        return;

    bInUnoBroadcast = true;
    pUnoBroadcaster->Broadcast( rHint );
    bInUnoBroadcast = false;

    // During Broadcast notification, Uno objects can add to pUnoListenerCalls.
    // The listener calls must be processed after completing the broadcast,
    // because they can add or remove objects from pUnoBroadcaster.

    if ( pUnoListenerCalls &&
            rHint.GetId() == SfxHintId::DataChanged &&
            !bInUnoListenerCall )
    {
        // Listener calls may lead to BroadcastUno calls again. The listener calls
        // are not nested, instead the calls are collected in the list, and the
        // outermost call executes them all.

        ScChartLockGuard aChartLockGuard(this);
        bInUnoListenerCall = true;
        pUnoListenerCalls->ExecuteAndClear();
        bInUnoListenerCall = false;
    }
}

void ScDocument::AddUnoListenerCall( const uno::Reference<util::XModifyListener>& rListener,
                                        const lang::EventObject& rEvent )
{
    OSL_ENSURE( bInUnoBroadcast, "AddUnoListenerCall is supposed to be called from BroadcastUno only" );

    if ( !pUnoListenerCalls )
        pUnoListenerCalls.reset( new ScUnoListenerCalls );
    pUnoListenerCalls->Add( rListener, rEvent );
}

void ScDocument::BeginUnoRefUndo()
{
    OSL_ENSURE( !pUnoRefUndoList, "BeginUnoRefUndo twice" );
    pUnoRefUndoList.reset( new ScUnoRefList );
}

std::unique_ptr<ScUnoRefList> ScDocument::EndUnoRefUndo()
{
    return std::move(pUnoRefUndoList);
    // Must be deleted by caller!
}

void ScDocument::AddUnoRefChange( sal_Int64 nId, const ScRangeList& rOldRanges )
{
    if ( pUnoRefUndoList )
        pUnoRefUndoList->Add( nId, rOldRanges );
}

void ScDocument::UpdateReference(
    sc::RefUpdateContext& rCxt, ScDocument* pUndoDoc, bool bIncludeDraw, bool bUpdateNoteCaptionPos )
{
    if (!ValidRange(rCxt.maRange) && !(rCxt.meMode == URM_INSDEL &&
                ((rCxt.mnColDelta < 0 &&    // convention from ScDocument::DeleteCol()
                  rCxt.maRange.aStart.Col() == GetMaxColCount() && rCxt.maRange.aEnd.Col() == GetMaxColCount()) ||
                 (rCxt.mnRowDelta < 0 &&    // convention from ScDocument::DeleteRow()
                  rCxt.maRange.aStart.Row() == GetMaxRowCount() && rCxt.maRange.aEnd.Row() == GetMaxRowCount()))))
        return;

    std::unique_ptr<sc::ExpandRefsSwitch> pExpandRefsSwitch;
    if (rCxt.isInserted())
        pExpandRefsSwitch.reset(new sc::ExpandRefsSwitch(*this, SC_MOD()->GetInputOptions().GetExpandRefs()));

    size_t nFirstTab, nLastTab;
    if (rCxt.meMode == URM_COPY)
    {
        nFirstTab = rCxt.maRange.aStart.Tab();
        nLastTab = rCxt.maRange.aEnd.Tab();
    }
    else
    {
        // TODO: Have these methods use the context object directly.
        ScRange aRange = rCxt.maRange;
        UpdateRefMode eUpdateRefMode = rCxt.meMode;
        SCCOL nDx = rCxt.mnColDelta;
        SCROW nDy = rCxt.mnRowDelta;
        SCTAB nDz = rCxt.mnTabDelta;
        SCCOL nCol1 = rCxt.maRange.aStart.Col(), nCol2 = rCxt.maRange.aEnd.Col();
        SCROW nRow1 = rCxt.maRange.aStart.Row(), nRow2 = rCxt.maRange.aEnd.Row();
        SCTAB nTab1 = rCxt.maRange.aStart.Tab(), nTab2 = rCxt.maRange.aEnd.Tab();

        xColNameRanges->UpdateReference( eUpdateRefMode, this, aRange, nDx, nDy, nDz );
        xRowNameRanges->UpdateReference( eUpdateRefMode, this, aRange, nDx, nDy, nDz );
        pDBCollection->UpdateReference( eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, nDx, nDy, nDz );
        if (pRangeName)
            pRangeName->UpdateReference(rCxt);
        if ( pDPCollection )
            pDPCollection->UpdateReference( eUpdateRefMode, aRange, nDx, nDy, nDz );
        UpdateChartRef( eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, nDx, nDy, nDz );
        UpdateRefAreaLinks( eUpdateRefMode, aRange, nDx, nDy, nDz );
        if ( pValidationList )
        {
            ScMutationGuard aGuard(*this, ScMutationGuardFlags::CORE);
            pValidationList->UpdateReference(rCxt);
        }
        if ( pDetOpList )
            pDetOpList->UpdateReference( this, eUpdateRefMode, aRange, nDx, nDy, nDz );
        if ( pUnoBroadcaster )
            pUnoBroadcaster->Broadcast( ScUpdateRefHint(
                                eUpdateRefMode, aRange, nDx, nDy, nDz ) );

        nFirstTab = 0;
        nLastTab = maTabs.size()-1;
    }

    for (size_t i = nFirstTab, n = maTabs.size() ; i <= nLastTab && i < n; ++i)
    {
        if (!maTabs[i])
            continue;

        maTabs[i]->UpdateReference(rCxt, pUndoDoc, bIncludeDraw, bUpdateNoteCaptionPos);
    }

    if ( bIsEmbedded )
    {
        SCCOL theCol1;
        SCROW theRow1;
        SCTAB theTab1;
        SCCOL theCol2;
        SCROW theRow2;
        SCTAB theTab2;
        theCol1 = aEmbedRange.aStart.Col();
        theRow1 = aEmbedRange.aStart.Row();
        theTab1 = aEmbedRange.aStart.Tab();
        theCol2 = aEmbedRange.aEnd.Col();
        theRow2 = aEmbedRange.aEnd.Row();
        theTab2 = aEmbedRange.aEnd.Tab();

        // TODO: Have ScRefUpdate::Update() use the context object directly.
        UpdateRefMode eUpdateRefMode = rCxt.meMode;
        SCCOL nDx = rCxt.mnColDelta;
        SCROW nDy = rCxt.mnRowDelta;
        SCTAB nDz = rCxt.mnTabDelta;
        SCCOL nCol1 = rCxt.maRange.aStart.Col(), nCol2 = rCxt.maRange.aEnd.Col();
        SCROW nRow1 = rCxt.maRange.aStart.Row(), nRow2 = rCxt.maRange.aEnd.Row();
        SCTAB nTab1 = rCxt.maRange.aStart.Tab(), nTab2 = rCxt.maRange.aEnd.Tab();

        if ( ScRefUpdate::Update( this, eUpdateRefMode, nCol1,nRow1,nTab1, nCol2,nRow2,nTab2,
                                    nDx,nDy,nDz, theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 ) )
        {
            aEmbedRange = ScRange( theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 );
        }
    }

    // After moving, no clipboard move ref-updates are possible
    if (rCxt.meMode != URM_COPY && IsClipboardSource())
    {
        ScDocument* pClipDoc = ScModule::GetClipDoc();
        if (pClipDoc)
            pClipDoc->GetClipParam().mbCutMode = false;
    }
}

void ScDocument::UpdateTranspose( const ScAddress& rDestPos, ScDocument* pClipDoc,
                                        const ScMarkData& rMark, ScDocument* pUndoDoc )
{
    OSL_ENSURE(pClipDoc->bIsClip, "UpdateTranspose: No Clip");

    ScRange aSource;
    ScClipParam& rClipParam = pClipDoc->GetClipParam();
    if (!rClipParam.maRanges.empty())
        aSource = rClipParam.maRanges.front();
    ScAddress aDest = rDestPos;

    SCTAB nClipTab = 0;
    for (SCTAB nDestTab = 0; nDestTab < GetTableCount() && maTabs[nDestTab]; nDestTab++)
        if (rMark.GetTableSelect(nDestTab))
        {
            while (!pClipDoc->maTabs[nClipTab]) nClipTab = (nClipTab+1) % (MAXTAB+1);
            aSource.aStart.SetTab( nClipTab );
            aSource.aEnd.SetTab( nClipTab );
            aDest.SetTab( nDestTab );

            // Like UpdateReference
            if (pRangeName)
                pRangeName->UpdateTranspose( aSource, aDest ); // Before the cells!
            for (SCTAB i = 0; i < GetTableCount(); i++)
                if (maTabs[i])
                    maTabs[i]->UpdateTranspose( aSource, aDest, pUndoDoc );

            nClipTab = (nClipTab+1) % (MAXTAB+1);
        }
}

void ScDocument::UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY )
{
    //TODO: pDBCollection
    //TODO: pPivotCollection
    //TODO: UpdateChartRef

    if (pRangeName)
        pRangeName->UpdateGrow( rArea, nGrowX, nGrowY );

    for (SCTAB i = 0; i < GetTableCount() && maTabs[i]; i++)
        maTabs[i]->UpdateGrow( rArea, nGrowX, nGrowY );
}

void ScDocument::Fill(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScProgress* pProgress, const ScMarkData& rMark,
                        sal_uInt64 nFillCount, FillDir eFillDir, FillCmd eFillCmd, FillDateCmd eFillDateCmd,
                        double nStepValue, double nMaxValue)
{
    PutInOrder( nCol1, nCol2 );
    PutInOrder( nRow1, nRow2 );
    const ScRange& aRange = rMark.GetMarkArea();
    SCTAB nMax = maTabs.size();
    for (const auto& rTab : rMark)
    {
        if (rTab >= nMax)
            break;
        if (maTabs[rTab])
        {
            maTabs[rTab]->Fill(nCol1, nRow1, nCol2, nRow2,
                            nFillCount, eFillDir, eFillCmd, eFillDateCmd,
                            nStepValue, tools::Duration(), nMaxValue, pProgress);
            RefreshAutoFilter(aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Col(), aRange.aEnd.Row(), rTab);
        }
    }
}

OUString ScDocument::GetAutoFillPreview( const ScRange& rSource, SCCOL nEndX, SCROW nEndY )
{
    SCTAB nTab = rSource.aStart.Tab();
    if (nTab < GetTableCount() && maTabs[nTab])
        return maTabs[nTab]->GetAutoFillPreview( rSource, nEndX, nEndY );

    return OUString();
}

void ScDocument::AutoFormat( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                    sal_uInt16 nFormatNo, const ScMarkData& rMark )
{
    PutInOrder( nStartCol, nEndCol );
    PutInOrder( nStartRow, nEndRow );
    SCTAB nMax = maTabs.size();
    for (const auto& rTab : rMark)
    {
        if (rTab >= nMax)
            break;
        if (maTabs[rTab])
            maTabs[rTab]->AutoFormat( nStartCol, nStartRow, nEndCol, nEndRow, nFormatNo );
    }
}

void ScDocument::GetAutoFormatData(SCTAB nTab, SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                    ScAutoFormatData& rData)
{
    if (ScTable* pTable = FetchTable(nTab))
    {
        PutInOrder(nStartCol, nEndCol);
        PutInOrder(nStartRow, nEndRow);
        pTable->GetAutoFormatData(nStartCol, nStartRow, nEndCol, nEndRow, rData);
    }
}

void ScDocument::GetSearchAndReplaceStart( const SvxSearchItem& rSearchItem,
        SCCOL& rCol, SCROW& rRow )
{
    SvxSearchCmd nCommand = rSearchItem.GetCommand();
    bool bReplace = ( nCommand == SvxSearchCmd::REPLACE ||
        nCommand == SvxSearchCmd::REPLACE_ALL );
    if ( rSearchItem.GetBackward() )
    {
        if ( rSearchItem.GetRowDirection() )
        {
            if ( rSearchItem.GetPattern() )
            {
                rCol = MaxCol();
                rRow = MaxRow()+1;
            }
            else if ( bReplace )
            {
                rCol = MaxCol();
                rRow = MaxRow();
            }
            else
            {
                rCol = MaxCol()+1;
                rRow = MaxRow();
            }
        }
        else
        {
            if ( rSearchItem.GetPattern() )
            {
                rCol = MaxCol()+1;
                rRow = MaxRow();
            }
            else if ( bReplace )
            {
                rCol = MaxCol();
                rRow = MaxRow();
            }
            else
            {
                rCol = MaxCol();
                rRow = MaxRow()+1;
            }
        }
    }
    else
    {
        if ( rSearchItem.GetRowDirection() )
        {
            if ( rSearchItem.GetPattern() )
            {
                rCol = 0;
                rRow = SCROW(-1);
            }
            else if ( bReplace )
            {
                rCol = 0;
                rRow = 0;
            }
            else
            {
                rCol = SCCOL(-1);
                rRow = 0;
            }
        }
        else
        {
            if ( rSearchItem.GetPattern() )
            {
                rCol = SCCOL(-1);
                rRow = 0;
            }
            else if ( bReplace )
            {
                rCol = 0;
                rRow = 0;
            }
            else
            {
                rCol = 0;
                rRow = SCROW(-1);
            }
        }
    }
}

// static
bool ScDocument::IsEmptyCellSearch( const SvxSearchItem& rSearchItem )
{
    return !rSearchItem.GetPattern() && (rSearchItem.GetCellType() != SvxSearchCellType::NOTE)
        && (rSearchItem.GetSearchOptions().searchString.isEmpty()
                || (rSearchItem.GetRegExp() && rSearchItem.GetSearchOptions().searchString == "^$"));
}

bool ScDocument::SearchAndReplace(
    const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow, SCTAB& rTab,
    const ScMarkData& rMark, ScRangeList& rMatchedRanges,
    OUString& rUndoStr, ScDocument* pUndoDoc, bool& bMatchedRangesWereClamped)
{
    // FIXME: Manage separated marks per table!
    bool bFound = false;
    if (rTab >= GetTableCount())
        OSL_FAIL("table out of range");
    if (ValidTab(rTab))
    {
        SCCOL nCol;
        SCROW nRow;
        SCTAB nTab;
        SvxSearchCmd nCommand = rSearchItem.GetCommand();
        if ( nCommand == SvxSearchCmd::FIND_ALL ||
             nCommand == SvxSearchCmd::REPLACE_ALL )
        {
            SCTAB nMax = maTabs.size();
            for (const auto& rMarkedTab : rMark)
            {
                if (rMarkedTab >= nMax)
                    break;
                if (maTabs[rMarkedTab])
                {
                    nCol = 0;
                    nRow = 0;
                    bFound |= maTabs[rMarkedTab]->SearchAndReplace(
                        rSearchItem, nCol, nRow, rMark, rMatchedRanges, rUndoStr, pUndoDoc, bMatchedRangesWereClamped);
                }
            }

            // Mark is set completely inside already
        }
        else
        {
            nCol = rCol;
            nRow = rRow;
            if (rSearchItem.GetBackward())
            {
                for (nTab = rTab; (nTab >= 0) && !bFound; nTab--)
                    if (maTabs[nTab])
                    {
                        if (rMark.GetTableSelect(nTab))
                        {
                            bFound = maTabs[nTab]->SearchAndReplace(
                                rSearchItem, nCol, nRow, rMark, rMatchedRanges, rUndoStr, pUndoDoc, bMatchedRangesWereClamped);
                            if (bFound)
                            {
                                rCol = nCol;
                                rRow = nRow;
                                rTab = nTab;
                            }
                            else
                            {
                                ScDocument::GetSearchAndReplaceStart(
                                    rSearchItem, nCol, nRow );

                                // notify LibreOfficeKit about changed page
                                if (comphelper::LibreOfficeKit::isActive())
                                {
                                    OString aPayload = OString::number(nTab);
                                    if (SfxViewShell* pViewShell = SfxViewShell::Current())
                                        pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_SET_PART, aPayload);
                                }
                            }
                        }
                    }
            }
            else
            {
                for (nTab = rTab; (nTab < GetTableCount()) && !bFound; nTab++)
                    if (maTabs[nTab])
                    {
                        if (rMark.GetTableSelect(nTab))
                        {
                            bFound = maTabs[nTab]->SearchAndReplace(
                                rSearchItem, nCol, nRow, rMark, rMatchedRanges, rUndoStr, pUndoDoc, bMatchedRangesWereClamped);
                            if (bFound)
                            {
                                rCol = nCol;
                                rRow = nRow;
                                rTab = nTab;
                            }
                            else
                            {
                                ScDocument::GetSearchAndReplaceStart(
                                    rSearchItem, nCol, nRow );

                                // notify LibreOfficeKit about changed page
                                if (comphelper::LibreOfficeKit::isActive())
                                {
                                    OString aPayload = OString::number(nTab);
                                    if(SfxViewShell* pViewShell = SfxViewShell::Current())
                                        pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_SET_PART, aPayload);
                                }
                            }
                        }
                    }
            }
        }
    }
    return bFound;
}

/**
 * Adapt Outline
 */
bool ScDocument::UpdateOutlineCol( SCCOL nStartCol, SCCOL nEndCol, SCTAB nTab, bool bShow )
{
    if (ScTable* pTable = FetchTable(nTab))
        return pTable->UpdateOutlineCol(nStartCol, nEndCol, bShow);

    OSL_FAIL("missing tab");
    return false;
}

bool ScDocument::UpdateOutlineRow( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, bool bShow )
{
    if (ScTable* pTable = FetchTable(nTab))
        return pTable->UpdateOutlineRow(nStartRow, nEndRow, bShow);

    OSL_FAIL("missing tab");
    return false;
}

void ScDocument::Sort(
    SCTAB nTab, const ScSortParam& rSortParam, bool bKeepQuery, bool bUpdateRefs,
    ScProgress* pProgress, sc::ReorderParam* pUndo )
{
    if (ScTable* pTable = FetchTable(nTab))
    {
        bool bOldEnableIdle = IsIdleEnabled();
        EnableIdle(false);
        pTable->Sort(rSortParam, bKeepQuery, bUpdateRefs, pProgress, pUndo);
        EnableIdle(bOldEnableIdle);
    }
}

void ScDocument::Reorder( const sc::ReorderParam& rParam )
{
    ScTable* pTable = FetchTable(rParam.maSortRange.aStart.Tab());
    if (!pTable)
        return;

    bool bOldEnableIdle = IsIdleEnabled();
    EnableIdle(false);
    pTable->Reorder(rParam);
    EnableIdle(bOldEnableIdle);
}

void ScDocument::PrepareQuery( SCTAB nTab, ScQueryParam& rQueryParam )
{
    if (ScTable* pTable = FetchTable(nTab))
        pTable->PrepareQuery(rQueryParam);
    else
    {
        OSL_FAIL("missing tab");
    }
}

SCSIZE ScDocument::Query(SCTAB nTab, const ScQueryParam& rQueryParam, bool bKeepSub)
{
    if (ScTable* pTable = FetchTable(nTab))
        return pTable->Query(rQueryParam, bKeepSub);

    OSL_FAIL("missing tab");
    return 0;
}

OUString ScDocument::GetUpperCellString(SCCOL nCol, SCROW nRow, SCTAB nTab)
{
    if (ScTable* pTable = FetchTable(nTab))
        return pTable->GetUpperCellString( nCol, nRow );
    else
        return OUString();
}

bool ScDocument::CreateQueryParam( const ScRange& rRange, ScQueryParam& rQueryParam )
{
    if (ScTable* pTable = FetchTable(rRange.aStart.Tab()))
        return pTable->CreateQueryParam(rRange.aStart.Col(), rRange.aStart.Row(), rRange.aEnd.Col(), rRange.aEnd.Row(), rQueryParam);

    OSL_FAIL("missing tab");
    return false;
}

bool ScDocument::HasAutoFilter( SCCOL nCurCol, SCROW nCurRow, SCTAB nCurTab )
{
    const ScDBData* pDBData = GetDBAtCursor( nCurCol, nCurRow, nCurTab, ScDBDataPortion::AREA );
    bool bHasAutoFilter = (pDBData != nullptr);

    if ( pDBData )
    {
        if ( pDBData->HasHeader() )
        {
            SCCOL nCol;
            SCROW nRow;
            ScMF  nFlag;

            ScQueryParam aParam;
            pDBData->GetQueryParam( aParam );
            nRow = aParam.nRow1;

            for ( nCol=aParam.nCol1; nCol<=aParam.nCol2 && bHasAutoFilter; nCol++ )
            {
                nFlag = GetAttr( nCol, nRow, nCurTab, ATTR_MERGE_FLAG )->GetValue();

                if ( !(nFlag & ScMF::Auto) )
                    bHasAutoFilter = false;
            }
        }
        else
            bHasAutoFilter = false;
    }

    return bHasAutoFilter;
}

bool ScDocument::HasColHeader( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                    SCTAB nTab )
{
    ScTable* pTable = FetchTable(nTab);
    return pTable && pTable->HasColHeader(nStartCol, nStartRow, nEndCol, nEndRow);
}

bool ScDocument::HasRowHeader( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                    SCTAB nTab )
{
    ScTable* pTable = FetchTable(nTab);
    return pTable && pTable->HasRowHeader(nStartCol, nStartRow, nEndCol, nEndRow);
}

void ScDocument::GetFilterSelCount( SCCOL nCol, SCROW nRow, SCTAB nTab, SCSIZE& nSelected, SCSIZE& nTotal )
{
    nSelected = 0;
    nTotal = 0;

    if (HasTable(nTab))
    {
        ScDBData* pDBData = GetDBAtCursor( nCol, nRow, nTab, ScDBDataPortion::AREA );
        if( pDBData && pDBData->HasAutoFilter() )
            pDBData->GetFilterSelCount( nSelected, nTotal );
    }
}

/**
 * Entries for AutoFilter listbox
 */
void ScDocument::GetFilterEntries(
    SCCOL nCol, SCROW nRow, SCTAB nTab, ScFilterEntries& rFilterEntries )
{
    if (!HasTable(nTab) || !pDBCollection)
        return;

    ScDBData* pDBData = pDBCollection->GetDBAtCursor(nCol, nRow, nTab, ScDBDataPortion::AREA);  //!??
    if (!pDBData)
        return;

    pDBData->ExtendDataArea(*this);
    SCTAB nAreaTab;
    SCCOL nStartCol;
    SCROW nStartRow;
    SCCOL nEndCol;
    SCROW nEndRow;
    pDBData->GetArea( nAreaTab, nStartCol, nStartRow, nEndCol, nEndRow );

    if (pDBData->HasHeader())
        ++nStartRow;

    ScQueryParam aParam;
    pDBData->GetQueryParam( aParam );

    // Return all filter entries, if a filter condition is connected with a boolean OR
    bool bFilter = true;
    SCSIZE nEntryCount = aParam.GetEntryCount();
    for ( SCSIZE i = 0; i < nEntryCount && aParam.GetEntry(i).bDoQuery; ++i )
    {
        ScQueryEntry& rEntry = aParam.GetEntry(i);
        if ( rEntry.eConnect != SC_AND )
        {
            bFilter = false;
            break;
        }
    }

    if ( bFilter )
    {
        maTabs[nTab]->GetFilteredFilterEntries( nCol, nStartRow, nEndRow, aParam, rFilterEntries, bFilter );
    }
    else
    {
        maTabs[nTab]->GetFilterEntries( nCol, nStartRow, nEndRow, rFilterEntries );
    }

    sortAndRemoveDuplicates( rFilterEntries.maStrData, aParam.bCaseSens);
}

/**
 * Entries for Filter dialog
 */
void ScDocument::GetFilterEntriesArea(
    SCCOL nCol, SCROW nStartRow, SCROW nEndRow, SCTAB nTab, bool bCaseSens,
    ScFilterEntries& rFilterEntries )
{
    if (ScTable* pTable = FetchTable(nTab))
    {
        pTable->GetFilterEntries(nCol, nStartRow, nEndRow, rFilterEntries, true);
        sortAndRemoveDuplicates(rFilterEntries.maStrData, bCaseSens);
    }
}

/**
 * Entries for selection list listbox (no numbers/formulas)
 */
void ScDocument::GetDataEntries(
    SCCOL nCol, SCROW nRow, SCTAB nTab,
    std::vector<ScTypedStrData>& rStrings, bool bValidation )
{
    if( bValidation )
    {
        /*  Try to generate the list from list validation. This part is skipped,
            if bValidation==false, because in that case this function is called to get
            cell values for auto completion on input. */
        sal_uInt32 nValidation = GetAttr( nCol, nRow, nTab, ATTR_VALIDDATA )->GetValue();
        if( nValidation )
        {
            const ScValidationData* pData = GetValidationEntry( nValidation );
            if( pData && pData->FillSelectionList( rStrings, ScAddress( nCol, nRow, nTab ) ) )
            {
                if (pData->GetListType() == css::sheet::TableValidationVisibility::SORTEDASCENDING)
                    sortAndRemoveDuplicates(rStrings, true/*bCaseSens*/);

                return;
            }
        }
    }

    if (!HasTable(nTab))
        return;

    std::set<ScTypedStrData> aStrings;
    if (maTabs[nTab]->GetDataEntries(nCol, nRow, aStrings))
    {
        rStrings.insert(rStrings.end(), aStrings.begin(), aStrings.end());
        sortAndRemoveDuplicates(rStrings, true/*bCaseSens*/);
    }
}

/**
 * Entries for Formula auto input
 */
void ScDocument::GetFormulaEntries( ScTypedCaseStrSet& rStrings )
{

    // Range name
    if ( pRangeName )
    {
        for (const auto& rEntry : *pRangeName)
            rStrings.insert(ScTypedStrData(rEntry.second->GetName(), 0.0, 0.0, ScTypedStrData::Name));
    }

    // Database collection
    if ( pDBCollection )
    {
        const ScDBCollection::NamedDBs& rDBs = pDBCollection->getNamedDBs();
        for (const auto& rxDB : rDBs)
            rStrings.insert(ScTypedStrData(rxDB->GetName(), 0.0, 0.0, ScTypedStrData::DbName));
    }

    // Content of name ranges
    ScRangePairList* pLists[2];
    pLists[0] = GetColNameRanges();
    pLists[1] = GetRowNameRanges();
    for (ScRangePairList* pList : pLists)
    {
        if (!pList)
            continue;

        for ( size_t i = 0, nPairs = pList->size(); i < nPairs; ++i )
        {
            const ScRangePair & rPair = (*pList)[i];
            const ScRange & rRange = rPair.GetRange(0);
            ScCellIterator aIter( *this, rRange );
            for (bool bHas = aIter.first(); bHas; bHas = aIter.next())
            {
                if (!aIter.hasString())
                    continue;

                OUString aStr = aIter.getString();
                rStrings.insert(ScTypedStrData(aStr, 0.0, 0.0, ScTypedStrData::Header));
            }
        }
    }
}

void ScDocument::GetEmbedded( ScRange& rRange ) const
{
    rRange = aEmbedRange;
}

tools::Rectangle ScDocument::GetEmbeddedRect() const // 1/100 mm
{
    tools::Rectangle aRect;
    ScTable* pTable = nullptr;
    if (aEmbedRange.aStart.Tab() < GetTableCount())
        pTable = maTabs[aEmbedRange.aStart.Tab()].get();
    else
        OSL_FAIL("table out of range");
    if (!pTable)
    {
        OSL_FAIL("GetEmbeddedRect without a table");
    }
    else
    {
        SCCOL i;

        for (i=0; i<aEmbedRange.aStart.Col(); i++)
            aRect.AdjustLeft(pTable->GetColWidth(i) );
        aRect.AdjustTop(pTable->GetRowHeight( 0, aEmbedRange.aStart.Row() - 1) );
        aRect.SetRight( aRect.Left() );
        for (i=aEmbedRange.aStart.Col(); i<=aEmbedRange.aEnd.Col(); i++)
            aRect.AdjustRight(pTable->GetColWidth(i) );
        aRect.SetBottom( aRect.Top() );
        aRect.AdjustBottom(pTable->GetRowHeight( aEmbedRange.aStart.Row(), aEmbedRange.aEnd.Row()) );

        aRect = o3tl::convert(aRect, o3tl::Length::twip, o3tl::Length::mm100);
    }
    return aRect;
}

void ScDocument::SetEmbedded( const ScRange& rRange )
{
    bIsEmbedded = true;
    aEmbedRange = rRange;
}

void ScDocument::ResetEmbedded()
{
    bIsEmbedded = false;
    aEmbedRange = ScRange();
}

/** Similar to ScViewData::AddPixelsWhile(), but add height twips and only
    while result is less than nStopTwips.
    @return true if advanced at least one row.
 */
static bool lcl_AddTwipsWhile( tools::Long & rTwips, tools::Long nStopTwips, SCROW & rPosY, SCROW nEndRow, const ScTable * pTable, bool bHiddenAsZero )
{
    SCROW nRow = rPosY;
    bool bAdded = false;
    bool bStop = false;
    while (rTwips < nStopTwips && nRow <= nEndRow && !bStop)
    {
        SCROW nHeightEndRow;
        sal_uInt16 nHeight = pTable->GetRowHeight( nRow, nullptr, &nHeightEndRow, bHiddenAsZero );
        if (nHeightEndRow > nEndRow)
            nHeightEndRow = nEndRow;
        if (!nHeight)
            nRow = nHeightEndRow + 1;
        else
        {
            SCROW nRows = nHeightEndRow - nRow + 1;
            sal_Int64 nAdd = static_cast<sal_Int64>(nHeight) * nRows;
            if (nAdd + rTwips >= nStopTwips)
            {
                sal_Int64 nDiff = nAdd + rTwips - nStopTwips;
                nRows -= static_cast<SCROW>(nDiff / nHeight);
                nAdd = static_cast<sal_Int64>(nHeight) * nRows;
                // We're looking for a value that satisfies loop condition.
                if (nAdd + rTwips >= nStopTwips)
                {
                    --nRows;
                    nAdd -= nHeight;
                }
                bStop = true;
            }
            rTwips += static_cast<tools::Long>(nAdd);
            nRow += nRows;
        }
    }
    if (nRow > rPosY)
    {
        --nRow;
        bAdded = true;
    }
    rPosY = nRow;
    return bAdded;
}

ScRange ScDocument::GetRange( SCTAB nTab, const tools::Rectangle& rMMRect, bool bHiddenAsZero ) const
{
    ScTable* pTable = nullptr;
    if (nTab < GetTableCount())
        pTable = maTabs[nTab].get();
    else
        OSL_FAIL("table out of range");
    if (!pTable)
    {
        OSL_FAIL("GetRange without a table");
        return ScRange();
    }

    tools::Rectangle aPosRect = o3tl::convert(rMMRect, o3tl::Length::mm100, o3tl::Length::twip);
    if ( IsNegativePage( nTab ) )
        ScDrawLayer::MirrorRectRTL( aPosRect ); // Always with positive (LTR) values

    tools::Long nSize;
    tools::Long nTwips;
    tools::Long nAdd;
    bool bEnd;

    nSize = 0;
    nTwips = aPosRect.Left();

    SCCOL nX1 = 0;
    bEnd = false;
    while (!bEnd)
    {
        nAdd = pTable->GetColWidth(nX1, bHiddenAsZero);
        if (nSize+nAdd <= nTwips+1 && nX1<MaxCol())
        {
            nSize += nAdd;
            ++nX1;
        }
        else
            bEnd = true;
    }


    SCCOL nX2 = nX1;
    if (!aPosRect.IsEmpty())
    {
        bEnd = false;
        nTwips = aPosRect.Right();
        while (!bEnd)
        {
            nAdd = pTable->GetColWidth(nX2, bHiddenAsZero);
            if (nSize+nAdd < nTwips && nX2<MaxCol())
            {
                nSize += nAdd;
                ++nX2;
            }
            else
                bEnd = true;
        }
    }

    nSize = 0;
    nTwips = aPosRect.Top();

    SCROW nY1 = 0;
    // Was if(nSize+nAdd<=nTwips+1) inside loop => if(nSize+nAdd<nTwips+2)
    if (lcl_AddTwipsWhile( nSize, nTwips+2, nY1, MaxRow(), pTable, bHiddenAsZero) && nY1 < MaxRow())
        ++nY1;  // original loop ended on last matched +1 unless that was rDoc.MaxRow()

    SCROW nY2 = nY1;
    if (!aPosRect.IsEmpty())
    {
        nTwips = aPosRect.Bottom();
        // Was if(nSize+nAdd<nTwips) inside loop => if(nSize+nAdd<nTwips)
        if (lcl_AddTwipsWhile( nSize, nTwips, nY2, MaxRow(), pTable, bHiddenAsZero) && nY2 < MaxRow())
            ++nY2;  // original loop ended on last matched +1 unless that was rDoc.MaxRow()
    }

    return ScRange( nX1,nY1,nTab, nX2,nY2,nTab );
}

void ScDocument::SetEmbedded( SCTAB nTab, const tools::Rectangle& rRect ) // From VisArea (1/100 mm)
{
    bIsEmbedded = true;
    aEmbedRange = GetRange( nTab, rRect );
}

ScDocProtection* ScDocument::GetDocProtection() const
{
    return pDocProtection.get();
}

void ScDocument::SetDocProtection(const ScDocProtection* pProtect)
{
    if (pProtect)
        pDocProtection.reset(new ScDocProtection(*pProtect));
    else
        pDocProtection.reset();
}

bool ScDocument::IsDocProtected() const
{
    return pDocProtection && pDocProtection->isProtected();
}

bool ScDocument::IsDocEditable() const
{
    // Import into read-only document is possible
    return !IsDocProtected() && ( bImportingXML || mbChangeReadOnlyEnabled || !mpShell || !mpShell->IsReadOnly() );
}

bool ScDocument::IsTabProtected( SCTAB nTab ) const
{
    if (const ScTable* pTable = FetchTable(nTab))
        return pTable->IsProtected();

    OSL_FAIL("Wrong table number");
    return false;
}

const ScTableProtection* ScDocument::GetTabProtection(SCTAB nTab) const
{
    if (const ScTable* pTable = FetchTable(nTab))
        return pTable->GetProtection();

    return nullptr;
}

void ScDocument::SetTabProtection(SCTAB nTab, const ScTableProtection* pProtect)
{
    if (ScTable* pTable = FetchTable(nTab))
        pTable->SetProtection(pProtect);
}

void ScDocument::CopyTabProtection(SCTAB nTabSrc, SCTAB nTabDest)
{
    if (!HasTable(nTabSrc) || !HasTable(nTabDest))
        return;

    maTabs[nTabDest]->SetProtection( maTabs[nTabSrc]->GetProtection() );
}

const ScDocOptions& ScDocument::GetDocOptions() const
{
    assert(pDocOptions && "No DocOptions! :-(");
    return *pDocOptions;
}

void ScDocument::SetDocOptions( const ScDocOptions& rOpt )
{
    assert(pDocOptions && "No DocOptions! :-(");

    *pDocOptions = rOpt;
    if (mxPoolHelper)
        mxPoolHelper->SetFormTableOpt(rOpt);
}

const ScViewOptions& ScDocument::GetViewOptions() const
{
    assert(pViewOptions && "No ViewOptions! :-(");
    return *pViewOptions;
}

void ScDocument::SetViewOptions( const ScViewOptions& rOpt )
{
    assert(pViewOptions && "No ViewOptions! :-(");
    *pViewOptions = rOpt;
}

void ScDocument::GetLanguage( LanguageType& rLatin, LanguageType& rCjk, LanguageType& rCtl ) const
{
    rLatin = eLanguage;
    rCjk = eCjkLanguage;
    rCtl = eCtlLanguage;
}

void ScDocument::SetLanguage( LanguageType eLatin, LanguageType eCjk, LanguageType eCtl )
{
    eLanguage = eLatin;
    eCjkLanguage = eCjk;
    eCtlLanguage = eCtl;
    if ( mxPoolHelper.is() )
    {
        ScDocumentPool* pPool = mxPoolHelper->GetDocPool();
        pPool->SetUserDefaultItem( SvxLanguageItem( eLanguage, ATTR_FONT_LANGUAGE ) );
        pPool->SetUserDefaultItem( SvxLanguageItem( eCjkLanguage, ATTR_CJK_FONT_LANGUAGE ) );
        pPool->SetUserDefaultItem( SvxLanguageItem( eCtlLanguage, ATTR_CTL_FONT_LANGUAGE ) );
    }

    UpdateDrawLanguages(); // Set edit engine defaults in drawing layer pool
}

tools::Rectangle ScDocument::GetMMRect( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, SCTAB nTab, bool bHiddenAsZero ) const
{
    if (!HasTable(nTab))
    {
        OSL_FAIL("GetMMRect: wrong table");
        return tools::Rectangle(0,0,0,0);
    }

    SCCOL i;
    tools::Rectangle aRect;

    for (i=0; i<nStartCol; i++)
        aRect.AdjustLeft(GetColWidth(i,nTab, bHiddenAsZero ) );
    aRect.AdjustTop(GetRowHeight( 0, nStartRow-1, nTab, bHiddenAsZero ) );

    aRect.SetRight( aRect.Left() );
    aRect.SetBottom( aRect.Top() );

    for (i=nStartCol; i<=nEndCol; i++)
        aRect.AdjustRight(GetColWidth(i,nTab, bHiddenAsZero) );
    aRect.AdjustBottom(GetRowHeight( nStartRow, nEndRow, nTab, bHiddenAsZero ) );

    aRect = o3tl::convert(aRect, o3tl::Length::twip, o3tl::Length::mm100);

    if ( IsNegativePage( nTab ) )
        ScDrawLayer::MirrorRectRTL( aRect );

    return aRect;
}

void ScDocument::SetExtDocOptions( std::unique_ptr<ScExtDocOptions> pNewOptions )
{
    pExtDocOptions = std::move(pNewOptions);
}

void ScDocument::SetClipOptions(std::unique_ptr<ScClipOptions> pClipOptions)
{
    mpClipOptions = std::move(pClipOptions);
}

void ScDocument::DoMergeContents( SCCOL nStartCol, SCROW nStartRow,
                                    SCCOL nEndCol, SCROW nEndRow, SCTAB nTab )
{
    OUStringBuffer aTotal;
    OUString aCellStr;
    SCCOL nCol;
    SCROW nRow;
    ScCellValue aCell;
    for (nRow=nStartRow; nRow<=nEndRow; nRow++)
        for (nCol=nStartCol; nCol<=nEndCol; nCol++)
        {
            aCellStr = GetString(nCol, nRow, nTab);
            if (!aCellStr.isEmpty())
            {
                if (!aTotal.isEmpty())
                    aTotal.append(' ');
                aTotal.append(aCellStr);
                ScAddress aPos(nCol, nRow, nTab);
                if ((GetCellType(aPos) == CELLTYPE_EDIT) && aCell.isEmpty())
                    aCell = ScRefCellValue(*this, aPos);
            }
            if (nCol != nStartCol || nRow != nStartRow)
                SetString(nCol,nRow,nTab,"");
        }

    if (aCell.isEmpty() || !GetString(nStartCol, nStartRow, nTab).isEmpty())
        SetString(nStartCol, nStartRow, nTab, aTotal.makeStringAndClear());
    else
        aCell.release(*this, ScAddress(nStartCol, nStartRow, nTab));
}

void ScDocument::DoEmptyBlock( SCCOL nStartCol, SCROW nStartRow,
                               SCCOL nEndCol, SCROW nEndRow, SCTAB nTab )
{
    SCCOL nCol;
    SCROW nRow;
    for (nRow=nStartRow; nRow<=nEndRow; nRow++)
        for (nCol=nStartCol; nCol<=nEndCol; nCol++)
        {  // empty block except first cell
            if (nCol != nStartCol || nRow != nStartRow)
                SetString(nCol,nRow,nTab,"");
        }
}

void ScDocument::DoMerge( SCCOL nStartCol, SCROW nStartRow,
                          SCCOL nEndCol, SCROW nEndRow, SCTAB nTab, bool bDeleteCaptions )
{
    ScTable* pTab = FetchTable(nTab);
    if (!pTab)
        return;

    pTab->SetMergedCells(nStartCol, nStartRow, nEndCol, nEndRow);

    // Remove all covered notes (removed captions are collected by drawing undo if active)
    InsertDeleteFlags nDelFlag = InsertDeleteFlags::NOTE | (bDeleteCaptions ? InsertDeleteFlags::NONE : InsertDeleteFlags::NOCAPTIONS);
    if( nStartCol < nEndCol )
        DeleteAreaTab( nStartCol + 1, nStartRow, nEndCol, nStartRow, nTab, nDelFlag );
    if( nStartRow < nEndRow )
        DeleteAreaTab( nStartCol, nStartRow + 1, nEndCol, nEndRow, nTab, nDelFlag );
}

void ScDocument::RemoveMerge( SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    const ScMergeAttr* pAttr = GetAttr( nCol, nRow, nTab, ATTR_MERGE );

    if ( pAttr->GetColMerge() <= 1 && pAttr->GetRowMerge() <= 1 )
        return;

    SCCOL nEndCol = nCol + pAttr->GetColMerge() - 1;
    SCROW nEndRow = nRow + pAttr->GetRowMerge() - 1;

    RemoveFlagsTab( nCol, nRow, nEndCol, nEndRow, nTab, ScMF::Hor | ScMF::Ver );

    const ScMergeAttr* pDefAttr = &mxPoolHelper->GetDocPool()->GetUserOrPoolDefaultItem( ATTR_MERGE );
    ApplyAttr( nCol, nRow, nTab, *pDefAttr );
}

void ScDocument::ExtendPrintArea( OutputDevice* pDev, SCTAB nTab,
                    SCCOL nStartCol, SCROW nStartRow, SCCOL& rEndCol, SCROW nEndRow ) const
{
    if (HasTable(nTab))
        maTabs[nTab]->ExtendPrintArea(pDev, nStartCol, nStartRow, rEndCol, nEndRow);
}

SCSIZE ScDocument::GetPatternCount( SCTAB nTab, SCCOL nCol ) const
{
    if (const ScTable* pTable = FetchTable(nTab))
        return pTable->GetPatternCount( nCol );
    else
        return 0;
}

SCSIZE ScDocument::GetPatternCount( SCTAB nTab, SCCOL nCol, SCROW nRow1, SCROW nRow2 ) const
{
    if (const ScTable* pTable = FetchTable(nTab))
        return pTable->GetPatternCount(nCol, nRow1, nRow2);
    return 0;
}

void ScDocument::ReservePatternCount( SCTAB nTab, SCCOL nCol, SCSIZE nReserve )
{
    if (ScTable* pTable = FetchTable(nTab))
        pTable->ReservePatternCount(nCol, nReserve);
}

void ScDocument::GetSortParam( ScSortParam& rParam, SCTAB nTab )
{
    rParam = mSheetSortParams[ nTab ];
}

void ScDocument::SetSortParam( const ScSortParam& rParam, SCTAB nTab )
{
    mSheetSortParams[ nTab ] = rParam;
}

SCCOL ScDocument::ClampToAllocatedColumns(SCTAB nTab, SCCOL nCol) const
{
    return maTabs[nTab]->ClampToAllocatedColumns(nCol);
}

SCCOL ScDocument::GetAllocatedColumnsCount(SCTAB nTab) const
{
    return maTabs[nTab]->GetAllocatedColumnsCount();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
