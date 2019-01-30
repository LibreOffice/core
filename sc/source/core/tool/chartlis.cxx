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

#include <memory>
#include <vcl/svapp.hxx>

#include <chartlis.hxx>
#include <brdcst.hxx>
#include <document.hxx>
#include <reftokenhelper.hxx>
#include <formula/token.hxx>
#include <com/sun/star/chart/XChartDataChangeEventListener.hpp>

using namespace com::sun::star;
using ::std::vector;
using ::std::for_each;

// Update chart listeners quickly, to get a similar behavior to loaded charts
// which register UNO listeners.

class ScChartUnoData
{
    uno::Reference< chart::XChartDataChangeEventListener >  xListener;
    uno::Reference< chart::XChartData >                     xSource;

public:
            ScChartUnoData( const uno::Reference< chart::XChartDataChangeEventListener >& rL,
                            const uno::Reference< chart::XChartData >& rS ) :
                    xListener( rL ), xSource( rS ) {}

    const uno::Reference< chart::XChartDataChangeEventListener >& GetListener() const   { return xListener; }
    const uno::Reference< chart::XChartData >& GetSource() const                        { return xSource; }
};

// ScChartListener
ScChartListener::ExternalRefListener::ExternalRefListener(ScChartListener& rParent, ScDocument* pDoc) :
    mrParent(rParent), mpDoc(pDoc)
{
}

ScChartListener::ExternalRefListener::~ExternalRefListener()
{
    if (!mpDoc || mpDoc->IsInDtorClear())
        // The document is being destroyed.  Do nothing.
        return;

    // Make sure to remove all pointers to this object.
    mpDoc->GetExternalRefManager()->removeLinkListener(this);
}

void ScChartListener::ExternalRefListener::notify(sal_uInt16 nFileId, ScExternalRefManager::LinkUpdateType eType)
{
    switch (eType)
    {
        case ScExternalRefManager::LINK_MODIFIED:
        {
            if (maFileIds.count(nFileId))
                // We are listening to this external document.  Send an update
                // request to the chart.
                mrParent.SetUpdateQueue();
        }
        break;
        case ScExternalRefManager::LINK_BROKEN:
            removeFileId(nFileId);
        break;
    }
}

void ScChartListener::ExternalRefListener::addFileId(sal_uInt16 nFileId)
{
    maFileIds.insert(nFileId);
}

void ScChartListener::ExternalRefListener::removeFileId(sal_uInt16 nFileId)
{
    maFileIds.erase(nFileId);
}

ScChartListener::ScChartListener( const OUString& rName, ScDocument* pDocP,
        const ScRangeListRef& rRangeList ) :
    SvtListener(),
    mpTokens(new vector<ScTokenRef>),
    maName(rName),
    mpDoc( pDocP ),
    bUsed( false ),
    bDirty( false )
{
    ScRefTokenHelper::getTokensFromRangeList(*mpTokens, *rRangeList);
}

ScChartListener::ScChartListener( const OUString& rName, ScDocument* pDocP, std::unique_ptr<vector<ScTokenRef>> pTokens ) :
    SvtListener(),
    mpTokens(std::move(pTokens)),
    maName(rName),
    mpDoc( pDocP ),
    bUsed( false ),
    bDirty( false )
{
}

ScChartListener::~ScChartListener()
{
    if ( HasBroadcaster() )
        EndListeningTo();
    pUnoData.reset();

    if (mpExtRefListener)
    {
        // Stop listening to all external files.
        ScExternalRefManager* pRefMgr = mpDoc->GetExternalRefManager();
        const std::unordered_set<sal_uInt16>& rFileIds = mpExtRefListener->getAllFileIds();
        for (const auto& rFileId : rFileIds)
            pRefMgr->removeLinkListener(rFileId, mpExtRefListener.get());
    }
}

void ScChartListener::SetUno(
        const uno::Reference< chart::XChartDataChangeEventListener >& rListener,
        const uno::Reference< chart::XChartData >& rSource )
{
    pUnoData.reset( new ScChartUnoData( rListener, rSource ) );
}

