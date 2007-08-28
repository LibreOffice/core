/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svditer.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2007-08-28 13:44:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SdrObjList;
class SdrObject;

// SdrObjListIter methods:
// IM_FLAT              : Flach ueber die Liste
// IM_DEEPWITHGROUPS    : Mit rekursivem Abstieg, Next() liefert auch Gruppenobjekte
// IM_DEEPNOGROUPS      : Mit rekursivem Abstieg, Next() liefert keine Gruppenobjekte
enum SdrIterMode { IM_FLAT, IM_DEEPWITHGROUPS, IM_DEEPNOGROUPS};

class SVX_DLLPUBLIC SdrObjListIter
{
    List                        maObjList;
    sal_uInt32                  mnIndex;
    BOOL                        mbReverse;

    void ImpProcessObjectList(const SdrObjList& rObjList, SdrIterMode eMode, BOOL bUseZOrder);

public:
    SdrObjListIter(const SdrObjList& rObjList, SdrIterMode eMode = IM_DEEPNOGROUPS, BOOL bReverse = FALSE);
    /** This variant lets the user choose the order in which to travel over
        the objects.
        @param bUseZOrder
            When <TRUE/> then the z-order defines the order of iteration.
            Otherwise the navigation position as returned by
            SdrObject::GetNavigationPosition() is used.
    */
    SdrObjListIter(const SdrObjList& rObjList, BOOL bUseZOrder, SdrIterMode eMode = IM_DEEPNOGROUPS, BOOL bReverse = FALSE);

    /* SJ: the following function can now be used with every
       SdrObject and is no longer limited to group objects */
    SdrObjListIter(const SdrObject& rObj, SdrIterMode eMode = IM_DEEPNOGROUPS, BOOL bReverse = FALSE);

    void Reset() { mnIndex = (mbReverse ? maObjList.Count() : 0L); }
    BOOL IsMore() const { return (mbReverse ? mnIndex != 0 : ( mnIndex < maObjList.Count())); }
    SdrObject* Next() { return (SdrObject*)maObjList.GetObject(mbReverse ? --mnIndex : mnIndex++); }

    sal_uInt32 Count() { return maObjList.Count(); }
};

#endif //_SVDITER_HXX

