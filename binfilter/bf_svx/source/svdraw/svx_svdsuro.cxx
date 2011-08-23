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

#include "svdsuro.hxx"
#include "svdobj.hxx"
#include "svdpage.hxx"
#include "svdmodel.hxx"

namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////

// fuer Schreiben
/*N*/ SdrObjSurrogate::SdrObjSurrogate(SdrObject* pObj1, const SdrObject* pRefObj1)
/*N*/ {
/*N*/ 	ImpClearVars();
/*N*/ 	pObj=pObj1;
/*N*/ 	pRefObj=pRefObj1;
/*N*/ 	ImpMakeSurrogate();
/*N*/ }

// fuer Lesen
/*N*/ SdrObjSurrogate::SdrObjSurrogate(const SdrModel& rMod, SvStream& rIn)
/*N*/ {
/*N*/ 	ImpClearVars();
/*N*/ 	pModel=&rMod;
/*N*/ 	ImpRead(rIn);
/*N*/ }

// fuer Lesen
/*N*/ SdrObjSurrogate::SdrObjSurrogate(const SdrObject& rRefObj, SvStream& rIn)
/*N*/ {
/*N*/ 	ImpClearVars();
/*N*/ 	pRefObj=&rRefObj;
/*N*/ 	pModel=rRefObj.GetModel();
/*N*/ 	ImpRead(rIn);
/*N*/ }

/*N*/ SdrObjSurrogate::~SdrObjSurrogate()
/*N*/ {
/*N*/ 	if (pGrpOrdNums!=NULL) delete [] pGrpOrdNums;
/*N*/ }

/*N*/ void SdrObjSurrogate::ImpClearVars()
/*N*/ {
/*N*/ 	pObj=NULL;
/*N*/ 	pRefObj=NULL;
/*N*/ 	pList=NULL;
/*N*/ 	pRootList=NULL;
/*N*/ 	pPage=NULL;
/*N*/ 	pModel=NULL;
/*N*/ 	eList=SDROBJLIST_UNKNOWN;
/*N*/ 	nOrdNum=0;
/*N*/ 	pGrpOrdNums=NULL;
/*N*/ 	nGrpLevel=0;
/*N*/ 	nPageNum=0;
/*N*/ }

