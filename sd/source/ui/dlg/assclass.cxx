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


#include <tools/list.hxx>
#include <tools/debug.hxx>
#include <vcl/ctrl.hxx>

#include "assclass.hxx"


Assistent::Assistent(int nNoOfPages)
{
    mnPages=nNoOfPages;
    if(mnPages>MAX_PAGES)
    {
        mnPages=MAX_PAGES;
    }

    mpPageStatus = new bool[mnPages];

    for(sal_uInt8 i=0;i<mnPages;i++)
    {
        mpPages[i]=new List();
        mpPageStatus[i] = sal_True;
    }
    mnCurrentPage=1;
}



bool Assistent::InsertControl(int nDestPage,Control* pUsedControl)
{
    DBG_ASSERT( (nDestPage > 0) && (nDestPage <= mnPages), "Seite nicht vorhanden!");
    if((nDestPage>0)&&(nDestPage<=mnPages))
    {
        mpPages[nDestPage-1]->Insert(pUsedControl,LIST_APPEND);
        pUsedControl->Hide();
        pUsedControl->Disable();
        return true;
    }
    else
    {
        return false;
    }
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
        int i;
        Control* pCurControl;
        int nIndex=mnCurrentPage-1;

        for(i=0;i<(int)mpPages[nIndex]->Count();i++)
        {
            pCurControl=(Control*)mpPages[nIndex]->GetObject(i);
            pCurControl->Disable();
            pCurControl->Hide();
                //schaltet die Controls der vorherigen Seite
                //zurueck
        }
        mnCurrentPage=nPageToGo;
        nIndex=mnCurrentPage-1;
        for(i=0;i<(int)mpPages[nIndex]->Count();i++)
        {

            pCurControl=(Control*)mpPages[nIndex]->GetObject(i);
            pCurControl->Enable();
            pCurControl->Show();
                //zeigt die neue Seite im Fenster an
        }
        return true;
    }
    else
    {
        return false;
    }
}


bool Assistent::IsLastPage()
{
    if(mnCurrentPage==mnPages)
    {
        return true;
    }
    else
    {
        int nPage = mnCurrentPage+1;
        while(nPage <= mnPages && !mpPageStatus[nPage-1])
            nPage++;

        return nPage > mnPages;
    }
}


bool Assistent::IsFirstPage()
{
    if(mnCurrentPage==1)
    {
        return true;
    }
    else
    {
        int nPage = mnCurrentPage-1;
        while(nPage > 0 && !mpPageStatus[nPage-1])
            nPage--;

        return nPage == 0;
    }
}



int Assistent::GetCurrentPage()
{
    return mnCurrentPage;
}

Assistent::~Assistent()
{
    for( int i=0;i<mnPages;i++)
    {
        delete mpPages[i];
    }

    delete [] mpPageStatus;
}

bool Assistent::IsEnabled( int nPage )
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
