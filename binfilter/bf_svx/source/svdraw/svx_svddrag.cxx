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

#include "svddrag.hxx"
namespace binfilter {

/*N*/ void SdrDragStat::Clear(FASTBOOL bLeaveOne)
/*N*/ {
/*N*/ 	void* pP=aPnts.First();
/*N*/ 	while (pP!=NULL) {
/*N*/ 		delete (Point*)pP;
/*N*/ 		pP=aPnts.Next();
/*N*/ 	}
/*N*/ 	if (pUser!=NULL) delete pUser;
/*N*/ 	pUser=NULL;
/*N*/ 	aPnts.Clear();
/*N*/ 	if (bLeaveOne) {
/*N*/ 		aPnts.Insert(new Point,CONTAINER_APPEND);
/*N*/ 	}
/*N*/ }

/*N*/ void SdrDragStat::Reset()
/*N*/ {
/*N*/ 	pView=NULL;
/*N*/ 	pPageView=NULL;
/*N*/ 	bShown=FALSE;
/*N*/ 	nMinMov=1;
/*N*/ 	bMinMoved=FALSE;
/*N*/ 	bHorFixed=FALSE;
/*N*/ 	bVerFixed=FALSE;
/*N*/ 	bWantNoSnap=FALSE;
/*N*/ 	pHdl=NULL;
/*N*/ 	bOrtho4=FALSE;
/*N*/ 	bOrtho8=FALSE;
/*N*/ 	pDragMethod=NULL;
/*N*/ 	bEndDragChangesAttributes=FALSE;
/*N*/ 	bEndDragChangesGeoAndAttributes=FALSE;
/*N*/ 	bMouseIsUp=FALSE;
/*N*/ 	Clear(TRUE);
/*N*/ 	aActionRect=Rectangle();
/*N*/ }










}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
