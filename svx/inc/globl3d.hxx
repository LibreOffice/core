/*************************************************************************
 *
 *  $RCSfile: globl3d.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:00:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

const UINT16 E3D_DISPLAYOBJ_ID  = 1;
const UINT16 E3D_SCENE_ID       = 2;
const UINT16 E3D_POLYSCENE_ID   = 3;
const UINT16 E3D_LIGHT_ID       = 5;
const UINT16 E3D_DISTLIGHT_ID   = 6;
const UINT16 E3D_POINTLIGHT_ID  = 7;
const UINT16 E3D_SPOTLIGHT_ID   = 8;
const UINT16 E3D_OBJECT_ID      = 10;
const UINT16 E3D_POLYOBJ_ID     = 11;
const UINT16 E3D_CUBEOBJ_ID     = 12;
const UINT16 E3D_SPHEREOBJ_ID   = 13;
const UINT16 E3D_POINTOBJ_ID    = 14;
const UINT16 E3D_EXTRUDEOBJ_ID  = 15;
const UINT16 E3D_LATHEOBJ_ID    = 16;
const UINT16 E3D_LABELOBJ_ID    = 20;
const UINT16 E3D_COMPOUNDOBJ_ID = 21;
const UINT16 E3D_POLYGONOBJ_ID  = 22;



#endif      // _E3D_GLOBL3D_HXX
