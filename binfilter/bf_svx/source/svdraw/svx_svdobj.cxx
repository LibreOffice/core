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

#include <vcl/cvtsvm.hxx>
#include <tools/line.hxx>
#include <vector>
#include "svdetc.hxx"
#include "svdio.hxx"
#include "svdpage.hxx"
#include "svdovirt.hxx"  // Fuer Add/Del Ref
#include "svdview.hxx"   // fuer Dragging (Ortho abfragen)
#include "svdstr.hrc"    // Objektname
#include "svdogrp.hxx"   // Factory
#include "svdopath.hxx"  // Factory
#include "svdocirc.hxx"  // Factory
#include "svdomeas.hxx"  // Factory
#include "svdograf.hxx"  // Factory
#include "svdoole2.hxx"  // Factory
#include "svdocapt.hxx"  // Factory
#include "svdopage.hxx"  // Factory
#include "svdouno.hxx"   // Factory

////////////////////////////////////////////////////////////////////////////////////////////////////

#include "xlnwtit.hxx"
#include "xlnstwit.hxx"
#include "xlnedwit.hxx"
#include "xlnstit.hxx"
#include "xlnedit.hxx"
#include "xlnstcit.hxx"
#include "xlnedcit.hxx"
#include "xlndsit.hxx"
#include "xlnclit.hxx"
#include "svditer.hxx"
#include "xlntrit.hxx"
#include "xlinjoit.hxx"
#include "unopage.hxx"
#include "eeitem.hxx"

#include "svdpool.hxx"

#include "editeng.hxx"

#include <vcl/salbtype.hxx>		// FRound

#include <bf_svtools/whiter.hxx>

// #97849#
#include "fmmodel.hxx"

#include <bf_sfx2/objsh.hxx>

#include <bf_sfx2/docfac.hxx>

#include "rectenum.hxx"

#include "svdoimp.hxx"

#include <vcl/graphictools.hxx>


#include "xoutx.hxx"

#include <bf_goodies/matrix3d.hxx>

