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

#include <tools/debug.hxx>
#include <vcl/ctrl.hxx>

#include "assclass.hxx"

Assistent::Assistent(int nNoOfPages)
    : mnPages(nNoOfPages), mnCurrentPage(1)
{
    if(mnPages > MAX_PAGES)
        mnPages = MAX_PAGES;

    mpPageStatus.reset(new bool[mnPages]);

    for(int i=0; i < mnPages; ++i)
        mpPageStatus[i] = true;
}

bool Assistent::InsertControl(int nDestPage,Control* pUsedControl)
{
    DBG_ASSERT( (nDestPage > 0) && (nDestPage <= mnPages), "Seite nicht vorhanden!");

    if((nDestPage>0)&&(nDestPage<=mnPages))
    {
        maPages[nDestPage-1].push_back(pUsedControl);
        pUsedControl->Hide();
        pUsedControl->Disable();
        return true;
    }

    return false;
}

bool Assistent::NextPage()
{
    if(mnCurrentPage<mnPages)
    {
        int nPage = mnCurrentPage+1;
        while(nPage <= mnPages && !mpPageStatus[nPage-1])
          nPage++;

        if(nPage <= mnPages)
            return GotoPage(nPage);
    }

    return false;
}


bool Assistent::PreviousPage()
{
    if(mnCurrentPage>1)
    {
        int nPage = mnCurrentPage-1;
        while(nPage >= 0 && !mpPageStatus[nPage-1])
            nPage--;

        if(nPage >= 0)
            return GotoPage(nPage);
    }
    return false;
}


bool Assistent::GotoPage(const int nPageToGo)
{
    DBG_ASSERT( (nPageToGo > 0) && (nPageToGo <= mnPages), "Seite nicht vorhanden!");

    if((nPageToGo>0)&&(nPageToGo<=mnPages)&&mpPageStatus[nPageToGo-1])
    {
        int nIndex=mnCurrentPage-1;

        std::vector<Control*>::iterator iter = maPages[nIndex].begin();
        std::vector<Control*>::iterator iterEnd = maPages[nIndex].end();

        for(; iter != iterEnd; ++iter)
        {
            (*iter)->Disable();
            (*iter)->Hide();
        }

        mnCurrentPage=nPageToGo;
        nIndex=mnCurrentPage-1;

        iter = maPages[nIndex].begin();
        iterEnd = maPages[nIndex].end();

        for(; iter != iterEnd; ++iter)
        {
            (*iter)->Enable();
            (*iter)->Show();
        }

        return true;
    }

    return false;
}


bool Assistent::IsLastPage() const
{
    if(mnCurrentPage == mnPages)
        return true;

    int nPage = mnCurrentPage+1;
    while(nPage <= mnPages && !mpPageStatus[nPage-1])
        nPage++;

    return nPage > mnPages;
}


bool Assistent::IsFirstPage() const
{
    if(mnCurrentPage == 1)
        return true;

    int nPage = mnCurrentPage-1;
    while(nPage > 0 && !mpPageStatus[nPage-1])
        nPage--;

    return nPage == 0;
}

int Assistent::GetCurrentPage() const
{
    return mnCurrentPage;
}

bool Assistent::IsEnabled( int nPage ) const
{
    DBG_ASSERT( (nPage>0) && (nPage <= mnPages), "Seite nicht vorhanden!" );

    return (nPage>0) && (nPage <= mnPages && mpPageStatus[nPage-1]);
}

void Assistent::EnablePage( int nPage )
{
    DBG_ASSERT( (nPage>0) && (nPage <= mnPages), "Seite nicht vorhanden!" );

    if((nPage>0) && (nPage < mnPages && !mpPageStatus[nPage-1]))
    {
        mpPageStatus[nPage-1] = true;
    }
}

void Assistent::DisablePage( int nPage )
{
    DBG_ASSERT( (nPage>0) && (nPage <= mnPages), "Seite nicht vorhanden!" );

    if((nPage>0) && (nPage <= mnPages && mpPageStatus[nPage-1]))
    {
        mpPageStatus[nPage-1] = false;
        if(mnCurrentPage == nPage)
            GotoPage(1);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
