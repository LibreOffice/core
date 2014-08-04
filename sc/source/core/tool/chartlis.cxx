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

#include <vcl/svapp.hxx>

#include "chartlis.hxx"
#include "brdcst.hxx"
#include "document.hxx"
#include "reftokenhelper.hxx"

#include <boost/checked_delete.hpp>

using namespace com::sun::star;
using ::std::vector;
using ::std::list;
using ::std::unary_function;
using ::std::for_each;

// Update chart listeners quickly, to get a similar behavior to loaded charts
// which register UNO listeners.
#define SC_CHARTTIMEOUT 10

class ScChartUnoData
{
    uno::Reference< chart::XChartDataChangeEventListener >  xListener;
    uno::Reference< chart::XChartData >                     xSource;

public:
            ScChartUnoData( const uno::Reference< chart::XChartDataChangeEventListener >& rL,
                            const uno::Reference< chart::XChartData >& rS ) :
                    xListener( rL ), xSource( rS ) {}
            ~ScChartUnoData() {}

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
                // requst to the chart.
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

boost::unordered_set<sal_uInt16>& ScChartListener::ExternalRefListener::getAllFileIds()
{
    return maFileIds;
}

ScChartListener::ScChartListener( const OUString& rName, ScDocument* pDocP,
        const ScRangeListRef& rRangeList ) :
    SvtListener(),
    mpExtRefListener(NULL),
    mpTokens(new vector<ScTokenRef>),
    maName(rName),
    pUnoData( NULL ),
    mpDoc( pDocP ),
    bUsed( false ),
    bDirty( false ),
    bSeriesRangesScheduled( false )
{
    ScRefTokenHelper::getTokensFromRangeList(*mpTokens, *rRangeList);
}

ScChartListener::ScChartListener( const OUString& rName, ScDocument* pDocP, vector<ScTokenRef>* pTokens ) :
    SvtListener(),
    mpExtRefListener(NULL),
    mpTokens(pTokens),
    maName(rName),
    pUnoData( NULL ),
    mpDoc( pDocP ),
    bUsed( false ),
    bDirty( false ),
    bSeriesRangesScheduled( false )
{
}

ScChartListener::ScChartListener( const ScChartListener& r ) :
    SvtListener(),
    mpExtRefListener(NULL),
    mpTokens(new vector<ScTokenRef>(*r.mpTokens)),
    maName(r.maName),
    pUnoData( NULL ),
    mpDoc( r.mpDoc ),
    bUsed( false ),
    bDirty( r.bDirty ),
    bSeriesRangesScheduled( r.bSeriesRangesScheduled )
{
    if ( r.pUnoData )
        pUnoData = new ScChartUnoData( *r.pUnoData );

    if (r.mpExtRefListener.get())
    {
        // Re-register this new listener for the files that the old listener
        // was listening to.

        ScExternalRefManager* pRefMgr = mpDoc->GetExternalRefManager();
        const boost::unordered_set<sal_uInt16>& rFileIds = r.mpExtRefListener->getAllFileIds();
        mpExtRefListener.reset(new ExternalRefListener(*this, mpDoc));
        boost::unordered_set<sal_uInt16>::const_iterator itr = rFileIds.begin(), itrEnd = rFileIds.end();
        for (; itr != itrEnd; ++itr)
        {
            pRefMgr->addLinkListener(*itr, mpExtRefListener.get());
            mpExtRefListener->addFileId(*itr);
        }
    }
}

ScChartListener::~ScChartListener()
{
    if ( HasBroadcaster() )
        EndListeningTo();
    delete pUnoData;

    if (mpExtRefListener.get())
    {
        // Stop listening to all external files.
        ScExternalRefManager* pRefMgr = mpDoc->GetExternalRefManager();
        const boost::unordered_set<sal_uInt16>& rFileIds = mpExtRefListener->getAllFileIds();
        boost::unordered_set<sal_uInt16>::const_iterator itr = rFileIds.begin(), itrEnd = rFileIds.end();
        for (; itr != itrEnd; ++itr)
            pRefMgr->removeLinkListener(*itr, mpExtRefListener.get());
    }
}

const OUString& ScChartListener::GetName() const
{
    return maName;
}

void ScChartListener::SetUno(
        const uno::Reference< chart::XChartDataChangeEventListener >& rListener,
        const uno::Reference< chart::XChartData >& rSource )
{
    delete pUnoData;
    pUnoData = new ScChartUnoData( rListener, rSource );
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
    if (p && (p->GetId() & SC_HINT_DATACHANGED))
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

class StartEndListening : public unary_function<ScTokenRef, void>
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
            mpDoc->StartListeningArea(rRange, &mrParent);
    }

