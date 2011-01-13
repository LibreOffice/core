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

//#define ESODEBUG
#define E3D_STREAMING

const double PrecisionLimit = 1.0e-14;

const sal_uInt32 E3dInventor = sal_uInt32('E')*0x00000001+
                           sal_uInt32('3')*0x00000100+
                           sal_uInt32('D')*0x00010000+
                           sal_uInt32('1')*0x01000000;

const sal_uInt16 E3D_SCENE_ID       = 1;
const sal_uInt16 E3D_POLYSCENE_ID   = 2;
const sal_uInt16 E3D_OBJECT_ID      = 3; // should not be used, it's only a helper class for E3DScene and E3DCompoundObject
const sal_uInt16 E3D_CUBEOBJ_ID     = 4;
const sal_uInt16 E3D_SPHEREOBJ_ID   = 5;
const sal_uInt16 E3D_EXTRUDEOBJ_ID  = 6;
const sal_uInt16 E3D_LATHEOBJ_ID    = 7;
const sal_uInt16 E3D_COMPOUNDOBJ_ID = 8;
const sal_uInt16 E3D_POLYGONOBJ_ID  = 9;

#endif      // _E3D_GLOBL3D_HXX
