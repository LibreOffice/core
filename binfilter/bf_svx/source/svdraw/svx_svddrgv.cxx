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

#include "svddrgv.hxx"
#include "svdio.hxx"
#include "svdstr.hrc"   // Namen aus der Resource
namespace binfilter {

#define XOR_DRAG_PEN   PEN_DOT

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@@  @@@@@   @@@@   @@@@   @@ @@ @@ @@@@@ @@   @@
//  @@  @@ @@  @@ @@  @@ @@  @@  @@ @@ @@ @@    @@   @@
//  @@  @@ @@  @@ @@  @@ @@      @@ @@ @@ @@    @@ @ @@
//  @@  @@ @@@@@  @@@@@@ @@ @@@  @@@@@ @@ @@@@  @@@@@@@
//  @@  @@ @@  @@ @@  @@ @@  @@   @@@  @@ @@    @@@@@@@
//  @@  @@ @@  @@ @@  @@ @@  @@   @@@  @@ @@    @@@ @@@
//  @@@@@  @@  @@ @@  @@  @@@@@    @   @@ @@@@@ @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrDragView::ImpClearVars()
/*N*/ {
/*N*/ 	bFramDrag=FALSE;
/*N*/ 	eDragMode=SDRDRAG_MOVE;
/*N*/ 	bDragLimit=FALSE;
/*N*/ 	bMarkedHitMovesAlways=FALSE;
/*N*/ 	eDragHdl=HDL_MOVE;
/*N*/ 	pDragHdl=NULL;
/*N*/ 	bDragHdl=FALSE;
/*N*/ 	bDragSpecial=FALSE;
/*N*/ 	pDragBla=NULL;
/*N*/ 	bDragStripes=FALSE;
/*N*/ 	bNoDragHdl=TRUE;
/*N*/ 	bMirrRefDragObj=TRUE;
/*N*/ 	bSolidDragging=FALSE;
/*N*/ 	bSolidDrgNow=FALSE;
/*N*/ 	bSolidDrgChk=FALSE;
/*N*/ 	bDragWithCopy=FALSE;
/*N*/ 	pInsPointUndo=NULL;
/*N*/ 	bInsAfter=FALSE;
/*N*/ 	bInsGluePoint=FALSE;
/*N*/ 	bInsObjPointMode=FALSE;
/*N*/ 	bInsGluePointMode=FALSE;
/*N*/ 	nDragXorPolyLimit=100;
/*N*/ 	nDragXorPointLimit=500;
/*N*/ 	bNoDragXorPolys=FALSE;
/*N*/ 	bAutoVertexCon=TRUE;
/*N*/ 	bAutoCornerCon=FALSE;
/*N*/ 	bRubberEdgeDragging=TRUE;
/*N*/ 	nRubberEdgeDraggingLimit=100;
/*N*/ 	bDetailedEdgeDragging=TRUE;
/*N*/ 	nDetailedEdgeDraggingLimit=10;
/*N*/ 	bResizeAtCenter=FALSE;
/*N*/ 	bCrookAtCenter=FALSE;
/*N*/ 	bMouseHideWhileDraggingPoints=FALSE;
/*N*/ }

/*N*/ void SdrDragView::ImpMakeDragAttr()
/*N*/ {
/*N*/ 	ImpDelDragAttr();
/*N*/ }

/*N*/ SdrDragView::SdrDragView(SdrModel* pModel1, OutputDevice* pOut):
/*N*/ 	SdrExchangeView(pModel1,pOut)
/*N*/ {
/*N*/ 	ImpClearVars();
/*N*/ 	ImpMakeDragAttr();
/*N*/ }

/*N*/ SdrDragView::~SdrDragView()
/*N*/ {
/*N*/ 	ImpDelDragAttr();
/*N*/ }

/*N*/ void SdrDragView::ImpDelDragAttr()
/*N*/ {
/*N*/ }

/*N*/ BOOL SdrDragView::IsAction() const
/*N*/ {
/*N*/ 	return SdrExchangeView::IsAction() || pDragBla!=NULL;
/*N*/ }




/*N*/ void SdrDragView::BrkAction()
/*N*/ {
/*N*/ 	SdrExchangeView::BrkAction();
/*N*/ 	BrkDragObj();
//STRIP012/*N*/ 	RefreshAllIAOManagers();
/*N*/ }


/*N*/ void SdrDragView::ToggleShownXor(OutputDevice* pOut, const Region* pRegion) const
/*N*/ {
/*N*/ 	SdrExchangeView::ToggleShownXor(pOut,pRegion);
    if (pDragBla!=NULL && aDragStat.IsShown() &&
    1 )//STRIP001 //STRIP001 		(!IS_TYPE(SdrDragMovHdl,pDragBla)) { // das ist ein Hack !!!!!!!!!!
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 DrawDragObj(pOut,TRUE);
/*N*/ }



////////////////////////////////////////////////////////////////////////////////////////////////////


/*N*/ void SdrDragView::BrkDragObj()
/*N*/ {
/*N*/ 	if (pDragBla!=NULL) {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	}
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrDragView::SetNoDragXorPolys(BOOL bOn)
/*N*/ {
/*N*/ 	if (IsNoDragXorPolys()!=bOn) {
/*N*/ 		BOOL bDragging=pDragBla!=NULL;
/*N*/ 		BOOL bShown=bDragging && aDragStat.IsShown();
/*N*/ 		if (bShown) {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 HideDragObj(pDragWin);
/*N*/ 		bNoDragXorPolys=bOn;
/*N*/ 		if (bDragging) {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 		}
/*N*/ 		if (bShown) {DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 ShowDragObj(pDragWin);
/*N*/ 	}
/*N*/ }

/*N*/ void SdrDragView::SetDragStripes(BOOL bOn)
/*N*/ {
/*N*/ 	if (pDragBla!=NULL && aDragStat.IsShown()) {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	} else {
/*N*/ 		bDragStripes=bOn;
/*N*/ 	}
/*N*/ }



////////////////////////////////////////////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrDragView::WriteRecords(SvStream& rOut) const
/*N*/ {
/*N*/ 	SdrExchangeView::WriteRecords(rOut);
/*N*/ 	{
/*N*/ 		SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWDRAGSTRIPES);
/*N*/ 		rOut<<(BOOL)bDragStripes;
/*N*/ 	} {
/*N*/ 		SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWDRAGHIDEHDL);
/*N*/ 		rOut<<(BOOL)bNoDragHdl;
/*N*/ 	} {
/*N*/ 		SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWOBJHITMOVES);
/*N*/ 		rOut<<(BOOL)bMarkedHitMovesAlways;
/*N*/ 	} {
/*N*/ 		SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWMIRRDRAGOBJ);
/*N*/ 		rOut<<(BOOL)bMirrRefDragObj;
/*N*/ 	}
/*N*/ }

