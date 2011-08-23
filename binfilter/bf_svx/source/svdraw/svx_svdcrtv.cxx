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

#include "svdcrtv.hxx"
#include "svdio.hxx"
#include "svdvmark.hxx"


namespace binfilter {

#define XOR_CREATE_PEN			PEN_SOLID

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ class ImpSdrConnectMarker: public SdrViewUserMarker
/*N*/ {
/*N*/ 	const SdrObject*			pAktObj;
/*N*/ 	const SdrPageView*			pAktPV;
/*N*/ 

/*N*/ public:
/*N*/ 	ImpSdrConnectMarker(SdrCreateView* pView): SdrViewUserMarker(pView),pAktObj(NULL),pAktPV(NULL) {}
/*N*/ 	~ImpSdrConnectMarker() {}
/*N*/ 	void SetTargetObject(const SdrObject* pObj);
/*N*/ }; // svdvmark


/*N*/ void ImpSdrConnectMarker::SetTargetObject(const SdrObject* pObj)
/*N*/ {
/*N*/ 	if (pAktObj!=pObj) {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	}
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   @@@@  @@@@@  @@@@@  @@@@  @@@@@@ @@@@@  @@ @@ @@ @@@@@ @@   @@
//  @@  @@ @@  @@ @@    @@  @@   @@   @@     @@ @@ @@ @@    @@   @@
//  @@     @@  @@ @@    @@  @@   @@   @@     @@ @@ @@ @@    @@ @ @@
//  @@     @@@@@  @@@@  @@@@@@   @@   @@@@   @@@@@ @@ @@@@  @@@@@@@
//  @@     @@  @@ @@    @@  @@   @@   @@      @@@  @@ @@    @@@@@@@
//  @@  @@ @@  @@ @@    @@  @@   @@   @@      @@@  @@ @@    @@@ @@@
//   @@@@  @@  @@ @@@@@ @@  @@   @@   @@@@@    @   @@ @@@@@ @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrCreateView::ImpClearVars()
/*N*/ {
/*N*/ 	nAktInvent=SdrInventor;
/*N*/ 	nAktIdent=OBJ_NONE;
/*N*/ 	pCurrentLibObj=NULL;
/*N*/ 	bCurrentLibObjMoveNoResize=FALSE;
/*N*/ 	bCurrentLibObjSetDefAttr=FALSE;
/*N*/ 	bCurrentLibObjSetDefLayer=FALSE;
/*N*/ 	pLibObjDragMeth=NULL;
/*N*/ 	pAktCreate=NULL;
/*N*/ 	pCreatePV=NULL;
/*N*/ 	bAutoTextEdit=FALSE;
/*N*/ 	b1stPointAsCenter=FALSE;
/*N*/ 	aAktCreatePointer=Pointer(POINTER_CROSS);
/*N*/ 	bUseIncompatiblePathCreateInterface=FALSE;
/*N*/ 	bAutoClosePolys=TRUE;
/*N*/ 	nAutoCloseDistPix=5;
/*N*/ 	nFreeHandMinDistPix=10;
/*N*/ 
/*N*/ 	pConnectMarker=new ImpSdrConnectMarker(this);
/*N*/ 	pConnectMarker->SetLineWidth(2);
/*N*/ 	pConnectMarker->SetAnimateDelay(500);
/*N*/ 	pConnectMarker->SetAnimateCount(3);
/*N*/ 	pConnectMarker->SetAnimateSpeed(100);
/*N*/ 	pConnectMarker->SetAnimateToggle(TRUE);
/*N*/ }

/*N*/ void SdrCreateView::ImpMakeCreateAttr()
/*N*/ {
/*N*/ }

/*N*/ SdrCreateView::SdrCreateView(SdrModel* pModel1, OutputDevice* pOut):
/*N*/ 	SdrDragView(pModel1,pOut)
/*N*/ {
/*N*/ 	ImpClearVars();
/*N*/ 	ImpMakeCreateAttr();
/*N*/ }

/*N*/ SdrCreateView::~SdrCreateView()
/*N*/ {
/*N*/ 	delete pConnectMarker;
/*N*/ 	delete pCurrentLibObj;
/*N*/ 	delete pAktCreate;
/*N*/ }


/*N*/ BOOL SdrCreateView::IsAction() const
/*N*/ {
/*N*/ 	return SdrDragView::IsAction() || pAktCreate!=NULL;
/*N*/ }




/*N*/ void SdrCreateView::BrkAction()
/*N*/ {
/*N*/ 	SdrDragView::BrkAction();
/*N*/ 	BrkCreateObj();
/*N*/ }


/*N*/ void SdrCreateView::ToggleShownXor(OutputDevice* pOut, const Region* pRegion) const
/*N*/ {
/*N*/ 	SdrDragView::ToggleShownXor(pOut,pRegion);
/*N*/ 	if (pAktCreate!=NULL && aDragStat.IsShown()) {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	}
/*N*/ }

/*N*/ BOOL SdrCreateView::CheckEdgeMode()
/*N*/ {
/*N*/ 	UINT32 nInv=nAktInvent;
/*N*/ 	UINT16 nIdn=nAktIdent;
/*N*/ 	if (pAktCreate!=NULL) {
/*?*/ 		nInv=pAktCreate->GetObjInventor();
/*?*/ 		nIdn=pAktCreate->GetObjIdentifier();
/*?*/ 		// wird vom EdgeObj gemanaged
/*?*/ 		if (nAktInvent==SdrInventor && nAktIdent==OBJ_EDGE) return FALSE;
/*N*/ 	}
/*N*/ 	if (!IsCreateMode() || nAktInvent!=SdrInventor || nAktIdent!=OBJ_EDGE || pCurrentLibObj!=NULL) {
/*?*/ 		if (pConnectMarker->IsVisible()) {DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 pConnectMarker->Hide();
/*N*/ 		pConnectMarker->SetTargetObject(NULL);
/*N*/ 		return FALSE;
/*N*/ 	} else {
/*N*/ 		// TRUE heisst: MouseMove soll Connect checken
/*?*/ 		return !IsAction();
/*N*/ 	}
/*N*/ }




/*N*/ BOOL SdrCreateView::IsTextTool() const
/*N*/ {
/*N*/ 	return eEditMode==SDREDITMODE_CREATE && pCurrentLibObj==NULL && nAktInvent==SdrInventor && (nAktIdent==OBJ_TEXT || nAktIdent==OBJ_TEXTEXT || nAktIdent==OBJ_TITLETEXT || nAktIdent==OBJ_OUTLINETEXT);
/*N*/ }

/*N*/ BOOL SdrCreateView::IsEdgeTool() const
/*N*/ {
/*N*/ 	return eEditMode==SDREDITMODE_CREATE && pCurrentLibObj==NULL && nAktInvent==SdrInventor && (nAktIdent==OBJ_EDGE);
/*N*/ }


/*N*/ void SdrCreateView::SetCurrentObj(UINT16 nIdent, UINT32 nInvent)
/*N*/ {
/*N*/ 	if (pCurrentLibObj!=NULL) {
/*?*/ 		delete pCurrentLibObj;
/*?*/ 		pCurrentLibObj=NULL;
/*N*/ 	}
/*N*/ 	if (nAktInvent!=nInvent || nAktIdent!=nIdent) {
/*N*/ 		nAktInvent=nInvent;
/*N*/ 		nAktIdent=nIdent;
/*N*/ 		SdrObject* pObj = SdrObjFactory::MakeNewObject(nInvent,nIdent,NULL,NULL);
/*N*/ 		
/*N*/ 		if(pObj) 
/*N*/ 		{
/*N*/ 			// Auf pers. Wunsch von Marco:
/*N*/ 			// Mauszeiger bei Textwerkzeug immer I-Beam. Fadenkreuz
/*N*/ 			// mit kleinem I-Beam erst bai MouseButtonDown
/*N*/ 			if(IsTextTool()) 
/*N*/ 			{
/*N*/ 				// #81944# AW: Here the correct pointer needs to be used
/*N*/ 				// if the default is set to vertical writing
/*?*/ 				aAktCreatePointer = POINTER_TEXT;
/*N*/ 			}
/*N*/ 			else 
/*N*/ 				aAktCreatePointer = pObj->GetCreatePointer();
/*N*/ 
/*N*/ 			delete pObj;
/*N*/ 		} 
/*N*/ 		else 
/*N*/ 		{
/*?*/ 			aAktCreatePointer = Pointer(POINTER_CROSS);
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	CheckEdgeMode();
/*N*/ 	ImpSetGlueVisible3(IsEdgeTool());
/*N*/ }









/*N*/ void SdrCreateView::BrkCreateObj()
/*N*/ {
/*N*/ 	if (pAktCreate!=NULL) {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	}
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrCreateView::WriteRecords(SvStream& rOut) const
/*N*/ {
/*N*/ 	SdrDragView::WriteRecords(rOut);
/*N*/ 	{
/*N*/ 		SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWCROBJECT);
/*N*/ 		rOut<<nAktInvent;
/*N*/ 		rOut<<nAktIdent;
/*N*/ 	} {
/*N*/ 		SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWCRFLAGS);
/*N*/ 		rOut<<BOOL(b1stPointAsCenter);
/*N*/ 	} {
/*N*/ 		// in der CreateView (statt ObjEditView) weil sonst inkompatibel.
/*N*/ 		SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWTEXTEDIT);
/*N*/ 		rOut<<BOOL(bQuickTextEditMode);
/*N*/ 	} {
/*N*/ 		// in der CreateView (statt ObjEditView) weil sonst inkompatibel.
/*N*/ 		SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWMACRO);
/*N*/ 		rOut<<BOOL(bMacroMode);
/*N*/ 	}
/*N*/ }

/*N*/ BOOL SdrCreateView::ReadRecord(const SdrIOHeader& rViewHead,
/*N*/ 	const SdrNamedSubRecord& rSubHead,
/*N*/ 	SvStream& rIn)
/*N*/ {
/*N*/ 	BOOL bRet=FALSE;
/*N*/ 	if (rSubHead.GetInventor()==SdrInventor) {
/*N*/ 		bRet=TRUE;
/*N*/ 		switch (rSubHead.GetIdentifier()) {
/*N*/ 			case SDRIORECNAME_VIEWCROBJECT: {
/*N*/ 				UINT32 nInvent;
/*N*/ 				UINT16 nIdent;
/*N*/ 				rIn>>nInvent;
/*N*/ 				rIn>>nIdent;
/*N*/ 				SetCurrentObj(nIdent,nInvent);
/*N*/ 			} break;
/*N*/ 			case SDRIORECNAME_VIEWCRFLAGS: {
/*N*/ 				BOOL bTmp; rIn>>bTmp; b1stPointAsCenter=bTmp;
/*N*/ 			} break;
/*N*/ 			case SDRIORECNAME_VIEWTEXTEDIT: {
/*N*/ 				// in der CreateView (statt ObjEditView) weil sonst inkompatibel.
/*N*/ 				BOOL bTmp; rIn>>bTmp; bQuickTextEditMode=bTmp;
/*N*/ 			} break;
/*N*/ 			case SDRIORECNAME_VIEWMACRO: {
/*N*/ 				// in der CreateView (statt ObjEditView) weil sonst inkompatibel.
/*N*/ 				BOOL bTmp; rIn>>bTmp; bMacroMode=bTmp;
/*N*/ 			} break;
/*N*/ 			default: bRet=FALSE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if (!bRet) bRet=SdrDragView::ReadRecord(rViewHead,rSubHead,rIn);
/*N*/ 	return bRet;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
