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

#define ITEMID_COLOR			SID_ATTR_3D_LIGHTCOLOR


#ifndef _E3D_CUBE3D_HXX
#include "cube3d.hxx"
#endif

#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif


namespace binfilter {

/*************************************************************************
|*
|* Klasse zum verwalten der 3D-Default Attribute
|*
\************************************************************************/

// Konstruktor
/*N*/ E3dDefaultAttributes::E3dDefaultAttributes()
/*N*/ {
/*N*/ 	Reset();
/*N*/ }

/*N*/ void E3dDefaultAttributes::Reset()
/*N*/ {
/*N*/ 	// Compound-Objekt
/*N*/ //	aDefaultFrontMaterial;
/*N*/ //	aDefaultBackMaterial;
/*N*/ 	bDefaultCreateNormals = TRUE;
/*N*/ 	bDefaultCreateTexture = TRUE;
/*N*/ 	bDefaultUseDifferentBackMaterial = FALSE;
/*N*/ 
/*N*/ 	// Cube-Objekt
/*N*/ 	aDefaultCubePos = Vector3D(-500.0, -500.0, -500.0);
/*N*/ 	aDefaultCubeSize = Vector3D(1000.0, 1000.0, 1000.0);
/*N*/ 	nDefaultCubeSideFlags = CUBE_FULL;
/*N*/ 	bDefaultCubePosIsCenter = FALSE;
/*N*/ 
/*N*/ 	// Sphere-Objekt
/*N*/ 	aDefaultSphereCenter = Vector3D(0.0, 0.0, 0.0);
/*N*/ 	aDefaultSphereSize = Vector3D(1000.0, 1000.0, 1000.0);
/*N*/ 
/*N*/ 	// Lathe-Objekt
/*N*/ 	nDefaultLatheEndAngle = 3600;
/*N*/ 	fDefaultLatheScale = 1.0;
/*N*/ 	bDefaultLatheSmoothed = TRUE;
/*N*/ 	bDefaultLatheSmoothFrontBack = FALSE;
/*N*/ 	bDefaultLatheCharacterMode = FALSE;
/*N*/ 	bDefaultLatheCloseFront = TRUE;
/*N*/ 	bDefaultLatheCloseBack = TRUE;
/*N*/ 
/*N*/ 	// Extrude-Objekt
/*N*/ 	fDefaultExtrudeScale = 1.0;
/*N*/ 	bDefaultExtrudeSmoothed = TRUE;
/*N*/ 	bDefaultExtrudeSmoothFrontBack = FALSE;
/*N*/ 	bDefaultExtrudeCharacterMode = FALSE;
/*N*/ 	bDefaultExtrudeCloseFront = TRUE;
/*N*/ 	bDefaultExtrudeCloseBack = TRUE;
/*N*/ 
/*N*/ 	// Scene-Objekt
/*N*/ //	aDefaultLightGroup;
/*N*/ 	bDefaultDither = TRUE;
/*N*/ }
}
