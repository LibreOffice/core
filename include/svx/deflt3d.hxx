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

#ifndef INCLUDED_SVX_DEFLT3D_HXX
#define INCLUDED_SVX_DEFLT3D_HXX

#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include <svx/svxdllapi.h>
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
    bool                bDefaultCreateNormals;
    bool                bDefaultCreateTexture;

    // Cube object
    basegfx::B3DPoint   aDefaultCubePos;
    basegfx::B3DVector  aDefaultCubeSize;
    sal_uInt16          nDefaultCubeSideFlags;
    bool                bDefaultCubePosIsCenter;

    // Sphere object
    basegfx::B3DPoint   aDefaultSphereCenter;
    basegfx::B3DVector  aDefaultSphereSize;

    // Lathe object
    long                nDefaultLatheEndAngle;
    bool                bDefaultLatheSmoothed;
    bool                bDefaultLatheSmoothFrontBack;
    bool                bDefaultLatheCharacterMode;
    bool                bDefaultLatheCloseFront;
    bool                bDefaultLatheCloseBack;

    // Extrude object
    bool                bDefaultExtrudeSmoothed;
    bool                bDefaultExtrudeSmoothFrontBack;
    bool                bDefaultExtrudeCharacterMode;
    bool                bDefaultExtrudeCloseFront;
    bool                bDefaultExtrudeCloseBack;

public:
    // Construktor
    E3dDefaultAttributes();

    // Reset to defaults
    void Reset();

    // Getter/Setter for default values of all 3D objects
    // Compound object
    const Color& GetDefaultAmbientColor() { return aDefaultAmbientColor; }
    void SetDefaultAmbientColor(const Color& rNew) { aDefaultAmbientColor = rNew; }

    bool GetDefaultCreateNormals() const { return bDefaultCreateNormals; }
    void SetDefaultCreateNormals(const bool bNew) { bDefaultCreateNormals = bNew; }
    bool GetDefaultCreateTexture() const { return bDefaultCreateTexture; }
    void SetDefaultCreateTexture(const bool bNew) { bDefaultCreateTexture = bNew; }

    // Cube object
    const basegfx::B3DPoint& GetDefaultCubePos() { return aDefaultCubePos; }
    void SetDefaultCubePos(const basegfx::B3DPoint& rNew) { aDefaultCubePos = rNew; }
    const basegfx::B3DVector& GetDefaultCubeSize() { return aDefaultCubeSize; }
    void SetDefaultCubeSize(const basegfx::B3DVector& rNew) { aDefaultCubeSize = rNew; }
    sal_uInt16 GetDefaultCubeSideFlags() const { return nDefaultCubeSideFlags; }
    void SetDefaultCubeSideFlags(const sal_uInt16 nNew) { nDefaultCubeSideFlags = nNew; }
    bool GetDefaultCubePosIsCenter() const { return bDefaultCubePosIsCenter; }
    void SetDefaultCubePosIsCenter(const bool bNew) { bDefaultCubePosIsCenter = bNew; }

    // Sphere object
    const basegfx::B3DPoint& GetDefaultSphereCenter() { return aDefaultSphereCenter; }
    void SetDefaultSphereCenter(const basegfx::B3DPoint& rNew) { aDefaultSphereCenter = rNew; }
    const basegfx::B3DVector& GetDefaultSphereSize() { return aDefaultSphereSize; }
    void SetDefaultSphereSize(const basegfx::B3DPoint& rNew) { aDefaultSphereSize = rNew; }

    // Lathe object
    long GetDefaultLatheEndAngle() const { return nDefaultLatheEndAngle; }
    void SetDefaultLatheEndAngle(const long nNew) { nDefaultLatheEndAngle = nNew; }
    bool GetDefaultLatheSmoothed() const { return bDefaultLatheSmoothed; }
    void SetDefaultLatheSmoothed(const bool bNew) { bDefaultLatheSmoothed = bNew; }
    bool GetDefaultLatheSmoothFrontBack() const { return bDefaultLatheSmoothFrontBack; }
    void SetDefaultLatheSmoothFrontBack(const bool bNew) { bDefaultLatheSmoothFrontBack = bNew; }
    bool GetDefaultLatheCharacterMode() const { return bDefaultLatheCharacterMode; }
    void SetDefaultLatheCharacterMode(const bool bNew) { bDefaultLatheCharacterMode = bNew; }
    bool GetDefaultLatheCloseFront() const { return bDefaultLatheCloseFront; }
    void SetDefaultLatheCloseFront(const bool bNew) { bDefaultLatheCloseFront = bNew; }
    bool GetDefaultLatheCloseBack() const { return bDefaultLatheCloseBack; }
    void SetDefaultLatheCloseBack(const bool bNew) { bDefaultLatheCloseBack = bNew; }

    // Extrude object
    bool GetDefaultExtrudeSmoothed() const { return bDefaultExtrudeSmoothed; }
    void SetDefaultExtrudeSmoothed(const bool bNew) { bDefaultExtrudeSmoothed = bNew; }
    bool GetDefaultExtrudeSmoothFrontBack() const { return bDefaultExtrudeSmoothFrontBack; }
    void SetDefaultExtrudeSmoothFrontBack(const bool bNew) { bDefaultExtrudeSmoothFrontBack = bNew; }
    bool GetDefaultExtrudeCharacterMode() const { return bDefaultExtrudeCharacterMode; }
    void SetDefaultExtrudeCharacterMode(const bool bNew) { bDefaultExtrudeCharacterMode = bNew; }
    bool GetDefaultExtrudeCloseFront() const { return bDefaultExtrudeCloseFront; }
    void SetDefaultExtrudeCloseFront(const bool bNew) { bDefaultExtrudeCloseFront = bNew; }
    bool GetDefaultExtrudeCloseBack() const { return bDefaultExtrudeCloseBack; }
    void SetDefaultExtrudeCloseBack(const bool bNew) { bDefaultExtrudeCloseBack = bNew; }
};

#endif // INCLUDED_SVX_DEFLT3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
