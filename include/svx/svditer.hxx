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

#include <svx/svxdllapi.h>

class SdrObjList;
class SdrObject;
class SdrPage;
class SdrMarkList;

// SdrObjListIter methods:
// SdrIterMode::Flat              : Flat over the list
// SdrIterMode::DeepWithGroups    : With recursive descent parser, Next() also returns group objects
// SdrIterMode::DeepNoGroups      : With recursive descent parser, Next() returns no group objects
enum class SdrIterMode { Flat, DeepWithGroups, DeepNoGroups };

class SVXCORE_DLLPUBLIC SdrObjListIter
{
    std::vector< const SdrObject* > maObjList;
    size_t                          mnIndex;
    bool                            mbReverse;
    bool                            mbUseZOrder;

    SAL_DLLPRIVATE void ImpProcessObjectList(const SdrObjList& rSdrObjList, SdrIterMode eMode);
    SAL_DLLPRIVATE void ImpProcessMarkList(const SdrMarkList& rMarkList, SdrIterMode eMode);
    SAL_DLLPRIVATE void ImpProcessObj(const SdrObject& rSdrObject, SdrIterMode eMode);

public:
    explicit SdrObjListIter(const SdrObjList* pObjList, SdrIterMode eMode = SdrIterMode::DeepNoGroups, bool bReverse = false);
    explicit SdrObjListIter(const SdrObjList* pObjList, bool bUseZOrder, SdrIterMode eMode = SdrIterMode::DeepNoGroups, bool bReverse = false);

    /* SJ: the following function can now be used with every
       SdrObject and is no longer limited to group objects */
    explicit SdrObjListIter(const SdrObject& rSdrObject, SdrIterMode eMode = SdrIterMode::DeepNoGroups, bool bReverse = false);
    explicit SdrObjListIter(const SdrPage* pSdrPage, SdrIterMode eMode = SdrIterMode::DeepNoGroups, bool bReverse = false);

    /** Iterates over a list of marked objects received from the SdrMarkView. TTTT used in sc */
    explicit SdrObjListIter(const SdrMarkList& rMarkList, SdrIterMode eMode = SdrIterMode::DeepNoGroups);

    void Reset() { mnIndex = (mbReverse ? maObjList.size() : 0L); }
    bool IsMore() const { return (mbReverse ? mnIndex != 0 : ( mnIndex < maObjList.size())); }
    SdrObject* Next()
    {
        const size_t idx(mbReverse ? --mnIndex : mnIndex++);
        return (idx < maObjList.size()) ? const_cast< SdrObject* >(maObjList[idx]) : nullptr;
    }

    size_t Count() const { return maObjList.size(); }
};

#endif // INCLUDED_SVX_SVDITER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
