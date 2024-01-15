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

// Class to manage the 3D default attributes

E3dDefaultAttributes::E3dDefaultAttributes() { Reset(); }

void E3dDefaultAttributes::Reset()
{
    // Cube object
    m_aDefaultCubePos = basegfx::B3DPoint(-500.0, -500.0, -500.0);
    m_aDefaultCubeSize = basegfx::B3DVector(1000.0, 1000.0, 1000.0);
    m_bDefaultCubePosIsCenter = false;

    // Sphere object
    m_aDefaultSphereCenter = basegfx::B3DPoint(0.0, 0.0, 0.0);
    m_aDefaultSphereSize = basegfx::B3DPoint(1000.0, 1000.0, 1000.0);

    // Lathe object
    m_bDefaultLatheSmoothed = true;
    m_bDefaultLatheSmoothFrontBack = false;
    m_bDefaultLatheCharacterMode = false;
    m_bDefaultLatheCloseFront = true;
    m_bDefaultLatheCloseBack = true;

    // Extrude object
    m_bDefaultExtrudeSmoothed = true;
    m_bDefaultExtrudeSmoothFrontBack = false;
    m_bDefaultExtrudeCharacterMode = false;
    m_bDefaultExtrudeCloseFront = true;
    m_bDefaultExtrudeCloseBack = true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
