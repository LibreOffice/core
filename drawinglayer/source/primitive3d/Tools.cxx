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
            return u"GROUPPRIMITIVE3D"_ustr;
        case PRIMITIVE3D_ID_HATCHTEXTUREPRIMITIVE3D:
            return u"HATCHTEXTUREPRIMITIVE3D"_ustr;
        case PRIMITIVE3D_ID_MODIFIEDCOLORPRIMITIVE3D:
            return u"MODIFIEDCOLORPRIMITIVE3D"_ustr;
        case PRIMITIVE3D_ID_POLYGONHAIRLINEPRIMITIVE3D:
            return u"POLYGONHAIRLINEPRIMITIVE3D"_ustr;
        case PRIMITIVE3D_ID_POLYGONSTROKEPRIMITIVE3D:
            return u"POLYGONSTROKEPRIMITIVE3D"_ustr;
        case PRIMITIVE3D_ID_POLYGONTUBEPRIMITIVE3D:
            return u"POLYGONTUBEPRIMITIVE3D"_ustr;
        case PRIMITIVE3D_ID_POLYPOLYGONMATERIALPRIMITIVE3D:
            return u"POLYPOLYGONMATERIALPRIMITIVE3D"_ustr;
        case PRIMITIVE3D_ID_SDRCUBEPRIMITIVE3D:
            return u"SDRCUBEPRIMITIVE3D"_ustr;
        case PRIMITIVE3D_ID_SDREXTRUDEPRIMITIVE3D:
            return u"SDREXTRUDEPRIMITIVE3D"_ustr;
        case PRIMITIVE3D_ID_SDRLATHEPRIMITIVE3D:
            return u"SDRLATHEPRIMITIVE3D"_ustr;
        case PRIMITIVE3D_ID_SDRPOLYPOLYGONPRIMITIVE3D:
            return u"SDRPOLYPOLYGONPRIMITIVE3D"_ustr;
        case PRIMITIVE3D_ID_SDRSPHEREPRIMITIVE3D:
            return u"SDRSPHEREPRIMITIVE3D"_ustr;
        case PRIMITIVE3D_ID_SHADOWPRIMITIVE3D:
            return u"SHADOWPRIMITIVE3D"_ustr;
        case PRIMITIVE3D_ID_UNIFIEDTRANSPARENCETEXTUREPRIMITIVE3D:
            return u"UNIFIEDTRANSPARENCETEXTUREPRIMITIVE3D"_ustr;
        case PRIMITIVE3D_ID_GRADIENTTEXTUREPRIMITIVE3D:
            return u"GRADIENTTEXTUREPRIMITIVE3D"_ustr;
        case PRIMITIVE3D_ID_BITMAPTEXTUREPRIMITIVE3D:
            return u"BITMAPTEXTUREPRIMITIVE3D"_ustr;
        case PRIMITIVE3D_ID_TRANSPARENCETEXTUREPRIMITIVE3D:
            return u"TRANSPARENCETEXTUREPRIMITIVE3D"_ustr;
        case PRIMITIVE3D_ID_TRANSFORMPRIMITIVE3D:
            return u"TRANSFORMPRIMITIVE3D"_ustr;
        case PRIMITIVE3D_ID_HIDDENGEOMETRYPRIMITIVE3D:
            return u"HIDDENGEOMETRYPRIMITIVE3D"_ustr;
        default:
            return OUString::number((nId >> 16) & 0xFF) + "|" + OUString::number(nId & 0xFF);
    }
}

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
