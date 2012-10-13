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

#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>
#include "scitems.hxx"
#include <editeng/langitem.hxx>
#include <svl/srchitem.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/objsh.hxx>
#include <svl/zforlist.hxx>
#include <svl/PasswordHelper.hxx>
#include <vcl/svapp.hxx>
#include "document.hxx"
#include "attrib.hxx"
#include "cell.hxx"
#include "table.hxx"
#include "rangenam.hxx"
#include "dbdata.hxx"
#include "pivot.hxx"
#include "docpool.hxx"
#include "poolhelp.hxx"
#include "autoform.hxx"
#include "rangelst.hxx"
#include "chartarr.hxx"
#include "chartlock.hxx"
#include "refupdat.hxx"
#include "docoptio.hxx"
#include "viewopti.hxx"
#include "scextopt.hxx"
#include "brdcst.hxx"
#include "bcaslot.hxx"
#include "tablink.hxx"
#include "externalrefmgr.hxx"
#include "markdata.hxx"
#include "validat.hxx"
#include "dociter.hxx"
#include "detdata.hxx"
#include "detfunc.hxx"
#include "inputopt.hxx"     // GetExpandRefs
#include "chartlis.hxx"
#include "sc.hrc"           // SID_LINK
#include "hints.hxx"
#include "dpobject.hxx"
#include "drwlayer.hxx"
#include "unoreflist.hxx"
#include "listenercalls.hxx"
#include "dpshttab.hxx"
#include "dpcache.hxx"
#include "tabprotection.hxx"
#include "formulaparserpool.hxx"
#include "clipparam.hxx"
#include "sheetevents.hxx"
#include "colorscale.hxx"
#include "queryentry.hxx"

#include "globalnames.hxx"
#include <memory>

using namespace com::sun::star;

namespace {

void sortAndRemoveDuplicates(std::vector<ScTypedStrData>& rStrings, bool bCaseSens)
{
    if (bCaseSens)
    {
        std::sort(rStrings.begin(), rStrings.end(), ScTypedStrData::LessCaseSensitive());
        std::vector<ScTypedStrData>::iterator it =
            std::unique(rStrings.begin(), rStrings.end(), ScTypedStrData::EqualCaseSensitive());
        rStrings.erase(it, rStrings.end());
    }
    else
    {
        std::sort(rStrings.begin(), rStrings.end(), ScTypedStrData::LessCaseInsensitive());
        std::vector<ScTypedStrData>::iterator it =
            std::unique(rStrings.begin(), rStrings.end(), ScTypedStrData::EqualCaseInsensitive());
        rStrings.erase(it, rStrings.end());
    }
}

}

void ScDocument::GetAllTabRangeNames(ScRangeName::TabNameCopyMap& rNames) const
{
    ScRangeName::TabNameCopyMap aNames;
    for (SCTAB i = 0; i < static_cast<SCTAB>(maTabs.size()); ++i)
    {
        if (!maTabs[i])
            // no more tables to iterate through.
            break;

        const ScRangeName* p = maTabs[i]->GetRangeName();
        if (!p || p->empty())
            // ignore empty ones.
            continue;

        aNames.insert(ScRangeName::TabNameCopyMap::value_type(i, p));
    }
    rNames.swap(aNames);
}

void ScDocument::SetAllRangeNames( const boost::ptr_map<rtl::OUString, ScRangeName>& rRangeMap)
{
    rtl::OUString aGlobalStr(RTL_CONSTASCII_USTRINGPARAM(STR_GLOBAL_RANGE_NAME));
    boost::ptr_map<rtl::OUString,ScRangeName>::const_iterator itr = rRangeMap.begin(), itrEnd = rRangeMap.end();
    for (; itr!=itrEnd; ++itr)
    {
        if (itr->first == aGlobalStr)
        {
            delete pRangeName;
            const ScRangeName* pName = itr->second;
            if (pName->empty())
                pRangeName = NULL;
            else
                pRangeName = new ScRangeName( *pName );
        }
        else
        {
            const ScRangeName* pName = itr->second;
            SCTAB nTab;
            GetTable(itr->first, nTab);
            if (pName->empty())
                SetRangeName( nTab, NULL );
            else
                SetRangeName( nTab, new ScRangeName( *pName ) );
        }
    }
}

void ScDocument::GetTabRangeNameMap(std::map<rtl::OUString, ScRangeName*>& aRangeNameMap)
{
    for (SCTAB i = 0; i < static_cast<SCTAB>(maTabs.size()); ++i)
    {
        if (!maTabs[i])
            continue;
        ScRangeName* p = maTabs[i]->GetRangeName();
        if (!p )
        {
            p = new ScRangeName();
            SetRangeName(i, p);
        }
        rtl::OUString aTableName;
        maTabs[i]->GetName(aTableName);
        aRangeNameMap.insert(std::pair<rtl::OUString, ScRangeName*>(aTableName,p));
    }
}

void ScDocument::GetRangeNameMap(std::map<rtl::OUString, ScRangeName*>& aRangeNameMap)
{
    GetTabRangeNameMap(aRangeNameMap);
    if (!pRangeName)
    {
        pRangeName = new ScRangeName();
    }
    rtl::OUString aGlobal(RTL_CONSTASCII_USTRINGPARAM(STR_GLOBAL_RANGE_NAME));
    aRangeNameMap.insert(std::pair<rtl::OUString, ScRangeName*>(aGlobal, pRangeName));
}

ScRangeName* ScDocument::GetRangeName(SCTAB nTab) const
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return NULL;

    return maTabs[nTab]->GetRangeName();
}

ScRangeName* ScDocument::GetRangeName() const
{
    if (!pRangeName)
        pRangeName = new ScRangeName;
    return pRangeName;
}

void ScDocument::SetRangeName(SCTAB nTab, ScRangeName* pNew)
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return;

    return maTabs[nTab]->SetRangeName(pNew);
}

void ScDocument::SetRangeName( ScRangeName* pNewRangeName )
{
    delete pRangeName;
    pRangeName = pNewRangeName;
}


const ScRangeData* ScDocument::GetRangeAtBlock( const ScRange& rBlock, rtl::OUString* pName ) const
{
    const ScRangeData* pData = NULL;
    if ( pRangeName )
    {
        pData = pRangeName->findByRange( rBlock );
        if (pData && pName)
            *pName = pData->GetName();
    }
    return pData;
}

ScDBCollection* ScDocument::GetDBCollection() const
{
    return pDBCollection;
}

void ScDocument::SetDBCollection( ScDBCollection* pNewDBCollection, bool bRemoveAutoFilter )
{
    if (pDBCollection && bRemoveAutoFilter)
    {
        //  remove auto filter attribute if new db data don't contain auto filter flag
        //  start position is also compared, so bRemoveAutoFilter must not be set from ref-undo!

        ScDBCollection::NamedDBs& rNamedDBs = pDBCollection->getNamedDBs();
        ScDBCollection::NamedDBs::const_iterator itr = rNamedDBs.begin(), itrEnd = rNamedDBs.end();
        for (; itr != itrEnd; ++itr)
        {
            const ScDBData& rOldData = *itr;
            if (!rOldData.HasAutoFilter())
                continue;

            ScRange aOldRange;
            rOldData.GetArea(aOldRange);

            bool bFound = false;
            if (pNewDBCollection)
            {
                ScDBData* pNewData = pNewDBCollection->getNamedDBs().findByName(rOldData.GetName());
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
                                aOldRange.aStart.Tab(), SC_MF_AUTO );
                RepaintRange( aOldRange );
            }
        }
    }

    delete pDBCollection;

    pDBCollection = pNewDBCollection;
}

const ScDBData* ScDocument::GetDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, bool bStartOnly) const
{
    if (pDBCollection)
        return pDBCollection->GetDBAtCursor(nCol, nRow, nTab, bStartOnly);
    else
        return NULL;
}

ScDBData* ScDocument::GetDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, bool bStartOnly)
{
    if (pDBCollection)
        return pDBCollection->GetDBAtCursor(nCol, nRow, nTab, bStartOnly);
    else
        return NULL;
}

