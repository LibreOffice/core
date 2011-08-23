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

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
namespace binfilter {

class SdrObjList;
class SdrObject;

// SdrObjListIter methods:
// IM_FLAT				: Flach ueber die Liste
// IM_DEEPWITHGROUPS	: Mit rekursivem Abstieg, Next() liefert auch Gruppenobjekte
// IM_DEEPNOGROUPS		: Mit rekursivem Abstieg, Next() liefert keine Gruppenobjekte
enum SdrIterMode { IM_FLAT, IM_DEEPWITHGROUPS, IM_DEEPNOGROUPS};

class SdrObjListIter
{
    List						maObjList;
    sal_uInt32					mnIndex;
    BOOL						mbReverse;

    void ImpProcessObjectList(const SdrObjList& rObjList, SdrIterMode eMode);

public:
    SdrObjListIter(const SdrObjList& rObjList, SdrIterMode eMode = IM_DEEPNOGROUPS, BOOL bReverse = FALSE);
    SdrObjListIter(const SdrObject& rGroup, SdrIterMode eMode = IM_DEEPNOGROUPS, BOOL bReverse = FALSE);

    void Reset() { mnIndex = (mbReverse ? maObjList.Count() : 0L); }
    BOOL IsMore() const { return (mbReverse ? mnIndex != 0 : ( mnIndex < maObjList.Count())); }
    SdrObject* Next() { return (SdrObject*)maObjList.GetObject(mbReverse ? --mnIndex : mnIndex++); }
};

}//end of namespace binfilter
#endif //_SVDITER_HXX

