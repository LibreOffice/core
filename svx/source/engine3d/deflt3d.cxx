/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


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
    bDefaultCreateNormals = true;
    bDefaultCreateTexture = true;

    // Cube object
    aDefaultCubePos = basegfx::B3DPoint(-500.0, -500.0, -500.0);
    aDefaultCubeSize = basegfx::B3DVector(1000.0, 1000.0, 1000.0);
    nDefaultCubeSideFlags = CUBE_FULL;
    bDefaultCubePosIsCenter = false;

    // Sphere object
    aDefaultSphereCenter = basegfx::B3DPoint(0.0, 0.0, 0.0);
    aDefaultSphereSize = basegfx::B3DPoint(1000.0, 1000.0, 1000.0);

    // Lathe object
    nDefaultLatheEndAngle = 3600;
    bDefaultLatheSmoothed = true;
    bDefaultLatheSmoothFrontBack = false;
    bDefaultLatheCharacterMode = false;
    bDefaultLatheCloseFront = true;
    bDefaultLatheCloseBack = true;

    // Extrude object
    bDefaultExtrudeSmoothed = true;
    bDefaultExtrudeSmoothFrontBack = false;
    bDefaultExtrudeCharacterMode = false;
    bDefaultExtrudeCloseFront = true;
    bDefaultExtrudeCloseBack = true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
