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



#include "polyob3d.hxx"

#include "plight3d.hxx"
namespace binfilter {

/*N*/ TYPEINIT1(E3dPointLight, E3dLight);

/*************************************************************************
|*
|* E3dPointLight-Konstruktor
|*
\************************************************************************/

/*N*/ E3dPointLight::E3dPointLight(const Vector3D& rPos, const Color& rColor,
/*N*/ 							 double fLightIntensity) :
/*N*/ 	E3dLight(rPos, rColor, fLightIntensity)
/*N*/ {
/*N*/ 	CreateLightObj();
/*N*/ }

/*************************************************************************
|*
|* E3dPointLight-Destruktor
|*
\************************************************************************/

/*N*/ E3dPointLight::~E3dPointLight()
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/


/*************************************************************************
|*
|* Lichtstaerke an uebergeben Punkt bestimmen und die akkumulierte
|* Punktfarbe berechnen
|*
\************************************************************************/

/*N*/ FASTBOOL E3dPointLight::CalcLighting(Color& rNewColor,
/*N*/ 									 const Vector3D& rPnt,
/*N*/ 									 const Vector3D& rPntNormal,
/*N*/ 									 const Color& rPntColor)
/*N*/ {
/*N*/ 	double fR = 0, fG = 0, fB = 0;
/*N*/ 
/*N*/ 	if ( IsOn() )
/*N*/ 	{
/*N*/ 		double fLight;
/*N*/ 		Vector3D aPntToLight = GetTransPosition() - rPnt;
/*N*/ 
/*N*/ 		aPntToLight.Normalize();
/*N*/ 		fLight = rPntNormal.Scalar(aPntToLight);
/*N*/ 
/*N*/ 		if ( fLight > 0 )
/*N*/ 		{
/*N*/ 			fR = fLight * GetRed();
/*N*/ 			fG = fLight * GetGreen();
/*N*/ 			fB = fLight * GetBlue();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return ImpCalcLighting(rNewColor, rPntColor, fR, fG, fB);
/*N*/ }

/*************************************************************************
|*
|* Darstellung des Lichtobjekts in der Szene ein-/ausschalten
|*
\************************************************************************/

/*N*/ void E3dPointLight::CreateLightObj()
/*N*/ {
/*N*/ 	pSub->Clear();
/*N*/ 
/*N*/ 	if ( IsLightObjVisible() )
/*N*/ 	{
/*N*/ 		Vector3D aDiff(0.5,0,0);
/*N*/ 		Insert3DObj(new E3dPolyObj(GetPosition() - aDiff, GetPosition() + aDiff));
/*N*/ 		aDiff = Vector3D(0,0.5,0);
/*N*/ 		Insert3DObj(new E3dPolyObj(GetPosition() - aDiff, GetPosition() + aDiff));
/*N*/ 		aDiff = Vector3D(0,0,0.5);
/*N*/ 		Insert3DObj(new E3dPolyObj(GetPosition() - aDiff, GetPosition() + aDiff));
/*N*/ 		aDiff = Vector3D(0.35,0.35,0.35);
/*N*/ 		Insert3DObj(new E3dPolyObj(GetPosition() - aDiff, GetPosition() + aDiff));
/*N*/ 		aDiff = Vector3D(0.35,0.35,-0.35);
/*N*/ 		Insert3DObj(new E3dPolyObj(GetPosition() - aDiff, GetPosition() + aDiff));
/*N*/ 		aDiff = Vector3D(-0.35,0.35,-0.35);
/*N*/ 		Insert3DObj(new E3dPolyObj(GetPosition() - aDiff, GetPosition() + aDiff));
/*N*/ 		aDiff = Vector3D(-0.35,0.35,0.35);
/*N*/ 		Insert3DObj(new E3dPolyObj(GetPosition() - aDiff, GetPosition() + aDiff));
/*N*/ 	}
/*N*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
