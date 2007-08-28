/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svditer.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2007-08-28 13:44:51 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _SVDITER_HXX
#include "svditer.hxx"
#endif

#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif

#ifndef _SVDOGRP_HXX
#include <svx/svdogrp.hxx>
#endif

#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif

// #99190#
#ifndef _E3D_SCENE3D_HXX
#include <svx/scene3d.hxx>
#endif

SdrObjListIter::SdrObjListIter(const SdrObjList& rObjList, SdrIterMode eMode, BOOL bReverse)
:   maObjList(1024, 64, 64),
    mnIndex(0L),
    mbReverse(bReverse)
{
    ImpProcessObjectList(rObjList, eMode, TRUE);
    Reset();
}

SdrObjListIter::SdrObjListIter(const SdrObjList& rObjList, BOOL bUseZOrder, SdrIterMode eMode, BOOL bReverse)
:   maObjList(1024, 64, 64),
    mnIndex(0L),
    mbReverse(bReverse)
{
    ImpProcessObjectList(rObjList, eMode, bUseZOrder);
    Reset();
}

SdrObjListIter::SdrObjListIter( const SdrObject& rObj, SdrIterMode eMode, BOOL bReverse )
:   maObjList(1024, 64, 64),
    mnIndex(0L),
    mbReverse(bReverse)
{
    if ( rObj.ISA( SdrObjGroup ) )
        ImpProcessObjectList(*rObj.GetSubList(), eMode, TRUE);
    else
        maObjList.Insert( (void*)&rObj, LIST_APPEND );
    Reset();
}

void SdrObjListIter::ImpProcessObjectList(const SdrObjList& rObjList, SdrIterMode eMode, BOOL bUseZOrder)
{
    for(sal_uInt32 a(0L); a < rObjList.GetObjCount(); a++)
    {
        SdrObject* pObj = NULL;
        if (bUseZOrder)
            pObj = rObjList.GetObj(a);
        else
            pObj = rObjList.GetObjectForNavigationPosition(a);
        if (pObj == NULL)
        {
            OSL_ASSERT(pObj!=NULL);
            continue;
        }

        sal_Bool bIsGroup(pObj->IsGroupObject());

        // #99190# 3D objects are no group objects, IsGroupObject()
        // only tests if pSub is not null ptr :-(
        if(bIsGroup && pObj->ISA(E3dObject) && !pObj->ISA(E3dScene))
            bIsGroup = sal_False;

        if(eMode != IM_DEEPNOGROUPS || !bIsGroup)
            maObjList.Insert(pObj, LIST_APPEND);

        if(bIsGroup && eMode != IM_FLAT)
            ImpProcessObjectList(*pObj->GetSubList(), eMode, bUseZOrder);
    }
}

