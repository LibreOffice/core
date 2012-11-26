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



#ifndef _E3D_DEFLT3D_HXX
#define _E3D_DEFLT3D_HXX

#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include "svx/svxdllapi.h"
#include <tools/color.hxx>

/*************************************************************************
|*
|* Klasse zum verwalten der 3D-Default Attribute
|*
\************************************************************************/

class SVX_DLLPUBLIC E3dDefaultAttributes
{
private:
    // Compound-Objekt
    Color               aDefaultAmbientColor;
    sal_Bool                bDefaultCreateNormals;
    sal_Bool                bDefaultCreateTexture;

    // Cube-Objekt
    basegfx::B3DPoint   aDefaultCubePos;
    basegfx::B3DVector  aDefaultCubeSize;
    sal_uInt16              nDefaultCubeSideFlags;
    sal_Bool                bDefaultCubePosIsCenter;

    // Sphere-Objekt
    basegfx::B3DPoint   aDefaultSphereCenter;
    basegfx::B3DVector  aDefaultSphereSize;

    // Lathe-Objekt
    long                nDefaultLatheEndAngle;
    sal_Bool                bDefaultLatheSmoothed;
    sal_Bool                bDefaultLatheSmoothFrontBack;
    sal_Bool                bDefaultLatheCharacterMode;
    sal_Bool                bDefaultLatheCloseFront;
    sal_Bool                bDefaultLatheCloseBack;

    // Extrude-Objekt
    sal_Bool                bDefaultExtrudeSmoothed;
    sal_Bool                bDefaultExtrudeSmoothFrontBack;
    sal_Bool                bDefaultExtrudeCharacterMode;
    sal_Bool                bDefaultExtrudeCloseFront;
    sal_Bool                bDefaultExtrudeCloseBack;

public:
    // Konstruktor
    E3dDefaultAttributes();

    // Defaults zuruecksetzen
    void Reset();

    // Getter/Setter fuer Default-Werte aller 3D-Objekte
    // Compound-Objekt
    const Color& GetDefaultAmbientColor() const { return aDefaultAmbientColor; }
    void SetDefaultAmbientColor(const Color& rNew) { aDefaultAmbientColor = rNew; }

    sal_Bool GetDefaultCreateNormals() const { return bDefaultCreateNormals; }
    void SetDefaultCreateNormals(const sal_Bool bNew) { bDefaultCreateNormals = bNew; }
    sal_Bool GetDefaultCreateTexture() const { return bDefaultCreateTexture; }
    void SetDefaultCreateTexture(const sal_Bool bNew) { bDefaultCreateTexture = bNew; }

    // Cube-Objekt
    const basegfx::B3DPoint& GetDefaultCubePos() const { return aDefaultCubePos; }
    void SetDefaultCubePos(const basegfx::B3DPoint& rNew) { aDefaultCubePos = rNew; }
    const basegfx::B3DVector& GetDefaultCubeSize() const { return aDefaultCubeSize; }
    void SetDefaultCubeSize(const basegfx::B3DVector& rNew) { aDefaultCubeSize = rNew; }
    sal_uInt16 GetDefaultCubeSideFlags() const { return nDefaultCubeSideFlags; }
    void SetDefaultCubeSideFlags(const sal_uInt16 nNew) { nDefaultCubeSideFlags = nNew; }
    sal_Bool GetDefaultCubePosIsCenter() const { return bDefaultCubePosIsCenter; }
    void SetDefaultCubePosIsCenter(const sal_Bool bNew) { bDefaultCubePosIsCenter = bNew; }

    // Sphere-Objekt
    const basegfx::B3DPoint& GetDefaultSphereCenter() const { return aDefaultSphereCenter; }
    void SetDefaultSphereCenter(const basegfx::B3DPoint& rNew) { aDefaultSphereCenter = rNew; }
    const basegfx::B3DVector& GetDefaultSphereSize() const { return aDefaultSphereSize; }
    void SetDefaultSphereSize(const basegfx::B3DPoint& rNew) { aDefaultSphereSize = rNew; }

    // Lathe-Objekt
    long GetDefaultLatheEndAngle() const { return nDefaultLatheEndAngle; }
    void SetDefaultLatheEndAngle(const long nNew) { nDefaultLatheEndAngle = nNew; }
    sal_Bool GetDefaultLatheSmoothed() const { return bDefaultLatheSmoothed; }
    void SetDefaultLatheSmoothed(const sal_Bool bNew) { bDefaultLatheSmoothed = bNew; }
    sal_Bool GetDefaultLatheSmoothFrontBack() const { return bDefaultLatheSmoothFrontBack; }
    void SetDefaultLatheSmoothFrontBack(const sal_Bool bNew) { bDefaultLatheSmoothFrontBack = bNew; }
    sal_Bool GetDefaultLatheCharacterMode() const { return bDefaultLatheCharacterMode; }
    void SetDefaultLatheCharacterMode(const sal_Bool bNew) { bDefaultLatheCharacterMode = bNew; }
    sal_Bool GetDefaultLatheCloseFront() const { return bDefaultLatheCloseFront; }
    void SetDefaultLatheCloseFront(const sal_Bool bNew) { bDefaultLatheCloseFront = bNew; }
    sal_Bool GetDefaultLatheCloseBack() const { return bDefaultLatheCloseBack; }
    void SetDefaultLatheCloseBack(const sal_Bool bNew) { bDefaultLatheCloseBack = bNew; }

    // Extrude-Objekt
    sal_Bool GetDefaultExtrudeSmoothed() const { return bDefaultExtrudeSmoothed; }
    void SetDefaultExtrudeSmoothed(const sal_Bool bNew) { bDefaultExtrudeSmoothed = bNew; }
    sal_Bool GetDefaultExtrudeSmoothFrontBack() const { return bDefaultExtrudeSmoothFrontBack; }
    void SetDefaultExtrudeSmoothFrontBack(const sal_Bool bNew) { bDefaultExtrudeSmoothFrontBack = bNew; }
    sal_Bool GetDefaultExtrudeCharacterMode() const { return bDefaultExtrudeCharacterMode; }
    void SetDefaultExtrudeCharacterMode(const sal_Bool bNew) { bDefaultExtrudeCharacterMode = bNew; }
    sal_Bool GetDefaultExtrudeCloseFront() const { return bDefaultExtrudeCloseFront; }
    void SetDefaultExtrudeCloseFront(const sal_Bool bNew) { bDefaultExtrudeCloseFront = bNew; }
    sal_Bool GetDefaultExtrudeCloseBack() const { return bDefaultExtrudeCloseBack; }
    void SetDefaultExtrudeCloseBack(const sal_Bool bNew) { bDefaultExtrudeCloseBack = bNew; }
};

#endif          // _E3D_DEFLT3D_HXX
