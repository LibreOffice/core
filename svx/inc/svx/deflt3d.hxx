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
    sal_Bool                bDefaultCreateNormals;
    sal_Bool                bDefaultCreateTexture;

    // Cube object
    basegfx::B3DPoint   aDefaultCubePos;
    basegfx::B3DVector  aDefaultCubeSize;
    sal_uInt16              nDefaultCubeSideFlags;
    sal_Bool                bDefaultCubePosIsCenter;

    // Sphere object
    basegfx::B3DPoint   aDefaultSphereCenter;
    basegfx::B3DVector  aDefaultSphereSize;

    // Lathe object
    long                nDefaultLatheEndAngle;
    sal_Bool                bDefaultLatheSmoothed;
    sal_Bool                bDefaultLatheSmoothFrontBack;
    sal_Bool                bDefaultLatheCharacterMode;
    sal_Bool                bDefaultLatheCloseFront;
    sal_Bool                bDefaultLatheCloseBack;

    // Extrude object
    sal_Bool                bDefaultExtrudeSmoothed;
    sal_Bool                bDefaultExtrudeSmoothFrontBack;
    sal_Bool                bDefaultExtrudeCharacterMode;
    sal_Bool                bDefaultExtrudeCloseFront;
    sal_Bool                bDefaultExtrudeCloseBack;

public:
    // Construktor
    E3dDefaultAttributes();

    // Reset to defaults
    void Reset();

    // Getter/Setter for default values of all 3D objects
    // Compound object
    const Color& GetDefaultAmbientColor() { return aDefaultAmbientColor; }
    void SetDefaultAmbientColor(const Color& rNew) { aDefaultAmbientColor = rNew; }

    sal_Bool GetDefaultCreateNormals() const { return bDefaultCreateNormals; }
    void SetDefaultCreateNormals(const sal_Bool bNew) { bDefaultCreateNormals = bNew; }
    sal_Bool GetDefaultCreateTexture() const { return bDefaultCreateTexture; }
    void SetDefaultCreateTexture(const sal_Bool bNew) { bDefaultCreateTexture = bNew; }

    // Cube object
    const basegfx::B3DPoint& GetDefaultCubePos() { return aDefaultCubePos; }
    void SetDefaultCubePos(const basegfx::B3DPoint& rNew) { aDefaultCubePos = rNew; }
    const basegfx::B3DVector& GetDefaultCubeSize() { return aDefaultCubeSize; }
    void SetDefaultCubeSize(const basegfx::B3DVector& rNew) { aDefaultCubeSize = rNew; }
    sal_uInt16 GetDefaultCubeSideFlags() const { return nDefaultCubeSideFlags; }
    void SetDefaultCubeSideFlags(const sal_uInt16 nNew) { nDefaultCubeSideFlags = nNew; }
    sal_Bool GetDefaultCubePosIsCenter() const { return bDefaultCubePosIsCenter; }
    void SetDefaultCubePosIsCenter(const sal_Bool bNew) { bDefaultCubePosIsCenter = bNew; }

    // Sphere object
    const basegfx::B3DPoint& GetDefaultSphereCenter() { return aDefaultSphereCenter; }
    void SetDefaultSphereCenter(const basegfx::B3DPoint& rNew) { aDefaultSphereCenter = rNew; }
    const basegfx::B3DVector& GetDefaultSphereSize() { return aDefaultSphereSize; }
    void SetDefaultSphereSize(const basegfx::B3DPoint& rNew) { aDefaultSphereSize = rNew; }

    // Lathe object
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

    // Extrude object
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
