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




#ifndef _E3D_GLOBL3D_HXX
#include "globl3d.hxx"
#endif

#ifndef _SVDIO_HXX
#include "svdio.hxx"
#endif

#ifndef _E3D_POLYOB3D_HXX
#include "polyob3d.hxx"
#endif

#ifndef _E3D_LATHE3D_HXX
#include "lathe3d.hxx"
#endif



#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif



namespace binfilter {

/*N*/ TYPEINIT1(E3dLatheObj, E3dCompoundObject);

/*************************************************************************
|*
|* Konstruktor aus SV-Polygon, Scale gibt den Umrechnungsfaktor fuer
|* die Koordinaten an
|*
\************************************************************************/

/*N*/ E3dLatheObj::E3dLatheObj(E3dDefaultAttributes& rDefault, const PolyPolygon& rPoly)
/*N*/ :	E3dCompoundObject(rDefault),
/*N*/ 	aPolyPoly3D (PolyPolygon3D(rPoly, rDefault.GetDefaultLatheScale()))
/*N*/ {
/*N*/ 	// Defaults setzen
/*N*/ 	SetDefaultAttributes(rDefault);
/*N*/ 
/*N*/ 	// Ueberfluessige Punkte entfernen, insbesondere doppelte
/*N*/ 	// Start- und Endpunkte verhindern
/*N*/ 	aPolyPoly3D.RemoveDoublePoints();
/*N*/ 
/*N*/ 	const Polygon3D rPoly3D = aPolyPoly3D[0];
/*N*/ 	sal_uInt32 nSegCnt((sal_uInt32)rPoly3D.GetPointCount());
/*N*/ 	if(nSegCnt && !rPoly3D.IsClosed())
/*N*/ 		nSegCnt -= 1;
/*N*/ 	mpObjectItemSet->Put(Svx3DVerticalSegmentsItem(nSegCnt));
/*N*/ 
/*N*/ 	// Geometrie erzeugen
/*N*/ 	CreateGeometry();
/*N*/ }

/*************************************************************************
|*
|* Leer-Konstruktor
|*
\************************************************************************/

/*N*/ E3dLatheObj::E3dLatheObj()
/*N*/ :    aPolyPoly3D(Polygon3D())
/*N*/ {
/*N*/ 	// Defaults setzen
/*N*/ 	E3dDefaultAttributes aDefault;
/*N*/ 	SetDefaultAttributes(aDefault);
/*N*/ }

/*N*/ void E3dLatheObj::SetDefaultAttributes(E3dDefaultAttributes& rDefault)
/*N*/ {
/*N*/ 	// Defaults setzen
/*N*/ 	ImpForceItemSet();
/*N*/ 
/*N*/ 	fLatheScale = rDefault.GetDefaultLatheScale();
/*N*/ 
/*N*/ 	// #107245#
/*N*/ 	mpObjectItemSet->Put(Svx3DSmoothNormalsItem(rDefault.GetDefaultLatheSmoothed()));
/*N*/ 	mpObjectItemSet->Put(Svx3DSmoothLidsItem(rDefault.GetDefaultLatheSmoothFrontBack()));
/*N*/ 	mpObjectItemSet->Put(Svx3DCharacterModeItem(rDefault.GetDefaultLatheCharacterMode()));
/*N*/ 	mpObjectItemSet->Put(Svx3DCloseFrontItem(rDefault.GetDefaultLatheCloseFront()));
/*N*/ 	mpObjectItemSet->Put(Svx3DCloseBackItem(rDefault.GetDefaultLatheCloseBack()));
/*N*/ }

/*************************************************************************
|*
|* Give out simple line geometry
|*
\************************************************************************/


/*************************************************************************
|*
|* Die eigentliche Konstruktionmethode, erzeugt einen Koerper durch
|* Rotation des uebergebenen Polygons um die senkrechte Y-Achse. Wenn
|* nEndAngle < 3600 ist, werden ausserdem zwei Deckelflaechen-Polygone
|* erzeugt, die den Koerper abschliessen. Das Polygon sollte in der
|* XY-Ebene liegen, mit X-Koordinaten >= 0; wenn die Anfangs- und End-
|* X-Koordinaten nicht 0 sind, sollte das Polygon geschlossen sein.
|* Wenn bDblSided TRUE ist, werden die Rotationsflaechen doppelseitig
|* angelegt und keine Deckelflaechen erzeugt.
|*
\************************************************************************/

// Geometrieerzeugung
/*N*/ void E3dLatheObj::CreateGeometry()
/*N*/ {
/*N*/ 	// Start der Geometrieerzeugung ankuendigen
/*N*/ 	StartCreateGeometry();
/*N*/ 
/*N*/ 	// #78972#
/*N*/ 	maLinePolyPolygon.Clear(); 
/*N*/ 
/*N*/ 	// Polygon erzeugen
/*N*/ 	PolyPolygon3D aLathePoly3D(aPolyPoly3D);
/*N*/ 
/*N*/ 	// Eventuelle Anpassung der Segmentanzahlen
/*N*/ 	aLathePoly3D = CreateLathePolyPoly(aPolyPoly3D, GetVerticalSegments());
/*N*/ 
/*N*/ 	// Normale holen
/*N*/ 	Vector3D aNormal = aLathePoly3D.GetNormal();
/*N*/ 	if(aNormal.Z() > 0.0)
/*N*/ 	{
/*N*/ 		aLathePoly3D.FlipDirections();
/*N*/ 		aNormal = aLathePoly3D.GetNormal();
/*N*/ 	}
/*N*/ 
/*N*/ 	// Orientierung evtl. vorhandener Loecher in einen definierten
/*N*/ 	// Ausgangszustand bringen
/*N*/ 	aLathePoly3D.SetDirections();
/*N*/ 
/*N*/ 	// Spezialfall Einzelnes Polygon erzeugen
/*N*/ 	BOOL bSinglePoly = (GetEndAngle() == 0 || GetHorizontalSegments() == 0);
/*N*/ 	if(bSinglePoly)
/*N*/ 	{
/*?*/ 		// nur ein Polygon erzeugen
/*?*/ 		mpObjectItemSet->Put(Svx3DDoubleSidedItem(TRUE));
/*?*/ 
/*?*/ 		// Fuer evtl. selbst erzeugte Normalen
/*?*/ 		PolyPolygon3D aNormalsFront;
/*?*/ 
/*?*/ 		// Normalen und Vorderseite selbst erzeugen
/*?*/ 		AddFrontNormals(aLathePoly3D, aNormalsFront, aNormal);
/*?*/ 		CreateFront(aLathePoly3D, aNormalsFront, GetCreateNormals(), GetCreateTexture());
/*?*/ 
/*?*/ 		// #78972#
/*?*/ 		maLinePolyPolygon.Insert(aLathePoly3D);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// Eventuell doppelseitig erzeugen?
/*N*/ 		if(!aLathePoly3D.IsClosed())
/*N*/ 			mpObjectItemSet->Put(Svx3DDoubleSidedItem(TRUE));
/*N*/ 
/*N*/ 		// Seiten genenrieren?
/*N*/ 		BOOL bCreateSides = ((GetEndAngle() < 3600 && !GetDoubleSided()) 
/*N*/ 			|| (GetBackScale() != 100));
/*N*/ 
/*N*/ 		// Polygone vorbereiten
/*N*/ 		PolyPolygon3D aPrev, aFront, aBack, aNext;
/*N*/ 
/*N*/ 		// Rotation vorbereiten
/*N*/ 		double fAng = DEG2RAD(double(GetEndAngle()) / 10);
/*N*/ 		Matrix4D aRotMat;
/*N*/ 
/*N*/ 		// Skalierung vorbereiten
/*N*/ 		double fScalePerStep;
/*N*/ 		if(GetBackScale() != 100)
/*N*/ 			fScalePerStep = (((double)GetBackScale() - 100.0) / 100.0) / (double)GetHorizontalSegments();
/*N*/ 
/*N*/ 		// Texturen erzeugen?
/*N*/ 		double fTextureDepth=1.0;
/*N*/ 		double fTextureStart=0.0;
/*N*/ 		if(!GetCreateTexture())
/*N*/ 			fTextureStart = fTextureDepth = 0.0;
/*N*/ 
/*N*/ 		// aPrev bis aBack ausfuellen als Startvorbereitung
/*N*/ 		aRotMat.RotateY(-(fAng / (double)GetHorizontalSegments()));
/*N*/ 		aPrev = aLathePoly3D;
/*N*/ 		RotatePoly(aPrev, aRotMat);
/*N*/ 		if(GetBackScale() != 100)
/*N*/ 		{
/*N*/ 			ScalePoly(aPrev, 1.0 - fScalePerStep);
/*N*/ 		}
/*N*/ 		aRotMat.Identity();
/*N*/ 		aRotMat.RotateY(fAng / (double)GetHorizontalSegments());
/*N*/ 		aFront = aLathePoly3D;
/*N*/ 		aBack = aLathePoly3D;
/*N*/ 		RotatePoly(aBack, aRotMat);
/*N*/ 		if(GetBackScale() != 100)
/*N*/ 		{
/*N*/ 			ScalePoly(aBack, 1.0 + fScalePerStep);
/*N*/ 		}
/*N*/ 
/*N*/ 		// Werte fuer Textur-Zwischensegmenterzeugung berechnen
/*N*/ 		double fTmpStart = 0.0;
/*N*/ 		double fTmpLength = fTextureDepth / (double)GetHorizontalSegments();
/*N*/ 		sal_uInt16 nUpperBound = (sal_uInt16)GetHorizontalSegments();
/*N*/ 
/*N*/ 		for(UINT16 a=0;a<nUpperBound;a++)
/*N*/ 		{
/*N*/ 			// Naechstes Polygon vorbereiten
/*N*/ 			aNext = aLathePoly3D;
/*N*/ 
/*N*/ 			// Rotieren
/*N*/ 			if(!(a+2 == nUpperBound && GetEndAngle() == 3600))
/*N*/ 			{
/*N*/ 				aRotMat.Identity();
/*N*/ 				aRotMat.RotateY((fAng * (double)(a+2))/ (double)nUpperBound);
/*N*/ 				RotatePoly(aNext, aRotMat);
/*N*/ 			}
/*N*/ 
/*N*/ 			// Skalieren
/*N*/ 			if(GetBackScale() != 100)
/*N*/ 			{
/*N*/ 				ScalePoly(aNext, 1.0 + (fScalePerStep * (double)(a+2)));
/*N*/ 			}
/*N*/ 
/*N*/ 			// Jetzt Segment erzeugen
/*N*/ 			ImpCreateSegment(
/*N*/ 				aFront,
/*N*/ 				aBack,
/*N*/ 				&aPrev,
/*N*/ 				&aNext,
/*N*/ 				(a == 0) && bCreateSides && GetCloseFront(), // #107245# bLatheCloseFront,
/*N*/ 				(a == nUpperBound-1) && bCreateSides && GetCloseBack(), // #107245# bLatheCloseBack,
/*N*/ 				((double)GetPercentDiagonal() / 200.0)
/*N*/ 					* (double(nUpperBound) / 6.0),
/*N*/ 				GetSmoothNormals(), // #107245# GetLatheSmoothed(),
/*N*/ 				GetSmoothNormals(), // #107245# GetLatheSmoothed(),
/*N*/ 				GetSmoothLids(), // #107245# GetLatheSmoothFrontBack(),
/*N*/ 				1.0,
/*N*/ 				fTmpStart,
/*N*/ 				fTmpLength,
/*N*/ 				GetCreateTexture(),
/*N*/ 				GetCreateNormals(),
/*N*/ 				GetCharacterMode(), // #107245# GetLatheCharacterMode(),
/*N*/ 				TRUE,
/*N*/ 				// #78972#
/*N*/ 				&maLinePolyPolygon);
/*N*/ 
/*N*/ 			// naechsten Schritt vorbereiten
/*N*/ 			fTmpStart += fTmpLength;
/*N*/ 			aPrev = aFront;
/*N*/ 			aFront = aBack;
/*N*/ 			aBack = aNext;
/*N*/ 		}
/*N*/ 	}

    // #78972#
/*N*/ 	BOOL bClosedLines = ((GetEndAngle() == 3600) && (GetBackScale() == 100));
/*N*/ 	ImpCompleteLinePolygon(maLinePolyPolygon, aLathePoly3D.Count(), bClosedLines);
/*N*/ 
/*N*/ 	// call parent
/*N*/ 	E3dCompoundObject::CreateGeometry();
/*N*/ }

/*N*/ PolyPolygon3D E3dLatheObj::CreateLathePolyPoly(PolyPolygon3D& rPolyPoly3D, long nVSegs)
/*N*/ {
/*N*/ 	PolyPolygon3D aLathePolyPolygon3D = rPolyPoly3D;
/*N*/ 	sal_uInt16 nCnt = aLathePolyPolygon3D.Count();
/*N*/ 	sal_uInt16 nOrigSegmentCnt = aPolyPoly3D[0].GetPointCount();
/*N*/ 
/*N*/ 	if(nOrigSegmentCnt && !aPolyPoly3D[0].IsClosed())
/*N*/ 		nOrigSegmentCnt -= 1;
/*N*/ 
/*N*/ 	if(nVSegs && nVSegs != nOrigSegmentCnt)
/*N*/ 	{
/*N*/ 		// make sure minimum is not too small, 3 edges for closed
/*N*/ 		// and 2 edges for open obects
/*N*/ 		sal_Int32 nMinVSegs = aPolyPoly3D[0].IsClosed() ? 3 : 2;
/*N*/ 		if(nVSegs <= nMinVSegs)
/*N*/ 			nVSegs = nMinVSegs;
/*N*/ 
/*N*/ 		if(nVSegs != nOrigSegmentCnt)
/*N*/ 		{
/*N*/ 			// Erstes Polygon anpassen
/*N*/ 			aLathePolyPolygon3D[0] = CreateLathePoly(aLathePolyPolygon3D[0], nVSegs);
/*N*/ 			mpObjectItemSet->Put(Svx3DVerticalSegmentsItem(nVSegs));
/*N*/ 
/*N*/ 			// andere Polygone im richtigen Verhaeltnis anpassen,
/*N*/ 			// aber nur, wenn Wert fuer erstes angepasst werden musste
/*N*/ 			for(UINT16 i = 1; i < nCnt; i++ )
/*N*/ 			{
/*?*/ 				Polygon3D &rPoly3D = aLathePolyPolygon3D[i];
/*?*/ 				sal_uInt16 nSegCnt(rPoly3D.GetPointCount());
/*?*/ 				if(nSegCnt && !rPoly3D.IsClosed())
/*?*/ 					nSegCnt -= 1;
/*?*/ 				long nNewVSegs = (nSegCnt * nVSegs) / nOrigSegmentCnt;
/*?*/ 
/*?*/ 				// make sure min is not too small for subpolys, too
/*?*/ 				if(nNewVSegs <= nMinVSegs)
/*?*/ 					nNewVSegs = nMinVSegs;
/*?*/ 
/*?*/ 				if(nNewVSegs && nNewVSegs != nSegCnt)
/*?*/ 				{
/*?*/ 					aLathePolyPolygon3D[i] = CreateLathePoly(aLathePolyPolygon3D[i], nNewVSegs);
/*?*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return aLathePolyPolygon3D;
/*N*/ }

/*N*/ Polygon3D E3dLatheObj::CreateLathePoly(Polygon3D& rPoly3D, long nVSegs)
/*N*/ {
/*N*/ 	// attention: Here number of SEGMENTS is given, while GetExpandedPolygon()
/*N*/ 	// takes number of points. Calc PntNum first
/*N*/ 	long nNumPts = rPoly3D.IsClosed() ? nVSegs : nVSegs + 1;
/*N*/ 	if(nNumPts != rPoly3D.GetPointCount())
/*N*/ 		return rPoly3D.GetExpandedPolygon(nNumPts);
/*N*/ 	return rPoly3D;
/*N*/ }

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

/*N*/ UINT16 E3dLatheObj::GetObjIdentifier() const
/*N*/ {
/*N*/ 	return E3D_LATHEOBJ_ID;
/*N*/ }

/*************************************************************************
|*
|* Wireframe erzeugen
|*
\************************************************************************/

/*N*/ void E3dLatheObj::CreateWireframe(Polygon3D& rWirePoly, const Matrix4D* pTf,
/*N*/ 	E3dDragDetail eDetail)
/*N*/ {
/*N*/ 	// Nur selbst erzeugen, wenn alle Linien angezeigt werden sollen
/*N*/ 	if ( eDetail == E3DDETAIL_ALLLINES ||
/*N*/ 		(eDetail == E3DDETAIL_DEFAULT && GetDragDetail() == E3DDETAIL_ALLLINES) )
/*N*/ 	{
/*N*/ 		// Detailliert erzeugen
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// call parent
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 E3dObject::CreateWireframe(rWirePoly, pTf, eDetail);
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Zuweisungsoperator
|*
\************************************************************************/


/*************************************************************************
|*
|* Objektdaten in Stream speichern
|*
\************************************************************************/

/*N*/ void E3dLatheObj::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	long nVersion = rOut.GetVersion(); // Build_Nr * 10 z.B. 3810
/*N*/ 	if(nVersion < 3800)
/*N*/ 	{
/*N*/ 		// Alte Geometrie erzeugen, um die E3dPolyObj's zu haben
/*N*/ 		((E3dCompoundObject*)this)->ReCreateGeometry(TRUE);
/*N*/ 	}
/*N*/ 
/*N*/ 	// leider kann das E3dLatheObj nicht auf E3dObject abgestuetzt werden,
/*N*/ 	// da neue Member hinzugekommen sind und die Kompatibilitaet erhalten
/*N*/ 	// bleiben muss.
/*N*/ 	SdrAttrObj::WriteData(rOut);
/*N*/ 
/*N*/ 	// Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ 	SdrDownCompat aCompat(rOut, STREAM_WRITE);
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("E3dLatheObj");
/*N*/ #endif
/*N*/ 
/*N*/ 	pSub->Save(rOut);
/*N*/ 
/*N*/ 	// Parameter aus E3dObject speichern
/*N*/ 	rOut << aLocalBoundVol;
/*N*/ 	Old_Matrix3D aMat3D;
/*N*/ 	aMat3D = aTfMatrix;
/*N*/ 	rOut << aMat3D;
/*N*/ 	rOut << nLogicalGroup;
/*N*/ 	rOut << nObjTreeLevel;
/*N*/ 	rOut << nPartOfParent;
/*N*/ 	rOut << UINT16(eDragDetail);
/*N*/ 
/*N*/ 	// neue Member
/*N*/ 	// Alte version schreibt Polygon3D raus, neue Version
/*N*/ 	// benutzt dafuer das erste Teilpolygon von PolyPolygon3D
/*N*/ 	// rOut << aPolyPoly3D;
/*N*/ 	rOut << aPolyPoly3D[0];
/*N*/ 
/*N*/ 	rOut << GetHorizontalSegments();
/*N*/ 
/*N*/ 	rOut << GetEndAngle();
/*N*/ 
/*N*/ 	rOut << ((E3dLatheObj*)this)->GetDoubleSided();
/*N*/ 	rOut << fLatheScale;
/*N*/ 
/*N*/ 	// Ab Version 364f (19.06.97)
/*N*/ 
/*N*/ 	// #83965# internally the real number of segments (edges) is
/*N*/ 	// used, no longer the number of points
/*N*/ 	sal_Int32 nVSegs = GetVerticalSegments();
/*N*/ 	if(!aPolyPoly3D[0].IsClosed())
/*N*/ 		nVSegs += 1;
/*N*/ 	
/*N*/ 	rOut << nVSegs;
/*N*/ 
/*N*/ 	// Ab Version 374 (15.12.97)
/*N*/ 	rOut << aPolyPoly3D;
/*N*/ 
/*N*/ 	rOut << ((double)GetBackScale() / 100.0);
/*N*/ 	
/*N*/ 	rOut << ((double)GetPercentDiagonal() / 200.0);
/*N*/ 
/*N*/ 	rOut << GetSmoothNormals(); // #107245# (BOOL)bLatheSmoothed;
/*N*/ 	rOut << GetSmoothLids(); // #107245# (BOOL)bLatheSmoothFrontBack;
/*N*/ 	rOut << GetCharacterMode(); // #107245# (BOOL)bLatheCharacterMode;
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
/*N*/ 	// Ab Version 513a (5.2.99): Parameter fuer das
/*N*/ 	// Erzeugen der Vorder/Rueckwand
/*N*/ 	rOut << GetCloseFront(); // #107245# BOOL(bLatheCloseFront);
/*N*/ 	rOut << GetCloseBack(); // #107245# BOOL(bLatheCloseBack);
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

/*N*/ void E3dLatheObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	if (ImpCheckSubRecords (rHead, rIn))
/*N*/ 	{
/*N*/ 		// leider kann das E3dLatheObj nicht auf E3dObject abgestuetzt werden,
/*N*/ 		// da neue Member hinzugekommen sind und die Kompatibilitaet erhalten
/*N*/ 		// bleiben muss.
/*N*/ 		SdrAttrObj::ReadData(rHead, rIn);
/*N*/ 
/*N*/ 		// Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ 		SdrDownCompat aCompat(rIn, STREAM_READ);
/*N*/ #ifdef DBG_UTIL
/*N*/ 		aCompat.SetID("E3dLatheObj");
/*N*/ #endif
/*N*/ 		// dann die Member
/*N*/ 		UINT16	nTmp16;
/*N*/ 
/*N*/ 		// #106240# Flag if poly was loaded (all versions above 3.0 and 3.1)
/*N*/ 		sal_Bool bPolyWasRead(sal_False);
/*N*/ 
/*N*/ 		pSub->Load(rIn, *pPage);
/*N*/ 
/*N*/ 		// Parameter aus E3dObject laden
/*N*/ 		rIn >> aLocalBoundVol;
/*N*/ 		Old_Matrix3D aMat3D;
/*N*/ 		rIn >> aMat3D;
/*N*/ 		aTfMatrix = Matrix4D(aMat3D);
/*N*/ 		rIn >> nLogicalGroup;
/*N*/ 		rIn >> nObjTreeLevel;
/*N*/ 		rIn >> nPartOfParent;
/*N*/ 		rIn >> nTmp16; eDragDetail = E3dDragDetail(nTmp16);
/*N*/ 
/*N*/ 		// BoundVolume muss neu berechnet werden
/*N*/ 		bBoundVolValid = FALSE;
/*N*/ 
/*N*/ 		if (aCompat.GetBytesLeft ())
/*N*/ 		{
/*N*/ 			// neue Member
/*N*/ 			BOOL bTmp;
/*N*/ 			sal_Int32 nTmp;
/*N*/ 
/*N*/ 			// alte Version holt sich nur ein Polygon3D, wird hier durch
/*N*/ 			// Eintragen als erstes Teilpolygon geladen
/*N*/ 			// rIn >> aPolyPoly3D;
/*N*/ 			rIn >> aPolyPoly3D[0];
/*N*/ 
/*N*/ 			// #106240# OK, this file does have a saved polygon
/*N*/ 			bPolyWasRead = sal_True;
/*N*/ 
/*N*/ 			rIn >> nTmp;
/*N*/ 			mpObjectItemSet->Put(Svx3DHorizontalSegmentsItem(nTmp));
/*N*/ 			
/*N*/ 			rIn >> nTmp;
/*N*/ 			mpObjectItemSet->Put(Svx3DEndAngleItem(nTmp));
/*N*/ 
/*N*/ 			rIn >> bTmp;
/*N*/ 			mpObjectItemSet->Put(Svx3DDoubleSidedItem(bTmp));
/*N*/ 
/*N*/ 			rIn >> fLatheScale;
/*N*/ 		}
/*N*/ 
/*N*/ 		// #106240# No PolyPolygon as base for the lathe object was saved.
/*N*/ 		// Reconstruct it from the objects in the SubList.
/*N*/ 		if(!bPolyWasRead)
/*N*/ 		{
/*N*/ 			// This is really a old 3.0 or 3.1 file, reconstruct
/*N*/ 			// the not saved polygon using the SubList.
/*N*/ 			SdrObjList* pSubList = GetSubList();
/*N*/ 
/*N*/ 			if(pSubList && pSubList->GetObjCount())
/*N*/ 			{
/*N*/ 				sal_uInt16 nHorSegCount = (sal_uInt16)GetHorizontalSegments() / 2;
/*N*/ 				sal_uInt16 nVerSegCount = (sal_uInt16)(pSubList->GetObjCount() / nHorSegCount);
/*N*/ 				Polygon3D aNewBasePoly;
/*N*/ 
/*N*/ 				for(sal_uInt16 a(0); a < nVerSegCount; a++)
/*N*/ 				{
/*N*/ 					E3dPolyObj* pCandidate = (E3dPolyObj*)pSubList->GetObj(a * nHorSegCount);
/*N*/ 					if(pCandidate->ISA(E3dPolyObj))
/*N*/ 					{
/*N*/ 						const PolyPolygon3D& rCandPoly = ((E3dPolyObj*)pCandidate)->GetPolyPolygon3D();
/*N*/ 
/*N*/ 						if(rCandPoly[0].GetPointCount() > 1)
/*N*/ 						{
/*N*/ 							aNewBasePoly[a] = rCandPoly[0][1];
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 				aPolyPoly3D.Clear();
/*N*/ 				aNewBasePoly.SetClosed(sal_True);
/*N*/ 				aPolyPoly3D.Insert(aNewBasePoly);
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		if (aCompat.GetBytesLeft())
/*N*/ 		{
/*N*/ 			// Ab Version 364f (19.06.97)
/*N*/ 			sal_Int32 nTmp;
/*N*/ 			rIn >> nTmp;
/*N*/ 
/*N*/ 			// #83965# internally the real number of segments (edges) is
/*N*/ 			// used, no longer the number of points
/*N*/ 			if(!aPolyPoly3D[0].IsClosed())
/*N*/ 				nTmp -= 1;
/*N*/ 			
/*N*/ 			mpObjectItemSet->Put(Svx3DVerticalSegmentsItem(nTmp));
/*N*/ 		}
/*N*/ 
/*N*/ 		if (aCompat.GetBytesLeft())
/*N*/ 		{
/*N*/ 			// Ab Version 374 (15.12.97)
/*N*/ 			// Gesamtes PolyPolygon laden
/*N*/ 			BOOL bTmp;
/*N*/ 			double fTmp;
/*N*/ 
/*N*/ 			aPolyPoly3D.Clear();
/*N*/ 			rIn >> aPolyPoly3D;
/*N*/ 
/*N*/ 			rIn >> fTmp;
/*N*/ 			mpObjectItemSet->Put(Svx3DBackscaleItem((sal_uInt16)(fTmp * 100.0)));
/*N*/ 			
/*N*/ 			rIn >> fTmp;
/*N*/ 			mpObjectItemSet->Put(Svx3DPercentDiagonalItem(sal_uInt16(fTmp * 200.0)));
/*N*/ 
/*N*/ 			rIn >> bTmp; // #107245# bLatheSmoothed = bTmp;
/*N*/ 			mpObjectItemSet->Put(Svx3DSmoothNormalsItem(bTmp));
/*N*/ 
/*N*/ 			rIn >> bTmp; // #107245# bLatheSmoothFrontBack = bTmp;
/*N*/ 			mpObjectItemSet->Put(Svx3DSmoothLidsItem(bTmp));
/*N*/ 			
/*N*/ 			rIn >> bTmp; // #107245# bLatheCharacterMode = bTmp;
/*N*/ 			mpObjectItemSet->Put(Svx3DCharacterModeItem(bTmp));
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// Geometrie aus erzeugten PolyObj's rekonstruieren
/*N*/ 			mpObjectItemSet->Put(Svx3DBackscaleItem(100));
/*N*/ 
/*N*/ 			mpObjectItemSet->Put(Svx3DPercentDiagonalItem(10));
/*N*/ 
/*N*/ 			// #107245# bLatheSmoothed = TRUE;
/*N*/ 			mpObjectItemSet->Put(Svx3DSmoothNormalsItem(sal_True));
/*N*/ 
/*N*/ 			// #107245# bLatheSmoothFrontBack = FALSE;
/*N*/ 			mpObjectItemSet->Put(Svx3DSmoothLidsItem(sal_False));
/*N*/ 
/*N*/ 			// #107245# bLatheCharacterMode = FALSE;
/*N*/ 			mpObjectItemSet->Put(Svx3DCharacterModeItem(sal_False));
/*N*/ 		}
/*N*/ 
/*N*/ 		if (aCompat.GetBytesLeft())
/*N*/ 		{
/*N*/ 			// Ab Version 395 (8.6.98): Parameter aus dem Objekt
/*N*/ 			// E3dCompoundObject. Da irgendwann mal jemand die Ableitungs-
/*N*/ 			// hierarchie beim FileFormat unterbrochen hat, wurden diese Attribute
/*N*/ 			// bisher NOCH NIE gespeichert (Grrr). Diese Stelle muss nun natuerlich
/*N*/ 			// auch IMMER MITGEPFLEGT werden, wenn sich Parameter in
/*N*/ 			// E3dCompoundObject oder E3dObject aendern.
/*N*/ 			BOOL bTmp, bTmp2;
/*N*/ 			sal_uInt16 nTmp;
/*N*/ 
/*N*/ 			rIn >> bTmp; 
/*N*/ 			mpObjectItemSet->Put(Svx3DDoubleSidedItem(bTmp));
/*N*/ 
/*N*/ 			rIn >> bTmp; bCreateNormals = bTmp;
/*N*/ 			rIn >> bTmp; bCreateTexture = bTmp;
/*N*/ 
/*N*/ 			rIn >> bTmp; 
/*N*/ 			rIn >> bTmp2;
/*N*/ 			if(bTmp == FALSE && bTmp2 == FALSE)
/*N*/ 				nTmp = 0;
/*N*/ 			else if(bTmp == TRUE && bTmp2 == FALSE)
/*N*/ 				nTmp = 1;
/*N*/ 			else
/*N*/ 				nTmp = 2;
/*N*/ 			mpObjectItemSet->Put(Svx3DNormalsKindItem(nTmp));
/*N*/ 
/*N*/ 			rIn >> bTmp; 
/*N*/ 			rIn >> bTmp2;
/*N*/ 			if(bTmp == FALSE && bTmp2 == FALSE)
/*N*/ 				nTmp = 0;
/*N*/ 			else if(bTmp == TRUE && bTmp2 == FALSE)
/*N*/ 				nTmp = 1;
/*N*/ 			else
/*N*/ 				nTmp = 2;
/*N*/ 			mpObjectItemSet->Put(Svx3DTextureProjectionXItem(nTmp));
/*N*/ 
/*N*/ 			rIn >> bTmp; 
/*N*/ 			rIn >> bTmp2;
/*N*/ 			if(bTmp == FALSE && bTmp2 == FALSE)
/*N*/ 				nTmp = 0;
/*N*/ 			else if(bTmp == TRUE && bTmp2 == FALSE)
/*N*/ 				nTmp = 1;
/*N*/ 			else
/*N*/ 				nTmp = 2;
/*N*/ 			mpObjectItemSet->Put(Svx3DTextureProjectionYItem(nTmp));
/*N*/ 
/*N*/ 			rIn >> bTmp; 
/*N*/ 			mpObjectItemSet->Put(Svx3DShadow3DItem(bTmp));
/*N*/ 
/*N*/ 			Color aCol;
/*N*/ 			
/*N*/ 			rIn >> aCol;
/*N*/ 			SetMaterialAmbientColor(aCol);
/*N*/ 			
/*N*/ 			rIn >> aCol;
/*N*/ 			// do NOT use, this is the old 3D-Color(!)
/*N*/ 			// SetItem(XFillColorItem(String(), aCol));
/*N*/ 			
/*N*/ 			rIn >> aCol;
/*N*/ 			mpObjectItemSet->Put(Svx3DMaterialSpecularItem(aCol));
/*N*/ 			
/*N*/ 			rIn >> aCol;
/*N*/ 			mpObjectItemSet->Put(Svx3DMaterialEmissionItem(aCol));
/*N*/ 			
/*N*/ 			rIn >> nTmp;
/*N*/ 			mpObjectItemSet->Put(Svx3DMaterialSpecularIntensityItem(nTmp));
/*N*/ 
/*N*/ 			aBackMaterial.ReadData(rIn);
/*N*/ 
/*N*/ 			rIn >> nTmp; 
/*N*/ 			mpObjectItemSet->Put(Svx3DTextureKindItem(nTmp));
/*N*/ 
/*N*/ 			rIn >> nTmp; 
/*N*/ 			mpObjectItemSet->Put(Svx3DTextureModeItem(nTmp));
/*N*/ 
/*N*/ 			rIn >> bTmp; 
/*N*/ 			mpObjectItemSet->Put(Svx3DNormalsInvertItem(bTmp));
/*N*/ 		}
/*N*/ 
/*N*/ 		if (aCompat.GetBytesLeft())
/*N*/ 		{
/*N*/ 			// Ab Version 513a (5.2.99): Parameter fuer das
/*N*/ 			// Erzeugen der Vorder/Rueckwand
/*N*/ 			BOOL bTmp;
/*N*/ 
/*N*/ 			rIn >> bTmp; // #107245# bLatheCloseFront = bTmp;
/*N*/ 			mpObjectItemSet->Put(Svx3DCloseFrontItem(bTmp));
/*N*/ 
/*N*/ 			rIn >> bTmp; // #107245# bLatheCloseBack = bTmp;
/*N*/ 			mpObjectItemSet->Put(Svx3DCloseBackItem(bTmp));
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// #107245# bLatheCloseFront = TRUE;
/*N*/ 			mpObjectItemSet->Put(Svx3DCloseFrontItem(sal_True));
/*N*/ 
/*N*/ 			// #107245# bLatheCloseBack = TRUE;
/*N*/ 			mpObjectItemSet->Put(Svx3DCloseBackItem(sal_True));
/*N*/ 		}
/*N*/ 
/*N*/ 		// neu ab 534: (hat noch gefehlt)
/*N*/ 		if (aCompat.GetBytesLeft () >= sizeof (BOOL))
/*N*/ 		{
/*N*/ 			BOOL bTmp;
/*N*/ 			rIn >> bTmp; 
/*N*/ 			mpObjectItemSet->Put(Svx3DTextureFilterItem(bTmp));
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// correct position of extrude polygon, in case it's not positioned
/*N*/ 	// at the Z==0 layer
/*N*/ 	if(aPolyPoly3D.Count() && aPolyPoly3D[0].GetPointCount())
/*N*/ 	{
/*N*/ 		const Vector3D& rFirstPoint = aPolyPoly3D[0][0];
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
/*N*/ 			aPolyPoly3D.Transform(aTransMat);
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// Geometrie neu erzeugen
/*N*/ 	ReCreateGeometry();
/*N*/ }

/*************************************************************************
|*
|* Wandle das Objekt in ein Gruppenobjekt bestehend aus n Polygonen
|*
\************************************************************************/


/*************************************************************************
|*
|* Neue Segmentierung (Beschreibung siehe Header-File)
|*
\************************************************************************/


/*************************************************************************
|*
|* Lokale Parameter setzen mit Geometrieneuerzeugung
|*
\************************************************************************/

/*N*/ void E3dLatheObj::SetPolyPoly3D(const PolyPolygon3D& rNew)
/*N*/ {
/*N*/ 	if(aPolyPoly3D != rNew)
/*N*/ 	{
/*N*/ 		aPolyPoly3D = rNew;
/*N*/ 
/*N*/ 		// #83965# take care of vertical segments, too.
/*N*/ 		sal_Int32 nNumVSegs = aPolyPoly3D[0].GetPointCount();
/*N*/ 		if(!aPolyPoly3D[0].IsClosed())
/*N*/ 			nNumVSegs -= 1;
/*N*/ 		ImpForceItemSet();
/*N*/ 		mpObjectItemSet->Put(Svx3DVerticalSegmentsItem(nNumVSegs));
/*N*/ 		
/*N*/ 		bGeometryValid = FALSE;
/*N*/ 	}
/*N*/ }


// #107245# 
// void E3dLatheObj::SetLatheSmoothed(BOOL bNew)
// {
// 	if(bLatheSmoothed != bNew)
// 	{
// 		bLatheSmoothed = bNew;
// 		bGeometryValid = FALSE;
// 	}
// }

// #107245# 
// void E3dLatheObj::SetLatheSmoothFrontBack(BOOL bNew)
// {
// 	if(bLatheSmoothFrontBack != bNew)
// 	{
// 		bLatheSmoothFrontBack = bNew;
// 		bGeometryValid = FALSE;
// 	}
// }

// #107245# 
// void E3dLatheObj::SetLatheCharacterMode(BOOL bNew)
// {
// 	if(bLatheCharacterMode != bNew)
// 	{
// 		bLatheCharacterMode = bNew;
// 		bGeometryValid = FALSE;
// 	}
// }

// #107245# 
// void E3dLatheObj::SetLatheCloseFront(BOOL bNew)
// {
// 	if(bLatheCloseFront != bNew)
// 	{
// 		bLatheCloseFront = bNew;
// 		bGeometryValid = FALSE;
// 	}
// }

// #107245# 
// void E3dLatheObj::SetLatheCloseBack(BOOL bNew)
// {
// 	if(bLatheCloseBack != bNew)
// 	{
// 		bLatheCloseBack = bNew;
// 		bGeometryValid = FALSE;
// 	}
// }

//////////////////////////////////////////////////////////////////////////////
// private support routines for ItemSet access

/*N*/ void E3dLatheObj::PostItemChange(const sal_uInt16 nWhich)
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	E3dCompoundObject::PostItemChange(nWhich);
/*N*/ 
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
/*N*/ 		case SDRATTR_3DOBJ_END_ANGLE:
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
