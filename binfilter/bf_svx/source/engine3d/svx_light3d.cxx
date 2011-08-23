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

#include "svdio.hxx"

#include "globl3d.hxx"

#include "light3d.hxx"

namespace binfilter {

/*N*/ TYPEINIT1(E3dLight, E3dPointObj);

/*************************************************************************
|*
|* E3dLight-Konstruktor
|*
\************************************************************************/

/*N*/ E3dLight::E3dLight(const Vector3D& rPos, const Color& rColor,
/*N*/ 					double fLightIntensity) :
/*N*/ 	E3dPointObj(rPos),
/*N*/ 	fIntensity(fLightIntensity),
/*N*/ 	bOn(TRUE),
/*N*/ 	bVisible(FALSE)
/*N*/ {
/*N*/ 	SetColor(rColor);
/*N*/ 	aLocalBoundVol = Volume3D();
/*N*/ 	bBoundVolValid = FALSE;
/*N*/ }

/*************************************************************************
|*
|* E3dLight-Konstruktor 2
|*
\************************************************************************/

/*N*/ E3dLight::E3dLight() :
/*N*/ 	E3dPointObj(Vector3D()),
/*N*/ 
/*N*/ 	aColor(255, 255, 255),
/*N*/ 
/*N*/ 	fRed(1.0),
/*N*/ 	fGreen(1.0),
/*N*/ 	fBlue(1.0),
/*N*/ 	fIntensity(1.0),
/*N*/ 	bOn(TRUE),
/*N*/ 	bVisible(FALSE)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* protected, daher kein Einfuegen von Objekten von aussen moeglich
|*
\************************************************************************/

/*N*/ void E3dLight::Insert3DObj(E3dObject* p3DObj)
/*N*/ {
/*N*/ //	E3dPointObj::Insert3DObj(p3DObj);
/*N*/ 	GetSubList()->InsertObject(p3DObj);
/*N*/ }

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

/*N*/ UINT16 E3dLight::GetObjIdentifier() const
/*N*/ {
/*N*/ 	return E3D_LIGHT_ID;
/*N*/ }

/*************************************************************************
|*
|* Lichtstaerke an uebergeben Punkt bestimmen und die akkumulierte
|* Punktfarbe berechnen; Return = TRUE bedeutet, dass die maximale
|* Farbintensitaet aller Farbanteile erreicht ist.
|*
\************************************************************************/

/*N*/ FASTBOOL E3dLight::ImpCalcLighting(Color& rNewColor, const Color& rPntColor,
/*N*/ 									double fR, double fG, double fB) const
/*N*/ {
/*N*/ 	ULONG	nR(rNewColor.GetRed()),
/*N*/ 			nG(rNewColor.GetGreen()),
/*N*/ 			nB(rNewColor.GetBlue());
/*N*/ 	ULONG	nPntR(rPntColor.GetRed()),
/*N*/ 			nPntG(rPntColor.GetGreen()),
/*N*/ 			nPntB(rPntColor.GetBlue());
/*N*/ 
/*N*/ 	if ( bOn )
/*N*/ 	{
/*N*/ 		nR += (ULONG) (fR * nPntR);
/*N*/ 		nG += (ULONG) (fG * nPntG);
/*N*/ 		nB += (ULONG) (fB * nPntB);
/*N*/ 
/*N*/ 		nR = Min(nR, nPntR);
/*N*/ 		nG = Min(nG, nPntG);
/*N*/ 		nB = Min(nB, nPntB);
/*N*/ 
/*N*/ 		rNewColor.SetRed((USHORT) nR);
/*N*/ 		rNewColor.SetGreen((USHORT) nG);
/*N*/ 		rNewColor.SetBlue((USHORT) nB);
/*N*/ 	}
/*N*/ 	return ( nR == nPntR && nG == nPntG && nB == nPntB );
/*N*/ }

/*************************************************************************
|*
|* Lichtstaerke an uebergeben Punkt bestimmen und die akkumulierte
|* Punktfarbe berechnen
|*
\************************************************************************/

/*N*/ FASTBOOL E3dLight::CalcLighting(Color& rNewColor,
/*N*/ 								const Vector3D& rPnt,
/*N*/ 								const Vector3D& rPntNormal,
/*N*/ 								const Color& rPntColor)
/*N*/ {
/*N*/ 	return ImpCalcLighting(rNewColor, rPntColor, fRed, fGreen, fBlue);
/*N*/ }

/*************************************************************************
|*
|* neue Position setzen und Aenderung an Parents (Szene) melden
|*
\************************************************************************/

/*N*/ void E3dLight::SetPosition(const Vector3D& rNewPos)
/*N*/ {
/*N*/ 	E3dPointObj::SetPosition(rNewPos);
/*N*/ 	CreateLightObj();
/*N*/ }

/*************************************************************************
|*
|* neue Intensitaet setzen und Aenderung an Parents (Szene) melden
|*
\************************************************************************/

/*N*/ void E3dLight::SetIntensity(double fNew)
/*N*/ {
/*N*/ 	if ( fIntensity != fNew )
/*N*/ 	{
/*N*/ 		fIntensity = fNew;
/*N*/ 		StructureChanged(this);
/*N*/ 		SetColor(aColor);
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* neue Farbe setzen und Aenderung an Parents (Szene) melden
|*
\************************************************************************/

/*N*/ void E3dLight::SetColor(const Color& rNewColor)
/*N*/ {
/*N*/ 	if ( aColor != rNewColor )
/*N*/ 	{
/*N*/ 		aColor = rNewColor;
/*N*/ 		StructureChanged(this);
/*N*/ 	}
/*N*/ 
/*N*/ 	fRed    = fIntensity * aColor.GetRed()   / 255;
/*N*/ 	fGreen  = fIntensity * aColor.GetGreen() / 255;
/*N*/ 	fBlue   = fIntensity * aColor.GetBlue()  / 255;
/*N*/ }

/*************************************************************************
|*
|* Lichtquelle ein-/ausschalten und Aenderung an Parents (Szene) melden
|*
\************************************************************************/


/*************************************************************************
|*
|* Darstellung des Lichtobjekts in der Szene ein-/ausschalten
|*
\************************************************************************/


/*************************************************************************
|*
|* Darstellung des Lichtobjekts in der Szene ein-/ausschalten
|*
\************************************************************************/

/*N*/ void E3dLight::CreateLightObj()
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* sichern
|*
\************************************************************************/

/*N*/ void E3dLight::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	E3dPointObj::WriteData(rOut);
/*N*/ 
/*N*/ #ifdef E3D_STREAMING
/*N*/ 
/*N*/ 	SdrDownCompat aCompat(rOut, STREAM_WRITE);
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("E3dLight");
/*N*/ #endif
/*N*/ 
/*N*/ 	rOut << aColor;
/*N*/ 	rOut << fIntensity;
/*N*/ 	rOut << fRed;
/*N*/ 	rOut << fGreen;
/*N*/ 	rOut << fBlue;
/*N*/ 	rOut << BOOL(bOn);
/*N*/ 	rOut << BOOL(bVisible);
/*N*/ #endif
/*N*/ #endif	// #ifndef SVX_LIGHT
/*N*/ }

/*************************************************************************
|*
|* laden
|*
\************************************************************************/

/*N*/ void E3dLight::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	if (ImpCheckSubRecords (rHead, rIn))
/*N*/ 	{
/*N*/ 		E3dPointObj::ReadData(rHead, rIn);
/*N*/ 
/*N*/ 		SdrDownCompat aCompat(rIn, STREAM_READ);
/*N*/ #ifdef DBG_UTIL
/*N*/ 		aCompat.SetID("E3dLight");
/*N*/ #endif
/*N*/ 
/*N*/ 		BOOL bTmp;
/*N*/ 
/*N*/ 		rIn >> aColor;
/*N*/ 		rIn >> fIntensity;
/*N*/ 		rIn >> fRed;
/*N*/ 		rIn >> fGreen;
/*N*/ 		rIn >> fBlue;
/*N*/ 		rIn >> bTmp; bOn = bTmp;
/*N*/ 		rIn >> bTmp; bVisible = bTmp;
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Zuweisungsoperator
|*
\************************************************************************/


/*************************************************************************
|*
|* Attribute setzen
|*
\************************************************************************/


/*************************************************************************
|*
|* Attribute setzen
|*
\************************************************************************/

/*N*/ void E3dLight::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Attribute abfragen
|*
\************************************************************************/

/*N*/ SfxStyleSheet*E3dLight::GetStyleSheet() const
/*N*/ {
/*N*/ 	return 0;
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
