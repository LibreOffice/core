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

#include "svditer.hxx"



// #99190#
#include "scene3d.hxx"
namespace binfilter {

/*N*/ SdrObjListIter::SdrObjListIter(const SdrObjList& rObjList, SdrIterMode eMode, BOOL bReverse)
/*N*/ :	maObjList(1024, 64, 64),
/*N*/ 	mnIndex(0L),
/*N*/ 	mbReverse(bReverse)
/*N*/ {
/*N*/ 	ImpProcessObjectList(rObjList, eMode);
/*N*/ 	Reset();
/*N*/ }

/*N*/ SdrObjListIter::SdrObjListIter(const SdrObject& rGroup, SdrIterMode eMode, BOOL bReverse)
/*N*/ :	maObjList(1024, 64, 64),
/*N*/ 	mnIndex(0L),
/*N*/ 	mbReverse(bReverse)
/*N*/ {
/*N*/ 	ImpProcessObjectList(*rGroup.GetSubList(), eMode);
/*N*/ 	Reset();
/*N*/ }

/*N*/ void SdrObjListIter::ImpProcessObjectList(const SdrObjList& rObjList, SdrIterMode eMode)
/*N*/ {
/*N*/ 	for(sal_uInt32 a(0L); a < rObjList.GetObjCount(); a++)
/*N*/ 	{
/*N*/ 		SdrObject* pObj = rObjList.GetObj(a);
/*N*/ 		sal_Bool bIsGroup(pObj->IsGroupObject());
/*N*/ 
/*N*/ 		// #99190# 3D objects are no group objects, IsGroupObject()
/*N*/ 		// only tests if pSub is not null ptr :-(
/*N*/ 		if(bIsGroup && pObj->ISA(E3dObject) && !pObj->ISA(E3dScene))
/*N*/ 			bIsGroup = sal_False;
/*N*/ 
/*N*/ 		if(eMode != IM_DEEPNOGROUPS || !bIsGroup)
/*N*/ 			maObjList.Insert(pObj, LIST_APPEND);
/*N*/ 
/*N*/ 		if(bIsGroup && eMode != IM_FLAT)
/*N*/ 			ImpProcessObjectList(*pObj->GetSubList(), eMode);
/*N*/ 	}
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
