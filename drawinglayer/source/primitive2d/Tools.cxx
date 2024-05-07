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

#include <drawinglayer/primitive2d/Tools.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>

using namespace css;

namespace drawinglayer::primitive2d
{
// get B2DRange from a given Primitive2DReference
basegfx::B2DRange
getB2DRangeFromPrimitive2DReference(const Primitive2DReference& rCandidate,
                                    const geometry::ViewInformation2D& aViewInformation)
{
    if (!rCandidate)
        return basegfx::B2DRange();

    return rCandidate->getB2DRange(aViewInformation);
}

bool arePrimitive2DReferencesEqual(const Primitive2DReference& rxA, const Primitive2DReference& rxB)
{
    const bool bAIs(rxA.is());

    if (bAIs != rxB.is())
    {
        return false;
    }

    if (!bAIs)
    {
        return true;
    }

    return rxA->operator==(*rxB);
}

bool arePrimitive2DReferencesEqual(const css::uno::Reference<css::graphic::XPrimitive2D>& rxA,
                                   const css::uno::Reference<css::graphic::XPrimitive2D>& rxB)
{
    const bool bAIs(rxA.is());

    if (bAIs != rxB.is())
    {
        return false;
    }

    if (!bAIs)
    {
        return true;
    }

    auto pA = static_cast<const UnoPrimitive2D*>(rxA.get());
    auto pB = static_cast<const UnoPrimitive2D*>(rxB.get());

    return (*pA->getBasePrimitive2D()) == (*pB->getBasePrimitive2D());
}

OUString idToString(sal_uInt32 nId)
{
    switch (nId)
    {
        case PRIMITIVE2D_ID_TRANSPARENCEPRIMITIVE2D:
            return u"TRANSPARENCE"_ustr;
        case PRIMITIVE2D_ID_ANIMATEDSWITCHPRIMITIVE2D:
            return u"ANIMATEDSWITCH"_ustr;
        case PRIMITIVE2D_ID_ANIMATEDBLINKPRIMITIVE2D:
            return u"ANIMATEDBLINK"_ustr;
        case PRIMITIVE2D_ID_ANIMATEDINTERPOLATEPRIMITIVE2D:
            return u"ANIMATEDINTERPOLATE"_ustr;
        case PRIMITIVE2D_ID_BACKGROUNDCOLORPRIMITIVE2D:
            return u"BACKGROUNDCOLOR"_ustr;
        case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D:
            return u"BITMAP"_ustr;
        case PRIMITIVE2D_ID_CONTROLPRIMITIVE2D:
            return u"CONTROL"_ustr;
        case PRIMITIVE2D_ID_EMBEDDED3DPRIMITIVE2D:
            return u"EMBEDDED3D"_ustr;
        case PRIMITIVE2D_ID_FILLGRAPHICPRIMITIVE2D:
            return u"FILLGRAPHIC"_ustr;
        case PRIMITIVE2D_ID_FILLGRADIENTPRIMITIVE2D:
            return u"FILLGRADIENT"_ustr;
        case PRIMITIVE2D_ID_FILLHATCHPRIMITIVE2D:
            return u"FILLHATCH"_ustr;
        case PRIMITIVE2D_ID_GRAPHICPRIMITIVE2D:
            return u"GRAPHIC"_ustr;
        case PRIMITIVE2D_ID_GRIDPRIMITIVE2D:
            return u"GRID"_ustr;
        case PRIMITIVE2D_ID_GROUPPRIMITIVE2D:
            return u"GROUP"_ustr;
        case PRIMITIVE2D_ID_HELPLINEPRIMITIVE2D:
            return u"HELPLINE"_ustr;
        case PRIMITIVE2D_ID_MARKERARRAYPRIMITIVE2D:
            return u"MARKERARRAY"_ustr;
        case PRIMITIVE2D_ID_MASKPRIMITIVE2D:
            return u"MASK"_ustr;
        case PRIMITIVE2D_ID_MEDIAPRIMITIVE2D:
            return u"MEDIA"_ustr;
        case PRIMITIVE2D_ID_METAFILEPRIMITIVE2D:
            return u"METAFILE"_ustr;
        case PRIMITIVE2D_ID_MODIFIEDCOLORPRIMITIVE2D:
            return u"MODIFIEDCOLOR"_ustr;
        case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D:
            return u"POLYGONHAIRLINE"_ustr;
        case PRIMITIVE2D_ID_POLYGONMARKERPRIMITIVE2D:
            return u"POLYGONMARKER"_ustr;
        case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D:
            return u"POLYGONSTROKE"_ustr;
        case PRIMITIVE2D_ID_POLYGONSTROKEARROWPRIMITIVE2D:
            return u"POLYGONSTROKEARROW"_ustr;
        case PRIMITIVE2D_ID_POLYPOLYGONSTROKEPRIMITIVE2D:
            return u"POLYPOLYGONSTROKE"_ustr;
        case PRIMITIVE2D_ID_POLYPOLYGONSTROKEARROWPRIMITIVE2D:
            return u"POLYPOLYGONSTROKEARROW"_ustr;
        case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D:
            return u"POLYPOLYGONCOLOR"_ustr;
        case PRIMITIVE2D_ID_POLYPOLYGONGRADIENTPRIMITIVE2D:
            return u"POLYPOLYGONGRADIENT"_ustr;
        case PRIMITIVE2D_ID_POLYPOLYGONHATCHPRIMITIVE2D:
            return u"POLYPOLYGONHATCH"_ustr;
        case PRIMITIVE2D_ID_POLYPOLYGONGRAPHICPRIMITIVE2D:
            return u"POLYPOLYGONGRAPHIC"_ustr;
        case PRIMITIVE2D_ID_SCENEPRIMITIVE2D:
            return u"SCENE"_ustr;
        case PRIMITIVE2D_ID_SHADOWPRIMITIVE2D:
            return u"SHADOW"_ustr;
        case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D:
            return u"TEXTSIMPLEPORTION"_ustr;
        case PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D:
            return u"TEXTDECORATEDPORTION"_ustr;
        case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D:
            return u"TRANSFORM"_ustr;
        case PRIMITIVE2D_ID_UNIFIEDTRANSPARENCEPRIMITIVE2D:
            return u"UNIFIEDTRANSPARENCE"_ustr;
        case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D:
            return u"POINTARRAY"_ustr;
        case PRIMITIVE2D_ID_TEXTHIERARCHYFIELDPRIMITIVE2D:
            return u"TEXTHIERARCHYFIELD"_ustr;
        case PRIMITIVE2D_ID_TEXTHIERARCHYLINEPRIMITIVE2D:
            return u"TEXTHIERARCHYLINE"_ustr;
        case PRIMITIVE2D_ID_TEXTHIERARCHYPARAGRAPHPRIMITIVE2D:
            return u"TEXTHIERARCHYPARAGRAPH"_ustr;
        case PRIMITIVE2D_ID_TEXTHIERARCHYBLOCKPRIMITIVE2D:
            return u"TEXTHIERARCHYBLOCK"_ustr;
        case PRIMITIVE2D_ID_TEXTHIERARCHYEDITPRIMITIVE2D:
            return u"TEXTHIERARCHYEDIT"_ustr;
        case PRIMITIVE2D_ID_POLYGONWAVEPRIMITIVE2D:
            return u"POLYGONWAVE"_ustr;
        case PRIMITIVE2D_ID_WRONGSPELLPRIMITIVE2D:
            return u"WRONGSPELL"_ustr;
        case PRIMITIVE2D_ID_TEXTEFFECTPRIMITIVE2D:
            return u"TEXTEFFECT"_ustr;
        case PRIMITIVE2D_ID_TEXTHIERARCHYBULLETPRIMITIVE2D:
            return u"TEXTHIERARCHYBULLET"_ustr;
        case PRIMITIVE2D_ID_POLYPOLYGONHAIRLINEPRIMITIVE2D:
            return u"POLYPOLYGONHAIRLINE"_ustr;
        case PRIMITIVE2D_ID_EXECUTEPRIMITIVE2D:
            return u"EXECUTE"_ustr;
        case PRIMITIVE2D_ID_PAGEPREVIEWPRIMITIVE2D:
            return u"PAGEPREVIEW"_ustr;
        case PRIMITIVE2D_ID_STRUCTURETAGPRIMITIVE2D:
            return u"STRUCTURETAG"_ustr;
        case PRIMITIVE2D_ID_BORDERLINEPRIMITIVE2D:
            return u"BORDERLINE"_ustr;
        case PRIMITIVE2D_ID_POLYPOLYGONMARKERPRIMITIVE2D:
            return u"POLYPOLYGONMARKER"_ustr;
        case PRIMITIVE2D_ID_HITTESTPRIMITIVE2D:
            return u"HITTEST"_ustr;
        case PRIMITIVE2D_ID_INVERTPRIMITIVE2D:
            return u"INVERT"_ustr;
        case PRIMITIVE2D_ID_DISCRETEBITMAPPRIMITIVE2D:
            return u"DISCRETEBITMAP"_ustr;
        case PRIMITIVE2D_ID_WALLPAPERBITMAPPRIMITIVE2D:
            return u"WALLPAPERBITMAP"_ustr;
        case PRIMITIVE2D_ID_TEXTLINEPRIMITIVE2D:
            return u"TEXTLINE"_ustr;
        case PRIMITIVE2D_ID_TEXTCHARACTERSTRIKEOUTPRIMITIVE2D:
            return u"TEXTCHARACTERSTRIKEOUT"_ustr;
        case PRIMITIVE2D_ID_TEXTGEOMETRYSTRIKEOUTPRIMITIVE2D:
            return u"TEXTGEOMETRYSTRIKEOUT"_ustr;
        case PRIMITIVE2D_ID_EPSPRIMITIVE2D:
            return u"EPS"_ustr;
        case PRIMITIVE2D_ID_DISCRETESHADOWPRIMITIVE2D:
            return u"DISCRETESHADOW"_ustr;
        case PRIMITIVE2D_ID_HIDDENGEOMETRYPRIMITIVE2D:
            return u"HIDDENGEOMETRY"_ustr;
        case PRIMITIVE2D_ID_SVGLINEARGRADIENTPRIMITIVE2D:
            return u"SVGLINEARGRADIENT"_ustr;
        case PRIMITIVE2D_ID_SVGRADIALGRADIENTPRIMITIVE2D:
            return u"SVGRADIALGRADIENT"_ustr;
        case PRIMITIVE2D_ID_SVGLINEARATOMPRIMITIVE2D:
            return u"SVGLINEARATOM"_ustr;
        case PRIMITIVE2D_ID_SVGRADIALATOMPRIMITIVE2D:
            return u"SVGRADIALATOM"_ustr;
        case PRIMITIVE2D_ID_CROPPRIMITIVE2D:
            return u"CROP"_ustr;
        case PRIMITIVE2D_ID_PATTERNFILLPRIMITIVE2D:
            return u"PATTERNFILL"_ustr;
        case PRIMITIVE2D_ID_OBJECTINFOPRIMITIVE2D:
            return u"OBJECTINFO"_ustr;
        case PRIMITIVE2D_ID_POLYPOLYGONSELECTIONPRIMITIVE2D:
            return u"POLYPOLYGONSELECTION"_ustr;
        case PRIMITIVE2D_ID_PAGEHIERARCHYPRIMITIVE2D:
            return u"PAGEHIERARCHY"_ustr;
        case PRIMITIVE2D_ID_GLOWPRIMITIVE2D:
            return u"GLOWPRIMITIVE"_ustr;
        case PRIMITIVE2D_ID_SOFTEDGEPRIMITIVE2D:
            return u"SOFTEDGEPRIMITIVE"_ustr;
        case PRIMITIVE2D_ID_LINERECTANGLEPRIMITIVE2D:
            return u"LINERECTANGLEPRIMITIVE"_ustr;
        case PRIMITIVE2D_ID_FILLEDRECTANGLEPRIMITIVE2D:
            return u"FILLEDRECTANGLEPRIMITIVE"_ustr;
        case PRIMITIVE2D_ID_SINGLELINEPRIMITIVE2D:
            return u"SINGLELINEPRIMITIVE"_ustr;
        case PRIMITIVE2D_ID_EXCLUSIVEEDITVIEWPRIMITIVE2D:
            return u"EXCLUSIVEEDITVIEWPRIMITIVE2D"_ustr;
        default:
            return OUString::number((nId >> 16) & 0xFF) + "|" + OUString::number(nId & 0xFF);
    }
}

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
