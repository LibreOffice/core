/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <drawinglayer/primitive3d/Tools.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>

using namespace css;

namespace drawinglayer::primitive3d
{
OUString idToString(sal_uInt32 nId)
{
    switch (nId)
    {
        case PRIMITIVE3D_ID_GROUPPRIMITIVE3D:
            return "GROUPPRIMITIVE3D";
        case PRIMITIVE3D_ID_HATCHTEXTUREPRIMITIVE3D:
            return "HATCHTEXTUREPRIMITIVE3D";
        case PRIMITIVE3D_ID_MODIFIEDCOLORPRIMITIVE3D:
            return "MODIFIEDCOLORPRIMITIVE3D";
        case PRIMITIVE3D_ID_POLYGONHAIRLINEPRIMITIVE3D:
            return "POLYGONHAIRLINEPRIMITIVE3D";
        case PRIMITIVE3D_ID_POLYGONSTROKEPRIMITIVE3D:
            return "POLYGONSTROKEPRIMITIVE3D";
        case PRIMITIVE3D_ID_POLYGONTUBEPRIMITIVE3D:
            return "POLYGONTUBEPRIMITIVE3D";
        case PRIMITIVE3D_ID_POLYPOLYGONMATERIALPRIMITIVE3D:
            return "POLYPOLYGONMATERIALPRIMITIVE3D";
        case PRIMITIVE3D_ID_SDRCUBEPRIMITIVE3D:
            return "SDRCUBEPRIMITIVE3D";
        case PRIMITIVE3D_ID_SDREXTRUDEPRIMITIVE3D:
            return "SDREXTRUDEPRIMITIVE3D";
        case PRIMITIVE3D_ID_SDRLATHEPRIMITIVE3D:
            return "SDRLATHEPRIMITIVE3D";
        case PRIMITIVE3D_ID_SDRPOLYPOLYGONPRIMITIVE3D:
            return "SDRPOLYPOLYGONPRIMITIVE3D";
        case PRIMITIVE3D_ID_SDRSPHEREPRIMITIVE3D:
            return "SDRSPHEREPRIMITIVE3D";
        case PRIMITIVE3D_ID_SHADOWPRIMITIVE3D:
            return "SHADOWPRIMITIVE3D";
        case PRIMITIVE3D_ID_UNIFIEDTRANSPARENCETEXTUREPRIMITIVE3D:
            return "UNIFIEDTRANSPARENCETEXTUREPRIMITIVE3D";
        case PRIMITIVE3D_ID_GRADIENTTEXTUREPRIMITIVE3D:
            return "GRADIENTTEXTUREPRIMITIVE3D";
        case PRIMITIVE3D_ID_BITMAPTEXTUREPRIMITIVE3D:
            return "BITMAPTEXTUREPRIMITIVE3D";
        case PRIMITIVE3D_ID_TRANSPARENCETEXTUREPRIMITIVE3D:
            return "TRANSPARENCETEXTUREPRIMITIVE3D";
        case PRIMITIVE3D_ID_TRANSFORMPRIMITIVE3D:
            return "TRANSFORMPRIMITIVE3D";
        case PRIMITIVE3D_ID_HIDDENGEOMETRYPRIMITIVE3D:
            return "HIDDENGEOMETRYPRIMITIVE3D";
        default:
            return OUString::number((nId >> 16) & 0xFF) + "|" + OUString::number(nId & 0xFF);
    }
}

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