const ScDBData* ScDocument::GetDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2) const
{
    if (pDBCollection)
        return pDBCollection->GetDBAtArea(nTab, nCol1, nRow1, nCol2, nRow2);
    else
        return NULL;
}

ScDBData* ScDocument::GetDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2)
{
    if (pDBCollection)
        return pDBCollection->GetDBAtArea(nTab, nCol1, nRow1, nCol2, nRow2);
    else
        return NULL;
}

ScDPCollection* ScDocument::GetDPCollection()
{
    if (!pDPCollection)
        pDPCollection = new ScDPCollection(this);
    return pDPCollection;
}

ScDPObject* ScDocument::GetDPAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab) const
{
    if (!pDPCollection)
        return NULL;

    sal_uInt16 nCount = pDPCollection->GetCount();
    ScAddress aPos( nCol, nRow, nTab );
    for (sal_uInt16 i=0; i<nCount; i++)
        if ( (*pDPCollection)[i]->GetOutRange().In( aPos ) )
            return (*pDPCollection)[i];

    return NULL;
}

ScDPObject* ScDocument::GetDPAtBlock( const ScRange & rBlock ) const
{
    if (!pDPCollection)
        return NULL;

    /* Walk the collection in reverse order to get something of an
     * approximation of MS Excels 'most recent' effect. */
    sal_uInt16 i = pDPCollection->GetCount();
    while ( i-- > 0 )
        if ( (*pDPCollection)[i]->GetOutRange().In( rBlock ) )
            return (*pDPCollection)[i];

    return NULL;
}

ScChartCollection* ScDocument::GetChartCollection() const
{
    return pChartCollection;
}

void ScDocument::StopTemporaryChartLock()
{
    if( apTemporaryChartLock.get() )
        apTemporaryChartLock->StopLocking();
}

ScChartListenerCollection* ScDocument::GetChartListenerCollection() const
{
    return pChartListenerCollection;
}

void ScDocument::SetChartListenerCollection(
            ScChartListenerCollection* pNewChartListenerCollection,
            bool bSetChartRangeLists )
{
    ScChartListenerCollection* pOld = pChartListenerCollection;
    pChartListenerCollection = pNewChartListenerCollection;
    if ( pChartListenerCollection )
    {
        if ( pOld )
            pChartListenerCollection->SetDiffDirty( *pOld, bSetChartRangeLists );
        pChartListenerCollection->StartAllListeners();
    }
    delete pOld;
}

void ScDocument::SetScenario( SCTAB nTab, bool bFlag )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        maTabs[nTab]->SetScenario(bFlag);
}

bool ScDocument::IsScenario( SCTAB nTab ) const
{
    return ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] &&maTabs[nTab]->IsScenario();
}

void ScDocument::SetScenarioData( SCTAB nTab, const rtl::OUString& rComment,
                                        const Color& rColor, sal_uInt16 nFlags )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] && maTabs[nTab]->IsScenario())
    {
        maTabs[nTab]->SetScenarioComment( rComment );
        maTabs[nTab]->SetScenarioColor( rColor );
        maTabs[nTab]->SetScenarioFlags( nFlags );
    }
}

Color ScDocument::GetTabBgColor( SCTAB nTab ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetTabBgColor();
    return Color(COL_AUTO);
}

void ScDocument::SetTabBgColor( SCTAB nTab, const Color& rColor )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        maTabs[nTab]->SetTabBgColor(rColor);
}

bool ScDocument::IsDefaultTabBgColor( SCTAB nTab ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetTabBgColor() == COL_AUTO;
    return true;
}

void ScDocument::GetScenarioData( SCTAB nTab, rtl::OUString& rComment,
                                        Color& rColor, sal_uInt16& rFlags ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] && maTabs[nTab]->IsScenario())
    {
        maTabs[nTab]->GetScenarioComment( rComment );
        rColor = maTabs[nTab]->GetScenarioColor();
        rFlags = maTabs[nTab]->GetScenarioFlags();
    }
}

void ScDocument::GetScenarioFlags( SCTAB nTab, sal_uInt16& rFlags ) const
{
    if (VALIDTAB(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] && maTabs[nTab]->IsScenario())
        rFlags = maTabs[nTab]->GetScenarioFlags();
}

bool ScDocument::IsLinked( SCTAB nTab ) const
{
    return ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] && maTabs[nTab]->IsLinked();
    // euqivalent to
    //if (ValidTab(nTab) && pTab[nTab])
    //  return pTab[nTab]->IsLinked();
    //return false;
}

formula::FormulaGrammar::AddressConvention ScDocument::GetAddressConvention() const
{
    return formula::FormulaGrammar::extractRefConvention(eGrammar);
}

formula::FormulaGrammar::Grammar ScDocument::GetGrammar() const
{
    return eGrammar;
}

void ScDocument::SetGrammar( formula::FormulaGrammar::Grammar eGram )
{
    eGrammar = eGram;
}

sal_uInt8 ScDocument::GetLinkMode( SCTAB nTab ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetLinkMode();
    return SC_LINK_NONE;
}

const rtl::OUString ScDocument::GetLinkDoc( SCTAB nTab ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetLinkDoc();
    return rtl::OUString();
}

const rtl::OUString ScDocument::GetLinkFlt( SCTAB nTab ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetLinkFlt();
    return rtl::OUString();
}

const rtl::OUString ScDocument::GetLinkOpt( SCTAB nTab ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetLinkOpt();
    return rtl::OUString();
}

const rtl::OUString ScDocument::GetLinkTab( SCTAB nTab ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetLinkTab();
    return rtl::OUString();
}

sal_uLong ScDocument::GetLinkRefreshDelay( SCTAB nTab ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetLinkRefreshDelay();
    return 0;
}

void ScDocument::SetLink( SCTAB nTab, sal_uInt8 nMode, const rtl::OUString& rDoc,
                            const rtl::OUString& rFilter, const rtl::OUString& rOptions,
                            const rtl::OUString& rTabName, sal_uLong nRefreshDelay )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        maTabs[nTab]->SetLink( nMode, rDoc, rFilter, rOptions, rTabName, nRefreshDelay );
}

bool ScDocument::HasLink( const rtl::OUString& rDoc,
                            const rtl::OUString& rFilter, const rtl::OUString& rOptions ) const
{
    SCTAB nCount = static_cast<SCTAB>(maTabs.size());
    for (SCTAB i=0; i<nCount; i++)
        if (maTabs[i]->IsLinked()
                && maTabs[i]->GetLinkDoc() == rDoc
                && maTabs[i]->GetLinkFlt() == rFilter
                && maTabs[i]->GetLinkOpt() == rOptions)
            return true;

    return false;
}

bool ScDocument::LinkExternalTab( SCTAB& rTab, const rtl::OUString& aDocTab,
        const rtl::OUString& aFileName, const rtl::OUString& aTabName )
{
    if ( IsClipboard() )
    {
        OSL_FAIL( "LinkExternalTab in Clipboard" );
        return false;
    }
    rTab = 0;
    rtl::OUString  aFilterName;        // wird vom Loader gefuellt
    rtl::OUString  aOptions;       // Filter-Optionen
    sal_uInt32 nLinkCnt = pExtDocOptions ? pExtDocOptions->GetDocSettings().mnLinkCnt : 0;
    ScDocumentLoader aLoader( aFileName, aFilterName, aOptions, nLinkCnt + 1 );
    if ( aLoader.IsError() )
        return false;
    ScDocument* pSrcDoc = aLoader.GetDocument();

    //  Tabelle kopieren
    SCTAB nSrcTab;
    if ( pSrcDoc->GetTable( aTabName, nSrcTab ) )
    {
        if ( !InsertTab( SC_TAB_APPEND, aDocTab, true ) )
        {
            OSL_FAIL("can't insert external document table");
            return false;
        }
        rTab = GetTableCount() - 1;
        // nicht neu einfuegen, nur Ergebnisse
        TransferTab( pSrcDoc, nSrcTab, rTab, false, true );
    }
    else
        return false;

    sal_uLong nRefreshDelay = 0;

    bool bWasThere = HasLink( aFileName, aFilterName, aOptions );
    SetLink( rTab, SC_LINK_VALUE, aFileName, aFilterName, aOptions, aTabName, nRefreshDelay );
    if ( !bWasThere )       // Link pro Quelldokument nur einmal eintragen
    {
        ScTableLink* pLink = new ScTableLink( pShell, aFileName, aFilterName, aOptions, nRefreshDelay );
        pLink->SetInCreate( true );
        String aFilName = aFilterName;
        GetLinkManager()->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, String(aFileName),
                                        &aFilName );
        pLink->Update();
        pLink->SetInCreate( false );
        SfxBindings* pBindings = GetViewBindings();
        if (pBindings)
            pBindings->Invalidate( SID_LINKS );
    }
    return true;
}