uno::Reference< chart::XChartDataChangeEventListener > ScChartListener::GetUnoListener() const
{
    if ( pUnoData )
        return pUnoData->GetListener();
    return uno::Reference< chart::XChartDataChangeEventListener >();
}

uno::Reference< chart::XChartData > ScChartListener::GetUnoSource() const
{
    if ( pUnoData )
        return pUnoData->GetSource();
    return uno::Reference< chart::XChartData >();
}

void ScChartListener::Notify( const SfxHint& rHint )
{
    const ScHint* p = dynamic_cast<const ScHint*>(&rHint);
    if (p && (p->GetId() == SfxHintId::ScDataChanged))
        SetUpdateQueue();
}

void ScChartListener::Update()
{
    if ( mpDoc->IsInInterpreter() )
    {   // If interpreting do nothing and restart timer so we don't
        // interfere with interpreter and don't produce an Err522 or similar.
        // This may happen if we are rescheduled via Basic function.
        mpDoc->GetChartListenerCollection()->StartTimer();
        return ;
    }
    if ( pUnoData )
    {
        bDirty = false;
        // recognize some day what has changed inside the Chart
        chart::ChartDataChangeEvent aEvent( pUnoData->GetSource(),
                                        chart::ChartDataChangeType_ALL,
                                        0, 0, 0, 0 );
        pUnoData->GetListener()->chartDataChanged( aEvent );
    }
    else if ( mpDoc->GetAutoCalc() )
    {
        bDirty = false;
        mpDoc->UpdateChart(GetName());
    }
}

ScRangeListRef ScChartListener::GetRangeList() const
{
    ScRangeListRef aRLRef(new ScRangeList);
    ScRefTokenHelper::getRangeListFromTokens(*aRLRef, *mpTokens, ScAddress());
    return aRLRef;
}

void ScChartListener::SetRangeList( const ScRangeListRef& rNew )
{
    vector<ScTokenRef> aTokens;
    ScRefTokenHelper::getTokensFromRangeList(aTokens, *rNew);
    mpTokens->swap(aTokens);
}

namespace {

class StartEndListening
{
public:
    StartEndListening(ScDocument* pDoc, ScChartListener& rParent, bool bStart) :
        mpDoc(pDoc), mrParent(rParent), mbStart(bStart) {}

    void operator() (const ScTokenRef& pToken)
    {
        if (!ScRefTokenHelper::isRef(pToken))
            return;

        bool bExternal = ScRefTokenHelper::isExternalRef(pToken);
        if (bExternal)
        {
            sal_uInt16 nFileId = pToken->GetIndex();
            ScExternalRefManager* pRefMgr = mpDoc->GetExternalRefManager();
            ScChartListener::ExternalRefListener* pExtRefListener = mrParent.GetExtRefListener();
            if (mbStart)
            {
                pRefMgr->addLinkListener(nFileId, pExtRefListener);
                pExtRefListener->addFileId(nFileId);
            }
            else
            {
                pRefMgr->removeLinkListener(nFileId, pExtRefListener);
                pExtRefListener->removeFileId(nFileId);
            }
        }
        else
        {
            ScRange aRange;
            ScRefTokenHelper::getRangeFromToken(aRange, pToken, ScAddress(), bExternal);
            if (mbStart)
                startListening(aRange);
            else
                endListening(aRange);
        }
    }
private:
    void startListening(const ScRange& rRange)
    {
        if (rRange.aStart == rRange.aEnd)
            mpDoc->StartListeningCell(rRange.aStart, &mrParent);
        else
            mpDoc->StartListeningArea(rRange, false, &mrParent);
    }

    void endListening(const ScRange& rRange)
    {
        if (rRange.aStart == rRange.aEnd)
            mpDoc->EndListeningCell(rRange.aStart, &mrParent);
        else
            mpDoc->EndListeningArea(rRange, false, &mrParent);
    }
private:
    ScDocument* mpDoc;
    ScChartListener& mrParent;
    bool const mbStart;
};

}

