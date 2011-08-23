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

#include "svdopage.hxx"
#include "svdstr.hrc"   // Objektname
#include "svdio.hxx"
#include "svdmodel.hxx"
#include "svdpage.hxx"


#ifndef _SFXITEMSET_HXX
#include <bf_svtools/itemset.hxx>
#endif
namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ TYPEINIT1(SdrPageObj,SdrObject);

/*N*/ SdrPageObj::SdrPageObj(USHORT nNewPageNum):
/*N*/ 	mpPageItemSet(0L),
/*N*/ 	nPageNum(nNewPageNum),
/*N*/ 	bPainting(FALSE),
/*N*/ 	bNotifying(FALSE)
/*N*/ {
/*N*/ }

/*N*/ SdrPageObj::~SdrPageObj()
/*N*/ {
/*N*/ 	if(mpPageItemSet)
/*?*/ 		delete mpPageItemSet;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// ItemSet access

/*N*/ const SfxItemSet& SdrPageObj::GetItemSet() const
/*N*/ {
/*N*/ 	if(!mpPageItemSet)
/*N*/ 		((SdrPageObj*)this)->mpPageItemSet = ((SdrPageObj*)this)->CreateNewItemSet((SfxItemPool&)(*GetItemPool()));
/*N*/ 	return *mpPageItemSet;
/*N*/ }

/*N*/ SfxItemSet* SdrPageObj::CreateNewItemSet(SfxItemPool& rPool)
/*N*/ {
/*N*/ 	return new SfxItemSet(rPool);
/*N*/ }

/*N*/ void SdrPageObj::ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem)
/*N*/ {
/*N*/ 	// #86481# simply ignore item setting on page objects
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void __EXPORT SdrPageObj::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId&, const SfxHint& rHint, const TypeId&)
/*N*/ {
/*N*/ 	SdrHint* pSdrHint = PTR_CAST(SdrHint, &rHint);
/*N*/ 
/*N*/ 	if(pSdrHint)
/*N*/ 	{
/*N*/ 		SdrHintKind eHint = pSdrHint->GetKind();
/*N*/ 
/*N*/ 		if(eHint == HINT_PAGEORDERCHG)
/*N*/ 		{
/*N*/ 			SendRepaintBroadcast();
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			const SdrPage* pChangedPage = pSdrHint->GetPage();
/*N*/ 
/*N*/ 			if(pSdrHint->GetObject() != this
/*N*/ 				&& pModel
/*N*/ 				&& bInserted
/*N*/ 				&& pChangedPage
/*N*/ 				&& pChangedPage != pPage)
/*N*/ 			{
/*N*/ 				const SdrPage* pShownPage = pModel->GetPage(nPageNum);
/*N*/ 
/*N*/ 				if(pShownPage)
/*N*/ 				{
/*N*/ 					if(pShownPage == pChangedPage)
/*N*/ 					{
/*N*/ 						if(eHint == HINT_OBJCHG || eHint == HINT_OBJLISTCLEARED)
/*N*/ 						{
/*N*/ 							// do nothing, else loop with HINT_OBJCHG cause
/*N*/ 							// it can not be decided if hint comes from 21 lines
/*N*/ 							// above (SendRepaintBroadcast())or normally from
/*N*/ 							// object. Doing nothing leads to not updating
/*N*/ 							// object moves on a 2nd opened view showing the page
/*N*/ 							// as handout or notice page. (AW 06052000)
/*N*/ 							//
/*N*/ 							// one solution would be to invalidate without using
/*N*/ 							// SendRepaintBroadcast(), so I made some tries, but it
/*N*/ 							// did'nt work:
/*N*/ 							//
/*N*/ 							//SdrHint aHint(*this);
/*N*/ 							//aHint.SetNeedRepaint(TRUE);
/*N*/ 							//aHint.SetKind(HINT_PAGECHG);
/*N*/ 							//pModel->Broadcast(aHint);
/*N*/ 							//SdrPageView* pPV;
/*N*/ 							//if(pModel && (pPV = pModel->GetPaintingPageView()))
/*N*/ 							//{
/*N*/ 							//	pPV->InvalidateAllWin(pSdrHint->GetRect(), TRUE);
/*N*/ 							//}
/*N*/ 							// GetPageView(pChangedPage);
/*N*/ 							// SdrPageView::InvalidateAllWin(pSdrHint->GetRect(), TRUE);
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 							// send normal
/*N*/ 							SendRepaintBroadcast();
/*N*/ 						}
/*N*/ 					}
/*N*/ 					else if(pChangedPage->IsMasterPage())
/*N*/ 					{
/*N*/ 						UINT16 nMaPgAnz = pShownPage->GetMasterPageCount();
/*N*/ 						BOOL bDone(FALSE);
/*N*/ 
/*N*/ 						for(UINT16 i=0; i<nMaPgAnz && !bDone; i++)
/*N*/ 						{
/*N*/ 							const SdrPage* pMaster = pShownPage->GetMasterPage(i);
/*N*/ 
/*N*/ 							if(pMaster == pChangedPage)
/*N*/ 							{
/*N*/ 								if(eHint == HINT_OBJCHG || eHint == HINT_OBJLISTCLEARED)
/*N*/ 								{
/*N*/ 									// see comment above...
/*N*/ 								}
/*N*/ 								else
/*N*/ 								{
/*N*/ 									// send normal
/*N*/ 									SendRepaintBroadcast();
/*N*/ 									bDone = TRUE;
/*N*/ 								}
/*N*/ 							}
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ void SdrPageObj::SetModel(SdrModel* pNewModel)
/*N*/ {
/*N*/ 	SdrModel* pOldMod=pModel;
/*N*/ 	SdrObject::SetModel(pNewModel);
/*N*/ 	if (pModel!=pOldMod) {
/*N*/ 		if (pOldMod!=NULL) EndListening(*pOldMod);
/*N*/ 		if (pModel!=NULL) StartListening(*pModel);
/*N*/ 	}
/*N*/ }


/*N*/ UINT16 SdrPageObj::GetObjIdentifier() const
/*N*/ {
/*N*/ 	return UINT16(OBJ_PAGE);
/*N*/ }











/*N*/ const Rectangle& SdrPageObj::GetBoundRect() const
/*N*/ {
/*N*/ 	return SdrObject::GetBoundRect();
/*N*/ }


/*N*/ const Rectangle& SdrPageObj::GetSnapRect() const
/*N*/ {
/*N*/ 	return SdrObject::GetSnapRect();
/*N*/ }


/*N*/ const Rectangle& SdrPageObj::GetLogicRect() const
/*N*/ {
/*N*/ 	return SdrObject::GetLogicRect();
/*N*/ }


/*N*/ void SdrPageObj::NbcSetSnapRect(const Rectangle& rRect)
/*N*/ {
/*N*/ 	SdrObject::NbcSetSnapRect(rRect);
/*N*/ }


/*N*/ void SdrPageObj::NbcSetLogicRect(const Rectangle& rRect)
/*N*/ {
/*N*/ 	SdrObject::NbcSetLogicRect(rRect);
/*N*/ }





/*N*/ void SdrPageObj::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ 	SdrObject::WriteData(rOut);
/*N*/ 	SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrPageObj");
/*N*/ #endif
/*N*/ 	rOut<<nPageNum;
/*N*/ }

/*N*/ void SdrPageObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	if (rIn.GetError()!=0) return;
/*N*/ 	SdrObject::ReadData(rHead,rIn);
/*N*/ 	SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrPageObj");
/*N*/ #endif
/*N*/ 	rIn>>nPageNum;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////


}
