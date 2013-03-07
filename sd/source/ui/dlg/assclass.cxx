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
    DBG_ASSERT( (nDestPage > 0) && (nDestPage <= mnPages), "Page not aviable!");

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
    DBG_ASSERT( (nPageToGo > 0) && (nPageToGo <= mnPages), "Page not aviable!");

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
    DBG_ASSERT( (nPage>0) && (nPage <= mnPages), "Page not aviable!" );

    return (nPage>0) && (nPage <= mnPages && mpPageStatus[nPage-1]);
}

void Assistent::EnablePage( int nPage )
{
    DBG_ASSERT( (nPage>0) && (nPage <= mnPages), "Page not aviable!" );

    if((nPage>0) && (nPage < mnPages && !mpPageStatus[nPage-1]))
    {
        mpPageStatus[nPage-1] = true;
    }
}

void Assistent::DisablePage( int nPage )
{
    DBG_ASSERT( (nPage>0) && (nPage <= mnPages), "Page not aviable!" );

    if((nPage>0) && (nPage <= mnPages && mpPageStatus[nPage-1]))
    {
        mpPageStatus[nPage-1] = false;
        if(mnCurrentPage == nPage)
            GotoPage(1);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
