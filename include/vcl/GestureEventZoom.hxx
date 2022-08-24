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

enum class GestureEventZoomType
{
    Begin,
    Update,
    End
};

class VCL_DLLPUBLIC GestureEventZoom
{
public:
    sal_Int32 mnX = 0;
    sal_Int32 mnY = 0;

    GestureEventZoomType meEventType = GestureEventZoomType::Begin;

    // The difference of between the current gesture scale and the scale at the beginning of the
    // gesture.
    double mfScaleDelta = 0;

    GestureEventZoom() = default;

    GestureEventZoom(sal_Int32 nX, sal_Int32 nY, GestureEventZoomType eEventType,
                     double fScaleDelta)
        : mnX(nX)
        , mnY(nY)
        , meEventType(eEventType)
        , mfScaleDelta(fScaleDelta)
    {
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
