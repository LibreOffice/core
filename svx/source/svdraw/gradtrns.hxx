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

#ifndef _GRADTRANS_HXX
#define _GRADTRANS_HXX

#include <svx/xgrad.hxx>
#include <basegfx/point/b2dpoint.hxx>

class SdrObject;

class GradTransVector
{
public:
    basegfx::B2DPoint           maPositionA;
    basegfx::B2DPoint           maPositionB;
    Color                       aCol1;
    Color                       aCol2;
};

class GradTransGradient
{
public:
    XGradient                   aGradient;
};

class GradTransformer
{
public:
    GradTransformer() {}

    void GradToVec(GradTransGradient& rG, GradTransVector& rV,
        const SdrObject* pObj);
    void VecToGrad(GradTransVector& rV, GradTransGradient& rG,
        GradTransGradient& rGOld, const SdrObject* pObj, bool bMoveSingle, bool bMoveFirst);
};

#endif // _GRADTRANS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
