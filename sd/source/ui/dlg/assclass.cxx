/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: assclass.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:58:45 $
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


#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_CTRL_HXX //autogen
#include <vcl/ctrl.hxx>
#endif

#include "assclass.hxx"


Assistent::Assistent(int nNoOfPages)
{
    mnPages=nNoOfPages;
    if(mnPages>MAX_PAGES)
    {
        mnPages=MAX_PAGES;
    }

    mpPageStatus = new bool[mnPages];

    for(UINT8 i=0;i<mnPages;i++)
    {
        mpPages[i]=new List();
        mpPageStatus[i] = TRUE;
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
