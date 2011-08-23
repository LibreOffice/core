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

#include "svdmark.hxx"
#include "svdorect.hxx" // GetMarkDescription
#include "svdstr.hrc"   // Namen aus der Resource
namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrMarkList::ForceSort()
/*N*/ {
/*N*/ 	if (!bSorted) {
/*N*/ 		bSorted=TRUE;
/*N*/ 		ULONG nAnz=aList.Count();
/*N*/ 		if (nAnz>1) {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SdrMarkList::Clear()
/*N*/ {
/*N*/ 	for (ULONG i=0; i<GetMarkCount(); i++) {
/*?*/ 		SdrMark* pMark=GetMark(i);
/*?*/ 		delete pMark;
/*N*/ 	}
/*N*/ 	aList.Clear();
/*N*/ 	SetNameDirty();
/*N*/ }

/*N*/ void SdrMarkList::operator=(const SdrMarkList& rLst)
/*N*/ {
/*N*/ 	Clear();
/*N*/ 	for (ULONG i=0; i<rLst.GetMarkCount(); i++) {
/*?*/ 		SdrMark* pMark=rLst.GetMark(i);
/*?*/ 		SdrMark* pNeuMark=new SdrMark(*pMark);
/*?*/ 		aList.Insert(pNeuMark,CONTAINER_APPEND);
/*N*/ 	}
/*N*/ 	aMarkName=rLst.aMarkName;
/*N*/ 	bNameOk=rLst.bNameOk;
/*N*/ 	aPointName=rLst.aPointName;
/*N*/ 	bPointNameOk=rLst.bPointNameOk;
/*N*/ 	aGluePointName=rLst.aGluePointName;
/*N*/ 	bGluePointNameOk=rLst.bGluePointNameOk;
/*N*/ 	bSorted=rLst.bSorted;
/*N*/ }

/*N*/ ULONG SdrMarkList::FindObject(const SdrObject* pObj)
/*N*/ {
    // #109658#
    //
    // Since relying on OrdNums is not allowed for the selection because objects in the
    // selection may not be inserted in a list if they are e.g. modified ATM, i changed
    // this loop to just look if the object pointer is in the selection.
    //
    // Problem is that GetOrdNum() which is const, internally casts no non-const and
    // hardly sets the OrdNum member of the object (nOrdNum) to 0 (ZERO) if the object
    // is not inserted in a object list.
    // Since this may be by purpose and necessary somewhere else i decided that it is
    // less dangerous to change this method then changing SdrObject::GetOrdNum().
/*N*/ 	if(pObj && aList.Count())
/*N*/ 	{
/*N*/ 		for(sal_uInt32 a(0L); a < aList.Count(); a++)
/*N*/ 		{
/*N*/ 			if(((SdrMark*)(aList.GetObject(a)))->GetObj() == pObj)
/*N*/ 			{
/*N*/ 				return a;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return CONTAINER_ENTRY_NOTFOUND;

    /*
    ForceSort();
    if (pObj==NULL || aList.Count()==0) return CONTAINER_ENTRY_NOTFOUND;
    const SdrObjList* pOL=pObj->GetObjList();
    ULONG nObjOrd=pObj->GetOrdNum();
    ULONG nL=0;
    ULONG nR=aList.Count();
    if (GetMark(nL)->GetObj()==pObj) return nL;
    while (nL+1<nR) { // Der Gesuchte befindet sich zwischen, nicht auf den Grenzen!
        ULONG nMid=(nL+nR)/2;
        const SdrObject* pMidObj=GetMark(nMid)->GetObj();
        if (pMidObj==pObj) return nMid; // Hurra, gefunden!
        const SdrObjList* pMidOL=pMidObj!=NULL ? pMidObj->GetObjList() : NULL;
        ULONG nMidObjOrd=pMidObj!=NULL ? pMidObj->GetOrdNum() : 0;
        if (pMidOL==pOL) {
            if (nMidObjOrd<nObjOrd) nL=nMid;
            else if (nMidObjOrd>nObjOrd) nR=nMid;
            else {
                DBG_ASSERT(FALSE,"SdrMarkList::FindObject(): Anderes Objekt mit gleicher OrdNum gefunden!");
                return CONTAINER_ENTRY_NOTFOUND;
            }
        } else if ((long)pMidOL<(long)pOL) nL=nMid;
        else nR=nMid;
    }
    return CONTAINER_ENTRY_NOTFOUND;
    */
/*N*/ }

/*N*/ void SdrMarkList::InsertEntry(const SdrMark& rMark, FASTBOOL bChkSort)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }




/*N*/ FASTBOOL SdrMarkList::DeletePageView(const SdrPageView& rPV)
/*N*/ {
/*N*/ 	FASTBOOL bChgd=FALSE;
/*N*/ 	for (ULONG i=GetMarkCount(); i>0;) {
/*?*/ 		i--;
/*?*/ 		SdrMark* pMark=GetMark(i);
/*?*/ 		if (pMark->GetPageView()==&rPV) {
/*?*/ 			aList.Remove(i);
/*?*/ 			delete pMark;
/*?*/ 			SetNameDirty();
/*?*/ 			bChgd=TRUE;
/*?*/ 		}
/*N*/ 	}
/*N*/ 	return bChgd;
/*N*/ }






/*N*/ FASTBOOL SdrMarkList::TakeBoundRect(SdrPageView* pPV, Rectangle& rRect) const
/*N*/ {
/*N*/ 	FASTBOOL bFnd=FALSE;
/*N*/ 	Rectangle aR;
/*N*/ 
/*N*/ 	for (ULONG i=0; i<GetMarkCount(); i++) {
/*?*/ 		SdrMark* pMark=GetMark(i);
/*?*/ 		if (pPV==NULL || pMark->GetPageView()==pPV) {
/*?*/ 			aR=pMark->GetObj()->GetBoundRect();
/*?*/ 			if (bFnd) {
/*?*/ 				rRect.Union(aR);
/*?*/ 			} else {
/*?*/ 				rRect=aR;
/*?*/ 				bFnd=TRUE;
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ 	return bFnd;
/*N*/ }

/*N*/ FASTBOOL SdrMarkList::TakeSnapRect(SdrPageView* pPV, Rectangle& rRect) const
/*N*/ {
/*N*/ 	FASTBOOL bFnd=FALSE;
/*N*/ 
/*N*/ 	for (ULONG i=0; i<GetMarkCount(); i++) {
/*?*/ 		SdrMark* pMark=GetMark(i);
/*?*/ 		if (pPV==NULL || pMark->GetPageView()==pPV) {
/*?*/ 			Rectangle aR(pMark->GetObj()->GetSnapRect());
/*?*/ 			if (bFnd) {
/*?*/ 				rRect.Union(aR);
/*?*/ 			} else {
/*?*/ 				rRect=aR;
/*?*/ 				bFnd=TRUE;
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ 	return bFnd;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
