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

#ifndef INCLUDED_SVX_GLOBL3D_HXX
#define INCLUDED_SVX_GLOBL3D_HXX

#include <sal/types.h>

const sal_uInt32 E3dInventor = sal_uInt32('E')*0x00000001+
                           sal_uInt32('3')*0x00000100+
                           sal_uInt32('D')*0x00010000+
                           sal_uInt32('1')*0x01000000;

const sal_uInt16 E3D_SCENE_ID       = 1;
const sal_uInt16 E3D_POLYSCENE_ID   = 2;
const sal_uInt16 E3D_OBJECT_ID      = 3; // should not be used, it's only a helper class for E3DScene and E3DCompoundObject
const sal_uInt16 E3D_CUBEOBJ_ID     = 4;
const sal_uInt16 E3D_SPHEREOBJ_ID   = 5;
const sal_uInt16 E3D_EXTRUDEOBJ_ID  = 6;
const sal_uInt16 E3D_LATHEOBJ_ID    = 7;
const sal_uInt16 E3D_COMPOUNDOBJ_ID = 8;
const sal_uInt16 E3D_POLYGONOBJ_ID  = 9;

#endif // INCLUDED_SVX_GLOBL3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
