/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <vcl/dllapi.h>

enum class GestureEventRotateType
{
    Begin,
    Update,
    End
};

class VCL_DLLPUBLIC GestureEventRotate
{
public:
    sal_Int32 mnX = 0;
    sal_Int32 mnY = 0;

    GestureEventRotateType meEventType = GestureEventRotateType::Begin;

    // The difference of between the current gesture scale and the scale at the beginning of the
    // gesture.
    double mfAngleDelta = 0;

    GestureEventRotate() = default;

    GestureEventRotate(sal_Int32 nInitialX, sal_Int32 nInitialY, GestureEventRotateType eEventType,
                       double fAngleDelta)
        : mnX(nInitialX)
        , mnY(nInitialY)
        , meEventType(eEventType)
        , mfAngleDelta(fAngleDelta)
    {
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
