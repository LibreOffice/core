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

#include "svdovirt.hxx"
#include "svdio.hxx"
#include "svdhdl.hxx"
namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ TYPEINIT1(SdrVirtObj,SdrObject);

/*N*/ SdrVirtObj::SdrVirtObj(SdrObject& rNewObj):
/*N*/ 	rRefObj(rNewObj)
/*N*/ {
/*N*/ 	bVirtObj=TRUE; // Ja, ich bin ein virtuelles Objekt
/*N*/ 	rRefObj.AddReference(*this);
/*N*/ 	bClosedObj=rRefObj.IsClosedObj();
/*N*/ }

/*N*/ SdrVirtObj::~SdrVirtObj()
/*N*/ {
/*N*/ 	rRefObj.DelReference(*this);
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ const SdrObject& SdrVirtObj::GetReferencedObj() const
/*N*/ {
/*N*/ 	return rRefObj;
/*N*/ }

/*N*/ SdrObject& SdrVirtObj::ReferencedObj()
/*N*/ {
/*N*/ 	return rRefObj;
/*N*/ }

/*N*/ void __EXPORT SdrVirtObj::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType)
/*N*/ {
/*N*/ 	bClosedObj=rRefObj.IsClosedObj();
/*N*/ 	SetRectsDirty(); // hier noch Optimieren.
/*N*/ 	SendRepaintBroadcast();
/*N*/ }


////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrVirtObj::SetModel(SdrModel* pNewModel)
/*N*/ {
/*N*/ 	SdrObject::SetModel(pNewModel);
/*N*/ 	rRefObj.SetModel(pNewModel);
/*N*/ }


/*N*/ UINT32 SdrVirtObj::GetObjInventor() const
/*N*/ {
/*N*/ 	return rRefObj.GetObjInventor();
/*N*/ }

/*N*/ UINT16 SdrVirtObj::GetObjIdentifier() const
/*N*/ {
/*N*/ 	return rRefObj.GetObjIdentifier();
/*N*/ }

/*N*/ SdrObjList* SdrVirtObj::GetSubList() const
/*N*/ {
/*N*/ 	return rRefObj.GetSubList();
/*N*/ }



/*N*/ void SdrVirtObj::SetChanged()
/*N*/ {
/*N*/ 	SdrObject::SetChanged();
/*N*/ }










////////////////////////////////////////////////////////////////////////////////////////////////////













////////////////////////////////////////////////////////////////////////////////////////////////////







////////////////////////////////////////////////////////////////////////////////////////////////////


/*N*/ void SdrVirtObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
/*N*/ {
/*N*/ 	rRefObj.NbcResize(rRef-aAnchor,xFact,yFact);
/*N*/ 	SetRectsDirty();
/*N*/ }




////////////////////////////////////////////////////////////////////////////////////////////////////


/*N*/ void SdrVirtObj::Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
/*N*/ {
/*N*/ 	if (xFact.GetNumerator()!=xFact.GetDenominator() || yFact.GetNumerator()!=yFact.GetDenominator()) {
/*N*/ 		Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
/*N*/ 		rRefObj.Resize(rRef-aAnchor,xFact,yFact);
/*N*/ 		SetRectsDirty();
/*N*/ 		SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
/*N*/ 	}
/*N*/ }




////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrVirtObj::RecalcSnapRect()
/*N*/ {
/*N*/ 	aSnapRect=rRefObj.GetSnapRect();
/*N*/ 	aSnapRect+=aAnchor;
/*N*/ }




////////////////////////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ long SdrVirtObj::GetRotateAngle() const
/*N*/ {
/*N*/ 	return rRefObj.GetRotateAngle();
/*N*/ }

/*N*/ long SdrVirtObj::GetShearAngle(FASTBOOL bVertical) const
/*N*/ {
/*N*/ 	return rRefObj.GetShearAngle(bVertical);
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////






/*N*/ void SdrVirtObj::NbcSetPoint(const Point& rPnt, USHORT i)
/*N*/ {
/*N*/ 	Point aP(rPnt);
/*N*/ 	aP-=aAnchor;
/*N*/ 	rRefObj.SetPoint(aP,i);
/*N*/ 	SetRectsDirty();
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ SdrObjGeoData* SdrVirtObj::NewGeoData() const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return NULL;//STRIP001 
/*N*/ }

/*N*/ void SdrVirtObj::SaveGeoData(SdrObjGeoData& rGeo) const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

/*N*/ void SdrVirtObj::RestGeoData(const SdrObjGeoData& rGeo)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// ItemSet access

/*N*/ const SfxItemSet& SdrVirtObj::GetItemSet() const
/*N*/ {
/*N*/ 	return rRefObj.GetItemSet();
/*N*/ }

/*N*/ SfxItemSet* SdrVirtObj::CreateNewItemSet(SfxItemPool& rPool)
/*N*/ {
/*N*/ 	return rRefObj.CreateNewItemSet(rPool);
/*N*/ }





////////////////////////////////////////////////////////////////////////////////////////////////////
// private support routines for ItemSet access




////////////////////////////////////////////////////////////////////////////////////////////////////
// pre- and postprocessing for objects for saving

/*N*/ void SdrVirtObj::PreSave()
/*N*/ {
/*N*/ }

/*N*/ void SdrVirtObj::PostSave()
/*N*/ {
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////


/*N*/ void SdrVirtObj::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

/*N*/ void SdrVirtObj::NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
/*N*/ {
/*N*/ 	rRefObj.NbcSetStyleSheet(pNewStyleSheet,bDontRemoveHardAttr);
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////







////////////////////////////////////////////////////////////////////////////////////////////////////

/*?*/ void SdrVirtObj::WriteData(SvStream& rOut) const
/*?*/ {
/*?*/ 	SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*?*/ #ifdef DBG_UTIL
/*?*/ 	aCompat.SetID("SdrVirtObj");
/*?*/ #endif
/*?*/ 	// fehlende Implementation
/*?*/ 	rOut<<aAnchor;
/*?*/ }

/*?*/ void SdrVirtObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*?*/ {
/*?*/ 	if (rIn.GetError()!=0) return;
/*?*/ 	if (rHead.GetVersion()>=4) {
/*?*/ 		SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*?*/ #ifdef DBG_UTIL
/*?*/ 		aCompat.SetID("SdrVirtObj");
/*?*/ #endif
/*?*/ 		// fehlende Implementation
/*?*/ 		rIn>>aAnchor;
/*?*/ 	} else {
/*?*/ 		rIn>>aAnchor;
/*?*/ 	}
/*?*/ }

/*?*/ void SdrVirtObj::AfterRead()
/*?*/ {
/*?*/ 	// fehlende Implementation
/*?*/ }

/*?*/ const Point SdrVirtObj::GetOffset() const
/*?*/ {
/*?*/     return Point(0,0);
/*?*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
