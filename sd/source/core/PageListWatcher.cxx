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

#include "PageListWatcher.hxx"

#include <sdpage.hxx>
#include <tools/debug.hxx>
#include <svx/svdmodel.hxx>
#include <sal/log.hxx>

void ImpPageListWatcher::ImpRecreateSortedPageListOnDemand()
{
    // clear vectors
    maPageVectorStandard.clear();
    maPageVectorNotes.clear();
    mpHandoutPage = nullptr;

    // build up vectors again
    const sal_uInt32 nPageCount(ImpGetPageCount());

    for(sal_uInt32 a(0); a < nPageCount; a++)
    {
        SdPage* pCandidate = ImpGetPage(a);
        assert(pCandidate && "ImpPageListWatcher::ImpRecreateSortedPageListOnDemand: Invalid PageList in Model (!)");

        switch(pCandidate->GetPageKind())
        {
            case PageKind::Standard:
            {
                maPageVectorStandard.push_back(pCandidate);
                break;
            }
            case PageKind::Notes:
            {
                maPageVectorNotes.push_back(pCandidate);
                break;
            }
            case PageKind::Handout:
            {
                DBG_ASSERT(!mpHandoutPage, "ImpPageListWatcher::ImpRecreateSortedPageListOnDemand: Two Handout pages in PageList of Model (!)");
                mpHandoutPage = pCandidate;
                break;
            }
        }
    }

    // set to valid
    mbPageListValid = true;
}

ImpPageListWatcher::ImpPageListWatcher(const SdrModel& rModel)
    : mrModel(rModel)
    , mpHandoutPage(nullptr)
    , mbPageListValid(false)
{
}

ImpPageListWatcher::~ImpPageListWatcher()
{
}

SdPage* ImpPageListWatcher::GetSdPage(PageKind ePgKind, sal_uInt32 nPgNum)
{
    SdPage* pRetval(nullptr);

    if(!mbPageListValid)
    {
        ImpRecreateSortedPageListOnDemand();
    }

    switch(ePgKind)
    {
        case PageKind::Standard:
        {
            if( nPgNum < static_cast<sal_uInt32>(maPageVectorStandard.size()) )
                pRetval = maPageVectorStandard[nPgNum];
            else
            {
                SAL_INFO( "sd.core",
                          "ImpPageListWatcher::GetSdPage(PageKind::Standard): page number " << nPgNum << " >= " << maPageVectorStandard.size() );
            }
            break;
        }
        case PageKind::Notes:
        {
            if( nPgNum < static_cast<sal_uInt32>(maPageVectorNotes.size()) )
                pRetval = maPageVectorNotes[nPgNum];
            else
            {
                SAL_INFO( "sd.core",
                          "ImpPageListWatcher::GetSdPage(PageKind::Notes): page number " << nPgNum << " >= " << maPageVectorNotes.size() );
            }
            break;
        }
        case PageKind::Handout:
        {
            // #11420# for models used to transfer drawing shapes via clipboard it's ok to not have a handout page
            DBG_ASSERT(nPgNum == 0, "ImpPageListWatcher::GetSdPage: access to non existing handout page (!)");
            if (nPgNum == 0)
                pRetval = mpHandoutPage;
            else
            {
                DBG_ASSERT(nPgNum == 0,
                    "ImpPageListWatcher::GetSdPage: access to non existing handout page (!)");
            }
            break;
        }
    }

    return pRetval;
}

sal_uInt32 ImpPageListWatcher::GetSdPageCount(PageKind ePgKind)
{
    sal_uInt32 nRetval(0);

    if(!mbPageListValid)
    {
        ImpRecreateSortedPageListOnDemand();
    }

    switch(ePgKind)
    {
        case PageKind::Standard:
        {
            nRetval = maPageVectorStandard.size();
            break;
        }
        case PageKind::Notes:
        {
            nRetval = maPageVectorNotes.size();
            break;
        }
        case PageKind::Handout:
        {
            if(mpHandoutPage)
            {
                nRetval = 1;
            }

            break;
        }
    }

    return nRetval;
}

sal_uInt32 ImpPageListWatcher::GetVisibleSdPageCount() const
{
    sal_uInt32 nVisiblePageCount = 0;

    // build up vectors again
    const sal_uInt32 nPageCount(ImpGetPageCount());

    for(sal_uInt32 a(0); a < nPageCount; a++)
    {
        SdPage* pCandidate = ImpGetPage(a);
        if ((pCandidate->GetPageKind() == PageKind::Standard)&&(!pCandidate->IsExcluded())) nVisiblePageCount++;
    }
    return nVisiblePageCount;
}

sal_uInt32 ImpDrawPageListWatcher::ImpGetPageCount() const
{
    return static_cast<sal_uInt32>(mrModel.GetPageCount());
}

SdPage* ImpDrawPageListWatcher::ImpGetPage(sal_uInt32 nIndex) const
{
    return const_cast<SdPage*>(static_cast<const SdPage*>(mrModel.GetPage(static_cast<sal_uInt16>(nIndex))));
}

ImpDrawPageListWatcher::ImpDrawPageListWatcher(const SdrModel& rModel)
:   ImpPageListWatcher(rModel)
{
}

ImpDrawPageListWatcher::~ImpDrawPageListWatcher()
{
}

sal_uInt32 ImpMasterPageListWatcher::ImpGetPageCount() const
{
    return static_cast<sal_uInt32>(mrModel.GetMasterPageCount());
}

SdPage* ImpMasterPageListWatcher::ImpGetPage(sal_uInt32 nIndex) const
{
    return const_cast<SdPage*>(static_cast<const SdPage*>(mrModel.GetMasterPage(static_cast<sal_uInt16>(nIndex))));
}

ImpMasterPageListWatcher::ImpMasterPageListWatcher(const SdrModel& rModel)
:   ImpPageListWatcher(rModel)
{
}

ImpMasterPageListWatcher::~ImpMasterPageListWatcher()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
