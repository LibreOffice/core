/*************************************************************************
 *
 *  $RCSfile: deflt3d.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2001-01-26 14:01:07 $
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

#define ITEMID_COLOR            SID_ATTR_3D_LIGHTCOLOR

#ifndef _E3D_DEFLT3D_HXX
#include "deflt3d.hxx"
#endif

#ifndef _E3D_CUBE3D_HXX
#include "cube3d.hxx"
#endif

#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif

#ifndef _SVX_COLRITEM_HXX
#include "colritem.hxx"
#endif

#ifndef _SVXE3DITEM_HXX
#include "e3ditem.hxx"
#endif

/*************************************************************************
|*
|* Klasse zum verwalten der 3D-Default Attribute
|*
\************************************************************************/

// Konstruktor
E3dDefaultAttributes::E3dDefaultAttributes()
{
    Reset();
}

void E3dDefaultAttributes::Reset()
{
    // Compound-Objekt
//  aDefaultFrontMaterial;
//  aDefaultBackMaterial;
    bDefaultCreateNormals = TRUE;
    bDefaultCreateTexture = TRUE;
    bDefaultUseDifferentBackMaterial = FALSE;

    // Cube-Objekt
    aDefaultCubePos = Vector3D(-500.0, -500.0, -500.0);
    aDefaultCubeSize = Vector3D(1000.0, 1000.0, 1000.0);
    nDefaultCubeSideFlags = CUBE_FULL;
    bDefaultCubePosIsCenter = FALSE;

    // Sphere-Objekt
    aDefaultSphereCenter = Vector3D(0.0, 0.0, 0.0);
    aDefaultSphereSize = Vector3D(1000.0, 1000.0, 1000.0);

    // Lathe-Objekt
    nDefaultLatheEndAngle = 3600;
    fDefaultLatheScale = 1.0;
    bDefaultLatheSmoothed = TRUE;
    bDefaultLatheSmoothFrontBack = FALSE;
    bDefaultLatheCharacterMode = FALSE;
    bDefaultLatheCloseFront = TRUE;
    bDefaultLatheCloseBack = TRUE;

    // Extrude-Objekt
    fDefaultExtrudeScale = 1.0;
    bDefaultExtrudeSmoothed = TRUE;
    bDefaultExtrudeSmoothFrontBack = FALSE;
    bDefaultExtrudeCharacterMode = FALSE;
    bDefaultExtrudeCloseFront = TRUE;
    bDefaultExtrudeCloseBack = TRUE;

    // Scene-Objekt
//  aDefaultLightGroup;
    bDefaultDither = TRUE;
}
