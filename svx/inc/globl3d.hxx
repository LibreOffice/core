/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: globl3d.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:50:39 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _E3D_GLOBL3D_HXX
#define _E3D_GLOBL3D_HXX

#include <tools/solar.h>

//#define ESODEBUG
#define E3D_STREAMING

const double PrecisionLimit = 1.0e-14;

const UINT32 E3dInventor = UINT32('E')*0x00000001+
                           UINT32('3')*0x00000100+
                           UINT32('D')*0x00010000+
                           UINT32('1')*0x01000000;

//BFS01const UINT16 E3D_DISPLAYOBJ_ID   = 1;
const UINT16 E3D_SCENE_ID       = 2;
const UINT16 E3D_POLYSCENE_ID   = 3;
//BFS01const UINT16 E3D_LIGHT_ID        = 5;
//BFS01const UINT16 E3D_DISTLIGHT_ID    = 6;
//BFS01const UINT16 E3D_POINTLIGHT_ID   = 7;
//BFS01const UINT16 E3D_SPOTLIGHT_ID    = 8;
const UINT16 E3D_OBJECT_ID      = 10;
//BFS01const UINT16 E3D_POLYOBJ_ID  = 11;
const UINT16 E3D_CUBEOBJ_ID     = 12;
const UINT16 E3D_SPHEREOBJ_ID   = 13;
const UINT16 E3D_POINTOBJ_ID    = 14;
const UINT16 E3D_EXTRUDEOBJ_ID  = 15;
const UINT16 E3D_LATHEOBJ_ID    = 16;
const UINT16 E3D_LABELOBJ_ID    = 20;
const UINT16 E3D_COMPOUNDOBJ_ID = 21;
const UINT16 E3D_POLYGONOBJ_ID  = 22;



#endif      // _E3D_GLOBL3D_HXX
