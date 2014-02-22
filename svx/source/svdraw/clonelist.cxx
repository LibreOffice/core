/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */





#include <clonelist.hxx>
#include <svx/svdoedge.hxx>
#include <svx/scene3d.hxx>

void CloneList::AddPair(const SdrObject* pOriginal, SdrObject* pClone)
{
    maOriginalList.push_back(pOriginal);
    maCloneList.push_back(pClone);

    
    bool bOriginalIsGroup(pOriginal->IsGroupObject());
    bool bCloneIsGroup(pClone->IsGroupObject());

    if(bOriginalIsGroup && pOriginal->ISA(E3dObject) && !pOriginal->ISA(E3dScene))
        bOriginalIsGroup = false;

    if(bCloneIsGroup && pClone->ISA(E3dObject) && !pClone->ISA(E3dScene))
        bCloneIsGroup = false;

    if(bOriginalIsGroup && bCloneIsGroup)
    {
        const SdrObjList* pOriginalList = pOriginal->GetSubList();
        SdrObjList* pCloneList = pClone->GetSubList();

        if(pOriginalList && pCloneList
            && pOriginalList->GetObjCount() == pCloneList->GetObjCount())
        {
            for(sal_uInt32 a(0); a < pOriginalList->GetObjCount(); a++)
            {
                
                AddPair(pOriginalList->GetObj(a), pCloneList->GetObj(a));
            }
        }
    }
}

const SdrObject* CloneList::GetOriginal(sal_uInt32 nIndex) const
{
    return maOriginalList[nIndex];
}

SdrObject* CloneList::GetClone(sal_uInt32 nIndex) const
{
    return maCloneList[nIndex];
}

void CloneList::CopyConnections() const
{
    sal_uInt32 cloneCount = maCloneList.size();

    for(sal_uInt32 a = 0; a < maOriginalList.size(); a++)
    {
        const SdrEdgeObj* pOriginalEdge = PTR_CAST(SdrEdgeObj, GetOriginal(a));
        SdrEdgeObj* pCloneEdge = PTR_CAST(SdrEdgeObj, GetClone(a));

        if(pOriginalEdge && pCloneEdge)
        {
            SdrObject* pOriginalNode1 = pOriginalEdge->GetConnectedNode(true);
            SdrObject* pOriginalNode2 = pOriginalEdge->GetConnectedNode(false);

            if(pOriginalNode1)
            {
                std::vector<const SdrObject*>::const_iterator it = std::find(maOriginalList.begin(),
                                                                 maOriginalList.end(),
                                                                 pOriginalNode1);

                sal_uInt32 nPos = it - maOriginalList.begin();

                if(it != maOriginalList.end())
                {
                    SdrObject *cObj = NULL;

                    if (nPos < cloneCount)
                        cObj = GetClone(nPos);

                    if(pOriginalEdge->GetConnectedNode(true) != cObj)
                        pCloneEdge->ConnectToNode(true, cObj);
                }
            }

            if(pOriginalNode2)
            {
                std::vector<const SdrObject*>::const_iterator it = std::find(maOriginalList.begin(),
                                                                 maOriginalList.end(),
                                                                 pOriginalNode2);

                sal_uInt32 nPos = it - maOriginalList.begin();

                if(it != maOriginalList.end())
                {
                    SdrObject *cObj = NULL;

                    if (nPos < cloneCount)
                        cObj = GetClone(nPos);

                    if(pOriginalEdge->GetConnectedNode(false) != cObj)
                        pCloneEdge->ConnectToNode(false, cObj);
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
