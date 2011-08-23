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

#include "svdvmark.hxx"
#include "svdobj.hxx"
#include "svdpntv.hxx"

namespace binfilter {

/*N*/ SdrViewUserMarker::SdrViewUserMarker(SdrPaintView* pView_)
/*N*/ :	pView(pView_),
/*N*/ 	pForcedOutDev(NULL),
/*N*/ 	pPoint(NULL),
/*N*/ 	pRect(NULL),
/*N*/ 	pPoly(NULL),
/*N*/ 	pPolyPoly(NULL),
/*N*/ 	pXPoly(NULL),
/*N*/ 	pXPolyPoly(NULL),
/*N*/ 	eAlign(SDRMARKER_ALIGNCENTER),
/*N*/ 	nPixelDistance(0),
/*N*/ 	nLineWdt(0),
/*N*/ 	nCrossSize(0),
/*N*/ 	bLineWdtLog(FALSE),
/*N*/ 	bCrossSizeLog(FALSE),
/*N*/ 	bSolidArea(FALSE),
/*N*/ 	bDashed(FALSE),
/*N*/ 	bCrossHair(FALSE),
/*N*/ 	bStripes(FALSE),
/*N*/ 	bEllipse(FALSE),
/*N*/ 	bPolyLine(FALSE),
/*N*/ 	bAnimate(FALSE),
/*N*/ 	bVisible(FALSE),
/*N*/ 	nAnimateDelay(0),
/*N*/ 	nAnimateSpeed(0),
/*N*/ 	nAnimateAnz(0),
/*N*/ 	bAnimateBwd(FALSE),
/*N*/ 	bAnimateToggle(FALSE),
/*N*/ 	nAnimateDelayCountDown(0),
/*N*/ 	nAnimateSpeedCountDown(0),
/*N*/ 	nAnimateNum(0),
/*N*/ 	bHasPointer(FALSE),
/*N*/ 	bMouseMovable(FALSE)
/*N*/ {
/*N*/ 	if (pView!=NULL) pView->ImpInsertUserMarker(this);
/*N*/ 	bAnimateToggle=TRUE;
/*N*/ }
/*N*/ 
/*N*/ SdrViewUserMarker::~SdrViewUserMarker()
/*N*/ {
/*N*/ 	if (bVisible) {DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 Hide();
/*N*/ 	if (pView!=NULL) pView->ImpRemoveUserMarker(this);
/*N*/ 	ImpDelGeometrics();
/*N*/ }
/*N*/ 
/*N*/ void SdrViewUserMarker::ImpDelGeometrics()
/*N*/ {
/*N*/ 	if (pPoint    !=NULL) { delete pPoint    ; pPoint    =NULL; }
/*N*/ 	if (pRect     !=NULL) { delete pRect     ; pRect     =NULL; }
/*N*/ 	if (pPoly     !=NULL) { delete pPoly     ; pPoly     =NULL; }
/*N*/ 	if (pPolyPoly !=NULL) { delete pPolyPoly ; pPolyPoly =NULL; }
/*N*/ 	if (pXPoly    !=NULL) { delete pXPoly    ; pXPoly    =NULL; }
/*N*/ 	if (pXPolyPoly!=NULL) { delete pXPolyPoly; pXPolyPoly=NULL; }
/*N*/ }

/*N*/ void SdrViewUserMarker::SetLineWidth(USHORT nWdt)
/*N*/ {
/*N*/ 	FASTBOOL bVis=bVisible;
/*N*/ 	if (bVis) Hide();
/*N*/ 	nLineWdt=nWdt;
/*N*/ 	if (bVis) Show();
/*N*/ }

/*N*/ void SdrViewUserMarker::Show()
/*N*/ {DBG_BF_ASSERT(0, "STRIP");
/*N*/ }

/*N*/ void SdrViewUserMarker::Hide()
/*N*/ {DBG_BF_ASSERT(0, "STRIP");
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
