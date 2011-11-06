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

class SdrObjList;
class SdrObject;
class SdrMarkList;

// SdrObjListIter methods:
// IM_FLAT              : Flach ueber die Liste
// IM_DEEPWITHGROUPS    : Mit rekursivem Abstieg, Next() liefert auch Gruppenobjekte
// IM_DEEPNOGROUPS      : Mit rekursivem Abstieg, Next() liefert keine Gruppenobjekte
enum SdrIterMode { IM_FLAT, IM_DEEPWITHGROUPS, IM_DEEPNOGROUPS};

class SVX_DLLPUBLIC SdrObjListIter
{
    List                        maObjList;
    sal_uInt32                  mnIndex;
    sal_Bool                        mbReverse;

    void ImpProcessObjectList(const SdrObjList& rObjList, SdrIterMode eMode, sal_Bool bUseZOrder);
    void ImpProcessMarkList(const SdrMarkList& rMarkList, SdrIterMode eMode);
    void ImpProcessObj(SdrObject* pObj, SdrIterMode eMode, sal_Bool bUseZOrder);

public:
    SdrObjListIter(const SdrObjList& rObjList, SdrIterMode eMode = IM_DEEPNOGROUPS, sal_Bool bReverse = sal_False);
    /** This variant lets the user choose the order in which to travel over
        the objects.
        @param bUseZOrder
            When <TRUE/> then the z-order defines the order of iteration.
            Otherwise the navigation position as returned by
            SdrObject::GetNavigationPosition() is used.
    */
    SdrObjListIter(const SdrObjList& rObjList, sal_Bool bUseZOrder, SdrIterMode eMode = IM_DEEPNOGROUPS, sal_Bool bReverse = sal_False);

    /* SJ: the following function can now be used with every
       SdrObject and is no longer limited to group objects */
    SdrObjListIter(const SdrObject& rObj, SdrIterMode eMode = IM_DEEPNOGROUPS, sal_Bool bReverse = sal_False);

    /** Iterates over a list of marked objects received from the SdrMarkView. */
    SdrObjListIter(const SdrMarkList& rMarkList, SdrIterMode eMode = IM_DEEPNOGROUPS, sal_Bool bReverse = sal_False);

    void Reset() { mnIndex = (mbReverse ? maObjList.Count() : 0L); }
    sal_Bool IsMore() const { return (mbReverse ? mnIndex != 0 : ( mnIndex < maObjList.Count())); }
    SdrObject* Next() { return (SdrObject*)maObjList.GetObject(mbReverse ? --mnIndex : mnIndex++); }

    sal_uInt32 Count() { return maObjList.Count(); }
};

#endif //_SVDITER_HXX

