/*************************************************************************
 *
 *  $RCSfile: clonelist.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-24 14:48:04 $
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

// #i13033#
// New mechanism to hold a ist of all original and cloned objects for later
// re-creating the connections for contained connectors

#ifndef _CLONELIST_HXX_
#include <clonelist.hxx>
#endif

#ifndef _SVDOEDGE_HXX
#include <svdoedge.hxx>
#endif

#ifndef _E3D_SCENE3D_HXX
#include "scene3d.hxx"
#endif

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
                sal_uInt32 nPos(maOriginalList.GetPos(pOriginalNode1));

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
                sal_uInt32 nPos(maOriginalList.GetPos(pOriginalNode2));

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
