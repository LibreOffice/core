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

#include "xpool.hxx"
#include "svdattrx.hxx"
#include "svdpool.hxx"
#include "svddrgv.hxx"
#include "svdtouch.hxx"
#include "svdio.hxx"
#include "svdsuro.hxx"
#include "svdstr.hrc"    // Objektname

#include <bf_svtools/style.hxx>


#include "eeitem.hxx"

#include "rectenum.hxx"


#include "xoutx.hxx"

namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ SdrObjConnection::~SdrObjConnection()
/*N*/ {
/*N*/ 	if (pSuro!=NULL) delete pSuro;
/*N*/ }

/*N*/ void SdrObjConnection::ResetVars()
/*N*/ {
/*N*/ 	pSuro=NULL;
/*N*/ 	pObj=NULL;
/*N*/ 	nConId=0;
/*N*/ 	nXDist=0;
/*N*/ 	nYDist=0;
/*N*/ 	bBestConn=TRUE;
/*N*/ 	bBestVertex=TRUE;
/*N*/ 	bXDistOvr=FALSE;
/*N*/ 	bYDistOvr=FALSE;
/*N*/ 	bAutoVertex=FALSE;
/*N*/ 	bAutoCorner=FALSE;
/*N*/ }

/*N*/ FASTBOOL SdrObjConnection::TakeGluePoint(SdrGluePoint& rGP, FASTBOOL bSetAbsPos) const
/*N*/ {
/*N*/ 	FASTBOOL bRet=FALSE;
/*N*/ 	if (pObj!=NULL) { // Ein Obj muss schon angedockt sein!
/*N*/ 		if (bAutoVertex) {
/*N*/ 			rGP=pObj->GetVertexGluePoint(nConId);
/*N*/ 			bRet=TRUE;
/*N*/ 		} else if (bAutoCorner) {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*?*/ 		} else {
/*?*/ 			const SdrGluePointList* pGPL=pObj->GetGluePointList();
/*N*/ 			if (pGPL!=NULL) { 
/*?*/ 				USHORT nNum=pGPL->FindGluePoint(nConId);
/*?*/ 				if (nNum!=SDRGLUEPOINT_NOTFOUND) {
/*?*/ 					rGP=(*pGPL)[nNum];
/*?*/ 					bRet=TRUE;
/*?*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if (bRet && bSetAbsPos) {
/*N*/ 		Point aPt(rGP.GetAbsolutePos(*pObj));
/*N*/ 		aPt+=aObjOfs;
/*N*/ 		rGP.SetPos(aPt);
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

/*N*/ void SdrObjConnection::Write(SvStream& rOut, const SdrObject* pEdgeObj) const
/*N*/ {
/*N*/ 	SdrIOHeader aHead(rOut,STREAM_WRITE,SdrIOConnID); // ab V11 eingepackt
/*N*/ 	SdrObjSurrogate aSuro(pObj,pEdgeObj);
/*N*/ 	rOut<<aSuro;
/*N*/ 	rOut<<nConId;
/*N*/ 	rOut<<nXDist;
/*N*/ 	rOut<<nYDist;
/*N*/ 	BOOL bTmp;
/*N*/ 	bTmp=bBestConn;   rOut<<bTmp;
/*N*/ 	bTmp=bBestVertex; rOut<<bTmp;
/*N*/ 	bTmp=bXDistOvr;   rOut<<bTmp;
/*N*/ 	bTmp=bYDistOvr;   rOut<<bTmp;
/*N*/ 	bTmp=bAutoVertex; rOut<<bTmp;
/*N*/ 	bTmp=bAutoCorner; rOut<<bTmp;
/*N*/ 	UINT32 nReserve=0;
/*N*/ 	rOut<<nReserve;
/*N*/ 	rOut<<nReserve;
/*N*/ }

/*N*/ void SdrObjConnection::Read(SvStream& rIn, const SdrObject* pEdgeObj)
/*N*/ {
/*N*/ 	if (rIn.GetError()!=0) return;
/*N*/ 	SdrIOHeader aHead(rIn,STREAM_READ,SdrIOConnID); // ab V11 eingepackt
/*N*/ 	pSuro=new SdrObjSurrogate(*pEdgeObj,rIn);
/*N*/ 	BOOL bBit;
/*N*/ 	rIn>>nConId;
/*N*/ 	rIn>>nXDist;
/*N*/ 	rIn>>nYDist;
/*N*/ 	rIn>>bBit; bBestConn  =bBit;
/*N*/ 	rIn>>bBit; bBestVertex=bBit;
/*N*/ 	rIn>>bBit; bXDistOvr  =bBit;
/*N*/ 	rIn>>bBit; bYDistOvr  =bBit;
/*N*/ 	rIn>>bBit; bAutoVertex=bBit;
/*N*/ 	rIn>>bBit; bAutoCorner=bBit;
/*N*/ 	UINT32 nReserve;
/*N*/ 	rIn>>nReserve;
/*N*/ 	rIn>>nReserve;
/*N*/ }

/*N*/ void SdrObjConnection::ReadTilV10(SvStream& rIn, const SdrObject* pEdgeObj)
/*N*/ {
/*N*/ 	if (rIn.GetError()!=0) return;
/*N*/ 	pSuro=new SdrObjSurrogate(*pEdgeObj,rIn);
/*N*/ 	BOOL bBit;
/*N*/ 	rIn>>nConId;
/*N*/ 	rIn>>nXDist;
/*N*/ 	rIn>>nYDist;
/*N*/ 	rIn>>bBit; bBestConn  =bBit;
/*N*/ 	rIn>>bBit; bBestVertex=bBit;
/*N*/ 	rIn>>bBit; bXDistOvr  =bBit;
/*N*/ 	rIn>>bBit; bYDistOvr  =bBit;
/*N*/ 	rIn>>bBit; bAutoVertex=bBit;
/*N*/ 	rIn>>bBit; bAutoCorner=bBit;
/*N*/ 	UINT32 nReserve;
/*N*/ 	rIn>>nReserve;
/*N*/ 	rIn>>nReserve;
/*N*/ }

/*N*/ void SdrObjConnection::AfterRead(const SdrObject* pEdgeObj)
/*N*/ {
/*N*/ 	if (pSuro!=NULL) {
/*N*/ 		pObj=pSuro->GetObject();
/*N*/ 		delete pSuro;
/*N*/ 		pSuro=NULL;
/*N*/ 	}
/*N*/ }

/*N*/ Point& SdrEdgeInfoRec::ImpGetLineVersatzPoint(SdrEdgeLineCode eLineCode)
/*N*/ {
/*N*/ 	switch (eLineCode) {
/*N*/ 		case OBJ1LINE2 : return aObj1Line2;
/*?*/ 		case OBJ1LINE3 : return aObj1Line3;
/*N*/ 		case OBJ2LINE2 : return aObj2Line2;
/*?*/ 		case OBJ2LINE3 : return aObj2Line3;
/*N*/ 		case MIDDLELINE: return aMiddleLine;
/*N*/ 	} // switch
/*N*/ 	return aMiddleLine;
/*N*/ }

/*N*/ USHORT SdrEdgeInfoRec::ImpGetPolyIdx(SdrEdgeLineCode eLineCode, const XPolygon& rXP) const
/*N*/ {
/*N*/ 	switch (eLineCode) {
/*N*/ 		case OBJ1LINE2 : return 1;
/*N*/ 		case OBJ1LINE3 : return 2;
/*N*/ 		case OBJ2LINE2 : return rXP.GetPointCount()-3;
/*N*/ 		case OBJ2LINE3 : return rXP.GetPointCount()-4;
/*N*/ 		case MIDDLELINE: return nMiddleLine;
/*N*/ 	} // switch
/*N*/ 	return 0;
/*N*/ }

/*N*/ FASTBOOL SdrEdgeInfoRec::ImpIsHorzLine(SdrEdgeLineCode eLineCode, const XPolygon& rXP) const
/*N*/ {
/*N*/ 	USHORT nIdx=ImpGetPolyIdx(eLineCode,rXP);
/*N*/ 	FASTBOOL bHorz=nAngle1==0 || nAngle1==18000;
/*N*/ 	if (eLineCode==OBJ2LINE2 || eLineCode==OBJ2LINE3) {
/*N*/ 		nIdx=rXP.GetPointCount()-nIdx; // #36314#
/*N*/ 		bHorz=nAngle2==0 || nAngle2==18000; // #52000#
/*N*/ 	}
/*N*/ 	if ((nIdx & 1)==1) bHorz=!bHorz;
/*N*/ 	return bHorz;
/*N*/ }

/*N*/ void SdrEdgeInfoRec::ImpSetLineVersatz(SdrEdgeLineCode eLineCode, const XPolygon& rXP, long nVal)
/*N*/ {
/*N*/ 	Point& rPt=ImpGetLineVersatzPoint(eLineCode);
/*N*/ 	if (ImpIsHorzLine(eLineCode,rXP)) rPt.Y()=nVal;
/*N*/ 	else rPt.X()=nVal;
/*N*/ }

/*N*/ long SdrEdgeInfoRec::ImpGetLineVersatz(SdrEdgeLineCode eLineCode, const XPolygon& rXP) const
/*N*/ {
/*N*/ 	const Point& rPt=ImpGetLineVersatzPoint(eLineCode);
/*N*/ 	if (ImpIsHorzLine(eLineCode,rXP)) return rPt.Y();
/*N*/ 	else return rPt.X();
/*N*/ }
/*N*/ 
/*N*/ SvStream& operator<<(SvStream& rOut, const SdrEdgeInfoRec& rEI)
/*N*/ {
/*N*/ 	SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrEdgeInfoRec");
/*N*/ #endif
/*N*/ 	rOut<<rEI.aObj1Line2;
/*N*/ 	rOut<<rEI.aObj1Line3;
/*N*/ 	rOut<<rEI.aObj2Line2;
/*N*/ 	rOut<<rEI.aObj2Line3;
/*N*/ 	rOut<<rEI.aMiddleLine;
/*N*/ 	rOut<<rEI.nAngle1;
/*N*/ 	rOut<<rEI.nAngle2;
/*N*/ 	rOut<<rEI.nObj1Lines;
/*N*/ 	rOut<<rEI.nObj2Lines;
/*N*/ 	rOut<<rEI.nMiddleLine;
/*N*/ 	rOut<<rEI.cOrthoForm;
/*N*/ 	return rOut;
/*N*/ }
/*N*/ 
/*N*/ SvStream& operator>>(SvStream& rIn, SdrEdgeInfoRec& rEI)
/*N*/ {
/*N*/ 	SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrEdgeInfoRec");
/*N*/ #endif
/*N*/ 	rIn>>rEI.aObj1Line2;
/*N*/ 	rIn>>rEI.aObj1Line3;
/*N*/ 	rIn>>rEI.aObj2Line2;
/*N*/ 	rIn>>rEI.aObj2Line3;
/*N*/ 	rIn>>rEI.aMiddleLine;
/*N*/ 	rIn>>rEI.nAngle1;
/*N*/ 	rIn>>rEI.nAngle2;
/*N*/ 	rIn>>rEI.nObj1Lines;
/*N*/ 	rIn>>rEI.nObj2Lines;
/*N*/ 	rIn>>rEI.nMiddleLine;
/*N*/ 	rIn>>rEI.cOrthoForm;
/*N*/ 	return rIn;
/*N*/ }
/*N*/ 
/*N*/ ////////////////////////////////////////////////////////////////////////////////////////////////////
/*N*/ 
/*N*/ TYPEINIT1(SdrEdgeObj,SdrTextObj);
/*N*/ 
/*N*/ SdrEdgeObj::SdrEdgeObj():
/*N*/ 	SdrTextObj()
/*N*/ {
/*N*/ 	bClosedObj=FALSE;
/*N*/ 	bIsEdge=TRUE;
/*N*/ 	bEdgeTrackDirty=FALSE;
/*N*/ 	nNotifyingCount=0;
/*N*/ 	pEdgeTrack=new XPolygon;
/*N*/ 
/*N*/ 	// #109007#
/*N*/ 	// Default is to allow default connects
/*N*/ 	mbSuppressDefaultConnect = (FASTBOOL)sal_False;
/*N*/ }
/*N*/ 
/*N*/ SdrEdgeObj::~SdrEdgeObj()
/*N*/ {
/*N*/ 	DisconnectFromNode(TRUE);
/*N*/ 	DisconnectFromNode(FALSE);
/*N*/ 	delete pEdgeTrack;
/*N*/ }
/*N*/ 
/*N*/ ////////////////////////////////////////////////////////////////////////////////////////////////////
/*N*/ // ItemSet access
/*N*/ 
/*N*/ SfxItemSet* SdrEdgeObj::CreateNewItemSet(SfxItemPool& rPool)
/*N*/ {
/*N*/ 	// include ALL items, 2D and 3D
/*N*/ 	return new SfxItemSet(rPool,
/*N*/ 		// ranges from SdrAttrObj
/*N*/ 		SDRATTR_START, SDRATTRSET_SHADOW,
/*N*/ 		SDRATTRSET_OUTLINER, SDRATTRSET_MISC,
/*N*/ 		SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,
/*N*/ 
/*N*/ 		// edge attributes
/*N*/ 		SDRATTR_EDGE_FIRST, SDRATTRSET_EDGE,
/*N*/ 
/*N*/ 		// outliner and end
/*N*/ 		EE_ITEMS_START, EE_ITEMS_END,
/*N*/ 		0, 0);
/*N*/ }
/*N*/ 
/*N*/ ////////////////////////////////////////////////////////////////////////////////////////////////////
/*N*/ // private support routines for ItemSet access
/*N*/ void SdrEdgeObj::ItemSetChanged(const SfxItemSet& rSet)
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	SdrTextObj::ItemSetChanged(rSet);
/*N*/ 
/*N*/ 	// local changes
/*N*/ 	ImpSetAttrToEdgeInfo();
/*N*/ }
/*N*/ 
/*N*/ ////////////////////////////////////////////////////////////////////////////////////////////////////
/*N*/ 
/*N*/ void SdrEdgeObj::NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
/*N*/ {
/*N*/ 	SdrTextObj::NbcSetStyleSheet(pNewStyleSheet,bDontRemoveHardAttr);
/*N*/ 	ImpSetAttrToEdgeInfo(); // Werte vom Pool nach aEdgeInfo kopieren
/*N*/ }
/*N*/ 
/*N*/ void SdrEdgeObj::ImpSetAttrToEdgeInfo()
/*N*/ {
/*N*/ 	const SfxItemSet& rSet = GetItemSet();
/*N*/ 	SdrEdgeKind eKind = ((SdrEdgeKindItem&)(rSet.Get(SDRATTR_EDGEKIND))).GetValue();
/*N*/ 	sal_Int32 nVal1 = ((SdrEdgeLine1DeltaItem&)rSet.Get(SDRATTR_EDGELINE1DELTA)).GetValue();
/*N*/ 	sal_Int32 nVal2 = ((SdrEdgeLine2DeltaItem&)rSet.Get(SDRATTR_EDGELINE2DELTA)).GetValue();
/*N*/ 	sal_Int32 nVal3 = ((SdrEdgeLine3DeltaItem&)rSet.Get(SDRATTR_EDGELINE3DELTA)).GetValue();
/*N*/ 
/*N*/ 	if(eKind == SDREDGE_ORTHOLINES || eKind == SDREDGE_BEZIER)
/*N*/ 	{
/*N*/ 		sal_Int32 nVals[3] = { nVal1, nVal2, nVal3 };
/*N*/ 		sal_uInt16 n = 0;
/*N*/ 
/*N*/ 		if(aEdgeInfo.nObj1Lines >= 2 && n < 3)
/*N*/ 		{
/*?*/ 			aEdgeInfo.ImpSetLineVersatz(OBJ1LINE2, *pEdgeTrack, nVals[n]);
/*?*/ 			n++;
/*N*/ 		}
/*N*/ 
/*N*/ 		if(aEdgeInfo.nObj1Lines >= 3 && n < 3)
/*N*/ 		{
/*?*/ 			aEdgeInfo.ImpSetLineVersatz(OBJ1LINE3, *pEdgeTrack, nVals[n]);
/*?*/ 			n++;
/*N*/ 		}
/*N*/ 
/*N*/ 		if(aEdgeInfo.nMiddleLine != 0xFFFF && n < 3)
/*N*/ 		{
/*N*/ 			aEdgeInfo.ImpSetLineVersatz(MIDDLELINE, *pEdgeTrack, nVals[n]);
/*N*/ 			n++;
/*N*/ 		}
/*N*/ 
/*N*/ 		if(aEdgeInfo.nObj2Lines >= 3 && n < 3)
/*N*/ 		{
/*?*/ 			aEdgeInfo.ImpSetLineVersatz(OBJ2LINE3, *pEdgeTrack, nVals[n]);
/*?*/ 			n++;
/*N*/ 		}
/*N*/ 
/*N*/ 		if(aEdgeInfo.nObj2Lines >= 2 && n < 3)
/*N*/ 		{
/*?*/ 			aEdgeInfo.ImpSetLineVersatz(OBJ2LINE2, *pEdgeTrack, nVals[n]);
/*?*/ 			n++;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if(eKind == SDREDGE_THREELINES)
/*N*/ 	{
/*?*/ 		BOOL bHor1 = aEdgeInfo.nAngle1 == 0 || aEdgeInfo.nAngle1 == 18000;
/*?*/ 		BOOL bHor2 = aEdgeInfo.nAngle2 == 0 || aEdgeInfo.nAngle2 == 18000;
/*?*/ 
/*?*/ 		if(bHor1)
/*?*/ 		{
/*?*/ 			aEdgeInfo.aObj1Line2.X() = nVal1;
/*?*/ 		}
/*?*/ 		else
/*?*/ 		{
/*?*/ 			aEdgeInfo.aObj1Line2.Y() = nVal1;
/*?*/ 		}
/*?*/ 
/*?*/ 		if(bHor2)
/*?*/ 		{
/*?*/ 			aEdgeInfo.aObj2Line2.X() = nVal2;
/*?*/ 		}
/*?*/ 		else
/*?*/ 		{
/*?*/ 			aEdgeInfo.aObj2Line2.Y() = nVal2;
/*?*/ 		}
/*N*/ 	}

    // #84649#
/*N*/ 	bEdgeTrackDirty = TRUE;
/*N*/ }

/*N*/ void SdrEdgeObj::ImpSetEdgeInfoToAttr()
/*N*/ {
/*N*/ 	const SfxItemSet& rSet = GetItemSet();
/*N*/ 	SdrEdgeKind eKind = ((SdrEdgeKindItem&)(rSet.Get(SDRATTR_EDGEKIND))).GetValue();
/*N*/ 	sal_Int32 nValAnz = ((SdrEdgeLineDeltaAnzItem&)rSet.Get(SDRATTR_EDGELINEDELTAANZ)).GetValue();
/*N*/ 	sal_Int32 nVal1 = ((SdrEdgeLine1DeltaItem&)rSet.Get(SDRATTR_EDGELINE1DELTA)).GetValue();
/*N*/ 	sal_Int32 nVal2 = ((SdrEdgeLine2DeltaItem&)rSet.Get(SDRATTR_EDGELINE2DELTA)).GetValue();
/*N*/ 	sal_Int32 nVal3 = ((SdrEdgeLine3DeltaItem&)rSet.Get(SDRATTR_EDGELINE3DELTA)).GetValue();
/*N*/ 	sal_Int32 nVals[3] = { nVal1, nVal2, nVal3 };
/*N*/ 	sal_uInt16 n = 0;
/*N*/ 
/*N*/ 	if(eKind == SDREDGE_ORTHOLINES || eKind == SDREDGE_BEZIER)
/*N*/ 	{
/*N*/ 		if(aEdgeInfo.nObj1Lines >= 2 && n < 3)
/*N*/ 		{
/*?*/ 			nVals[n] = aEdgeInfo.ImpGetLineVersatz(OBJ1LINE2, *pEdgeTrack);
/*?*/ 			n++;
/*N*/ 		}
/*N*/ 
/*N*/ 		if(aEdgeInfo.nObj1Lines >= 3 && n < 3)
/*N*/ 		{
/*N*/ 			nVals[n] = aEdgeInfo.ImpGetLineVersatz(OBJ1LINE3, *pEdgeTrack);
/*N*/ 			n++;
/*N*/ 		}
/*N*/ 
/*N*/ 		if(aEdgeInfo.nMiddleLine != 0xFFFF && n < 3)
/*N*/ 		{
/*?*/ 			nVals[n] = aEdgeInfo.ImpGetLineVersatz(MIDDLELINE, *pEdgeTrack);
/*?*/ 			n++;
/*N*/ 		}
/*N*/ 
/*N*/ 		if(aEdgeInfo.nObj2Lines >= 3 && n < 3)
/*N*/ 		{
/*N*/ 			nVals[n] = aEdgeInfo.ImpGetLineVersatz(OBJ2LINE3, *pEdgeTrack);
/*N*/ 			n++;
/*N*/ 		}
/*N*/ 
/*N*/ 		if(aEdgeInfo.nObj2Lines >= 2 && n < 3)
/*N*/ 		{
/*N*/ 			nVals[n] = aEdgeInfo.ImpGetLineVersatz(OBJ2LINE2, *pEdgeTrack);
/*N*/ 			n++;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if(eKind == SDREDGE_THREELINES)
/*N*/ 	{
/*?*/ 		BOOL bHor1 = aEdgeInfo.nAngle1 == 0 || aEdgeInfo.nAngle1 == 18000;
/*?*/ 		BOOL bHor2 = aEdgeInfo.nAngle2 == 0 || aEdgeInfo.nAngle2 == 18000;
/*?*/ 
/*?*/ 		n = 2;
/*?*/ 		nVals[0] = bHor1 ? aEdgeInfo.aObj1Line2.X() : aEdgeInfo.aObj1Line2.Y();
/*?*/ 		nVals[1] = bHor2 ? aEdgeInfo.aObj2Line2.X() : aEdgeInfo.aObj2Line2.Y();
/*N*/ 	}
/*N*/ 
/*N*/ 	if(n != nValAnz || nVals[0] != nVal1 || nVals[1] != nVal2 || nVals[2] != nVal3)
/*N*/ 	{
/*N*/ 		// #75371# Here no more notifying is necessary, just local changes are OK.
/*N*/ 		ImpForceItemSet();
/*N*/ 
/*N*/ 		if(n != nValAnz)
/*?*/ 			mpObjectItemSet->Put(SdrEdgeLineDeltaAnzItem(n));
/*N*/ 
/*N*/ 		if(nVals[0] != nVal1)
/*?*/ 			mpObjectItemSet->Put(SdrEdgeLine1DeltaItem(nVals[0]));
/*N*/ 
/*N*/ 		if(nVals[1] != nVal2)
/*?*/ 			mpObjectItemSet->Put(SdrEdgeLine2DeltaItem(nVals[1]));
/*N*/ 
/*N*/ 		if(nVals[2] != nVal3)
/*?*/ 			mpObjectItemSet->Put(SdrEdgeLine3DeltaItem(nVals[2]));
/*N*/ 
/*N*/ 		if(n < 3)
/*N*/ 			mpObjectItemSet->ClearItem(SDRATTR_EDGELINE3DELTA);
/*N*/ 
/*N*/ 		if(n < 2)
/*N*/ 			mpObjectItemSet->ClearItem(SDRATTR_EDGELINE2DELTA);
/*N*/ 
/*N*/ 		if(n < 1)
/*N*/ 			mpObjectItemSet->ClearItem(SDRATTR_EDGELINE1DELTA);
/*N*/ 	}
/*N*/ }


/*N*/ UINT16 SdrEdgeObj::GetObjIdentifier() const
/*N*/ {
/*N*/ 	return UINT16(OBJ_EDGE);
/*N*/ }
/*N*/ 
/*N*/ const Rectangle& SdrEdgeObj::GetBoundRect() const
/*N*/ {
/*N*/ 	if (bEdgeTrackDirty) {
/*N*/ 		((SdrEdgeObj*)this)->ImpRecalcEdgeTrack();
/*N*/ 	}
/*N*/ 	return SdrTextObj::GetBoundRect();
/*N*/ }
/*N*/ 
/*N*/ const Rectangle& SdrEdgeObj::GetSnapRect() const
/*N*/ {
/*N*/ 	if (bEdgeTrackDirty) {
/*N*/ 		((SdrEdgeObj*)this)->ImpRecalcEdgeTrack();
/*N*/ 	}
/*N*/ 	return SdrTextObj::GetSnapRect();
/*N*/ }
/*N*/ 
/*N*/ void SdrEdgeObj::RecalcSnapRect()
/*N*/ {
/*N*/ 	maSnapRect=pEdgeTrack->GetBoundRect();
/*N*/ }
/*N*/ 
/*N*/ void SdrEdgeObj::RecalcBoundRect()
/*N*/ {
/*N*/ 	aOutRect=GetSnapRect();
/*N*/ 	long nLineWdt=ImpGetLineWdt();
/*N*/ 	// Linienenden beruecksichtigen
/*N*/ 	long nLEndWdt=ImpGetLineEndAdd();
/*N*/ 	if (nLEndWdt>nLineWdt) nLineWdt=nLEndWdt;
/*N*/ 	if (nLineWdt!=0) {
/*N*/ 		aOutRect.Left  ()-=nLineWdt;
/*N*/ 		aOutRect.Top   ()-=nLineWdt;
/*N*/ 		aOutRect.Right ()+=nLineWdt;
/*N*/ 		aOutRect.Bottom()+=nLineWdt;
/*N*/ 	}
/*N*/ 	ImpAddShadowToBoundRect();
/*N*/ 	ImpAddTextToBoundRect();
/*N*/ }

/*N*/ void SdrEdgeObj::TakeUnrotatedSnapRect(Rectangle& rRect) const
/*N*/ {
/*N*/ 	rRect=GetSnapRect();
/*N*/ }


/*N*/ SdrObject* SdrEdgeObj::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
/*N*/ {
/*N*/ 	if (pVisiLayer!=NULL && !pVisiLayer->IsSet(nLayerId)) return NULL;
/*N*/ 	INT32 nMyTol=nTol;
/*N*/ 
/*N*/ 	INT32 nWdt=ImpGetLineWdt()/2; // Halbe Strichstaerke
/*N*/ 	if (nWdt>nMyTol) nMyTol=nWdt; // Bei dicker Linie keine Toleranz noetig
/*N*/ 	Rectangle aR(rPnt,rPnt);
/*N*/ 	aR.Left()  -=nMyTol;
/*N*/ 	aR.Right() +=nMyTol;
/*N*/ 	aR.Top()   -=nMyTol;
/*N*/ 	aR.Bottom()+=nMyTol;
/*N*/ 
/*N*/ 	FASTBOOL bHit=FALSE;
/*N*/ 
/*N*/ 	Polygon aPoly=XOutCreatePolygon(*pEdgeTrack,NULL);
/*N*/ 	bHit=IsRectTouchesLine(aPoly,aR);
/*N*/ 	if (!bHit && HasText()) bHit=SdrTextObj::CheckHit(rPnt,nTol,pVisiLayer)!=NULL;
/*N*/ 	return bHit ? (SdrObject*)this : NULL;
/*N*/ }

/*N*/ FASTBOOL SdrEdgeObj::IsNode() const
/*N*/ {
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ SdrGluePoint SdrEdgeObj::GetVertexGluePoint(USHORT nNum) const
/*N*/ {
/*N*/ 	Point aPt;
/*N*/ 	USHORT nPntAnz=pEdgeTrack->GetPointCount();
/*N*/ 	if (nPntAnz>0) {
/*N*/ 		Point aOfs(GetSnapRect().Center());
/*N*/ 		if (nNum==2 && GetConnectedNode(TRUE)==NULL) aPt=(*pEdgeTrack)[0];
/*N*/ 		else if (nNum==3 && GetConnectedNode(FALSE)==NULL) aPt=(*pEdgeTrack)[nPntAnz-1];
/*N*/ 		else {
/*N*/ 			if ((nPntAnz & 1) ==1) {
/*N*/ 				aPt=(*pEdgeTrack)[nPntAnz/2];
/*N*/ 			} else {
/*N*/ 				Point aPt1((*pEdgeTrack)[nPntAnz/2-1]);
/*N*/ 				Point aPt2((*pEdgeTrack)[nPntAnz/2]);
/*N*/ 				aPt1+=aPt2;
/*N*/ 				aPt1.X()/=2;
/*N*/ 				aPt1.Y()/=2;
/*N*/ 				aPt=aPt1;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		aPt-=aOfs;
/*N*/ 	}
/*N*/ 	SdrGluePoint aGP(aPt);
/*N*/ 	aGP.SetPercent(FALSE);
/*N*/ 	return aGP;
/*N*/ }





/*N*/ void SdrEdgeObj::ConnectToNode(FASTBOOL bTail1, SdrObject* pObj)
/*N*/ {
/*N*/ 	SdrObjConnection& rCon=GetConnection(bTail1);
/*N*/ 	DisconnectFromNode(bTail1);
/*N*/ 	if (pObj!=NULL) {
/*N*/ 		pObj->AddListener(*this);
/*N*/ 		rCon.pObj=pObj;
/*N*/ 		bEdgeTrackDirty=TRUE;
/*N*/ 	}
/*N*/ }

/*N*/ void SdrEdgeObj::DisconnectFromNode(FASTBOOL bTail1)
/*N*/ {
/*N*/ 	SdrObjConnection& rCon=GetConnection(bTail1);
/*N*/ 	if (rCon.pObj!=NULL) {
/*?*/ 		rCon.pObj->RemoveListener(*this);
/*?*/ 		rCon.pObj=NULL;
/*N*/ 	}
/*N*/ }

/*N*/ SdrObject* SdrEdgeObj::GetConnectedNode(FASTBOOL bTail1) const
/*N*/ {
/*N*/ 	SdrObject* pObj=GetConnection(bTail1).pObj;
/*N*/ 	if (pObj!=NULL && (pObj->GetPage()!=pPage || !pObj->IsInserted())) pObj=NULL;
/*N*/ 	return pObj;
/*N*/ }


/*N*/ void SdrEdgeObj::ImpSetTailPoint(FASTBOOL bTail1, const Point& rPt)
/*N*/ {
/*N*/ 	USHORT nPtAnz=pEdgeTrack->GetPointCount();
/*N*/ 	if (nPtAnz==0) {
/*?*/ 		(*pEdgeTrack)[0]=rPt;
/*?*/ 		(*pEdgeTrack)[1]=rPt;
/*N*/ 	} else if (nPtAnz==1) {
/*?*/ 		if (!bTail1) (*pEdgeTrack)[1]=rPt;
/*?*/ 		else { (*pEdgeTrack)[1]=(*pEdgeTrack)[0]; (*pEdgeTrack)[0]=rPt; }
/*N*/ 	} else {
/*?*/ 		if (!bTail1) (*pEdgeTrack)[USHORT(nPtAnz-1)]=rPt;
/*?*/ 		else (*pEdgeTrack)[0]=rPt;
/*N*/ 	}
/*N*/ 	ImpRecalcEdgeTrack();
/*N*/ 	SetRectsDirty();
/*N*/ }


/*N*/ void SdrEdgeObj::ImpRecalcEdgeTrack()
/*N*/ {
/*N*/ 	Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=aOutRect; // war vorher =GetBoundRect() #36431#
/*N*/ 	SetRectsDirty();
/*N*/ 	if (!bEdgeTrackDirty) SendRepaintBroadcast();
/*N*/ 	*pEdgeTrack=ImpCalcEdgeTrack(*pEdgeTrack,aCon1,aCon2,&aEdgeInfo);
/*N*/ 	ImpSetEdgeInfoToAttr(); // Die Werte aus aEdgeInfo in den Pool kopieren
/*N*/ 	bEdgeTrackDirty=FALSE;
/*N*/ 	SendRepaintBroadcast();
/*N*/ 	SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
/*N*/ }

/*N*/ USHORT SdrEdgeObj::ImpCalcEscAngle(SdrObject* pObj, const Point& rPt) const
/*N*/ {
/*N*/ 	if (pObj==NULL) return SDRESC_ALL;
/*N*/ 	Rectangle aR(pObj->GetSnapRect());
/*N*/ 	long dxl=rPt.X()-aR.Left();
/*N*/ 	long dyo=rPt.Y()-aR.Top();
/*N*/ 	long dxr=aR.Right()-rPt.X();
/*N*/ 	long dyu=aR.Bottom()-rPt.Y();
/*N*/ 	FASTBOOL bxMitt=Abs(dxl-dxr)<2;
/*N*/ 	FASTBOOL byMitt=Abs(dyo-dyu)<2;
/*N*/ 	long dx=Min(dxl,dxr);
/*N*/ 	long dy=Min(dyo,dyu);
/*N*/ 	FASTBOOL bDiag=Abs(dx-dy)<2;
/*N*/ 	if (bxMitt && byMitt) return SDRESC_ALL; // In der Mitte
/*N*/ 	if (bDiag) {  // diagonal
/*?*/ 		USHORT nRet=0;
/*?*/ 		if (byMitt) nRet|=SDRESC_VERT;
/*?*/ 		if (bxMitt) nRet|=SDRESC_HORZ;
/*?*/ 		if (dxl<dxr) { // Links
/*?*/ 			if (dyo<dyu) nRet|=SDRESC_LEFT | SDRESC_TOP;
/*?*/ 			else nRet|=SDRESC_LEFT | SDRESC_BOTTOM;
/*?*/ 		} else {       // Rechts
/*?*/ 			if (dyo<dyu) nRet|=SDRESC_RIGHT | SDRESC_TOP;
/*?*/ 			else nRet|=SDRESC_RIGHT | SDRESC_BOTTOM;
/*?*/ 		}
/*?*/ 		return nRet;
/*N*/ 	}
/*N*/ 	if (dx<dy) { // waagerecht
/*N*/ 		if (bxMitt) return SDRESC_HORZ;
/*N*/ 		if (dxl<dxr) return SDRESC_LEFT;
/*N*/ 		else return SDRESC_RIGHT;
/*N*/ 	} else {     // senkrecht
/*N*/ 		if (byMitt) return SDRESC_VERT;
/*N*/ 		if (dyo<dyu) return SDRESC_TOP;
/*N*/ 		else return SDRESC_BOTTOM;
/*N*/ 	}
/*N*/ }


/*N*/ XPolygon SdrEdgeObj::ImpCalcObjToCenter(const Point& rStPt, long nEscAngle, const Rectangle& rRect, const Point& rMeeting) const
/*N*/ {
/*N*/ 	XPolygon aXP;
/*N*/ 	aXP.Insert(XPOLY_APPEND,rStPt,XPOLY_NORMAL);
/*N*/ 	FASTBOOL bRts=nEscAngle==0;
/*N*/ 	FASTBOOL bObn=nEscAngle==9000;
/*N*/ 	FASTBOOL bLks=nEscAngle==18000;
/*N*/ 	FASTBOOL bUnt=nEscAngle==27000;
/*N*/ 	FASTBOOL bHor=bLks || bRts;
/*N*/ 	FASTBOOL bVer=bObn || bUnt;
/*N*/ 
/*N*/ 	Point aP1(rStPt); // erstmal den Pflichtabstand
/*N*/ 	if (bLks) aP1.X()=rRect.Left();
/*N*/ 	if (bRts) aP1.X()=rRect.Right();
/*N*/ 	if (bObn) aP1.Y()=rRect.Top();
/*N*/ 	if (bUnt) aP1.Y()=rRect.Bottom();
/*N*/ 
/*N*/ 	FASTBOOL bFinish=FALSE;
/*N*/ 	if (!bFinish) {
/*N*/ 		Point aP2(aP1); // Und nun den Pflichtabstand ggf. bis auf Meetinghoehe erweitern
/*N*/ 		if (bLks && rMeeting.X()<=aP2.X()) aP2.X()=rMeeting.X();
/*N*/ 		if (bRts && rMeeting.X()>=aP2.X()) aP2.X()=rMeeting.X();
/*N*/ 		if (bObn && rMeeting.Y()<=aP2.Y()) aP2.Y()=rMeeting.Y();
/*N*/ 		if (bUnt && rMeeting.Y()>=aP2.Y()) aP2.Y()=rMeeting.Y();
/*N*/ 		aXP.Insert(XPOLY_APPEND,aP2,XPOLY_NORMAL);
/*N*/ 
/*N*/ 		Point aP3(aP2);
/*N*/ 		if ((bLks && rMeeting.X()>aP2.X()) || (bRts && rMeeting.X()<aP2.X())) { // Aussenrum
/*N*/ 			if (rMeeting.Y()<aP2.Y()) {
/*N*/ 				aP3.Y()=rRect.Top();
/*N*/ 				if (rMeeting.Y()<aP3.Y()) aP3.Y()=rMeeting.Y();
/*N*/ 			} else {
/*N*/ 				aP3.Y()=rRect.Bottom();
/*N*/ 				if (rMeeting.Y()>aP3.Y()) aP3.Y()=rMeeting.Y();
/*N*/ 			}
/*N*/ 			aXP.Insert(XPOLY_APPEND,aP3,XPOLY_NORMAL);
/*N*/ 			if (aP3.Y()!=rMeeting.Y()) {
/*N*/ 				aP3.X()=rMeeting.X();
/*N*/ 				aXP.Insert(XPOLY_APPEND,aP3,XPOLY_NORMAL);
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ((bObn && rMeeting.Y()>aP2.Y()) || (bUnt && rMeeting.Y()<aP2.Y())) { // Aussenrum
/*N*/ 			if (rMeeting.X()<aP2.X()) {
/*N*/ 				aP3.X()=rRect.Left();
/*N*/ 				if (rMeeting.X()<aP3.X()) aP3.X()=rMeeting.X();
/*N*/ 			} else {
/*N*/ 				aP3.X()=rRect.Right();
/*N*/ 				if (rMeeting.X()>aP3.X()) aP3.X()=rMeeting.X();
/*N*/ 			}
/*N*/ 			aXP.Insert(XPOLY_APPEND,aP3,XPOLY_NORMAL);
/*N*/ 			if (aP3.X()!=rMeeting.X()) {
/*N*/ 				aP3.Y()=rMeeting.Y();
/*N*/ 				aXP.Insert(XPOLY_APPEND,aP3,XPOLY_NORMAL);
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ #ifdef DBG_UTIL
/*N*/ 	if (aXP.GetPointCount()>4) {
/*N*/ 		DBG_ERROR("SdrEdgeObj::ImpCalcObjToCenter(): Polygon hat mehr als 4 Punkte!");
/*N*/ 	}
/*N*/ #endif
/*N*/ 	return aXP;
/*N*/ }

/*N*/ XPolygon SdrEdgeObj::ImpCalcEdgeTrack(const XPolygon& rTrack0, SdrObjConnection& rCon1, SdrObjConnection& rCon2, SdrEdgeInfoRec* pInfo) const
/*N*/ {
/*N*/ 	Point aPt1,aPt2;
/*N*/ 	SdrGluePoint aGP1,aGP2;
/*N*/ 	USHORT nEsc1=SDRESC_ALL,nEsc2=SDRESC_ALL;
/*N*/ 	Rectangle aBoundRect1;
/*N*/ 	Rectangle aBoundRect2;
/*N*/ 	Rectangle aBewareRect1;
/*N*/ 	Rectangle aBewareRect2;
/*N*/ 	// Erstmal die alten Endpunkte wiederholen
/*N*/ 	if (rTrack0.GetPointCount()!=0) {
/*N*/ 		aPt1=rTrack0[0];
/*N*/ 		USHORT nSiz=rTrack0.GetPointCount();
/*N*/ 		nSiz--;
/*N*/ 		aPt2=rTrack0[nSiz];
/*N*/ 	} else {
/*?*/ 		if (!aOutRect.IsEmpty()) {
/*?*/ 			aPt1=aOutRect.TopLeft();
/*?*/ 			aPt2=aOutRect.BottomRight();
/*?*/ 		}
/*N*/ 	}
/*N*/ 	FASTBOOL bCon1=rCon1.pObj!=NULL && rCon1.pObj->GetPage()==pPage && rCon1.pObj->IsInserted();
/*N*/ 	FASTBOOL bCon2=rCon2.pObj!=NULL && rCon2.pObj->GetPage()==pPage && rCon2.pObj->IsInserted();
/*N*/ 	const SfxItemSet& rSet = GetItemSet();
/*N*/ 
/*N*/ 	if (bCon1) {
/*N*/ 		if (rCon1.pObj==(SdrObject*)this) { // sicherheitshalber Abfragen #44515#
/*?*/ 			aBoundRect1=aOutRect;
/*N*/ 		} else {
/*N*/ 			aBoundRect1=rCon1.pObj->GetBoundRect();
/*N*/ 		}
/*N*/ 		aBoundRect1.Move(rCon1.aObjOfs.X(),rCon1.aObjOfs.Y());
/*N*/ 		aBewareRect1=aBoundRect1;
/*N*/ 		
/*N*/ 		sal_Int32 nH = ((SdrEdgeNode1HorzDistItem&)rSet.Get(SDRATTR_EDGENODE1HORZDIST)).GetValue();
/*N*/ 		sal_Int32 nV = ((SdrEdgeNode1VertDistItem&)rSet.Get(SDRATTR_EDGENODE1VERTDIST)).GetValue();
/*N*/ 		
/*N*/ 		aBewareRect1.Left()-=nH;
/*N*/ 		aBewareRect1.Right()+=nH;
/*N*/ 		aBewareRect1.Top()-=nV;
/*N*/ 		aBewareRect1.Bottom()+=nV;
/*N*/ 	} else {
/*N*/ 		aBoundRect1=Rectangle(aPt1,aPt1);
/*N*/ 		aBoundRect1.Move(rCon1.aObjOfs.X(),rCon1.aObjOfs.Y());
/*N*/ 		aBewareRect1=aBoundRect1;
/*N*/ 	}
/*N*/ 	if (bCon2) {
/*N*/ 		if (rCon2.pObj==(SdrObject*)this) { // sicherheitshalber Abfragen #44515#
/*N*/ 			aBoundRect2=aOutRect;
/*N*/ 		} else {
/*N*/ 			aBoundRect2=rCon2.pObj->GetBoundRect();
/*N*/ 		}
/*N*/ 		aBoundRect2.Move(rCon2.aObjOfs.X(),rCon2.aObjOfs.Y());
/*N*/ 		aBewareRect2=aBoundRect2;
/*N*/ 
/*N*/ 		sal_Int32 nH = ((SdrEdgeNode2HorzDistItem&)rSet.Get(SDRATTR_EDGENODE2HORZDIST)).GetValue();
/*N*/ 		sal_Int32 nV = ((SdrEdgeNode2VertDistItem&)rSet.Get(SDRATTR_EDGENODE2VERTDIST)).GetValue();
/*N*/ 
/*N*/ 		aBewareRect2.Left()-=nH;
/*N*/ 		aBewareRect2.Right()+=nH;
/*N*/ 		aBewareRect2.Top()-=nV;
/*N*/ 		aBewareRect2.Bottom()+=nV;
/*N*/ 	} else {
/*N*/ 		aBoundRect2=Rectangle(aPt2,aPt2);
/*N*/ 		aBoundRect2.Move(rCon2.aObjOfs.X(),rCon2.aObjOfs.Y());
/*N*/ 		aBewareRect2=aBoundRect2;
/*N*/ 	}
/*N*/ 	XPolygon aBestXP;
/*N*/ 	ULONG nBestQual=0xFFFFFFFF;
/*N*/ 	SdrEdgeInfoRec aBestInfo;
/*N*/ 	FASTBOOL bAuto1=bCon1 && rCon1.bBestVertex;
/*N*/ 	FASTBOOL bAuto2=bCon2 && rCon2.bBestVertex;
/*N*/ 	if (bAuto1) rCon1.bAutoVertex=TRUE;
/*N*/ 	if (bAuto2) rCon2.bAutoVertex=TRUE;
/*N*/ 	USHORT nBestAuto1=0;
/*N*/ 	USHORT nBestAuto2=0;
/*N*/ 	USHORT nAnz1=bAuto1 ? 4 : 1;
/*N*/ 	USHORT nAnz2=bAuto2 ? 4 : 1;
/*N*/ 	for (USHORT nNum1=0; nNum1<nAnz1; nNum1++) {
/*N*/ 		if (bAuto1) rCon1.nConId=nNum1;
/*N*/ 		if (bCon1 && rCon1.TakeGluePoint(aGP1,TRUE)) {
/*N*/ 			aPt1=aGP1.GetPos();
/*N*/ 			nEsc1=aGP1.GetEscDir();
/*N*/ 			if (nEsc1==SDRESC_SMART) nEsc1=ImpCalcEscAngle(rCon1.pObj,aPt1-rCon1.aObjOfs);
/*N*/ 		}
/*N*/ 		for (USHORT nNum2=0; nNum2<nAnz2; nNum2++) {
/*N*/ 			if (bAuto2) rCon2.nConId=nNum2;
/*N*/ 			if (bCon2 && rCon2.TakeGluePoint(aGP2,TRUE)) {
/*N*/ 				aPt2=aGP2.GetPos();
/*N*/ 				nEsc2=aGP2.GetEscDir();
/*N*/ 				if (nEsc2==SDRESC_SMART) nEsc2=ImpCalcEscAngle(rCon2.pObj,aPt2-rCon2.aObjOfs);
/*N*/ 			}
/*N*/ 			for (long nA1=0; nA1<36000; nA1+=9000) {
/*N*/ 				USHORT nE1=nA1==0 ? SDRESC_RIGHT : nA1==9000 ? SDRESC_TOP : nA1==18000 ? SDRESC_LEFT : nA1==27000 ? SDRESC_BOTTOM : 0;
/*N*/ 				for (long nA2=0; nA2<36000; nA2+=9000) {
/*N*/ 					USHORT nE2=nA2==0 ? SDRESC_RIGHT : nA2==9000 ? SDRESC_TOP : nA2==18000 ? SDRESC_LEFT : nA2==27000 ? SDRESC_BOTTOM : 0;
/*N*/ 					if ((nEsc1&nE1)!=0 && (nEsc2&nE2)!=0) {
/*N*/ 						ULONG nQual=0;
/*N*/ 						SdrEdgeInfoRec aInfo;
/*N*/ 						if (pInfo!=NULL) aInfo=*pInfo;
/*N*/ 						XPolygon aXP(ImpCalcEdgeTrack(aPt1,nA1,aBoundRect1,aBewareRect1,aPt2,nA2,aBoundRect2,aBewareRect2,&nQual,&aInfo));
/*N*/ 						if (nQual<nBestQual) {
/*N*/ 							aBestXP=aXP;
/*N*/ 							nBestQual=nQual;
/*N*/ 							aBestInfo=aInfo;
/*N*/ 							nBestAuto1=nNum1;
/*N*/ 							nBestAuto2=nNum2;
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if (bAuto1) rCon1.nConId=nBestAuto1;
/*N*/ 	if (bAuto2) rCon2.nConId=nBestAuto2;
/*N*/ 	if (pInfo!=NULL) *pInfo=aBestInfo;
/*N*/ 	return aBestXP;
/*N*/ }

/*N*/ XPolygon SdrEdgeObj::ImpCalcEdgeTrack(const Point& rPt1, long nAngle1, const Rectangle& rBoundRect1, const Rectangle& rBewareRect1,
/*N*/ 	const Point& rPt2, long nAngle2, const Rectangle& rBoundRect2, const Rectangle& rBewareRect2,
/*N*/ 	ULONG* pnQuality, SdrEdgeInfoRec* pInfo) const
/*N*/ {
/*N*/ 	SdrEdgeKind eKind=((SdrEdgeKindItem&)(GetItem(SDRATTR_EDGEKIND))).GetValue();
/*N*/ 	FASTBOOL bRts1=nAngle1==0;
/*N*/ 	FASTBOOL bObn1=nAngle1==9000;
/*N*/ 	FASTBOOL bLks1=nAngle1==18000;
/*N*/ 	FASTBOOL bUnt1=nAngle1==27000;
/*N*/ 	FASTBOOL bHor1=bLks1 || bRts1;
/*N*/ 	FASTBOOL bVer1=bObn1 || bUnt1;
/*N*/ 	FASTBOOL bRts2=nAngle2==0;
/*N*/ 	FASTBOOL bObn2=nAngle2==9000;
/*N*/ 	FASTBOOL bLks2=nAngle2==18000;
/*N*/ 	FASTBOOL bUnt2=nAngle2==27000;
/*N*/ 	FASTBOOL bHor2=bLks2 || bRts2;
/*N*/ 	FASTBOOL bVer2=bObn2 || bUnt2;
/*N*/ 	FASTBOOL bInfo=pInfo!=NULL;
/*N*/ 	if (bInfo) {
/*N*/ 		pInfo->cOrthoForm=0;
/*N*/ 		pInfo->nAngle1=nAngle1;
/*N*/ 		pInfo->nAngle2=nAngle2;
/*N*/ 		pInfo->nObj1Lines=1;
/*N*/ 		pInfo->nObj2Lines=1;
/*N*/ 		pInfo->nMiddleLine=0xFFFF;
/*N*/ 	}
/*N*/ 	Point aPt1(rPt1);
/*N*/ 	Point aPt2(rPt2);
/*N*/ 	Rectangle aBoundRect1 (rBoundRect1 );
/*N*/ 	Rectangle aBoundRect2 (rBoundRect2 );
/*N*/ 	Rectangle aBewareRect1(rBewareRect1);
/*N*/ 	Rectangle aBewareRect2(rBewareRect2);
/*N*/ 	Point aMeeting((aPt1.X()+aPt2.X()+1)/2,(aPt1.Y()+aPt2.Y()+1)/2);
/*N*/ 	FASTBOOL bMeetingXMid=TRUE;
/*N*/ 	FASTBOOL bMeetingYMid=TRUE;
/*N*/ 	if (eKind==SDREDGE_ONELINE) {
/*?*/ 		XPolygon aXP(2);
/*?*/ 		aXP[0]=rPt1;
/*?*/ 		aXP[1]=rPt2;
/*?*/ 		if (pnQuality!=NULL) {
/*?*/ 			*pnQuality=Abs(rPt1.X()-rPt2.X())+Abs(rPt1.Y()-rPt2.Y());
/*?*/ 		}
/*?*/ 		return aXP;
/*N*/ 	} else if (eKind==SDREDGE_THREELINES) {
/*?*/ 		XPolygon aXP(4);
/*?*/ 		aXP[0]=rPt1;
/*?*/ 		aXP[1]=rPt1;
/*?*/ 		aXP[2]=rPt2;
/*?*/ 		aXP[3]=rPt2;
/*?*/ 		if (bRts1) aXP[1].X()=aBewareRect1.Right();  //+=500;
/*?*/ 		if (bObn1) aXP[1].Y()=aBewareRect1.Top();    //-=500;
/*?*/ 		if (bLks1) aXP[1].X()=aBewareRect1.Left();   //-=500;
/*?*/ 		if (bUnt1) aXP[1].Y()=aBewareRect1.Bottom(); //+=500;
/*?*/ 		if (bRts2) aXP[2].X()=aBewareRect2.Right();  //+=500;
/*?*/ 		if (bObn2) aXP[2].Y()=aBewareRect2.Top();    //-=500;
/*?*/ 		if (bLks2) aXP[2].X()=aBewareRect2.Left();   //-=500;
/*?*/ 		if (bUnt2) aXP[2].Y()=aBewareRect2.Bottom(); //+=500;
/*?*/ 		if (pnQuality!=NULL) {
/*?*/ 			long nQ=Abs(aXP[1].X()-aXP[0].X())+Abs(aXP[1].Y()-aXP[0].Y());
/*?*/ 				nQ+=Abs(aXP[2].X()-aXP[1].X())+Abs(aXP[2].Y()-aXP[1].Y());
/*?*/ 				nQ+=Abs(aXP[3].X()-aXP[2].X())+Abs(aXP[3].Y()-aXP[2].Y());
/*?*/ 			*pnQuality=nQ;
/*?*/ 		}
/*?*/ 		USHORT n1=1;
/*?*/ 		USHORT n2=1;
/*?*/ 		if (bInfo) {
/*?*/ 			pInfo->nObj1Lines=2;
/*?*/ 			pInfo->nObj2Lines=2;
/*?*/ 			if (bHor1) {
/*?*/ 				aXP[1].X()+=pInfo->aObj1Line2.X();
/*?*/ 			} else {
/*?*/ 				aXP[1].Y()+=pInfo->aObj1Line2.Y();
/*?*/ 			}
/*?*/ 			if (bHor2) {
/*?*/ 				aXP[2].X()+=pInfo->aObj2Line2.X();
/*?*/ 			} else {
/*?*/ 				aXP[2].Y()+=pInfo->aObj2Line2.Y();
/*?*/ 			}
/*?*/ 		}
/*?*/ 		return aXP;
/*N*/ 	}
/*N*/ 	USHORT nIntersections=0;
/*N*/ 	FASTBOOL bForceMeeting=FALSE; // Muss die Linie durch den MeetingPoint laufen?
/*N*/ 	{
/*N*/ 		Point aC1(aBewareRect1.Center());
/*N*/ 		Point aC2(aBewareRect2.Center());
/*N*/ 		if (aBewareRect1.Left()<=aBewareRect2.Right() && aBewareRect1.Right()>=aBewareRect2.Left()) {
/*N*/ 			// Ueberschneidung auf der X-Achse
/*N*/ 			long n1=Max(aBewareRect1.Left(),aBewareRect2.Left());
/*N*/ 			long n2=Min(aBewareRect1.Right(),aBewareRect2.Right());
/*N*/ 			aMeeting.X()=(n1+n2+1)/2;
/*N*/ 		} else {
/*N*/ 			// Ansonsten den Mittelpunkt des Freiraums
/*N*/ 			if (aC1.X()<aC2.X()) {
/*N*/ 				aMeeting.X()=(aBewareRect1.Right()+aBewareRect2.Left()+1)/2;
/*N*/ 			} else {
/*N*/ 				aMeeting.X()=(aBewareRect1.Left()+aBewareRect2.Right()+1)/2;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if (aBewareRect1.Top()<=aBewareRect2.Bottom() && aBewareRect1.Bottom()>=aBewareRect2.Top()) {
/*N*/ 			// Ueberschneidung auf der Y-Achse
/*N*/ 			long n1=Max(aBewareRect1.Top(),aBewareRect2.Top());
/*N*/ 			long n2=Min(aBewareRect1.Bottom(),aBewareRect2.Bottom());
/*N*/ 			aMeeting.Y()=(n1+n2+1)/2;
/*N*/ 		} else {
/*N*/ 			// Ansonsten den Mittelpunkt des Freiraums
/*N*/ 			if (aC1.Y()<aC2.Y()) {
/*N*/ 				aMeeting.Y()=(aBewareRect1.Bottom()+aBewareRect2.Top()+1)/2;
/*N*/ 			} else {
/*N*/ 				aMeeting.Y()=(aBewareRect1.Top()+aBewareRect2.Bottom()+1)/2;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		// Im Prinzip gibt es 3 zu unterscheidene Faelle:
/*N*/ 		//   1. Beide in die selbe Richtung
/*N*/ 		//   2. Beide in genau entgegengesetzte Richtungen
/*N*/ 		//   3. Einer waagerecht und der andere senkrecht
/*N*/ 		long nXMin=Min(aBewareRect1.Left(),aBewareRect2.Left());
/*N*/ 		long nXMax=Max(aBewareRect1.Right(),aBewareRect2.Right());
/*N*/ 		long nYMin=Min(aBewareRect1.Top(),aBewareRect2.Top());
/*N*/ 		long nYMax=Max(aBewareRect1.Bottom(),aBewareRect2.Bottom());
/*N*/ 		FASTBOOL bBoundOverlap=aBoundRect1.Right()>aBoundRect2.Left() && aBoundRect1.Left()<aBoundRect2.Right() &&
/*N*/ 							   aBoundRect1.Bottom()>aBoundRect2.Top() && aBoundRect1.Top()<aBoundRect2.Bottom();
/*N*/ 		FASTBOOL bBewareOverlap=aBewareRect1.Right()>aBewareRect2.Left() && aBewareRect1.Left()<aBewareRect2.Right() &&
/*N*/ 								aBewareRect1.Bottom()>aBewareRect2.Top() && aBewareRect1.Top()<aBewareRect2.Bottom();
/*N*/ 		unsigned nMainCase=3;
/*N*/ 		if (nAngle1==nAngle2) nMainCase=1;
/*N*/ 		else if ((bHor1 && bHor2) || (bVer1 && bVer2)) nMainCase=2;
/*N*/ 		if (nMainCase==1) { // Fall 1: Beide in eine Richtung moeglich.
/*N*/ 			if (bVer1) aMeeting.X()=(aPt1.X()+aPt2.X()+1)/2; // ist hier besser, als der
/*N*/ 			if (bHor1) aMeeting.Y()=(aPt1.Y()+aPt2.Y()+1)/2; // Mittelpunkt des Freiraums
/*N*/ 			// bX1Ok bedeutet, dass die Vertikale, die aus Obj1 austritt, keinen Konflikt mit Obj2 bildet, ...
/*N*/ 			FASTBOOL bX1Ok=aPt1.X()<=aBewareRect2.Left() || aPt1.X()>=aBewareRect2.Right();
/*N*/ 			FASTBOOL bX2Ok=aPt2.X()<=aBewareRect1.Left() || aPt2.X()>=aBewareRect1.Right();
/*N*/ 			FASTBOOL bY1Ok=aPt1.Y()<=aBewareRect2.Top() || aPt1.Y()>=aBewareRect2.Bottom();
/*N*/ 			FASTBOOL bY2Ok=aPt2.Y()<=aBewareRect1.Top() || aPt2.Y()>=aBewareRect1.Bottom();
/*N*/ 			if (bLks1 && (bY1Ok || aBewareRect1.Left()<aBewareRect2.Right()) && (bY2Ok || aBewareRect2.Left()<aBewareRect1.Right())) {
/*N*/ 				aMeeting.X()=nXMin;
/*N*/ 				bMeetingXMid=FALSE;
/*N*/ 			}
/*N*/ 			if (bRts1 && (bY1Ok || aBewareRect1.Right()>aBewareRect2.Left()) && (bY2Ok || aBewareRect2.Right()>aBewareRect1.Left())) {
/*N*/ 				aMeeting.X()=nXMax;
/*N*/ 				bMeetingXMid=FALSE;
/*N*/ 			}
/*N*/ 			if (bObn1 && (bX1Ok || aBewareRect1.Top()<aBewareRect2.Bottom()) && (bX2Ok || aBewareRect2.Top()<aBewareRect1.Bottom())) {
/*N*/ 				aMeeting.Y()=nYMin;
/*N*/ 				bMeetingYMid=FALSE;
/*N*/ 			}
/*N*/ 			if (bUnt1 && (bX1Ok || aBewareRect1.Bottom()>aBewareRect2.Top()) && (bX2Ok || aBewareRect2.Bottom()>aBewareRect1.Top())) {
/*N*/ 				aMeeting.Y()=nYMax;
/*N*/ 				bMeetingYMid=FALSE;
/*N*/ 			}
/*N*/ 		} else if (nMainCase==2) {
/*N*/ 			// Fall 2:
/*N*/ 			bForceMeeting=TRUE;
/*N*/ 			if (bHor1) { // beide waagerecht
/*N*/ 				// 9 Moeglichkeiten:                   ???
/*N*/ 				//   2.1 Gegenueber, Ueberschneidung   ???
/*N*/ 				//       nur auf der Y-Achse           ???
/*N*/ 				//   2.2, 2.3 Gegenueber, vertikal versetzt. ???  ???
/*N*/ 				//            Ueberschneidung weder auf der  ???  ???
/*N*/ 				//            X- noch auf der Y-Achse        ???  ???
/*N*/ 				//   2.4, 2.5 Untereinander,   ???  ???
/*N*/ 				//            Ueberschneidung  ???  ???
/*N*/ 				//            nur auf X-Achse  ???  ???
/*N*/ 				//   2.6, 2.7 Gegeneinander, vertikal versetzt. ???  ???
/*N*/ 				//            Ueberschneidung weder auf der     ???  ???
/*N*/ 				//            X- noch auf der Y-Achse.          ???  ???
/*N*/ 				//   2.8 Gegeneinander.       ???
/*N*/ 				//       Ueberschneidung nur  ???
/*N*/ 				//       auf der Y-Achse.     ???
/*N*/ 				//   2.9 Die BewareRects der Objekte ueberschneiden
/*N*/ 				//       sich auf X- und Y-Achse.
/*N*/ 				// Die Faelle gelten entsprechend umgesetzt auch fuer
/*N*/ 				// senkrechte Linienaustritte.
/*N*/ 				// Die Faelle 2.1-2.7 werden mir dem Default-Meeting ausreichend
/*N*/ 				// gut behandelt. Spezielle MeetingPoints werden hier also nur
/*N*/ 				// fuer 2.8 und 2.9 bestimmt.
/*N*/ 
/*N*/ 				// Normalisierung. aR1 soll der nach rechts und
/*N*/ 				// aR2 der nach links austretende sein.
/*N*/ 				Rectangle aBewR1(bRts1 ? aBewareRect1 : aBewareRect2);
/*N*/ 				Rectangle aBewR2(bRts1 ? aBewareRect2 : aBewareRect1);
/*N*/ 				Rectangle aBndR1(bRts1 ? aBoundRect1 : aBoundRect2);
/*N*/ 				Rectangle aBndR2(bRts1 ? aBoundRect2 : aBoundRect1);
/*N*/ 				if (aBewR1.Bottom()>aBewR2.Top() && aBewR1.Top()<aBewR2.Bottom()) {
/*N*/ 					// Ueberschneidung auf der Y-Achse. Faelle 2.1, 2.8, 2.9
/*N*/ 					if (aBewR1.Right()>aBewR2.Left()) {
/*N*/ 						// Faelle 2.8, 2.9
/*N*/ 						// Fall 2.8 ist immer Aussenrumlauf (bDirect=FALSE).
/*N*/ 						// Fall 2.9 kann auch Direktverbindung sein (bei geringer
/*N*/ 						// Ueberschneidung der BewareRects ohne Ueberschneidung der
/*N*/ 						// Boundrects wenn die Linienaustritte sonst das BewareRect
/*N*/ 						// des jeweils anderen Objekts verletzen wuerden.
/*N*/ 						FASTBOOL bCase29Direct=FALSE;
/*N*/ 						FASTBOOL bCase29=aBewR1.Right()>aBewR2.Left();
/*N*/ 						if (aBndR1.Right()<=aBndR2.Left()) { // Fall 2.9 und keine Boundrectueberschneidung
/*?*/ 							if ((aPt1.Y()>aBewareRect2.Top() && aPt1.Y()<aBewareRect2.Bottom()) ||
/*?*/ 								(aPt2.Y()>aBewareRect1.Top() && aPt2.Y()<aBewareRect1.Bottom())) {
/*N*/ 							   bCase29Direct=TRUE;
/*N*/ 							}
/*N*/ 						}
/*N*/ 						if (!bCase29Direct) {
/*N*/ 							FASTBOOL bObenLang=Abs(nYMin-aMeeting.Y())<=Abs(nYMax-aMeeting.Y());
/*N*/ 							if (bObenLang) {
/*N*/ 								aMeeting.Y()=nYMin;
/*N*/ 							} else {
/*?*/ 								aMeeting.Y()=nYMax;
/*N*/ 							}
/*N*/ 							bMeetingYMid=FALSE;
/*N*/ 							if (bCase29) {
/*N*/ 								// und nun noch dafuer sorgen, dass das
/*N*/ 								// umzingelte Obj nicht durchquert wird
/*N*/ 								if (aBewR1.Center().Y()<aBewR2.Center().Y() != bObenLang) {
/*N*/ 									aMeeting.X()=aBewR2.Right();
/*N*/ 								} else {
/*N*/ 									aMeeting.X()=aBewR1.Left();
/*N*/ 								}
/*N*/ 								bMeetingXMid=FALSE;
/*N*/ 							}
/*N*/ 						} else {
/*N*/ 							// Direkte Verbindung (3-Linien Z-Verbindung), da
/*N*/ 							// Verletzung der BewareRects unvermeidlich ist.
/*?*/ 							// Via Dreisatz werden die BewareRects nun verkleinert.
/*?*/ 							long nWant1=aBewR1.Right()-aBndR1.Right(); // Abstand bei Obj1
/*?*/ 							long nWant2=aBndR2.Left()-aBewR2.Left();   // Abstand bei Obj2
/*?*/ 							long nSpace=aBndR2.Left()-aBndR1.Right(); // verfuegbarer Platz
/*?*/ 							long nGet1=BigMulDiv(nWant1,nSpace,nWant1+nWant2);
/*?*/ 							long nGet2=nSpace-nGet1;
/*?*/ 							if (bRts1) { // Normalisierung zurueckwandeln
/*?*/ 								aBewareRect1.Right()+=nGet1-nWant1;
/*?*/ 								aBewareRect2.Left()-=nGet2-nWant2;
/*?*/ 							} else {
/*?*/ 								aBewareRect2.Right()+=nGet1-nWant1;
/*?*/ 								aBewareRect1.Left()-=nGet2-nWant2;
/*?*/ 							}
/*?*/ 							nIntersections++; // Qualitaet herabsetzen
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			} else if (bVer1) { // beide senkrecht
/*N*/ 				Rectangle aBewR1(bUnt1 ? aBewareRect1 : aBewareRect2);
/*N*/ 				Rectangle aBewR2(bUnt1 ? aBewareRect2 : aBewareRect1);
/*N*/ 				Rectangle aBndR1(bUnt1 ? aBoundRect1 : aBoundRect2);
/*N*/ 				Rectangle aBndR2(bUnt1 ? aBoundRect2 : aBoundRect1);
/*N*/ 				if (aBewR1.Right()>aBewR2.Left() && aBewR1.Left()<aBewR2.Right()) {
/*N*/ 					// Ueberschneidung auf der Y-Achse. Faelle 2.1, 2.8, 2.9
/*N*/ 					if (aBewR1.Bottom()>aBewR2.Top()) {
/*N*/ 						// Faelle 2.8, 2.9
/*N*/ 						// Fall 2.8 ist immer Aussenrumlauf (bDirect=FALSE).
/*N*/ 						// Fall 2.9 kann auch Direktverbindung sein (bei geringer
/*N*/ 						// Ueberschneidung der BewareRects ohne Ueberschneidung der
/*N*/ 						// Boundrects wenn die Linienaustritte sonst das BewareRect
/*N*/ 						// des jeweils anderen Objekts verletzen wuerden.
/*N*/ 						FASTBOOL bCase29Direct=FALSE;
/*N*/ 						FASTBOOL bCase29=aBewR1.Bottom()>aBewR2.Top();
/*N*/ 						if (aBndR1.Bottom()<=aBndR2.Top()) { // Fall 2.9 und keine Boundrectueberschneidung
/*N*/ 							if ((aPt1.X()>aBewareRect2.Left() && aPt1.X()<aBewareRect2.Right()) ||
/*N*/ 								(aPt2.X()>aBewareRect1.Left() && aPt2.X()<aBewareRect1.Right())) {
/*N*/ 							   bCase29Direct=TRUE;
/*N*/ 							}
/*N*/ 						}
/*N*/ 						if (!bCase29Direct) {
/*N*/ 							FASTBOOL bLinksLang=Abs(nXMin-aMeeting.X())<=Abs(nXMax-aMeeting.X());
/*N*/ 							if (bLinksLang) {
/*N*/ 								aMeeting.X()=nXMin;
/*N*/ 							} else {
/*N*/ 								aMeeting.X()=nXMax;
/*N*/ 							}
/*N*/ 							bMeetingXMid=FALSE;
/*N*/ 							if (bCase29) {
/*N*/ 								// und nun noch dafuer sorgen, dass das
/*N*/ 								// umzingelte Obj nicht durchquert wird
/*N*/ 								if (aBewR1.Center().X()<aBewR2.Center().X() != bLinksLang) {
/*N*/ 									aMeeting.Y()=aBewR2.Bottom();
/*N*/ 								} else {
/*N*/ 									aMeeting.Y()=aBewR1.Top();
/*N*/ 								}
/*N*/ 								bMeetingYMid=FALSE;
/*N*/ 							}
/*N*/ 						} else {
/*N*/ 							// Direkte Verbindung (3-Linien Z-Verbindung), da
/*N*/ 							// Verletzung der BewareRects unvermeidlich ist.
/*N*/ 							// Via Dreisatz werden die BewareRects nun verkleinert.
/*N*/ 							long nWant1=aBewR1.Bottom()-aBndR1.Bottom(); // Abstand bei Obj1
/*N*/ 							long nWant2=aBndR2.Top()-aBewR2.Top();   // Abstand bei Obj2
/*N*/ 							long nSpace=aBndR2.Top()-aBndR1.Bottom(); // verfuegbarer Platz
/*N*/ 							long nGet1=BigMulDiv(nWant1,nSpace,nWant1+nWant2);
/*N*/ 							long nGet2=nSpace-nGet1;
/*N*/ 							if (bUnt1) { // Normalisierung zurueckwandeln
/*?*/ 								aBewareRect1.Bottom()+=nGet1-nWant1;
/*?*/ 								aBewareRect2.Top()-=nGet2-nWant2;
/*N*/ 							} else {
/*N*/ 								aBewareRect2.Bottom()+=nGet1-nWant1;
/*N*/ 								aBewareRect1.Top()-=nGet2-nWant2;
/*N*/ 							}
/*N*/ 							nIntersections++; // Qualitaet herabsetzen
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		} else if (nMainCase==3) { // Fall 3: Einer waagerecht und der andere senkrecht. Sehr viele Fallunterscheidungen
/*N*/ 			// Kleine Legende: ?????-> Ohne Ueberschneidung, maximal Beruehrung.
/*N*/ 			//                 ?????-> Ueberschneidung
/*N*/ 			//                 ?????-> Selbe Hoehe
/*N*/ 			//                 ?????-> Ueberschneidung
/*N*/ 			//                 ?????-> Ohne Ueberschneidung, maximal Beruehrung.
/*N*/ 			// Linienaustritte links ? rechts ? oben ?und unten ?
/*N*/ 			// Insgesamt sind 96 Konstellationen moeglich, wobei einige nicht einmal
/*N*/ 			// eindeutig einem Fall und damit einer Behandlungsmethode zugeordnet werden
/*N*/ 			// koennen.
/*N*/ 			// 3.1: Hierzu moegen alle Konstellationen zaehlen, die durch den
/*N*/ 			//      Default-MeetingPoint zufriedenstellend abgedeckt sind (20+12).
/*N*/ 			//   ?????   ?????  Diese 12  ?????   ?????   ?????   ?????
/*N*/ 			//   ?????   ?????  Konstel.  ?????   ?????   ?????   ?????
/*N*/ 			//   ?????   ?????  jedoch    ?????   ?????   ?????   ?????
/*N*/ 			//   ?????   ?????  nur zum   ?????   ?????   ?????   ?????
/*N*/ 			//   ?????   ?????  Teil:     ?????   ?????   ?????   ?????
/*N*/ 			//   Letztere 16 Faelle scheiden aus, sobald sich die Objekte offen
/*N*/ 			//   gegenueberstehen (siehe Fall 3.2).
/*N*/ 			// 3.2: Die Objekte stehen sich offen gegenueber und somit ist eine
/*N*/ 			//      Verbindung mit lediglich 2 Linien moeglich (4+20).
/*N*/ 			//      Dieser Fall hat 1. Prioritaet.
/*N*/ 			//   ?????  ?????  Diese 20  ?????  ?????  ?????  ?????
/*N*/ 			//   ?????  ?????  Konstel.  ?????  ?????  ?????  ?????
/*N*/ 			//   ?????  ?????  jedoch    ?????  ?????  ?????  ?????
/*N*/ 			//   ?????  ?????  nur zum   ?????  ?????  ?????  ?????
/*N*/ 			//   ?????  ?????  Teil:     ?????  ?????  ?????  ?????
/*N*/ 			// 3.3: Die Linienaustritte zeigen vom anderen Objekt weg bzw. hinter
/*N*/ 			//      dessen Ruecken vorbei (52+4).
/*N*/ 			//   ?????  ?????  ?????  ?????  Diese 4   ?????  ?????
/*N*/ 			//   ?????  ?????  ?????  ?????  Konstel.  ?????  ?????
/*N*/ 			//   ?????  ?????  ?????  ?????  jedoch    ?????  ?????
/*N*/ 			//   ?????  ?????  ?????  ?????  nur zum   ?????  ?????
/*N*/ 			//   ?????  ?????  ?????  ?????  Teil:     ?????  ?????
/*N*/ 
/*N*/ 			// Fall 3.2
/*N*/ 			Rectangle aTmpR1(aBewareRect1);
/*N*/ 			Rectangle aTmpR2(aBewareRect2);
/*N*/ 			if (bBewareOverlap) {
/*N*/ 				// Ueberschneidung der BewareRects: BoundRects fuer Check auf Fall 3.2 verwenden.
/*N*/ 				aTmpR1=aBoundRect1;
/*N*/ 				aTmpR2=aBoundRect2;
/*N*/ 			}
/*N*/ 			if ((((bRts1 && aTmpR1.Right ()<=aPt2.X()) || (bLks1 && aTmpR1.Left()>=aPt2.X())) &&
/*N*/ 				 ((bUnt2 && aTmpR2.Bottom()<=aPt1.Y()) || (bObn2 && aTmpR2.Top ()>=aPt1.Y()))) ||
/*N*/ 				(((bRts2 && aTmpR2.Right ()<=aPt1.X()) || (bLks2 && aTmpR2.Left()>=aPt1.X())) &&
/*N*/ 				 ((bUnt1 && aTmpR1.Bottom()<=aPt2.Y()) || (bObn1 && aTmpR1.Top ()>=aPt2.Y())))) {
/*N*/ 				// Fall 3.2 trifft zu: Verbindung mit lediglich 2 Linien
/*N*/ 				bForceMeeting=TRUE;
/*N*/ 				bMeetingXMid=FALSE;
/*N*/ 				bMeetingYMid=FALSE;
/*N*/ 				if (bHor1) {
/*N*/ 					aMeeting.X()=aPt2.X();
/*N*/ 					aMeeting.Y()=aPt1.Y();
/*N*/ 				} else {
/*N*/ 					aMeeting.X()=aPt1.X();
/*N*/ 					aMeeting.Y()=aPt2.Y();
/*N*/ 				}
/*N*/ 				// Falls Ueberschneidung der BewareRects:
/*N*/ 				aBewareRect1=aTmpR1;
/*N*/ 				aBewareRect2=aTmpR2;
/*N*/ 			} else if ((((bRts1 && aBewareRect1.Right ()>aBewareRect2.Left  ()) ||
/*N*/ 						 (bLks1 && aBewareRect1.Left  ()<aBewareRect2.Right ())) &&
/*N*/ 						((bUnt2 && aBewareRect2.Bottom()>aBewareRect1.Top   ()) ||
/*N*/ 						 (bObn2 && aBewareRect2.Top   ()<aBewareRect1.Bottom()))) ||
/*N*/ 					   (((bRts2 && aBewareRect2.Right ()>aBewareRect1.Left  ()) ||
/*N*/ 						 (bLks2 && aBewareRect2.Left  ()<aBewareRect1.Right ())) &&
/*N*/ 						((bUnt1 && aBewareRect1.Bottom()>aBewareRect2.Top   ()) ||
/*N*/ 						 (bObn1 && aBewareRect1.Top   ()<aBewareRect2.Bottom())))) {
/*N*/ 				// Fall 3.3
/*N*/ 				bForceMeeting=TRUE;
/*N*/ 				if (bRts1 || bRts2) { aMeeting.X()=nXMax; bMeetingXMid=FALSE; }
/*N*/ 				if (bLks1 || bLks2) { aMeeting.X()=nXMin; bMeetingXMid=FALSE; }
/*N*/ 				if (bUnt1 || bUnt2) { aMeeting.Y()=nYMax; bMeetingYMid=FALSE; }
/*N*/ 				if (bObn1 || bObn2) { aMeeting.Y()=nYMin; bMeetingYMid=FALSE; }
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	XPolygon aXP1(ImpCalcObjToCenter(aPt1,nAngle1,aBewareRect1,aMeeting));
/*N*/ 	XPolygon aXP2(ImpCalcObjToCenter(aPt2,nAngle2,aBewareRect2,aMeeting));
/*N*/ 	USHORT nXP1Anz=aXP1.GetPointCount();
/*N*/ 	USHORT nXP2Anz=aXP2.GetPointCount();
/*N*/ 	if (bInfo) {
/*N*/ 		pInfo->nObj1Lines=nXP1Anz; if (nXP1Anz>1) pInfo->nObj1Lines--;
/*N*/ 		pInfo->nObj2Lines=nXP2Anz; if (nXP2Anz>1) pInfo->nObj2Lines--;
/*N*/ 	}
/*N*/ 	Point aEP1(aXP1[nXP1Anz-1]);
/*N*/ 	Point aEP2(aXP2[nXP2Anz-1]);
/*N*/ 	FASTBOOL bInsMeetingPoint=aEP1.X()!=aEP2.X() && aEP1.Y()!=aEP2.Y();
/*N*/ 	FASTBOOL bHorzE1=aEP1.Y()==aXP1[nXP1Anz-2].Y(); // letzte Linie von XP1 horizontal?
/*N*/ 	FASTBOOL bHorzE2=aEP2.Y()==aXP2[nXP2Anz-2].Y(); // letzte Linie von XP2 horizontal?
/*N*/ 	if (aEP1==aEP2 && (bHorzE1 && bHorzE2 && aEP1.Y()==aEP2.Y()) || (!bHorzE1 && !bHorzE2 && aEP1.X()==aEP2.X())) {
/*N*/ 		// Sonderbehandlung fuer 'I'-Verbinder
/*N*/ 		nXP1Anz--; aXP1.Remove(nXP1Anz,1);
/*N*/ 		nXP2Anz--; aXP2.Remove(nXP2Anz,1);
/*N*/ 		bMeetingXMid=FALSE;
/*N*/ 		bMeetingYMid=FALSE;
/*N*/ 	}
/*N*/ 	if (bInsMeetingPoint) {
/*N*/ 		aXP1.Insert(XPOLY_APPEND,aMeeting,XPOLY_NORMAL);
/*N*/ 		if (bInfo) {
/*N*/ 			// Durch einfuegen des MeetingPoints kommen 2 weitere Linie hinzu.
/*N*/ 			// Evtl. wird eine von diesen die Mittellinie.
/*N*/ 			if (pInfo->nObj1Lines==pInfo->nObj2Lines) {
/*N*/ 				pInfo->nObj1Lines++;
/*N*/ 				pInfo->nObj2Lines++;
/*N*/ 			} else {
/*N*/ 				if (pInfo->nObj1Lines>pInfo->nObj2Lines) {
/*N*/ 					pInfo->nObj2Lines++;
/*N*/ 					pInfo->nMiddleLine=nXP1Anz-1;
/*N*/ 				} else {
/*N*/ 					pInfo->nObj1Lines++;
/*N*/ 					pInfo->nMiddleLine=nXP1Anz;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	} else if (bInfo && aEP1!=aEP2 && nXP1Anz+nXP2Anz>=4) {
/*N*/ 		// Durch Verbinden der beiden Enden kommt eine weitere Linie hinzu.
/*N*/ 		// Dies wird die Mittellinie.
/*N*/ 		pInfo->nMiddleLine=nXP1Anz-1;
/*N*/ 	}
/*N*/ 	USHORT nNum=aXP2.GetPointCount();
/*N*/ 	if (aXP1[nXP1Anz-1]==aXP2[nXP2Anz-1] && nXP1Anz>1 && nXP2Anz>1) nNum--;
/*N*/ 	while (nNum>0) {
/*N*/ 		nNum--;
/*N*/ 		aXP1.Insert(XPOLY_APPEND,aXP2[nNum],XPOLY_NORMAL);
/*N*/ 	}
/*N*/ 	USHORT nPntAnz=aXP1.GetPointCount();
/*N*/ 	char cForm=0;
/*N*/ 	if (bInfo || pnQuality!=NULL) {
/*N*/ 		cForm='?';
/*N*/ 		if (nPntAnz==2) cForm='I';
/*N*/ 		else if (nPntAnz==3) cForm='L';
/*N*/ 		else if (nPntAnz==4) { // Z oder U
/*N*/ 			if (nAngle1==nAngle2) cForm='U';
/*N*/ 			else cForm='Z';
/*N*/ 		} else if (nPntAnz==4) { // ?? ??
/*N*/ 			// ...                 -?    -?
/*N*/ 		} else if (nPntAnz==6) { // S oder C oder ...
/*N*/ 			if (nAngle1!=nAngle2) {
/*N*/ 				// Fuer Typ S hat Linie2 dieselbe Richtung wie Linie4.
/*N*/ 				// Bei Typ C sind die beiden genau entgegengesetzt.
/*N*/ 				Point aP1(aXP1[1]);
/*N*/ 				Point aP2(aXP1[2]);
/*N*/ 				Point aP3(aXP1[3]);
/*N*/ 				Point aP4(aXP1[4]);
/*N*/ 				if (aP1.Y()==aP2.Y()) { // beide Linien Horz
/*N*/ 					if (aP1.X()<aP2.X()==aP3.X()<aP4.X()) cForm='S';
/*N*/ 					else cForm='C';
/*N*/ 				} else { // sonst beide Linien Vert
/*N*/ 					if (aP1.Y()<aP2.Y()==aP3.Y()<aP4.Y()) cForm='S';
/*N*/ 					else cForm='C';
/*N*/ 				}
/*N*/ 			} else cForm='4'; // sonst der 3. Fall mit 5 Linien
/*N*/ 		} else cForm='?';  //
/*N*/ 		// Weitere Formen:
/*N*/ 		if (bInfo) {
/*N*/ 			pInfo->cOrthoForm=cForm;
/*N*/ 			if (cForm=='I' || cForm=='L' || cForm=='Z' || cForm=='U') {
/*N*/ 				pInfo->nObj1Lines=1;
/*N*/ 				pInfo->nObj2Lines=1;
/*N*/ 				if (cForm=='Z' || cForm=='U') {
/*N*/ 					pInfo->nMiddleLine=1;
/*N*/ 				} else {
/*N*/ 					pInfo->nMiddleLine=0xFFFF;
/*N*/ 				}
/*N*/ 			} else if (cForm=='S' || cForm=='C') {
/*N*/ 				pInfo->nObj1Lines=2;
/*N*/ 				pInfo->nObj2Lines=2;
/*N*/ 				pInfo->nMiddleLine=2;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if (pnQuality!=NULL) {
/*N*/ 		ULONG nQual=0;
/*N*/ 		ULONG nQual0=nQual; // Ueberlaeufe vorbeugen
/*N*/ 		FASTBOOL bOverflow=FALSE;
/*N*/ 		Point aPt0(aXP1[0]);
/*N*/ 		for (USHORT nPntNum=1; nPntNum<nPntAnz; nPntNum++) {
/*N*/ 			Point aPt1(aXP1[nPntNum]);
/*N*/ 			nQual+=Abs(aPt1.X()-aPt0.X())+Abs(aPt1.Y()-aPt0.Y());
/*N*/ 			if (nQual<nQual0) bOverflow=TRUE;
/*N*/ 			nQual0=nQual;
/*N*/ 			aPt0=aPt1;
/*N*/ 		}
/*N*/ 
/*N*/ 		USHORT nTmp=nPntAnz;
/*N*/ 		if (cForm=='Z') {
/*N*/ 			nTmp=2; // Z-Form hat gute Qualitaet (nTmp=2 statt 4)
/*N*/ 			ULONG n1=Abs(aXP1[1].X()-aXP1[0].X())+Abs(aXP1[1].Y()-aXP1[0].Y());
/*N*/ 			ULONG n2=Abs(aXP1[2].X()-aXP1[1].X())+Abs(aXP1[2].Y()-aXP1[1].Y());
/*N*/ 			ULONG n3=Abs(aXP1[3].X()-aXP1[2].X())+Abs(aXP1[3].Y()-aXP1[2].Y());
/*N*/ 			// fuer moeglichst gleichlange Linien sorgen
/*N*/ 			ULONG nBesser=0;
/*N*/ 			n1+=n3;
/*N*/ 			n3=n2/4;
/*N*/ 			if (n1>=n2) nBesser=6;
/*N*/ 			else if (n1>=3*n3) nBesser=4;
/*N*/ 			else if (n1>=2*n3) nBesser=2;
/*N*/ 			if (aXP1[0].Y()!=aXP1[1].Y()) nBesser++; // Senkrechte Startlinie kriegt auch noch einen Pluspunkt (fuer H/V-Prio)
/*N*/ 			if (nQual>nBesser) nQual-=nBesser; else nQual=0;
/*N*/ 		}
/*N*/ 		if (nTmp>=3) {
/*N*/ 			nQual0=nQual;
/*N*/ 			nQual+=(ULONG)nTmp*0x01000000;
/*N*/ 			if (nQual<nQual0 || nTmp>15) bOverflow=TRUE;
/*N*/ 		}
/*N*/ 		if (nPntAnz>=2) { // Austrittswinkel nochmal pruefen
/*N*/ 			Point aP1(aXP1[1]); aP1-=aXP1[0];
/*N*/ 			Point aP2(aXP1[nPntAnz-2]); aP2-=aXP1[nPntAnz-1];
/*N*/ 			long nAng1=0; if (aP1.X()<0) nAng1=18000; if (aP1.Y()>0) nAng1=27000;
/*N*/ 			if (aP1.Y()<0) nAng1=9000; if (aP1.X()!=0 && aP1.Y()!=0) nAng1=1; // Schraeg!?!
/*N*/ 			long nAng2=0; if (aP2.X()<0) nAng2=18000; if (aP2.Y()>0) nAng2=27000;
/*N*/ 			if (aP2.Y()<0) nAng2=9000; if (aP2.X()!=0 && aP2.Y()!=0) nAng2=1; // Schraeg!?!
/*N*/ 			if (nAng1!=nAngle1) nIntersections++;
/*N*/ 			if (nAng2!=nAngle2) nIntersections++;
/*N*/ 		}
/*N*/ 
/*N*/ 		// Fuer den Qualitaetscheck wieder die Original-Rects verwenden und
/*N*/ 		// gleichzeitig checken, ob eins fuer die Edge-Berechnung verkleinert
/*N*/ 		// wurde (z.B. Fall 2.9)
/*N*/ 		aBewareRect1=rBewareRect1;
/*N*/ 		aBewareRect2=rBewareRect2;
/*N*/ 
/*N*/ 		for (USHORT i=0; i<nPntAnz; i++) {
/*N*/ 			Point aPt1(aXP1[i]);
/*N*/ 			FASTBOOL b1=aPt1.X()>aBewareRect1.Left() && aPt1.X()<aBewareRect1.Right() &&
/*N*/ 						aPt1.Y()>aBewareRect1.Top() && aPt1.Y()<aBewareRect1.Bottom();
/*N*/ 			FASTBOOL b2=aPt1.X()>aBewareRect2.Left() && aPt1.X()<aBewareRect2.Right() &&
/*N*/ 						aPt1.Y()>aBewareRect2.Top() && aPt1.Y()<aBewareRect2.Bottom();
/*N*/ 			USHORT nInt0=nIntersections;
/*N*/ 			if (i==0 || i==nPntAnz-1) {
/*N*/ 				if (b1 && b2) nIntersections++;
/*N*/ 			} else {
/*N*/ 				if (b1) nIntersections++;
/*N*/ 				if (b2) nIntersections++;
/*N*/ 			}
/*N*/ 			// und nun noch auf Ueberschneidungen checken
/*N*/ 			if (i>0 && nInt0==nIntersections) {
/*N*/ 				if (aPt0.Y()==aPt1.Y()) { // Horizontale Linie
/*N*/ 					if (aPt0.Y()>aBewareRect1.Top() && aPt0.Y()<aBewareRect1.Bottom() &&
/*N*/ 						((aPt0.X()<=aBewareRect1.Left() && aPt1.X()>=aBewareRect1.Right()) ||
/*N*/ 						 (aPt1.X()<=aBewareRect1.Left() && aPt0.X()>=aBewareRect1.Right()))) nIntersections++;
/*N*/ 					if (aPt0.Y()>aBewareRect2.Top() && aPt0.Y()<aBewareRect2.Bottom() &&
/*N*/ 						((aPt0.X()<=aBewareRect2.Left() && aPt1.X()>=aBewareRect2.Right()) ||
/*N*/ 						 (aPt1.X()<=aBewareRect2.Left() && aPt0.X()>=aBewareRect2.Right()))) nIntersections++;
/*N*/ 				} else { // Vertikale Linie
/*N*/ 					if (aPt0.X()>aBewareRect1.Left() && aPt0.X()<aBewareRect1.Right() &&
/*N*/ 						((aPt0.Y()<=aBewareRect1.Top() && aPt1.Y()>=aBewareRect1.Bottom()) ||
/*N*/ 						 (aPt1.Y()<=aBewareRect1.Top() && aPt0.Y()>=aBewareRect1.Bottom()))) nIntersections++;
/*N*/ 					if (aPt0.X()>aBewareRect2.Left() && aPt0.X()<aBewareRect2.Right() &&
/*N*/ 						((aPt0.Y()<=aBewareRect2.Top() && aPt1.Y()>=aBewareRect2.Bottom()) ||
/*N*/ 						 (aPt1.Y()<=aBewareRect2.Top() && aPt0.Y()>=aBewareRect2.Bottom()))) nIntersections++;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			aPt0=aPt1;
/*N*/ 		}
/*N*/ 		if (nPntAnz<=1) nIntersections++;
/*N*/ 		nQual0=nQual;
/*N*/ 		nQual+=(ULONG)nIntersections*0x10000000;
/*N*/ 		if (nQual<nQual0 || nIntersections>15) bOverflow=TRUE;
/*N*/ 
/*N*/ 		if (bOverflow || nQual==0xFFFFFFFF) nQual=0xFFFFFFFE;
/*N*/ 		*pnQuality=nQual;
/*N*/ 	}
/*N*/ 	if (bInfo) { // nun die Linienversaetze auf aXP1 anwenden
/*N*/ 		if (pInfo->nMiddleLine!=0xFFFF) {
/*N*/ 			USHORT nIdx=pInfo->ImpGetPolyIdx(MIDDLELINE,aXP1);
/*N*/ 			if (pInfo->ImpIsHorzLine(MIDDLELINE,aXP1)) {
/*N*/ 				aXP1[nIdx].Y()+=pInfo->aMiddleLine.Y();
/*N*/ 				aXP1[nIdx+1].Y()+=pInfo->aMiddleLine.Y();
/*N*/ 			} else {
/*N*/ 				aXP1[nIdx].X()+=pInfo->aMiddleLine.X();
/*N*/ 				aXP1[nIdx+1].X()+=pInfo->aMiddleLine.X();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if (pInfo->nObj1Lines>=2) {
/*N*/ 			USHORT nIdx=pInfo->ImpGetPolyIdx(OBJ1LINE2,aXP1);
/*N*/ 			if (pInfo->ImpIsHorzLine(OBJ1LINE2,aXP1)) {
/*N*/ 				aXP1[nIdx].Y()+=pInfo->aObj1Line2.Y();
/*N*/ 				aXP1[nIdx+1].Y()+=pInfo->aObj1Line2.Y();
/*N*/ 			} else {
/*N*/ 				aXP1[nIdx].X()+=pInfo->aObj1Line2.X();
/*N*/ 				aXP1[nIdx+1].X()+=pInfo->aObj1Line2.X();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if (pInfo->nObj1Lines>=3) {
/*N*/ 			USHORT nIdx=pInfo->ImpGetPolyIdx(OBJ1LINE3,aXP1);
/*N*/ 			if (pInfo->ImpIsHorzLine(OBJ1LINE3,aXP1)) {
/*N*/ 				aXP1[nIdx].Y()+=pInfo->aObj1Line3.Y();
/*N*/ 				aXP1[nIdx+1].Y()+=pInfo->aObj1Line3.Y();
/*N*/ 			} else {
/*N*/ 				aXP1[nIdx].X()+=pInfo->aObj1Line3.X();
/*N*/ 				aXP1[nIdx+1].X()+=pInfo->aObj1Line3.X();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if (pInfo->nObj2Lines>=2) {
/*N*/ 			USHORT nIdx=pInfo->ImpGetPolyIdx(OBJ2LINE2,aXP1);
/*N*/ 			if (pInfo->ImpIsHorzLine(OBJ2LINE2,aXP1)) {
/*N*/ 				aXP1[nIdx].Y()+=pInfo->aObj2Line2.Y();
/*N*/ 				aXP1[nIdx+1].Y()+=pInfo->aObj2Line2.Y();
/*N*/ 			} else {
/*N*/ 				aXP1[nIdx].X()+=pInfo->aObj2Line2.X();
/*N*/ 				aXP1[nIdx+1].X()+=pInfo->aObj2Line2.X();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if (pInfo->nObj2Lines>=3) {
/*N*/ 			USHORT nIdx=pInfo->ImpGetPolyIdx(OBJ2LINE3,aXP1);
/*N*/ 			if (pInfo->ImpIsHorzLine(OBJ2LINE3,aXP1)) {
/*N*/ 				aXP1[nIdx].Y()+=pInfo->aObj2Line3.Y();
/*N*/ 				aXP1[nIdx+1].Y()+=pInfo->aObj2Line3.Y();
/*N*/ 			} else {
/*N*/ 				aXP1[nIdx].X()+=pInfo->aObj2Line3.X();
/*N*/ 				aXP1[nIdx+1].X()+=pInfo->aObj2Line3.X();
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	// Nun mache ich ggf. aus dem Verbinder eine Bezierkurve
/*N*/ 	if (eKind==SDREDGE_BEZIER && nPntAnz>2) {
/*?*/ 		Point* pPt1=&aXP1[0];
/*?*/ 		Point* pPt2=&aXP1[1];
/*?*/ 		Point* pPt3=&aXP1[nPntAnz-2];
/*?*/ 		Point* pPt4=&aXP1[nPntAnz-1];
/*?*/ 		long dx1=pPt2->X()-pPt1->X();
/*?*/ 		long dy1=pPt2->Y()-pPt1->Y();
/*?*/ 		long dx2=pPt3->X()-pPt4->X();
/*?*/ 		long dy2=pPt3->Y()-pPt4->Y();
/*?*/ 		if (cForm=='L') { // nPntAnz==3
/*?*/ 			aXP1.SetFlags(1,XPOLY_CONTROL);
/*?*/ 			Point aPt3(*pPt2);
/*?*/ 			aXP1.Insert(2,aPt3,XPOLY_CONTROL);
/*?*/ 			nPntAnz=aXP1.GetPointCount();
/*?*/ 			pPt1=&aXP1[0];
/*?*/ 			pPt2=&aXP1[1];
/*?*/ 			pPt3=&aXP1[nPntAnz-2];
/*?*/ 			pPt4=&aXP1[nPntAnz-1];
/*?*/ 			pPt2->X()-=dx1/3;
/*?*/ 			pPt2->Y()-=dy1/3;
/*?*/ 			pPt3->X()-=dx2/3;
/*?*/ 			pPt3->Y()-=dy2/3;
/*?*/ 		} else if (nPntAnz>=4 && nPntAnz<=6) { // Z oder U oder ...
/*?*/ 			// fuer Alle Anderen werden die Endpunkte der Ausgangslinien
/*?*/ 			// erstmal zu Kontrollpunkten. Bei nPntAnz>4 ist also noch
/*?*/ 			// Nacharbeit erforderlich!
/*?*/ 			aXP1.SetFlags(1,XPOLY_CONTROL);
/*?*/ 			aXP1.SetFlags(nPntAnz-2,XPOLY_CONTROL);
/*?*/ 			// Distanz x1.5
/*?*/ 			pPt2->X()+=dx1/2;
/*?*/ 			pPt2->Y()+=dy1/2;
/*?*/ 			pPt3->X()+=dx2/2;
/*?*/ 			pPt3->Y()+=dy2/2;
/*?*/ 			if (nPntAnz==5) {
/*?*/ 				// Vor und hinter dem Mittelpunkt jeweils
/*?*/ 				// noch einen Kontrollpunkt einfuegen
/*?*/ 				Point aCenter(aXP1[2]);
/*?*/ 				long dx1=aCenter.X()-aXP1[1].X();
/*?*/ 				long dy1=aCenter.Y()-aXP1[1].Y();
/*?*/ 				long dx2=aCenter.X()-aXP1[3].X();
/*?*/ 				long dy2=aCenter.Y()-aXP1[3].Y();
/*?*/ 				aXP1.Insert(2,aCenter,XPOLY_CONTROL);
/*?*/ 				aXP1.SetFlags(3,XPOLY_SYMMTR);
/*?*/ 				aXP1.Insert(4,aCenter,XPOLY_CONTROL);
/*?*/ 				aXP1[2].X()-=dx1/2;
/*?*/ 				aXP1[2].Y()-=dy1/2;
/*?*/ 				aXP1[3].X()-=(dx1+dx2)/4;
/*?*/ 				aXP1[3].Y()-=(dy1+dy2)/4;
/*?*/ 				aXP1[4].X()-=dx2/2;
/*?*/ 				aXP1[4].Y()-=dy2/2;
/*?*/ 			}
/*?*/ 			if (nPntAnz==6) {
/*?*/ 				Point aPt1(aXP1[2]);
/*?*/ 				Point aPt2(aXP1[3]);
/*?*/ 				aXP1.Insert(2,aPt1,XPOLY_CONTROL);
/*?*/ 				aXP1.Insert(5,aPt2,XPOLY_CONTROL);
/*?*/ 				long dx=aPt1.X()-aPt2.X();
/*?*/ 				long dy=aPt1.Y()-aPt2.Y();
/*?*/ 				aXP1[3].X()-=dx/2;
/*?*/ 				aXP1[3].Y()-=dy/2;
/*?*/ 				aXP1.SetFlags(3,XPOLY_SYMMTR);
/*?*/ 				//aXP1[4].X()+=dx/2;
/*?*/ 				//aXP1[4].Y()+=dy/2;
/*?*/ 				aXP1.Remove(4,1); // weil identisch mit aXP1[3]
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ 	return aXP1;
/*N*/ }

/*
Nach einer einfachen Rechnung koennte es max. 64 unterschiedliche Verlaeufe mit
5 Linien, 32 mit 4 Linien, 16 mit 3, 8 mit 2 Linien und 4 mit 1 Linie geben=124.
Normalisiert auf 1. Austrittswinkel nach rechts bleiben dann noch 31.
Dann noch eine vertikale Spiegelung wegnormalisiert bleiben noch 16
characteristische Verlaufszuege mit 1-5 Linien:
Mit 1 Linie (Typ 'I'):  --
Mit 2 Linien (Typ 'L'): -?
Mit 3 Linien (Typ 'U'):  -? (Typ 'Z'):  ?
                         -?            -?
Mit 4 Linien: 1 ist nicht plausibel, 3 ist=2 (90deg Drehung). Verbleibt 2,4
     ?? ڿ  �  ڿ                               ڿ  ??
    -?  -?  -? -?                             -?   -?
Mit 5 Linien: nicht plausibel sind 1,2,4,5. 7 ist identisch mit 3 (Richtungsumkehr)
              Bleibt also 3,6 und 8.              '4'  'S'  'C'
       ?   ?            -?  ?  ??                 ?
     ?? ?? ??  ??  �  � -????        ?? �  ??
    -?  -?  -?? -??  -? -? --???       -?? -? ??
Insgesamt sind also 9 Grundtypen zu unterscheiden die den 400 Konstellationen
aus Objektposition und Austrittswinkeln zuzuordnen sind.
4 der 9 Grundtypen haben eine 'Mittellinie'. Die Anzahl der zu Objektabstaende
je Objekt variiert von 0-3:
     Mi   O1   O2   Anmerkung
'I':  n   0    0
'L':  n   0    0
'U':  n  0-1  0-1
'Z':  j   0    0
4.1:  j   0    1    = U+1 bzw. 1+U
4.2:  n  0-2  0-2   = Z+1
'4':  j   0    2    = Z+2
'S':  j   1    1    = 1+Z+1
'C':  n  0-3  0-3   = 1+U+1
*/

/*N*/ void __EXPORT SdrEdgeObj::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId&, const SfxHint& rHint, const TypeId&)
/*N*/ {
/*N*/ 	SfxSimpleHint* pSimple=PTR_CAST(SfxSimpleHint,&rHint);
/*N*/ 	ULONG nId=pSimple==0 ? 0 : pSimple->GetId();
/*N*/ 	FASTBOOL bDataChg=nId==SFX_HINT_DATACHANGED;
/*N*/ 	FASTBOOL bDying=nId==SFX_HINT_DYING;
/*N*/ 	FASTBOOL bObj1=aCon1.pObj!=NULL && aCon1.pObj->GetBroadcaster()==&rBC;
/*N*/ 	FASTBOOL bObj2=aCon2.pObj!=NULL && aCon2.pObj->GetBroadcaster()==&rBC;
/*N*/ 	if (bDying && (bObj1 || bObj2)) {
/*N*/ 		// #35605# Dying vorher abfangen, damit AttrObj nicht
/*N*/ 		// wg. vermeintlicher Vorlagenaenderung rumbroadcastet
/*N*/ 		if (bObj1) aCon1.pObj=NULL;
/*N*/ 		if (bObj2) aCon2.pObj=NULL;
/*N*/ 		return; // Und mehr braucht hier nicht getan werden.
/*N*/ 	}
/*N*/ 	SdrTextObj::SFX_NOTIFY(rBC,rBCType,rHint,rHintType);
/*N*/ 	if (nNotifyingCount==0) { // Hier nun auch ein VerriegelungsFlag
/*N*/ 		((SdrEdgeObj*)this)->nNotifyingCount++;
/*N*/ 		SdrHint* pSdrHint=PTR_CAST(SdrHint,&rHint);
/*N*/ 		if (bDataChg) { // StyleSheet geaendert
/*N*/ 			ImpSetAttrToEdgeInfo(); // Werte bei Vorlagenaenderung vom Pool nach aEdgeInfo kopieren
/*N*/ 		}
/*N*/ 		if (bDataChg                                ||
/*N*/ 			(bObj1 && aCon1.pObj->GetPage()==pPage) ||
/*N*/ 			(bObj2 && aCon2.pObj->GetPage()==pPage) ||
/*N*/ 			(pSdrHint && pSdrHint->GetKind()==HINT_OBJREMOVED))
/*N*/ 		{
/*N*/ 			// Broadcasting nur, wenn auf der selben Page
/*N*/ 			Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
/*N*/ 			if (!bEdgeTrackDirty) SendRepaintBroadcast();
/*N*/ 			bEdgeTrackDirty=TRUE;
/*N*/ 			SendRepaintBroadcast();
/*N*/ 			SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
/*N*/ 		}
/*N*/ 		((SdrEdgeObj*)this)->nNotifyingCount--;
/*N*/ 	}
/*N*/ }

/** updates edges that are connected to the edges of this object
    as if the connected objects send a repaint broadcast
    #103122#
*/
/*N*/ void SdrEdgeObj::Reformat()
/*N*/ {
/*N*/ 	if( NULL != aCon1.pObj )
/*N*/ 	{
/*N*/ 		SfxSimpleHint aHint( SFX_HINT_DATACHANGED );
/*N*/ 		SFX_NOTIFY( *const_cast<SfxBroadcaster*>(aCon1.pObj->GetBroadcaster()), NULL, aHint, NULL );
/*N*/ 	}
/*N*/ 
/*N*/ 	if( NULL != aCon2.pObj )
/*N*/ 	{
/*N*/ 		SfxSimpleHint aHint( SFX_HINT_DATACHANGED );
/*N*/ 		SFX_NOTIFY( *const_cast<SfxBroadcaster*>(aCon2.pObj->GetBroadcaster()), NULL, aHint, NULL );
/*N*/ 	}
/*N*/ }


























/*N*/ void SdrEdgeObj::NbcSetSnapRect(const Rectangle& rRect)
/*N*/ {
/*N*/ 	Rectangle aOld(GetSnapRect());
/*N*/ 	long nMulX = rRect.Right()  - rRect.Left();
/*N*/ 	long nDivX = aOld.Right()   - aOld.Left();
/*N*/ 	long nMulY = rRect.Bottom() - rRect.Top();
/*N*/ 	long nDivY = aOld.Bottom()  - aOld.Top();
/*N*/ 	if ( nDivX == 0 ) { nMulX = 1; nDivX = 1; }
/*N*/ 	if ( nDivY == 0 ) { nMulY = 1; nDivY = 1; }
/*N*/ 	Fraction aX(nMulX, nDivX);
/*N*/ 	Fraction aY(nMulY, nDivY);
/*N*/ 	NbcResize(aOld.TopLeft(), aX, aY);
/*N*/ 	NbcMove(Size(rRect.Left() - aOld.Left(), rRect.Top() - aOld.Top()));
/*N*/ }

/*N*/ void SdrEdgeObj::NbcMove(const Size& rSiz)
/*N*/ {
/*N*/ 	SdrTextObj::NbcMove(rSiz);
/*N*/ 	MoveXPoly(*pEdgeTrack,rSiz);
/*N*/ }

/*N*/ void SdrEdgeObj::NbcResize(const Point& rRefPnt, const Fraction& aXFact, const Fraction& aYFact)
/*N*/ {
/*N*/ 	SdrTextObj::NbcResize(rRefPnt,aXFact,aXFact);
/*N*/ 	ResizeXPoly(*pEdgeTrack,rRefPnt,aXFact,aYFact);
/*N*/ 
/*N*/ 	// #75371# if resize is not from paste, forget user distances
/*N*/ 	if(!GetModel()->IsPasteResize())
/*N*/ 	{
/*N*/ 		// #75735#
/*N*/ 		aEdgeInfo.aObj1Line2 = Point();
/*N*/ 		aEdgeInfo.aObj1Line3 = Point();
/*N*/ 		aEdgeInfo.aObj2Line2 = Point();
/*N*/ 		aEdgeInfo.aObj2Line3 = Point();
/*N*/ 		aEdgeInfo.aMiddleLine = Point();
/*N*/ 	}
/*N*/ }







/*N*/ void SdrEdgeObj::NbcSetPoint(const Point& rPnt, USHORT i)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	// ToDo: Umconnekten fehlt noch
/*N*/ }






////////////////////////////////////////////////////////////////////////////////////////////////////
// pre- and postprocessing for objects for saving

/*N*/ void SdrEdgeObj::PreSave()
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	SdrTextObj::PreSave();
/*N*/ 
/*N*/ 	// prepare SetItems for storage
/*N*/ 	const SfxItemSet& rSet = GetUnmergedItemSet();
/*N*/ 	const SfxItemSet* pParent = GetStyleSheet() ? &GetStyleSheet()->GetItemSet() : 0L;
/*N*/ 	SdrEdgeSetItem aEdgeAttr(rSet.GetPool());
/*N*/ 	aEdgeAttr.GetItemSet().Put(rSet);
/*N*/ 	aEdgeAttr.GetItemSet().SetParent(pParent);
/*N*/ 	mpObjectItemSet->Put(aEdgeAttr);
/*N*/ }

/*N*/ void SdrEdgeObj::PostSave()
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	SdrTextObj::PostSave();
/*N*/ 
/*N*/ 	// remove SetItems from local itemset
/*N*/ 	mpObjectItemSet->ClearItem(SDRATTRSET_EDGE);
/*N*/ }
/*N*/ 
/*N*/ ////////////////////////////////////////////////////////////////////////////////////////////////////
/*N*/ 
/*N*/ void SdrEdgeObj::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ 	SdrTextObj::WriteData(rOut);
/*N*/ 	SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrEdgeObj");
/*N*/ #endif
/*N*/ 
/*N*/ 	{
/*N*/ 		SdrDownCompat aTrackCompat(rOut,STREAM_WRITE); // ab V11 eingepackt
/*N*/ #ifdef DBG_UTIL
/*N*/ 		aTrackCompat.SetID("SdrEdgeObj(EdgeTrack)");
/*N*/ #endif
/*N*/ 		rOut << *pEdgeTrack;
/*N*/ 	}
/*N*/ 	
/*N*/ 	aCon1.Write(rOut, this); // Die Connections haben
/*N*/ 	aCon2.Write(rOut, this); // ihren eigenen Header
/*N*/ 	
/*N*/ 	SfxItemPool* pPool = GetItemPool();
/*N*/ 	
/*N*/ 	if(pPool) 
/*N*/ 	{
/*N*/ 		const SfxItemSet& rSet = GetUnmergedItemSet();
/*N*/ 
/*N*/ 		pPool->StoreSurrogate(rOut, &rSet.Get(SDRATTRSET_EDGE));
/*N*/ 	} 
/*N*/ 	else 
/*N*/ 	{
/*N*/ 		rOut << UINT16(SFX_ITEMS_NULL);
/*N*/ 	}
/*N*/ 	
/*N*/ 	rOut << aEdgeInfo;
/*N*/ }
/*N*/ 
/*N*/ void SdrEdgeObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	if (rIn.GetError()!=0) return;
/*N*/ 	SdrTextObj::ReadData(rHead,rIn);
/*N*/ 	if (rHead.GetVersion()<2) { // frueher war EdgeObj von PathObj abgeleitet
/*N*/ 		DBG_ERROR("SdrEdgeObj::ReadData(): Dateiversion<2 wird nicht mehr unterstuetzt");
/*N*/ 		rIn.SetError(SVSTREAM_WRONGVERSION); // Format-Fehler, File zu alt
/*N*/ 		return;
/*N*/ 	}
/*N*/ 	SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrEdgeObj");
/*N*/ #endif
/*N*/ 	if (rHead.GetVersion()>=11) { // ab V11 ist alles eingepackt
/*N*/ 		{
/*N*/ 			SdrDownCompat aTrackCompat(rIn,STREAM_READ); // ab V11 eingepackt
/*N*/ #ifdef DBG_UTIL
/*N*/ 			aTrackCompat.SetID("SdrEdgeObj(EdgeTrack)");
/*N*/ #endif
/*N*/ 			rIn>>*pEdgeTrack;
/*N*/ 		}
/*N*/ 		aCon1.Read(rIn,this); // Die Connections haben
/*N*/ 		aCon2.Read(rIn,this); // ihren eigenen Header.
/*N*/ 	} else {
/*N*/ 		rIn>>*pEdgeTrack;
/*N*/ 		if (rHead.GetBytesLeft()>0) { // Aha, da ist noch mehr (Verbindungsdaten)
/*N*/ 			aCon1.ReadTilV10(rIn,this); // Import der
/*N*/ 			aCon2.ReadTilV10(rIn,this); // Connections
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if(aCompat.GetBytesLeft() > 0) 
/*N*/ 	{ 
/*N*/ 		// ab 10-08-1996 (noch Vers 12) Items fuer Verbinder
/*N*/ 		SfxItemPool* pPool = GetItemPool();
/*N*/ 		if(pPool) 
/*N*/ 		{
/*N*/ 			sal_uInt16 nSetID = SDRATTRSET_EDGE;
/*N*/ 			const SdrEdgeSetItem* pEdgeAttr = (const SdrEdgeSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
/*N*/ 			if(pEdgeAttr)
/*N*/ 				SetItemSet(pEdgeAttr->GetItemSet());
/*N*/ 		} 
/*N*/ 		else 
/*N*/ 		{
/*N*/ 			sal_uInt16 nSuroDum;
/*N*/ 			rIn >> nSuroDum;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if(aCompat.GetBytesLeft() > 0) 
/*N*/ 	{ 
/*N*/ 		// ab 14-01-1997 (noch Vers 12) EdgeInfoRec
/*N*/ 		rIn >> aEdgeInfo;
/*N*/ 	}
/*N*/ }
/*N*/ 
/*N*/ void SdrEdgeObj::AfterRead()
/*N*/ {
/*N*/ 	SdrTextObj::AfterRead();
/*N*/ 	aCon1.AfterRead(this);
/*N*/ 	aCon2.AfterRead(this);
/*N*/ 	if (aCon1.pObj!=NULL) aCon1.pObj->AddListener(*this);
/*N*/ 	if (aCon2.pObj!=NULL) aCon2.pObj->AddListener(*this);
/*N*/ 
/*N*/ 	// #84026# always recalculate edgetrack after load
/*N*/ 	bEdgeTrackDirty=TRUE;
/*N*/ }
/*N*/ 
/*N*/ Point SdrEdgeObj::GetTailPoint( BOOL bTail ) const
/*N*/ {
/*N*/ 	if( pEdgeTrack && pEdgeTrack->GetPointCount()!=0)
/*N*/ 	{
/*N*/ 		const XPolygon& rTrack0 = *pEdgeTrack;
/*N*/ 		if(bTail)
/*N*/ 		{
/*N*/ 			return rTrack0[0];
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			const USHORT nSiz = rTrack0.GetPointCount() - 1;
/*N*/ 			return rTrack0[nSiz];
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if(bTail)
/*N*/ 			return aOutRect.TopLeft();
/*N*/ 		else
/*N*/ 			return aOutRect.BottomRight();
/*N*/ 	}
/*N*/ 
/*N*/ }
/*N*/ 
/*N*/ void SdrEdgeObj::SetTailPoint( BOOL bTail, const Point& rPt )
/*N*/ {
/*N*/ 	ImpSetTailPoint( bTail, rPt );
/*N*/ 	SetChanged();
/*N*/ }

/** this method is used by the api to set a glue point for a connection
    nId == -1 :		The best default point is automaticly choosen
    0 <= nId <= 3 : One of the default points is choosen
    nId >= 4 :		A user defined glue point is choosen
*/
/*N*/ void SdrEdgeObj::setGluePointIndex( sal_Bool bTail, sal_Int32 nIndex /* = -1 */ )
/*N*/ {
/*N*/ 	Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
/*N*/ 	SendRepaintBroadcast();
/*N*/ 
/*N*/ 	SdrObjConnection& rConn1 = GetConnection( bTail );
/*N*/ 
/*N*/ 	rConn1.SetAutoVertex( nIndex >= 0 && nIndex <= 3 );
/*N*/ 	rConn1.SetBestConnection( nIndex < 0 );
/*N*/ 	rConn1.SetBestVertex( nIndex < 0 );
/*N*/ 
/*N*/ 	if( nIndex > 3 )
/*N*/ 	{
/*N*/  		nIndex -= 4;
/*N*/  		// for user defined glue points we have
/*N*/  		// to get the id for this index first
/*N*/ 		const SdrGluePointList* pList = rConn1.GetObject() ? rConn1.GetObject()->GetGluePointList() : NULL;
/*N*/ 		if( pList == NULL || SDRGLUEPOINT_NOTFOUND == pList->FindGluePoint((sal_uInt16)nIndex) )
/*N*/ 			return;
/*N*/ 	}
/*N*/ 	else if( nIndex < 0 )
/*N*/ 	{
/*N*/ 		nIndex = 0;
/*N*/ 	}
/*N*/ 
/*N*/ 	rConn1.SetConnectorId( (USHORT)nIndex );
/*N*/ 
/*N*/ 	SetChanged();
/*N*/ 	SetRectsDirty();
/*N*/ 	ImpRecalcEdgeTrack();
/*N*/ 	bEdgeTrackDirty=TRUE;
/*N*/ }

/** this method is used by the api to return a glue point id for a connection.
    See setGluePointId for possible return values */
/*N*/ sal_Int32 SdrEdgeObj::getGluePointIndex( sal_Bool bTail )
/*N*/ {
/*N*/ 	SdrObjConnection& rConn1 = GetConnection( bTail );
/*N*/ 	sal_Int32 nId = -1;
/*N*/ 	if( !rConn1.IsBestConnection() )
/*N*/ 	{
/*N*/ 		nId = rConn1.GetConnectorId();
/*N*/ 		if( !rConn1.IsAutoVertex() )
/*N*/ 			nId += 4;
/*N*/ 	}
/*N*/ 	return nId;
/*N*/ }

// #102344# Implementation was missing; edge track needs to be invalidated additionally.
/*N*/ void SdrEdgeObj::NbcSetAnchorPos(const Point& rPnt)
/*N*/ {
/*N*/ 	// call parent functionality
/*N*/ 	SdrTextObj::NbcSetAnchorPos(rPnt);
/*N*/ 
/*N*/ 	// Additionally, invalidate edge track
/*N*/ 	bEdgeTrackDirty = TRUE;
/*N*/ }

// eof
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
