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

#include "svdpoev.hxx"  // fuer die PolyPossiblities
#include "svdstr.hrc"   // Namen aus der Resource

// #i13033#
namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@@ @@@@@  @@ @@@@@@  @@ @@ @@ @@@@@ @@   @@
//  @@    @@  @@ @@   @@    @@ @@ @@ @@    @@   @@
//  @@    @@  @@ @@   @@    @@ @@ @@ @@    @@ @ @@
//  @@@@  @@  @@ @@   @@    @@@@@ @@ @@@@  @@@@@@@
//  @@    @@  @@ @@   @@     @@@  @@ @@    @@@@@@@
//  @@    @@  @@ @@   @@     @@@  @@ @@    @@@ @@@
//  @@@@@ @@@@@  @@   @@      @   @@ @@@@@ @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrEditView::ImpResetPossibilityFlags()
/*N*/ {
/*N*/ 	bReadOnly               =FALSE;
/*N*/ 
/*N*/ 	bGroupPossible          =FALSE;
/*N*/ 	bUnGroupPossible        =FALSE;
/*N*/ 	bGrpEnterPossible       =FALSE;
/*N*/ 	bDeletePossible         =FALSE;
/*N*/ 	bToTopPossible          =FALSE;
/*N*/ 	bToBtmPossible          =FALSE;
/*N*/ 	bReverseOrderPossible   =FALSE;
/*N*/ 
/*N*/ 	bImportMtfPossible      =FALSE;
/*N*/ 	bCombinePossible        =FALSE;
/*N*/ 	bDismantlePossible      =FALSE;
/*N*/ 	bCombineNoPolyPolyPossible =FALSE;
/*N*/ 	bDismantleMakeLinesPossible=FALSE;
/*N*/ 	bOrthoDesiredOnMarked   =FALSE;
/*N*/ 
/*N*/ 	bMoreThanOneNotMovable  =FALSE;
/*N*/ 	bOneOrMoreMovable       =FALSE;
/*N*/ 	bMoreThanOneNoMovRot    =FALSE;
/*N*/ 	bContortionPossible     =FALSE;
/*N*/ 	bAllPolys               =FALSE;
/*N*/ 	bOneOrMorePolys         =FALSE;
/*N*/ 	bMoveAllowed            =FALSE;
/*N*/ 	bResizeFreeAllowed      =FALSE;
/*N*/ 	bResizePropAllowed      =FALSE;
/*N*/ 	bRotateFreeAllowed      =FALSE;
/*N*/ 	bRotate90Allowed        =FALSE;
/*N*/ 	bMirrorFreeAllowed      =FALSE;
/*N*/ 	bMirror45Allowed        =FALSE;
/*N*/ 	bMirror90Allowed        =FALSE;
/*N*/ 	bTransparenceAllowed	=FALSE;
/*N*/ 	bGradientAllowed		=FALSE;
/*N*/ 	bShearAllowed           =FALSE;
/*N*/ 	bEdgeRadiusAllowed		=FALSE;
/*N*/ 	bCanConvToPath          =FALSE;
/*N*/ 	bCanConvToPoly          =FALSE;
/*N*/ 	bCanConvToContour		=FALSE;
/*N*/ 	bCanConvToPathLineToArea=FALSE;
/*N*/ 	bCanConvToPolyLineToArea=FALSE;
/*N*/ 	bMoveProtect            =FALSE;
/*N*/ 	bResizeProtect          =FALSE;
/*N*/ }

/*N*/ void SdrEditView::ImpClearVars()
/*N*/ {
/*N*/ 	ImpResetPossibilityFlags();
/*N*/ 	bPossibilitiesDirty=TRUE;   // << war von Purify angemeckert
/*N*/ 	bCombineError=FALSE;
/*N*/ 	bBundleVirtObj=FALSE;
/*N*/ }

/*N*/ SdrEditView::SdrEditView(SdrModel* pModel1, OutputDevice* pOut):
/*N*/ 	SdrMarkView(pModel1,pOut)
/*N*/ {
/*N*/ 	ImpClearVars();
/*N*/ }

/*N*/ SdrEditView::~SdrEditView()
/*N*/ {
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace binfilter
namespace binfilter {//STRIP009

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ####   ###   ####  #### # ####  # #    # ##### # #####  ####
//  #   # #   # #     #     # #   # # #    #   #   # #     #
//  ####  #   #  ###   ###  # ####  # #    #   #   # ####   ###
//  #     #   #     #     # # #   # # #    #   #   # #         #
//  #      ###  ####  ####  # ####  # #### #   #   # ##### ####
//
////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrEditView::MarkListHasChanged()
/*N*/ {
/*N*/ 	SdrMarkView::MarkListHasChanged();
/*N*/ 	bPossibilitiesDirty=TRUE;
/*N*/ }

/*N*/ void SdrEditView::ModelHasChanged()
/*N*/ {
/*N*/ 	SdrMarkView::ModelHasChanged();
/*N*/ 	bPossibilitiesDirty=TRUE;
/*N*/ }

/*N*/ void SdrEditView::CheckPossibilities()
/*N*/ {
/*N*/ 	if (bSomeObjChgdFlag) bPossibilitiesDirty=TRUE;
/*N*/ 	
/*N*/ 	if(bSomeObjChgdFlag) 
/*N*/ 	{
/*N*/ 		// This call IS necessary to correct the MarkList, in which
/*N*/ 		// no longer to the model belonging objects still can reside.
/*N*/ 		// These ones nned to be removed.
/*N*/ 		CheckMarked();
/*N*/ 	}
/*N*/ 	
/*N*/ 	if (bPossibilitiesDirty) {
/*N*/ 		ImpResetPossibilityFlags();
/*N*/ 		aMark.ForceSort();
/*N*/ 		ULONG nMarkAnz=aMark.GetMarkCount();
/*N*/ 		if (nMarkAnz!=0) {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 		}
/*N*/ 		ImpCheckToTopBtmPossible();
/*N*/ 		((SdrPolyEditView*)this)->ImpCheckPolyPossibilities();
/*N*/ 		bPossibilitiesDirty=FALSE;
/*N*/ 
/*N*/ 		if (bReadOnly) {
/*?*/ 			BOOL bMerker1=bGrpEnterPossible;
/*?*/ 			ImpResetPossibilityFlags();
/*?*/ 			bReadOnly=TRUE;
/*?*/ 			bGrpEnterPossible=bMerker1;
/*N*/ 		}
/*N*/ 		if (bMoveAllowed) {
/*?*/ 			// Verschieben von angeklebten Verbindern unterbinden
/*?*/ 			// Derzeit nur fuer Einfachselektion implementiert.
/*?*/ 			if (nMarkAnz==1) {
/*?*/ 				SdrObject* pObj=aMark.GetMark(0)->GetObj();
/*?*/ 				SdrEdgeObj* pEdge=PTR_CAST(SdrEdgeObj,pObj);
/*?*/ 				if (pEdge!=NULL) {
/*?*/ 					SdrObject* pNode1=pEdge->GetConnectedNode(TRUE);
/*?*/ 					SdrObject* pNode2=pEdge->GetConnectedNode(FALSE);
/*?*/ 					if (pNode1!=NULL || pNode2!=NULL) bMoveAllowed=FALSE;
/*?*/ 				}
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef WIN




    // nur nach dem 1. BegUndo oder vor dem letzten EndUndo:

#endif

}