namespace binfilter {

using namespace ::com::sun::star;

// #104018# replace macros above with type-detecting methods
inline double ImplTwipsToMM(double fVal) { return (fVal * (127.0 / 72.0)); }
inline double ImplMMToTwips(double fVal) { return (fVal * (72.0 / 127.0)); }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ TYPEINIT0(SdrObjUserCall);

/*N*/ SdrObjUserCall::~SdrObjUserCall()
/*N*/ {
/*N*/ }

/*N*/ void SdrObjUserCall::Changed(const SdrObject& rObj, SdrUserCallType eType, const Rectangle& rOldBoundRect)
/*N*/ {
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ TYPEINIT0(SdrObjUserData);




/*N*/ SdrObjUserData::~SdrObjUserData()
/*N*/ {
/*N*/ }

/*N*/ void SdrObjUserData::WriteData(SvStream& rOut)
/*N*/ {
/*N*/ 	rOut<<nInventor;
/*N*/ 	rOut<<nIdentifier;
/*N*/ 	rOut<<nVersion;
/*N*/ }

/*N*/ void SdrObjUserData::ReadData(SvStream& rIn)
/*N*/ {
/*N*/ 	//Inventor und Identifier wurden bereits von Aufrufer gelesen,
/*N*/ 	//sonst haette er mich ja nicht erzeugen koennen (kein SeekBack!).
/*N*/ 	rIn>>nVersion; // Miniatur-Versionsverwaltung.
/*N*/ }

/*N*/ void SdrObjUserData::AfterRead()
/*N*/ {
/*N*/ }

/*N*/ FASTBOOL SdrObjUserData::HasMacro(const SdrObject* pObj) const
/*N*/ {
/*N*/ 	return FALSE;
/*N*/ }






/*N*/ void SdrObjUserDataList::Clear()
/*N*/ {
/*N*/ 	USHORT nAnz=GetUserDataCount();
/*N*/ 	for (USHORT i=0; i<nAnz; i++) {
/*N*/ 		delete GetUserData(i);
/*N*/ 	}
/*N*/ 	aList.Clear();
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ DBG_NAME(SdrObjGeoData)

/*N*/ SdrObjGeoData::SdrObjGeoData():
/*N*/ 	pGPL(NULL),
/*N*/ 	bMovProt(FALSE),
/*N*/ 	bSizProt(FALSE),
/*N*/ 	bNoPrint(FALSE),
/*N*/ 	bClosedObj(FALSE),
/*N*/ 	nLayerId(0)
/*N*/ {
/*N*/ 	DBG_CTOR(SdrObjGeoData,NULL);
/*N*/ }

/*N*/ SdrObjGeoData::~SdrObjGeoData()
/*N*/ {
/*N*/ 	DBG_DTOR(SdrObjGeoData,NULL);
/*N*/ 	delete pGPL;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ TYPEINIT0(SdrObjPlusData);

/*N*/ SdrObjPlusData::SdrObjPlusData():
/*N*/ 	pBroadcast(NULL),
/*N*/ 	pUserDataList(NULL),
/*N*/ 	pGluePoints(NULL),
///*N*/ 	pAnimator(NULL),
/*N*/ 	pAutoTimer(NULL)
/*N*/ {
/*N*/ }

/*N*/ SdrObjPlusData::~SdrObjPlusData()
/*N*/ {
/*N*/ 	if (pBroadcast   !=NULL) delete pBroadcast;
/*N*/ 	if (pUserDataList!=NULL) delete pUserDataList;
/*N*/ 	if (pGluePoints  !=NULL) delete pGluePoints;
///*N*/ 	if (pAnimator    !=NULL) delete pAnimator;
/*N*/ 	if (pAutoTimer   !=NULL) delete pAutoTimer;
/*N*/ }


///////////////////////////////////////////////////////////////////////////////

static double SMALLEST_DASH_WIDTH(26.95);

/*N*/ ImpLineStyleParameterPack::ImpLineStyleParameterPack(const SfxItemSet& rSet,
/*N*/ 	BOOL _bForceHair, OutputDevice* pOut)
/*N*/ :	mpOut(pOut),
/*N*/ 	rStartPolygon(((const XLineStartItem&)(rSet.Get(XATTR_LINESTART))).GetValue()),
/*N*/ 	rEndPolygon(((const XLineEndItem&)(rSet.Get(XATTR_LINEEND))).GetValue()),
/*N*/ 	bForceNoArrowsLeft(FALSE),
/*N*/ 	bForceNoArrowsRight(FALSE),
/*N*/     bForceHair(_bForceHair)
/*N*/ {
/*N*/     // #i12227# now storing the real line width, not corrected by
/*N*/     // bForceHair. This is done within the GetDisplay*Width accessors,
/*N*/     // and preserves the true value for the Get*Width accessors.
/*N*/ 	nLineWidth = ((const XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue();
/*N*/ 	eLineStyle = (XLineStyle)((const XLineStyleItem&)rSet.Get(XATTR_LINESTYLE)).GetValue();
/*N*/
/*N*/ 	nStartWidth = ((const XLineStartWidthItem&)(rSet.Get(XATTR_LINESTARTWIDTH))).GetValue();
/*N*/ 	if(nStartWidth < 0)
/*N*/ 		nStartWidth = -nLineWidth * nStartWidth / 100;
/*N*/
/*N*/ 	nEndWidth = ((const XLineEndWidthItem&)(rSet.Get(XATTR_LINEENDWIDTH))).GetValue();
/*N*/ 	if(nEndWidth < 0)
/*N*/ 		nEndWidth = -nLineWidth * nEndWidth / 100;
/*N*/
/*N*/ 	bStartCentered = ((const XLineStartCenterItem&)(rSet.Get(XATTR_LINESTARTCENTER))).GetValue();
/*N*/ 	bEndCentered = ((const XLineEndCenterItem&)(rSet.Get(XATTR_LINEENDCENTER))).GetValue();
/*N*/
/*N*/ 	fDegreeStepWidth = 10.0;
/*N*/ 	eLineJoint = ((const XLineJointItem&)(rSet.Get(XATTR_LINEJOINT))).GetValue();
/*N*/
/*N*/ 	aDash = ((const XLineDashItem&)(rSet.Get(XATTR_LINEDASH))).GetValue();
/*N*/
/*N*/ 	// fill local dash info
/*N*/ 	UINT16 nNumDotDashArray = (GetDots() + GetDashes()) * 2;
/*N*/ 	aDotDashArray.resize( nNumDotDashArray, 0.0 );
/*N*/ 	UINT16 a;
/*N*/ 	UINT16 nIns = 0;
/*N*/ 	double fDashDotDistance = (double)GetDashDistance();
/*N*/ 	double fSingleDashLen = (double)GetDashLen();
/*N*/ 	double fSingleDotLen = (double)GetDotLen();
/*N*/ 	double fLineWidth = (double)GetDisplayLineWidth();
/*N*/
/*N*/ 	if(GetDashStyle() == XDASH_RECTRELATIVE || GetDashStyle() == XDASH_ROUNDRELATIVE)
/*?*/ 	{
/*?*/ 		if(GetDisplayLineWidth())
/*?*/ 		{
/*?*/ 			double fFactor = fLineWidth / 100.0;
/*?*/
/*?*/ 			if(GetDashes())
/*?*/ 			{
/*?*/ 				if(GetDashLen())
/*?*/ 				{
/*?*/ 					// is a dash
/*?*/ 					fSingleDashLen *= fFactor;
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					// is a dot
/*?*/ 					fSingleDashLen = fLineWidth;
/*?*/ 				}
/*?*/ 			}
/*?*/
/*?*/ 			if(GetDots())
/*?*/ 			{
/*?*/ 				if(GetDotLen())
/*?*/ 				{
/*?*/ 					// is a dash
/*?*/ 					fSingleDotLen *= fFactor;
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					// is a dot
/*?*/ 					fSingleDotLen = fLineWidth;
/*?*/ 				}
/*?*/ 			}
/*?*/
/*?*/ 			if(GetDashes() || GetDots())
/*?*/ 			{
/*?*/ 				if(GetDashDistance())
/*?*/ 					fDashDotDistance *= fFactor;
/*?*/ 				else
/*?*/ 					fDashDotDistance = fLineWidth;
/*?*/ 			}
/*?*/ 		}
/*?*/ 		else
/*?*/ 		{
/*?*/ 			if(GetDashes())
/*?*/ 			{
/*?*/ 				if(GetDashLen())
/*?*/ 				{
/*?*/ 					// is a dash
/*?*/ 					fSingleDashLen = (SMALLEST_DASH_WIDTH * fSingleDashLen) / 100.0;
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					// is a dot
/*?*/ 					fSingleDashLen = SMALLEST_DASH_WIDTH;
/*?*/ 				}
/*?*/ 			}
/*?*/
/*?*/ 			if(GetDots())
/*?*/ 			{
/*?*/ 				if(GetDotLen())
/*?*/ 				{
/*?*/ 					// is a dash
/*?*/ 					fSingleDotLen = (SMALLEST_DASH_WIDTH * fSingleDotLen) / 100.0;
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					// is a dot
/*?*/ 					fSingleDotLen = SMALLEST_DASH_WIDTH;
/*?*/ 				}
/*?*/ 			}
/*?*/
/*?*/ 			if(GetDashes() || GetDots())
/*?*/ 			{
/*?*/ 				if(GetDashDistance())
/*?*/ 				{
/*?*/ 					// dash as distance
/*?*/ 					fDashDotDistance = (SMALLEST_DASH_WIDTH * fDashDotDistance) / 100.0;
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					// dot as distance
/*?*/ 					fDashDotDistance = SMALLEST_DASH_WIDTH;
/*?*/ 				}
/*?*/ 			}
/*?*/ 		}
/*?*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// smallest dot size compare value
/*N*/ 		double fDotCompVal(GetDisplayLineWidth() ? fLineWidth : SMALLEST_DASH_WIDTH);
/*N*/
/*N*/ 		// absolute values
/*N*/ 		if(GetDashes())
/*N*/ 		{
/*N*/ 			if(GetDashLen())
/*N*/ 			{
/*N*/ 				// is a dash
/*N*/ 				if(fSingleDashLen < SMALLEST_DASH_WIDTH)
/*N*/ 					fSingleDashLen = SMALLEST_DASH_WIDTH;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				// is a dot
/*?*/ 				if(fSingleDashLen < fDotCompVal)
/*?*/ 					fSingleDashLen = fDotCompVal;
/*N*/ 			}
/*N*/ 		}
/*N*/
/*N*/ 		if(GetDots())
/*N*/ 		{
/*N*/ 			if(GetDotLen())
/*N*/ 			{
/*N*/ 				// is a dash
/*N*/ 				if(fSingleDotLen < SMALLEST_DASH_WIDTH)
/*N*/ 					fSingleDotLen = SMALLEST_DASH_WIDTH;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				// is a dot
/*?*/ 				if(fSingleDotLen < fDotCompVal)
/*?*/ 					fSingleDotLen = fDotCompVal;
/*N*/ 			}
/*N*/ 		}
/*N*/
/*N*/ 		if(GetDashes() || GetDots())
/*N*/ 		{
/*N*/ 			if(GetDashDistance())
/*N*/ 			{
/*N*/ 				// dash as distance
/*N*/ 				if(fDashDotDistance < SMALLEST_DASH_WIDTH)
/*N*/ 					fDashDotDistance = SMALLEST_DASH_WIDTH;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				// dot as distance
/*?*/ 				if(fDashDotDistance < fDotCompVal)
/*?*/ 					fDashDotDistance = fDotCompVal;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	fFullDashDotLen = 0.0;
/*N*/
/*N*/ 	for(a=0;a<GetDots();a++)
/*N*/ 	{
/*N*/ 		aDotDashArray[nIns++] = fSingleDotLen;
/*N*/ 		fFullDashDotLen += fSingleDotLen;
/*N*/ 		aDotDashArray[nIns++] = fDashDotDistance;
/*N*/ 		fFullDashDotLen += fDashDotDistance;
/*N*/ 	}
/*N*/
/*N*/ 	for(a=0;a<GetDashes();a++)
/*N*/ 	{
/*N*/ 		aDotDashArray[nIns++] = fSingleDashLen;
/*N*/ 		fFullDashDotLen += fSingleDashLen;
/*N*/ 		aDotDashArray[nIns++] = fDashDotDistance;
/*N*/ 		fFullDashDotLen += fDashDotDistance;
/*N*/ 	}
/*N*/ }

/*N*/ ImpLineStyleParameterPack::~ImpLineStyleParameterPack()
/*N*/ {
/*N*/ }

/*N*/ UINT16 ImpLineStyleParameterPack::GetFirstDashDotIndex(double fPos, double& rfDist) const
/*N*/ {
/*N*/ 	double fIndPos = fPos - (fFullDashDotLen * (double)((UINT32)(fPos / fFullDashDotLen)));
/*N*/ 	UINT16 nPos = 0;
/*N*/
/*N*/ 	while(fIndPos && fIndPos - aDotDashArray[nPos] > -SMALL_DVALUE)
/*N*/ 	{
/*N*/ 		fIndPos -= aDotDashArray[nPos];
/*N*/ 		nPos = (static_cast< size_t >(nPos + 1) == aDotDashArray.size()) ? 0 : nPos + 1;
/*N*/ 	}
/*N*/
/*N*/ 	rfDist = aDotDashArray[nPos] - fIndPos;
/*N*/ 	nPos = (static_cast< size_t >(nPos + 1) == aDotDashArray.size()) ? 0 : nPos + 1;
/*N*/
/*N*/ 	return nPos;
/*N*/ }

/*N*/ UINT16 ImpLineStyleParameterPack::GetNextDashDotIndex(UINT16 nPos, double& rfDist) const
/*N*/ {
/*N*/ 	rfDist = aDotDashArray[nPos];
/*N*/ 	nPos = (static_cast< size_t >(nPos + 1) == aDotDashArray.size()) ? 0 : nPos + 1;
/*N*/ 	return nPos;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ double ImpLineGeometryCreator::ImpSimpleFindCutPoint(
/*N*/ 	const Vector3D& rEdge1Start, const Vector3D& rEdge1Delta,
/*N*/ 	const Vector3D& rEdge2Start, const Vector3D& rEdge2Delta)
/*N*/ {
/*N*/ 	double fZwi = (rEdge1Delta.X() * rEdge2Delta.Y()) - (rEdge1Delta.Y() * rEdge2Delta.X());
/*N*/ 	double fRetval = 0.0;
/*N*/
/*N*/ 	if(fabs(fZwi) > SMALL_DVALUE)
/*N*/ 	{
/*N*/ 		fRetval = (rEdge2Delta.Y() * (rEdge2Start.X() - rEdge1Start.X())
/*N*/ 			+ rEdge2Delta.X() * (rEdge1Start.Y() - rEdge2Start.Y())) / fZwi;
/*N*/ 	}
/*N*/ 	return fRetval;
/*N*/ }

/*N*/ void ImpLineGeometryCreator::ImpCreateLineSegment(const Vector3D* pPrev, const Vector3D* pLeft, const Vector3D* pRight, const Vector3D* pNext)
/*N*/ {
/*N*/ 	if(mrLineAttr.GetDisplayLineWidth())
/*N*/ 	{
/*N*/ 		double fHalfLineWidth((double)mrLineAttr.GetDisplayLineWidth() / 2.0);
/*N*/ 		Vector3D aEdge = *pRight - *pLeft;
/*N*/
/*N*/ 		// #78972#
/*N*/ 		Vector3D aPerpend(-aEdge.Y(), aEdge.X(), 0.0);
/*N*/ 		aPerpend.Normalize();
/*N*/
/*N*/ 		XLineJoint eJoint = mrLineAttr.GetLineJoint();
/*N*/
/*N*/ 		// joints need eventually not be done
/*N*/ 		if((eJoint == XLINEJOINT_MIDDLE || eJoint == XLINEJOINT_MITER) && (!pPrev && !pNext))
/*N*/ 			eJoint = XLINEJOINT_NONE;
/*N*/
/*N*/ 		switch(eJoint)
/*N*/ 		{
/*?*/ 			case XLINEJOINT_NONE: 		// no rounding
/*?*/ 			{
/*?*/ 				Polygon3D aNewPoly(4);
/*?*/
/*?*/ 				aPerpend *= fHalfLineWidth;
/*?*/ 				aNewPoly[0] = *pLeft + aPerpend;
/*?*/ 				aNewPoly[1] = *pRight + aPerpend;
/*?*/ 				aNewPoly[2] = *pRight - aPerpend;
/*?*/ 				aNewPoly[3] = *pLeft - aPerpend;
/*?*/
/*?*/ 				aNewPoly.SetClosed(TRUE);
/*?*/ 				mrPolyPoly3D.Insert(aNewPoly);
/*?*/
/*?*/ 				break;
/*?*/ 			}
/*?*/ 			case XLINEJOINT_MIDDLE: 	// calc middle value between joints
/*?*/ 			{
/*?*/ 				Polygon3D aNewPoly(4);
/*?*/ 				Vector3D aPerpendLeft(aPerpend);
/*?*/ 				Vector3D aPerpendRight(aPerpend);
/*?*/
/*?*/ 				if(pPrev)
/*?*/ 				{
/*?*/ 					aPerpendLeft = *pLeft - *pPrev;
/*?*/
/*?*/ 					// #78972#
/*?*/ 					aPerpendLeft = Vector3D(-aPerpendLeft.Y(), aPerpendLeft.X(), 0.0);
/*?*/ 					aPerpendLeft.Normalize();
/*?*/ 				}
/*?*/
/*?*/ 				if(pNext)
/*?*/ 				{
/*?*/ 					aPerpendRight = *pNext - *pRight;
/*?*/
/*?*/ 					// #78972#
/*?*/ 					aPerpendRight = Vector3D(-aPerpendRight.Y(), aPerpendRight.X(), 0.0);
/*?*/ 					aPerpendRight.Normalize();
/*?*/ 				}
/*?*/
/*?*/ 				aPerpendLeft = (aPerpend + aPerpendLeft) * (fHalfLineWidth / 2.0);
/*?*/ 				aPerpendRight = (aPerpend + aPerpendRight) * (fHalfLineWidth / 2.0);
/*?*/
/*?*/ 				aNewPoly[0] = *pLeft + aPerpendLeft;
/*?*/ 				aNewPoly[1] = *pRight + aPerpendRight;
/*?*/ 				aNewPoly[2] = *pRight - aPerpendRight;
/*?*/ 				aNewPoly[3] = *pLeft - aPerpendLeft;
/*?*/
/*?*/ 				aNewPoly.SetClosed(TRUE);
/*?*/ 				mrPolyPoly3D.Insert(aNewPoly);
/*?*/
/*?*/ 				break;
/*?*/ 			}
/*N*/ 			case XLINEJOINT_BEVEL: 		// join edges with line
/*N*/ 			default: // #73428# case XLINEJOINT_ROUND: 		// create arc
/*N*/ 			{
/*N*/ 				Vector3D aPerpendRight(aPerpend);
/*N*/ 				BOOL bCreateSimplePart(TRUE);
/*N*/
/*N*/ 				if(pNext)
/*N*/ 				{
/*N*/ 					aPerpendRight = *pNext - *pRight;
/*N*/
/*N*/ 					// #78972#
/*N*/ 					aPerpendRight = Vector3D(-aPerpendRight.Y(), aPerpendRight.X(), 0.0);
/*N*/ 					aPerpendRight.Normalize();
/*N*/
/*N*/ 					double fAngle = atan2(aPerpend.Y(), aPerpend.X());
/*N*/ 					double fRightAngle = atan2(aPerpendRight.Y(), aPerpendRight.X());
/*N*/ 					double fAngleDiff = fAngle - fRightAngle;
/*N*/ 					double fDegreeStepWidth = mrLineAttr.GetDegreeStepWidth() * F_PI180;
/*N*/
/*N*/ 					// go to range [0.0..2*F_PI[
/*N*/ 					while(fAngleDiff < 0.0)
/*N*/ 						fAngleDiff += (F_PI * 2.0);
/*N*/ 					while(fAngleDiff >= (F_PI * 2.0))
/*N*/ 						fAngleDiff -= (F_PI * 2.0);
/*N*/
/*N*/ 					if((fAngleDiff > fDegreeStepWidth) && (fAngleDiff < ((F_PI * 2.0) - fDegreeStepWidth)))
/*N*/ 					{
/*N*/ 						bCreateSimplePart = FALSE;
/*N*/ 						aPerpend *= fHalfLineWidth;
/*N*/ 						aPerpendRight *= fHalfLineWidth;
/*N*/
/*N*/ 						if(eJoint == XLINEJOINT_BEVEL)
/*?*/ 						{
/*?*/ 							UINT16 nPolyPoints(pPrev ? 7 : 6);
/*?*/ 							Polygon3D aNewPoly(nPolyPoints);
/*?*/
/*?*/ 							aNewPoly[0] = *pLeft + aPerpend;
/*?*/ 							aNewPoly[1] = *pRight + aPerpend;
/*?*/ 							aNewPoly[4] = *pRight - aPerpend;
/*?*/ 							aNewPoly[5] = *pLeft - aPerpend;
/*?*/
/*?*/ 							if(pPrev)
/*?*/ 								aNewPoly[6] = *pLeft;
/*?*/
/*?*/ 							if(fAngleDiff > F_PI)
/*?*/ 							{
/*?*/ 								// lower side
/*?*/ 								aNewPoly[2] = *pRight;
/*?*/ 								aNewPoly[3] = *pRight - aPerpendRight;
/*?*/ 							}
/*?*/ 							else
/*?*/ 							{
/*?*/ 								// upper side
/*?*/ 								aNewPoly[2] = *pRight + aPerpendRight;
/*?*/ 								aNewPoly[3] = *pRight;
/*?*/ 							}
/*?*/
/*?*/ 							aNewPoly.SetClosed(TRUE);
/*?*/ 							mrPolyPoly3D.Insert(aNewPoly);
/*?*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 							BOOL bUseLowerSide(fAngleDiff > F_PI);
/*N*/ 							UINT16 nSegments;
/*N*/
/*N*/ 							if(bUseLowerSide)
/*N*/ 							{
/*?*/ 								fAngleDiff = (F_PI * 2.0) - fAngleDiff;
/*?*/ 								nSegments = (UINT16)(fAngleDiff / fDegreeStepWidth);
/*N*/ 							}
/*N*/ 							else
/*N*/ 							{
/*N*/ 								nSegments = (UINT16)(fAngleDiff / fDegreeStepWidth);
/*N*/ 							}
/*N*/
/*N*/ 							UINT16 nPolyPoints(pPrev ? 7 : 6);
/*N*/ 							Polygon3D aNewPoly(nPolyPoints + nSegments);
/*N*/
/*N*/ 							aNewPoly[0] = *pLeft + aPerpend;
/*N*/ 							aNewPoly[1] = *pRight + aPerpend;
/*N*/ 							aNewPoly[4 + nSegments] = *pRight - aPerpend;
/*N*/ 							aNewPoly[5 + nSegments] = *pLeft - aPerpend;
/*N*/
/*N*/ 							if(pPrev)
/*N*/ 								aNewPoly[6 + nSegments] = *pLeft;
/*N*/
/*N*/ 							fAngleDiff /= (double)(nSegments + 1);
/*N*/
/*N*/ 							if(bUseLowerSide)
/*N*/ 							{
/*N*/ 								// lower side
/*?*/ 								aNewPoly[2] = *pRight;
/*?*/ 								aNewPoly[3] = *pRight - aPerpendRight;
/*?*/
/*?*/ 								for(UINT16 a=0;a<nSegments;a++)
/*?*/ 								{
/*?*/ 									double fDegree = fRightAngle - (double)a * fAngleDiff;
/*?*/ 									Vector3D aNewPos(
/*?*/ 										pRight->X() - (cos(fDegree) * fHalfLineWidth),
/*?*/ 										pRight->Y() - (sin(fDegree) * fHalfLineWidth),
/*?*/ 										pRight->Z()); // #78972#
/*?*/ 									aNewPoly[4 + a] = aNewPos;
/*N*/ 								}
/*N*/ 							}
/*N*/ 							else
/*N*/ 							{
/*N*/ 								// upper side
/*N*/ 								aNewPoly[2 + nSegments] = *pRight + aPerpendRight;
/*N*/ 								aNewPoly[3 + nSegments] = *pRight;
/*N*/
/*N*/ 								for(UINT16 a=0;a<nSegments;a++)
/*N*/ 								{
/*N*/ 									double fDegree = fAngle - (double)a * fAngleDiff;
/*N*/ 									Vector3D aNewPos(
/*N*/ 										pRight->X() + (cos(fDegree) * fHalfLineWidth),
/*N*/ 										pRight->Y() + (sin(fDegree) * fHalfLineWidth),
/*N*/ 										pRight->Z()); // #78972#
/*N*/ 									aNewPoly[2 + a] = aNewPos;
/*N*/ 								}
/*N*/ 							}
/*N*/
/*N*/ 							aNewPoly.SetClosed(TRUE);
/*N*/ 							mrPolyPoly3D.Insert(aNewPoly);
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/
/*N*/ 				if(bCreateSimplePart)
/*N*/ 				{
/*N*/ 					// angle smaller DegreeStepWidth, create simple segment
/*N*/ 					UINT16 nNumPnt(4);
/*N*/
/*N*/ 					if(pPrev)
/*N*/ 						nNumPnt++;
/*N*/
/*N*/ 					if(pNext)
/*N*/ 						nNumPnt++;
/*N*/
/*N*/ 					Polygon3D aNewPoly(nNumPnt);
/*N*/
/*N*/ 					aPerpend *= fHalfLineWidth;
/*N*/ 					aPerpendRight *= fHalfLineWidth;
/*N*/ 					nNumPnt = 0;
/*N*/
/*N*/ 					if(pPrev)
/*N*/ 						aNewPoly[nNumPnt++] = *pLeft;
/*N*/
/*N*/ 					aNewPoly[nNumPnt++] = *pLeft + aPerpend;
/*N*/ 					aNewPoly[nNumPnt++] = *pRight + aPerpendRight;
/*N*/
/*N*/ 					if(pNext)
/*N*/ 						aNewPoly[nNumPnt++] = *pRight;
/*N*/
/*N*/ 					aNewPoly[nNumPnt++] = *pRight - aPerpendRight;
/*N*/ 					aNewPoly[nNumPnt++] = *pLeft - aPerpend;
/*N*/
/*N*/ 					aNewPoly.SetClosed(TRUE);
/*N*/ 					mrPolyPoly3D.Insert(aNewPoly);
/*N*/ 				}
/*N*/
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			case XLINEJOINT_MITER: 		// extend till cut
/*?*/ 			{
/*?*/ 				Polygon3D aNewPoly(4);
/*?*/ 				aPerpend *= fHalfLineWidth;
/*?*/ 				BOOL bLeftSolved(FALSE);
/*?*/ 				BOOL bRightSolved(FALSE);
/*?*/
/*?*/ 				if(pPrev)
/*?*/ 				{
/*?*/ 					Vector3D aLeftVec(*pLeft - *pPrev);
/*?*/
/*?*/ 					// #78972#
/*?*/ 					Vector3D aPerpendLeft(-aLeftVec.Y(), aLeftVec.X(), 0.0);
/*?*/ 					aPerpendLeft.Normalize();
/*?*/
/*?*/ 					aPerpendLeft *= fHalfLineWidth;
/*?*/ 					double fUpperCut = ImpSimpleFindCutPoint(*pPrev + aPerpendLeft, aLeftVec, *pRight + aPerpend, -aEdge);
/*?*/
/*?*/ 					if(fUpperCut != 0.0 && fUpperCut < mrLineAttr.GetLinejointMiterUpperBound())
/*?*/ 					{
/*?*/ 						double fLowerCut = ImpSimpleFindCutPoint(*pPrev - aPerpendLeft, aLeftVec, *pRight - aPerpend, -aEdge);
/*?*/
/*?*/ 						if(fLowerCut < mrLineAttr.GetLinejointMiterUpperBound())
/*?*/ 						{
/*?*/ 							Vector3D aParam1 = *pPrev + aPerpendLeft;
/*?*/ 							Vector3D aParam2 = *pLeft + aPerpendLeft;
/*?*/ 							aNewPoly[0].CalcInBetween(aParam1, aParam2, fUpperCut);
/*?*/ 							aParam1 = *pPrev - aPerpendLeft;
/*?*/ 							aParam2 = *pLeft - aPerpendLeft;
/*?*/ 							aNewPoly[3].CalcInBetween(aParam1, aParam2, fLowerCut);
/*?*/ 							bLeftSolved = TRUE;
/*?*/ 						}
/*?*/ 					}
/*?*/ 				}
/*?*/ 				if(!bLeftSolved)
/*?*/ 				{
/*?*/ 					aNewPoly[0] = *pLeft + aPerpend;
/*?*/ 					aNewPoly[3] = *pLeft - aPerpend;
/*?*/ 				}
/*?*/
/*?*/ 				if(pNext)
/*?*/ 				{
/*?*/ 					Vector3D aRightVec(*pRight - *pNext);
/*?*/ 					Vector3D aPerpendRight = -aRightVec;
/*?*/
/*?*/ 					// #78972#
/*?*/ 					aPerpendRight = Vector3D(-aPerpendRight.Y(), aPerpendRight.X(), 0.0);
/*?*/ 					aPerpendRight.Normalize();
/*?*/
/*?*/ 					aPerpendRight *= fHalfLineWidth;
/*?*/ 					double fUpperCut = ImpSimpleFindCutPoint(*pNext + aPerpendRight, aRightVec, *pRight + aPerpend, aEdge);
/*?*/
/*?*/ 					if(fUpperCut != 0.0 && fUpperCut < mrLineAttr.GetLinejointMiterUpperBound())
/*?*/ 					{
/*?*/ 						double fLowerCut = ImpSimpleFindCutPoint(*pNext - aPerpendRight, aRightVec, *pRight - aPerpend, aEdge);
/*?*/
/*?*/ 						if(fLowerCut < mrLineAttr.GetLinejointMiterUpperBound())
/*?*/ 						{
/*?*/ 							Vector3D aParam1 = *pNext + aPerpendRight;
/*?*/ 							Vector3D aParam2 = *pRight + aPerpendRight;
/*?*/ 							aNewPoly[1].CalcInBetween(aParam1, aParam2, fUpperCut);
/*?*/ 							aParam1 = *pNext - aPerpendRight;
/*?*/ 							aParam2 = *pRight - aPerpendRight;
/*?*/ 							aNewPoly[2].CalcInBetween(aParam1, aParam2, fLowerCut);
/*?*/ 							bRightSolved = TRUE;
/*?*/ 						}
/*?*/ 					}
/*?*/ 				}
/*?*/ 				if(!bRightSolved)
/*?*/ 				{
/*?*/ 					aNewPoly[1] = *pRight + aPerpend;
/*?*/ 					aNewPoly[2] = *pRight - aPerpend;
/*?*/ 				}
/*?*/
/*?*/ 				aNewPoly.SetClosed(TRUE);
/*?*/ 				mrPolyPoly3D.Insert(aNewPoly);
/*?*/
/*?*/ 				break;
/*?*/ 			}
/*?*/ 		}
/*?*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		Polygon3D aNewPoly(2);
/*?*/
/*?*/ 		aNewPoly[0] = *pLeft;
/*?*/ 		aNewPoly[1] = *pRight;
/*?*/
/*?*/ 		aNewPoly.SetClosed(FALSE);
/*?*/ 		mrPolyLine3D.Insert(aNewPoly);
/*N*/ 	}
/*N*/ }

/*N*/ void ImpLineGeometryCreator::ImpCreateSegmentsForLine(const Vector3D* pPrev, const Vector3D* pLeft, const Vector3D* pRight, const Vector3D* pNext, double fPolyPos)
/*N*/ {
/*N*/ 	Vector3D aEdge(*pRight - *pLeft);
/*N*/ 	double fLen = aEdge.GetLength();
/*N*/ 	double fPos = 0.0;
/*N*/ 	double fDist;
/*N*/ 	BOOL bFirst(TRUE);
/*N*/ 	BOOL bLast(FALSE);
/*N*/ 	UINT16 nInd = mrLineAttr.GetFirstDashDotIndex(fPolyPos, fDist);
/*N*/
/*N*/ 	do {
/*N*/ 		// nInd right edge, fDist to it
/*N*/ 		if((nInd % 2) && fDist > SMALL_DVALUE)
/*N*/ 		{
/*N*/ 			// left is fpos, get right
/*N*/ 			double fRight = fPos + fDist;
/*N*/
/*N*/ 			if(fRight > fLen)
/*N*/ 			{
/*N*/ 				fRight = fLen;
/*N*/ 				bLast = TRUE;
/*N*/ 			}
/*N*/
/*N*/ 			// create segment from fPos to fRight
/*N*/ 			Vector3D aLeft(*pLeft);
/*N*/ 			Vector3D aRight(*pRight);
/*N*/
/*N*/ 			if(!bFirst)
/*N*/ 				aLeft.CalcInBetween(*pLeft, *pRight, fPos / fLen);
/*N*/ 			if(!bLast)
/*N*/ 				aRight.CalcInBetween(*pLeft, *pRight, fRight / fLen);
/*N*/
/*N*/ 			ImpCreateLineSegment(bFirst ? pPrev : 0L, &aLeft, &aRight, bLast ? pNext : 0L);
/*N*/ 		}
/*N*/
/*N*/ 		bFirst = FALSE;
/*N*/ 		fPos += fDist;
/*N*/ 		nInd = mrLineAttr.GetNextDashDotIndex(nInd, fDist);
/*N*/ 	} while(fPos < fLen);
/*N*/ }

/*N*/ double ImpLineGeometryCreator::ImpCreateLineStartEnd(Polygon3D& rArrowPoly, const Polygon3D& rSourcePoly, BOOL bFront, double fWantedWidth, BOOL bCentered)
/*N*/ {
/*N*/ 	double fRetval(0.0);
/*N*/ 	double fOffset(0.0);
/*N*/ 	Volume3D aPolySize(rArrowPoly.GetPolySize());
/*N*/ 	double fScaleValue(fWantedWidth / aPolySize.GetWidth());
/*N*/ 	Matrix4D aTrans;
/*N*/ 	Vector3D aCenter;
/*N*/
/*N*/ 	if(bCentered)
/*N*/ 	{
/*N*/ 		aCenter = Vector3D(
/*N*/ 			(aPolySize.MinVec().X() + aPolySize.MaxVec().X()) / 2.0,
/*N*/ 			(aPolySize.MinVec().Y() + aPolySize.MaxVec().Y()) / 2.0, 0.0);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		aCenter = Vector3D(rArrowPoly.GetMiddle());
/*N*/ 	}
/*N*/
/*N*/ 	aTrans.Translate(-aCenter);
/*N*/ 	aTrans.Scale(fScaleValue, fScaleValue, fScaleValue);
/*N*/
/*N*/ 	if(bCentered)
/*N*/ 	{
/*N*/ 		Vector3D aLowerCenter(aCenter.X(), aPolySize.MinVec().Y(), 0.0);
/*N*/
/*N*/ 		aLowerCenter *= aTrans;
/*N*/ 		aCenter *= aTrans;
/*N*/ 		fOffset = (aCenter - aLowerCenter).GetLength();
/*N*/ 		fRetval = fOffset / 2.0;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		Vector3D aLowerCenter(aCenter.X(), aPolySize.MinVec().Y(), 0.0);
/*N*/ 		Vector3D aUpperCenter(aCenter.X(), aPolySize.MaxVec().Y(), 0.0);
/*N*/
/*N*/ 		aUpperCenter *= aTrans;
/*N*/ 		aLowerCenter *= aTrans;
/*N*/ 		fOffset = (aUpperCenter - aLowerCenter).GetLength();
/*N*/ 		fRetval = fOffset * 0.8;
/*N*/ 	}
/*N*/
/*N*/ 	Vector3D aHead = (bFront) ? rSourcePoly[0] : rSourcePoly[rSourcePoly.GetPointCount() - 1];
/*N*/ 	Vector3D aTail = (bFront) ? rSourcePoly[1] : rSourcePoly[rSourcePoly.GetPointCount() - 2];
/*N*/
/*N*/ 	if(fOffset != 0.0)
/*N*/ 	{
/*N*/ 		if(!bFront)
/*N*/ 			fOffset = rSourcePoly.GetLength() - fOffset;
/*N*/ 		aTail = rSourcePoly.GetPosition(fOffset);
/*N*/ 	}
/*N*/
/*N*/ 	Vector3D aDirection = aHead - aTail;
/*N*/ 	aDirection.Normalize();
/*N*/ 	double fRotation = atan2(aDirection.Y(), aDirection.X()) - (90.0 * F_PI180);
/*N*/
/*N*/ 	aTrans.RotateZ(fRotation);
/*N*/ 	aTrans.Translate(aHead);
/*N*/
/*N*/ 	if(!bCentered)
/*N*/ 	{
/*N*/ 		Vector3D aUpperCenter(aCenter.X(), aPolySize.MaxVec().Y(), 0.0);
/*N*/
/*N*/ 		aUpperCenter *= aTrans;
/*N*/ 		aCenter *= aTrans;
/*N*/ 		aTrans.Translate(aCenter - aUpperCenter);
/*N*/ 	}
/*N*/
/*N*/ 	rArrowPoly.Transform(aTrans);
/*N*/ 	rArrowPoly.SetClosed(TRUE);
/*N*/
/*N*/ 	return fRetval;
/*N*/ }

/*N*/ void ImpLineGeometryCreator::ImpCreateLineGeometry(const Polygon3D& rSourcePoly)
/*N*/ {
/*N*/ 	UINT16 nPntCnt = rSourcePoly.GetPointCount();
/*N*/
/*N*/ 	if(nPntCnt > 1)
/*N*/ 	{
/*N*/ 		BOOL bClosed = rSourcePoly.IsClosed();
/*N*/ 		UINT16 nCount = nPntCnt;
/*N*/ 		Polygon3D aPoly = rSourcePoly;
/*N*/
/*N*/ 		if(!bClosed)
/*N*/ 		{
/*N*/ 			nCount = nPntCnt-1;
/*N*/ 			double fPolyLength = rSourcePoly.GetLength();
/*N*/ 			double fStart = 0.0;
/*N*/ 			double fEnd = fPolyLength;
/*N*/
/*N*/ 			if(mrLineAttr.IsStartActive())
/*N*/ 			{
/*?*/ 				// create line start polygon and move line end
/*?*/ 				Polygon3D aArrowPoly(XOutCreatePolygon(mrLineAttr.GetStartPolygon(), mrLineAttr.GetOutDev()));
/*?*/ 				fStart = ImpCreateLineStartEnd(
/*?*/ 					aArrowPoly, rSourcePoly, TRUE,
/*?*/ 					(double)mrLineAttr.GetStartWidth(), mrLineAttr.IsStartCentered());
/*?*/ 				mrPolyPoly3D.Insert(aArrowPoly);
/*N*/ 			}
/*N*/
/*N*/ 			if(mrLineAttr.IsEndActive())
/*N*/ 			{
/*?*/ 				// create line end polygon and move line end
/*?*/ 				Polygon3D aArrowPoly(XOutCreatePolygon(mrLineAttr.GetEndPolygon(), mrLineAttr.GetOutDev()));
/*?*/ 				fEnd = fPolyLength - ImpCreateLineStartEnd(
/*?*/ 					aArrowPoly, rSourcePoly, FALSE,
/*?*/ 					(double)mrLineAttr.GetEndWidth(), mrLineAttr.IsEndCentered());
/*?*/ 				mrPolyPoly3D.Insert(aArrowPoly);
/*N*/ 			}
/*N*/
/*N*/ 			if(fStart != 0.0 || fEnd != fPolyLength)
/*N*/ 			{
/*?*/ 				// build new poly, consume something from old poly
/*?*/ 				aPoly = Polygon3D(nCount);
/*?*/ 				UINT16 nInsPos(0);
/*?*/ 				double fPolyPos = 0.0;
/*?*/
/*?*/ 				for(UINT16 a=0;a<nCount;a++)
/*?*/ 				{
/*?*/ 					Vector3D aEdge = rSourcePoly[a+1] - rSourcePoly[a];
/*?*/ 					double fLength = aEdge.GetLength();
/*?*/
/*?*/ 					if(fStart != 0.0)
/*?*/ 					{
/*?*/ 						if(fStart - fLength > -SMALL_DVALUE)
/*?*/ 						{
/*?*/ 							fStart -= fLength;
/*?*/ 						}
/*?*/ 						else
/*?*/ 						{
/*?*/ 							Vector3D aNewPos;
/*?*/ 							aNewPos.CalcInBetween(rSourcePoly[a], rSourcePoly[a+1], fStart / fLength);
/*?*/ 							aPoly[nInsPos++] = aNewPos;
/*?*/ 							fStart = 0.0;
/*?*/ 						}
/*?*/ 					}
/*?*/ 					else
/*?*/ 					{
/*?*/ 						aPoly[nInsPos++] = rSourcePoly[a];
/*?*/ 					}
/*?*/
/*?*/ 					if((fPolyPos + fLength) - fEnd > -SMALL_DVALUE)
/*?*/ 					{
/*?*/ 						Vector3D aNewPos;
/*?*/ 						aNewPos.CalcInBetween(rSourcePoly[a], rSourcePoly[a+1], (fEnd - fPolyPos) / fLength);
/*?*/ 						aPoly[nInsPos++] = aNewPos;
/*?*/ 						a = nCount;
/*?*/ 					}
/*?*/
/*?*/ 					// next PolyPos
/*?*/ 					fPolyPos += fLength;
/*?*/ 				}
/*?*/
/*?*/ 				nCount = aPoly.GetPointCount() - 1;
/*N*/ 			}
/*N*/ 		}
/*N*/
/*N*/ 		if(nCount)
/*N*/ 		{
/*N*/ 			if(!mrLineAttr.GetDisplayLineWidth()
/*N*/ 				&& (mbLineDraft || mrLineAttr.GetLineStyle() == XLINE_SOLID))
/*N*/ 			{
/*N*/ 				// LineWidth zero, solid line -> add directly to linePoly
/*N*/ 				mrPolyLine3D.Insert(aPoly);
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				const Vector3D* pPrev = NULL;
/*N*/ 				const Vector3D* pLeft = NULL;
/*N*/ 				const Vector3D* pRight = NULL;
/*N*/ 				const Vector3D* pNext = NULL;
/*N*/ 				double fPolyPos = 0.0;
/*N*/
/*N*/ 				for(UINT16 a=0;a<nCount;a++)
/*N*/ 				{
/*N*/ 					BOOL bStart(!a);
/*N*/ 					BOOL bEnd(a+1 == nCount);
/*N*/
/*N*/ 					// get left, right positions
/*N*/ 					pLeft = &aPoly[a];
/*N*/
/*N*/ 					// get length
/*N*/ 					if(bClosed)
/*N*/ 					{
/*N*/ 						pRight = &aPoly[(a+1) % nCount];
/*N*/ 						pPrev = &aPoly[(a+nCount-1) % nCount];
/*N*/ 						pNext = &aPoly[(a+2) % nCount];
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*?*/ 						pRight = &aPoly[a+1];
/*?*/
/*?*/ 						if(bStart)
/*?*/ 						{
/*?*/ 							pPrev = NULL;
/*?*/ 						}
/*?*/ 						else
/*?*/ 						{
/*?*/ 							pPrev = &aPoly[a-1];
/*?*/ 						}
/*?*/
/*?*/ 						if(bEnd)
/*?*/ 						{
/*?*/ 							pNext = NULL;
/*?*/ 						}
/*?*/ 						else
/*?*/ 						{
/*?*/ 							pNext = &aPoly[a+2];
/*?*/ 						}
/*N*/ 					}

                    // positions are in pPrev, pLeft, pRight and pNext.
/*N*/ 					if(!mbLineDraft && mrLineAttr.GetLineStyle() == XLINE_DASH)
/*?*/ 						ImpCreateSegmentsForLine(pPrev, pLeft, pRight, pNext, fPolyPos);
/*N*/ 					else
/*?*/ 						ImpCreateLineSegment(pPrev, pLeft, pRight, pNext);
/*N*/
/*N*/ 					// increment PolyPos
/*N*/ 					Vector3D aEdge = *pRight - *pLeft;
/*N*/ 					fPolyPos += aEdge.GetLength();
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// class to remember broadcast start positions

/*N*/ SdrBroadcastItemChange::SdrBroadcastItemChange(const SdrObject& rObj)
/*N*/ {
/*N*/ 	if(rObj.ISA(SdrObjGroup))
/*N*/ 	{
/*N*/ 		SdrObjListIter aIter((const SdrObjGroup&)rObj, IM_DEEPNOGROUPS);
/*N*/ 		mpData = new List();
/*N*/
/*N*/ 		while(aIter.IsMore())
/*N*/ 		{
/*N*/ 			SdrObject* pObj = aIter.Next();
/*N*/ 			if(pObj)
/*N*/ 				((List*)mpData)->Insert(new Rectangle(pObj->GetBoundRect()), LIST_APPEND);
/*N*/ 		}
/*N*/
/*N*/ 		mnCount = ((List*)mpData)->Count();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		mnCount = 1;
/*N*/ 		mpData = new Rectangle(rObj.GetBoundRect());
/*N*/ 	}
/*N*/ }

/*N*/ SdrBroadcastItemChange::~SdrBroadcastItemChange()
/*N*/ {
/*N*/ 	if(mnCount > 1)
/*N*/ 	{
/*N*/ 		for(sal_uInt32 a(0); a < mnCount;a++)
/*N*/ 			delete ((Rectangle*)((List*)mpData)->GetObject(a));
/*N*/ 		delete ((List*)mpData);
/*N*/ 	}
/*N*/ 	else
/*N*/ 		delete ((Rectangle*)mpData);
/*N*/ }

/*N*/ const Rectangle& SdrBroadcastItemChange::GetRectangle(sal_uInt32 nIndex) const
/*N*/ {
/*N*/ 	if(mnCount > 1)
/*N*/ 		return *((Rectangle*)((List*)mpData)->GetObject(nIndex));
/*N*/ 	else
/*N*/ 		return *((Rectangle*)mpData);
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   @@@@  @@@@@  @@@@@@ @@@@@  @@@@  @@@@@@
//  @@  @@ @@  @@     @@ @@    @@  @@   @@
//  @@  @@ @@  @@     @@ @@    @@       @@
//  @@  @@ @@@@@      @@ @@@@  @@       @@
//  @@  @@ @@  @@     @@ @@    @@       @@
//  @@  @@ @@  @@ @@  @@ @@    @@  @@   @@
//   @@@@  @@@@@   @@@@  @@@@@  @@@@    @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ DBG_NAME(SdrObject)
/*N*/ TYPEINIT1(SdrObject,SfxListener);

/*N*/ SdrObject::SdrObject():
/*N*/ 	pObjList(NULL),
/*N*/ 	pPage(NULL),
/*N*/ 	pModel(NULL),
/*N*/ 	pUserCall(NULL),
/*N*/ 	pPlusData(NULL),
/*N*/ 	nOrdNum(0),
/*N*/ 	nLayerId(0)
/*N*/ {
/*N*/ 	DBG_CTOR(SdrObject,NULL);
/*N*/ 	bVirtObj         =FALSE;
/*N*/ 	bBoundRectDirty  =TRUE;
/*N*/ 	bSnapRectDirty   =TRUE;
/*N*/ 	bNetLock         =FALSE;
/*N*/ 	bInserted        =FALSE;
/*N*/ 	bGrouped         =FALSE;
/*N*/ 	bMovProt         =FALSE;
/*N*/ 	bSizProt         =FALSE;
/*N*/ 	bNoPrint         =FALSE;
/*N*/ 	bEmptyPresObj    =FALSE;
/*N*/ 	bNotPersistent   =FALSE;
/*N*/ 	bNeedColorRestore=FALSE;
/*N*/ 	bNotVisibleAsMaster=FALSE;
/*N*/ 	bClosedObj       =FALSE;
/*N*/ 	bWriterFlyFrame  =FALSE;
/*N*/ 	bNotMasterCachable=FALSE;
/*N*/ 	bIsEdge=FALSE;
/*N*/ 	bIs3DObj=FALSE;
/*N*/ 	bMarkProt=FALSE;
/*N*/ 	bIsUnoObj=FALSE;
/*N*/ }

/*N*/ SdrObject::~SdrObject()
/*N*/ {
/*N*/ 	uno::Reference< lang::XComponent > xShape( mxUnoShape, uno::UNO_QUERY );
/*N*/ 	if( xShape.is() )
/*N*/ 		xShape->dispose();
/*N*/
/*N*/ 	DBG_DTOR(SdrObject,NULL);
/*N*/ 	SendUserCall(SDRUSERCALL_DELETE,GetBoundRect());
/*N*/ 	if (pPlusData!=NULL) delete pPlusData;
/*N*/ }

/*N*/ SdrObjPlusData* SdrObject::NewPlusData() const
/*N*/ {
/*N*/ 	return new SdrObjPlusData;
/*N*/ }

/*N*/ void SdrObject::SetRectsDirty(FASTBOOL bNotMyself)
/*N*/ {
/*N*/ 	if (!bNotMyself) {
/*N*/ 		bBoundRectDirty=TRUE;
/*N*/ 		bSnapRectDirty=TRUE;
/*N*/ 	}
/*N*/ 	if (pObjList!=NULL) {
/*N*/ 		pObjList->SetRectsDirty();
/*N*/ 	}
/*N*/ }

/*N*/ void SdrObject::SetModel(SdrModel* pNewModel)
/*N*/ {
/*N*/ 	if(pNewModel && pPage)
/*N*/ 	{
/*N*/ 		if(pPage->GetModel() != pNewModel)
/*N*/ 		{
/*N*/ 			pPage = NULL;
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	pModel = pNewModel;
/*N*/ }

/*N*/ void SdrObject::SetObjList(SdrObjList* pNewObjList)
/*N*/ {
/*N*/ 	pObjList=pNewObjList;
/*N*/ }

/*N*/ void SdrObject::SetPage(SdrPage* pNewPage)
/*N*/ {
/*N*/ 	pPage=pNewPage;
/*N*/ 	if (pPage!=NULL) {
/*N*/ 		SdrModel* pMod=pPage->GetModel();
/*N*/ 		if (pMod!=pModel && pMod!=NULL) {
/*N*/ 			SetModel(pMod);
/*N*/ 		}
/*N*/ 	}
/*N*/ }

// init global static itempool
/*N*/ SdrItemPool* SdrObject::mpGlobalItemPool = NULL;
/*N*/
/*N*/ SdrItemPool* SdrObject::GetGlobalDrawObjectItemPool()
/*N*/ {
/*N*/ 	if(!mpGlobalItemPool)
/*N*/ 	{
/*N*/ 		mpGlobalItemPool = new SdrItemPool(SDRATTR_START, SDRATTR_END);
/*N*/ 		SfxItemPool* pGlobalOutlPool = EditEngine::CreatePool();
/*N*/ 		mpGlobalItemPool->SetSecondaryPool(pGlobalOutlPool);
/*N*/ 		mpGlobalItemPool->SetDefaultMetric((SfxMapUnit)SdrEngineDefaults::GetMapUnit());
/*N*/ 		mpGlobalItemPool->FreezeIdRanges();
/*N*/ 	}
/*N*/
/*N*/ 	return mpGlobalItemPool;
/*N*/ }


/*N*/ SdrItemPool* SdrObject::GetItemPool() const
/*N*/ {
/*N*/ 	if(pModel)
/*N*/ 		return (SdrItemPool*)(&pModel->GetItemPool());
/*N*/
/*N*/ 	// use a static global default pool
/*N*/ 	return SdrObject::GetGlobalDrawObjectItemPool();
/*N*/ }

/*N*/ UINT32 SdrObject::GetObjInventor()   const
/*N*/ {
/*N*/ 	return SdrInventor;
/*N*/ }

/*N*/ UINT16 SdrObject::GetObjIdentifier() const
/*N*/ {
/*N*/ 	return UINT16(OBJ_NONE);
/*N*/ }


/*N*/ SdrLayerID SdrObject::GetLayer() const
/*N*/ {
/*N*/ 	return SdrLayerID(nLayerId);
/*N*/ }

/*N*/ void SdrObject::GetLayer(SetOfByte& rSet) const
/*N*/ {
/*N*/ 	rSet.Set((BYTE)nLayerId);
/*N*/ 	SdrObjList* pOL=GetSubList();
/*N*/ 	if (pOL!=NULL) {
/*N*/ 		ULONG nObjAnz=pOL->GetObjCount();
/*N*/ 		for (ULONG nObjNum=0; nObjNum<nObjAnz; nObjNum++) {
/*N*/ 			pOL->GetObj(nObjNum)->GetLayer(rSet);
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SdrObject::NbcSetLayer(SdrLayerID nLayer)
/*N*/ {
/*N*/ 	nLayerId=nLayer;
/*N*/ }

/*N*/ void SdrObject::SetLayer(SdrLayerID nLayer)
/*N*/ {
/*N*/ 	NbcSetLayer(nLayer);
/*N*/ 	SetChanged();
/*N*/ 	SendRepaintBroadcast();
/*N*/ }

/*N*/ void SdrObject::AddListener(SfxListener& rListener)
/*N*/ {
/*N*/ 	ImpForcePlusData();
/*N*/ 	if (pPlusData->pBroadcast==NULL) pPlusData->pBroadcast=new SfxBroadcaster;
/*N*/ 	rListener.StartListening(*pPlusData->pBroadcast);
/*N*/ }

/*N*/ void SdrObject::RemoveListener(SfxListener& rListener)
/*N*/ {
/*N*/ 	if (pPlusData!=NULL && pPlusData->pBroadcast!=NULL) {
/*N*/ 		rListener.EndListening(*pPlusData->pBroadcast);
/*N*/ 		if (!pPlusData->pBroadcast->HasListeners()) {
/*N*/ 			delete pPlusData->pBroadcast;
/*N*/ 			pPlusData->pBroadcast=NULL;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SdrObject::AddReference(SdrVirtObj& rVrtObj)
/*N*/ {
/*N*/ 	AddListener(rVrtObj);
/*N*/ }

/*N*/ void SdrObject::DelReference(SdrVirtObj& rVrtObj)
/*N*/ {
/*N*/ 	RemoveListener(rVrtObj);
/*N*/ }






/*N*/ SdrObjList* SdrObject::GetSubList() const
/*N*/ {
/*N*/ 	return NULL;
/*N*/ }

/*N*/ SdrObject* SdrObject::GetUpGroup() const
/*N*/ {
/*N*/ 	return pObjList!=NULL ? pObjList->GetOwnerObj() : NULL;
/*N*/ }


/*N*/ void SdrObject::SetName(const XubString& rStr)
/*NBFF*/ {
/*NBFF*/ 	if(rStr.Len())
/*NBFF*/ 	{
/*NBFF*/ 		ImpForcePlusData();
/*NBFF*/ 		pPlusData->aObjName = rStr;
/*NBFF*/ 	}
/*NBFF*/ 	else
/*NBFF*/ 	{
/*NBFF*/ 		if(pPlusData)
/*NBFF*/ 		{
/*NBFF*/ 			pPlusData->aObjName = rStr;
/*NBFF*/ 		}
/*NBFF*/ 	}
/*N*/ }

/*N*/ XubString SdrObject::GetName() const
/*N*/ {
/*N*/ 	SdrObjPlusData* pPlus=pPlusData;
/*N*/ 	if (pPlus!=NULL) {
/*N*/ 		return pPlus->aObjName;
/*N*/ 	}
/*N*/ 	return String();
/*N*/ }

// support for HTMLName

// support for HTMLName

/*N*/ UINT32 SdrObject::GetOrdNum() const
/*N*/ {
/*N*/ 	if (pObjList!=NULL) {
/*N*/ 		if (pObjList->IsObjOrdNumsDirty()) {
/*N*/ 			pObjList->RecalcObjOrdNums();
/*N*/ 		}
/*N*/ 	} else ((SdrObject*)this)->nOrdNum=0;
/*N*/ 	return nOrdNum;
/*N*/ }

/*N*/ const Rectangle& SdrObject::GetBoundRect() const
/*N*/ {
/*N*/ 	if (bBoundRectDirty) {
/*N*/ 		((SdrObject*)this)->RecalcBoundRect();
/*N*/ 		((SdrObject*)this)->bBoundRectDirty=FALSE;
/*N*/ 	}
/*N*/ 	return aOutRect;
/*N*/ }

/*N*/ void SdrObject::RecalcBoundRect()
/*N*/ {
/*N*/ }

/*N*/ void SdrObject::SendRepaintBroadcast(const Rectangle& rRect) const
/*N*/ {
/*N*/ 	if( pModel && pModel->isLocked() )
/*N*/ 		return;
/*N*/
/*N*/ 	BOOL bBrd(pPlusData && pPlusData->pBroadcast);
/*N*/ 	BOOL bPnt(bInserted && pModel);
/*N*/
/*N*/ 	if(bPnt || bBrd)
/*N*/ 	{
/*N*/ 		SdrHint aHint(*this, rRect);
/*N*/
/*N*/ 		if(bBrd)
/*?*/ 			pPlusData->pBroadcast->Broadcast(aHint);
/*N*/
/*N*/ 		if(bPnt)
/*N*/ 			pModel->Broadcast(aHint);
/*N*/
/*N*/ 		// alle Animationen wegen Obj-Aenderung neustarten
/*N*/ 		RestartAnimation(NULL);
/*N*/ 	}
/*N*/ }

/*N*/ void SdrObject::SendRepaintBroadcast(BOOL bNoPaintNeeded) const
/*N*/ {
/*N*/ 	if( pModel && pModel->isLocked() )
/*N*/ 		return;
/*N*/
/*N*/ 	BOOL bBrd(pPlusData && pPlusData->pBroadcast);
/*N*/ 	BOOL bPnt(bInserted && pModel);
/*N*/
/*N*/ 	if(bPnt || bBrd)
/*N*/ 	{
/*N*/ 		SdrHint aHint(*this);
/*N*/ 		aHint.SetNeedRepaint(!bNoPaintNeeded);
/*N*/
/*N*/ 		if(bBrd)
/*N*/ 			pPlusData->pBroadcast->Broadcast(aHint);
/*N*/
/*N*/ 		if(bPnt)
/*N*/ 			pModel->Broadcast(aHint);
/*N*/
/*N*/ 		// alle Animationen wegen Obj-Aenderung neustarten
/*N*/ 		RestartAnimation(NULL);
/*N*/ 	}
/*N*/ }

/*N*/ void SdrObject::SetChanged()
/*N*/ {
/*N*/ 	if (bInserted && pModel!=NULL) pModel->SetChanged();
/*N*/ }

/*N*/ FASTBOOL SdrObject::Paint(ExtOutputDevice& rXOut, const SdrPaintInfoRec& /*rInfoRec*/) const
/*N*/ {
/*N*/ 	Color aRedColor( COL_RED );
/*N*/ 	Color aYellowColor( COL_YELLOW );
/*N*/ 	rXOut.OverrideLineColor( aRedColor );
/*N*/ 	rXOut.OverrideFillColor( aYellowColor );
/*N*/ 	rXOut.DrawRect(GetBoundRect());
/*N*/
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ ::std::auto_ptr< SdrLineGeometry >  SdrObject::CreateLinePoly( OutputDevice& 		rOut,
/*N*/                                                                BOOL 				bForceOnePixel,
/*N*/                                                                BOOL 				bForceTwoPixel,
/*N*/                                                                BOOL 				bIsLineDraft	) const
/*N*/ {
/*N*/     PolyPolygon3D aPolyPoly3D;
/*N*/     PolyPolygon3D aLinePoly3D;
/*N*/
/*N*/ 	// get XOR Poly as base
/*N*/ 	XPolyPolygon aTmpPolyPolygon;
/*N*/ 	TakeXorPoly(aTmpPolyPolygon, TRUE);
/*N*/
/*N*/ 	// get ImpLineStyleParameterPack
/*N*/ 	ImpLineStyleParameterPack aLineAttr(GetItemSet(), bForceOnePixel || bForceTwoPixel || bIsLineDraft, &rOut);
/*N*/ 	ImpLineGeometryCreator aLineCreator(aLineAttr, aPolyPoly3D, aLinePoly3D, bIsLineDraft);
/*N*/
/*N*/ 	// compute single lines
/*N*/ 	for(UINT16 a=0;a<aTmpPolyPolygon.Count();a++)
/*N*/ 	{
/*N*/ 		// expand splines into polygons and convert to double
/*N*/ 		Polygon3D aPoly3D(XOutCreatePolygon(aTmpPolyPolygon[a], &rOut));
/*N*/ 		aPoly3D.RemoveDoublePoints();
/*N*/
/*N*/ 		// convert line to single Polygons; make sure the part
/*N*/ 		// polygons are all clockwise oriented
/*N*/ 		aLineCreator.AddPolygon3D(aPoly3D);
/*N*/ 	}
/*N*/
/*N*/     if(aPolyPoly3D.Count() || aLinePoly3D.Count())
/*N*/         return ::std::auto_ptr< SdrLineGeometry > (new SdrLineGeometry(aPolyPoly3D, aLinePoly3D,
/*N*/                                                                        aLineAttr, bForceOnePixel, bForceTwoPixel));
/*N*/     else
/*?*/ 		return ::std::auto_ptr< SdrLineGeometry > (NULL);
/*N*/ }

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//#define TEST_SKELETON
#ifdef TEST_SKELETON

static OutputDevice* pImpTestOut = 0L;

class ImpSkeleton;


///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////


#endif
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/*N*/ ::std::auto_ptr< SdrLineGeometry > SdrObject::ImpPrepareLineGeometry( ExtOutputDevice& rXOut, const SfxItemSet& rSet,
/*N*/                                                                       BOOL bIsLineDraft) const
/*N*/ {
/*N*/ 	XLineStyle eXLS = (XLineStyle)((const XLineStyleItem&)rSet.Get(XATTR_LINESTYLE)).GetValue();
/*N*/ 	if(eXLS != XLINE_NONE)
/*N*/ 	{
/*N*/ 		// need to force single point line?
/*N*/ 		INT32 nLineWidth = ((const XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue();
/*N*/ 		Size aSize(nLineWidth, nLineWidth);
/*N*/ 		aSize = rXOut.GetOutDev()->LogicToPixel(aSize);
/*N*/ 		BOOL bForceOnePixel(aSize.Width() <= 1 || aSize.Height() <= 1);
/*N*/ 		BOOL bForceTwoPixel(!bForceOnePixel && (aSize.Width() <= 2 || aSize.Height() <= 2));
/*N*/
/*N*/ 		// no force to two pixel when connected to MetaFile, so that not
/*N*/ 		// four lines instead of one is recorded (e.g.)
/*N*/ 		if(bForceTwoPixel && rXOut.GetOutDev()->GetConnectMetaFile())
/*N*/ 			bForceTwoPixel = FALSE;
/*N*/
/*N*/ 		// #78210# switch off bForceTwoPixel when line draft mode
/*N*/ 		if(bForceTwoPixel && bIsLineDraft)
/*N*/ 		{
/*N*/ 			bForceTwoPixel = FALSE;
/*N*/ 			bForceOnePixel = TRUE;
/*N*/ 		}
/*N*/
/*N*/ 		// create line geometry
/*N*/ 		return CreateLinePoly(*rXOut.GetOutDev(),
/*N*/                               bForceOnePixel, bForceTwoPixel, bIsLineDraft);
/*N*/ 	}
/*N*/
/*N*/ 	return ::std::auto_ptr< SdrLineGeometry > (0L);
/*N*/ }


/*N*/ void SdrObject::ImpDrawColorLineGeometry(
/*N*/ 	ExtOutputDevice& rXOut, const SfxItemSet& rSet, SdrLineGeometry& rLineGeometry) const
/*N*/ {
/*N*/ 	Color aColor = ((XLineColorItem&)rSet.Get(XATTR_LINECOLOR)).GetValue();
/*N*/ 	sal_uInt16 nTrans = ((const XLineTransparenceItem&)(rSet.Get(XATTR_LINETRANSPARENCE))).GetValue();
/*N*/
/*N*/ 	// draw the line geometry
/*N*/ 	ImpDrawLineGeometry(rXOut, aColor, nTrans, rLineGeometry);
/*N*/ }

/*N*/ void SdrObject::ImpDrawLineGeometry(   ExtOutputDevice& 	rXOut,
/*N*/                                        Color&              rColor,
/*N*/                                        sal_uInt16        	nTransparence,
/*N*/                                        SdrLineGeometry&    rLineGeometry,
/*N*/                                        sal_Int32          	nDX,
/*N*/                                        sal_Int32          	nDY				) const
/*N*/ {
/*N*/     Color aLineColor( rColor );
/*N*/
/*N*/ 	// #72796# black/white option active?
/*N*/ 	const UINT32 nOldDrawMode(rXOut.GetOutDev()->GetDrawMode());
/*N*/
/*N*/ 	// #72796# if yes, force to DRAWMODE_BLACKFILL for these are LINES to be drawn as polygons
/*N*/ 	if( ( nOldDrawMode & DRAWMODE_WHITEFILL ) && ( nOldDrawMode & DRAWMODE_BLACKLINE ) )
/*N*/     {
/*?*/         aLineColor = Color( COL_BLACK );
/*?*/         rXOut.GetOutDev()->SetDrawMode( nOldDrawMode & (~DRAWMODE_WHITEFILL) );
/*N*/     }
/*N*/     else if( ( nOldDrawMode & DRAWMODE_SETTINGSFILL ) && ( nOldDrawMode & DRAWMODE_SETTINGSLINE ) )
/*N*/     {
/*?*/ 		ColorConfig aColorConfig;
/*?*/         aLineColor = Color( aColorConfig.GetColorValue( FONTCOLOR ).nColor );
/*?*/         rXOut.GetOutDev()->SetDrawMode( nOldDrawMode & (~DRAWMODE_SETTINGSFILL) );
/*N*/     }
/*N*/
/*N*/     // #103692# Hold local copy of geometry
/*N*/     PolyPolygon3D aPolyPoly = rLineGeometry.GetPolyPoly3D();
/*N*/     PolyPolygon3D aLinePoly = rLineGeometry.GetLinePoly3D();
/*N*/
/*N*/     // #103692# Offset geometry (extracted from SdrObject::ImpDrawShadowLineGeometry)
/*N*/ 	if( nDX || nDY )
/*N*/ 	{
/*?*/ 		// transformation necessary
/*?*/ 		Matrix4D aTrans;
/*?*/
/*?*/ 		aTrans.Translate((double)nDX, -(double)nDY, 0.0);
/*?*/ 		aPolyPoly.Transform(aTrans);
/*?*/ 		aLinePoly.Transform(aTrans);
/*N*/ 	}
/*N*/
/*N*/     // #100127# Bracket output with a comment, if recording a Mtf
/*N*/     GDIMetaFile* pMtf=NULL;
/*N*/     bool bMtfCommentWritten( false );
/*N*/     if( (pMtf=rXOut.GetOutDev()->GetConnectMetaFile()) )
/*N*/     {
/*N*/         XPolyPolygon aPolyPoly;
/*N*/         TakeXorPoly(aPolyPoly, TRUE);
/*N*/
/*N*/         // #103692# Offset original geometry, too
/*N*/         if( nDX || nDY )
/*N*/         {
/*?*/             // transformation necessary
/*?*/             aPolyPoly.Move( nDX, nDY );
/*N*/         }
/*N*/
/*N*/         // for geometries with more than one polygon, dashing, arrows
/*N*/         // etc. become ambiguous (e.g. measure objects have no arrows
/*N*/         // on the end line), thus refrain from writing the comment
/*N*/         // here.
/*N*/         if( aPolyPoly.Count() == 1 )
/*N*/         {
/*N*/             // add completely superfluous color action (gets overwritten
/*N*/             // below), to store our line color reliably
/*N*/             rXOut.GetOutDev()->SetLineColor(aLineColor);
/*N*/
/*N*/             const ImpLineStyleParameterPack& rLineParameters = rLineGeometry.GetLineAttr();
/*N*/
/*N*/             XPolygon aStartPoly( rLineParameters.GetStartPolygon() );
/*N*/             XPolygon aEndPoly( rLineParameters.GetEndPolygon() );
/*N*/
/*N*/             // scale arrows to specified stroke width
/*N*/             if( aStartPoly.GetPointCount() )
/*?*/             {
/*?*/                 Rectangle aBounds( aStartPoly.GetBoundRect() );
/*?*/
/*?*/                 // mirror and translate to origin
/*?*/                 aStartPoly.Scale(-1,-1);
/*?*/                 aStartPoly.Translate( Point(aBounds.GetWidth() / 2, aBounds.GetHeight()) );
/*?*/
/*?*/                 if( aBounds.GetWidth() )
/*?*/                 {
/*?*/ 					// #104527# Avoid division by zero. If rLineParameters.GetLineWidth
/*?*/ 					// is zero this is a hairline which can be handled as 1.0.
/*?*/ 					double fLineWidth(rLineParameters.GetLineWidth() ? (double)rLineParameters.GetLineWidth() : 1.0);
/*?*/
/*?*/                     double fScale( (double)rLineParameters.GetStartWidth() / fLineWidth *
/*?*/                                    (double)SvtGraphicStroke::normalizedArrowWidth / (double)aBounds.GetWidth() );
/*?*/                     aStartPoly.Scale( fScale, fScale );
/*?*/                 }
/*?*/
/*?*/                 if( rLineParameters.IsStartCentered() )
/*?*/                     aStartPoly.Translate( Point(0, -aStartPoly.GetBoundRect().GetHeight() / 2) );
/*?*/             }
/*N*/             if( aEndPoly.GetPointCount() )
/*?*/             {
/*?*/                 Rectangle aBounds( aEndPoly.GetBoundRect() );
/*?*/
/*?*/                 // mirror and translate to origin
/*?*/                 aEndPoly.Scale(-1,-1);
/*?*/                 aEndPoly.Translate( Point(aBounds.GetWidth() / 2, aBounds.GetHeight()) );
/*?*/
/*?*/                 if( aBounds.GetWidth() )
/*?*/                 {
/*?*/ 					// #104527# Avoid division by zero. If rLineParameters.GetLineWidth
/*?*/ 					// is zero this is a hairline which we can be handled as 1.0.
/*?*/ 					double fLineWidth(rLineParameters.GetLineWidth() ? (double)rLineParameters.GetLineWidth() : 1.0);
/*?*/
/*?*/                     double fScale( (double)rLineParameters.GetEndWidth() / fLineWidth *
/*?*/                                    (double)SvtGraphicStroke::normalizedArrowWidth / (double)aBounds.GetWidth() );
/*?*/                     aEndPoly.Scale( fScale, fScale );
/*?*/                 }
/*?*/
/*?*/                 if( rLineParameters.IsEndCentered() )
/*?*/                     aEndPoly.Translate( Point(0, -aEndPoly.GetBoundRect().GetHeight() / 2) );
/*?*/             }
/*N*/
/*N*/             SvtGraphicStroke aStroke( XOutCreatePolygonBezier( aPolyPoly[0], rXOut.GetOutDev() ),
/*N*/                                       XOutCreatePolygonBezier( aStartPoly, rXOut.GetOutDev() ),
/*N*/                                       XOutCreatePolygonBezier( aEndPoly, rXOut.GetOutDev() ),
/*N*/                                       nTransparence / 100.0,
/*N*/                                       rLineParameters.GetLineWidth(),
/*N*/                                       SvtGraphicStroke::capButt,
/*N*/                                       SvtGraphicStroke::joinRound,
/*N*/                                       rLineParameters.GetLinejointMiterUpperBound(),
/*N*/                                       rLineParameters.GetLineStyle() == XLINE_DASH ? rLineParameters.GetDotDash() : SvtGraphicStroke::DashArray() );
/*N*/
/*N*/ #ifdef DBG_UTIL
/*N*/             ::rtl::OString aStr( aStroke.toString() );
/*N*/ #endif
/*N*/
/*N*/             SvMemoryStream	aMemStm;
/*N*/
/*N*/             aMemStm << aStroke;
/*N*/
/*N*/             pMtf->AddAction( new MetaCommentAction( "XPATHSTROKE_SEQ_BEGIN", 0,
/*N*/                                                     static_cast<const BYTE*>(aMemStm.GetData()),
/*N*/                                                     aMemStm.Seek( STREAM_SEEK_TO_END ) ) );
/*N*/             bMtfCommentWritten = true;
/*N*/         }
/*N*/     }
/*N*/
/*N*/ 	if(nTransparence)
/*?*/ 	{
/*?*/ 		if(nTransparence != 100)
/*?*/ 		{
/*?*/ 			// to be shown line has transparence, output via MetaFile
/*?*/ 			UINT8 nScaledTrans((UINT8)((nTransparence * 255)/100));
/*?*/ 			Color aTransColor(nScaledTrans, nScaledTrans, nScaledTrans);
/*?*/ 			Gradient aGradient(GRADIENT_LINEAR, aTransColor, aTransColor);
/*?*/ 			GDIMetaFile aMetaFile;
/*?*/ 			VirtualDevice aVDev;
/*?*/ 			Volume3D aVolume;
/*?*/ 			MapMode aMap(rXOut.GetOutDev()->GetMapMode());
/*?*/
/*?*/ 			// StepCount to someting small
/*?*/ 			aGradient.SetSteps(3);
/*?*/
/*?*/ 			// get bounds of geometry
/*?*/ 			if(aPolyPoly.Count())
/*?*/ 				aVolume.Union(aPolyPoly.GetPolySize());
/*?*/ 			if(aLinePoly.Count())
/*?*/ 				aVolume.Union(aLinePoly.GetPolySize());
/*?*/
/*?*/ 			// get pixel size in logic coor for 1,2 pixel cases
/*?*/ 			Size aSizeSinglePixel(1, 1);
/*?*/
/*?*/ 			if(rLineGeometry.DoForceOnePixel() || rLineGeometry.DoForceTwoPixel())
/*?*/ 				aSizeSinglePixel = rXOut.GetOutDev()->PixelToLogic(aSizeSinglePixel);
/*?*/
/*?*/ 			// create BoundRectangle
/*?*/ 			Rectangle aBound(
/*?*/ 				(INT32)aVolume.MinVec().X(),
/*?*/ 				(INT32)-aVolume.MaxVec().Y(),
/*?*/ 				(INT32)aVolume.MaxVec().X(),
/*?*/ 				(INT32)-aVolume.MinVec().Y());
/*?*/
/*?*/ 			if(rLineGeometry.DoForceOnePixel() || rLineGeometry.DoForceTwoPixel())
/*?*/ 			{
/*?*/ 				// enlarge aBound
/*?*/ 				if(rLineGeometry.DoForceTwoPixel())
/*?*/ 				{
/*?*/ 					aBound.Right() += 2 * (aSizeSinglePixel.Width() - 1);
/*?*/ 					aBound.Bottom() += 2 * (aSizeSinglePixel.Height() - 1);
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					aBound.Right() += (aSizeSinglePixel.Width() - 1);
/*?*/ 					aBound.Bottom() += (aSizeSinglePixel.Height() - 1);
/*?*/ 				}
/*?*/ 			}
/*?*/
/*?*/ 			// prepare VDev and MetaFile
/*?*/ 			aVDev.EnableOutput(FALSE);
/*?*/ 			aVDev.SetMapMode(rXOut.GetOutDev()->GetMapMode());
/*?*/ 			aMetaFile.Record(&aVDev);
/*?*/ 			aVDev.SetLineColor(aLineColor);
/*?*/ 			aVDev.SetFillColor(aLineColor);
/*?*/ 			aVDev.SetFont(rXOut.GetOutDev()->GetFont());
/*?*/ 			aVDev.SetDrawMode(rXOut.GetOutDev()->GetDrawMode());
/*?*/ 			aVDev.SetRefPoint(rXOut.GetOutDev()->GetRefPoint());
/*?*/
/*?*/ 			// create output
/*?*/ 			if(aPolyPoly.Count())
/*?*/ 			{
/*?*/ 				PolyPolygon aVCLPolyPoly = aPolyPoly.GetPolyPolygon();
/*?*/
/*?*/ 				for(UINT16 a=0;a<aVCLPolyPoly.Count();a++)
/*?*/ 					aMetaFile.AddAction(new MetaPolygonAction(aVCLPolyPoly[a]));
/*?*/ 			}
/*?*/
/*?*/ 			if(aLinePoly.Count())
/*?*/ 			{
/*?*/ 				PolyPolygon aVCLLinePoly = aLinePoly.GetPolyPolygon();
/*?*/
/*?*/ 				if(rLineGeometry.DoForceTwoPixel())
/*?*/ 				{
/*?*/ 					UINT16 a;
/*?*/
/*?*/ 					for(a=0;a<aVCLLinePoly.Count();a++)
/*?*/ 						aMetaFile.AddAction(new MetaPolyLineAction(aVCLLinePoly[a]));
/*?*/
/*?*/ 					aVCLLinePoly.Move(aSizeSinglePixel.Width() - 1, 0);
/*?*/
/*?*/ 					for(a=0;a<aVCLLinePoly.Count();a++)
/*?*/ 						aMetaFile.AddAction(new MetaPolyLineAction(aVCLLinePoly[a]));
/*?*/
/*?*/ 					aVCLLinePoly.Move(0, aSizeSinglePixel.Height() - 1);
/*?*/
/*?*/ 					for(a=0;a<aVCLLinePoly.Count();a++)
/*?*/ 						aMetaFile.AddAction(new MetaPolyLineAction(aVCLLinePoly[a]));
/*?*/
/*?*/ 					aVCLLinePoly.Move(-aSizeSinglePixel.Width() - 1, 0);
/*?*/
/*?*/ 					for(a=0;a<aVCLLinePoly.Count();a++)
/*?*/ 						aMetaFile.AddAction(new MetaPolyLineAction(aVCLLinePoly[a]));
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					for(UINT16 a=0;a<aVCLLinePoly.Count();a++)
/*?*/ 						aMetaFile.AddAction(new MetaPolyLineAction(aVCLLinePoly[a]));
/*?*/ 				}
/*?*/ 			}
/*?*/
/*?*/ 			// draw metafile
/*?*/ 			aMetaFile.Stop();
/*?*/ 			aMetaFile.WindStart();
/*?*/ 			aMap.SetOrigin(aBound.TopLeft());
/*?*/ 			aMetaFile.SetPrefMapMode(aMap);
/*?*/ 			aMetaFile.SetPrefSize(aBound.GetSize());
/*?*/ 			rXOut.GetOutDev()->DrawTransparent(aMetaFile, aBound.TopLeft(), aBound.GetSize(), aGradient);
/*?*/ 		}
/*?*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// no transparence, simple output
/*N*/ 		if(aPolyPoly.Count())
/*N*/ 		{
/*N*/ 			PolyPolygon aVCLPolyPoly = aPolyPoly.GetPolyPolygon();
/*N*/
/*N*/ 			rXOut.GetOutDev()->SetLineColor();
/*N*/ 			rXOut.GetOutDev()->SetFillColor(aLineColor);
/*N*/
/*N*/ 			for(UINT16 a=0;a<aVCLPolyPoly.Count();a++)
/*N*/ 				rXOut.GetOutDev()->DrawPolygon(aVCLPolyPoly[a]);
/*N*/ 		}
/*N*/
/*N*/ 		if(aLinePoly.Count())
/*N*/ 		{
/*N*/ 			PolyPolygon aVCLLinePoly = aLinePoly.GetPolyPolygon();
/*N*/
/*N*/ 			rXOut.GetOutDev()->SetLineColor(aLineColor);
/*N*/ 			rXOut.GetOutDev()->SetFillColor();
/*N*/
/*N*/ 			if(rLineGeometry.DoForceTwoPixel())
/*?*/ 			{
/*?*/ 				PolyPolygon aPolyPolyPixel( rXOut.GetOutDev()->LogicToPixel(aVCLLinePoly) );
/*?*/ 				BOOL bWasEnabled = rXOut.GetOutDev()->IsMapModeEnabled();
/*?*/ 				rXOut.GetOutDev()->EnableMapMode(FALSE);
/*?*/ 				UINT16 a;
/*?*/
/*?*/ 				for(a=0;a<aVCLLinePoly.Count();a++)
/*?*/ 					rXOut.GetOutDev()->DrawPolyLine(aPolyPolyPixel[a]);
/*?*/
/*?*/ 				aPolyPolyPixel.Move(1,0);
/*?*/
/*?*/ 				for(a=0;a<aVCLLinePoly.Count();a++)
/*?*/ 					rXOut.GetOutDev()->DrawPolyLine(aPolyPolyPixel[a]);
/*?*/
/*?*/ 				aPolyPolyPixel.Move(0,1);
/*?*/
/*?*/ 				for(a=0;a<aVCLLinePoly.Count();a++)
/*?*/ 					rXOut.GetOutDev()->DrawPolyLine(aPolyPolyPixel[a]);
/*?*/
/*?*/ 				aPolyPolyPixel.Move(-1,0);
/*?*/
/*?*/ 				for(a=0;a<aVCLLinePoly.Count();a++)
/*?*/ 					rXOut.GetOutDev()->DrawPolyLine(aPolyPolyPixel[a]);
/*?*/
/*?*/ 				rXOut.GetOutDev()->EnableMapMode(bWasEnabled);
/*?*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				for( UINT16 a = 0; a < aVCLLinePoly.Count(); a++ )
/*N*/                 {
/*N*/                     const Polygon&  rPoly = aVCLLinePoly[ a ];
/*N*/                     BOOL            bDrawn = FALSE;
/*N*/
/*N*/                     if( rPoly.GetSize() == 2 )
/*N*/                     {
/*N*/ 						if ( !rXOut.GetOutDev()->GetConnectMetaFile() )
/*N*/ 						{
/*?*/ 							const Line  aLine( rXOut.GetOutDev()->LogicToPixel( rPoly[ 0 ] ),
/*?*/ 											   rXOut.GetOutDev()->LogicToPixel( rPoly[ 1 ] ) );
/*?*/
/*?*/ 							if( aLine.GetLength() > 16000 )
/*?*/ 							{
/*?*/ 								Point       aPoint;
/*?*/ 								Rectangle   aOutRect( aPoint, rXOut.GetOutDev()->GetOutputSizePixel() );
/*?*/ 								Line        aIntersection;
/*?*/
/*?*/ 								if( aLine.Intersection( aOutRect, aIntersection ) )
/*?*/ 								{
/*?*/ 									rXOut.GetOutDev()->DrawLine( rXOut.GetOutDev()->PixelToLogic( aIntersection.GetStart() ),
/*?*/ 																 rXOut.GetOutDev()->PixelToLogic( aIntersection.GetEnd() ) );
/*?*/ 								}
/*?*/ 								bDrawn = TRUE;
/*?*/ 							}
/*?*/ 						}
/*N*/                     }
/*N*/                     if( !bDrawn )
/*N*/                         rXOut.GetOutDev()->DrawPolyLine( rPoly );
/*N*/                 }
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/     // #100127# Bracket output with a comment, if recording a Mtf
/*N*/     if( bMtfCommentWritten && pMtf )
/*N*/         pMtf->AddAction( new MetaCommentAction( "XPATHSTROKE_SEQ_END" ) );
/*N*/
/*N*/     rXOut.GetOutDev()->SetDrawMode( nOldDrawMode );
/*N*/ }


///////////////////////////////////////////////////////////////////////////////


/*N*/ SdrObject* SdrObject::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
/*N*/ {DBG_BF_ASSERT(0, "STRIP");  return NULL;//STRIP001
/*N*/ }

/*N*/ SdrObject* SdrObject::Clone() const
/*N*/ {
/*N*/ 	SdrObject* pObj=SdrObjFactory::MakeNewObject(GetObjInventor(),GetObjIdentifier(),NULL);
/*N*/ 	if (pObj!=NULL) {
/*N*/ 		pObj->pModel=pModel;
/*N*/ 		pObj->pPage=pPage;
/*N*/ 		*pObj=*this;
/*N*/ 	}
/*N*/ 	return pObj;
/*N*/ }

/*N*/ SdrObject* SdrObject::Clone(SdrPage* pNewPage, SdrModel* pNewModel) const
/*N*/ {
/*N*/ 	SdrObject* pObj=SdrObjFactory::MakeNewObject(GetObjInventor(),GetObjIdentifier(),NULL);
/*N*/ 	if (pObj!=NULL) {
/*N*/ 		pObj->pModel=pNewModel;
/*N*/ 		pObj->pPage=pNewPage;
/*N*/ 		*pObj=*this;
/*N*/ 	}
/*N*/ 	return pObj;
/*N*/ }

/*N*/ void SdrObject::operator=(const SdrObject& rObj)
/*N*/ {
/*N*/ 	pModel  =rObj.pModel;
/*N*/ 	aOutRect=rObj.GetBoundRect();
/*N*/ 	nLayerId=rObj.GetLayer();
/*N*/ 	aAnchor =rObj.aAnchor;
/*N*/ 	bVirtObj=rObj.bVirtObj;
/*N*/ 	bSizProt=rObj.bSizProt;
/*N*/ 	bMovProt=rObj.bMovProt;
/*N*/ 	bNoPrint=rObj.bNoPrint;
/*N*/ 	bMarkProt=rObj.bMarkProt;
/*N*/ 	//EmptyPresObj wird nicht kopiert: nun doch! (25-07-1995, Joe)
/*N*/ 	bEmptyPresObj =rObj.bEmptyPresObj;
/*N*/ 	//NotVisibleAsMaster wird nicht kopiert: nun doch! (25-07-1995, Joe)
/*N*/ 	bNotVisibleAsMaster=rObj.bNotVisibleAsMaster;
/*N*/
/*N*/ 	bBoundRectDirty=rObj.bBoundRectDirty;
/*N*/ 	bSnapRectDirty=TRUE; //rObj.bSnapRectDirty;
/*N*/ 	bNotMasterCachable=rObj.bNotMasterCachable;
/*N*/ 	if (pPlusData!=NULL) { delete pPlusData; pPlusData=NULL; }
/*N*/ 	if (rObj.pPlusData!=NULL) {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ 	}
/*N*/ 	if (pPlusData!=NULL && pPlusData->pBroadcast!=NULL) {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ 	}
/*N*/ }






/*N*/ void SdrObject::TakeXorPoly(XPolyPolygon& rPoly, FASTBOOL /*bDetail*/) const
/*N*/ {
/*N*/ 	rPoly=XPolyPolygon(XPolygon(GetBoundRect()));
/*N*/ }

/*N*/ void SdrObject::TakeContour( XPolyPolygon& rPoly ) const
/*N*/ {
/*N*/ 	VirtualDevice   aBlackHole;
/*N*/ 	GDIMetaFile     aMtf;
/*N*/ 	SdrPaintInfoRec aPaintInfo;
/*N*/ 	XPolygon		aXPoly;
/*N*/
/*N*/ 	aBlackHole.EnableOutput( FALSE );
/*N*/ 	aBlackHole.SetDrawMode( DRAWMODE_NOFILL );
/*N*/
/*N*/ 	ExtOutputDevice	aXOut( &aBlackHole );
/*N*/ 	SdrObject*		pClone = Clone();
/*N*/
/*N*/ 	if(pClone && ISA(SdrEdgeObj))
/*N*/ 	{
/*N*/ 		// #102344# Flat cloned SdrEdgeObj, copy connections to original object(s).
/*N*/ 		// This is deleted later at delete pClone.
/*N*/ 		SdrObject* pLeft = ((SdrEdgeObj*)this)->GetConnectedNode(TRUE);
/*N*/ 		SdrObject* pRight = ((SdrEdgeObj*)this)->GetConnectedNode(FALSE);
/*N*/
/*N*/ 		if(pLeft)
/*N*/ 		{
/*N*/ 			pClone->ConnectToNode(TRUE, pLeft);
/*N*/ 		}
/*N*/
/*N*/ 		if(pRight)
/*N*/ 		{
/*N*/ 			pClone->ConnectToNode(FALSE, pRight);
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	pClone->SetItem(XLineStyleItem(XLINE_SOLID));
/*N*/ 	pClone->SetItem(XLineColorItem(String(), Color(COL_BLACK)));
/*N*/ 	pClone->SetItem(XFillStyleItem(XFILL_NONE));
/*N*/
/*N*/ 	aMtf.Record( &aBlackHole );
/*N*/ 	aPaintInfo.nPaintMode = SDRPAINTMODE_DRAFTTEXT | SDRPAINTMODE_DRAFTGRAF;
/*N*/ 	pClone->Paint( aXOut, aPaintInfo );
/*N*/ 	delete pClone;
/*N*/ 	aMtf.Stop();
/*N*/ 	aMtf.WindStart();
/*N*/ 	rPoly.Clear();
/*N*/
/*N*/ 	for( ULONG nActionNum = 0, nActionAnz = aMtf.GetActionCount(); nActionNum < nActionAnz; nActionNum++ )
/*N*/ 	{
/*N*/ 		const MetaAction&   rAct = *aMtf.GetAction( nActionNum );
/*N*/ 		BOOL                bXPoly = FALSE;
/*N*/
/*N*/ 		switch( rAct.GetType() )
/*N*/ 		{
/*?*/ 			case META_RECT_ACTION:
/*?*/ 			{
/*?*/ 				const Rectangle& rRect = ( (const MetaRectAction&) rAct ).GetRect();
/*?*/
/*?*/ 				if( rRect.GetWidth() && rRect.GetHeight() )
/*?*/ 				{
/*?*/ 					aXPoly = rRect;
/*?*/ 					bXPoly = TRUE;
/*?*/ 				}
/*?*/ 			}
/*?*/ 			break;
/*?*/
/*?*/ 			case META_ELLIPSE_ACTION:
/*?*/ 			{
/*?*/ 				const Rectangle& rRect = ( (const MetaEllipseAction&) rAct ).GetRect();
/*?*/
/*?*/ 				if( rRect.GetWidth() && rRect.GetHeight() )
/*?*/ 				{
/*?*/ 					aXPoly = XPolygon( rRect.Center(), rRect.GetWidth() >> 1, rRect.GetHeight() >> 1 );
/*?*/ 					bXPoly = TRUE;
/*?*/ 				}
/*?*/ 			}
/*?*/ 			break;
/*?*/
/*N*/ 			case META_POLYGON_ACTION:
/*N*/ 			{
/*N*/ 				const Polygon& rPoly = ( (const MetaPolygonAction&) rAct ).GetPolygon();
/*N*/
/*N*/ 				if( rPoly.GetSize() > 2 )
/*N*/ 				{
/*N*/ 					aXPoly = rPoly;
/*N*/ 					bXPoly = TRUE;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/
/*?*/ 			case META_POLYPOLYGON_ACTION:
/*?*/ 			{DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*?*/ 			}
/*?*/ 			break;
/*?*/
/*N*/ 			case META_POLYLINE_ACTION:
/*N*/ 			{
/*N*/ 				const Polygon& rPoly = ( (const MetaPolyLineAction&) rAct ).GetPolygon();
/*N*/
/*N*/ 				if( rPoly.GetSize() > 1 )
/*N*/ 				{
/*N*/ 					aXPoly = rPoly;
/*N*/ 					bXPoly = TRUE;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/
/*N*/ 			case META_LINE_ACTION:
/*N*/ 			{
/*N*/ 				aXPoly = XPolygon( 2 );
/*N*/ 				aXPoly[ 0 ] = ( (const MetaLineAction&) rAct ).GetStartPoint();
/*N*/ 				aXPoly[ 1 ] = ( (const MetaLineAction&) rAct ).GetEndPoint();
/*N*/ 				bXPoly = TRUE;
/*N*/ 			}
/*N*/ 			break;
/*N*/
/*N*/ 			default:
/*N*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		if( bXPoly )
/*N*/ 			rPoly.Insert( aXPoly );
/*N*/ 	}
/*N*/
/*N*/ 	// if we only have the outline of the object, we have _no_ contouir
/*N*/ 	if( rPoly.Count() == 1 )
/*N*/ 		rPoly.Clear();
/*N*/ }

/*?*/ void SdrObject::TakeContour(XPolyPolygon& rXPoly, SdrContourType eType) const
/*?*/ {
/*?*/ }

// Handles






// Drag









// Create






/*N*/ Pointer SdrObject::GetCreatePointer() const
/*N*/ {
/*N*/ 	return Pointer(POINTER_CROSS);
/*N*/ }

// Transformationen
/*N*/ void SdrObject::NbcMove(const Size& rSiz)
/*N*/ {
/*N*/ 	MoveRect(aOutRect,rSiz);
/*N*/ 	SetRectsDirty();
/*N*/ }

/*N*/ void SdrObject::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
/*N*/ {
/*N*/ 	FASTBOOL bXMirr=(xFact.GetNumerator()<0) != (xFact.GetDenominator()<0);
/*N*/ 	FASTBOOL bYMirr=(yFact.GetNumerator()<0) != (yFact.GetDenominator()<0);
/*N*/ 	if (bXMirr || bYMirr) {
/*N*/ 		Point aRef1(GetSnapRect().Center());
/*N*/ 		if (bXMirr) {
/*N*/ 			Point aRef2(aRef1);
/*N*/ 			aRef2.Y()++;
/*N*/ 			NbcMirrorGluePoints(aRef1,aRef2);
/*N*/ 		}
/*N*/ 		if (bYMirr) {
/*N*/ 			Point aRef2(aRef1);
/*N*/ 			aRef2.X()++;
/*N*/ 			NbcMirrorGluePoints(aRef1,aRef2);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	ResizeRect(aOutRect,rRef,xFact,yFact);
/*N*/ 	SetRectsDirty();
/*N*/ }

/*N*/ void SdrObject::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
/*N*/ {
/*N*/ 	SetGlueReallyAbsolute(TRUE);
/*N*/ 	aOutRect.Move(-rRef.X(),-rRef.Y());
/*N*/ 	Rectangle R(aOutRect);
/*N*/ 	if (sn==1.0 && cs==0.0) { // 90?
/*N*/ 		aOutRect.Left()  =-R.Bottom();
/*N*/ 		aOutRect.Right() =-R.Top();
/*N*/ 		aOutRect.Top()   =R.Left();
/*N*/ 		aOutRect.Bottom()=R.Right();
/*N*/ 	} else if (sn==0.0 && cs==-1.0) { // 180?
/*N*/ 		aOutRect.Left()  =-R.Right();
/*N*/ 		aOutRect.Right() =-R.Left();
/*N*/ 		aOutRect.Top()   =-R.Bottom();
/*N*/ 		aOutRect.Bottom()=-R.Top();
/*N*/ 	} else if (sn==-1.0 && cs==0.0) { // 270?
/*N*/ 		aOutRect.Left()  =R.Top();
/*N*/ 		aOutRect.Right() =R.Bottom();
/*N*/ 		aOutRect.Top()   =-R.Right();
/*N*/ 		aOutRect.Bottom()=-R.Left();
/*N*/ 	}
/*N*/ 	aOutRect.Move(rRef.X(),rRef.Y());
/*N*/ 	aOutRect.Justify(); // Sicherheitshalber
/*N*/ 	SetRectsDirty();
/*N*/ 	NbcRotateGluePoints(rRef,nWink,sn,cs);
/*N*/ 	SetGlueReallyAbsolute(FALSE);
/*N*/ }

/*N*/ void SdrObject::NbcMirror(const Point& rRef1, const Point& rRef2)
/*N*/ {
/*N*/ 	SetGlueReallyAbsolute(TRUE);
/*N*/ 	aOutRect.Move(-rRef1.X(),-rRef1.Y());
/*N*/ 	Rectangle R(aOutRect);
/*N*/ 	long dx=rRef2.X()-rRef1.X();
/*N*/ 	long dy=rRef2.Y()-rRef1.Y();
/*N*/ 	if (dx==0) {          // Vertikale Achse
/*N*/ 		aOutRect.Left() =-R.Right();
/*N*/ 		aOutRect.Right()=-R.Left();
/*N*/ 	} else if (dy==0) {   // Horizontale Achse
/*N*/ 		aOutRect.Top()   =-R.Bottom();
/*N*/ 		aOutRect.Bottom()=-R.Top();
/*N*/ 	} else if (dx==dy) {  /* 45 Grad Achse \ */
/*N*/ 		aOutRect.Left()  =R.Top();
/*N*/ 		aOutRect.Right() =R.Bottom();
/*N*/ 		aOutRect.Top()   =R.Left();
/*N*/ 		aOutRect.Bottom()=R.Right();
/*N*/ 	} else if (dx==-dy) { // 45 Grad Achse /
/*N*/ 		aOutRect.Left()  =-R.Bottom();
/*N*/ 		aOutRect.Right() =-R.Top();
/*N*/ 		aOutRect.Top()   =-R.Right();
/*N*/ 		aOutRect.Bottom()=-R.Left();
/*N*/ 	}
/*N*/ 	aOutRect.Move(rRef1.X(),rRef1.Y());
/*N*/ 	aOutRect.Justify(); // Sicherheitshalber
/*N*/ 	SetRectsDirty();
/*N*/ 	NbcMirrorGluePoints(rRef1,rRef2);
/*N*/ 	SetGlueReallyAbsolute(FALSE);
/*N*/ }

/*N*/ void SdrObject::NbcShear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear)
/*N*/ {
/*N*/ 	SetGlueReallyAbsolute(TRUE);
/*N*/ 	NbcShearGluePoints(rRef,nWink,tn,bVShear);
/*N*/ 	SetGlueReallyAbsolute(FALSE);
/*N*/ }

/*N*/ void SdrObject::Move(const Size& rSiz)
/*N*/ {
/*N*/ 	if (rSiz.Width()!=0 || rSiz.Height()!=0) {
/*N*/ 		Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
/*N*/ 		SendRepaintBroadcast();
/*N*/ 		NbcMove(rSiz);
/*N*/ 		SetChanged();
/*N*/ 		SendRepaintBroadcast();
/*N*/ 		SendUserCall(SDRUSERCALL_MOVEONLY,aBoundRect0);
/*N*/ 	}
/*N*/ }

/*N*/ void SdrObject::Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
/*N*/ {
/*N*/ 	if (xFact.GetNumerator()!=xFact.GetDenominator() || yFact.GetNumerator()!=yFact.GetDenominator()) {
/*N*/ 		Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
/*N*/ 		SendRepaintBroadcast();
/*N*/ 		NbcResize(rRef,xFact,yFact);
/*N*/ 		SetChanged();
/*N*/ 		SendRepaintBroadcast();
/*N*/ 		SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
/*N*/ 	}
/*N*/ }

/*N*/ void SdrObject::Rotate(const Point& rRef, long nWink, double sn, double cs)
/*N*/ {
/*N*/ 	if (nWink!=0) {
/*N*/ 		Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
/*N*/ 		SendRepaintBroadcast();
/*N*/ 		NbcRotate(rRef,nWink,sn,cs);
/*N*/ 		SetChanged();
/*N*/ 		SendRepaintBroadcast();
/*N*/ 		SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
/*N*/ 	}
/*N*/ }


/*N*/ void SdrObject::Shear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear)
/*N*/ {
/*N*/ 	if (nWink!=0) {
/*N*/ 		Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
/*N*/ 		SendRepaintBroadcast();
/*N*/ 		NbcShear(rRef,nWink,tn,bVShear);
/*N*/ 		SetChanged();
/*N*/ 		SendRepaintBroadcast();
/*N*/ 		SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
/*N*/ 	}
/*N*/ }

/*N*/ void SdrObject::NbcSetRelativePos(const Point& rPnt)
/*N*/ {
/*N*/ 	Point aRelPos0(GetSnapRect().TopLeft()-aAnchor);
/*N*/ 	Size aSiz(rPnt.X()-aRelPos0.X(),rPnt.Y()-aRelPos0.Y());
/*N*/ 	NbcMove(aSiz); // Der ruft auch das SetRectsDirty()
/*N*/ }

/*N*/ void SdrObject::SetRelativePos(const Point& rPnt)
/*N*/ {
/*N*/ 	if (rPnt!=GetRelativePos()) {
/*N*/ 		Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
/*N*/ 		SendRepaintBroadcast();
/*N*/ 		NbcSetRelativePos(rPnt);
/*N*/ 		SetChanged();
/*N*/ 		SendRepaintBroadcast();
/*N*/ 		SendUserCall(SDRUSERCALL_MOVEONLY,aBoundRect0);
/*N*/ 	}
/*N*/ }
/*N*/
/*N*/ Point SdrObject::GetRelativePos() const
/*N*/ {
/*N*/ 	return GetSnapRect().TopLeft()-aAnchor;
/*N*/ }
/*N*/
/*N*/ void SdrObject::NbcSetAnchorPos(const Point& rPnt)
/*N*/ {
/*N*/ 	Size aSiz(rPnt.X()-aAnchor.X(),rPnt.Y()-aAnchor.Y());
/*N*/ 	aAnchor=rPnt;
/*N*/ 	NbcMove(aSiz); // Der ruft auch das SetRectsDirty()
/*N*/ }
/*N*/
/*N*/ void SdrObject::SetAnchorPos(const Point& rPnt)
/*N*/ {
/*N*/ 	if (rPnt!=aAnchor) {
/*N*/ 		Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
/*N*/ 		SendRepaintBroadcast();
/*N*/ 		NbcSetAnchorPos(rPnt);
/*N*/ 		SetChanged();
/*N*/ 		SendRepaintBroadcast();
/*N*/ 		SendUserCall(SDRUSERCALL_MOVEONLY,aBoundRect0);
/*N*/ 	}
/*N*/ }
/*N*/
/*N*/ const Point& SdrObject::GetAnchorPos() const
/*N*/ {
/*N*/ 	return aAnchor;
/*N*/ }

/*N*/ void SdrObject::RecalcSnapRect()
/*N*/ {
/*N*/ }

/*N*/ const Rectangle& SdrObject::GetSnapRect() const
/*N*/ {
/*N*/ 	return aOutRect;
/*N*/ }

/*N*/ void SdrObject::NbcSetSnapRect(const Rectangle& rRect)
/*N*/ {
/*N*/ 	aOutRect=rRect;
/*N*/ }

/*N*/ const Rectangle& SdrObject::GetLogicRect() const
/*N*/ {
/*N*/ 	return GetSnapRect();
/*N*/ }

/*N*/ void SdrObject::NbcSetLogicRect(const Rectangle& rRect)
/*N*/ {
/*N*/ 	NbcSetSnapRect(rRect);
/*N*/ }

/*N*/ void SdrObject::SetSnapRect(const Rectangle& rRect)
/*N*/ {
/*N*/ 	Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
/*N*/ 	SendRepaintBroadcast();
/*N*/ 	NbcSetSnapRect(rRect);
/*N*/ 	SetChanged();
/*N*/ 	SendRepaintBroadcast();
/*N*/ 	SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
/*N*/ }

/*N*/ void SdrObject::SetLogicRect(const Rectangle& rRect)
/*N*/ {
/*N*/ 	Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
/*N*/ 	SendRepaintBroadcast();
/*N*/ 	NbcSetLogicRect(rRect);
/*N*/ 	SetChanged();
/*N*/ 	SendRepaintBroadcast();
/*N*/ 	SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
/*N*/ }

/*N*/ long SdrObject::GetRotateAngle() const
/*N*/ {
/*N*/ 	return 0;
/*N*/ }

/*N*/ long SdrObject::GetShearAngle(FASTBOOL bVertical) const
/*N*/ {
/*N*/ 	return 0;
/*N*/ }



/*N*/ FASTBOOL SdrObject::IsPolyObj() const
/*N*/ {
/*N*/ 	return FALSE;
/*N*/ }

/*?*/ USHORT SdrObject::GetPointCount() const
/*?*/ {
/*?*/ 	return 0;
/*?*/ }

/*?*/ const Point& SdrObject::GetPoint(USHORT i) const
/*?*/ {
/*?*/ 	return *((Point*)NULL);
/*?*/ }

/*N*/ void SdrObject::SetPoint(const Point& rPnt, USHORT i)
/*N*/ {
/*N*/ 	Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
/*N*/ 	SendRepaintBroadcast();
/*N*/ 	NbcSetPoint(rPnt,i);
/*N*/ 	SetChanged();
/*N*/ 	SendRepaintBroadcast();
/*N*/ 	SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
/*N*/ }

/*N*/ void SdrObject::NbcSetPoint(const Point& rPnt, USHORT i)
/*N*/ {
/*N*/ }











////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ FASTBOOL SdrObject::HasTextEdit() const
/*N*/ {
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ SdrObject* SdrObject::CheckTextEditHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
/*N*/ {
/*N*/ 	return CheckHit(rPnt,nTol,pVisiLayer);
/*N*/ }

/*N*/ FASTBOOL SdrObject::BegTextEdit(SdrOutliner& rOutl)
/*N*/ {
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ void SdrObject::EndTextEdit(SdrOutliner& rOutl)
/*N*/ {
/*N*/ }

/*N*/ void SdrObject::SetOutlinerParaObject(OutlinerParaObject* pTextObject)
/*N*/ {
/*N*/ 	Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
/*N*/ 	SendRepaintBroadcast();
/*N*/ 	NbcSetOutlinerParaObject(pTextObject);
/*N*/ 	SetChanged();
/*N*/ 	SendRepaintBroadcast();
/*N*/ 	if (GetBoundRect()!=aBoundRect0) {
/*N*/ 		SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
/*N*/ 	}
/*N*/ }

/*N*/ void SdrObject::NbcSetOutlinerParaObject(OutlinerParaObject* pTextObject)
/*N*/ {
/*N*/ }

/*N*/ OutlinerParaObject* SdrObject::GetOutlinerParaObject() const
/*N*/ {
/*N*/ 	return NULL;
/*N*/ }

/*N*/ void SdrObject::NbcReformatText()
/*N*/ {
/*N*/ }

/*N*/ void SdrObject::ReformatText()
/*N*/ {
/*N*/ 	Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
/*N*/ 	SendRepaintBroadcast();
/*N*/ 	NbcReformatText();
/*N*/ 	SetChanged();
/*N*/ 	SendRepaintBroadcast();
/*N*/ 	if (GetBoundRect()!=aBoundRect0) {
/*N*/ 		SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
/*N*/ 	}
/*N*/ }


/*N*/ void SdrObject::RestartAnimation(SdrPageView* pPageView) const
/*N*/ {
/*N*/ }

/*N*/ #define Imp2ndKennung (0x434F4D43)
/*N*/ SdrObjUserData* SdrObject::ImpGetMacroUserData() const
/*N*/ {
/*N*/ 	SdrObjUserData* pData=NULL;
/*N*/ 	USHORT nAnz=GetUserDataCount();
/*N*/ 	for (USHORT nNum=nAnz; nNum>0 && pData==NULL;) {
/*N*/ 		nNum--;
/*N*/ 		pData=GetUserData(nNum);
/*N*/ 		if (!pData->HasMacro(this)) pData=NULL;
/*N*/ 	}
/*N*/ 	return pData;
/*N*/ }

/*N*/ FASTBOOL SdrObject::HasMacro() const
/*N*/ {
/*N*/ 	SdrObjUserData* pData=ImpGetMacroUserData();
/*N*/ 	return pData!=NULL ? pData->HasMacro(this) : FALSE;
/*N*/ }






////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ SdrObjGeoData* SdrObject::NewGeoData() const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return NULL;//STRIP001
/*N*/ }

/*N*/ void SdrObject::SaveGeoData(SdrObjGeoData& rGeo) const
/*N*/ {
/*N*/ 	rGeo.aBoundRect    =GetBoundRect();
/*N*/ 	rGeo.aAnchor       =aAnchor       ;
/*N*/ 	rGeo.bMovProt      =bMovProt      ;
/*N*/ 	rGeo.bSizProt      =bSizProt      ;
/*N*/ 	rGeo.bNoPrint      =bNoPrint      ;
/*N*/ 	rGeo.bClosedObj    =bClosedObj    ;
/*N*/ 	rGeo.nLayerId      =nLayerId      ;
/*N*/
/*N*/ 	// Benutzerdefinierte Klebepunkte
/*N*/ 	if (pPlusData!=NULL && pPlusData->pGluePoints!=NULL) {
/*?*/ 		if (rGeo.pGPL!=NULL) {
/*?*/ 			*rGeo.pGPL=*pPlusData->pGluePoints;
/*?*/ 		} else {
/*?*/ 			rGeo.pGPL=new SdrGluePointList(*pPlusData->pGluePoints);
/*N*/ 		}
/*N*/ 	} else {
/*N*/ 		if (rGeo.pGPL!=NULL) {
/*?*/ 			delete rGeo.pGPL;
/*?*/ 			rGeo.pGPL=NULL;
/*?*/ 		}
/*N*/ 	}
/*N*/ }
/*N*/
/*N*/ void SdrObject::RestGeoData(const SdrObjGeoData& rGeo)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ }

/*N*/ SdrObjGeoData* SdrObject::GetGeoData() const
/*N*/ {
/*N*/ 	SdrObjGeoData* pGeo=NewGeoData();
/*N*/ 	SaveGeoData(*pGeo);
/*N*/ 	return pGeo;
/*N*/ }


////////////////////////////////////////////////////////////////////////////////////////////////////
// ItemSet access

/*N*/ SfxItemSet* SdrObject::mpEmptyItemSet = 0L;
/*N*/ const SfxItemSet& SdrObject::GetItemSet() const
/*N*/ {
/*N*/ 	if(!mpEmptyItemSet)
/*N*/ 		mpEmptyItemSet = ((SdrObject*)this)->CreateNewItemSet((SfxItemPool&)(*GetItemPool()));
/*N*/ 	DBG_ASSERT(mpEmptyItemSet, "Could not create an SfxItemSet(!)");
/*N*/ 	DBG_ASSERT(FALSE,"SdrObject::GetItemSet() should never be called, SdrObject has no Items");
/*N*/ 	return *mpEmptyItemSet;
/*N*/ }

/*N*/ SfxItemSet* SdrObject::CreateNewItemSet(SfxItemPool& rPool)
/*N*/ {
/*N*/ 	// Basic implementation; Basic object has NO attributes
/*N*/ 	DBG_ASSERT(FALSE,"SdrObject::CreateNewItemSet() should never be called, SdrObject has no Items");
/*N*/ 	return new SfxItemSet(rPool, SDRATTR_START,	SDRATTR_END);
/*N*/ }

/*N*/ void SdrObject::SetItem( const SfxPoolItem& rItem )
/*N*/ {
/*N*/ 	sal_uInt16 nWhichID(rItem.Which());
/*N*/
/*N*/ 	if(AllowItemChange(nWhichID, &rItem))
/*N*/ 	{
/*N*/ 		ItemChange(nWhichID, &rItem);
/*N*/ 		PostItemChange(nWhichID);
/*N*/
/*N*/ 		SfxItemSet aSet( *GetItemPool(), nWhichID, nWhichID, 0 );
/*N*/ 		aSet.Put( rItem );
/*N*/ 		ItemSetChanged( aSet );
/*N*/ 	}
/*N*/ }

/*N*/ void SdrObject::ClearItem( const sal_uInt16 nWhich )
/*N*/ {
/*N*/ 	if(AllowItemChange(nWhich))
/*N*/ 	{
/*N*/ 		ItemChange(nWhich);
/*N*/ 		PostItemChange(nWhich);
/*N*/
/*N*/ 		SfxItemSet aSet( *GetItemPool(), nWhich, nWhich, 0 );
/*N*/ 		ItemSetChanged( aSet );
/*N*/ 	}
/*N*/ }
/*N*/
/*N*/ void SdrObject::SetItemSet( const SfxItemSet& rSet )
/*N*/ {
/*N*/ 	SfxWhichIter aWhichIter(rSet);
/*N*/ 	sal_uInt16 nWhich(aWhichIter.FirstWhich());
/*N*/ 	const SfxPoolItem *pPoolItem;
/*N*/ 	std::vector< sal_uInt16 > aPostItemChangeList;
/*N*/ 	BOOL bDidChange(FALSE);
/*N*/ 	SfxItemSet aSet( *GetItemPool(), SDRATTR_START, EE_ITEMS_END, 0 );
/*N*/
/*N*/ 	while(nWhich)
/*N*/ 	{
/*N*/ 		if(SFX_ITEM_SET == rSet.GetItemState(nWhich, FALSE, &pPoolItem))
/*N*/ 		{
/*N*/ 			if(AllowItemChange(nWhich, pPoolItem))
/*N*/ 			{
/*N*/ 				bDidChange = TRUE;
/*N*/ 				ItemChange(nWhich, pPoolItem);
/*N*/ 				aPostItemChangeList.push_back( nWhich );
/*N*/ 				aSet.Put( *pPoolItem );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		nWhich = aWhichIter.NextWhich();
/*N*/ 	}
/*N*/
/*N*/ 	if(bDidChange)
/*N*/ 	{
/*N*/ 		std::vector< sal_uInt16 >::iterator aIter = aPostItemChangeList.begin();
/*N*/ 		const std::vector< sal_uInt16 >::iterator aEnd = aPostItemChangeList.end();
/*N*/ 		while( aIter != aEnd )
/*N*/ 		{
/*N*/ 			PostItemChange((*aIter));
/*N*/ 			aIter++;
/*N*/ 		}
/*N*/
/*N*/ 		ItemSetChanged( aSet );
/*N*/ 	}
/*N*/ }
/*N*/
/*N*/ ////////////////////////////////////////////////////////////////////////////////////////////////////
/*N*/ // ItemSet was changed, maybe user wants to react
/*N*/
/*N*/ void SdrObject::ItemSetChanged(const SfxItemSet& rSet)
/*N*/ {
/*N*/ }
/*N*/
/*N*/ void SdrObject::BroadcastItemChange(const SdrBroadcastItemChange& rChange)
/*N*/ {
/*N*/ 	sal_uInt32 nCount(rChange.GetRectangleCount());
/*N*/
/*N*/ 	sal_uInt32 a; for(a=0; a < nCount; a++)
/*N*/ 		SendRepaintBroadcast(rChange.GetRectangle(a));
/*N*/
/*N*/ 	if(ISA(SdrObjGroup))
/*N*/ 	{
/*N*/ 		SdrObjListIter aIter(*((SdrObjGroup*)this), IM_DEEPNOGROUPS);
/*N*/ 		while(aIter.IsMore())
/*N*/ 		{
/*N*/ 			SdrObject* pObj = aIter.Next();
/*N*/ 			SendRepaintBroadcast(pObj->GetBoundRect());
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		SendRepaintBroadcast(GetBoundRect());
/*N*/
/*N*/ 	for(a = 0; a < nCount; a++)
/*N*/ 		SendUserCall(SDRUSERCALL_CHGATTR, rChange.GetRectangle(a));
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// syntactical sugar for ItemSet accesses



/*N*/ void SdrObject::SetItemSetAndBroadcast(const SfxItemSet& rSet)
/*N*/ {
/*N*/ 	SdrBroadcastItemChange aC(*this);
/*N*/ 	SetItemSet(rSet);
/*N*/ 	BroadcastItemChange(aC);
/*N*/ }

/*N*/ const SfxPoolItem& SdrObject::GetItem(const sal_uInt16 nWhich) const
/*N*/ {
/*N*/ 	return GetItemSet().Get(nWhich);
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// private support routines for ItemSet access

/*N*/ BOOL SdrObject::AllowItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem) const
/*N*/ {
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ void SdrObject::ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem)
/*N*/ {
/*N*/ 	DBG_ASSERT(FALSE,"SdrObject::ItemChange() should never be called, SdrObject has no Items");
/*N*/ }

/*N*/ void SdrObject::PostItemChange(const sal_uInt16 nWhich)
/*N*/ {
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// pre- and postprocessing for objects for saving

/*N*/ void SdrObject::PreSave()
/*N*/ {
/*N*/ }

/*N*/ void SdrObject::PostSave()
/*N*/ {
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrObject::ApplyNotPersistAttr(const SfxItemSet& rAttr)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ }



/*N*/ void SdrObject::TakeNotPersistAttr(SfxItemSet& rAttr, FASTBOOL bMerge) const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ }

/*N*/ SfxStyleSheet* SdrObject::GetStyleSheet() const
/*N*/ {
/*N*/ 	// Das hier ist ein Hack:
/*N*/ 	return NULL;
/*N*/ }

/*N*/ void SdrObject::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
/*N*/ {
/*N*/ }

/*N*/ void SdrObject::NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
/*N*/ {
/*N*/ }

// Das Broadcasting beim Setzen der Attribute wird vom AttrObj gemanagt
////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ FASTBOOL SdrObject::IsNode() const
/*N*/ {
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ SdrGluePoint SdrObject::GetVertexGluePoint(USHORT nPosNum) const
/*N*/ {
/*N*/ 	Rectangle aR(GetBoundRect());
/*N*/ 	Point aPt;
/*N*/ 	switch (nPosNum) {
/*N*/ 		case 0 : aPt=aR.TopCenter();    break;
/*N*/ 		case 1 : aPt=aR.RightCenter();  break;
/*N*/ 		case 2 : aPt=aR.BottomCenter(); break;
/*N*/ 		case 3 : aPt=aR.LeftCenter();   break;
/*N*/ 	}
/*N*/ 	aPt-=GetSnapRect().Center();
/*N*/ 	SdrGluePoint aGP(aPt);
/*N*/ 	aGP.SetPercent(FALSE);
/*N*/ 	return aGP;
/*N*/ }



/*N*/ SdrGluePointList* SdrObject::GetGluePointList()
/*N*/ {
/*N*/ 	if (pPlusData!=NULL) return pPlusData->pGluePoints;
/*N*/ 	return NULL;
/*N*/ }

/*N*/ SdrGluePointList* SdrObject::ForceGluePointList()
/*N*/ {
/*N*/ 	ImpForcePlusData();
/*N*/ 	if (pPlusData->pGluePoints==NULL) {
/*N*/ 		pPlusData->pGluePoints=new SdrGluePointList;
/*N*/ 	}
/*N*/ 	return pPlusData->pGluePoints;
/*N*/ }

/*N*/ void SdrObject::SetGlueReallyAbsolute(FASTBOOL bOn)
/*N*/ {
/*N*/ 	// erst Const-Aufruf um zu sehen, ob
/*N*/ 	// ueberhaupt Klebepunkte da sind
/*N*/ 	// const-Aufruf erzwingen!
/*N*/ 	if (GetGluePointList()!=NULL) {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ 	}
/*N*/ }

/*N*/ void SdrObject::NbcRotateGluePoints(const Point& rRef, long nWink, double sn, double cs)
/*N*/ {
/*N*/ 	// erst Const-Aufruf um zu sehen, ob
/*N*/ 	// ueberhaupt Klebepunkte da sind
/*N*/ 	// const-Aufruf erzwingen!
/*N*/ 	if (GetGluePointList()!=NULL) {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ 	}
/*N*/ }

/*N*/ void SdrObject::NbcMirrorGluePoints(const Point& rRef1, const Point& rRef2)
/*N*/ {
/*N*/ 	// erst Const-Aufruf um zu sehen, ob
/*N*/ 	// ueberhaupt Klebepunkte da sind
/*N*/ 	// const-Aufruf erzwingen!
/*N*/ 	if (GetGluePointList()!=NULL) {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ 	}
/*N*/ }

/*N*/ void SdrObject::NbcShearGluePoints(const Point& rRef, long nWink, double tn, FASTBOOL bVShear)
/*N*/ {
/*N*/ 	// erst Const-Aufruf um zu sehen, ob
/*N*/ 	// ueberhaupt Klebepunkte da sind
/*N*/ 	// const-Aufruf erzwingen!
/*N*/ 	if (GetGluePointList()!=NULL) {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ 	}
/*N*/ }




/*N*/ void SdrObject::ConnectToNode(FASTBOOL bTail1, SdrObject* pObj)
/*N*/ {
/*N*/ }

/*N*/ void SdrObject::DisconnectFromNode(FASTBOOL bTail1)
/*N*/ {
/*N*/ }


////////////////////////////////////////////////////////////////////////////////////////////////////


// convert this path object to contour object, even when it is a group

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ SdrObject* SdrObject::ConvertToPolyObj(BOOL bBezier, BOOL bLineToArea) const
/*N*/ {
/*N*/ 	SdrObject* pRet = DoConvertToPolyObj(bBezier);
/*N*/
/*N*/ 	if(pRet && bLineToArea)
/*N*/ 	{DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ 	}
/*N*/
/*N*/ 	return pRet;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ SdrObject* SdrObject::DoConvertToPolyObj(BOOL bBezier) const
/*N*/ {
/*N*/ 	return NULL;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// Streams

/*N*/ void SdrObject::AfterRead()
/*N*/ {
/*N*/ 	USHORT nAnz=GetUserDataCount();
/*N*/ 	for (USHORT i=0; i<nAnz; i++) {
/*N*/ 		GetUserData(i)->AfterRead();
/*N*/ 	}
/*N*/ }

/*N*/ void SdrObject::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	if (rIn.GetError()!=0) return;
/*N*/ 	SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrObject");
/*N*/ #endif
/*N*/ 	rIn>>aOutRect;
/*N*/ 	rIn>>nLayerId;
/*N*/ 	rIn>>aAnchor;

    // #97849# when in a Draw/Impress binary import the anchor pos is set it's an error.
    // So, when could figure out that a Draw/Impress is running, correct that position
    // to (0,0). Anchor is not used but with 6.0 and XML no longer ignored in Draw/Impress
    // so this correction needs to be made for objects with that error. These could
    // be created when copying back anchor based draw objects from Calc or Writer to
    // Draw/Impress, this did not reset the anchor position.
/*N*/ 	if((aAnchor.X() || aAnchor.Y()) && GetModel() && GetModel()->ISA(FmFormModel))
/*N*/ 	{
/*N*/ 		// find out which application is running
/*N*/ 		SfxObjectShell* pObjectShell = ((FmFormModel*)GetModel())->GetObjectShell();
/*N*/ 		if(pObjectShell)
/*N*/ 		{
                SfxObjectFactory& rFac = pObjectShell->GetFactory();
                if ( rFac.GetShortName() &&
                    ( strcmp(rFac.GetShortName(), "sdraw") == 0 ||
                      strcmp(rFac.GetShortName(), "simpress") == 0 ) )
/*N*/ 			{
/*N*/               // it's a draw/Impress, reset anchor pos hard
/*N*/               aAnchor = Point(0, 0);
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	BOOL bTemp;
/*N*/ 	rIn>>bTemp; bMovProt=bTemp;
/*N*/ 	rIn>>bTemp; bSizProt=bTemp;
/*N*/ 	rIn>>bTemp; bNoPrint=bTemp;
/*N*/ 	rIn>>bTemp; bMarkProt=bTemp; // war ganz frueher mal bTextToContour
/*N*/ 	rIn>>bTemp; bEmptyPresObj=bTemp;
/*N*/ 	if (rHead.GetVersion()>=4) {
/*N*/ 		rIn>>bTemp; bNotVisibleAsMaster=bTemp;
/*N*/ 	}
/*N*/ 	if (rHead.GetVersion()>=11) { // ab V11 sind Connectors in DownCompat gefasst (mit Flag davor)
/*N*/ 		rIn>>bTemp;
/*N*/ 		if (bTemp) {
/*?*/ 			SdrDownCompat aGluePointsCompat(rIn,STREAM_READ);
/*?*/ #ifdef DBG_UTIL
/*?*/ 			aGluePointsCompat.SetID("SdrObject(Klebepunkte)");
/*?*/ #endif
/*?*/ 			if (aGluePointsCompat.GetBytesLeft()!=0) {
/*?*/ 				ImpForcePlusData();
/*?*/ 				if (pPlusData->pGluePoints==NULL) pPlusData->pGluePoints=new SdrGluePointList;
/*?*/ 				rIn>>*pPlusData->pGluePoints;
/*?*/ 			}
/*N*/ 		}
/*N*/ 	} else {
/*N*/ 		Polygon aTmpPoly;
/*N*/ 		rIn>>aTmpPoly; // aConnectors; ehemals Konnektoren
/*N*/ 	}
/*N*/
/*N*/ 	if (pPlusData!=NULL && pPlusData->pUserDataList!=NULL) {
/*N*/ 		delete pPlusData->pUserDataList;
/*N*/ 		pPlusData->pUserDataList=NULL;
/*N*/ 	}
/*N*/ 	FASTBOOL bReadUserDataList=FALSE;
/*N*/ 	SdrDownCompat* pUserDataListCompat=NULL;
/*N*/ 	if (rHead.GetVersion()>=11) { // ab V11 ist die UserDataList in DownCompat gefasst (mit Flag davor)
/*N*/ 		rIn>>bTemp;
/*N*/ 		bReadUserDataList=bTemp;
/*N*/ 		if (bTemp) {
/*N*/ 			pUserDataListCompat=new SdrDownCompat(rIn,STREAM_READ); // Record fuer UserDataList oeffnen
/*N*/ #ifdef DBG_UTIL
/*N*/ 			pUserDataListCompat->SetID("SdrObject(UserDataList)");
/*N*/ #endif
/*N*/ 		}
/*N*/ 	} else {
/*N*/ 		bReadUserDataList=TRUE;
/*N*/ 	}
/*N*/ 	if (bReadUserDataList) {
/*N*/ 		USHORT nUserDataAnz;
/*N*/ 		rIn>>nUserDataAnz;
/*N*/ 		if (nUserDataAnz!=0) {
/*N*/ 			ImpForcePlusData();
/*N*/ 			pPlusData->pUserDataList=new SdrObjUserDataList;
/*N*/ 			for (USHORT i=0; i<nUserDataAnz; i++) {
/*N*/ 				SdrDownCompat* pUserDataCompat=NULL;
/*N*/ 				if (rHead.GetVersion()>=11) { // ab V11 sind UserData in DownCompat gefasst
/*N*/ 					//SdrDownCompat aUserDataCompat(rIn,STREAM_READ); // Record fuer UserData oeffnen (seit V11)
/*N*/ 					pUserDataCompat=new SdrDownCompat(rIn,STREAM_READ); // Record fuer UserData oeffnen (seit V11)
/*N*/ #ifdef DBG_UTIL
/*N*/ 					pUserDataCompat->SetID("SdrObject(UserData)");
/*N*/ #endif
/*N*/ 				}
/*N*/ 				UINT32 nInvent;
/*N*/ 				UINT16 nIdent;
/*N*/ 				rIn>>nInvent;
/*N*/ 				rIn>>nIdent;
/*N*/ 				SdrObjUserData* pData=SdrObjFactory::MakeNewObjUserData(nInvent,nIdent,this);
/*N*/ 				if (pData!=NULL)
/*N*/ 				{
/*N*/ 					pData->ReadData(rIn);
/*N*/ 					pPlusData->pUserDataList->InsertUserData(pData);
/*N*/ 				} else {
/*N*/ 					// Wenn UserDataFactory nicht gesetzt ist, kann auch keiner
/*N*/ 					// etwas mit diesen Daten anfangen; durch Compat werden sie
/*N*/ 					// eh ueberlesen, daher ist diese Assertion überflüssig (KA)
/*N*/ 					// DBG_ERROR("SdrObject::ReadData(): ObjFactory kann UserData nicht erzeugen");
/*N*/ 				}
/*N*/ 				if (pUserDataCompat!=NULL) { // Aha, UserData war eingepackt. Record nun schliessen
/*N*/ 					delete pUserDataCompat;
/*N*/ 					pUserDataCompat=NULL;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if (pUserDataListCompat!=NULL) { // Aha, UserDataList war eingepackt. Record nun schliessen
/*N*/ 			delete pUserDataListCompat;
/*N*/ 			pUserDataListCompat=NULL;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SdrObject::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ 	SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrObject");
/*N*/ #endif
/*N*/ 	rOut<<GetBoundRect();
/*N*/ 	rOut<<nLayerId;
/*N*/ 	rOut<<aAnchor;
/*N*/ 	BOOL bTemp;
/*N*/ 	bTemp=bMovProt;       rOut<<bTemp;
/*N*/ 	bTemp=bSizProt;       rOut<<bTemp;
/*N*/ 	bTemp=bNoPrint;       rOut<<bTemp;
/*N*/ 	bTemp=bMarkProt;      rOut<<bTemp;
/*N*/ 	bTemp=bEmptyPresObj;  rOut<<bTemp;
/*N*/ 	bTemp=bNotVisibleAsMaster; rOut<<bTemp;
/*N*/
/*N*/ 	// Konnektoren
/*N*/ 	bTemp=pPlusData!=NULL && pPlusData->pGluePoints!=NULL && pPlusData->pGluePoints->GetCount()!=0;
/*N*/ 	rOut<<bTemp; // Flag fuer GluePointList vorhanden
/*N*/ 	if (bTemp) {
/*?*/ 		SdrDownCompat aConnectorsCompat(rOut,STREAM_WRITE); // ab V11 Konnektoren einpacken
/*N*/ #ifdef DBG_UTIL
/*?*/ 		aConnectorsCompat.SetID("SdrObject(Klebepunkte)");
/*N*/ #endif
/*?*/ 		rOut<<*pPlusData->pGluePoints;
/*N*/ 	}

    // UserData
/*N*/ 	USHORT nUserDataAnz=GetUserDataCount();
/*N*/ 	bTemp=nUserDataAnz!=0;
/*N*/ 	rOut<<bTemp;
/*N*/ 	if (bTemp) {
/*N*/ 		SdrDownCompat aUserDataListCompat(rOut,STREAM_WRITE); // Record fuer UserDataList oeffnen (seit V11)
/*N*/ #ifdef DBG_UTIL
/*N*/ 		aUserDataListCompat.SetID("SdrObject(UserDataList)");
/*N*/ #endif
/*N*/ 		rOut<<nUserDataAnz;
/*N*/ 		for (USHORT i=0; i<nUserDataAnz; i++) {
/*N*/ 			SdrDownCompat aUserDataCompat(rOut,STREAM_WRITE); // Record fuer UserData oeffnen (seit V11)
/*N*/ #ifdef DBG_UTIL
/*N*/ 			aUserDataCompat.SetID("SdrObject(UserData)");
/*N*/ #endif
/*N*/ 			pPlusData->pUserDataList->GetUserData(i)->WriteData(rOut);
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ }

/*N*/ SvStream& operator>>(SvStream& rIn, SdrObject& rObj)
/*N*/ {
/*N*/ 	DBG_ASSERT(!rObj.IsNotPersistent(),"operator>>(SdrObject): Ein nicht persistentes Zeichenobjekts wird gestreamt");
/*N*/ 	SdrObjIOHeader aHead(rIn,STREAM_READ,&rObj);
/*N*/ 	rObj.ReadData(aHead,rIn);
/*N*/ 	return rIn;
/*N*/ }

/*N*/ SvStream& operator<<(SvStream& rOut, const SdrObject& rObj)
/*N*/ {
/*N*/ 	DBG_ASSERT(!rObj.IsNotPersistent(),"operator<<(SdrObject): Ein nicht persistentes Zeichenobjekts wird gestreamt");
/*N*/ 	SdrObjIOHeader aHead(rOut,STREAM_WRITE,&rObj);
/*N*/
/*N*/ 	if(rObj.ISA(SdrVirtObj))
/*N*/ 	{
/*N*/ 		// #108784#
/*N*/ 		// force to write a naked SdrObj
/*N*/ 		aHead.nIdentifier = OBJ_NONE;
/*N*/ 		rObj.SdrObject::WriteData(rOut);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		rObj.WriteData(rOut);
/*N*/ 	}
/*N*/
/*N*/ 	return rOut;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrObject::SetInserted(FASTBOOL bIns)
/*N*/ {
/*N*/ 	if (bIns!=bInserted) {
/*N*/ 		bInserted=bIns;
/*N*/ 		Rectangle aBoundRect0(GetBoundRect());
/*N*/ 		if (bIns) SendUserCall(SDRUSERCALL_INSERTED,aBoundRect0);
/*N*/ 		else SendUserCall(SDRUSERCALL_REMOVED,aBoundRect0);
/*N*/
/*N*/ 		if (pPlusData!=NULL && pPlusData->pBroadcast!=NULL) { // #42522#
/*N*/ 			SdrHint aHint(*this);
/*N*/ 			aHint.SetKind(bIns?HINT_OBJINSERTED:HINT_OBJREMOVED);
/*N*/ 			pPlusData->pBroadcast->Broadcast(aHint);
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SdrObject::SetMoveProtect(FASTBOOL bProt)
/*N*/ {
/*N*/ 	bMovProt=bProt;
/*N*/ 	SetChanged();
/*N*/ 	if (bInserted && pModel!=NULL) {
/*N*/ 		SdrHint aHint(*this);
/*N*/ 		aHint.SetNeedRepaint(FALSE);
/*N*/ 		pModel->Broadcast(aHint);
/*N*/ 	}
/*N*/ }

/*N*/ void SdrObject::SetResizeProtect(FASTBOOL bProt)
/*N*/ {
/*N*/ 	bSizProt=bProt;
/*N*/ 	SetChanged();
/*N*/ 	if (bInserted && pModel!=NULL) {
/*N*/ 		SdrHint aHint(*this);
/*N*/ 		aHint.SetNeedRepaint(FALSE);
/*N*/ 		pModel->Broadcast(aHint);
/*N*/ 	}
/*N*/ }

/*N*/ void SdrObject::SetPrintable(FASTBOOL bPrn)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ USHORT SdrObject::GetUserDataCount() const
/*N*/ {
/*N*/ 	if (pPlusData==NULL || pPlusData->pUserDataList==NULL) return 0;
/*N*/ 	return pPlusData->pUserDataList->GetUserDataCount();
/*N*/ }

/*N*/ SdrObjUserData* SdrObject::GetUserData(USHORT nNum) const
/*N*/ {
/*N*/ 	if (pPlusData==NULL || pPlusData->pUserDataList==NULL) return NULL;
/*N*/ 	return pPlusData->pUserDataList->GetUserData(nNum);
/*N*/ }
/*N*/
/*N*/ void SdrObject::InsertUserData(SdrObjUserData* pData, USHORT nPos)
/*N*/ {
/*N*/ 	if (pData!=NULL) {
/*N*/ 		ImpForcePlusData();
/*N*/ 		if (pPlusData->pUserDataList==NULL) pPlusData->pUserDataList=new SdrObjUserDataList;
/*N*/ 		pPlusData->pUserDataList->InsertUserData(pData,nPos);
/*N*/ 	} else {
/*N*/ 		DBG_ERROR("SdrObject::InsertUserData(): pData ist NULL-Pointer");
/*N*/ 	}
/*N*/ }


/*N*/ void SdrObject::SendUserCall(SdrUserCallType eUserCall, const Rectangle& rBoundRect)
/*N*/ {
/*N*/ 	SdrObjGroup* pGroup = NULL;
/*N*/
/*N*/ 	if( pObjList && pObjList->GetListKind() == SDROBJLIST_GROUPOBJ )
/*N*/ 		pGroup = (SdrObjGroup*) pObjList->GetOwnerObj();
/*N*/
/*N*/ 	if ( pUserCall )
/*N*/ 	{
/*N*/ 		// UserCall ausfuehren
/*N*/ 		pUserCall->Changed( *this, eUserCall, rBoundRect );
/*N*/ 	}
/*N*/
/*N*/ 	while( pGroup )
/*N*/ 	{
/*N*/ 		// Gruppe benachrichtigen
/*N*/ 		if( pGroup->GetUserCall() )
/*N*/ 		{
/*N*/ 			SdrUserCallType eChildUserType = SDRUSERCALL_CHILD_CHGATTR;
/*N*/
/*N*/ 			switch( eUserCall )
/*N*/ 			{
/*N*/ 				case SDRUSERCALL_MOVEONLY:
/*N*/ 					eChildUserType = SDRUSERCALL_CHILD_MOVEONLY;
/*N*/ 				break;
/*N*/
/*N*/ 				case SDRUSERCALL_RESIZE:
/*N*/ 					eChildUserType = SDRUSERCALL_CHILD_RESIZE;
/*N*/ 				break;
/*N*/
/*N*/ 				case SDRUSERCALL_CHGATTR:
/*N*/ 					eChildUserType = SDRUSERCALL_CHILD_CHGATTR;
/*N*/ 				break;
/*N*/
/*N*/ 				case SDRUSERCALL_DELETE:
/*N*/ 					eChildUserType = SDRUSERCALL_CHILD_DELETE;
/*N*/ 				break;
/*N*/
/*N*/ 				case SDRUSERCALL_COPY:
/*N*/ 					eChildUserType = SDRUSERCALL_CHILD_COPY;
/*N*/ 				break;
/*N*/
/*N*/ 				case SDRUSERCALL_INSERTED:
/*N*/ 					eChildUserType = SDRUSERCALL_CHILD_INSERTED;
/*N*/ 				break;
/*N*/
/*N*/ 				case SDRUSERCALL_REMOVED:
/*N*/ 					eChildUserType = SDRUSERCALL_CHILD_REMOVED;
/*N*/ 				break;
/*N*/ 			}
/*N*/
/*N*/ 			pGroup->GetUserCall()->Changed( *this, eChildUserType, rBoundRect );
/*N*/ 		}
/*N*/
/*N*/ 		if( pGroup->GetObjList()                                       &&
/*N*/ 			pGroup->GetObjList()->GetListKind() == SDROBJLIST_GROUPOBJ &&
/*N*/ 			pGroup != (SdrObjGroup*) pObjList->GetOwnerObj() )
/*N*/ 			pGroup = (SdrObjGroup*) pObjList->GetOwnerObj();
/*N*/ 		else
/*N*/ 			pGroup = NULL;
/*N*/ 	}
/*N*/ }

// ItemPool fuer dieses Objekt wechseln
/*N*/ void SdrObject::MigrateItemPool(SfxItemPool* pSrcPool, SfxItemPool* pDestPool, SdrModel* pNewModel)
/*N*/ {
/*N*/ 	// Hier passiert erst was in SdrAttrObj und in SdrObjGroup
/*N*/ }


/*N*/ ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SdrObject::getUnoShape()
/*N*/ {
/*N*/ 	// try weak reference first
/*N*/ 	uno::Reference< uno::XInterface > xShape( mxUnoShape );
/*N*/
/*N*/ 	if( !xShape.is() && pPage )
/*N*/ 	{
/*N*/ 		uno::Reference< uno::XInterface > xPage( pPage->getUnoPage() );
/*N*/ 		if( xPage.is() )
/*N*/ 		{
/*N*/ 			SvxDrawPage* pDrawPage = SvxDrawPage::getImplementation(xPage);
/*N*/ 			if( pDrawPage )
/*N*/ 			{
/*N*/ 				// create one
/*N*/ 				xShape = pDrawPage->_CreateShape( this );
/*N*/ 				mxUnoShape = xShape;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	return xShape;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// transformation interface for StarOfficeAPI. This implements support for
// homogen 3x3 matrices containing the transformation of the SdrObject. At the
// moment it contains a shearX, rotation and translation, but for setting all linear
// transforms like Scale, ShearX, ShearY, Rotate and Translate are supported.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
// gets base transformation and rectangle of object. If it's an SdrPathObj it fills the PolyPolygon
// with the base geometry and returns TRUE. Otherwise it returns FALSE.
/*N*/ BOOL SdrObject::TRGetBaseGeometry(Matrix3D& rMat, XPolyPolygon& rPolyPolygon) const
/*N*/ {
/*N*/ 	// any kind of SdrObject, just use SnapRect
/*N*/ 	Rectangle aRectangle(GetSnapRect());
/*N*/
/*N*/ 	// convert to transformation values
/*N*/ 	Vector2D aScale((double)aRectangle.GetWidth(), (double)aRectangle.GetHeight());
/*N*/ 	Vector2D aTranslate((double)aRectangle.Left(), (double)aRectangle.Top());
/*N*/
/*N*/ 	// position maybe relative to anchorpos, convert
/*N*/ 	if( pModel->IsWriter() )
/*N*/ 	{
/*?*/ 		if(GetAnchorPos().X() != 0 || GetAnchorPos().Y() != 0)
/*?*/ 			aTranslate -= Vector2D(GetAnchorPos().X(), GetAnchorPos().Y());
/*N*/ 	}

    // force MapUnit to 100th mm
/*N*/ 	SfxMapUnit eMapUnit = pModel->GetItemPool().GetMetric(0);
/*N*/ 	if(eMapUnit != SFX_MAPUNIT_100TH_MM)
/*N*/ 	{
/*N*/ 		switch(eMapUnit)
/*N*/ 		{
/*?*/ 			case SFX_MAPUNIT_TWIP :
/*?*/ 			{
/*?*/ 				// postion
/*?*/ 				// #104018#
/*?*/ 				aTranslate.X() = ImplTwipsToMM(aTranslate.X());
/*?*/ 				aTranslate.Y() = ImplTwipsToMM(aTranslate.Y());
/*?*/
/*?*/ 				// size
/*?*/ 				// #104018#
/*?*/ 				aScale.X() = ImplTwipsToMM(aScale.X());
/*?*/ 				aScale.Y() = ImplTwipsToMM(aScale.Y());
/*?*/
/*?*/ 				break;
/*?*/ 			}
/*?*/ 			default:
/*?*/ 			{
/*?*/ 				DBG_ERROR("TRGetBaseGeometry: Missing unit translation to 100th mm!");
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}

    // build matrix
/*N*/ 	rMat.Identity();
/*N*/ 	if(aScale.X() != 1.0 || aScale.Y() != 1.0)
/*N*/ 		rMat.Scale(aScale.X(), aScale.Y());
/*N*/ 	if(aTranslate.X() != 0.0 || aTranslate.Y() != 0.0)
/*N*/ 		rMat.Translate(aTranslate.X(), aTranslate.Y());
/*N*/
/*N*/ 	return FALSE;
/*N*/ }

// sets the base geometry of the object using infos contained in the homogen 3x3 matrix.
// If it's an SdrPathObj it will use the provided geometry information. The Polygon has
// to use (0,0) as upper left and will be scaled to the given size in the matrix.
/*N*/ void SdrObject::TRSetBaseGeometry(const Matrix3D& rMat, const XPolyPolygon& rPolyPolygon)
/*N*/ {
/*N*/ 	// break up matrix
/*N*/ 	Vector2D aScale, aTranslate;
/*N*/ 	double fShear, fRotate;
/*N*/ 	rMat.DecomposeAndCorrect(aScale, fShear, fRotate, aTranslate);
/*N*/
/*N*/ 	// force metric to pool metric
/*N*/ 	SfxMapUnit eMapUnit = pModel->GetItemPool().GetMetric(0);
/*N*/ 	if(eMapUnit != SFX_MAPUNIT_100TH_MM)
/*N*/ 	{
/*N*/ 		switch(eMapUnit)
/*?*/ 		{
/*?*/ 			case SFX_MAPUNIT_TWIP :
/*?*/ 			{
/*?*/ 				// position
/*?*/ 				// #104018#
/*?*/ 				aTranslate.X() = ImplMMToTwips(aTranslate.X());
/*?*/ 				aTranslate.Y() = ImplMMToTwips(aTranslate.Y());
/*?*/
/*?*/ 				// size
/*?*/ 				// #104018#
/*?*/ 				aScale.X() = ImplMMToTwips(aScale.X());
/*?*/ 				aScale.Y() = ImplMMToTwips(aScale.Y());
/*?*/
/*?*/ 				break;
/*?*/ 			}
/*?*/ 			default:
/*?*/ 			{
/*?*/ 				DBG_ERROR("TRSetBaseGeometry: Missing unit translation to PoolMetric!");
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}

    // if anchor is used, make position relative to it
/*N*/ 	if( pModel->IsWriter() )
/*N*/ 	{
/*?*/ 		if(GetAnchorPos().X() != 0 || GetAnchorPos().Y() != 0)
/*?*/ 			aTranslate -= Vector2D(GetAnchorPos().X(), GetAnchorPos().Y());
/*N*/ 	}
/*N*/
/*N*/ 	// build BaseRect
/*N*/ 	Point aPoint(FRound(aTranslate.X()), FRound(aTranslate.Y()));
/*N*/ 	Rectangle aBaseRect(aPoint, Size(FRound(aScale.X()), FRound(aScale.Y())));
/*N*/
/*N*/ 	// set BaseRect
/*N*/ 	SetSnapRect(aBaseRect);
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   @@@@  @@@@@  @@@@@@  @@@@@  @@@@   @@@@  @@@@@@  @@@@  @@@@@  @@  @@
//  @@  @@ @@  @@     @@  @@    @@  @@ @@  @@   @@   @@  @@ @@  @@ @@  @@
//  @@  @@ @@  @@     @@  @@    @@  @@ @@       @@   @@  @@ @@  @@ @@  @@
//  @@  @@ @@@@@      @@  @@@@  @@@@@@ @@       @@   @@  @@ @@@@@   @@@@
//  @@  @@ @@  @@     @@  @@    @@  @@ @@       @@   @@  @@ @@  @@   @@
//  @@  @@ @@  @@ @@  @@  @@    @@  @@ @@  @@   @@   @@  @@ @@  @@   @@
//   @@@@  @@@@@   @@@@   @@    @@  @@  @@@@    @@    @@@@  @@  @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ SdrObject* SdrObjFactory::MakeNewObject(UINT32 nInvent, UINT16 nIdent, SdrPage* pPage, SdrModel* pModel)
/*N*/ {
/*N*/ 	if(pModel == NULL && pPage != NULL)
/*N*/ 		pModel = pPage->GetModel();
/*N*/ 	SdrObject* pObj = NULL;
/*N*/
/*N*/ 	if(nInvent == SdrInventor)
/*N*/ 	{
/*N*/ 		switch (nIdent)
/*N*/ 		{
/*N*/ 			case USHORT(OBJ_NONE       ): pObj=new SdrObject;                   break;
/*N*/ 			case USHORT(OBJ_GRUP       ): pObj=new SdrObjGroup;                 break;
/*N*/ 			case USHORT(OBJ_LINE       ): pObj=new SdrPathObj(OBJ_LINE       ); break;
/*N*/ 			case USHORT(OBJ_POLY       ): pObj=new SdrPathObj(OBJ_POLY       ); break;
/*N*/ 			case USHORT(OBJ_PLIN       ): pObj=new SdrPathObj(OBJ_PLIN       ); break;
/*N*/ 			case USHORT(OBJ_PATHLINE   ): pObj=new SdrPathObj(OBJ_PATHLINE   ); break;
/*N*/ 			case USHORT(OBJ_PATHFILL   ): pObj=new SdrPathObj(OBJ_PATHFILL   ); break;
/*N*/ 			case USHORT(OBJ_FREELINE   ): pObj=new SdrPathObj(OBJ_FREELINE   ); break;
/*N*/ 			case USHORT(OBJ_FREEFILL   ): pObj=new SdrPathObj(OBJ_FREEFILL   ); break;
/*?*/ 			case USHORT(OBJ_PATHPOLY   ): pObj=new SdrPathObj(OBJ_POLY       ); break;
/*?*/ 			case USHORT(OBJ_PATHPLIN   ): pObj=new SdrPathObj(OBJ_PLIN       ); break;
/*N*/ 			case USHORT(OBJ_EDGE       ): pObj=new SdrEdgeObj;                  break;
/*N*/ 			case USHORT(OBJ_RECT       ): pObj=new SdrRectObj;                  break;
/*N*/ 			case USHORT(OBJ_CIRC       ): pObj=new SdrCircObj(OBJ_CIRC       ); break;
/*N*/ 			case USHORT(OBJ_SECT       ): pObj=new SdrCircObj(OBJ_SECT       ); break;
/*N*/ 			case USHORT(OBJ_CARC       ): pObj=new SdrCircObj(OBJ_CARC       ); break;
/*N*/ 			case USHORT(OBJ_CCUT       ): pObj=new SdrCircObj(OBJ_CCUT       ); break;
/*N*/ 			case USHORT(OBJ_TEXT       ): pObj=new SdrRectObj(OBJ_TEXT       ); break;
/*?*/ 			case USHORT(OBJ_TEXTEXT    ): pObj=new SdrRectObj(OBJ_TEXTEXT    ); break;
/*N*/ 			case USHORT(OBJ_TITLETEXT  ): pObj=new SdrRectObj(OBJ_TITLETEXT  ); break;
/*N*/ 			case USHORT(OBJ_OUTLINETEXT): pObj=new SdrRectObj(OBJ_OUTLINETEXT); break;
/*N*/ 			case USHORT(OBJ_MEASURE    ): pObj=new SdrMeasureObj;               break;
/*N*/ 			case USHORT(OBJ_GRAF       ): pObj=new SdrGrafObj;                  break;
/*N*/ 			case USHORT(OBJ_OLE2       ): pObj=new SdrOle2Obj;                  break;
/*?*/ 			case USHORT(OBJ_FRAME      ): pObj=new SdrOle2Obj(TRUE);            break;
/*N*/ 			case USHORT(OBJ_CAPTION    ): pObj=new SdrCaptionObj;               break;
/*N*/ 			case USHORT(OBJ_PAGE       ): pObj=new SdrPageObj;                  break;
/*?*/ 			case USHORT(OBJ_UNO        ): pObj=new SdrUnoObj(String());    break;
/*N*/ 		}
/*N*/ 	}

/*N*/ 	if(pObj == NULL)
/*N*/ 	{
/*N*/ 		SdrObjFactory* pFact=new SdrObjFactory(nInvent,nIdent,pPage,pModel);
/*N*/ 		SdrLinkList& rLL=ImpGetUserMakeObjHdl();
/*N*/ 		unsigned nAnz=rLL.GetLinkCount();
/*N*/ 		unsigned i=0;
/*N*/ 		while (i<nAnz && pObj==NULL) {
/*N*/ 			rLL.GetLink(i).Call((void*)pFact);
/*N*/ 			pObj=pFact->pNewObj;
/*N*/ 			i++;
/*N*/ 		}
/*N*/ 		delete pFact;
/*N*/ 	}
/*N*/
/*N*/ 	if(pObj == NULL)
/*N*/ 	{
/*N*/ 		// Na wenn's denn keiner will ...
/*N*/ 	}
/*N*/
/*N*/ 	if(pObj != NULL)
/*N*/ 	{
/*N*/ 		if(pPage != NULL)
/*N*/ 			pObj->SetPage(pPage);
/*N*/ 		else if(pModel != NULL)
/*?*/ 			pObj->SetModel(pModel);
/*N*/ 	}
/*N*/ 
/*N*/ 	return pObj;
/*N*/ }

/*N*/ SdrObjUserData* SdrObjFactory::MakeNewObjUserData(UINT32 nInvent, UINT16 nIdent, SdrObject* pObj1)
/*N*/ {
/*N*/ 	SdrObjUserData* pData=NULL;
/*N*/ 	if (nInvent==SdrInventor) {
/*N*/ 		switch (nIdent) {
/*?*/ 			case USHORT(SDRUSERDATA_OBJGROUPLINK): pData=new ImpSdrObjGroupLinkUserData(pObj1); break;
/*?*/ 			case USHORT(SDRUSERDATA_OBJTEXTLINK) : pData=new ImpSdrObjTextLinkUserData((SdrTextObj*)pObj1); break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if (pData==NULL) {
/*N*/ 		SdrObjFactory aFact(nInvent,nIdent,pObj1);
/*N*/ 		SdrLinkList& rLL=ImpGetUserMakeObjUserDataHdl();
/*N*/ 		unsigned nAnz=rLL.GetLinkCount();
/*N*/ 		unsigned i=0;
/*N*/ 		while (i<nAnz && pData==NULL) {
/*N*/ 			rLL.GetLink(i).Call((void*)&aFact);
/*N*/ 			pData=aFact.pNewData;
/*N*/ 			i++;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pData;
/*N*/ }

/*N*/ void SdrObjFactory::InsertMakeObjectHdl(const Link& rLink)
/*N*/ {
/*N*/ 	SdrLinkList& rLL=ImpGetUserMakeObjHdl();
/*N*/ 	rLL.InsertLink(rLink);
/*N*/ }

/*N*/ void SdrObjFactory::RemoveMakeObjectHdl(const Link& rLink)
/*N*/ {
/*N*/ 	SdrLinkList& rLL=ImpGetUserMakeObjHdl();
/*N*/ 	rLL.RemoveLink(rLink);
/*N*/ }

/*N*/ void SdrObjFactory::InsertMakeUserDataHdl(const Link& rLink)
/*N*/ {
/*N*/ 	SdrLinkList& rLL=ImpGetUserMakeObjUserDataHdl();
/*N*/ 	rLL.InsertLink(rLink);
/*N*/ }

/*N*/ void SdrObjFactory::RemoveMakeUserDataHdl(const Link& rLink)
/*N*/ {
/*N*/ 	SdrLinkList& rLL=ImpGetUserMakeObjUserDataHdl();
/*N*/ 	rLL.RemoveLink(rLink);
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
