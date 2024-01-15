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

#include <config_options.h>
#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include <svx/svxdllapi.h>

/*************************************************************************
|*
|* Class for managing the 3D default attributes
|*
\************************************************************************/

class SAL_WARN_UNUSED UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) E3dDefaultAttributes
{
private:
    // Cube object
    basegfx::B3DPoint   m_aDefaultCubePos;
    basegfx::B3DVector  m_aDefaultCubeSize;
    bool                m_bDefaultCubePosIsCenter;

    // Sphere object
    basegfx::B3DPoint   m_aDefaultSphereCenter;
    basegfx::B3DVector  m_aDefaultSphereSize;

    // Lathe object
    bool                m_bDefaultLatheSmoothed;
    bool                m_bDefaultLatheSmoothFrontBack;
    bool                m_bDefaultLatheCharacterMode;
    bool                m_bDefaultLatheCloseFront;
    bool                m_bDefaultLatheCloseBack;

    // Extrude object
    bool                m_bDefaultExtrudeSmoothed;
    bool                m_bDefaultExtrudeSmoothFrontBack;
    bool                m_bDefaultExtrudeCharacterMode;
    bool                m_bDefaultExtrudeCloseFront;
    bool                m_bDefaultExtrudeCloseBack;

public:
    // Constructor
    E3dDefaultAttributes();

    // Reset to defaults
    void Reset();

    // Cube object
    const basegfx::B3DPoint& GetDefaultCubePos() const { return m_aDefaultCubePos; }
    const basegfx::B3DVector& GetDefaultCubeSize() const { return m_aDefaultCubeSize; }
    bool GetDefaultCubePosIsCenter() const { return m_bDefaultCubePosIsCenter; }

    // Sphere object
    const basegfx::B3DPoint& GetDefaultSphereCenter() const { return m_aDefaultSphereCenter; }
    const basegfx::B3DVector& GetDefaultSphereSize() const { return m_aDefaultSphereSize; }

    // Lathe object
    bool GetDefaultLatheSmoothed() const { return m_bDefaultLatheSmoothed; }
    bool GetDefaultLatheSmoothFrontBack() const { return m_bDefaultLatheSmoothFrontBack; }
    bool GetDefaultLatheCharacterMode() const { return m_bDefaultLatheCharacterMode; }
    void SetDefaultLatheCharacterMode(const bool bNew) { m_bDefaultLatheCharacterMode = bNew; }
    bool GetDefaultLatheCloseFront() const { return m_bDefaultLatheCloseFront; }
    bool GetDefaultLatheCloseBack() const { return m_bDefaultLatheCloseBack; }

    // Extrude object
    bool GetDefaultExtrudeSmoothed() const { return m_bDefaultExtrudeSmoothed; }
    bool GetDefaultExtrudeSmoothFrontBack() const { return m_bDefaultExtrudeSmoothFrontBack; }
    bool GetDefaultExtrudeCharacterMode() const { return m_bDefaultExtrudeCharacterMode; }
    void SetDefaultExtrudeCharacterMode(const bool bNew) { m_bDefaultExtrudeCharacterMode = bNew; }
    bool GetDefaultExtrudeCloseFront() const { return m_bDefaultExtrudeCloseFront; }
    void SetDefaultExtrudeCloseFront(const bool bNew) { m_bDefaultExtrudeCloseFront = bNew; }
    bool GetDefaultExtrudeCloseBack() const { return m_bDefaultExtrudeCloseBack; }
    void SetDefaultExtrudeCloseBack(const bool bNew) { m_bDefaultExtrudeCloseBack = bNew; }
};

#endif // INCLUDED_SVX_DEFLT3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
