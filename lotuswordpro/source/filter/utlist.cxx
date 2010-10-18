/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "first.hxx"

namespace OpenStormBento
{

CUtListElmt::CUtListElmt(pCUtList pList)
{
    if (pList == NULL)
        cpNext = NULL;
    else InsertAfter(pList->GetLast());
}

CUtListElmt::~CUtListElmt()
{
    if (OnList())
    {
        cpPrev->cpNext = cpNext;
        cpNext->cpPrev = cpPrev;
    }
}




CUtList::~CUtList()
{
    pCUtListElmt pTerminating = GetTerminating();
    for (pCUtListElmt pCurr = GetFirst(); pCurr != pTerminating; )
    {
        pCUtListElmt pNext = pCurr->GetNext();
        pCurr->MakeNotOnList();
        pCurr = pNext;
    }
    pTerminating->SetPrev(pTerminating);
    pTerminating->SetNext(pTerminating);
}

int
CUtList::GetCount()
{
    int Count = 0;

    pCUtListElmt pTerminating = GetTerminating();
    for (pCUtListElmt pCurr = GetFirst(); pCurr != pTerminating;
                pCurr = pCurr->GetNext())
    {
        Count++;
    }
    return Count;
}

pCUtListElmt
CUtList::GetIndex(int Index)
{
    int Count = 0;

    pCUtListElmt pTerminating = GetTerminating();
    for (pCUtListElmt pCurr = GetFirst(); pCurr != pTerminating;
                pCurr = pCurr->GetNext())
    {
        if (Count == Index)
            return pCurr;
        Count++;
    }
    return NULL;
}

int
CUtList::GetIndex(pCUtListElmt pNode)
{
    int Count = 0;

    pCUtListElmt pTerminating = GetTerminating();
    for (pCUtListElmt pCurr = GetFirst(); pCurr != pTerminating;
                pCurr = pCurr->GetNext())
    {
        if (pNode == pCurr)
            break;
        Count++;
    }
    return Count;
}

// If pCurr is last item in list, returns first item in list (terminating
// item is skipped when circle around)
pCUtListElmt
CUtList::CircularGetNext(pConstCUtListElmt pCurr)
{
    pCUtListElmt pNext = pCurr->GetNext();
    if (pNext == GetTerminating())
        pNext = GetFirst();
    return pNext;
}

pCUtListElmt
CUtList::CircularGetPrev(pConstCUtListElmt pCurr)
{
    pCUtListElmt pPrev = pCurr->GetPrev();
    if (pPrev == GetTerminating())
        pPrev = GetLast();
    return pPrev;
}

// If pCurr is NULL, returns first item in list.  Otherwise, returns item
// in list after pCurr or NULL if no more items in list.  Terminating item
// is never returned
pCUtListElmt
CUtList::GetNextOrNULL(pCUtListElmt pCurr)
{
    pCUtListElmt pNext;

    if (pCurr == NULL)
        pNext = GetFirst();
    else pNext = pCurr->GetNext();
    if (pNext == GetTerminating())
        pNext = NULL;
    return pNext;
}

// If pCurr is NULL, returns last item in list.  Otherwise, returns item
// in list before pCurr or NULL if no more items in list.  Terminating item
// is never returned
pCUtListElmt
CUtList::GetPrevOrNULL(pCUtListElmt pCurr)
{
    pCUtListElmt pPrev;

    if (pCurr == NULL)
        pPrev = GetLast();
    else pPrev = pCurr->GetPrev();
    if (pPrev == GetTerminating())
        pPrev = NULL;
    return pPrev;
}

void
CUtList::Destroy()
{
    pCUtListElmt pTerminating = GetTerminating();
    for (pCUtListElmt pCurr = GetFirst(); pCurr != pTerminating; )
    {
        pCUtListElmt pNext = pCurr->GetNext();
        delete pCurr;
        pCurr = pNext;
    }
}




CUtOwningList::~CUtOwningList()
{
    Destroy();
}
}//end namespace OpenStormBento

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
