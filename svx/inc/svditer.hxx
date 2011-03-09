/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SVDITER_HXX
#define _SVDITER_HXX

#include <vector>

#include <sal/types.h>
#include "svx/svxdllapi.h"

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
    SdrObjListIter(const SdrObjList& rObjList, SdrIterMode eMode = IM_DEEPNOGROUPS, bool bReverse = false);
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
    SdrObjListIter(const SdrObject& rObj, SdrIterMode eMode = IM_DEEPNOGROUPS, bool bReverse = false);

    /** Iterates over a list of marked objects received from the SdrMarkView. */
    SdrObjListIter(const SdrMarkList& rMarkList, SdrIterMode eMode = IM_DEEPNOGROUPS, bool bReverse = false);

    void Reset() { mnIndex = (mbReverse ? maObjList.size() : 0L); }
    bool IsMore() const { return (mbReverse ? mnIndex != 0 : ( mnIndex < maObjList.size())); }
    SdrObject* Next()
    {
        sal_uInt32 idx = (mbReverse ? --mnIndex : mnIndex++);
        return idx < maObjList.size() ? maObjList[idx] : NULL;
    }

    sal_uInt32 Count() { return maObjList.size(); }
};

#endif //_SVDITER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
