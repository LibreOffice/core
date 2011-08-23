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

#include "svdmrkv.hxx"
namespace binfilter {

/*N*/ BOOL SdrMarkView::HasMarkedPoints() const
/*N*/ {
/*N*/ 	ForceUndirtyMrkPnt();
/*N*/ 	BOOL bRet=FALSE;
/*N*/ 	if (!ImpIsFrameHandles()) {
/*N*/ 		ULONG nMarkAnz=aMark.GetMarkCount();
/*N*/ 		if (nMarkAnz<=nFrameHandlesLimit) {
/*N*/ 			for (ULONG nMarkNum=0; nMarkNum<nMarkAnz && !bRet; nMarkNum++) {
/*?*/ 				const SdrMark* pM=aMark.GetMark(nMarkNum);
/*?*/ 				const SdrUShortCont* pPts=pM->GetMarkedPoints();
/*?*/ 				bRet=pPts!=NULL && pPts->GetCount()!=0;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

/*N*/ void SdrMarkView::SetPlusHandlesAlwaysVisible(BOOL bOn)
/*N*/ { // HandlePaint optimieren !!!!!!!
/*N*/ 	ForceUndirtyMrkPnt();
/*N*/ 	if (bOn!=bPlusHdlAlways) {
/*?*/ 		BOOL bVis=IsMarkHdlShown();
/*?*/ 		if (bVis) HideMarkHdl(NULL);
/*?*/ 		bPlusHdlAlways=bOn;
/*?*/ 		SetMarkHandles();
/*?*/ 		if (bVis) ShowMarkHdl(NULL);
/*?*/ 		MarkListHasChanged();
/*N*/ 	}
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// ImpSetPointsRects() ist fuer PolyPoints und GluePoints!
////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////
// UndirtyMrkPnt() ist fuer PolyPoints und GluePoints!
////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrMarkView::UndirtyMrkPnt() const
/*N*/ {
/*N*/ 	BOOL bChg=FALSE;
/*N*/ 	ULONG nMarkAnz=aMark.GetMarkCount();
/*N*/ 	for (ULONG nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++) {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	}
/*N*/ 	if (bChg) ((SdrMarkView*)this)->bMarkedPointsRectsDirty=TRUE;
/*N*/ 	((SdrMarkView*)this)->bMrkPntDirty=FALSE;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////



/*N*/ BOOL SdrMarkView::HasMarkedGluePoints() const
/*N*/ {
/*N*/ 	ForceUndirtyMrkPnt();
/*N*/ 	BOOL bRet=FALSE;
/*N*/ 	ULONG nMarkAnz=aMark.GetMarkCount();
/*N*/ 	for (ULONG nMarkNum=0; nMarkNum<nMarkAnz && !bRet; nMarkNum++) {
/*?*/ 		const SdrMark* pM=aMark.GetMark(nMarkNum);
/*?*/ 		const SdrUShortCont* pPts=pM->GetMarkedGluePoints();
/*?*/ 		bRet=pPts!=NULL && pPts->GetCount()!=0;
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }


/*N*/ BOOL SdrMarkView::MarkGluePoints(const Rectangle* pRect, BOOL bUnmark)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 
/*N*/ }

}