void ScChartListener::StartListeningTo()
{
    if (!mpTokens || mpTokens->empty())
        // no references to listen to.
        return;

    for_each(mpTokens->begin(), mpTokens->end(), StartEndListening(mpDoc, *this, true));
}

void ScChartListener::EndListeningTo()
{
    if (!mpTokens || mpTokens->empty())
        // no references to listen to.
        return;

    for_each(mpTokens->begin(), mpTokens->end(), StartEndListening(mpDoc, *this, false));
}

void ScChartListener::ChangeListening( const ScRangeListRef& rRangeListRef,
                                       bool bDirtyP )
{
    EndListeningTo();
    SetRangeList( rRangeListRef );
    StartListeningTo();
    if ( bDirtyP )
        SetDirty( true );
}

void ScChartListener::UpdateChartIntersecting( const ScRange& rRange )
{
    ScTokenRef pToken;
    ScRefTokenHelper::getTokenFromRange(pToken, rRange);

    if (ScRefTokenHelper::intersects(*mpTokens, pToken, ScAddress()))
    {
        // force update (chart has to be loaded), don't use ScChartListener::Update
        mpDoc->UpdateChart(GetName());
    }
}

ScChartListener::ExternalRefListener* ScChartListener::GetExtRefListener()
{
    if (!mpExtRefListener)
        mpExtRefListener.reset(new ExternalRefListener(*this, mpDoc));

    return mpExtRefListener.get();
}

void ScChartListener::SetUpdateQueue()
{
    bDirty = true;
    mpDoc->GetChartListenerCollection()->StartTimer();
}

bool ScChartListener::operator==( const ScChartListener& r ) const
{
    bool b1 = (mpTokens.get() && !mpTokens->empty());
    bool b2 = (r.mpTokens.get() && !r.mpTokens->empty());

    if (mpDoc != r.mpDoc || bUsed != r.bUsed || bDirty != r.bDirty ||
        GetName() != r.GetName() || b1 != b2)
        return false;

    if (!b1 && !b2)
        // both token list instances are empty.
        return true;

    return *mpTokens == *r.mpTokens;
}

bool ScChartListener::operator!=( const ScChartListener& r ) const
{
    return !operator==(r);
}

ScChartHiddenRangeListener::ScChartHiddenRangeListener()
{
}

ScChartHiddenRangeListener::~ScChartHiddenRangeListener()
{
    // empty d'tor
}

void ScChartListenerCollection::Init()
{
    aIdle.SetInvokeHandler( LINK( this, ScChartListenerCollection, TimerHdl ) );
    aIdle.SetPriority( TaskPriority::REPAINT );
    aIdle.SetDebugName( "sc::ScChartListenerCollection aIdle" );
}

ScChartListenerCollection::ScChartListenerCollection( ScDocument* pDocP ) :
    meModifiedDuringUpdate( SC_CLCUPDATE_NONE ),
    pDoc( pDocP )
{
    Init();
}

ScChartListenerCollection::ScChartListenerCollection(
        const ScChartListenerCollection& rColl ) :
    meModifiedDuringUpdate( SC_CLCUPDATE_NONE ),
    pDoc( rColl.pDoc )
{
    Init();
}

ScChartListenerCollection::~ScChartListenerCollection()
{
    //  remove ChartListener objects before aIdle dtor is called, because
    //  ScChartListener::EndListeningTo may cause ScChartListenerCollection::StartTimer
    //  to be called if an empty ScNoteCell is deleted

    m_Listeners.clear();
}

void ScChartListenerCollection::StartAllListeners()
{
    for (auto const& it : m_Listeners)
    {
        it.second->StartListeningTo();
    }
}

void ScChartListenerCollection::insert(ScChartListener* pListener)
{
    if (meModifiedDuringUpdate == SC_CLCUPDATE_RUNNING)
        meModifiedDuringUpdate =  SC_CLCUPDATE_MODIFIED;
    OUString aName = pListener->GetName();
    m_Listeners.insert(std::make_pair(aName, std::unique_ptr<ScChartListener>(pListener)));
}

