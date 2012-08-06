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


#include <svx/deflt3d.hxx>
#include <svx/cube3d.hxx>
#include <svx/svxids.hrc>
#include <editeng/colritem.hxx>
#include <svx/e3ditem.hxx>

// Class to manage the 3D default attributes

E3dDefaultAttributes::E3dDefaultAttributes()
{
    Reset();
}

void E3dDefaultAttributes::Reset()
{
    // Compound object
    bDefaultCreateNormals = sal_True;
    bDefaultCreateTexture = sal_True;

    // Cube object
    aDefaultCubePos = basegfx::B3DPoint(-500.0, -500.0, -500.0);
    aDefaultCubeSize = basegfx::B3DVector(1000.0, 1000.0, 1000.0);
    nDefaultCubeSideFlags = CUBE_FULL;
    bDefaultCubePosIsCenter = sal_False;

    // Sphere object
    aDefaultSphereCenter = basegfx::B3DPoint(0.0, 0.0, 0.0);
    aDefaultSphereSize = basegfx::B3DPoint(1000.0, 1000.0, 1000.0);

    // Lathe object
    nDefaultLatheEndAngle = 3600;
    bDefaultLatheSmoothed = sal_True;
    bDefaultLatheSmoothFrontBack = sal_False;
    bDefaultLatheCharacterMode = sal_False;
    bDefaultLatheCloseFront = sal_True;
    bDefaultLatheCloseBack = sal_True;

    // Extrude object
    bDefaultExtrudeSmoothed = sal_True;
    bDefaultExtrudeSmoothFrontBack = sal_False;
    bDefaultExtrudeCharacterMode = sal_False;
    bDefaultExtrudeCloseFront = sal_True;
    bDefaultExtrudeCloseBack = sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
