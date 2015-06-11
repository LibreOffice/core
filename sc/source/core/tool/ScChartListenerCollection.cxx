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

#include "ScChartListenerCollection.hxx"

#include "chartlis.hxx"
#include "document.hxx"

#include "vcl/apptypes.hxx"
#include "vcl/svapp.hxx"

namespace chart = com::sun::star::chart;
namespace uno = com::sun::star::uno;

ScChartListenerCollection::RangeListenerItem::RangeListenerItem(const ScRange& rRange, ScChartHiddenRangeListener* p) :
    maRange(rRange), mpListener(p)
{
}

ScChartListenerCollection::ScChartListenerCollection( ScDocument* pDocP ) :
    meModifiedDuringUpdate( SC_CLCUPDATE_NONE ),
    pDoc( pDocP )
{
    aIdle.SetIdleHdl( LINK( this, ScChartListenerCollection, TimerHdl ) );
}

ScChartListenerCollection::ScChartListenerCollection(
        const ScChartListenerCollection& rColl ) :
    meModifiedDuringUpdate( SC_CLCUPDATE_NONE ),
    pDoc( rColl.pDoc )
{
    aIdle.SetIdleHdl( LINK( this, ScChartListenerCollection, TimerHdl ) );
}

ScChartListenerCollection::~ScChartListenerCollection()
{
    //  remove ChartListener objects before aIdle dtor is called, because
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
    if (meModifiedDuringUpdate == SC_CLCUPDATE_RUNNING)
        meModifiedDuringUpdate =  SC_CLCUPDATE_MODIFIED;
    OUString aName = pListener->GetName();
    maListeners.insert(aName, pListener);
}

void ScChartListenerCollection::removeByName(const OUString& rName)
{
    if (meModifiedDuringUpdate == SC_CLCUPDATE_RUNNING)
        meModifiedDuringUpdate =  SC_CLCUPDATE_MODIFIED;
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
    if (meModifiedDuringUpdate == SC_CLCUPDATE_RUNNING)
        meModifiedDuringUpdate =  SC_CLCUPDATE_MODIFIED;

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
    if (meModifiedDuringUpdate == SC_CLCUPDATE_RUNNING)
        meModifiedDuringUpdate =  SC_CLCUPDATE_MODIFIED;

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
    // coverity[leaked_storage] - no leak, because because we will take care of them below
    maListeners.release().release();

    // Re-insert the listeners we need to keep.
    std::for_each(aUsed.begin(), aUsed.end(), InsertChartListener(maListeners));

    // Now, delete the ones no longer needed.
    std::for_each(aUnused.begin(), aUnused.end(), boost::checked_deleter<ScChartListener>());
}

void ScChartListenerCollection::StartTimer()
{
    aIdle.SetPriority( SchedulerPriority::REPAINT );
    aIdle.Start();
}

IMPL_LINK_NOARG_TYPED(ScChartListenerCollection, TimerHdl, Idle *, void)
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
    // and similar that modify maListeners and invalidate iterators.
    meModifiedDuringUpdate = SC_CLCUPDATE_RUNNING;

    ListenersType::iterator it = maListeners.begin(), itEnd = maListeners.end();
    for (; it != itEnd; ++it)
    {
        ScChartListener* p = it->second;
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
    {
        StartTimer();
    }

    for (auto itr = maHiddenListeners.begin(); itr != maHiddenListeners.end(); ++itr)
    {
        if (itr->second.maRange.Intersects(rRange))
        {
            itr->second.mpListener->notify();
        }
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
    auto it = maHiddenListeners.find(pListener);
    assert(it == maHiddenListeners.end());
    maHiddenListeners.insert(std::make_pair<>(pListener, aItem));
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
    auto range = maHiddenListeners.equal_range(pListener);
    maHiddenListeners.erase(range.first, range.second);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
