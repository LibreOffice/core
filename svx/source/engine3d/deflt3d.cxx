/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: deflt3d.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:55:07 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

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