ScExternalRefManager* ScDocument::GetExternalRefManager() const
{
    ScDocument* pThis = const_cast<ScDocument*>(this);
    if (!pExternalRefMgr.get())
        pThis->pExternalRefMgr.reset( new ScExternalRefManager( pThis));

    return pExternalRefMgr.get();
}

bool ScDocument::IsInExternalReferenceMarking() const
{
    return pExternalRefMgr.get() && pExternalRefMgr->isInReferenceMarking();
}

void ScDocument::MarkUsedExternalReferences()
{
    if (!pExternalRefMgr.get())
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
    if( !mxFormulaParserPool.get() )
        mxFormulaParserPool.reset( new ScFormulaParserPool( *this ) );
    return *mxFormulaParserPool;
}

const ScSheetEvents* ScDocument::GetSheetEvents( SCTAB nTab ) const
{
    if (VALIDTAB(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetSheetEvents();
    return NULL;
}

void ScDocument::SetSheetEvents( SCTAB nTab, const ScSheetEvents* pNew )
{
    if (VALIDTAB(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        maTabs[nTab]->SetSheetEvents( pNew );
}

bool ScDocument::HasSheetEventScript( SCTAB nTab, sal_Int32 nEvent, bool bWithVbaEvents ) const
{
    if (nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
    {
        // check if any event handler script has been configured
        const ScSheetEvents* pEvents = maTabs[nTab]->GetSheetEvents();
        if ( pEvents && pEvents->GetScript( nEvent ) )
            return true;
        // check if VBA event handlers exist
        if (bWithVbaEvents && mxVbaEvents.is()) try
        {
            uno::Sequence< uno::Any > aArgs( 1 );
            aArgs[ 0 ] <<= nTab;
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

bool ScDocument::HasAnySheetEventScript( sal_Int32 nEvent, bool bWithVbaEvents ) const
{
    SCTAB nSize = static_cast<SCTAB>(maTabs.size());
    for (SCTAB nTab = 0; nTab < nSize; nTab++)
        if (HasSheetEventScript( nTab, nEvent, bWithVbaEvents ))
            return true;
    return false;
}

bool ScDocument::HasAnyCalcNotification() const
{
    SCTAB nSize = static_cast<SCTAB>(maTabs.size());
    for (SCTAB nTab = 0; nTab < nSize; nTab++)
        if (maTabs[nTab] && maTabs[nTab]->GetCalcNotification())
            return true;
    return false;
}

bool ScDocument::HasCalcNotification( SCTAB nTab ) const
{
    if (VALIDTAB(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetCalcNotification();
    return false;
}

void ScDocument::SetCalcNotification( SCTAB nTab )
{
    // set only if not set before
    if (VALIDTAB(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] && !maTabs[nTab]->GetCalcNotification())
        maTabs[nTab]->SetCalcNotification(true);
}

void ScDocument::ResetCalcNotifications()
{
    SCTAB nSize = static_cast<SCTAB>(maTabs.size());
    for (SCTAB nTab = 0; nTab < nSize; nTab++)
        if (maTabs[nTab] && maTabs[nTab]->GetCalcNotification())
            maTabs[nTab]->SetCalcNotification(false);
}

ScOutlineTable* ScDocument::GetOutlineTable( SCTAB nTab, bool bCreate )
{
    ScOutlineTable* pVal = NULL;

    if (VALIDTAB(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
        if (maTabs[nTab])
        {
            pVal = maTabs[nTab]->GetOutlineTable();
            if (!pVal)
                if (bCreate)
                {
                    maTabs[nTab]->StartOutlineTable();
                    pVal = maTabs[nTab]->GetOutlineTable();
                }
        }

    return pVal;
}

bool ScDocument::SetOutlineTable( SCTAB nTab, const ScOutlineTable* pNewOutline )
{
    return VALIDTAB(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] && maTabs[nTab]->SetOutlineTable(pNewOutline);
}

void ScDocument::DoAutoOutline( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow, SCTAB nTab )
{
    if (VALIDTAB(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        maTabs[nTab]->DoAutoOutline( nStartCol, nStartRow, nEndCol, nEndRow );
}

bool ScDocument::TestRemoveSubTotals( SCTAB nTab, const ScSubTotalParam& rParam )
{
    return VALIDTAB(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] && maTabs[nTab]->TestRemoveSubTotals( rParam );
}

void ScDocument::RemoveSubTotals( SCTAB nTab, ScSubTotalParam& rParam )
{
    if ( VALIDTAB(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->RemoveSubTotals( rParam );
}

bool ScDocument::DoSubTotals( SCTAB nTab, ScSubTotalParam& rParam )
{
    return VALIDTAB(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] && maTabs[nTab]->DoSubTotals( rParam );
}

bool ScDocument::HasSubTotalCells( const ScRange& rRange )
{
    ScCellIterator aIter( this, rRange );
    ScBaseCell* pCell = aIter.GetFirst();
    while (pCell)
    {
        if ( pCell->GetCellType() == CELLTYPE_FORMULA && ((ScFormulaCell*)pCell)->IsSubTotal() )
            return true;

        pCell = aIter.GetNext();
    }
    return false;   // none found
}

//  kopiert aus diesem Dokument die Zellen von Positionen, an denen in pPosDoc
//  auch Zellen stehen, nach pDestDoc

void ScDocument::CopyUpdated( ScDocument* pPosDoc, ScDocument* pDestDoc )
{
    SCTAB nCount = static_cast<SCTAB>(maTabs.size());
    for (SCTAB nTab=0; nTab<nCount; nTab++)
        if (maTabs[nTab] && pPosDoc->maTabs[nTab] && pDestDoc->maTabs[nTab])
            maTabs[nTab]->CopyUpdated( pPosDoc->maTabs[nTab], pDestDoc->maTabs[nTab] );
}

void ScDocument::CopyScenario( SCTAB nSrcTab, SCTAB nDestTab, bool bNewScenario )
{
    if (ValidTab(nSrcTab) && ValidTab(nDestTab) && nSrcTab < static_cast<SCTAB>(maTabs.size())
                && nDestTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nSrcTab] && maTabs[nDestTab])
    {
        //  Flags fuer aktive Szenarios richtig setzen
        //  und aktuelle Werte in bisher aktive Szenarios zurueckschreiben

        ScRangeList aRanges = *maTabs[nSrcTab]->GetScenarioRanges();

        //  nDestTab ist die Zieltabelle
        for ( SCTAB nTab = nDestTab+1;
                nTab< static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] && maTabs[nTab]->IsScenario();
                nTab++ )
        {
            if ( maTabs[nTab]->IsActiveScenario() )     // auch wenn's dasselbe Szenario ist
            {
                bool bTouched = false;
                for ( size_t nR=0, nRangeCount = aRanges.size(); nR < nRangeCount && !bTouched; nR++ )
                {
                    const ScRange* pRange = aRanges[ nR ];
                    if ( maTabs[nTab]->HasScenarioRange( *pRange ) )
                        bTouched = true;
                }
                if (bTouched)
                {
                    maTabs[nTab]->SetActiveScenario(false);
                    if ( maTabs[nTab]->GetScenarioFlags() & SC_SCENARIO_TWOWAY )
                        maTabs[nTab]->CopyScenarioFrom( maTabs[nDestTab] );
                }
            }
        }

        maTabs[nSrcTab]->SetActiveScenario(true);       // da kommt's her...
        if (!bNewScenario)                          // Daten aus dem ausgewaehlten Szenario kopieren
        {
            bool bOldAutoCalc = GetAutoCalc();
            SetAutoCalc( false );   // Mehrfachberechnungen vermeiden
            maTabs[nSrcTab]->CopyScenarioTo( maTabs[nDestTab] );
            SetDirty();
            SetAutoCalc( bOldAutoCalc );
        }
    }
}

void ScDocument::MarkScenario( SCTAB nSrcTab, SCTAB nDestTab, ScMarkData& rDestMark,
                                bool bResetMark, sal_uInt16 nNeededBits ) const
{
    if (bResetMark)
        rDestMark.ResetMark();

    if (ValidTab(nSrcTab) && nSrcTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nSrcTab])
        maTabs[nSrcTab]->MarkScenarioIn( rDestMark, nNeededBits );

    rDestMark.SetAreaTab( nDestTab );
}

bool ScDocument::HasScenarioRange( SCTAB nTab, const ScRange& rRange ) const
{
    return ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] && maTabs[nTab]->HasScenarioRange( rRange );
}

const ScRangeList* ScDocument::GetScenarioRanges( SCTAB nTab ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetScenarioRanges();

    return NULL;
}

bool ScDocument::IsActiveScenario( SCTAB nTab ) const
{
    return ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] && maTabs[nTab]->IsActiveScenario(  );
}

void ScDocument::SetActiveScenario( SCTAB nTab, bool bActive )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        maTabs[nTab]->SetActiveScenario( bActive );
}

bool ScDocument::TestCopyScenario( SCTAB nSrcTab, SCTAB nDestTab ) const
{
    if (ValidTab(nSrcTab) && nSrcTab < static_cast<SCTAB>(maTabs.size())
                && nDestTab < static_cast<SCTAB>(maTabs.size())&& ValidTab(nDestTab))
        return maTabs[nSrcTab]->TestCopyScenarioTo( maTabs[nDestTab] );

    OSL_FAIL("falsche Tabelle bei TestCopyScenario");
    return false;
}

void ScDocument::AddUnoObject( SfxListener& rObject )
{
    if (!pUnoBroadcaster)
        pUnoBroadcaster = new SfxBroadcaster;

    rObject.StartListening( *pUnoBroadcaster );
}

void ScDocument::RemoveUnoObject( SfxListener& rObject )
{
    if (pUnoBroadcaster)
    {
        rObject.EndListening( *pUnoBroadcaster );

        if ( bInUnoBroadcast )
        {
            //  Broadcasts from ScDocument::BroadcastUno are the only way that
            //  uno object methods are called without holding a reference.
            //
            //  If RemoveUnoObject is called from an object dtor in the finalizer thread
            //  while the main thread is calling BroadcastUno, the dtor thread must wait
            //  (or the object's Notify might try to access a deleted object).
            //  The SolarMutex can't be locked here because if a component is called from
            //  a VCL event, the main thread has the SolarMutex locked all the time.
            //
            //  This check is done after calling EndListening, so a later BroadcastUno call
            //  won't touch this object.

            osl::SolarMutex& rSolarMutex = Application::GetSolarMutex();
            if ( rSolarMutex.tryToAcquire() )
            {
                //  BroadcastUno is always called with the SolarMutex locked, so if it
                //  can be acquired, this is within the same thread (should not happen)
                OSL_FAIL( "RemoveUnoObject called from BroadcastUno" );
                rSolarMutex.release();
            }
            else
            {
                //  let the thread that called BroadcastUno continue
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
    if (pUnoBroadcaster)
    {
        bInUnoBroadcast = true;
        pUnoBroadcaster->Broadcast( rHint );
        bInUnoBroadcast = false;

        // During Broadcast notification, Uno objects can add to pUnoListenerCalls.
        // The listener calls must be processed after completing the broadcast,
        // because they can add or remove objects from pUnoBroadcaster.

        if ( pUnoListenerCalls && rHint.ISA( SfxSimpleHint ) &&
                ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DATACHANGED &&
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
}

void ScDocument::AddUnoListenerCall( const uno::Reference<util::XModifyListener>& rListener,
                                        const lang::EventObject& rEvent )
{
    OSL_ENSURE( bInUnoBroadcast, "AddUnoListenerCall is supposed to be called from BroadcastUno only" );

    if ( !pUnoListenerCalls )
        pUnoListenerCalls = new ScUnoListenerCalls;
    pUnoListenerCalls->Add( rListener, rEvent );
}

void ScDocument::BeginUnoRefUndo()
{
    OSL_ENSURE( !pUnoRefUndoList, "BeginUnoRefUndo twice" );
    delete pUnoRefUndoList;

    pUnoRefUndoList = new ScUnoRefList;
}

ScUnoRefList* ScDocument::EndUnoRefUndo()
{
    ScUnoRefList* pRet = pUnoRefUndoList;
    pUnoRefUndoList = NULL;
    return pRet;                // must be deleted by caller!
}

void ScDocument::AddUnoRefChange( sal_Int64 nId, const ScRangeList& rOldRanges )
{
    if ( pUnoRefUndoList )
        pUnoRefUndoList->Add( nId, rOldRanges );
}

sal_Int64 ScDocument::GetNewUnoId()
{
    return ++nUnoObjectId;
}

void ScDocument::UpdateReference( UpdateRefMode eUpdateRefMode,
                                    SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                    SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                                    ScDocument* pUndoDoc, bool bIncludeDraw,
                                    bool bUpdateNoteCaptionPos )
{
    PutInOrder( nCol1, nCol2 );
    PutInOrder( nRow1, nRow2 );
    PutInOrder( nTab1, nTab2 );
    if (VALIDTAB(nTab1) && VALIDTAB(nTab2))
    {
        bool bExpandRefsOld = IsExpandRefs();
        if ( eUpdateRefMode == URM_INSDEL && (nDx > 0 || nDy > 0 || nDz > 0) )
            SetExpandRefs( SC_MOD()->GetInputOptions().GetExpandRefs() );
        SCTAB i;
        SCTAB iMax;
        if ( eUpdateRefMode == URM_COPY )
        {
            i = nTab1;
            iMax = nTab2;
        }
        else
        {
            ScRange aRange( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
            xColNameRanges->UpdateReference( eUpdateRefMode, this, aRange, nDx, nDy, nDz );
            xRowNameRanges->UpdateReference( eUpdateRefMode, this, aRange, nDx, nDy, nDz );
            pDBCollection->UpdateReference( eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, nDx, nDy, nDz );
            if (pRangeName)
                pRangeName->UpdateReference( eUpdateRefMode, aRange, nDx, nDy, nDz );
            if ( pDPCollection )
                pDPCollection->UpdateReference( eUpdateRefMode, aRange, nDx, nDy, nDz );
            UpdateChartRef( eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, nDx, nDy, nDz );
            UpdateRefAreaLinks( eUpdateRefMode, aRange, nDx, nDy, nDz );
            if ( pValidationList )
                pValidationList->UpdateReference( eUpdateRefMode, aRange, nDx, nDy, nDz );
            if ( pDetOpList )
                pDetOpList->UpdateReference( this, eUpdateRefMode, aRange, nDx, nDy, nDz );
            if ( pUnoBroadcaster )
                pUnoBroadcaster->Broadcast( ScUpdateRefHint(
                                    eUpdateRefMode, aRange, nDx, nDy, nDz ) );
            i = 0;
            iMax = static_cast<SCTAB>(maTabs.size())-1;
        }
        for ( ; i<=iMax && i < static_cast<SCTAB>(maTabs.size()); i++)
            if (maTabs[i])
                maTabs[i]->UpdateReference(
                    eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2,
                    nDx, nDy, nDz, pUndoDoc, bIncludeDraw, bUpdateNoteCaptionPos );

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
            if ( ScRefUpdate::Update( this, eUpdateRefMode, nCol1,nRow1,nTab1, nCol2,nRow2,nTab2,
                                        nDx,nDy,nDz, theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 ) )
            {
                aEmbedRange = ScRange( theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 );
            }
        }
        SetExpandRefs( bExpandRefsOld );

        // after moving, no clipboard move ref-updates are possible
        if ( eUpdateRefMode != URM_COPY && IsClipboardSource() )
        {
            ScDocument* pClipDoc = SC_MOD()->GetClipDoc();
            if (pClipDoc)
                pClipDoc->GetClipParam().mbCutMode = false;
        }
    }
}

void ScDocument::UpdateTranspose( const ScAddress& rDestPos, ScDocument* pClipDoc,
                                        const ScMarkData& rMark, ScDocument* pUndoDoc )
{
    OSL_ENSURE(pClipDoc->bIsClip, "UpdateTranspose: kein Clip");

    ScRange aSource;
    ScClipParam& rClipParam = GetClipParam();
    if (!rClipParam.maRanges.empty())
        aSource = *rClipParam.maRanges.front();
    ScAddress aDest = rDestPos;

    SCTAB nClipTab = 0;
    for (SCTAB nDestTab=0; nDestTab< static_cast<SCTAB>(maTabs.size()) && maTabs[nDestTab]; nDestTab++)
        if (rMark.GetTableSelect(nDestTab))
        {
            while (!pClipDoc->maTabs[nClipTab]) nClipTab = (nClipTab+1) % (MAXTAB+1);
            aSource.aStart.SetTab( nClipTab );
            aSource.aEnd.SetTab( nClipTab );
            aDest.SetTab( nDestTab );

            //  wie UpdateReference
            if (pRangeName)
                pRangeName->UpdateTranspose( aSource, aDest );      // vor den Zellen!
            for (SCTAB i=0; i< static_cast<SCTAB>(maTabs.size()); i++)
                if (maTabs[i])
                    maTabs[i]->UpdateTranspose( aSource, aDest, pUndoDoc );

            nClipTab = (nClipTab+1) % (MAXTAB+1);
        }
}

void ScDocument::UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY )
{
    //! pDBCollection
    //! pPivotCollection
    //! UpdateChartRef

    if (pRangeName)
        pRangeName->UpdateGrow( rArea, nGrowX, nGrowY );

    for (SCTAB i=0; i< static_cast<SCTAB>(maTabs.size()) && maTabs[i]; i++)
        maTabs[i]->UpdateGrow( rArea, nGrowX, nGrowY );
}

void ScDocument::Fill(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScProgress* pProgress, const ScMarkData& rMark,
                        sal_uLong nFillCount, FillDir eFillDir, FillCmd eFillCmd, FillDateCmd eFillDateCmd,
                        double nStepValue, double nMaxValue)
{
    PutInOrder( nCol1, nCol2 );
    PutInOrder( nRow1, nRow2 );
    SCTAB nMax = maTabs.size();
    ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd && *itr < nMax; ++itr)
        if (maTabs[*itr])
            maTabs[*itr]->Fill(nCol1, nRow1, nCol2, nRow2,
                            nFillCount, eFillDir, eFillCmd, eFillDateCmd,
                            nStepValue, nMaxValue, pProgress);
}

rtl::OUString ScDocument::GetAutoFillPreview( const ScRange& rSource, SCCOL nEndX, SCROW nEndY )
{
    SCTAB nTab = rSource.aStart.Tab();
    if (nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetAutoFillPreview( rSource, nEndX, nEndY );

    return rtl::OUString();
}

void ScDocument::AutoFormat( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                    sal_uInt16 nFormatNo, const ScMarkData& rMark )
{
    PutInOrder( nStartCol, nEndCol );
    PutInOrder( nStartRow, nEndRow );
    SCTAB nMax = maTabs.size();
    ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd && *itr < nMax; ++itr)
        if (maTabs[*itr])
            maTabs[*itr]->AutoFormat( nStartCol, nStartRow, nEndCol, nEndRow, nFormatNo );
}

void ScDocument::GetAutoFormatData(SCTAB nTab, SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                    ScAutoFormatData& rData)
{
    if (VALIDTAB(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
    {
        if (maTabs[nTab])
        {
            PutInOrder(nStartCol, nEndCol);
            PutInOrder(nStartRow, nEndRow);
            maTabs[nTab]->GetAutoFormatData(nStartCol, nStartRow, nEndCol, nEndRow, rData);
        }
    }
}

void ScDocument::GetSearchAndReplaceStart( const SvxSearchItem& rSearchItem,
        SCCOL& rCol, SCROW& rRow )
{
    sal_uInt16 nCommand = rSearchItem.GetCommand();
    bool bReplace = ( nCommand == SVX_SEARCHCMD_REPLACE ||
        nCommand == SVX_SEARCHCMD_REPLACE_ALL );
    if ( rSearchItem.GetBackward() )
    {
        if ( rSearchItem.GetRowDirection() )
        {
            if ( rSearchItem.GetPattern() )
            {
                rCol = MAXCOL;
                rRow = MAXROW+1;
            }
            else if ( bReplace )
            {
                rCol = MAXCOL;
                rRow = MAXROW;
            }
            else
            {
                rCol = MAXCOL+1;
                rRow = MAXROW;
            }
        }
        else
        {
            if ( rSearchItem.GetPattern() )
            {
                rCol = MAXCOL+1;
                rRow = MAXROW;
            }
            else if ( bReplace )
            {
                rCol = MAXCOL;
                rRow = MAXROW;
            }
            else
            {
                rCol = MAXCOL;
                rRow = MAXROW+1;
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
                rRow = (SCROW) -1;
            }
            else if ( bReplace )
            {
                rCol = 0;
                rRow = 0;
            }
            else
            {
                rCol = (SCCOL) -1;
                rRow = 0;
            }
        }
        else
        {
            if ( rSearchItem.GetPattern() )
            {
                rCol = (SCCOL) -1;
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
                rRow = (SCROW) -1;
            }
        }
    }
}

bool ScDocument::SearchAndReplace(
    const SvxSearchItem& rSearchItem, SCCOL& rCol, SCROW& rRow, SCTAB& rTab,
    const ScMarkData& rMark, ScRangeList& rMatchedRanges,
    rtl::OUString& rUndoStr, ScDocument* pUndoDoc)
{
    //!     getrennte Markierungen pro Tabelle verwalten !!!!!!!!!!!!!

    bool bFound = false;
    if (rTab >= static_cast<SCTAB>(maTabs.size()))
        OSL_FAIL("table out of range");
    if (VALIDTAB(rTab))
    {
        SCCOL nCol;
        SCROW nRow;
        SCTAB nTab;
        sal_uInt16 nCommand = rSearchItem.GetCommand();
        if ( nCommand == SVX_SEARCHCMD_FIND_ALL ||
             nCommand == SVX_SEARCHCMD_REPLACE_ALL )
        {
            SCTAB nMax = maTabs.size();
            ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
            for (; itr != itrEnd && *itr < nMax; ++itr)
                if (maTabs[*itr])
                {
                    nCol = 0;
                    nRow = 0;
                    bFound |= maTabs[*itr]->SearchAndReplace(
                        rSearchItem, nCol, nRow, rMark, rMatchedRanges, rUndoStr, pUndoDoc);
                }

            //  Markierung wird innen schon komplett gesetzt
        }
        else
        {
            nCol = rCol;
            nRow = rRow;
            if (rSearchItem.GetBackward())
            {
                for (nTab = rTab; ((SCsTAB)nTab >= 0) && !bFound; nTab--)
                    if (maTabs[nTab])
                    {
                        if (rMark.GetTableSelect(nTab))
                        {
                            bFound = maTabs[nTab]->SearchAndReplace(
                                rSearchItem, nCol, nRow, rMark, rMatchedRanges, rUndoStr, pUndoDoc);
                            if (bFound)
                            {
                                rCol = nCol;
                                rRow = nRow;
                                rTab = nTab;
                            }
                            else
                                ScDocument::GetSearchAndReplaceStart(
                                    rSearchItem, nCol, nRow );
                        }
                    }
            }
            else
            {
                for (nTab = rTab; (nTab < static_cast<SCTAB>(maTabs.size())) && !bFound; nTab++)
                    if (maTabs[nTab])
                    {
                        if (rMark.GetTableSelect(nTab))
                        {
                            bFound = maTabs[nTab]->SearchAndReplace(
                                rSearchItem, nCol, nRow, rMark, rMatchedRanges, rUndoStr, pUndoDoc);
                            if (bFound)
                            {
                                rCol = nCol;
                                rRow = nRow;
                                rTab = nTab;
                            }
                            else
                                ScDocument::GetSearchAndReplaceStart(
                                    rSearchItem, nCol, nRow );
                        }
                    }
            }
        }
    }
    return bFound;
}

//  Outline anpassen

bool ScDocument::UpdateOutlineCol( SCCOL nStartCol, SCCOL nEndCol, SCTAB nTab, bool bShow )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->UpdateOutlineCol( nStartCol, nEndCol, bShow );

    OSL_FAIL("missing tab");
    return false;
}

bool ScDocument::UpdateOutlineRow( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, bool bShow )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->UpdateOutlineRow( nStartRow, nEndRow, bShow );

    OSL_FAIL("missing tab");
    return false;
}

void ScDocument::Sort(SCTAB nTab, const ScSortParam& rSortParam, bool bKeepQuery, ScProgress* pProgress)
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
    {
        bool bOldDisableIdle = IsIdleDisabled();
        DisableIdle( true );
        maTabs[nTab]->Sort(rSortParam, bKeepQuery, pProgress);
        DisableIdle( bOldDisableIdle );
    }
}

SCSIZE ScDocument::Query(SCTAB nTab, const ScQueryParam& rQueryParam, bool bKeepSub)
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->Query((ScQueryParam&)rQueryParam, bKeepSub);

    OSL_FAIL("missing tab");
    return 0;
}


void ScDocument::GetUpperCellString(SCCOL nCol, SCROW nRow, SCTAB nTab, rtl::OUString& rStr)
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->GetUpperCellString( nCol, nRow, rStr );
    else
        rStr = rtl::OUString();
}

bool ScDocument::CreateQueryParam(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, SCTAB nTab, ScQueryParam& rQueryParam)
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->CreateQueryParam(nCol1, nRow1, nCol2, nRow2, rQueryParam);

    OSL_FAIL("missing tab");
    return false;
}

bool ScDocument::HasAutoFilter( SCCOL nCurCol, SCROW nCurRow, SCTAB nCurTab )
{
    const ScDBData* pDBData = GetDBAtCursor( nCurCol, nCurRow, nCurTab );
    bool bHasAutoFilter = (pDBData != NULL);

    if ( pDBData )
    {
        if ( pDBData->HasHeader() )
        {
            SCCOL nCol;
            SCROW nRow;
            sal_Int16  nFlag;

            ScQueryParam aParam;
            pDBData->GetQueryParam( aParam );
            nRow = aParam.nRow1;

            for ( nCol=aParam.nCol1; nCol<=aParam.nCol2 && bHasAutoFilter; nCol++ )
            {
                nFlag = ((ScMergeFlagAttr*)
                            GetAttr( nCol, nRow, nCurTab, ATTR_MERGE_FLAG ))->
                                GetValue();

                if ( (nFlag & SC_MF_AUTO) == 0 )
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
    return VALIDTAB(nTab) && maTabs[nTab] && maTabs[nTab]->HasColHeader( nStartCol, nStartRow, nEndCol, nEndRow );
}

bool ScDocument::HasRowHeader( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                    SCTAB nTab )
{
    return VALIDTAB(nTab) && maTabs[nTab] && maTabs[nTab]->HasRowHeader( nStartCol, nStartRow, nEndCol, nEndRow );
}

//
//  GetFilterEntries - Eintraege fuer AutoFilter-Listbox
//

bool ScDocument::GetFilterEntries(
    SCCOL nCol, SCROW nRow, SCTAB nTab, bool bFilter, std::vector<ScTypedStrData>& rStrings, bool& rHasDates)
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] && pDBCollection )
    {
        ScDBData* pDBData = pDBCollection->GetDBAtCursor(nCol, nRow, nTab, false);  //!??
        if (pDBData)
        {
            pDBData->ExtendDataArea(this);
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

            // return all filter entries, if a filter condition is connected with a boolean OR
            if ( bFilter )
            {
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
            }

            if ( bFilter )
            {
                maTabs[nTab]->GetFilteredFilterEntries( nCol, nStartRow, nEndRow, aParam, rStrings, rHasDates );
            }
            else
            {
                maTabs[nTab]->GetFilterEntries( nCol, nStartRow, nEndRow, rStrings, rHasDates );
            }

            sortAndRemoveDuplicates(rStrings, aParam.bCaseSens);
            return true;
        }
    }

    return false;
}

//
//  GetFilterEntriesArea - Eintraege fuer Filter-Dialog
//

bool ScDocument::GetFilterEntriesArea(
    SCCOL nCol, SCROW nStartRow, SCROW nEndRow, SCTAB nTab, bool bCaseSens,
    std::vector<ScTypedStrData>& rStrings, bool& rHasDates)
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
    {
        maTabs[nTab]->GetFilterEntries( nCol, nStartRow, nEndRow, rStrings, rHasDates );
        sortAndRemoveDuplicates(rStrings, bCaseSens);
        return true;
    }

    return false;
}

//
//  GetDataEntries - Eintraege fuer Auswahlliste-Listbox (keine Zahlen / Formeln)
//

bool ScDocument::GetDataEntries(
    SCCOL nCol, SCROW nRow, SCTAB nTab, bool bCaseSens,
    std::vector<ScTypedStrData>& rStrings, bool bLimit )
{
    if( !bLimit )
    {
        /*  Try to generate the list from list validation. This part is skipped,
            if bLimit==true, because in that case this function is called to get
            cell values for auto completion on input. */
        sal_uInt32 nValidation = static_cast< const SfxUInt32Item* >( GetAttr( nCol, nRow, nTab, ATTR_VALIDDATA ) )->GetValue();
        if( nValidation )
        {
            const ScValidationData* pData = GetValidationEntry( nValidation );
            if( pData && pData->FillSelectionList( rStrings, ScAddress( nCol, nRow, nTab ) ) )
            {
                if (pData->GetListType() == ValidListType::SORTEDASCENDING)
                    sortAndRemoveDuplicates(rStrings, bCaseSens);

                return true;
            }
        }
    }

    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()))
        return false;

    if (!maTabs[nTab])
        return false;

    std::set<ScTypedStrData> aStrings;
    bool bRet = maTabs[nTab]->GetDataEntries(nCol, nRow, aStrings, bLimit);
    rStrings.insert(rStrings.end(), aStrings.begin(), aStrings.end());
    sortAndRemoveDuplicates(rStrings, bCaseSens);

    return bRet;
}

//
//  GetFormulaEntries - Eintraege fuer Formel-AutoEingabe
//

bool ScDocument::GetFormulaEntries( ScTypedCaseStrSet& rStrings )
{
    //
    //  Bereichsnamen
    //

    if ( pRangeName )
    {
        ScRangeName::const_iterator itr = pRangeName->begin(), itrEnd = pRangeName->end();
        for (; itr != itrEnd; ++itr)
            rStrings.insert(ScTypedStrData(itr->second->GetName(), 0.0, ScTypedStrData::Name));
    }

    //
    //  Datenbank-Bereiche
    //

    if ( pDBCollection )
    {
        const ScDBCollection::NamedDBs& rDBs = pDBCollection->getNamedDBs();
        ScDBCollection::NamedDBs::const_iterator itr = rDBs.begin(), itrEnd = rDBs.end();
        for (; itr != itrEnd; ++itr)
            rStrings.insert(ScTypedStrData(itr->GetName(), 0.0, ScTypedStrData::DbName));
    }

    //
    //  Inhalte von Beschriftungsbereichen
    //

    ScRangePairList* pLists[2];
    pLists[0] = GetColNameRanges();
    pLists[1] = GetRowNameRanges();
    for (sal_uInt16 nListNo=0; nListNo<2; nListNo++)
    {
        ScRangePairList* pList = pLists[ nListNo ];
        if (pList)
            for ( size_t i = 0, nPairs = pList->size(); i < nPairs; ++i )
            {
                ScRangePair* pPair = (*pList)[i];
                ScRange aRange = pPair->GetRange(0);
                ScCellIterator aIter( this, aRange );
                for ( ScBaseCell* pCell = aIter.GetFirst(); pCell; pCell = aIter.GetNext() )
                    if ( pCell->HasStringData() )
                    {
                        rtl::OUString aStr = pCell->GetStringData();
                        rStrings.insert(ScTypedStrData(aStr, 0.0, ScTypedStrData::Header));
                    }
            }
    }

    return true;
}


bool ScDocument::IsEmbedded() const
{
    return bIsEmbedded;
}

void ScDocument::GetEmbedded( ScRange& rRange ) const
{
    rRange = aEmbedRange;
}

Rectangle ScDocument::GetEmbeddedRect() const                       // 1/100 mm
{
    Rectangle aRect;
    ScTable* pTable = NULL;
    if ( aEmbedRange.aStart.Tab() < static_cast<SCTAB>(maTabs.size()) )
        pTable = maTabs[aEmbedRange.aStart.Tab()];
    else
        OSL_FAIL("table out of range");
    if (!pTable)
    {
        OSL_FAIL("GetEmbeddedRect ohne Tabelle");
    }
    else
    {
        SCCOL i;

        for (i=0; i<aEmbedRange.aStart.Col(); i++)
            aRect.Left() += pTable->GetColWidth(i);
        aRect.Top() += pTable->GetRowHeight( 0, aEmbedRange.aStart.Row() - 1);
        aRect.Right() = aRect.Left();
        for (i=aEmbedRange.aStart.Col(); i<=aEmbedRange.aEnd.Col(); i++)
            aRect.Right() += pTable->GetColWidth(i);
        aRect.Bottom() = aRect.Top();
        aRect.Bottom() += pTable->GetRowHeight( aEmbedRange.aStart.Row(), aEmbedRange.aEnd.Row());

        aRect.Left()   = (long) ( aRect.Left()   * HMM_PER_TWIPS );
        aRect.Right()  = (long) ( aRect.Right()  * HMM_PER_TWIPS );
        aRect.Top()    = (long) ( aRect.Top()    * HMM_PER_TWIPS );
        aRect.Bottom() = (long) ( aRect.Bottom() * HMM_PER_TWIPS );
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
static bool lcl_AddTwipsWhile( long & rTwips, long nStopTwips, SCROW & rPosY, SCROW nEndRow, const ScTable * pTable )
{
    SCROW nRow = rPosY;
    bool bAdded = false;
    bool bStop = false;
    while (rTwips < nStopTwips && nRow <= nEndRow && !bStop)
    {
        SCROW nHeightEndRow;
        sal_uInt16 nHeight = pTable->GetRowHeight( nRow, NULL, &nHeightEndRow);
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
                nAdd = nHeight * nRows;
                // We're looking for a value that satisfies loop condition.
                if (nAdd + rTwips >= nStopTwips)
                {
                    --nRows;
                    nAdd -= nHeight;
                }
                bStop = true;
            }
            rTwips += static_cast<long>(nAdd);
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

ScRange ScDocument::GetRange( SCTAB nTab, const Rectangle& rMMRect ) const
{
    ScTable* pTable = NULL;
    if (nTab < static_cast<SCTAB>(maTabs.size()))
        pTable = maTabs[nTab];
    else
        OSL_FAIL("table out of range");
    if (!pTable)
    {
        OSL_FAIL("GetRange ohne Tabelle");
        return ScRange();
    }

    Rectangle aPosRect = rMMRect;
    if ( IsNegativePage( nTab ) )
        ScDrawLayer::MirrorRectRTL( aPosRect );         // always with positive (LTR) values

    long nSize;
    long nTwips;
    long nAdd;
    bool bEnd;

    nSize = 0;
    nTwips = (long) (aPosRect.Left() / HMM_PER_TWIPS);

    SCCOL nX1 = 0;
    bEnd = false;
    while (!bEnd)
    {
        nAdd = (long) pTable->GetColWidth(nX1);
        if (nSize+nAdd <= nTwips+1 && nX1<MAXCOL)
        {
            nSize += nAdd;
            ++nX1;
        }
        else
            bEnd = true;
    }

    nTwips = (long) (aPosRect.Right() / HMM_PER_TWIPS);

    SCCOL nX2 = nX1;
    bEnd = false;
    while (!bEnd)
    {
        nAdd = (long) pTable->GetColWidth(nX2);
        if (nSize+nAdd < nTwips && nX2<MAXCOL)
        {
            nSize += nAdd;
            ++nX2;
        }
        else
            bEnd = true;
    }


    nSize = 0;
    nTwips = (long) (aPosRect.Top() / HMM_PER_TWIPS);

    SCROW nY1 = 0;
    // Was if(nSize+nAdd<=nTwips+1) inside loop => if(nSize+nAdd<nTwips+2)
    if (lcl_AddTwipsWhile( nSize, nTwips+2, nY1, MAXROW, pTable) && nY1 < MAXROW)
        ++nY1;  // original loop ended on last matched +1 unless that was MAXROW

    nTwips = (long) (aPosRect.Bottom() / HMM_PER_TWIPS);

    SCROW nY2 = nY1;
    // Was if(nSize+nAdd<nTwips) inside loop => if(nSize+nAdd<nTwips)
    if (lcl_AddTwipsWhile( nSize, nTwips, nY2, MAXROW, pTable) && nY2 < MAXROW)
        ++nY2;  // original loop ended on last matched +1 unless that was MAXROW

    return ScRange( nX1,nY1,nTab, nX2,nY2,nTab );
}

void ScDocument::SetEmbedded( const Rectangle& rRect )          // aus VisArea (1/100 mm)
{
    bIsEmbedded = true;
    aEmbedRange = GetRange( nVisibleTab, rRect );
}

//  VisArea auf Zellgrenzen anpassen

static void lcl_SnapHor( ScTable* pTable, long& rVal, SCCOL& rStartCol )
{
    SCCOL nCol = 0;
    long nTwips = (long) (rVal / HMM_PER_TWIPS);
    long nSnap = 0;
    while ( nCol<MAXCOL )
    {
        long nAdd = pTable->GetColWidth(nCol);
        if ( nSnap + nAdd/2 < nTwips || nCol < rStartCol )
        {
            nSnap += nAdd;
            ++nCol;
        }
        else
            break;
    }
    rVal = (long) ( nSnap * HMM_PER_TWIPS );
    rStartCol = nCol;
}

static void lcl_SnapVer( ScTable* pTable, long& rVal, SCROW& rStartRow )
{
    SCROW nRow = 0;
    long nTwips = (long) (rVal / HMM_PER_TWIPS);
    long nSnap = 0;

    bool bFound = false;
    for (SCROW i = nRow; i <= MAXROW; ++i)
    {
        SCROW nLastRow;
        if (pTable->RowHidden(i, NULL, &nLastRow))
        {
            i = nLastRow;
            continue;
        }

        nRow = i;
        long nAdd = pTable->GetRowHeight(i);
        if ( nSnap + nAdd/2 < nTwips || nRow < rStartRow )
        {
            nSnap += nAdd;
            ++nRow;
        }
        else
        {
            bFound = true;
            break;
        }
    }
    if (!bFound)
        nRow = MAXROW;  // all hidden down to the bottom

    rVal = (long) ( nSnap * HMM_PER_TWIPS );
    rStartRow = nRow;
}

void ScDocument::SnapVisArea( Rectangle& rRect ) const
{
    ScTable* pTable = NULL;
    if (nVisibleTab < static_cast<SCTAB>(maTabs.size()))
        pTable = maTabs[nVisibleTab];
    else
        OSL_FAIL("table out of range");
    if (!pTable)
    {
        OSL_FAIL("SetEmbedded ohne Tabelle");
        return;
    }

    bool bNegativePage = IsNegativePage( nVisibleTab );
    if ( bNegativePage )
        ScDrawLayer::MirrorRectRTL( rRect );        // calculate with positive (LTR) values

    SCCOL nCol = 0;
    lcl_SnapHor( pTable, rRect.Left(), nCol );
    ++nCol;                                         // mindestens eine Spalte
    lcl_SnapHor( pTable, rRect.Right(), nCol );

    SCROW nRow = 0;
    lcl_SnapVer( pTable, rRect.Top(), nRow );
    ++nRow;                                         // mindestens eine Zeile
    lcl_SnapVer( pTable, rRect.Bottom(), nRow );

    if ( bNegativePage )
        ScDrawLayer::MirrorRectRTL( rRect );        // back to real rectangle
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
        pDocProtection.reset(NULL);
}

bool ScDocument::IsDocProtected() const
{
    return pDocProtection.get() && pDocProtection->isProtected();
}

bool ScDocument::IsDocEditable() const
{
    // import into read-only document is possible
    return !IsDocProtected() && ( bImportingXML || mbChangeReadOnlyEnabled || !pShell || !pShell->IsReadOnly() );
}

bool ScDocument::IsTabProtected( SCTAB nTab ) const
{
    if (VALIDTAB(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->IsProtected();

    OSL_FAIL("Falsche Tabellennummer");
    return false;
}

ScTableProtection* ScDocument::GetTabProtection( SCTAB nTab ) const
{
    if (VALIDTAB(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetProtection();

    return NULL;
}

void ScDocument::SetTabProtection(SCTAB nTab, const ScTableProtection* pProtect)
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()))
        return;

    maTabs[nTab]->SetProtection(pProtect);
}

void ScDocument::CopyTabProtection(SCTAB nTabSrc, SCTAB nTabDest)
{
    if (!ValidTab(nTabSrc) || nTabSrc >= static_cast<SCTAB>(maTabs.size()) || nTabDest >= static_cast<SCTAB>(maTabs.size()) || !ValidTab(nTabDest))
        return;

    maTabs[nTabDest]->SetProtection( maTabs[nTabSrc]->GetProtection() );
}

const ScDocOptions& ScDocument::GetDocOptions() const
{
    OSL_ENSURE( pDocOptions, "No DocOptions! :-(" );
    return *pDocOptions;
}

void ScDocument::SetDocOptions( const ScDocOptions& rOpt )
{
    OSL_ENSURE( pDocOptions, "No DocOptions! :-(" );

    *pDocOptions = rOpt;
    xPoolHelper->SetFormTableOpt(rOpt);
}

const ScViewOptions& ScDocument::GetViewOptions() const
{
    OSL_ENSURE( pViewOptions, "No ViewOptions! :-(" );
    return *pViewOptions;
}

void ScDocument::SetViewOptions( const ScViewOptions& rOpt )
{
    OSL_ENSURE( pViewOptions, "No ViewOptions! :-(" );
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
    if ( xPoolHelper.is() )
    {
        ScDocumentPool* pPool = xPoolHelper->GetDocPool();
        pPool->SetPoolDefaultItem( SvxLanguageItem( eLanguage, ATTR_FONT_LANGUAGE ) );
        pPool->SetPoolDefaultItem( SvxLanguageItem( eCjkLanguage, ATTR_CJK_FONT_LANGUAGE ) );
        pPool->SetPoolDefaultItem( SvxLanguageItem( eCtlLanguage, ATTR_CTL_FONT_LANGUAGE ) );
    }

    UpdateDrawLanguages();      // set edit engine defaults in drawing layer pool
}

void ScDocument::SetDrawDefaults()
{
    bSetDrawDefaults = true;
    UpdateDrawDefaults();
}

Rectangle ScDocument::GetMMRect( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, SCTAB nTab ) const
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
    {
        OSL_FAIL("GetMMRect: falsche Tabelle");
        return Rectangle(0,0,0,0);
    }

    SCCOL i;
    Rectangle aRect;

    for (i=0; i<nStartCol; i++)
        aRect.Left() += GetColWidth(i,nTab);
    aRect.Top() += GetRowHeight( 0, nStartRow-1, nTab);

    aRect.Right()  = aRect.Left();
    aRect.Bottom() = aRect.Top();

    for (i=nStartCol; i<=nEndCol; i++)
        aRect.Right() += GetColWidth(i,nTab);
    aRect.Bottom() += GetRowHeight( nStartRow, nEndRow, nTab);

    aRect.Left()    = (long)(aRect.Left()   * HMM_PER_TWIPS);
    aRect.Right()   = (long)(aRect.Right()  * HMM_PER_TWIPS);
    aRect.Top()     = (long)(aRect.Top()    * HMM_PER_TWIPS);
    aRect.Bottom()  = (long)(aRect.Bottom() * HMM_PER_TWIPS);

    if ( IsNegativePage( nTab ) )
        ScDrawLayer::MirrorRectRTL( aRect );

    return aRect;
}

void ScDocument::SetExtDocOptions( ScExtDocOptions* pNewOptions )
{
    delete pExtDocOptions;
    pExtDocOptions = pNewOptions;
}

void ScDocument::DoMergeContents( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                    SCCOL nEndCol, SCROW nEndRow )
{
    rtl::OUString aEmpty;
    rtl::OUStringBuffer aTotal;
    rtl::OUString aCellStr;
    SCCOL nCol;
    SCROW nRow;
    for (nRow=nStartRow; nRow<=nEndRow; nRow++)
        for (nCol=nStartCol; nCol<=nEndCol; nCol++)
        {
            GetString(nCol,nRow,nTab,aCellStr);
            if (!aCellStr.isEmpty())
            {
                if (aTotal.getLength())
                    aTotal.append(' ');
                aTotal.append(aCellStr);
            }
            if (nCol != nStartCol || nRow != nStartRow)
                SetString(nCol,nRow,nTab,aEmpty);
        }

    SetString(nStartCol,nStartRow,nTab,aTotal.makeStringAndClear());
}

void ScDocument::DoMerge( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                    SCCOL nEndCol, SCROW nEndRow, bool bDeleteCaptions )
{
    ScMergeAttr aAttr( nEndCol-nStartCol+1, nEndRow-nStartRow+1 );
    ApplyAttr( nStartCol, nStartRow, nTab, aAttr );

    if ( nEndCol > nStartCol )
        ApplyFlagsTab( nStartCol+1, nStartRow, nEndCol, nStartRow, nTab, SC_MF_HOR );
    if ( nEndRow > nStartRow )
        ApplyFlagsTab( nStartCol, nStartRow+1, nStartCol, nEndRow, nTab, SC_MF_VER );
    if ( nEndCol > nStartCol && nEndRow > nStartRow )
        ApplyFlagsTab( nStartCol+1, nStartRow+1, nEndCol, nEndRow, nTab, SC_MF_HOR | SC_MF_VER );

    // remove all covered notes (removed captions are collected by drawing undo if active)
    sal_uInt16 nDelFlag = IDF_NOTE | (bDeleteCaptions ? 0 : IDF_NOCAPTIONS);
    if( nStartCol < nEndCol )
        DeleteAreaTab( nStartCol + 1, nStartRow, nEndCol, nStartRow, nTab, nDelFlag );
    if( nStartRow < nEndRow )
        DeleteAreaTab( nStartCol, nStartRow + 1, nEndCol, nEndRow, nTab, nDelFlag );
}

void ScDocument::RemoveMerge( SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    const ScMergeAttr* pAttr = (const ScMergeAttr*)
                                    GetAttr( nCol, nRow, nTab, ATTR_MERGE );

    if ( pAttr->GetColMerge() <= 1 && pAttr->GetRowMerge() <= 1 )
        return;

    SCCOL nEndCol = nCol + pAttr->GetColMerge() - 1;
    SCROW nEndRow = nRow + pAttr->GetRowMerge() - 1;

    RemoveFlagsTab( nCol, nRow, nEndCol, nEndRow, nTab, SC_MF_HOR | SC_MF_VER );

    const ScMergeAttr* pDefAttr = (const ScMergeAttr*)
                                        &xPoolHelper->GetDocPool()->GetDefaultItem( ATTR_MERGE );
    ApplyAttr( nCol, nRow, nTab, *pDefAttr );
}

void ScDocument::ExtendPrintArea( OutputDevice* pDev, SCTAB nTab,
                    SCCOL nStartCol, SCROW nStartRow, SCCOL& rEndCol, SCROW nEndRow ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->ExtendPrintArea( pDev, nStartCol, nStartRow, rEndCol, nEndRow );
}

void ScDocument::IncSizeRecalcLevel( SCTAB nTab )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->IncRecalcLevel();
}

void ScDocument::DecSizeRecalcLevel( SCTAB nTab, bool bUpdateNoteCaptionPos )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->DecRecalcLevel( bUpdateNoteCaptionPos );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
