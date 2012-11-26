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
#include "precompiled_svx.hxx"

#include <clonelist.hxx>
#include <svx/svdoedge.hxx>
#include <svx/scene3d.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

CloneList::CloneList()
:   maOriginalList(),
    maCloneList()
{
}

CloneList::~CloneList()
{
}

void CloneList::AddPair(const SdrObject* pOriginal, SdrObject* pClone)
{
    maOriginalList.push_back(const_cast< SdrObject* >(pOriginal));
    maCloneList.push_back(pClone);

    // look for subobjects, too.
    const bool bOriginalIsGroup(pOriginal->getChildrenOfSdrObject());
    const bool bCloneIsGroup(pClone->getChildrenOfSdrObject());

    if(bOriginalIsGroup && bCloneIsGroup)
    {
        const SdrObjList* pOriginalList = pOriginal->getChildrenOfSdrObject();
        SdrObjList* pCloneList = pClone->getChildrenOfSdrObject();

        if(pOriginalList && pCloneList && pOriginalList->GetObjCount() == pCloneList->GetObjCount())
        {
            for(sal_uInt32 a(0); a < pOriginalList->GetObjCount(); a++)
            {
                // recursive call
                AddPair(pOriginalList->GetObj(a), pCloneList->GetObj(a));
            }
        }
    }
}

void CloneList::CopyConnections() const
{
    SdrObjectVector::const_iterator aOuterOriginal(maOriginalList.begin());
    SdrObjectVector::const_iterator aOuterClone(maCloneList.begin());

    for(; aOuterOriginal != maOriginalList.begin(); aOuterOriginal++, aOuterClone++)
    {
        const SdrEdgeObj* pOriginalEdge = dynamic_cast< const SdrEdgeObj* >(*aOuterOriginal);
        SdrEdgeObj* pCloneEdge = dynamic_cast< SdrEdgeObj* >(*aOuterClone);

        if(pOriginalEdge && pCloneEdge)
        {
            SdrObject* pOriginalNode1 = pOriginalEdge->GetConnectedNode(true);

            if(pOriginalNode1)
            {
                SdrObjectVector::const_iterator aOriginal(maOriginalList.begin());
                SdrObjectVector::const_iterator aClone(maCloneList.begin());

                for(;aOriginal != maOriginalList.end(); aOriginal++, aClone++)
                {
                    if(*aOriginal == pOriginalNode1)
                    {
                        pCloneEdge->ConnectToNode(true, *aClone);
                        break;
                    }
                }
            }

            SdrObject* pOriginalNode2 = pOriginalEdge->GetConnectedNode(false);

            if(pOriginalNode2)
            {
                SdrObjectVector::const_iterator aOriginal(maOriginalList.begin());
                SdrObjectVector::const_iterator aClone(maCloneList.begin());

                for(;aOriginal != maOriginalList.end(); aOriginal++, aClone++)
                {
                    if(*aOriginal == pOriginalNode2)
                    {
                        pCloneEdge->ConnectToNode(false, *aClone);
                        break;
                    }
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