/*N*/ void SdrObjSurrogate::ImpMakeSurrogate()
/*N*/ {
/*N*/ 	if (pObj!=NULL) { // ansonsten bleibt eList=SDROBJLIST_UNKNOWN, weil Obj=NULL
/*N*/ 		FASTBOOL bSameList=FALSE;
/*N*/ 		FASTBOOL bSamePage=FALSE;
/*N*/ 		pModel=pObj->GetModel();
/*N*/ 		pList=pObj->GetObjList();
/*N*/ 		pRootList=pList;
/*N*/ 		pPage=pObj->GetPage();
/*N*/ 		nOrdNum=pObj->GetOrdNum();
/*N*/ 		DBG_ASSERT(pModel!=NULL,"ImpMakeSurrogate(): Zielobjekt hat kein Model");
/*N*/ 		DBG_ASSERT(pList!=NULL,"ImpMakeSurrogate(): Zielobjekt hat keine ObjList");
/*N*/ 		if (pModel!=NULL && pList!=NULL) {
/*N*/ 			if (pRefObj!=NULL) {
/*N*/ 				if (pList==pRefObj->GetObjList()) bSameList=TRUE;
/*N*/ 				else if (pPage!=NULL && pPage==pRefObj->GetPage()) bSamePage=TRUE;
/*N*/ 			}
/*N*/ 			if (!bSameList) {
/*N*/ 				//if (eList==SDROBJLIST_UNKNOWN) eList=pList->GetListKind();
/*N*/ 				SdrObjList* pL=pList->GetUpList();
/*N*/ 				nGrpLevel=0;
/*N*/ 				while (pL!=NULL) { pL=pL->GetUpList(); nGrpLevel++; }
/*N*/ 				if (nGrpLevel!=0) { DBG_BF_ASSERT(0, "STRIP"); //STRIP001 // Aha, verschachtelt
/*N*/ 				} // if (nGrpLevel!=0)
/*N*/ 			} // if (eList!=SDROBJLIST_SAMELIST)
/*N*/ 			if (bSameList) eList=SDROBJLIST_SAMELIST;
/*N*/ 			else if (bSamePage) eList=SDROBJLIST_SAMEPAGE;
/*N*/ 			else eList=pRootList->GetListKind();
/*N*/ 			if (eList==SDROBJLIST_GROUPOBJ || eList==SDROBJLIST_UNKNOWN) {
/*?*/ 				if (pGrpOrdNums!=NULL) {
/*?*/ 					delete [] pGrpOrdNums;
/*?*/ 					pGrpOrdNums=NULL;
/*?*/ 					nGrpLevel=0;
/*?*/ 				}
/*?*/ 				eList=SDROBJLIST_UNKNOWN;
/*?*/ 				return;
/*N*/ 			}
/*N*/ 			if (SdrIsPageKind(eList)) {
/*N*/ 				if (pPage!=NULL) {
/*N*/ 					nPageNum=pPage->GetPageNum();
/*N*/ 				} else {
/*?*/ 					DBG_ERROR("ImpMakeSurrogate(): eList ist eine Page, aber pPage==NULL");
/*?*/ 					if (pGrpOrdNums!=NULL) {
/*?*/ 						delete [] pGrpOrdNums;
/*?*/ 						pGrpOrdNums=NULL;
/*?*/ 						nGrpLevel=0;
/*N*/ 					}
/*N*/ 					eList=SDROBJLIST_UNKNOWN;
/*N*/ 					return;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			// so, nun ist alles beisammen und kann gestreamt werden.
/*N*/ 		} // if (pModel!=NULL && pList!=NULL)
/*N*/ 	} // if (pObj!=NULL)
/*N*/ }

/*N*/ void SdrObjSurrogate::ImpWriteValue(SvStream& rOut, UINT32 nVal, unsigned nByteAnz) const
/*N*/ {
/*N*/ 	switch (nByteAnz) {
/*N*/ 		case 0: rOut<<BYTE(nVal);   break;
/*N*/ 		case 1: rOut<<UINT16(nVal); break;
/*N*/ 		case 3: rOut<<nVal;         break;
/*N*/ 		default: DBG_ERROR("ImpWriteValue(): Nur Bytelaengen 1, 2 oder 4 sind erlaubt!");
/*N*/ 	} // switch
/*N*/ }

/*N*/ void SdrObjSurrogate::ImpReadValue(SvStream& rIn, UINT32& rVal, unsigned nByteAnz) const
/*N*/ {
/*N*/ 	switch (nByteAnz) {
/*N*/ 		case 0: { BYTE n;   rIn>>n; rVal=n; } break;
/*N*/ 		case 1: { UINT16 n; rIn>>n; rVal=n; } break;
/*N*/ 		case 3: {           rIn>>rVal;      } break;
/*N*/ 		default: DBG_ERROR("ImpReadValue(): Nur Bytelaengen 1, 2 oder 4 sind erlaubt!");
/*N*/ 	} // switch
/*N*/ }

/*N*/ SvStream& operator<<(SvStream& rOut, const SdrObjSurrogate& rSurro)
/*N*/ {
/*N*/ 	BYTE nId=BYTE(rSurro.eList);
/*N*/ 	if (rSurro.eList==SDROBJLIST_UNKNOWN) {
/*N*/ 		rOut<<nId;
/*N*/ 	} else {
/*N*/ 		FASTBOOL bGrp=rSurro.nGrpLevel!=0;
/*N*/ 		if (bGrp) nId|=0x20; // Gruppierung
/*N*/ 		ULONG nMaxOrdNum=rSurro.nOrdNum;
/*N*/ 		unsigned i;
/*N*/ 		for (i=0; i<rSurro.nGrpLevel; i++) {
/*?*/ 			if (nMaxOrdNum<rSurro.pGrpOrdNums[i]) nMaxOrdNum=rSurro.pGrpOrdNums[i];
/*N*/ 		}
/*N*/ 		unsigned nByteAnz=0;
/*N*/ 		if (nMaxOrdNum>0xFF) nByteAnz++;
/*N*/ 		if (nMaxOrdNum>0xFFFF) nByteAnz+=2;
/*N*/ 		nId|=BYTE(nByteAnz<<6); // Byteanzahl an der richtigen Stelle im ID-Byte eintragen
/*N*/ 		rOut<<nId;
/*N*/ 		rSurro.ImpWriteValue(rOut,rSurro.nOrdNum,nByteAnz);
/*N*/ 		if (SdrIsPageKind(rSurro.eList)) { // Seitennummer schreiben
/*N*/ 			rOut<<rSurro.nPageNum;
/*N*/ 		}
/*N*/ 		if (bGrp) { // Gruppierung
/*?*/ 			rOut<<rSurro.nGrpLevel;
/*?*/ 			for (i=0; i<rSurro.nGrpLevel; i++) {
/*?*/ 				rSurro.ImpWriteValue(rOut,rSurro.pGrpOrdNums[i],nByteAnz);
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return rOut;
/*N*/ }

/*N*/ void SdrObjSurrogate::ImpRead(SvStream& rIn)
/*N*/ {
/*N*/ 	BYTE nId;
/*N*/ 	rIn>>nId;
/*N*/ 	eList=SdrObjListKind(nId & 0x1F);
/*N*/ 	if (eList!=SDROBJLIST_UNKNOWN) {
/*N*/ 		FASTBOOL bGrp=(nId & 0x20)!=0;      // Id-Byte
/*N*/ 		unsigned nByteAnz=(nId & 0xC0)>>6;  // aufschluesseln
/*N*/ 		ImpReadValue(rIn,nOrdNum,nByteAnz); // Ordnungsnummer des Objekts
/*N*/ 		if (SdrIsPageKind(eList)) { // Seitennummer schreiben
/*N*/ 			rIn>>nPageNum;
/*N*/ 		}
/*N*/ 		if (bGrp) { // Gruppierung
/*?*/ 			rIn>>nGrpLevel;
/*?*/ 			pGrpOrdNums=new UINT32[nGrpLevel];
/*?*/ 			for (unsigned i=0; i<nGrpLevel; i++) {
/*?*/ 				ImpReadValue(rIn,pGrpOrdNums[i],nByteAnz);
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SdrObjSurrogate::ImpFindObj()
/*N*/ {
/*N*/ 	if (eList!=SDROBJLIST_UNKNOWN) {
/*N*/ 		switch (eList) {
/*N*/ 			case SDROBJLIST_SAMEPAGE: case SDROBJLIST_SAMELIST: {
/*N*/ 				if (pRefObj!=NULL) {
/*N*/ 					pPage=pRefObj->GetPage();
/*N*/ 					if (eList==SDROBJLIST_SAMELIST) {
/*N*/ 						pRootList=pRefObj->GetObjList();
/*N*/ 						if (pRootList==NULL) {
/*N*/ 							DBG_ERROR("SdrObjSurrogate::ImpFindObj(): eList=SDROBJLIST_SAMELIST, pRefObj hat jedoch keine ObjList");
/*N*/ 							return;
/*N*/ 						}
/*N*/ 					} else {
/*N*/ 						if (pPage!=NULL) {
/*N*/ 							pRootList=pPage;
/*N*/ 						} else {
/*N*/ 							DBG_ERROR("SdrObjSurrogate::ImpFindObj(): eList=SDROBJLIST_SAMEPAGE, pRefObj hat jedoch keine Page");
/*N*/ 							return;
/*N*/ 						}
/*N*/ 					}
/*N*/ 				} else {
/*N*/ 					DBG_ERROR("SdrObjSurrogate::ImpFindObj(): Relatives Surrogat benoetigt Referenzobjekt");
/*N*/ 					return;
/*N*/ 				}
/*N*/ 			} break;
/*N*/ 			case SDROBJLIST_DRAWPAGE: pPage=pModel->GetPage(nPageNum); pRootList=pPage; break;
/*N*/ 			case SDROBJLIST_MASTERPAGE: pPage=pModel->GetMasterPage(nPageNum); pRootList=pPage; break;
/*N*/ 			case SDROBJLIST_VIRTOBJECTS: {
/*N*/ 				DBG_ERROR("SdrObjSurrogate::ImpFindObj(): Persistente virtuelle Objekte in dieser Version nicht verfuegbar");
/*N*/ 				return;
/*N*/ 			} break;
/*N*/ 			case SDROBJLIST_SYMBOLTABLE: {
/*N*/ 				DBG_ERROR("SdrObjSurrogate::ImpFindObj(): Symbole in dieser Version nicht verfuegbar");
/*N*/ 				return;
/*N*/ 			} break;
/*N*/ 			default: {
/*N*/ 				DBG_ERROR("SdrObjSurrogate::ImpFindObj(): Unbekannter oder unsinniger Objektlistentyp");
/*N*/ 				return;
/*N*/ 			}
/*N*/ 		} // switch
/*N*/ 		pList=pRootList;
/*N*/ 		for (unsigned i=0; i<nGrpLevel; i++) {
/*?*/ 			SdrObject* pO=pList->GetObj(pGrpOrdNums[i]);
/*?*/ 			if (pO!=NULL) {
/*?*/ 				pList=pO->GetSubList();
/*?*/ 				if (pList==NULL) {
/*?*/ 					DBG_ERROR("SdrObjSurrogate::ImpFindObj(): Objekt ist kein Gruppenobjekt");
/*?*/ 					return;
/*?*/ 				}
/*?*/ 			} else {
/*?*/ 				DBG_ERROR("SdrObjSurrogate::ImpFindObj(): Gruppenobjekt nicht gefunden");
/*?*/ 				return;
/*?*/ 			}
/*N*/ 		}
/*N*/ 		pObj=pList->GetObj(nOrdNum);
/*N*/ 		DBG_ASSERT(pObj!=NULL,"SdrObjSurrogate::ImpFindObj(): Zielobjekt nicht gefunden");
/*N*/ 	}
/*N*/ }

/*N*/ SdrObject* SdrObjSurrogate::GetObject()
/*N*/ {
/*N*/ 	if (pObj==NULL && eList!=SDROBJLIST_UNKNOWN) ImpFindObj();
/*N*/ 	return pObj;
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
