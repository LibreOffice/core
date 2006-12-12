/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PageListWatcher.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:30:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
