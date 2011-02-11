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
#include "precompiled_svx.hxx"

// #i13033#
// New mechanism to hold a ist of all original and cloned objects for later
// re-creating the connections for contained connectors
#include <clonelist.hxx>
#include <svx/svdoedge.hxx>
#include <svx/scene3d.hxx>

CloneList::CloneList()
{
}

CloneList::~CloneList()
{
}

void CloneList::AddPair(const SdrObject* pOriginal, SdrObject* pClone)
{
    maOriginalList.Insert((SdrObject*)pOriginal, LIST_APPEND);
    maCloneList.Insert(pClone, LIST_APPEND);

    // look for subobjects, too.
    sal_Bool bOriginalIsGroup(pOriginal->IsGroupObject());
    sal_Bool bCloneIsGroup(pClone->IsGroupObject());

    if(bOriginalIsGroup && pOriginal->ISA(E3dObject) && !pOriginal->ISA(E3dScene))
        bOriginalIsGroup = sal_False;

    if(bCloneIsGroup && pClone->ISA(E3dObject) && !pClone->ISA(E3dScene))
        bCloneIsGroup = sal_False;

    if(bOriginalIsGroup && bCloneIsGroup)
    {
        const SdrObjList* pOriginalList = pOriginal->GetSubList();
        SdrObjList* pCloneList = pClone->GetSubList();

        if(pOriginalList && pCloneList
            && pOriginalList->GetObjCount() == pCloneList->GetObjCount())
        {
            for(sal_uInt32 a(0); a < pOriginalList->GetObjCount(); a++)
            {
                // recursive call
                AddPair(pOriginalList->GetObj(a), pCloneList->GetObj(a));
            }
        }
    }
}

sal_uInt32 CloneList::Count() const
{
    return maOriginalList.Count();
}

const SdrObject* CloneList::GetOriginal(sal_uInt32 nIndex) const
{
    return (SdrObject*)maOriginalList.GetObject(nIndex);
}

SdrObject* CloneList::GetClone(sal_uInt32 nIndex) const
{
    return (SdrObject*)maCloneList.GetObject(nIndex);
}

void CloneList::CopyConnections() const
{
    for(sal_uInt32 a(0); a < maOriginalList.Count(); a++)
    {
        const SdrEdgeObj* pOriginalEdge = PTR_CAST(SdrEdgeObj, GetOriginal(a));
        SdrEdgeObj* pCloneEdge = PTR_CAST(SdrEdgeObj, GetClone(a));

        if(pOriginalEdge && pCloneEdge)
        {
            SdrObject* pOriginalNode1 = pOriginalEdge->GetConnectedNode(sal_True);
            SdrObject* pOriginalNode2 = pOriginalEdge->GetConnectedNode(sal_False);

            if(pOriginalNode1)
            {
                sal_uLong nPos(maOriginalList.GetPos(pOriginalNode1));

                if(LIST_ENTRY_NOTFOUND != nPos)
                {
                    if(pOriginalEdge->GetConnectedNode(sal_True) != GetClone(nPos))
                    {
                        pCloneEdge->ConnectToNode(sal_True, GetClone(nPos));
                    }
                }
            }

            if(pOriginalNode2)
            {
                sal_uLong nPos(maOriginalList.GetPos(pOriginalNode2));

                if(LIST_ENTRY_NOTFOUND != nPos)
                {
                    if(pOriginalEdge->GetConnectedNode(sal_False) != GetClone(nPos))
                    {
                        pCloneEdge->ConnectToNode(sal_False, GetClone(nPos));
                    }
                }
            }
        }
    }
}

// eof
