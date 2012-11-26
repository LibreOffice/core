/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "PageListWatcher.hxx"

#include "sdpage.hxx"
#include <tools/debug.hxx>
#include <svx/svdmodel.hxx>

//////////////////////////////////////////////////////////////////////////////
// #109538#

void ImpPageListWatcher::ImpRecreateSortedPageListOnDemand()
{
    // clear vectors
    maPageVectorStandard.clear();
    maPageVectorNotes.clear();
    mpHandoutPage = 0L;

    // build up vectors again
    const sal_uInt32 nPageCount(ImpGetPageCount());

    for(sal_uInt32 a(0L); a < nPageCount; a++)
    {
        SdPage* pCandidate = ImpGetPage(a);
        DBG_ASSERT(pCandidate, "ImpPageListWatcher::ImpRecreateSortedPageListOnDemand: Invalid PageList in Model (!)");

        switch(pCandidate->GetPageKind())
        {
            case PK_STANDARD:
            {
                maPageVectorStandard.push_back(pCandidate);
                break;
            }
            case PK_NOTES:
            {
                maPageVectorNotes.push_back(pCandidate);
                break;
            }
            case PK_HANDOUT:
            {
                DBG_ASSERT(!mpHandoutPage, "ImpPageListWatcher::ImpRecreateSortedPageListOnDemand: Two Handout pages in PageList of Model (!)");
                mpHandoutPage = pCandidate;
                break;
            }
        }
    }

    // set to valid
    mbPageListValid = sal_True;
}

ImpPageListWatcher::ImpPageListWatcher(const SdrModel& rModel)
:   mrModel(rModel),
    mpHandoutPage(0L),
    mbPageListValid(sal_False)
{
    // register as listener at SdrModel
    StartListening(const_cast< SdrModel& >(rModel));
}

ImpPageListWatcher::~ImpPageListWatcher()
{
    // unregister as listener at SdrModel
    EndListening(const_cast< SdrModel& >(mrModel));
}

SdPage* ImpPageListWatcher::GetSdPage(PageKind ePgKind, sal_uInt32 nPgNum)
{
    SdPage* pRetval(0L);

    if(!mbPageListValid)
    {
        ImpRecreateSortedPageListOnDemand();
    }

    switch(ePgKind)
    {
        case PK_STANDARD:
        {
            if( nPgNum < (sal_uInt32)maPageVectorStandard.size() )
                pRetval = maPageVectorStandard[nPgNum];
            else
            {
                DBG_ASSERT(nPgNum <= maPageVectorStandard.size(),
                    "ImpPageListWatcher::GetSdPage(PK_STANDARD): access out of range");
                DBG_WARNING2 ("    %d  > %d",
                    nPgNum, nPgNum<maPageVectorStandard.size());
            }
            break;
        }
        case PK_NOTES:
        {
            if( nPgNum < (sal_uInt32)maPageVectorNotes.size() )
                pRetval = maPageVectorNotes[nPgNum];
            else
            {
                DBG_ASSERT(nPgNum <= maPageVectorNotes.size(),
                    "ImpPageListWatcher::GetSdPage(PK_NOTES): access out of range");
                DBG_WARNING2("    %d > %d",
                    nPgNum, nPgNum<maPageVectorNotes.size());
            }
            break;
        }
        case PK_HANDOUT:
        {
//          #11420# for models used to transfer drawing shapes via clipboard its
//          ok to not have a handout page
//          DBG_ASSERT(mpHandoutPage, "ImpPageListWatcher::GetSdPage: access to non existing handout page (!)");
            DBG_ASSERT(nPgNum == 0L, "ImpPageListWatcher::GetSdPage: access to non existing handout page (!)");
            if (nPgNum == 0)
                pRetval = mpHandoutPage;
            else
            {
                DBG_ASSERT(nPgNum == 0L,
                    "ImpPageListWatcher::GetSdPage: access to non existing handout page (!)");
            }
            break;
        }
    }

    return pRetval;
}

sal_uInt32 ImpPageListWatcher::GetSdPageCount(PageKind ePgKind)
{
    sal_uInt32 nRetval(0L);

    if(!mbPageListValid)
    {
        ImpRecreateSortedPageListOnDemand();
    }

    switch(ePgKind)
    {
        case PK_STANDARD:
        {
            nRetval = maPageVectorStandard.size();
            break;
        }
        case PK_NOTES:
        {
            nRetval = maPageVectorNotes.size();
            break;
        }
        case PK_HANDOUT:
        {
            if(mpHandoutPage)
            {
                nRetval = 1L;
            }

            break;
        }
    }

    return nRetval;
}

//////////////////////////////////////////////////////////////////////////////

sal_uInt32 ImpDrawPageListWatcher::ImpGetPageCount() const
{
    return (sal_uInt32)mrModel.GetPageCount();
}

SdPage* ImpDrawPageListWatcher::ImpGetPage(sal_uInt32 nIndex) const
{
    return (SdPage*)mrModel.GetPage((sal_uInt16)nIndex);
}

ImpDrawPageListWatcher::ImpDrawPageListWatcher(const SdrModel& rModel)
:   ImpPageListWatcher(rModel)
{
}

ImpDrawPageListWatcher::~ImpDrawPageListWatcher()
{
}

void ImpDrawPageListWatcher::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    const SdrBaseHint* pSdrBaseHint = dynamic_cast< const SdrBaseHint* >(&rHint);

    if(pSdrBaseHint
        && HINT_PAGEORDERCHG == pSdrBaseHint->GetSdrHintKind()
        && pSdrBaseHint->GetSdrHintPage()
        && !pSdrBaseHint->GetSdrHintPage()->IsMasterPage())
    {
        Invalidate();
    }
}

//////////////////////////////////////////////////////////////////////////////

sal_uInt32 ImpMasterPageListWatcher::ImpGetPageCount() const
{
    return (sal_uInt32)mrModel.GetMasterPageCount();
}

SdPage* ImpMasterPageListWatcher::ImpGetPage(sal_uInt32 nIndex) const
{
    return (SdPage*)mrModel.GetMasterPage((sal_uInt16)nIndex);
}

ImpMasterPageListWatcher::ImpMasterPageListWatcher(const SdrModel& rModel)
:   ImpPageListWatcher(rModel)
{
}

ImpMasterPageListWatcher::~ImpMasterPageListWatcher()
{
}

void ImpMasterPageListWatcher::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    const SdrBaseHint* pSdrBaseHint = dynamic_cast< const SdrBaseHint* >(&rHint);

    if(pSdrBaseHint
        && HINT_PAGEORDERCHG == pSdrBaseHint->GetSdrHintKind()
        && pSdrBaseHint->GetSdrHintPage()
        && pSdrBaseHint->GetSdrHintPage()->IsMasterPage())
    {
        Invalidate();
    }
}

//////////////////////////////////////////////////////////////////////////////
// eof
