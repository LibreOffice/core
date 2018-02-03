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
#include <vcl/color.hxx>

/*************************************************************************
|*
|* Class for managing the 3D default attributes
|*
\************************************************************************/

class SAL_WARN_UNUSED SVX_DLLPUBLIC E3dDefaultAttributes
{
private:
    // Cube object
    basegfx::B3DPoint   aDefaultCubePos;
    basegfx::B3DVector  aDefaultCubeSize;
    bool                bDefaultCubePosIsCenter;

    // Sphere object
    basegfx::B3DPoint   aDefaultSphereCenter;
    basegfx::B3DVector  aDefaultSphereSize;

    // Lathe object
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
    // Constructor
    E3dDefaultAttributes();

    // Reset to defaults
    void Reset();

    // Cube object
    const basegfx::B3DPoint& GetDefaultCubePos() { return aDefaultCubePos; }
    const basegfx::B3DVector& GetDefaultCubeSize() { return aDefaultCubeSize; }
    bool GetDefaultCubePosIsCenter() const { return bDefaultCubePosIsCenter; }

    // Sphere object
    const basegfx::B3DPoint& GetDefaultSphereCenter() { return aDefaultSphereCenter; }
    const basegfx::B3DVector& GetDefaultSphereSize() { return aDefaultSphereSize; }

    // Lathe object
    bool GetDefaultLatheSmoothed() const { return bDefaultLatheSmoothed; }
    bool GetDefaultLatheSmoothFrontBack() const { return bDefaultLatheSmoothFrontBack; }
    bool GetDefaultLatheCharacterMode() const { return bDefaultLatheCharacterMode; }
    void SetDefaultLatheCharacterMode(const bool bNew) { bDefaultLatheCharacterMode = bNew; }
    bool GetDefaultLatheCloseFront() const { return bDefaultLatheCloseFront; }
    bool GetDefaultLatheCloseBack() const { return bDefaultLatheCloseBack; }

    // Extrude object
    bool GetDefaultExtrudeSmoothed() const { return bDefaultExtrudeSmoothed; }
    bool GetDefaultExtrudeSmoothFrontBack() const { return bDefaultExtrudeSmoothFrontBack; }
    bool GetDefaultExtrudeCharacterMode() const { return bDefaultExtrudeCharacterMode; }
    void SetDefaultExtrudeCharacterMode(const bool bNew) { bDefaultExtrudeCharacterMode = bNew; }
    bool GetDefaultExtrudeCloseFront() const { return bDefaultExtrudeCloseFront; }
    void SetDefaultExtrudeCloseFront(const bool bNew) { bDefaultExtrudeCloseFront = bNew; }
    bool GetDefaultExtrudeCloseBack() const { return bDefaultExtrudeCloseBack; }
    void SetDefaultExtrudeCloseBack(const bool bNew) { bDefaultExtrudeCloseBack = bNew; }
};

#endif // INCLUDED_SVX_DEFLT3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
