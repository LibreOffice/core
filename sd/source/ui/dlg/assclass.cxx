/*************************************************************************
 *
 *  $RCSfile: assclass.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

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


Assistent::Assistent(UINT8 nNoOfPages)
{
    nPages=nNoOfPages;
    if(nPages>MAX_PAGES)
    {
        nPages=MAX_PAGES;
    }

    pPageStatus = new BOOL[nPages];

    for(UINT8 i=0;i<nPages;i++)
    {
        pPages[i]=new List();
        pPageStatus[i] = TRUE;
    }
    nCurrentPage=1;
}



BOOL Assistent::InsertControl(UINT8 nDestPage,Control* pUsedControl)
{
    DBG_ASSERT( (nDestPage > 0) && (nDestPage <= nPages), "Seite nicht vorhanden!");
    if((nDestPage>0)&&(nDestPage<=nPages))
    {
        pPages[nDestPage-1]->Insert(pUsedControl,LIST_APPEND);
        pUsedControl->Hide();
        pUsedControl->Disable();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


BOOL Assistent::NextPage()
{
    if(nCurrentPage<nPages)
    {
        UINT8 nPage = nCurrentPage+1;
        while(nPage <= nPages && !pPageStatus[nPage-1])
          nPage++;

        if(nPage <= nPages)
            return GotoPage(nPage);
    }
    return FALSE;
}


BOOL Assistent::PreviousPage()
{
    if(nCurrentPage>1)
    {
        UINT8 nPage = nCurrentPage-1;
        while(nPage >= 0 && !pPageStatus[nPage-1])
            nPage--;

        if(nPage >= 0)
            return GotoPage(nPage);
    }
    return FALSE;
}


BOOL Assistent::GotoPage(const UINT8 nPageToGo)
{
    DBG_ASSERT( (nPageToGo > 0) && (nPageToGo <= nPages), "Seite nicht vorhanden!");

    if((nPageToGo>0)&&(nPageToGo<=nPages)&&pPageStatus[nPageToGo-1])
    {
        ULONG i;
        Control* pCurControl;
        UINT8 nIndex=nCurrentPage-1;

        for(i=0;i<pPages[nIndex]->Count();i++)
        {
            pCurControl=(Control*)pPages[nIndex]->GetObject(i);
            pCurControl->Disable();
            pCurControl->Hide();
                //schaltet die Controls der vorherigen Seite
                //zurueck
        }
        nCurrentPage=nPageToGo;
        nIndex=nCurrentPage-1;
        for(i=0;i<pPages[nIndex]->Count();i++)
        {

            pCurControl=(Control*)pPages[nIndex]->GetObject(i);
            pCurControl->Enable();
            pCurControl->Show();
                //zeigt die neue Seite im Fenster an
        }
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


BOOL Assistent::IsLastPage()
{
    if(nCurrentPage==nPages)
    {
        return TRUE;
    }
    else
    {
        UINT8 nPage = nCurrentPage+1;
        while(nPage <= nPages && !pPageStatus[nPage-1])
            nPage++;

        return nPage > nPages;
    }
}


BOOL Assistent::IsFirstPage()
{
    if(nCurrentPage==1)
    {
        return TRUE;
    }
    else
    {
        UINT8 nPage = nCurrentPage-1;
        while(nPage > 0 && !pPageStatus[nPage-1])
            nPage--;

        return nPage == 0;
    }
}



UINT8 Assistent::GetCurrentPage()
{
    return nCurrentPage;
}

Assistent::~Assistent()
{
    for( UINT8 i=0;i<nPages;i++)
    {
        delete pPages[i];
    }

    delete pPageStatus;
}

BOOL Assistent::IsEnabled( UINT8 nPage )
{
    DBG_ASSERT( (nPage>0) && (nPage <= nPages), "Seite nicht vorhanden!" );

    return (nPage>0) && (nPage <= nPages && pPageStatus[nPage-1]);
}

void Assistent::EnablePage( UINT8 nPage )
{
    DBG_ASSERT( (nPage>0) && (nPage <= nPages), "Seite nicht vorhanden!" );

    if((nPage>0) && (nPage < nPages && !pPageStatus[nPage-1]))
    {
        pPageStatus[nPage-1] = TRUE;
    }
}

void Assistent::DisablePage( UINT8 nPage )
{
    DBG_ASSERT( (nPage>0) && (nPage <= nPages), "Seite nicht vorhanden!" );

    if((nPage>0) && (nPage <= nPages && pPageStatus[nPage-1]))
    {
        pPageStatus[nPage-1] = FALSE;
        if(nCurrentPage == nPage)
            GotoPage(1);
    }
}
