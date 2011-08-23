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

#include "polyob3d.hxx"

#include "dlight3d.hxx"

namespace binfilter {

/*N*/ TYPEINIT1(E3dDistantLight, E3dLight);

/*************************************************************************
|*
|* E3dDistantLight-Konstruktor
|*
\************************************************************************/

/*N*/ E3dDistantLight::E3dDistantLight(const Vector3D& rPos, const Vector3D& rDir,
/*N*/ 								 const Color& rColor, double fLightIntensity) :
/*N*/ 	E3dLight(rPos, rColor, fLightIntensity)
/*N*/ {
/*N*/ 	SetDirection(rDir);
/*N*/ 	CreateLightObj();
/*N*/ }

/*************************************************************************
|*
|* E3dDistantLight-Destruktor
|*
\************************************************************************/

/*N*/ E3dDistantLight::~E3dDistantLight()
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

/*N*/ UINT16 E3dDistantLight::GetObjIdentifier() const
/*N*/ {
/*N*/ 	return E3D_DISTLIGHT_ID;
/*N*/ }

/*************************************************************************
|*
|* Richtung ZUR Lichtquelle setzen
|*
\************************************************************************/

/*N*/ void E3dDistantLight::SetDirection(const Vector3D& rNewDir)
/*N*/ {
/*N*/ 	if ( aDirection != rNewDir )
/*N*/ 	{
/*N*/ 		aDirection = rNewDir;
/*N*/ 		aDirection.Normalize();
/*N*/ 		CreateLightObj();
/*N*/ 		StructureChanged(this);
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Lichtstaerke an uebergeben Punkt bestimmen und die akkumulierte
|* Punktfarbe berechnen
|*
\************************************************************************/

/*?*/ FASTBOOL E3dDistantLight::CalcLighting(Color& rNewColor,
/*?*/ 										const Vector3D& rPnt,
/*?*/ 										const Vector3D& rPntNormal,
/*?*/ 										const Color& rPntColor)
/*?*/ {
/*?*/ 	double fR = 0, fG = 0, fB = 0;
/*?*/ 
/*?*/ 	if ( IsOn() )
/*?*/ 	{
/*?*/ 		double fLight = rPntNormal.Scalar(aDirection);
/*?*/ 
/*?*/ 		if ( fLight > 0 )
/*?*/ 		{
/*?*/ 			fR = fLight * GetRed();
/*?*/ 			fG = fLight * GetGreen();
/*?*/ 			fB = fLight * GetBlue();
/*?*/ 		}
/*?*/ 	}
/*?*/ 	return ImpCalcLighting(rNewColor, rPntColor, fR, fG, fB);
/*?*/ }

/*************************************************************************
|*
|* Darstellung des Lichtobjekts in der Szene ein-/ausschalten
|*
\************************************************************************/

/*N*/ void E3dDistantLight::CreateLightObj()
/*N*/ {
/*N*/ 	pSub->Clear();
/*N*/ 
/*N*/ 	if ( IsLightObjVisible() )
/*N*/ 	{
/*N*/ 		Insert3DObj(new E3dPolyObj(GetPosition() + aDirection, GetPosition()));
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* sichern
|*
\************************************************************************/

/*N*/ void E3dDistantLight::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	E3dLight::WriteData(rOut);
/*N*/ 
/*N*/ #ifdef E3D_STREAMING
/*N*/ 	SdrDownCompat aCompat(rOut, STREAM_WRITE);
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("E3dDistantLight");
/*N*/ #endif
/*N*/ 
/*N*/ 	rOut << aDirection;
/*N*/ #endif
/*N*/ #endif	// #ifndef SVX_LIGHT
/*N*/ }

/*************************************************************************
|*
|* laden
|*
\************************************************************************/

/*N*/ void E3dDistantLight::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	if (ImpCheckSubRecords (rHead, rIn))
/*N*/ 	{
/*N*/ 		E3dLight::ReadData(rHead, rIn);
/*N*/ 
/*N*/ 		SdrDownCompat aCompat(rIn, STREAM_READ);
/*N*/ #ifdef DBG_UTIL
/*N*/ 		aCompat.SetID("E3dDistantLight");
/*N*/ #endif
/*N*/ 
/*N*/ 		rIn >> aDirection;
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* Zuweisungsoperator
|*
\************************************************************************/



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
