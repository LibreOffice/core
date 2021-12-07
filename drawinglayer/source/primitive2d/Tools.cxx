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
#include <basegfx/utils/canvastools.hxx>

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
            return "TRANSPARENCE";
        case PRIMITIVE2D_ID_ANIMATEDSWITCHPRIMITIVE2D:
            return "ANIMATEDSWITCH";
        case PRIMITIVE2D_ID_ANIMATEDBLINKPRIMITIVE2D:
            return "ANIMATEDBLINK";
        case PRIMITIVE2D_ID_ANIMATEDINTERPOLATEPRIMITIVE2D:
            return "ANIMATEDINTERPOLATE";
        case PRIMITIVE2D_ID_BACKGROUNDCOLORPRIMITIVE2D:
            return "BACKGROUNDCOLOR";
        case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D:
            return "BITMAP";
        case PRIMITIVE2D_ID_CONTROLPRIMITIVE2D:
            return "CONTROL";
        case PRIMITIVE2D_ID_EMBEDDED3DPRIMITIVE2D:
            return "EMBEDDED3D";
        case PRIMITIVE2D_ID_FILLGRAPHICPRIMITIVE2D:
            return "FILLGRAPHIC";
        case PRIMITIVE2D_ID_FILLGRADIENTPRIMITIVE2D:
            return "FILLGRADIENT";
        case PRIMITIVE2D_ID_FILLHATCHPRIMITIVE2D:
            return "FILLHATCH";
        case PRIMITIVE2D_ID_GRAPHICPRIMITIVE2D:
            return "GRAPHIC";
        case PRIMITIVE2D_ID_GRIDPRIMITIVE2D:
            return "GRID";
        case PRIMITIVE2D_ID_GROUPPRIMITIVE2D:
            return "GROUP";
        case PRIMITIVE2D_ID_HELPLINEPRIMITIVE2D:
            return "HELPLINE";
        case PRIMITIVE2D_ID_MARKERARRAYPRIMITIVE2D:
            return "MARKERARRAY";
        case PRIMITIVE2D_ID_MASKPRIMITIVE2D:
            return "MASK";
        case PRIMITIVE2D_ID_MEDIAPRIMITIVE2D:
            return "MEDIA";
        case PRIMITIVE2D_ID_METAFILEPRIMITIVE2D:
            return "METAFILE";
        case PRIMITIVE2D_ID_MODIFIEDCOLORPRIMITIVE2D:
            return "MODIFIEDCOLOR";
        case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D:
            return "POLYGONHAIRLINE";
        case PRIMITIVE2D_ID_POLYGONMARKERPRIMITIVE2D:
            return "POLYGONMARKER";
        case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D:
            return "POLYGONSTROKE";
        case PRIMITIVE2D_ID_POLYGONSTROKEARROWPRIMITIVE2D:
            return "POLYGONSTROKEARROW";
        case PRIMITIVE2D_ID_POLYPOLYGONSTROKEPRIMITIVE2D:
            return "POLYPOLYGONSTROKE";
        case PRIMITIVE2D_ID_POLYPOLYGONSTROKEARROWPRIMITIVE2D:
            return "POLYPOLYGONSTROKEARROW";
        case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D:
            return "POLYPOLYGONCOLOR";
        case PRIMITIVE2D_ID_POLYPOLYGONGRADIENTPRIMITIVE2D:
            return "POLYPOLYGONGRADIENT";
        case PRIMITIVE2D_ID_POLYPOLYGONHATCHPRIMITIVE2D:
            return "POLYPOLYGONHATCH";
        case PRIMITIVE2D_ID_POLYPOLYGONGRAPHICPRIMITIVE2D:
            return "POLYPOLYGONGRAPHIC";
        case PRIMITIVE2D_ID_SCENEPRIMITIVE2D:
            return "SCENE";
        case PRIMITIVE2D_ID_SHADOWPRIMITIVE2D:
            return "SHADOW";
        case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D:
            return "TEXTSIMPLEPORTION";
        case PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D:
            return "TEXTDECORATEDPORTION";
        case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D:
            return "TRANSFORM";
        case PRIMITIVE2D_ID_UNIFIEDTRANSPARENCEPRIMITIVE2D:
            return "UNIFIEDTRANSPARENCE";
        case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D:
            return "POINTARRAY";
        case PRIMITIVE2D_ID_TEXTHIERARCHYFIELDPRIMITIVE2D:
            return "TEXTHIERARCHYFIELD";
        case PRIMITIVE2D_ID_TEXTHIERARCHYLINEPRIMITIVE2D:
            return "TEXTHIERARCHYLINE";
        case PRIMITIVE2D_ID_TEXTHIERARCHYPARAGRAPHPRIMITIVE2D:
            return "TEXTHIERARCHYPARAGRAPH";
        case PRIMITIVE2D_ID_TEXTHIERARCHYBLOCKPRIMITIVE2D:
            return "TEXTHIERARCHYBLOCK";
        case PRIMITIVE2D_ID_TEXTHIERARCHYEDITPRIMITIVE2D:
            return "TEXTHIERARCHYEDIT";
        case PRIMITIVE2D_ID_POLYGONWAVEPRIMITIVE2D:
            return "POLYGONWAVE";
        case PRIMITIVE2D_ID_WRONGSPELLPRIMITIVE2D:
            return "WRONGSPELL";
        case PRIMITIVE2D_ID_TEXTEFFECTPRIMITIVE2D:
            return "TEXTEFFECT";
        case PRIMITIVE2D_ID_TEXTHIERARCHYBULLETPRIMITIVE2D:
            return "TEXTHIERARCHYBULLET";
        case PRIMITIVE2D_ID_POLYPOLYGONHAIRLINEPRIMITIVE2D:
            return "POLYPOLYGONHAIRLINE";
        case PRIMITIVE2D_ID_EXECUTEPRIMITIVE2D:
            return "EXECUTE";
        case PRIMITIVE2D_ID_PAGEPREVIEWPRIMITIVE2D:
            return "PAGEPREVIEW";
        case PRIMITIVE2D_ID_STRUCTURETAGPRIMITIVE2D:
            return "STRUCTURETAG";
        case PRIMITIVE2D_ID_BORDERLINEPRIMITIVE2D:
            return "BORDERLINE";
        case PRIMITIVE2D_ID_POLYPOLYGONMARKERPRIMITIVE2D:
            return "POLYPOLYGONMARKER";
        case PRIMITIVE2D_ID_HITTESTPRIMITIVE2D:
            return "HITTEST";
        case PRIMITIVE2D_ID_INVERTPRIMITIVE2D:
            return "INVERT";
        case PRIMITIVE2D_ID_DISCRETEBITMAPPRIMITIVE2D:
            return "DISCRETEBITMAP";
        case PRIMITIVE2D_ID_WALLPAPERBITMAPPRIMITIVE2D:
            return "WALLPAPERBITMAP";
        case PRIMITIVE2D_ID_TEXTLINEPRIMITIVE2D:
            return "TEXTLINE";
        case PRIMITIVE2D_ID_TEXTCHARACTERSTRIKEOUTPRIMITIVE2D:
            return "TEXTCHARACTERSTRIKEOUT";
        case PRIMITIVE2D_ID_TEXTGEOMETRYSTRIKEOUTPRIMITIVE2D:
            return "TEXTGEOMETRYSTRIKEOUT";
        case PRIMITIVE2D_ID_EPSPRIMITIVE2D:
            return "EPS";
        case PRIMITIVE2D_ID_DISCRETESHADOWPRIMITIVE2D:
            return "DISCRETESHADOW";
        case PRIMITIVE2D_ID_HIDDENGEOMETRYPRIMITIVE2D:
            return "HIDDENGEOMETRY";
        case PRIMITIVE2D_ID_SVGLINEARGRADIENTPRIMITIVE2D:
            return "SVGLINEARGRADIENT";
        case PRIMITIVE2D_ID_SVGRADIALGRADIENTPRIMITIVE2D:
            return "SVGRADIALGRADIENT";
        case PRIMITIVE2D_ID_SVGLINEARATOMPRIMITIVE2D:
            return "SVGLINEARATOM";
        case PRIMITIVE2D_ID_SVGRADIALATOMPRIMITIVE2D:
            return "SVGRADIALATOM";
        case PRIMITIVE2D_ID_CROPPRIMITIVE2D:
            return "CROP";
        case PRIMITIVE2D_ID_PATTERNFILLPRIMITIVE2D:
            return "PATTERNFILL";
        case PRIMITIVE2D_ID_OBJECTINFOPRIMITIVE2D:
            return "OBJECTINFO";
        case PRIMITIVE2D_ID_POLYPOLYGONSELECTIONPRIMITIVE2D:
            return "POLYPOLYGONSELECTION";
        case PRIMITIVE2D_ID_PAGEHIERARCHYPRIMITIVE2D:
            return "PAGEHIERARCHY";
        case PRIMITIVE2D_ID_GLOWPRIMITIVE2D:
            return "GLOWPRIMITIVE";
        case PRIMITIVE2D_ID_SOFTEDGEPRIMITIVE2D:
            return "SOFTEDGEPRIMITIVE";
        default:
            return OUString::number((nId >> 16) & 0xFF) + "|" + OUString::number(nId & 0xFF);
    }
}

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
