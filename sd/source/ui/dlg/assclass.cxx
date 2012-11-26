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
        mpPageStatus[i] = true;
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
