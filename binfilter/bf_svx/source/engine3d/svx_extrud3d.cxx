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


#include "globl3d.hxx"

#include "polyob3d.hxx"

#include "extrud3d.hxx"

#include "e3dcmpt.hxx"



#include "svxids.hrc"




namespace binfilter {

/*N*/ TYPEINIT1(E3dExtrudeObj, E3dCompoundObject);

/*************************************************************************
|*
|* Konstruktor, erzeugt zwei Deckelflaechen-PolyPolygone und (PointCount-1)
|* Seitenflaechen-Rechtecke aus dem uebergebenen PolyPolygon
|*
\************************************************************************/

/*N*/ E3dExtrudeObj::E3dExtrudeObj(E3dDefaultAttributes& rDefault, const PolyPolygon& rPP, double fDepth)
/*N*/ :	E3dCompoundObject(rDefault),
/*N*/ 	aExtrudePolygon(rPP, rDefault.GetDefaultExtrudeScale())
/*N*/ {
/*N*/ 	// Defaults setzen
/*N*/ 	SetDefaultAttributes(rDefault);
/*N*/ 
/*N*/ 	// set extrude depth
/*N*/ 	mpObjectItemSet->Put(Svx3DDepthItem((sal_uInt32)(fDepth + 0.5)));
/*N*/ 
/*N*/ 	// Geometrie erzeugen
/*N*/ 	CreateGeometry();
/*N*/ }

/*************************************************************************
|*
|* wie voriger Konstruktor, nur mit XPolygon; das XPolygon wird
|* jedoch nicht Bezier-konvertiert, sondern es werden nur seine
|* Punktkoordinaten uebernommen
|*
\************************************************************************/

/*N*/ E3dExtrudeObj::E3dExtrudeObj(E3dDefaultAttributes& rDefault, const XPolyPolygon& rXPP, double fDepth)
/*N*/ :	E3dCompoundObject(rDefault),
/*N*/ 	aExtrudePolygon(rXPP, rDefault.GetDefaultExtrudeScale())
/*N*/ {
/*N*/ 	// Defaults setzen
/*N*/ 	SetDefaultAttributes(rDefault);
/*N*/ 
/*N*/ 	// set extrude depth
/*N*/ 	mpObjectItemSet->Put(Svx3DDepthItem((sal_uInt32)(fDepth + 0.5)));
/*N*/ 
/*N*/ 	// Geometrie erzeugen
/*N*/ 	CreateGeometry();
/*N*/ }

/*N*/ E3dExtrudeObj::E3dExtrudeObj()
/*N*/ :	E3dCompoundObject()
/*N*/ {
/*N*/ 	// Defaults setzen
/*N*/ 	E3dDefaultAttributes aDefault;
/*N*/ 	SetDefaultAttributes(aDefault);
/*N*/ }

/*N*/ void E3dExtrudeObj::SetDefaultAttributes(E3dDefaultAttributes& rDefault)
/*N*/ {
/*N*/ 	// Defaults setzen
/*N*/ 	ImpForceItemSet();
/*N*/ 
/*N*/ 	fExtrudeScale = rDefault.GetDefaultExtrudeScale();
/*N*/ 
/*N*/ 	// #107245#
/*N*/ 	mpObjectItemSet->Put(Svx3DSmoothNormalsItem(rDefault.GetDefaultExtrudeSmoothed()));
/*N*/ 	mpObjectItemSet->Put(Svx3DSmoothLidsItem(rDefault.GetDefaultExtrudeSmoothFrontBack()));
/*N*/ 	mpObjectItemSet->Put(Svx3DCharacterModeItem(rDefault.GetDefaultExtrudeCharacterMode()));
/*N*/ 	mpObjectItemSet->Put(Svx3DCloseFrontItem(rDefault.GetDefaultExtrudeCloseFront()));
/*N*/ 	mpObjectItemSet->Put(Svx3DCloseBackItem(rDefault.GetDefaultExtrudeCloseBack()));
/*N*/ 
/*N*/ 	// Bei extrudes defaultmaessig StdTexture in X und Y
/*N*/ 	mpObjectItemSet->Put(Svx3DTextureProjectionXItem(1));
/*N*/ 	mpObjectItemSet->Put(Svx3DTextureProjectionYItem(1));
/*N*/ }

/*************************************************************************
|*
|* Geometrieerzeugung
|*
\************************************************************************/

/*N*/ PolyPolygon3D E3dExtrudeObj::GetFrontSide()
/*N*/ {
/*N*/ 	// Polygon als Grundlage holen
/*N*/ 	PolyPolygon3D aPolyPoly3D(aExtrudePolygon);
/*N*/ 
/*N*/ 	// Ueberfluessige Punkte entfernen, insbesondere doppelte
/*N*/ 	// Start- und Endpunkte verhindern
/*N*/ 	aPolyPoly3D.RemoveDoublePoints();
/*N*/ 
/*N*/ 	// Normale holen
/*N*/ 	Vector3D aNormal = aPolyPoly3D.GetNormal();
/*N*/ 	if((aNormal.Z() > 0.0) != (GetExtrudeDepth() != 0))
/*N*/ 		aPolyPoly3D.FlipDirections();
/*N*/ 
/*N*/ 	// Orientierung evtl. vorhandener Loecher in einen definierten
/*N*/ 	// Ausgangszustand bringen
/*N*/ 	aPolyPoly3D.SetDirections();
/*N*/ 
/*N*/ 	return aPolyPoly3D;
/*N*/ }

/*N*/ PolyPolygon3D E3dExtrudeObj::GetBackSide(const PolyPolygon3D& rFrontSide)
/*N*/ {
/*N*/ 	PolyPolygon3D aBackSide(rFrontSide);
/*N*/ 
/*N*/ 	if(GetExtrudeDepth() != 0)
/*N*/ 	{
/*N*/ 		// Extrudevektor bilden
/*N*/ 		Vector3D aNormal = aBackSide.GetNormal();
/*N*/ 		if(aNormal.Z() < 0.0)
/*N*/ 			aNormal.Z() = -aNormal.Z();
/*N*/ 		Vector3D aOffset = aNormal * (double)GetExtrudeDepth();
/*N*/ 
/*N*/ 		// eventuell Skalieren
/*N*/ 		if(GetPercentBackScale() != 100)
/*N*/ 			ScalePoly(aBackSide, (double)GetPercentBackScale() / 100.0);
/*N*/ 
/*N*/ 		// Verschieben
/*N*/ 		Matrix4D aTrans;
/*N*/ 		aTrans.Translate(aOffset);
/*N*/ 		aBackSide.Transform(aTrans);
/*N*/ 	}
/*N*/ 
/*N*/ 	return aBackSide;
/*N*/ }

/*************************************************************************
|*
|* Give out simple line geometry
|*
\************************************************************************/


/*N*/ void E3dExtrudeObj::CreateGeometry()
/*N*/ {
/*N*/ 	// Start der Geometrieerzeugung ankuendigen
/*N*/ 	StartCreateGeometry();
/*N*/ 
/*N*/ 	// #78972# prepare new line geometry creation
/*N*/ 	maLinePolyPolygon.Clear();
/*N*/ 
/*N*/ 	// Polygon als Grundlage holen
/*N*/ 	PolyPolygon3D aFrontSide = GetFrontSide();
/*N*/ 
/*N*/ 	if(GetExtrudeDepth() != 0)
/*N*/ 	{
/*N*/ 		// Hinteres Polygon erzeugen
/*N*/ 		PolyPolygon3D aBackSide = GetBackSide(aFrontSide);
/*N*/ 
/*N*/ 		// Was muss erzeugt werden?
/*N*/ 		if(!aFrontSide.IsClosed())
/*N*/ 			mpObjectItemSet->Put(Svx3DDoubleSidedItem(TRUE));
/*N*/ 
/*N*/ 		double fTextureDepth=1.0;
/*N*/ 		double fTextureStart=0.0;
/*N*/ 
/*N*/ 		// Texturen erzeugen?
/*N*/ 		if(!GetCreateTexture())
/*N*/ 			fTextureStart = fTextureDepth = 0.0;
/*N*/ 
/*N*/ 		// Falls Texturen erzeugen Randbreite fuer diese bestimmen
/*N*/ 		double fSurroundFactor = 1.0;
/*N*/ 		if(GetCreateTexture())
/*N*/ 		{
/*N*/ 			fSurroundFactor = aFrontSide.GetLength() / sqrt(aFrontSide.GetPolyArea());
/*N*/ 			fSurroundFactor = (double)((long)(fSurroundFactor - 0.5));
/*N*/ 			if(fSurroundFactor == 0.0)
/*N*/ 				fSurroundFactor = 1.0;
/*N*/ 		}
/*N*/ 
/*N*/ 		// Segment erzeugen
/*N*/ 		ImpCreateSegment(
/*N*/ 			aFrontSide,
/*N*/ 			aBackSide,
/*N*/ 			0L,
/*N*/ 			0L,
/*N*/ 			GetCloseFront(), // #107245# bExtrudeCloseFront,
/*N*/ 			GetCloseBack(), // #107245# bExtrudeCloseBack,
/*N*/ 			(double)GetPercentDiagonal() / 200.0,
/*N*/ 			GetSmoothNormals(), // #107245# GetExtrudeSmoothed(),
/*N*/ 			GetSmoothNormals(), // #107245# GetExtrudeSmoothed(),
/*N*/ 			GetSmoothLids(), // #107245# GetExtrudeSmoothFrontBack(),
/*N*/ 			fSurroundFactor,
/*N*/ 			fTextureStart,
/*N*/ 			fTextureDepth,
/*N*/ 			GetCreateNormals(),
/*N*/ 			GetCreateTexture(),
/*N*/ 			GetCharacterMode(), // #107245# bExtrudeCharacterMode,
/*N*/ 			FALSE,
/*N*/ 			// #78972#
/*N*/ 			&maLinePolyPolygon);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// nur ein Polygon erzeugen
/*?*/ 		mpObjectItemSet->Put(Svx3DDoubleSidedItem(TRUE));
/*?*/ 
/*?*/ 		// Fuer evtl. selbst erzeugte Normalen
/*?*/ 		PolyPolygon3D aNormalsFront;
/*?*/ 
/*?*/ 		// Extrudevektor bilden
/*?*/ 		Vector3D aNormal = aFrontSide.GetNormal();
/*?*/ 		Vector3D aOffset = aNormal * (double)GetExtrudeDepth();
/*?*/ 
/*?*/ 		// Normalen und Vorderseite selbst erzeugen
/*?*/ 		AddFrontNormals(aFrontSide, aNormalsFront, aOffset);
/*?*/ 		CreateFront(aFrontSide, aNormalsFront, GetCreateNormals(), GetCreateTexture());
/*?*/ 
/*?*/ 		// #78972#
/*?*/ 		maLinePolyPolygon.Insert(aFrontSide);
/*N*/ 	}
/*N*/ 
/*N*/ 	// #78972#
/*N*/ 	ImpCompleteLinePolygon(maLinePolyPolygon, aFrontSide.Count(), FALSE);
/*N*/ 
/*N*/ 	// call parent
/*N*/ 	E3dCompoundObject::CreateGeometry();
/*N*/ }

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

/*N*/ UINT16 E3dExtrudeObj::GetObjIdentifier() const
/*N*/ {
/*N*/ 	return E3D_EXTRUDEOBJ_ID;
/*N*/ }

/*************************************************************************
|*
|* Wireframe erzeugen
|*
\************************************************************************/

/*N*/ void E3dExtrudeObj::CreateWireframe(Polygon3D& rWirePoly, const Matrix4D* pTf,
/*N*/ 	E3dDragDetail eDetail)
/*N*/ {
/*N*/ 	if ( eDetail == E3DDETAIL_ALLLINES ||
/*N*/ 		(eDetail == E3DDETAIL_DEFAULT && GetDragDetail() == E3DDETAIL_ALLLINES) )
/*N*/ 	{
/*N*/ 		// Detailliert erzeugen
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// call parent
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	E3dObject::CreateWireframe(rWirePoly, pTf, eDetail);
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Objektdaten in Stream speichern
|*
\************************************************************************/

/*N*/ void E3dExtrudeObj::WriteData(SvStream& rOut) const
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
/*N*/ 	E3dCompoundObject::WriteData(rOut);
/*N*/ 
/*N*/ 	E3dIOCompat aCompat(rOut, STREAM_WRITE, 1);
/*N*/ 	rOut << aExtrudePolygon;
/*N*/ 	rOut << fExtrudeScale;
/*N*/ 
/*N*/ 	rOut << (double)GetExtrudeDepth();
/*N*/ 
/*N*/ 	rOut << (double)GetPercentBackScale() / 100.0;
/*N*/ 
/*N*/ 	rOut << (double)GetPercentDiagonal() / 200.0;
/*N*/ 
/*N*/ 	rOut << GetSmoothNormals(); // #107245# (BOOL)bExtrudeSmoothed;
/*N*/ 	rOut << GetSmoothLids(); // #107245# (BOOL)bExtrudeSmoothFrontBack;
/*N*/ 	rOut << GetCharacterMode(); // #107245# (BOOL)bExtrudeCharacterMode;
/*N*/ 
/*N*/ 	// Ab Version 513a (5.2.99): Parameter fuer das
/*N*/ 	// Erzeugen der Vorder/Rueckwand
/*N*/ 	rOut << GetCloseFront(); // #107245# (BOOL)bExtrudeCloseFront;
/*N*/ 	rOut << GetCloseBack(); // #107245# (BOOL)bExtrudeCloseBack;
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

/*N*/ void E3dExtrudeObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	E3dCompoundObject::ReadData(rHead, rIn);
/*N*/ 
/*N*/ 	// Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ 	BOOL bAllDone(FALSE);
/*N*/ 
/*N*/ 	if(AreBytesLeft())
/*N*/ 	{
/*N*/ 		E3dIOCompat aIoCompat(rIn, STREAM_READ);
/*N*/ 		if(aIoCompat.GetVersion() >= 1)
/*N*/ 		{
/*N*/ 			BOOL bTmp;
/*N*/ 			double fTmp;
/*N*/ 
/*N*/ 			rIn >> aExtrudePolygon;
/*N*/ 			rIn >> fExtrudeScale;
/*N*/ 
/*N*/ 			rIn >> fTmp;
/*N*/ 			mpObjectItemSet->Put(Svx3DDepthItem(sal_uInt32(fTmp + 0.5)));
/*N*/ 			
/*N*/ 			rIn >> fTmp;
/*N*/ 			mpObjectItemSet->Put(Svx3DBackscaleItem(sal_uInt16(fTmp * 100.0)));
/*N*/ 
/*N*/ 			rIn >> fTmp;
/*N*/ 			mpObjectItemSet->Put(Svx3DPercentDiagonalItem(sal_uInt16(fTmp * 200.0)));
/*N*/ 
/*N*/ 			rIn >> bTmp; // #107245# bExtrudeSmoothed = bTmp;
/*N*/ 			mpObjectItemSet->Put(Svx3DSmoothNormalsItem(bTmp));
/*N*/ 
/*N*/ 			rIn >> bTmp; // #107245# bExtrudeSmoothFrontBack = bTmp;
/*N*/ 			mpObjectItemSet->Put(Svx3DSmoothLidsItem(bTmp));
/*N*/ 
/*N*/ 			rIn >> bTmp; // #107245# bExtrudeCharacterMode = bTmp;
/*N*/ 			mpObjectItemSet->Put(Svx3DCharacterModeItem(bTmp));
/*N*/ 
/*N*/ 			bAllDone = TRUE;
/*N*/ 
/*N*/ 			if(aIoCompat.GetBytesLeft())
/*N*/ 			{
/*N*/ 				// Ab Version 513a (5.2.99): Parameter fuer das
/*N*/ 				// Erzeugen der Vorder/Rueckwand
/*N*/ 				BOOL bTmp;
/*N*/ 
/*N*/ 				rIn >> bTmp; // #107245# bExtrudeCloseFront = bTmp;
/*N*/ 				mpObjectItemSet->Put(Svx3DCloseFrontItem(bTmp));
/*N*/ 
/*N*/ 				rIn >> bTmp; // #107245# bExtrudeCloseBack = bTmp;
/*N*/ 				mpObjectItemSet->Put(Svx3DCloseBackItem(bTmp));
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				// #107245# bExtrudeCloseFront = TRUE;
/*N*/ 				mpObjectItemSet->Put(Svx3DCloseFrontItem(sal_True));
/*N*/ 				
/*N*/ 				// #107245# bExtrudeCloseBack = TRUE;
/*N*/ 				mpObjectItemSet->Put(Svx3DCloseBackItem(sal_True));
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if(!bAllDone)
/*N*/ 	{
/*N*/ 		// Geometrie aus geladenen PolyObj's rekonstruieren
/*N*/ 		SdrObjList* pSubList = GetSubList();
/*N*/ 		if(pSubList && pSubList->GetObjCount())
/*N*/ 		{
/*N*/ 			// Vorderseite und Rueckseite sind die ersten
/*N*/ 			// PolyObj's in der Liste, hole diese
/*N*/ 			E3dPolyObj* pFront = NULL;
/*N*/ 			E3dPolyObj* pBack = NULL;
/*N*/ 			E3dPolyObj* pOther = NULL;
/*N*/ 
/*N*/ 			UINT16 a;
/*N*/ 			for(a=0;a<pSubList->GetObjCount();a++)
/*N*/ 			{
/*N*/ 				E3dPolyObj* pCandidate = (E3dPolyObj*)pSubList->GetObj(a);
/*N*/ 				if(pCandidate->ISA(E3dPolyObj))
/*N*/ 				{
/*N*/ 					// Die Nromalen der Vorder/Rueckseiten zeigen in Z-Richtung,
/*N*/ 					// nutze dies aus
/*N*/ 					const Vector3D& rNormal = pCandidate->GetNormal();
/*N*/ 					if(fabs(rNormal.X()) < 0.0000001 && fabs(rNormal.Y()) < 0.0000001)
/*N*/ 					{
/*N*/ 						if(rNormal.Z() > 0.0)
/*N*/ 						{
/*N*/ 							// Vorderseite
/*N*/ 							pFront = pCandidate;
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 							// Rueckseite
/*N*/ 							pBack = pCandidate;
/*N*/ 						}
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						if(!pOther)
/*N*/ 							pOther = pCandidate;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			// Extrude-Tiefe feststellen
/*N*/ 			if(pOther)
/*N*/ 			{
/*N*/ 				const PolyPolygon3D& rOtherPoly = pOther->GetPolyPolygon3D();
/*N*/ 				// Hintereinanderliegende Paare in der alten Version waren
/*N*/ 				// 0,1 und 3,2 (0,3 vorne)
/*N*/ 				double fVal = (rOtherPoly[0][1] - rOtherPoly[0][0]).GetLength();
/*N*/ 				mpObjectItemSet->Put(Svx3DDepthItem(sal_uInt32(fVal + 0.5)));
/*N*/ 			}
/*N*/ 			else
/*N*/ 				// Einen Default vorsehen, kann aber eigentlich nie geschehen
/*N*/ 				mpObjectItemSet->Put(Svx3DDepthItem(100));
/*N*/ 
/*N*/ 			// Polygon fuer Vorderseite holen
/*N*/ 			if(pFront)
/*N*/ 			{
/*N*/ 				aExtrudePolygon = pFront->GetPolyPolygon3D();
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				if(pBack)
/*N*/ 				{
/*N*/ 					// Rueckseite benutzen und um -fExtrudeDepth in Z
/*N*/ 					// verschieben
/*N*/ 					aExtrudePolygon = pBack->GetPolyPolygon3D();
/*N*/ 					Matrix4D aMat;
/*N*/ 					aMat.Translate(Vector3D(0.0, 0.0, -(double)GetExtrudeDepth()));
/*N*/ 					aExtrudePolygon.Transform(aMat);
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					// Die Polygondaten koennen aus den Vorderkanten
/*N*/ 					// der weiteren Polygone (Punkte 0,3) restauriert werden.
/*N*/ 					// evtl. spaeter ergaenzen
/*N*/ 					aExtrudePolygon.Clear();
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			// Bestimmen, ob die Teilpolygone von aExtrudePolygon
/*N*/ 			// geschlossen waren. Sie waren geschlossen, wenn ein
/*N*/ 			// entsprechendes PolyObj existiert
/*N*/ 			for(a=0;a<aExtrudePolygon.Count();a++)
/*N*/ 			{
/*N*/ 				Polygon3D &rPoly = aExtrudePolygon[a];
/*N*/ 				USHORT nCnt = rPoly.GetPointCount();
/*N*/ 
/*N*/ 				if(nCnt)
/*N*/ 				{
/*N*/ 					Vector3D& rFirst = rPoly[0];
/*N*/ 					Vector3D& rLast = rPoly[nCnt - 1];
/*N*/ 					BOOL bClosePoly(FALSE);
/*N*/ 
/*N*/ 					for(UINT16 b=0;b<pSubList->GetObjCount();b++)
/*N*/ 					{
/*N*/ 						E3dPolyObj* pCandidate = (E3dPolyObj*)pSubList->GetObj(b);
/*N*/ 						if(pCandidate->ISA(E3dPolyObj)
/*N*/ 							&& pCandidate != pFront && pCandidate != pBack)
/*N*/ 						{
/*N*/ 							const PolyPolygon3D& rCandPoly = pCandidate->GetPolyPolygon3D();
/*N*/ 							if(rCandPoly[0].GetPointCount() > 2)
/*N*/ 							{
/*N*/ 								if(rCandPoly[0][0] == rFirst && rCandPoly[0][3] == rLast)
/*N*/ 									bClosePoly = TRUE;
/*N*/ 								if(rCandPoly[0][3] == rFirst && rCandPoly[0][0] == rLast)
/*N*/ 									bClosePoly = TRUE;
/*N*/ 							}
/*N*/ 						}
/*N*/ 					}
/*N*/ 
/*N*/ 					rPoly.SetClosed(bClosePoly);
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			// Setze die weiteren Parameter auf die defaults
/*N*/ 			fExtrudeScale = 1.0;
/*N*/ 
/*N*/ 			mpObjectItemSet->Put(Svx3DBackscaleItem(100));
/*N*/ 			
/*N*/ 			mpObjectItemSet->Put(Svx3DPercentDiagonalItem(10));
/*N*/ 
/*N*/ 			// #107245# bExtrudeSmoothed = TRUE;
/*N*/ 			mpObjectItemSet->Put(Svx3DSmoothNormalsItem(sal_True));
/*N*/ 
/*N*/ 			// #107245# bExtrudeSmoothFrontBack = FALSE;
/*N*/ 			mpObjectItemSet->Put(Svx3DSmoothLidsItem(sal_False));
/*N*/ 
/*N*/ 			// #107245# bExtrudeCharacterMode = FALSE;
/*N*/ 			mpObjectItemSet->Put(Svx3DCharacterModeItem(sal_False));
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// correct position of extrude polygon, in case it's not positioned
/*N*/ 	// at the Z==0 layer
/*N*/ 	if(aExtrudePolygon.Count() && aExtrudePolygon[0].GetPointCount())
/*N*/ 	{
/*N*/ 		const Vector3D& rFirstPoint = aExtrudePolygon[0][0];
/*N*/ 		if(rFirstPoint.Z() != 0.0)
/*N*/ 		{
/*N*/ 			// change transformation so that source poly lies in Z == 0,
/*N*/ 			// so it can be exported as 2D polygon
/*N*/ 			//
/*N*/ 			// ATTENTION: the translation has to be multiplied from LEFT
/*N*/ 			// SIDE since it was executed as the first translate for this 
/*N*/ 			// 3D object during it's creation.
/*N*/ 			double fTransDepth(rFirstPoint.Z());
/*N*/ 			Matrix4D aTransMat;
/*N*/ 			aTransMat.TranslateZ(fTransDepth);
/*N*/ 			NbcSetTransform(aTransMat * GetTransform());
/*N*/ 
/*N*/ 			// correct polygon itself
/*N*/ 			aTransMat.Identity();
/*N*/ 			aTransMat.TranslateZ(-fTransDepth);
/*N*/ 			aExtrudePolygon.Transform(aTransMat);
/*N*/ 		}
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

/*N*/ void E3dExtrudeObj::SetExtrudePolygon(const PolyPolygon3D &rNew)
/*N*/ {
/*N*/ 	if(aExtrudePolygon != rNew)
/*N*/ 	{
/*N*/ 		aExtrudePolygon = rNew;
/*N*/ 		bGeometryValid = FALSE;
/*N*/ 	}
/*N*/ }


// #107245# 
// void E3dExtrudeObj::SetExtrudeSmoothed(BOOL bNew)
// {
// 	if(bExtrudeSmoothed != bNew)
// 	{
// 		bExtrudeSmoothed = bNew;
// 		bGeometryValid = FALSE;
// 	}
// }

// #107245# 
// void E3dExtrudeObj::SetExtrudeSmoothFrontBack(BOOL bNew)
// {
// 	if(bExtrudeSmoothFrontBack != bNew)
// 	{
// 		bExtrudeSmoothFrontBack = bNew;
// 		bGeometryValid = FALSE;
// 	}
// }

// #107245# 
// void E3dExtrudeObj::SetExtrudeCharacterMode(BOOL bNew)
// {
// 	if(bExtrudeCharacterMode != bNew)
// 	{
// 		bExtrudeCharacterMode = bNew;
// 		bGeometryValid = FALSE;
// 	}
// }

// #107245# 
// void E3dExtrudeObj::SetExtrudeCloseFront(BOOL bNew)
// {
// 	if(bExtrudeCloseFront != bNew)
// 	{
// 		bExtrudeCloseFront = bNew;
// 		bGeometryValid = FALSE;
// 	}
// }

// #107245# 
// void E3dExtrudeObj::SetExtrudeCloseBack(BOOL bNew)
// {
// 	if(bExtrudeCloseBack != bNew)
// 	{
// 		bExtrudeCloseBack = bNew;
// 		bGeometryValid = FALSE;
// 	}
// }

//////////////////////////////////////////////////////////////////////////////
// private support routines for ItemSet access

/*N*/ void E3dExtrudeObj::PostItemChange(const sal_uInt16 nWhich)
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	E3dCompoundObject::PostItemChange(nWhich);
/*N*/ 
/*N*/ 	switch(nWhich)
/*N*/ 	{
/*N*/ 		case SDRATTR_3DOBJ_PERCENT_DIAGONAL:
/*N*/ 		{
/*N*/ 			bGeometryValid = FALSE;
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DOBJ_BACKSCALE:
/*N*/ 		{
/*N*/ 			bGeometryValid = FALSE;
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SDRATTR_3DOBJ_DEPTH:
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


/*************************************************************************
|*
|* Aufbrechen
|*
\************************************************************************/



// EOF
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
