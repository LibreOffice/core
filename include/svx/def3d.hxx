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

#ifndef _SVX_DEF3D_HXX
#define _SVX_DEF3D_HXX

#include <math.h>
#include <tools/solar.h>

const double fPiDiv180 = 0.01745329251994;
const double EPSILON = 1e-06;

#define DEG2RAD(fAngle) (fPiDiv180 * (fAngle))

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++ 3D helper functions ++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

enum E3dDragConstraint { E3DDRAG_CONSTR_X   = 0x0001,
                         E3DDRAG_CONSTR_Y   = 0x0002,
                         E3DDRAG_CONSTR_Z   = 0x0004,
                         E3DDRAG_CONSTR_XY  = 0x0003,
                         E3DDRAG_CONSTR_XZ  = 0x0005,
                         E3DDRAG_CONSTR_YZ  = 0x0006,
                         E3DDRAG_CONSTR_XYZ = 0x0007
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