void ScChartListenerCollection::removeByName(const OUString& rName)
{
    if (meModifiedDuringUpdate == SC_CLCUPDATE_RUNNING)
        meModifiedDuringUpdate =  SC_CLCUPDATE_MODIFIED;
    m_Listeners.erase(rName);
}

ScChartListener* ScChartListenerCollection::findByName(const OUString& rName)
{
    ListenersType::iterator const it = m_Listeners.find(rName);
    return it == m_Listeners.end() ? nullptr : it->second.get();
}

const ScChartListener* ScChartListenerCollection::findByName(const OUString& rName) const
{
    ListenersType::const_iterator const it = m_Listeners.find(rName);
    return it == m_Listeners.end() ? nullptr : it->second.get();
}

bool ScChartListenerCollection::hasListeners() const
{
    return !m_Listeners.empty();
}

OUString ScChartListenerCollection::getUniqueName(const OUString& rPrefix) const
{
    for (sal_Int32 nNum = 1; nNum < 10000; ++nNum) // arbitrary limit to prevent infinite loop.
    {
        OUStringBuffer aBuf(rPrefix);
        aBuf.append(nNum);
        OUString aTestName = aBuf.makeStringAndClear();
        if (m_Listeners.find(aTestName) == m_Listeners.end())
            return aTestName;
    }
    return OUString();
}

void ScChartListenerCollection::ChangeListening( const OUString& rName,
        const ScRangeListRef& rRangeListRef )
{
    ScChartListener* pCL = findByName(rName);
    if (pCL)
    {
        pCL->EndListeningTo();
        pCL->SetRangeList( rRangeListRef );
    }
    else
    {
        pCL = new ScChartListener(rName, pDoc, rRangeListRef);
        insert(pCL);
    }
    pCL->StartListeningTo();
}

void ScChartListenerCollection::FreeUnused()
{
    if (meModifiedDuringUpdate == SC_CLCUPDATE_RUNNING)
        meModifiedDuringUpdate =  SC_CLCUPDATE_MODIFIED;

    ListenersType aUsed;

    for (auto & pair : m_Listeners)
    {
        ScChartListener* p = pair.second.get();
        if (p->IsUno())
        {
            // We don't delete UNO charts; they are to be deleted separately via FreeUno().
            aUsed.insert(std::make_pair(pair.first, std::move(pair.second)));
            continue;
        }

        if (p->IsUsed())
        {
            p->SetUsed(false);
            aUsed.insert(std::make_pair(pair.first, std::move(pair.second)));
        }
    }

    m_Listeners = std::move(aUsed);
}

void ScChartListenerCollection::FreeUno( const uno::Reference< chart::XChartDataChangeEventListener >& rListener,
                                         const uno::Reference< chart::XChartData >& rSource )
{
    if (meModifiedDuringUpdate == SC_CLCUPDATE_RUNNING)
        meModifiedDuringUpdate =  SC_CLCUPDATE_MODIFIED;

    for (auto it = m_Listeners.begin(); it != m_Listeners.end(); )
    {
        ScChartListener *const p = it->second.get();
        if (p->IsUno() && p->GetUnoListener() == rListener && p->GetUnoSource() == rSource)
            it = m_Listeners.erase(it);
        else
            ++it;
    }
}

void ScChartListenerCollection::StartTimer()
{
    aIdle.Start();
}

IMPL_LINK_NOARG(ScChartListenerCollection, TimerHdl, Timer *, void)
{
    if ( Application::AnyInput( VclInputFlags::KEYBOARD ) )
    {
        aIdle.Start();
        return;
    }
    UpdateDirtyCharts();
}

