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

#include "svdpoev.hxx"
#include <math.h>
#include "svdopath.hxx"
#include "svdstr.hrc"   // Namen aus der Resource
namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrPolyEditView::ImpResetPolyPossibilityFlags()
/*N*/ {
/*N*/ 	eMarkedPointsSmooth=SDRPATHSMOOTH_DONTCARE;
/*N*/ 	eMarkedSegmentsKind=SDRPATHSEGMENT_DONTCARE;
/*N*/ 	bSetMarkedPointsSmoothPossible=FALSE;
/*N*/ 	bSetMarkedSegmentsKindPossible=FALSE;
/*N*/ }

/*N*/ void SdrPolyEditView::ImpClearVars()
/*N*/ {
/*N*/ 	ImpResetPolyPossibilityFlags();
/*N*/ }

/*N*/ SdrPolyEditView::SdrPolyEditView(SdrModel* pModel1, OutputDevice* pOut):
/*N*/ 	SdrEditView(pModel1,pOut)
/*N*/ {
/*N*/ 	ImpClearVars();
/*N*/ }

/*N*/ SdrPolyEditView::~SdrPolyEditView()
/*N*/ {
/*N*/ }

/*N*/ void SdrPolyEditView::ImpCheckPolyPossibilities()
/*N*/ {
/*N*/ 	ImpResetPolyPossibilityFlags();
/*N*/ 	ULONG nMarkAnz=aMark.GetMarkCount();
/*N*/ 	if (nMarkAnz>0 && !ImpIsFrameHandles()) {
/*?*/ 		BOOL bReadOnly=FALSE;
/*?*/ 		BOOL b1stSmooth=TRUE;
/*?*/ 		BOOL b1stSegm=TRUE;
/*?*/ 		BOOL bCurve=FALSE;
/*?*/ 		BOOL bSmoothFuz=FALSE;
/*?*/ 		BOOL bSegmFuz=FALSE;
/*?*/ 		XPolyFlags eSmooth=XPOLY_NORMAL;
/*?*/ 
/*?*/ 		for (ULONG nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++) {
/*?*/ 			SdrMark* pM=aMark.GetMark(nMarkNum);
/*?*/ 			SdrObject* pObj=pM->GetObj();
/*?*/ 			SdrUShortCont* pPts=pM->GetMarkedPoints();
/*?*/ 			SdrPathObj* pPath=PTR_CAST(SdrPathObj,pObj);
/*?*/ 			if (pPath!=NULL && pPts!=NULL) {
/*?*/ 				ULONG nMarkedPntAnz=pPts->GetCount();
/*?*/ 				if (nMarkedPntAnz!=0) {
/*?*/ 					BOOL bClosed=pPath->IsClosed();
/*?*/ 					bSetMarkedPointsSmoothPossible=TRUE;
/*?*/ 					if (bClosed) bSetMarkedSegmentsKindPossible=TRUE;
/*?*/ 					const XPolyPolygon& rXPP=pPath->GetPathPoly();
/*?*/ 					for (USHORT nMarkedPntNum=0; nMarkedPntNum<(USHORT)nMarkedPntAnz; nMarkedPntNum++) {
/*?*/ 						USHORT nNum=pPts->GetObject(nMarkedPntNum);
/*?*/ 						USHORT nPolyNum=0,nPntNum=0;
/*?*/ 						if (pPath->FindPolyPnt(nNum,nPolyNum,nPntNum,FALSE)) {
/*?*/ 							const XPolygon& rXP=rXPP[nPolyNum];
/*?*/ 							BOOL bCanSegment=bClosed || nPntNum<rXP.GetPointCount()-1;
/*?*/ 
/*?*/ 							if (!bSetMarkedSegmentsKindPossible && bCanSegment) {
/*?*/ 								bSetMarkedSegmentsKindPossible=TRUE;
/*?*/ 							}
/*?*/ 							if (!bSmoothFuz) {
/*?*/ 								if (b1stSmooth) {
/*?*/ 									b1stSmooth=FALSE;
/*?*/ 									eSmooth=rXP.GetFlags(nPntNum);
/*?*/ 								} else {
/*?*/ 									bSmoothFuz=eSmooth!=rXP.GetFlags(nPntNum);
/*?*/ 								}
/*?*/ 							}
/*?*/ 							if (!bSegmFuz) {
/*?*/ 								if (bCanSegment) {
/*?*/ 									BOOL bCrv=rXP.IsControl(nPntNum+1);
/*?*/ 									if (b1stSegm) {
/*?*/ 										b1stSegm=FALSE;
/*?*/ 										bCurve=bCrv;
/*?*/ 									} else {
/*?*/ 										bSegmFuz=bCrv!=bCurve;
/*?*/ 									}
/*?*/ 								}
/*?*/ 							}
/*?*/ 						}
/*?*/ 					}
/*?*/ 					if (!b1stSmooth && !bSmoothFuz) {
/*?*/ 						if (eSmooth==XPOLY_NORMAL) eMarkedPointsSmooth=SDRPATHSMOOTH_ANGULAR;
/*?*/ 						if (eSmooth==XPOLY_SMOOTH) eMarkedPointsSmooth=SDRPATHSMOOTH_ASYMMETRIC;
/*?*/ 						if (eSmooth==XPOLY_SYMMTR) eMarkedPointsSmooth=SDRPATHSMOOTH_SYMMETRIC;
/*?*/ 					}
/*?*/ 					if (!b1stSegm && !bSegmFuz) {
/*?*/ 						eMarkedSegmentsKind= bCurve ? SDRPATHSEGMENT_CURVE : SDRPATHSEGMENT_LINE;
/*?*/ 					}
/*?*/ 				}
/*?*/ 			}
/*?*/ 		}
/*?*/ 		if (bReadOnly) {
/*?*/ 			bSetMarkedPointsSmoothPossible=FALSE;
/*?*/ 			bSetMarkedSegmentsKindPossible=FALSE;
/*?*/ 		}
/*N*/ 	}
/*N*/ }










////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
