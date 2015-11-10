/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVX_SVDITER_HXX
#define INCLUDED_SVX_SVDITER_HXX

#include <vector>

#include <sal/types.h>
#include <svx/svxdllapi.h>

class SdrObjList;
class SdrObject;
class SdrMarkList;

// SdrObjListIter methods:
// IM_FLAT              : Flat over the list
// IM_DEEPWITHGROUPS    : With recursive descent parser, Next() also returns group objects
// IM_DEEPNOGROUPS      : With recursive descent parser, Next() returns no group objects
enum SdrIterMode { IM_FLAT, IM_DEEPWITHGROUPS, IM_DEEPNOGROUPS};

class SVX_DLLPUBLIC SdrObjListIter
{
    std::vector<SdrObject*>     maObjList;
    sal_uInt32                  mnIndex;
    bool                        mbReverse;

    void ImpProcessObjectList(const SdrObjList& rObjList, SdrIterMode eMode, bool bUseZOrder);
    void ImpProcessMarkList(const SdrMarkList& rMarkList, SdrIterMode eMode);
    void ImpProcessObj(SdrObject* pObj, SdrIterMode eMode, bool bUseZOrder);

public:
    explicit SdrObjListIter(const SdrObjList& rObjList, SdrIterMode eMode = IM_DEEPNOGROUPS, bool bReverse = false);
    /** This variant lets the user choose the order in which to travel over
        the objects.
        @param bUseZOrder
            When <TRUE/> then the z-order defines the order of iteration.
            Otherwise the navigation position as returned by
            SdrObject::GetNavigationPosition() is used.
    */
    SdrObjListIter(const SdrObjList& rObjList, bool bUseZOrder, SdrIterMode eMode = IM_DEEPNOGROUPS, bool bReverse = false);

    /* SJ: the following function can now be used with every
       SdrObject and is no longer limited to group objects */
    explicit SdrObjListIter(const SdrObject& rObj, SdrIterMode eMode = IM_DEEPNOGROUPS, bool bReverse = false);

    /** Iterates over a list of marked objects received from the SdrMarkView. */
    explicit SdrObjListIter(const SdrMarkList& rMarkList, SdrIterMode eMode = IM_DEEPNOGROUPS, bool bReverse = false);

    void Reset() { mnIndex = (mbReverse ? maObjList.size() : 0L); }
    bool IsMore() const { return (mbReverse ? mnIndex != 0 : ( mnIndex < maObjList.size())); }
    SdrObject* Next()
    {
        sal_uInt32 idx = (mbReverse ? --mnIndex : mnIndex++);
        return idx < maObjList.size() ? maObjList[idx] : nullptr;
    }

    sal_uInt32 Count() { return maObjList.size(); }
};

#endif // INCLUDED_SVX_SVDITER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
