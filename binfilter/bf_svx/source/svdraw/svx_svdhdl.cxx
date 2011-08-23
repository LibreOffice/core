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

#include "svdetc.hxx"
#include "svdmrkv.hxx"







#include "svdstr.hrc"


// #105678#
namespace binfilter {

/*N*/ SdrHdl::~SdrHdl()
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ class ImpSdrHdlListSorter: public ContainerSorter {
/*N*/ public:
/*N*/     ImpSdrHdlListSorter(Container& rNewCont): ContainerSorter(rNewCont) {}
/*N*/     virtual int Compare(const void* pElem1, const void* pElem2) const;
/*N*/ };

/*N*/ int ImpSdrHdlListSorter::Compare(const void* pElem1, const void* pElem2) const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// #97016# II

/*N*/ class ImplHdlListData
/*N*/ {
/*N*/ public:
/*N*/ 	sal_uInt32					mnFocusIndex;
/*N*/ 	SdrMarkView*				pView;
/*N*/ 
/*N*/ 	ImplHdlListData(SdrMarkView* pV): mnFocusIndex(CONTAINER_ENTRY_NOTFOUND), pView(pV) {}
/*N*/ };


// #105678# Help struct for re-sorting handles
struct ImplHdlAndIndex
{
    SdrHdl*						mpHdl;
    sal_uInt32					mnIndex;
};

// #105678# Help method for sorting handles taking care of OrdNums, keeping order in
// single objects and re-sorting polygon handles intuitively

////////////////////////////////////////////////////////////////////////////////////////////////////
// #97016# II


/*N*/ SdrHdl* SdrHdlList::GetFocusHdl() const
/*N*/ {
/*N*/ 	if(pImpl->mnFocusIndex != CONTAINER_ENTRY_NOTFOUND && pImpl->mnFocusIndex < GetHdlCount())
/*?*/ 		return GetHdl(pImpl->mnFocusIndex);
/*N*/ 	else
/*?*/ 		return 0L;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ SdrHdlList::SdrHdlList(SdrMarkView* pV)
/*N*/ :	aList(1024,32,32),
/*N*/ 	pImpl(new ImplHdlListData(pV))
/*N*/ 	//#97016# II
/*N*/ 	//pView(pV)
/*N*/ { 
/*N*/ 	nHdlSize = 3; 
/*N*/ 	bRotateShear = FALSE; 
/*N*/ 	bMoveOutside = FALSE; 
/*N*/ 	bDistortShear = FALSE; 
/*N*/ 	bFineHandles = FALSE;
/*N*/ }
/*N*/ 
/*N*/ SdrHdlList::~SdrHdlList() 
/*N*/ { 
/*N*/ 	Clear(); 
/*N*/ 	//#97016# II
/*N*/ 	delete pImpl;
/*N*/ }



/*N*/ void SdrHdlList::SetRotateShear(BOOL bOn)              
/*N*/ { 
/*N*/ 	bRotateShear = bOn; 
/*N*/ }

/*N*/ void SdrHdlList::SetDistortShear(BOOL bOn)             
/*N*/ { 
/*N*/ 	bDistortShear = bOn; 
/*N*/ }

/*N*/ void SdrHdlList::SetFineHdl(BOOL bOn)                 
/*N*/ { 
/*N*/ 	if(bFineHandles != bOn)
/*N*/ 	{
/*N*/ 		// remember new state
/*N*/ 		bFineHandles = bOn; 
/*N*/ 
/*N*/ 		// propagate change to IAOs
/*N*/ 		for(UINT32 i=0; i<GetHdlCount(); i++) 
/*N*/ 		{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ void SdrHdlList::Clear()
/*N*/ {
/*N*/ 	for (ULONG i=0; i<GetHdlCount(); i++) 
/*N*/ 	{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	}
/*N*/ 	aList.Clear();
/*N*/ 
/*N*/ 	// immediately remove from display
//STRIP012/*N*/ 	if(pImpl->pView)
//STRIP012/*N*/ 		pImpl->pView->RefreshAllIAOManagers();
/*N*/ 
/*N*/ 	bRotateShear=FALSE;
/*N*/ 	bDistortShear=FALSE;
/*N*/ }

/*N*/ void SdrHdlList::Sort()
/*N*/ {
/*N*/ 	// #97016# II: remember current focused handle
/*N*/ 	SdrHdl* pPrev = GetFocusHdl();
/*N*/ 
/*N*/     ImpSdrHdlListSorter aSort(aList);
/*N*/     aSort.DoSort();

    // #97016# II: get now and compare
/*N*/ 	SdrHdl* pNow = GetFocusHdl();
/*N*/ 
/*N*/ 	if(pPrev != pNow)
/*N*/ 	{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	}
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
