/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/deflt3d.hxx>
#include <svx/cube3d.hxx>
#include <svx/svxids.hrc>
#include <editeng/colritem.hxx>
#include <svx/e3ditem.hxx>

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
    bDefaultCreateNormals = sal_True;
    bDefaultCreateTexture = sal_True;

    // Cube-Objekt
    aDefaultCubePos = basegfx::B3DPoint(-500.0, -500.0, -500.0);
    aDefaultCubeSize = basegfx::B3DVector(1000.0, 1000.0, 1000.0);
    nDefaultCubeSideFlags = CUBE_FULL;
    bDefaultCubePosIsCenter = sal_False;

    // Sphere-Objekt
    aDefaultSphereCenter = basegfx::B3DPoint(0.0, 0.0, 0.0);
    aDefaultSphereSize = basegfx::B3DPoint(1000.0, 1000.0, 1000.0);

    // Lathe-Objekt
    nDefaultLatheEndAngle = 3600;
    bDefaultLatheSmoothed = sal_True;
    bDefaultLatheSmoothFrontBack = sal_False;
    bDefaultLatheCharacterMode = sal_False;
    bDefaultLatheCloseFront = sal_True;
    bDefaultLatheCloseBack = sal_True;

    // Extrude-Objekt
    bDefaultExtrudeSmoothed = sal_True;
    bDefaultExtrudeSmoothFrontBack = sal_False;
    bDefaultExtrudeCharacterMode = sal_False;
    bDefaultExtrudeCloseFront = sal_True;
    bDefaultExtrudeCloseBack = sal_True;
}

// eof