/*N*/ BOOL SdrDragView::ReadRecord(const SdrIOHeader& rViewHead,
/*N*/ 	const SdrNamedSubRecord& rSubHead,
/*N*/ 	SvStream& rIn)
/*N*/ {
/*N*/ 	BOOL bRet=FALSE;
/*N*/ 	if (rSubHead.GetInventor()==SdrInventor) {
/*N*/ 		bRet=TRUE;
/*N*/ 		switch (rSubHead.GetIdentifier()) {
/*N*/ 			case SDRIORECNAME_VIEWDRAGSTRIPES: {
/*N*/ 				BOOL bZwi; rIn >> bZwi; bDragStripes = bZwi;
/*N*/ 			} break;
/*N*/ 			case SDRIORECNAME_VIEWDRAGHIDEHDL: {
/*N*/ 				BOOL bZwi; rIn >> bZwi; bNoDragHdl = bZwi;
/*N*/ 			} break;
/*N*/ 			case SDRIORECNAME_VIEWOBJHITMOVES: {
/*N*/ 				BOOL bZwi; rIn >> bZwi; bMarkedHitMovesAlways = bZwi;
/*N*/ 			} break;
/*N*/ 			case SDRIORECNAME_VIEWMIRRDRAGOBJ: {
/*N*/ 				BOOL bZwi; rIn >> bZwi; bMirrRefDragObj = bZwi;
/*N*/ 			} break;
/*N*/ 			default: bRet=FALSE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if (!bRet) bRet=SdrExchangeView::ReadRecord(rViewHead,rSubHead,rIn);
/*N*/ 	return bRet;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
