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

#include "svdviter.hxx"
#include "svdpage.hxx"
#include "svdview.hxx"
#include "svdpagv.hxx"

namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrViewIter::ImpInitVars()
/*N*/ {
/*N*/ 	nListenerNum=0;
/*N*/ 	nPageViewNum=0;
/*N*/ 	nOutDevNum=0;
/*N*/ 	pAktView=NULL;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ SdrViewIter::SdrViewIter(const SdrObject* pObject_, FASTBOOL bNoMasterPage_)
/*N*/ {
/*N*/ 	pObject=pObject_;
/*N*/ 	pModel=pObject_!=NULL ? pObject_->GetModel() : NULL;
/*N*/ 	pPage=pObject_!=NULL ? pObject_->GetPage() : NULL;
/*N*/ 	bNoMasterPage=bNoMasterPage_;
/*N*/ 	if (pModel==NULL || pPage==NULL) {
/*N*/ 		pModel=NULL;
/*N*/ 		pPage=NULL;
/*N*/ 	}
/*N*/ 	ImpInitVars();
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ FASTBOOL SdrViewIter::ImpCheckPageView(SdrPageView* pPV) const
/*N*/ {
/*N*/ 	if (pPage!=NULL) {
/*N*/ 		FASTBOOL bMaster=pPage->IsMasterPage();
/*N*/ 		USHORT nPageNum=pPage->GetPageNum();
/*N*/ 		SdrPage* pPg=pPV->GetPage();
/*N*/ 		if (pPg==pPage) {
/*N*/ 			if (pObject!=NULL) {
/*N*/ 				// Objekt gewuenscht? Na dann erstmal sehen, ob
/*N*/ 				// das Obj in dieser PageView auch sichtbar ist.
/*N*/ 				SetOfByte aObjLay;
/*N*/ 				pObject->GetLayer(aObjLay);
/*N*/ 				aObjLay&=pPV->GetVisibleLayers();
/*N*/ 				return !aObjLay.IsEmpty();
/*N*/ 			} else {
/*N*/ 				return TRUE;
/*N*/ 			}
/*N*/ 		} else {
/*N*/ 			if (!bNoMasterPage && bMaster && (pObject==NULL || !pObject->IsNotVisibleAsMaster())) {
/*N*/ 				USHORT nMasterPageAnz=pPg->GetMasterPageCount();
/*N*/ 				USHORT nMasterPagePos=0;
/*N*/ 				while (nMasterPagePos<nMasterPageAnz) {
/*N*/ 					if (nPageNum==pPg->GetMasterPageNum(nMasterPagePos)) {
/*N*/ 						// Aha, die gewuenschte Page ist also MasterPage in dieser PageView
/*N*/ 						if (pObject!=NULL) {
/*N*/ 							// Objekt gewuenscht? Na dann erstmal sehen, ob
/*N*/ 							// das Obj in dieser PageView auch sichtbar ist.
/*N*/ 							SetOfByte aObjLay;
/*N*/ 							pObject->GetLayer(aObjLay);
/*N*/ 							aObjLay&=pPV->GetVisibleLayers();
/*N*/ 							aObjLay&=pPg->GetMasterPageVisibleLayers(nMasterPagePos);
/*N*/ 							if (!aObjLay.IsEmpty()) {
/*N*/ 								return TRUE;
/*N*/ 							} // ansonsten die naechste MasterPage der Page ansehen...
/*N*/ 						} else {
/*N*/ 							return TRUE;
/*N*/ 						}
/*N*/ 					}
/*N*/ 					nMasterPagePos++;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			// MasterPage nicht erlaubt oder keine passende gefunden
/*N*/ 			return FALSE;
/*N*/ 		}
/*N*/ 	} else {
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ SdrView* SdrViewIter::ImpFindView()
/*N*/ {
/*N*/ 	if (pModel!=NULL) {
/*N*/ 		USHORT nLsAnz=pModel->GetListenerCount();
/*N*/ 		while (nListenerNum<nLsAnz) {
/*N*/ 			SfxListener* pLs=pModel->GetListener(nListenerNum);
/*N*/ 			pAktView=PTR_CAST(SdrView,pLs);
/*N*/ 			if (pAktView!=NULL) {
/*N*/ 				if (pPage!=NULL) {
/*N*/ 					USHORT nPvAnz=pAktView->GetPageViewCount();
/*N*/ 					USHORT nPvNum=0;
/*N*/ 					while (nPvNum<nPvAnz) {
/*N*/ 						SdrPageView* pPV=pAktView->GetPageViewPvNum(nPvNum);
/*N*/ 						if (ImpCheckPageView(pPV)) {
/*N*/ 							return pAktView;
/*N*/ 						}
/*N*/ 						nPvNum++;
/*N*/ 					}
/*N*/ 				} else {
/*N*/ 					return pAktView;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			nListenerNum++;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	pAktView=NULL;
/*N*/ 	return pAktView;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ SdrView* SdrViewIter::FirstView()
/*N*/ {
/*N*/ 	ImpInitVars();
/*N*/ 	return ImpFindView();
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ SdrView* SdrViewIter::NextView()
/*N*/ {
/*N*/ 	nListenerNum++;
/*N*/ 	return ImpFindView();
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
