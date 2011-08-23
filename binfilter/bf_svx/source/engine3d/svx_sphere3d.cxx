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

#include "svdstr.hrc"

#include "svdio.hxx"

#include "svditer.hxx"

#include "svdmodel.hxx"



#include "polyob3d.hxx"

#include "sphere3d.hxx"


#include "svxids.hrc"

namespace binfilter {

/*N*/ TYPEINIT1(E3dSphereObj, E3dCompoundObject);

/*************************************************************************
|*
|* Kugel erzeugen ohne die Polygone darin zu erzeugen
|*
\************************************************************************/

// FG: Dieser Aufruf erfolgt von der 3D-Object Factory (objfac3d.cxx) und zwar ausschliesslich beim
//     laden von Dokumenten. Hier braucht man keinen CreateSphere-Aufruf, denn die wirkliche
//     Anzahl Segmente ist ja noch nicht bekannt. Dies war bis zum 10.2.97 ein (kleines)
//     Speicherleck.
/*N*/ E3dSphereObj::E3dSphereObj(int dummy) // den Parameter braucht es um unterscheiden zu koennen, welcher
/*N*/ {                                     // der beiden Konstruktoren gemeint ist. Der obige halt per Default
/*N*/ 	// Defaults setzen
/*N*/ 	E3dDefaultAttributes aDefault;
/*N*/ 	SetDefaultAttributes(aDefault);
/*N*/ }

/*N*/ void E3dSphereObj::SetDefaultAttributes(E3dDefaultAttributes& rDefault)
/*N*/ {
/*N*/ 	// Defaults setzen
/*N*/ 	aCenter = rDefault.GetDefaultSphereCenter();
/*N*/ 	aSize = rDefault.GetDefaultSphereSize();
/*N*/ }

/*************************************************************************
|*
|* Give out simple line geometry
|*
\************************************************************************/

/*N*/ void E3dSphereObj::GetLineGeometry(PolyPolygon3D& rLinePolyPolygon) const
/*N*/ {
/*N*/ 	// add geometry describing polygons to rLinePolyPolygon
/*N*/ 	sal_uInt16 nCntHor = (sal_uInt16)GetHorizontalSegments();
/*N*/ 	sal_uInt16 nCntVer = (sal_uInt16)GetVerticalSegments();
/*N*/ 	const Vector3D aRadius = aSize / 2;
/*N*/ 	const double fHInc = (double)DEG2RAD(360) / nCntHor;
/*N*/ 	const double fVInc = (double)DEG2RAD(180) / nCntVer;
/*N*/ 	sal_uInt16 nCntHorPoly = nCntVer - 1;
/*N*/ 	sal_uInt16 nIndHorPoly = rLinePolyPolygon.Count();
/*N*/ 	sal_uInt16 a;
/*N*/ 	double fHAng = 0.0;
/*N*/ 
/*N*/ 	for(a = 0; a < nCntHorPoly; a++)
/*N*/ 	{
/*N*/ 		Polygon3D aNewHor(nCntHor + 1);
/*N*/ 		//aNewHor.SetClosed(TRUE);
/*N*/ 		rLinePolyPolygon.Insert(aNewHor);
/*N*/ 	}
/*N*/ 
/*N*/ 	for(sal_uInt16 nH(0); nH < nCntHor; nH++)
/*N*/ 	{
/*N*/ 		double fHSin = sin(fHAng);
/*N*/ 		double fHCos = cos(fHAng);
/*N*/ 		fHAng += fHInc;
/*N*/ 		double fVAng = DEG2RAD(90);
/*N*/ 		Polygon3D aNewVer(nCntVer + 1);
/*N*/ 
/*N*/ 		for(sal_uInt16 nV(0); nV <= nCntVer; nV++)
/*N*/ 		{
/*N*/ 			double fVSin = sin(fVAng);
/*N*/ 			double fVCos = cos(fVAng);
/*N*/ 			fVAng -= fVInc;
/*N*/ 
/*N*/ 			Vector3D aPos = aCenter;
/*N*/ 			double fRx = aRadius.X() * fVCos;
/*N*/ 			double fRz = aRadius.Z() * fVCos;
/*N*/ 			aPos.X() += fRx * fHCos;
/*N*/ 			aPos.Y() += aRadius.Y() * fVSin;
/*N*/ 			aPos.Z() += fRz * fHSin;
/*N*/ 
/*N*/ 			if(nV == 0)
/*N*/ 			{
/*N*/ 				// top position, only interesting for vertical line
/*N*/ 				aNewVer[0] = aPos;
/*N*/ 			}
/*N*/ 			else if(nV == nCntVer)
/*N*/ 			{
/*N*/ 				// bottom position, only interesting for vertical line
/*N*/ 				aNewVer[nCntVer] = aPos;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				// normal position, insert vertical
/*N*/ 				aNewVer[nV] = aPos;
/*N*/ 
/*N*/ 				// insert horizontal
/*N*/ 				rLinePolyPolygon[nIndHorPoly + (nV - 1)][nH] = aPos;
/*N*/ 				if(!nH)
/*N*/ 					rLinePolyPolygon[nIndHorPoly + (nV - 1)][nCntHor] = aPos;
/*N*/ 			}
/*N*/ 		}	
/*N*/ 
/*N*/ 		/// insert new vertical poly
/*N*/ 		rLinePolyPolygon.Insert(aNewVer);
/*N*/ 	}
/*N*/ 
/*N*/ 	// don't call parent
/*N*/ 	// E3dCompoundObject::GetLineGeometry(rLinePolyPolygon);
/*N*/ }

/*************************************************************************
|*
|* Geometrieerzeugung
|*
\************************************************************************/

/*N*/ void E3dSphereObj::CreateGeometry()
/*N*/ {
/*N*/ 	Polygon3D	aRect3D(4);
/*N*/ 	Polygon3D	aNormal3D(4);
/*N*/ 	Polygon3D	aTexture3D(4);
/*N*/ 
/*N*/ 	if ( GetHorizontalSegments() <  3  )  
/*N*/ 		mpObjectItemSet->Put(Svx3DHorizontalSegmentsItem(3));
/*N*/ 	
/*N*/ 	if ( GetHorizontalSegments() > 100 )  
/*N*/ 		mpObjectItemSet->Put(Svx3DHorizontalSegmentsItem(100));
/*N*/ 	
/*N*/ 	if ( GetVerticalSegments() <  2  )  
/*N*/ 		mpObjectItemSet->Put(Svx3DVerticalSegmentsItem(2));
/*N*/ 	
/*N*/ 	if ( GetVerticalSegments() > 100 )  
/*N*/ 		mpObjectItemSet->Put(Svx3DVerticalSegmentsItem(100));
/*N*/ 
/*N*/ 	// Start der Geometrieerzeugung ankuendigen
/*N*/ 	StartCreateGeometry();
/*N*/ 
/*N*/ 	Vector3D aRadius = aSize / 2;
/*N*/ 	double fHSin1, fHSin2, fHCos1, fHCos2;
/*N*/ 	double fHAng = 0;
/*N*/ 	double fHInc = DEG2RAD(360) / GetHorizontalSegments();
/*N*/ 	double fVInc = DEG2RAD(180) / GetVerticalSegments();
/*N*/ 
/*N*/ 	fHSin2 = 0.0; // sin(0)
/*N*/ 	fHCos2 = 1.0; // cos(0)
/*N*/ 	sal_uInt16 nUpperBound = (sal_uInt16)GetHorizontalSegments();
/*N*/ 
/*N*/ 	for (USHORT nH = 0; nH < nUpperBound; nH++)
/*N*/ 	{
/*N*/ 		fHAng += fHInc;
/*N*/ 		fHSin1 = fHSin2;
/*N*/ 		fHSin2 = sin(fHAng);
/*N*/ 		fHCos1 = fHCos2;
/*N*/ 		fHCos2 = cos(fHAng);
/*N*/ 
/*N*/ 		double fRx, fRz;
/*N*/ 		double fVAng = DEG2RAD(90);
/*N*/ 		double fVSin1, fVSin2, fVCos1, fVCos2;
/*N*/ 		fVSin2 = 1.0; // sin(90)
/*N*/ 		fVCos2 = 0.0; // cos(90)
/*N*/ 		sal_uInt16 nUpperVert = (sal_uInt16)GetVerticalSegments();
/*N*/ 
/*N*/ 		for (USHORT nV = 0; nV < nUpperVert; nV++)
/*N*/ 		{
/*N*/ 			Vector3D aPos;
/*N*/ 
/*N*/ 			fVAng -= fVInc;
/*N*/ 			fVSin1 = fVSin2;
/*N*/ 			fVSin2 = sin(fVAng);
/*N*/ 			fVCos1 = fVCos2;
/*N*/ 			fVCos2 = cos(fVAng);
/*N*/ 
/*N*/ 			aPos = aCenter;
/*N*/ 			fRx = aRadius.X() * fVCos1;
/*N*/ 			fRz = aRadius.Z() * fVCos1;
/*N*/ 			aPos.X() += fRx * fHCos1;
/*N*/ 			aPos.Y() += aRadius.Y() * fVSin1;
/*N*/ 			aPos.Z() += fRz * fHSin1;
/*N*/ 			aRect3D[1] = aPos;
/*N*/ 
/*N*/ 			aPos.X() = aCenter.X() + fRx * fHCos2;
/*N*/ 			aPos.Z() = aCenter.Z() + fRz * fHSin2;
/*N*/ 			aRect3D[2] = aPos;
/*N*/ 
/*N*/ 			aPos = aCenter;
/*N*/ 			fRx = aRadius.X() * fVCos2;
/*N*/ 			fRz = aRadius.Z() * fVCos2;
/*N*/ 			aPos.X() += fRx * fHCos1;
/*N*/ 			aPos.Y() += aRadius.Y() * fVSin2;
/*N*/ 			aPos.Z() += fRz * fHSin1;
/*N*/ 			aRect3D[0] = aPos;
/*N*/ 
/*N*/ 			aPos.X() = aCenter.X() + fRx * fHCos2;
/*N*/ 			aPos.Z() = aCenter.Z() + fRz * fHSin2;
/*N*/ 			aRect3D[3] = aPos;
/*N*/ 
/*N*/ 			if(GetCreateTexture())
/*N*/ 			{
/*N*/ 				aTexture3D[1].X() =
/*N*/ 					(double)(nUpperBound - (nH + 1)) / (double)nUpperBound;
/*N*/ 				aTexture3D[1].Y() =
/*N*/ 					(double)nV / (double)nUpperVert;
/*N*/ 
/*N*/ 				aTexture3D[2].X() =
/*N*/ 					(double)((nUpperBound - (nH + 1)) - 1) / (double)nUpperBound;
/*N*/ 				aTexture3D[2].Y() = aTexture3D[1].Y();
/*N*/ 
/*N*/ 				aTexture3D[3].X() = aTexture3D[2].X();
/*N*/ 				aTexture3D[3].Y() =
/*N*/ 					(double)(nV+1) / (double)nUpperVert;
/*N*/ 
/*N*/ 				aTexture3D[0].X() = aTexture3D[1].X();
/*N*/ 				aTexture3D[0].Y() = aTexture3D[3].Y();
/*N*/ 			}
/*N*/ 
/*N*/ 			if(GetCreateNormals())
/*N*/ 			{
/*N*/ 				aNormal3D = aRect3D;
/*N*/ 				aNormal3D[0].Normalize();
/*N*/ 				aNormal3D[1].Normalize();
/*N*/ 				aNormal3D[2].Normalize();
/*N*/ 				aNormal3D[3].Normalize();
/*N*/ 
/*N*/ 				if(GetCreateTexture())
/*N*/ 					AddGeometry(aRect3D, aNormal3D, aTexture3D, FALSE);
/*N*/ 				else
/*N*/ 					AddGeometry(aRect3D, aNormal3D, FALSE);
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				AddGeometry(aRect3D, FALSE);
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// call parent
/*N*/ 	E3dCompoundObject::CreateGeometry();
/*N*/ }

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/


/*************************************************************************
|*
|* Wireframe erzeugen
|*
\************************************************************************/


/*************************************************************************
|*
|* Wandle das Objekt in ein Gruppenobjekt bestehend aus n Polygonen
|*
\************************************************************************/

/*N*/ SdrObject *E3dSphereObj::DoConvertToPolyObj(BOOL bBezier) const
/*N*/ {
/*N*/ 	return NULL;
/*N*/ }

/*************************************************************************
|*
|* Objektdaten in Stream speichern
|*
\************************************************************************/

/*N*/ void E3dSphereObj::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	long nVersion = rOut.GetVersion(); // Build_Nr * 10 z.B. 3810
/*N*/ 	if(nVersion < 3800)
/*N*/ 	{
/*N*/ 		// Alte Geometrie erzeugen, um die E3dPolyObj's zu haben
/*N*/ 		((E3dCompoundObject*)this)->ReCreateGeometry(TRUE);
/*N*/ 	}
/*N*/ 
/*N*/ 	// call parent
/*N*/ 	SdrAttrObj::WriteData(rOut);
/*N*/ 
/*N*/ 	// Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ 	SdrDownCompat aCompat(rOut, STREAM_WRITE);
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("E3dSphereObj");
/*N*/ #endif
/*N*/ 
/*N*/ 	if (rOut.GetVersion() < 3560) // FG: kleiner als die Final Beta der Version 4.0
/*N*/ 	{
/*N*/ 		pSub->Save(rOut);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// [FG] Jetzt wird die Kindliste abgeklapptert, allerdings weiss ich im Gegensatz zu
/*N*/ 		// Joe dass es nur E3dPolyObj - Kindobjekte sein koennen.
/*N*/ 		// Jedes dieser Objekte frage ich ob es eigene Attribute enthaelt.  Falls OwnStyle ()
/*N*/ 		// true liefert, werde ich das Polygon nicht wegspeichern.
/*N*/ 
/*N*/ 		SdrObjListIter aIter(*pSub,IM_FLAT);
/*N*/ 		while (aIter.IsMore()) {
/*N*/ 			E3dPolyObj* pObj=(E3dPolyObj *) aIter.Next();
/*N*/ 			if ((!pObj->IsNotPersistent()) && (pObj->OwnAttrs() || pObj->OwnStyle()))
/*N*/ 			{
/*N*/ 				rOut<<*pObj;
/*N*/ 			}
/*N*/ 			if (pSub->GetModel()!=NULL) pSub->GetModel()->IncProgress();
/*N*/ 		}
/*N*/ 		SdrIOHeader(rOut,STREAM_WRITE,SdrIOEndeID); // Endemarke
/*N*/ 	}
/*N*/ 
/*N*/ 	// Daß hier gehört zum E3dObject (ohne Basisklassen);
/*N*/ 	if (rOut.GetVersion() < 3560)
/*N*/ 	{
/*N*/ 		rOut << aLocalBoundVol;
/*N*/ 
/*N*/ 		Old_Matrix3D aMat3D;
/*N*/ 		aMat3D = aTfMatrix;
/*N*/ 		rOut << aMat3D;
/*N*/ 
/*N*/ 		rOut << nLogicalGroup;
/*N*/ 		rOut << nObjTreeLevel;
/*N*/ 		rOut << nPartOfParent;
/*N*/ 		rOut << UINT16(eDragDetail);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		E3dObject::WriteOnlyOwnMembers(rOut);
/*N*/ 	}
/*N*/ 	// Das gehört zu E3dSphere
/*N*/ 	rOut << GetHorizontalSegments();
/*N*/ 
/*N*/ 	rOut << GetVerticalSegments();
/*N*/ 
/*N*/ 	rOut << aCenter;
/*N*/ 	rOut << aSize;
/*N*/ 
/*N*/ 	// Das hier ist ein Merkmal eines Compound-Objektes
/*N*/ 	rOut << GetDoubleSided();
/*N*/ 
/*N*/ 	// Ab Version 395 (8.6.98): Parameter aus dem Objekt
/*N*/ 	// E3dCompoundObject. Da irgendwann mal jemand die Ableitungs-
/*N*/ 	// hierarchie beim FileFormat unterbrochen hat, wurden diese Attribute
/*N*/ 	// bisher NOCH NIE gespeichert (Grrr). Diese Stelle muss nun natuerlich
/*N*/ 	// auch IMMER MITGEPFLEGT werden, wenn sich Parameter in
/*N*/ 	// E3dCompoundObject oder E3dObject aendern.
/*N*/ 	rOut << GetDoubleSided();
/*N*/ 
/*N*/ 	rOut << BOOL(bCreateNormals);
/*N*/ 	rOut << BOOL(bCreateTexture);
/*N*/ 
/*N*/ 	sal_uInt16 nVal = GetNormalsKind();
/*N*/ 	rOut << BOOL(nVal > 0);
/*N*/ 	rOut << BOOL(nVal > 1);
/*N*/ 	
/*N*/ 	nVal = GetTextureProjectionX();
/*N*/ 	rOut << BOOL(nVal > 0);
/*N*/ 	rOut << BOOL(nVal > 1);
/*N*/ 	
/*N*/ 	nVal = GetTextureProjectionY();
/*N*/ 	rOut << BOOL(nVal > 0);
/*N*/ 	rOut << BOOL(nVal > 1);
/*N*/ 	
/*N*/ 	rOut << BOOL(GetShadow3D());
/*N*/ 
/*N*/ 	rOut << GetMaterialAmbientColor();
/*N*/ 	rOut << GetMaterialColor();
/*N*/ 	rOut << GetMaterialSpecular();
/*N*/ 	rOut << GetMaterialEmission();
/*N*/ 	rOut << GetMaterialSpecularIntensity();
/*N*/ 	
/*N*/ 	aBackMaterial.WriteData(rOut);
/*N*/ 
/*N*/ 	rOut << (UINT16)GetTextureKind();
/*N*/ 
/*N*/ 	rOut << (UINT16)GetTextureMode();
/*N*/ 
/*N*/ 	rOut << BOOL(GetNormalsInvert());
/*N*/ 
/*N*/ 	// neu ab 534: (hat noch gefehlt)
/*N*/ 	rOut << BOOL(GetTextureFilter());
/*N*/ 
/*N*/ 	if(nVersion < 3800)
/*N*/ 	{
/*N*/ 		// Geometrie neu erzeugen, um E3dPolyObj's wieder loszuwerden
/*N*/ 		((E3dCompoundObject*)this)->ReCreateGeometry();
/*N*/ 	}
/*N*/ #endif
/*N*/ }

/*************************************************************************
|*
|* Objektdaten aus Stream laden
|*
\************************************************************************/

/*N*/ void E3dSphereObj::ReadData31(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	SdrDownCompat aCompat(rIn, STREAM_READ);
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("E3dSphereObj");
/*N*/ #endif
/*N*/ 	// dann die Member
/*N*/ 	UINT16	nTmp16;
/*N*/ 	sal_Int32 nTmp32;
/*N*/ 
/*N*/ 	pSub->Load(rIn, *pPage);
/*N*/ 	// FG: Die Daten des 3D-Objektes
/*N*/ 	rIn >> aLocalBoundVol;
/*N*/ 
/*N*/ 	Old_Matrix3D aMat3D;
/*N*/ 	rIn >> aMat3D;
/*N*/ 	aTfMatrix = Matrix4D(aMat3D);
/*N*/ 
/*N*/ 	rIn >> nLogicalGroup;
/*N*/ 	rIn >> nObjTreeLevel;
/*N*/ 	rIn >> nPartOfParent;
/*N*/ 	rIn >> nTmp16; eDragDetail = E3dDragDetail(nTmp16);
/*N*/ 	// FG: Die Daten der Kugel
/*N*/ 
/*N*/ 	rIn >> nTmp32;
/*N*/ 	mpObjectItemSet->Put(Svx3DHorizontalSegmentsItem(nTmp32));
/*N*/ 
/*N*/ 	rIn >> nTmp32;
/*N*/ 	mpObjectItemSet->Put(Svx3DVerticalSegmentsItem(nTmp32));
/*N*/ 
/*N*/ 	rIn >> aCenter;
/*N*/ 	rIn >> aSize;
/*N*/ 
/*N*/ 	bBoundVolValid = FALSE;
/*N*/ 
/*N*/ 	// Geometrie neu erzeugen
/*N*/ 	ReCreateGeometry();
/*N*/ }

/*************************************************************************
|*
|* Objektdaten aus Stream laden
|*
\************************************************************************/

/*N*/ void E3dSphereObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	// FG (10.2.97) Der Pointer ist ein Kunstgriff er wird in der ReadData-Routine erzeugt dort werden
/*N*/ 	//    die abgespeicherten attributierten Flaechen abgelegt. Danach wird eine Kugel mit Default-Paramtern
/*N*/ 	//    erzeugt und dann werden die per Default erzeugten einfach, falls noetig mit denen aus dieser Liste
/*N*/ 	//    ausgetauscht. Am Ende von ReadData wird die Liste wieder zerstoert.
/*N*/ 	E3dObjList *pLoadedE3dPolyObjs;
/*N*/ 
/*N*/ 	if (ImpCheckSubRecords (rHead, rIn))
/*N*/ 	{
/*N*/ 		// leider kann das E3dLatheObj nicht auf E3dObject abgestuetzt werden,
/*N*/ 		// da neue Member hinzugekommen sind und die Kompatibilitaet erhalten
/*N*/ 		// bleiben muss.
/*N*/ 		SdrAttrObj::ReadData(rHead, rIn);
/*N*/ 		if ((rIn.GetVersion() < 3560) || (rHead.GetVersion() <= 12))
/*N*/ 		{
/*N*/ 			ReadData31(rHead, rIn);
/*N*/ 			return;
/*N*/ 		}
/*N*/ 
/*N*/ 
/*N*/ 		// Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ 		SdrDownCompat aCompat(rIn, STREAM_READ);
/*N*/ #ifdef DBG_UTIL
/*N*/ 		aCompat.SetID("E3dSphereObj");
/*N*/ #endif
/*N*/ 		// dann die Member
/*N*/ 		BOOL    bMyDoubleSided;
/*N*/ 
/*N*/ 		// [FG]: Es wird erstmal gelesen was da ist, ob es vollstaendig ist zeigt sich bei
/*N*/ 		// dem Aufruf von CreateSphere, dort werden die fehlenden Flaechen einfach
/*N*/ 		// erzeugt.
/*N*/ 		pLoadedE3dPolyObjs = new E3dObjList(NULL, NULL);
/*N*/ 		pLoadedE3dPolyObjs->SetOwnerObj(this);
/*N*/ 		pLoadedE3dPolyObjs->SetListKind(SDROBJLIST_GROUPOBJ);
/*N*/ 		pLoadedE3dPolyObjs->Load(rIn, *pPage);
/*N*/ 		E3dObject::ReadOnlyOwnMembers(rHead, rIn);
/*N*/ 
/*N*/ 		if (aCompat.GetBytesLeft ())
/*N*/ 		{
/*N*/ 			// neue Member
/*N*/ 			sal_Int32 nTmp32;
/*N*/ 
/*N*/ 			rIn >> nTmp32;
/*N*/ 			mpObjectItemSet->Put(Svx3DHorizontalSegmentsItem(nTmp32));
/*N*/ 
/*N*/ 			rIn >> nTmp32;
/*N*/ 			mpObjectItemSet->Put(Svx3DVerticalSegmentsItem(nTmp32));
/*N*/ 
/*N*/ 			rIn >> aCenter;
/*N*/ 			rIn >> aSize;
/*N*/ 			if (aCompat.GetBytesLeft ())
/*N*/ 			{
/*N*/ 				rIn >> bMyDoubleSided;
/*N*/ 				mpObjectItemSet->Put(Svx3DDoubleSidedItem(bMyDoubleSided));
/*N*/ 			}                                    
/*N*/ 
/*N*/ 			if (aCompat.GetBytesLeft())
/*N*/ 			{
/*N*/ 				// Ab Version 395 (8.6.98): Parameter aus dem Objekt
/*N*/ 				// E3dCompoundObject. Da irgendwann mal jemand die Ableitungs-
/*N*/ 				// hierarchie beim FileFormat unterbrochen hat, wurden diese Attribute
/*N*/ 				// bisher NOCH NIE gespeichert (Grrr). Diese Stelle muss nun natuerlich
/*N*/ 				// auch IMMER MITGEPFLEGT werden, wenn sich Parameter in
/*N*/ 				// E3dCompoundObject oder E3dObject aendern.
/*N*/ 				BOOL bTmp, bTmp2;
/*N*/ 				sal_uInt16 nTmp;
/*N*/ 
/*N*/ 				rIn >> bTmp; 
/*N*/ 				mpObjectItemSet->Put(Svx3DDoubleSidedItem(bTmp));
/*N*/ 
/*N*/ 				rIn >> bTmp; bCreateNormals = bTmp;
/*N*/ 				rIn >> bTmp; bCreateTexture = bTmp;
/*N*/ 
/*N*/ 				rIn >> bTmp; 
/*N*/ 				rIn >> bTmp2;
/*N*/ 				if(bTmp == FALSE && bTmp2 == FALSE)
/*N*/ 					nTmp = 0;
/*N*/ 				else if(bTmp == TRUE && bTmp2 == FALSE)
/*N*/ 					nTmp = 1;
/*N*/ 				else
/*N*/ 					nTmp = 2;
/*N*/ 				mpObjectItemSet->Put(Svx3DNormalsKindItem(nTmp));
/*N*/ 
/*N*/ 				rIn >> bTmp; 
/*N*/ 				rIn >> bTmp2;
/*N*/ 				if(bTmp == FALSE && bTmp2 == FALSE)
/*N*/ 					nTmp = 0;
/*N*/ 				else if(bTmp == TRUE && bTmp2 == FALSE)
/*N*/ 					nTmp = 1;
/*N*/ 				else
/*N*/ 					nTmp = 2;
/*N*/ 				mpObjectItemSet->Put(Svx3DTextureProjectionXItem(nTmp));
/*N*/ 
/*N*/ 				rIn >> bTmp; 
/*N*/ 				rIn >> bTmp2;
/*N*/ 				if(bTmp == FALSE && bTmp2 == FALSE)
/*N*/ 					nTmp = 0;
/*N*/ 				else if(bTmp == TRUE && bTmp2 == FALSE)
/*N*/ 					nTmp = 1;
/*N*/ 				else
/*N*/ 					nTmp = 2;
/*N*/ 				mpObjectItemSet->Put(Svx3DTextureProjectionYItem(nTmp));
/*N*/ 
/*N*/ 				rIn >> bTmp; 
/*N*/ 				mpObjectItemSet->Put(Svx3DShadow3DItem(bTmp));
/*N*/ 
/*N*/ 				Color aCol;
/*N*/ 				
/*N*/ 				rIn >> aCol;
/*N*/ 				SetMaterialAmbientColor(aCol);
/*N*/ 				
/*N*/ 				rIn >> aCol;
/*N*/ 				// do NOT use, this is the old 3D-Color(!)
/*N*/ 				// SetItem(XFillColorItem(String(), aCol));
/*N*/ 				
/*N*/ 				rIn >> aCol;
/*N*/ 				mpObjectItemSet->Put(Svx3DMaterialSpecularItem(aCol));
/*N*/ 				
/*N*/ 				rIn >> aCol;
/*N*/ 				mpObjectItemSet->Put(Svx3DMaterialEmissionItem(aCol));
/*N*/ 				
/*N*/ 				rIn >> nTmp;
/*N*/ 				mpObjectItemSet->Put(Svx3DMaterialSpecularIntensityItem(nTmp));
/*N*/ 
/*N*/ 				aBackMaterial.ReadData(rIn);
/*N*/ 
/*N*/ 				rIn >> nTmp; 
/*N*/ 				mpObjectItemSet->Put(Svx3DTextureKindItem(nTmp));
/*N*/ 
/*N*/ 				rIn >> nTmp; 
/*N*/ 				mpObjectItemSet->Put(Svx3DTextureModeItem(nTmp));
/*N*/ 
/*N*/ 				rIn >> bTmp; 
/*N*/ 				mpObjectItemSet->Put(Svx3DNormalsInvertItem(bTmp));
/*N*/ 
/*N*/ 			}
/*N*/ 
/*N*/ 			// neu ab 534: (hat noch gefehlt)
/*N*/ 			if (aCompat.GetBytesLeft () >= sizeof (BOOL))
/*N*/ 			{
/*N*/ 				BOOL bTmp;
/*N*/ 				rIn >> bTmp; 
/*N*/ 				mpObjectItemSet->Put(Svx3DTextureFilterItem(bTmp));
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			DBG_ERROR("AW: Kugel laden: nicht vorgesehener Fall");
/*N*/ 		}
/*N*/ 		pLoadedE3dPolyObjs->Clear();
/*N*/ 		delete pLoadedE3dPolyObjs;
/*N*/ 	}
/*N*/ 
/*N*/ 	// Geometrie neu erzeugen
/*N*/ 	ReCreateGeometry();
/*N*/ }

/*************************************************************************
|*
|* Zuweisungsoperator
|*
\************************************************************************/


/*************************************************************************
|*
|* Lokale Parameter setzen mit Geometrieneuerzeugung
|*
\************************************************************************/

/*N*/ void E3dSphereObj::SetCenter(const Vector3D& rNew)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

/*N*/ void E3dSphereObj::SetSize(const Vector3D& rNew)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

//////////////////////////////////////////////////////////////////////////////
// private support routines for ItemSet access

/*N*/ void E3dSphereObj::PostItemChange(const sal_uInt16 nWhich)
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	E3dCompoundObject::PostItemChange(nWhich);
/*N*/ 
/*N*/ 	// local changes
/*N*/ 	switch(nWhich)
/*N*/ 	{
/*N*/ 		case SDRATTR_3DOBJ_HORZ_SEGS:
/*N*/ 		{
/*N*/ 			bGeometryValid = FALSE;
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DOBJ_VERT_SEGS:
/*N*/ 		{
/*N*/ 			bGeometryValid = FALSE;
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Get the name of the object (singular)
|*
\************************************************************************/


/*************************************************************************
|*
|* Get the name of the object (plural)
|*
\************************************************************************/


// EOF
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
