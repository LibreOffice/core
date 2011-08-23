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

#ifndef _E3D_GLOBL3D_HXX
#define _E3D_GLOBL3D_HXX

#include <tools/solar.h>
namespace binfilter {

//#define ESODEBUG
#define E3D_STREAMING

const double PrecisionLimit = 1.0e-14;

const UINT32 E3dInventor = UINT32('E')*0x00000001+
                           UINT32('3')*0x00000100+
                           UINT32('D')*0x00010000+
                           UINT32('1')*0x01000000;

const UINT16 E3D_DISPLAYOBJ_ID	= 1;
const UINT16 E3D_SCENE_ID		= 2;
const UINT16 E3D_POLYSCENE_ID	= 3;
const UINT16 E3D_LIGHT_ID		= 5;
const UINT16 E3D_DISTLIGHT_ID	= 6;
const UINT16 E3D_POINTLIGHT_ID	= 7;
const UINT16 E3D_SPOTLIGHT_ID	= 8;
const UINT16 E3D_OBJECT_ID		= 10;
const UINT16 E3D_POLYOBJ_ID 	= 11;
const UINT16 E3D_CUBEOBJ_ID 	= 12;
const UINT16 E3D_SPHEREOBJ_ID	= 13;
const UINT16 E3D_POINTOBJ_ID	= 14;
const UINT16 E3D_EXTRUDEOBJ_ID	= 15;
const UINT16 E3D_LATHEOBJ_ID	= 16;
const UINT16 E3D_LABELOBJ_ID	= 20;
const UINT16 E3D_COMPOUNDOBJ_ID = 21;
const UINT16 E3D_POLYGONOBJ_ID 	= 22;

}//end of namespace binfilter
#endif		// _E3D_GLOBL3D_HXX