void ScChartListenerCollection::UpdateDirtyCharts()
{
    // During ScChartListener::Update() the most nasty things can happen due to
    // UNO listeners, e.g. reentrant calls via BASIC to insert() and FreeUno()
    // and similar that modify m_Listeners and invalidate iterators.
    meModifiedDuringUpdate = SC_CLCUPDATE_RUNNING;

    for (auto const& it : m_Listeners)
    {
        ScChartListener *const p = it.second.get();
        if (p->IsDirty())
            p->Update();

        if (meModifiedDuringUpdate == SC_CLCUPDATE_MODIFIED)
            break;      // iterator is invalid

        if (aIdle.IsActive() && !pDoc->IsImportingXML())
            break;                      // one interfered
    }
    meModifiedDuringUpdate = SC_CLCUPDATE_NONE;
}

void ScChartListenerCollection::SetDirty()
{
    for (auto const& it : m_Listeners)
    {
        it.second->SetDirty(true);
    }

    StartTimer();
}

void ScChartListenerCollection::SetDiffDirty(
            const ScChartListenerCollection& rCmp, bool bSetChartRangeLists )
{
    bool bDirty = false;
    for (auto const& it : m_Listeners)
    {
        ScChartListener *const pCL = it.second.get();
        assert(pCL);
        const ScChartListener* pCLCmp = rCmp.findByName(pCL->GetName());
        if (!pCLCmp || *pCL != *pCLCmp)
        {
            if ( bSetChartRangeLists )
            {
                if (pCLCmp)
                {
                    const ScRangeListRef& rList1 = pCL->GetRangeList();
                    const ScRangeListRef& rList2 = pCLCmp->GetRangeList();
                    bool b1 = rList1.is();
                    bool b2 = rList2.is();
                    if ( b1 != b2 || (b1 && b2 && (*rList1 != *rList2)) )
                        pDoc->SetChartRangeList( pCL->GetName(), rList1 );
                }
                else
                    pDoc->SetChartRangeList( pCL->GetName(), pCL->GetRangeList() );
            }
            bDirty = true;
            pCL->SetDirty( true );
        }
    }
    if ( bDirty )
        StartTimer();
}

void ScChartListenerCollection::SetRangeDirty( const ScRange& rRange )
{
    bool bDirty = false;
    for (auto const& it : m_Listeners)
    {
        ScChartListener *const pCL = it.second.get();
        const ScRangeListRef& rList = pCL->GetRangeList();
        if ( rList.is() && rList->Intersects( rRange ) )
        {
            bDirty = true;
            pCL->SetDirty( true );
        }
    }
    if ( bDirty )
        StartTimer();

    // New hidden range listener implementation
    for (auto& [pListener, rHiddenRange] : maHiddenListeners)
    {
        if (rHiddenRange.Intersects(rRange))
        {
            pListener->notify();
        }
    }
}

void ScChartListenerCollection::UpdateChartsContainingTab( SCTAB nTab )
{
    ScRange aRange( 0, 0, nTab, MAXCOL, MAXROW, nTab );
    for (auto const& it : m_Listeners)
    {
        it.second->UpdateChartIntersecting(aRange);
    }
}

bool ScChartListenerCollection::operator==( const ScChartListenerCollection& r ) const
{
    // Do not use ScStrCollection::operator==() here that uses IsEqual and Compare.
    // Use ScChartListener::operator==() instead.
    if (pDoc != r.pDoc)
        return false;

    return std::equal(m_Listeners.begin(), m_Listeners.end(), r.m_Listeners.begin(), r.m_Listeners.end(),
        [](const ListenersType::value_type& lhs, const ListenersType::value_type& rhs) {
            return (lhs.first == rhs.first) && (*lhs.second == *rhs.second);
        });
}

void ScChartListenerCollection::StartListeningHiddenRange( const ScRange& rRange, ScChartHiddenRangeListener* pListener )
{
    maHiddenListeners.insert(std::make_pair<>(pListener, rRange));
}

void ScChartListenerCollection::EndListeningHiddenRange( ScChartHiddenRangeListener* pListener )
{
    auto range = maHiddenListeners.equal_range(pListener);
    maHiddenListeners.erase(range.first, range.second);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
