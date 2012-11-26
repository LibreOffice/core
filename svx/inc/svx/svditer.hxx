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



#ifndef _SVDITER_HXX
#define _SVDITER_HXX

#include <sal/types.h>
#include <tools/list.hxx>
#include "svx/svxdllapi.h"
#include <vector>
#include <svx/svdobj.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
// predefines

class SdrObjList;

////////////////////////////////////////////////////////////////////////////////////////////////////
// defines

enum SdrIterMode
{
    IM_FLAT,                // : Flach ueber die Liste
    IM_DEEPWITHGROUPS,      // : Mit rekursivem Abstieg, Next() liefert auch Gruppenobjekte
    IM_DEEPNOGROUPS         // : Mit rekursivem Abstieg, Next() liefert keine Gruppenobjekte
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrObjListIter
{
private:
    SdrObjectVector             maObjList;
    sal_uInt32                  mnIndex;

    /// bitfield
    bool                        mbReverse : 1;

    void ImpProcessObjectList(const SdrObjList& rObjList, SdrIterMode eMode); // , bool bUseZOrder);
    void ImpProcessObj(const SdrObject& rObj, SdrIterMode eMode); // , bool bUseZOrder);

public:
    SdrObjListIter(const SdrObjList& rObjList, SdrIterMode eMode = IM_DEEPNOGROUPS, bool bReverse = false);

    /** This variant lets the user choose the order in which to travel over
        the objects.
        @param bUseZOrder
            When <true/> then the z-order defines the order of iteration.
            Otherwise the navigation position as returned by
            SdrObject::GetNavigationPosition() is used.
    */
//  SdrObjListIter(const SdrObjList& rObjList, bool bUseZOrder, SdrIterMode eMode = IM_DEEPNOGROUPS, bool bReverse = false);

    /* SJ: the following function can now be used with every
       SdrObject and is no longer limited to group objects */
    SdrObjListIter(const SdrObject& rObj, SdrIterMode eMode = IM_DEEPNOGROUPS, bool bReverse = false);

    void Reset() { mnIndex = (mbReverse ? maObjList.size() : 0); }
    bool IsMore() const { return (mbReverse ? (mnIndex != 0) : (mnIndex < maObjList.size())); }
    SdrObject* Next() { return (mbReverse ? (mnIndex != 0 ? maObjList[--mnIndex] : 0) : (mnIndex < maObjList.size() ? maObjList[mnIndex++] : 0)); }
    sal_uInt32 Count() { return maObjList.size(); }
    bool Contains(const SdrObject& rObject) { for(SdrObjectVector::const_iterator aCandidate(maObjList.begin()); aCandidate != maObjList.end(); aCandidate++) if(*aCandidate == &rObject) return true; return false; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDITER_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
