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
}

ImpPageListWatcher::~ImpPageListWatcher()
{
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
//          #11420# for models used to transfer drawing shapes via clipboard its ok to not have a handout page
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
