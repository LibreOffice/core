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

#ifndef _E3D_DEFLT3D_HXX
#define _E3D_DEFLT3D_HXX

#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include "svx/svxdllapi.h"
#include <tools/color.hxx>

/*************************************************************************
|*
|* Class for managing the 3D default attributes
|*
\************************************************************************/

class SVX_DLLPUBLIC E3dDefaultAttributes
{
private:
    // Compound object
    Color               aDefaultAmbientColor;
    BOOL                bDefaultCreateNormals;
    BOOL                bDefaultCreateTexture;

    // Cube object
    basegfx::B3DPoint   aDefaultCubePos;
    basegfx::B3DVector  aDefaultCubeSize;
    UINT16              nDefaultCubeSideFlags;
    BOOL                bDefaultCubePosIsCenter;

    // Sphere object
    basegfx::B3DPoint   aDefaultSphereCenter;
    basegfx::B3DVector  aDefaultSphereSize;

    // Lathe object
    long                nDefaultLatheEndAngle;
    BOOL                bDefaultLatheSmoothed;
    BOOL                bDefaultLatheSmoothFrontBack;
    BOOL                bDefaultLatheCharacterMode;
    BOOL                bDefaultLatheCloseFront;
    BOOL                bDefaultLatheCloseBack;

    // Extrude object
    BOOL                bDefaultExtrudeSmoothed;
    BOOL                bDefaultExtrudeSmoothFrontBack;
    BOOL                bDefaultExtrudeCharacterMode;
    BOOL                bDefaultExtrudeCloseFront;
    BOOL                bDefaultExtrudeCloseBack;

public:
    // Construktor
    E3dDefaultAttributes();

    // Reset to defaults
    void Reset();

    // Getter/Setter for default values of all 3D objects
    // Compound object
    const Color& GetDefaultAmbientColor() { return aDefaultAmbientColor; }
    void SetDefaultAmbientColor(const Color& rNew) { aDefaultAmbientColor = rNew; }

    BOOL GetDefaultCreateNormals() const { return bDefaultCreateNormals; }
    void SetDefaultCreateNormals(const BOOL bNew) { bDefaultCreateNormals = bNew; }
    BOOL GetDefaultCreateTexture() const { return bDefaultCreateTexture; }
    void SetDefaultCreateTexture(const BOOL bNew) { bDefaultCreateTexture = bNew; }

    // Cube object
    const basegfx::B3DPoint& GetDefaultCubePos() { return aDefaultCubePos; }
    void SetDefaultCubePos(const basegfx::B3DPoint& rNew) { aDefaultCubePos = rNew; }
    const basegfx::B3DVector& GetDefaultCubeSize() { return aDefaultCubeSize; }
    void SetDefaultCubeSize(const basegfx::B3DVector& rNew) { aDefaultCubeSize = rNew; }
    UINT16 GetDefaultCubeSideFlags() const { return nDefaultCubeSideFlags; }
    void SetDefaultCubeSideFlags(const UINT16 nNew) { nDefaultCubeSideFlags = nNew; }
    BOOL GetDefaultCubePosIsCenter() const { return bDefaultCubePosIsCenter; }
    void SetDefaultCubePosIsCenter(const BOOL bNew) { bDefaultCubePosIsCenter = bNew; }

    // Sphere object
    const basegfx::B3DPoint& GetDefaultSphereCenter() { return aDefaultSphereCenter; }
    void SetDefaultSphereCenter(const basegfx::B3DPoint& rNew) { aDefaultSphereCenter = rNew; }
    const basegfx::B3DVector& GetDefaultSphereSize() { return aDefaultSphereSize; }
    void SetDefaultSphereSize(const basegfx::B3DPoint& rNew) { aDefaultSphereSize = rNew; }

    // Lathe object
    long GetDefaultLatheEndAngle() const { return nDefaultLatheEndAngle; }
    void SetDefaultLatheEndAngle(const long nNew) { nDefaultLatheEndAngle = nNew; }
    BOOL GetDefaultLatheSmoothed() const { return bDefaultLatheSmoothed; }
    void SetDefaultLatheSmoothed(const BOOL bNew) { bDefaultLatheSmoothed = bNew; }
    BOOL GetDefaultLatheSmoothFrontBack() const { return bDefaultLatheSmoothFrontBack; }
    void SetDefaultLatheSmoothFrontBack(const BOOL bNew) { bDefaultLatheSmoothFrontBack = bNew; }
    BOOL GetDefaultLatheCharacterMode() const { return bDefaultLatheCharacterMode; }
    void SetDefaultLatheCharacterMode(const BOOL bNew) { bDefaultLatheCharacterMode = bNew; }
    BOOL GetDefaultLatheCloseFront() const { return bDefaultLatheCloseFront; }
    void SetDefaultLatheCloseFront(const BOOL bNew) { bDefaultLatheCloseFront = bNew; }
    BOOL GetDefaultLatheCloseBack() const { return bDefaultLatheCloseBack; }
    void SetDefaultLatheCloseBack(const BOOL bNew) { bDefaultLatheCloseBack = bNew; }

    // Extrude object
    BOOL GetDefaultExtrudeSmoothed() const { return bDefaultExtrudeSmoothed; }
    void SetDefaultExtrudeSmoothed(const BOOL bNew) { bDefaultExtrudeSmoothed = bNew; }
    BOOL GetDefaultExtrudeSmoothFrontBack() const { return bDefaultExtrudeSmoothFrontBack; }
    void SetDefaultExtrudeSmoothFrontBack(const BOOL bNew) { bDefaultExtrudeSmoothFrontBack = bNew; }
    BOOL GetDefaultExtrudeCharacterMode() const { return bDefaultExtrudeCharacterMode; }
    void SetDefaultExtrudeCharacterMode(const BOOL bNew) { bDefaultExtrudeCharacterMode = bNew; }
    BOOL GetDefaultExtrudeCloseFront() const { return bDefaultExtrudeCloseFront; }
    void SetDefaultExtrudeCloseFront(const BOOL bNew) { bDefaultExtrudeCloseFront = bNew; }
    BOOL GetDefaultExtrudeCloseBack() const { return bDefaultExtrudeCloseBack; }
    void SetDefaultExtrudeCloseBack(const BOOL bNew) { bDefaultExtrudeCloseBack = bNew; }
};

#endif          // _E3D_DEFLT3D_HXX