    void endListening(const ScRange& rRange)
    {
        if (rRange.aStart == rRange.aEnd)
            mpDoc->EndListeningCell(rRange.aStart, &mrParent);
        else
            mpDoc->EndListeningArea(rRange, &mrParent);
    }
private:
    ScDocument* mpDoc;
    ScChartListener& mrParent;
    bool mbStart;
};

}

void ScChartListener::StartListeningTo()
{
    if (!mpTokens.get() || mpTokens->empty())
        // no references to listen to.
        return;

    for_each(mpTokens->begin(), mpTokens->end(), StartEndListening(mpDoc, *this, true));
}

void ScChartListener::EndListeningTo()
{
    if (!mpTokens.get() || mpTokens->empty())
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

void ScChartListener::UpdateScheduledSeriesRanges()
{
    if ( bSeriesRangesScheduled )
    {
        bSeriesRangesScheduled = false;
        UpdateSeriesRanges();
    }
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

void ScChartListener::UpdateSeriesRanges()
{
    ScRangeListRef pRangeList(new ScRangeList);
    ScRefTokenHelper::getRangeListFromTokens(*pRangeList, *mpTokens, ScAddress());
    mpDoc->SetChartRangeList(GetName(), pRangeList);
}

ScChartListener::ExternalRefListener* ScChartListener::GetExtRefListener()
{
    if (!mpExtRefListener.get())
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
        bSeriesRangesScheduled != r.bSeriesRangesScheduled ||
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

// ScChartListenerCollection
ScChartListenerCollection::RangeListenerItem::RangeListenerItem(const ScRange& rRange, ScChartHiddenRangeListener* p) :
    maRange(rRange), mpListener(p)
{
}

ScChartListenerCollection::ScChartListenerCollection( ScDocument* pDocP ) :
    pDoc( pDocP )
{
    aTimer.SetTimeoutHdl( LINK( this, ScChartListenerCollection, TimerHdl ) );
}

ScChartListenerCollection::ScChartListenerCollection(
        const ScChartListenerCollection& rColl ) :
    pDoc( rColl.pDoc )
{
    aTimer.SetTimeoutHdl( LINK( this, ScChartListenerCollection, TimerHdl ) );
}

ScChartListenerCollection::~ScChartListenerCollection()
{
    //  remove ChartListener objects before aTimer dtor is called, because
    //  ScChartListener::EndListeningTo may cause ScChartListenerCollection::StartTimer
    //  to be called if an empty ScNoteCell is deleted

    maListeners.clear();
}

void ScChartListenerCollection::StartAllListeners()
{
    ListenersType::iterator it = maListeners.begin(), itEnd = maListeners.end();
    for (; it != itEnd; ++it)
        it->second->StartListeningTo();
}

void ScChartListenerCollection::insert(ScChartListener* pListener)
{
    OUString aName = pListener->GetName();
    maListeners.insert(aName, pListener);
}

void ScChartListenerCollection::removeByName(const OUString& rName)
{
    maListeners.erase(rName);
}

ScChartListener* ScChartListenerCollection::findByName(const OUString& rName)
{
    ListenersType::iterator it = maListeners.find(rName);
    return it == maListeners.end() ? NULL : it->second;
}

const ScChartListener* ScChartListenerCollection::findByName(const OUString& rName) const
{
    ListenersType::const_iterator it = maListeners.find(rName);
    return it == maListeners.end() ? NULL : it->second;
}

bool ScChartListenerCollection::hasListeners() const
{
    return !maListeners.empty();
}

const ScChartListenerCollection::ListenersType& ScChartListenerCollection::getListeners() const
{
    return maListeners;
}

ScChartListenerCollection::ListenersType& ScChartListenerCollection::getListeners()
{
    return maListeners;
}

ScChartListenerCollection::StringSetType& ScChartListenerCollection::getNonOleObjectNames()
{
    return maNonOleObjectNames;
}

OUString ScChartListenerCollection::getUniqueName(const OUString& rPrefix) const
{
    for (sal_Int32 nNum = 1; nNum < 10000; ++nNum) // arbitrary limit to prevent infinite loop.
    {
        OUStringBuffer aBuf(rPrefix);
        aBuf.append(nNum);
        OUString aTestName = aBuf.makeStringAndClear();
        if (maListeners.find(aTestName) == maListeners.end())
            return aTestName;
    }
    return OUString();
}

void ScChartListenerCollection::ChangeListening( const OUString& rName,
        const ScRangeListRef& rRangeListRef, bool bDirty )
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
    if ( bDirty )
        pCL->SetDirty( true );
}

namespace {

class InsertChartListener : public std::unary_function<ScChartListener*, void>
{
    ScChartListenerCollection::ListenersType& mrListeners;
public:
    InsertChartListener(ScChartListenerCollection::ListenersType& rListeners) :
        mrListeners(rListeners) {}

    void operator() (ScChartListener* p)
    {
        OUString aName = p->GetName();
        mrListeners.insert(aName, p);
    }
};

}

void ScChartListenerCollection::FreeUnused()
{
    ListenersType aUsed, aUnused;

    // First, filter each listener into 'used' and 'unused' categories.
    {
        while(!maListeners.empty())
        {
            ScChartListener* p = maListeners.begin()->second;
            if (p->IsUno())
            {
                // We don't delete UNO charts; they are to be deleted separately via FreeUno().
                aUsed.transfer(maListeners.begin(), maListeners);
                continue;
            }

            if (p->IsUsed())
            {
                p->SetUsed(false);
                aUsed.transfer(maListeners.begin(), maListeners);
            }
            else
                aUnused.transfer(maListeners.begin(), maListeners);

        }
    }

    std::swap(aUsed, maListeners);
}

void ScChartListenerCollection::FreeUno( const uno::Reference< chart::XChartDataChangeEventListener >& rListener,
                                         const uno::Reference< chart::XChartData >& rSource )
{
    std::vector<ScChartListener*> aUsed, aUnused;

    // First, filter each listener into 'used' and 'unused' categories.
    {
        ListenersType::iterator it = maListeners.begin(), itEnd = maListeners.end();
        for (; it != itEnd; ++it)
        {
            ScChartListener* p = it->second;
            if (p->IsUno() && p->GetUnoListener() == rListener && p->GetUnoSource() == rSource)
                aUnused.push_back(p);
            else
                aUsed.push_back(p);
        }
    }

    // Release all pointers currently managed by the ptr_map container.
    maListeners.release().release();

    // Re-insert the listeners we need to keep.
    std::for_each(aUsed.begin(), aUsed.end(), InsertChartListener(maListeners));

    // Now, delete the ones no longer needed.
    std::for_each(aUnused.begin(), aUnused.end(), boost::checked_deleter<ScChartListener>());
}

void ScChartListenerCollection::StartTimer()
{
    aTimer.SetTimeout( SC_CHARTTIMEOUT );
    aTimer.Start();
}

IMPL_LINK_NOARG(ScChartListenerCollection, TimerHdl)
{
    if ( Application::AnyInput( VCL_INPUT_KEYBOARD ) )
    {
        aTimer.Start();
        return 0;
    }
    UpdateDirtyCharts();
    return 0;
}

void ScChartListenerCollection::UpdateDirtyCharts()
{
    ListenersType::iterator it = maListeners.begin(), itEnd = maListeners.end();
    for (; it != itEnd; ++it)
    {
        ScChartListener* p = it->second;
        if (p->IsDirty())
            p->Update();

        if (aTimer.IsActive() && !pDoc->IsImportingXML())
            break;                      // one interfered
    }
}

void ScChartListenerCollection::SetDirty()
{
    ListenersType::iterator it = maListeners.begin(), itEnd = maListeners.end();
    for (; it != itEnd; ++it)
        it->second->SetDirty(true);

    StartTimer();
}

void ScChartListenerCollection::SetDiffDirty(
            const ScChartListenerCollection& rCmp, bool bSetChartRangeLists )
{
    bool bDirty = false;
    ListenersType::iterator it = maListeners.begin(), itEnd = maListeners.end();
    for (; it != itEnd; ++it)
    {
        ScChartListener* pCL = it->second;
        OSL_ASSERT(pCL);
        const ScChartListener* pCLCmp = rCmp.findByName(pCL->GetName());
        if (!pCLCmp || *pCL != *pCLCmp)
        {
            if ( bSetChartRangeLists )
            {
                if (pCLCmp)
                {
                    const ScRangeListRef& rList1 = pCL->GetRangeList();
                    const ScRangeListRef& rList2 = pCLCmp->GetRangeList();
                    bool b1 = rList1.Is();
                    bool b2 = rList2.Is();
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
    ListenersType::iterator it = maListeners.begin(), itEnd = maListeners.end();
    for (; it != itEnd; ++it)
    {
        ScChartListener* pCL = it->second;
        const ScRangeListRef& rList = pCL->GetRangeList();
        if ( rList.Is() && rList->Intersects( rRange ) )
        {
            bDirty = true;
            pCL->SetDirty( true );
        }
    }
    if ( bDirty )
        StartTimer();

    // New hidden range listener implementation
    for (list<RangeListenerItem>::iterator itr = maHiddenListeners.begin(), itrEnd = maHiddenListeners.end();
          itr != itrEnd; ++itr)
    {
        if (itr->maRange.Intersects(rRange))
            itr->mpListener->notify();
    }
}

void ScChartListenerCollection::UpdateScheduledSeriesRanges()
{
    ListenersType::iterator it = maListeners.begin(), itEnd = maListeners.end();
    for (; it != itEnd; ++it)
        it->second->UpdateScheduledSeriesRanges();
}

void ScChartListenerCollection::UpdateChartsContainingTab( SCTAB nTab )
{
    ScRange aRange( 0, 0, nTab, MAXCOL, MAXROW, nTab );
    ListenersType::iterator it = maListeners.begin(), itEnd = maListeners.end();
    for (; it != itEnd; ++it)
        it->second->UpdateChartIntersecting(aRange);
}

bool ScChartListenerCollection::operator==( const ScChartListenerCollection& r ) const
{
    // Do not use ScStrCollection::operator==() here that uses IsEqual und Compare.
    // Use ScChartListener::operator==() instead.
    if (pDoc != r.pDoc || maListeners.size() != r.maListeners.size())
        return false;

    ListenersType::const_iterator it = maListeners.begin(), itEnd = maListeners.end();
    ListenersType::const_iterator it2 = r.maListeners.begin();
    for (; it != itEnd; ++it, ++it2)
    {
        if (*it != *it2)
            return false;
    }
    return true;
}

bool ScChartListenerCollection::operator!=( const ScChartListenerCollection& r ) const
{
    return !operator==(r);
}

void ScChartListenerCollection::StartListeningHiddenRange( const ScRange& rRange, ScChartHiddenRangeListener* pListener )
{
    RangeListenerItem aItem(rRange, pListener);
    maHiddenListeners.push_back(aItem);
}

namespace {

struct MatchListener : public ::std::unary_function<
        ScChartListenerCollection::RangeListenerItem, bool>
{
    MatchListener(const ScChartHiddenRangeListener* pMatch) :
        mpMatch(pMatch)
    {
    }

    bool operator() (const ScChartListenerCollection::RangeListenerItem& rItem) const
    {
        return mpMatch == rItem.mpListener;
    }

private:
    const ScChartHiddenRangeListener* mpMatch;
};

}
void ScChartListenerCollection::EndListeningHiddenRange( ScChartHiddenRangeListener* pListener )
{
    maHiddenListeners.remove_if(MatchListener(pListener));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
