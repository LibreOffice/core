/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_GESTUREEVENT_HXX
#define INCLUDED_VCL_GESTUREEVENT_HXX

#include <vcl/dllapi.h>

enum class GestureEventPanType
{
    Begin,
    Update,
    End
};

enum class PanningOrientation
{
    Horizontal,
    Vertical
};

class VCL_DLLPUBLIC GestureEventPan
{
public:
    sal_Int32 mnX;
    sal_Int32 mnY;
    GestureEventPanType meEventType;

    sal_Int32 mnOffset;
    PanningOrientation meOrientation;

    GestureEventPan()
        : mnX(0)
        , mnY(0)
        , meEventType(GestureEventPanType::Begin)
        , mnOffset(0)
        , meOrientation(PanningOrientation::Horizontal)
    {
    }

    GestureEventPan(sal_Int32 nX, sal_Int32 nY, GestureEventPanType eEventType, sal_Int32 nOffset,
                    PanningOrientation eOrientation)
        : mnX(nX)
        , mnY(nY)
        , meEventType(eEventType)
        , mnOffset(nOffset)
        , meOrientation(eOrientation)
    {
    }
};

#endif // INCLUDED_VCL_GESTUREEVENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
