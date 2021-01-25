/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.hxx>
#include <unordered_map>
#include "presetooxhandleadjustmentrelations.hxx"

namespace
{
typedef std::unordered_map<OUString, OUString> HandleAdjRelHashMap;

struct HandleAdjRel
{
    // Shape name without leading "ooxml-", underscore, zero based handle index
    // e.g. The third handle in shape of type "ooxml-circularArrow" will be
    // identified by key "circularArrow_2"
    const OUString sShape_Handle;

    // 4 tokens with separator "|"
    // first: RefX or RefR, na if not exists
    // second: adj, or adj1 or adj2, etc. as in preset, na if not exists
    // third: RefY or RefAngle, na if not exists
    // forth: adj, or adj1 or adj2, etc. as in preset, na if not exists
    // e.g. The third handle in shape <circularArrow> has in the preset
    // the tag <ahPolar gdRefR="adj5" minR="0" maxR="25000"> .
    // The resulting value in the map here is "RefR|adj5|na|na"
    const OUString sAdjReferences;
};

// The array initializer has been extracted from
// oox/source/drawingml/customshapes/presetShapeDefinitions.xml
// by using an XSLT file. That file is attached to tdf#126512.
const HandleAdjRel aHandleAdjRelArray[]
    = { { "accentBorderCallout1_0", "RefX|adj2|RefY|adj1" },
        { "accentBorderCallout1_1", "RefX|adj4|RefY|adj3" },
        { "accentBorderCallout2_0", "RefX|adj2|RefY|adj1" },
        { "accentBorderCallout2_1", "RefX|adj4|RefY|adj3" },
        { "accentBorderCallout2_2", "RefX|adj6|RefY|adj5" },
        { "accentBorderCallout3_0", "RefX|adj2|RefY|adj1" },
        { "accentBorderCallout3_1", "RefX|adj4|RefY|adj3" },
        { "accentBorderCallout3_2", "RefX|adj6|RefY|adj5" },
        { "accentBorderCallout3_3", "RefX|adj8|RefY|adj7" },
        { "accentCallout1_0", "RefX|adj2|RefY|adj1" },
        { "accentCallout1_1", "RefX|adj4|RefY|adj3" },
        { "accentCallout2_0", "RefX|adj2|RefY|adj1" },
        { "accentCallout2_1", "RefX|adj4|RefY|adj3" },
        { "accentCallout2_2", "RefX|adj6|RefY|adj5" },
        { "accentCallout3_0", "RefX|adj2|RefY|adj1" },
        { "accentCallout3_1", "RefX|adj4|RefY|adj3" },
        { "accentCallout3_2", "RefX|adj6|RefY|adj5" },
        { "accentCallout3_3", "RefX|adj8|RefY|adj7" },
        { "arc_0", "na|na|RefAngle|adj1" },
        { "arc_1", "na|na|RefAngle|adj2" },
        { "bentArrow_0", "RefX|adj1|na|na" },
        { "bentArrow_1", "na|na|RefY|adj2" },
        { "bentArrow_2", "RefX|adj3|na|na" },
        { "bentArrow_3", "RefX|adj4|na|na" },
        { "bentConnector3_0", "RefX|adj1|na|na" },
        { "bentConnector4_0", "RefX|adj1|na|na" },
        { "bentConnector4_1", "na|na|RefY|adj2" },
        { "bentConnector5_0", "RefX|adj1|na|na" },
        { "bentConnector5_1", "na|na|RefY|adj2" },
        { "bentConnector5_2", "RefX|adj3|na|na" },
        { "bentUpArrow_0", "na|na|RefY|adj1" },
        { "bentUpArrow_1", "RefX|adj2|na|na" },
        { "bentUpArrow_2", "na|na|RefY|adj3" },
        { "bevel_0", "RefX|adj|na|na" },
        { "blockArc_0", "na|na|RefAngle|adj1" },
        { "blockArc_1", "RefR|adj3|RefAngle|adj2" },
        { "borderCallout1_0", "RefX|adj2|RefY|adj1" },
        { "borderCallout1_1", "RefX|adj4|RefY|adj3" },
        { "borderCallout2_0", "RefX|adj2|RefY|adj1" },
        { "borderCallout2_1", "RefX|adj4|RefY|adj3" },
        { "borderCallout2_2", "RefX|adj6|RefY|adj5" },
        { "borderCallout3_0", "RefX|adj2|RefY|adj1" },
        { "borderCallout3_1", "RefX|adj4|RefY|adj3" },
        { "borderCallout3_2", "RefX|adj6|RefY|adj5" },
        { "borderCallout3_3", "RefX|adj8|RefY|adj7" },
        { "bracePair_0", "na|na|RefY|adj" },
        { "bracketPair_0", "na|na|RefY|adj" },
        { "callout1_0", "RefX|adj2|RefY|adj1" },
        { "callout1_1", "RefX|adj4|RefY|adj3" },
        { "callout2_0", "RefX|adj2|RefY|adj1" },
        { "callout2_1", "RefX|adj4|RefY|adj3" },
        { "callout2_2", "RefX|adj6|RefY|adj5" },
        { "callout3_0", "RefX|adj2|RefY|adj1" },
        { "callout3_1", "RefX|adj4|RefY|adj3" },
        { "callout3_2", "RefX|adj6|RefY|adj5" },
        { "callout3_3", "RefX|adj8|RefY|adj7" },
        { "can_0", "na|na|RefY|adj" },
        { "chevron_0", "RefX|adj|na|na" },
        { "chord_0", "na|na|RefAngle|adj1" },
        { "chord_1", "na|na|RefAngle|adj2" },
        { "circularArrow_0", "na|na|RefAngle|adj2" },
        { "circularArrow_1", "na|na|RefAngle|adj4" },
        { "circularArrow_2", "RefR|adj1|RefAngle|adj3" },
        { "circularArrow_3", "RefR|adj5|na|na" },
        { "cloudCallout_0", "RefX|adj1|RefY|adj2" },
        { "corner_0", "na|na|RefY|adj1" },
        { "corner_1", "RefX|adj2|na|na" },
        { "cube_0", "na|na|RefY|adj" },
        { "curvedConnector3_0", "RefX|adj1|na|na" },
        { "curvedConnector4_0", "RefX|adj1|na|na" },
        { "curvedConnector4_1", "na|na|RefY|adj2" },
        { "curvedConnector5_0", "RefX|adj1|na|na" },
        { "curvedConnector5_1", "na|na|RefY|adj2" },
        { "curvedConnector5_2", "RefX|adj3|na|na" },
        { "curvedDownArrow_0", "RefX|adj1|na|na" },
        { "curvedDownArrow_1", "RefX|adj2|na|na" },
        { "curvedDownArrow_2", "na|na|RefY|adj3" },
        { "curvedLeftArrow_0", "na|na|RefY|adj1" },
        { "curvedLeftArrow_1", "na|na|RefY|adj2" },
        { "curvedLeftArrow_2", "RefX|adj3|na|na" },
        { "curvedRightArrow_0", "na|na|RefY|adj1" },
        { "curvedRightArrow_1", "na|na|RefY|adj2" },
        { "curvedRightArrow_2", "RefX|adj3|na|na" },
        { "curvedUpArrow_0", "RefX|adj1|na|na" },
        { "curvedUpArrow_1", "RefX|adj2|na|na" },
        { "curvedUpArrow_2", "na|na|RefY|adj3" },
        { "diagStripe_0", "na|na|RefY|adj" },
        { "donut_0", "RefR|adj|na|na" },
        { "doubleWave_0", "na|na|RefY|adj1" },
        { "doubleWave_1", "RefX|adj2|na|na" },
        { "downArrow_0", "RefX|adj1|na|na" },
        { "downArrow_1", "na|na|RefY|adj2" },
        { "downArrowCallout_0", "RefX|adj1|na|na" },
        { "downArrowCallout_1", "RefX|adj2|na|na" },
        { "downArrowCallout_2", "na|na|RefY|adj3" },
        { "downArrowCallout_3", "na|na|RefY|adj4" },
        { "ellipseRibbon_0", "na|na|RefY|adj1" },
        { "ellipseRibbon_1", "RefX|adj2|na|na" },
        { "ellipseRibbon_2", "na|na|RefY|adj3" },
        { "ellipseRibbon2_0", "na|na|RefY|adj1" },
        { "ellipseRibbon2_1", "RefX|adj2|na|na" },
        { "ellipseRibbon2_2", "na|na|RefY|adj3" },
        { "foldedCorner_0", "RefX|adj|na|na" },
        { "frame_0", "RefX|adj1|na|na" },
        { "gear6_0", "na|na|RefY|adj1" },
        { "gear6_1", "RefX|adj2|na|na" },
        { "gear9_0", "na|na|RefY|adj1" },
        { "gear9_1", "RefX|adj2|na|na" },
        { "halfFrame_0", "na|na|RefY|adj1" },
        { "halfFrame_1", "RefX|adj2|na|na" },
        { "hexagon_0", "RefX|adj|na|na" },
        { "homePlate_0", "RefX|adj|na|na" },
        { "horizontalScroll_0", "RefX|adj|na|na" },
        { "leftArrow_0", "na|na|RefY|adj1" },
        { "leftArrow_1", "RefX|adj2|na|na" },
        { "leftArrowCallout_0", "na|na|RefY|adj1" },
        { "leftArrowCallout_1", "na|na|RefY|adj2" },
        { "leftArrowCallout_2", "RefX|adj3|na|na" },
        { "leftArrowCallout_3", "RefX|adj4|na|na" },
        { "leftBrace_0", "na|na|RefY|adj1" },
        { "leftBrace_1", "na|na|RefY|adj2" },
        { "leftBracket_0", "na|na|RefY|adj" },
        { "leftCircularArrow_0", "na|na|RefAngle|adj2" },
        { "leftCircularArrow_1", "na|na|RefAngle|adj4" },
        { "leftCircularArrow_2", "RefR|adj1|RefAngle|adj3" },
        { "leftCircularArrow_3", "RefR|adj5|na|na" },
        { "leftRightArrow_0", "na|na|RefY|adj1" },
        { "leftRightArrow_1", "RefX|adj2|na|na" },
        { "leftRightArrowCallout_0", "na|na|RefY|adj1" },
        { "leftRightArrowCallout_1", "na|na|RefY|adj2" },
        { "leftRightArrowCallout_2", "RefX|adj3|na|na" },
        { "leftRightArrowCallout_3", "RefX|adj4|na|na" },
        { "leftRightCircularArrow_0", "na|na|RefAngle|adj2" },
        { "leftRightCircularArrow_1", "na|na|RefAngle|adj4" },
        { "leftRightCircularArrow_2", "RefR|adj1|RefAngle|adj3" },
        { "leftRightCircularArrow_3", "RefR|adj5|na|na" },
        { "leftRightRibbon_0", "na|na|RefY|adj1" },
        { "leftRightRibbon_1", "RefX|adj2|na|na" },
        { "leftRightRibbon_2", "na|na|RefY|adj3" },
        { "leftRightUpArrow_0", "RefX|adj1|na|na" },
        { "leftRightUpArrow_1", "RefX|adj2|na|na" },
        { "leftRightUpArrow_2", "na|na|RefY|adj3" },
        { "leftUpArrow_0", "na|na|RefY|adj1" },
        { "leftUpArrow_1", "RefX|adj2|na|na" },
        { "leftUpArrow_2", "na|na|RefY|adj3" },
        { "mathDivide_0", "na|na|RefY|adj1" },
        { "mathDivide_1", "na|na|RefY|adj2" },
        { "mathDivide_2", "RefX|adj3|na|na" },
        { "mathEqual_0", "na|na|RefY|adj1" },
        { "mathEqual_1", "na|na|RefY|adj2" },
        { "mathMinus_0", "na|na|RefY|adj1" },
        { "mathMultiply_0", "na|na|RefY|adj1" },
        { "mathNotEqual_0", "na|na|RefY|adj1" },
        { "mathNotEqual_1", "na|na|RefAngle|adj2" },
        { "mathNotEqual_2", "na|na|RefY|adj3" },
        { "mathPlus_0", "na|na|RefY|adj1" },
        { "moon_0", "RefX|adj|na|na" },
        { "nonIsoscelesTrapezoid_0", "RefX|adj1|na|na" },
        { "nonIsoscelesTrapezoid_1", "RefX|adj2|na|na" },
        { "noSmoking_0", "RefR|adj|na|na" },
        { "notchedRightArrow_0", "na|na|RefY|adj1" },
        { "notchedRightArrow_1", "RefX|adj2|na|na" },
        { "octagon_0", "RefX|adj|na|na" },
        { "parallelogram_0", "RefX|adj|na|na" },
        { "pie_0", "na|na|RefAngle|adj1" },
        { "pie_1", "na|na|RefAngle|adj2" },
        { "plaque_0", "RefX|adj|na|na" },
        { "plus_0", "RefX|adj|na|na" },
        { "quadArrow_0", "RefX|adj1|na|na" },
        { "quadArrow_1", "RefX|adj2|na|na" },
        { "quadArrow_2", "na|na|RefY|adj3" },
        { "quadArrowCallout_0", "RefX|adj1|na|na" },
        { "quadArrowCallout_1", "RefX|adj2|na|na" },
        { "quadArrowCallout_2", "na|na|RefY|adj3" },
        { "quadArrowCallout_3", "na|na|RefY|adj4" },
        { "ribbon_0", "na|na|RefY|adj1" },
        { "ribbon_1", "RefX|adj2|na|na" },
        { "ribbon2_0", "na|na|RefY|adj1" },
        { "ribbon2_1", "RefX|adj2|na|na" },
        { "rightArrow_0", "na|na|RefY|adj1" },
        { "rightArrow_1", "RefX|adj2|na|na" },
        { "rightArrowCallout_0", "na|na|RefY|adj1" },
        { "rightArrowCallout_1", "na|na|RefY|adj2" },
        { "rightArrowCallout_2", "RefX|adj3|na|na" },
        { "rightArrowCallout_3", "RefX|adj4|na|na" },
        { "rightBrace_0", "na|na|RefY|adj1" },
        { "rightBrace_1", "na|na|RefY|adj2" },
        { "rightBracket_0", "na|na|RefY|adj" },
        { "round1Rect_0", "RefX|adj|na|na" },
        { "round2DiagRect_0", "RefX|adj1|na|na" },
        { "round2DiagRect_1", "RefX|adj2|na|na" },
        { "round2SameRect_0", "RefX|adj1|na|na" },
        { "round2SameRect_1", "RefX|adj2|na|na" },
        { "roundRect_0", "RefX|adj|na|na" },
        { "smileyFace_0", "na|na|RefY|adj" },
        { "snip1Rect_0", "RefX|adj|na|na" },
        { "snip2DiagRect_0", "RefX|adj1|na|na" },
        { "snip2DiagRect_1", "RefX|adj2|na|na" },
        { "snip2SameRect_0", "RefX|adj1|na|na" },
        { "snip2SameRect_1", "RefX|adj2|na|na" },
        { "snipRoundRect_0", "RefX|adj1|na|na" },
        { "snipRoundRect_1", "RefX|adj2|na|na" },
        { "star10_0", "na|na|RefY|adj" },
        { "star12_0", "na|na|RefY|adj" },
        { "star16_0", "na|na|RefY|adj" },
        { "star24_0", "na|na|RefY|adj" },
        { "star32_0", "na|na|RefY|adj" },
        { "star4_0", "na|na|RefY|adj" },
        { "star5_0", "na|na|RefY|adj" },
        { "star6_0", "na|na|RefY|adj" },
        { "star7_0", "na|na|RefY|adj" },
        { "star8_0", "na|na|RefY|adj" },
        { "stripedRightArrow_0", "na|na|RefY|adj1" },
        { "stripedRightArrow_1", "RefX|adj2|na|na" },
        { "sun_0", "RefX|adj|na|na" },
        { "swooshArrow_0", "na|na|RefY|adj1" },
        { "swooshArrow_1", "RefX|adj2|na|na" },
        { "teardrop_0", "RefX|adj|na|na" },
        { "trapezoid_0", "RefX|adj|na|na" },
        { "triangle_0", "RefX|adj|na|na" },
        { "upArrowCallout_0", "RefX|adj1|na|na" },
        { "upArrowCallout_1", "RefX|adj2|na|na" },
        { "upArrowCallout_2", "na|na|RefY|adj3" },
        { "upArrowCallout_3", "na|na|RefY|adj4" },
        { "upDownArrow_0", "RefX|adj1|na|na" },
        { "upDownArrow_1", "na|na|RefY|adj2" },
        { "upArrow_0", "RefX|adj1|na|na" },
        { "upArrow_1", "na|na|RefY|adj2" },
        { "upDownArrowCallout_0", "RefX|adj1|na|na" },
        { "upDownArrowCallout_1", "RefX|adj2|na|na" },
        { "upDownArrowCallout_2", "na|na|RefY|adj3" },
        { "upDownArrowCallout_3", "na|na|RefY|adj4" },
        { "uturnArrow_0", "RefX|adj1|na|na" },
        { "uturnArrow_1", "RefX|adj2|na|na" },
        { "uturnArrow_2", "na|na|RefY|adj3" },
        { "uturnArrow_3", "RefX|adj4|na|na" },
        { "uturnArrow_4", "na|na|RefY|adj5" },
        { "verticalScroll_0", "na|na|RefY|adj" },
        { "wave_0", "na|na|RefY|adj1" },
        { "wave_1", "RefX|adj2|na|na" },
        { "wedgeEllipseCallout_0", "RefX|adj1|RefY|adj2" },
        { "wedgeRectCallout_0", "RefX|adj1|RefY|adj2" },
        { "wedgeRoundRectCallout_0", "RefX|adj1|RefY|adj2" } };
}

static sal_Int32 lcl_getAdjIndexFromToken(const sal_Int32 nTokenPos, const OUString& rMapValue)
{
    OUString sAdjRef = rMapValue.getToken(nTokenPos, '|');
    OUString sNumber; // number part from "adj1", "adj2" etc.
    if (sAdjRef.startsWith("adj", &sNumber))
    {
        if (sNumber.isEmpty() || sNumber == "1")
            return 0;
        else
            return sNumber.toInt32() - 1;
    }
    else
        return -1;
}

void PresetOOXHandleAdj::GetOOXHandleAdjRelation(
    const OUString& sFullOOXShapeName, const sal_Int32 nHandleIndex, OUString& rFirstRefType,
    sal_Int32& rFirstAdjValueIndex, OUString& rSecondRefType, sal_Int32& rSecondAdjValueIndex)
{
    static const HandleAdjRelHashMap s_HashMap = []() {
        HandleAdjRelHashMap aH;
        for (const auto& item : aHandleAdjRelArray)
            aH[item.sShape_Handle] = item.sAdjReferences;
        return aH;
    }();

    OUString sKey;
    OUString sValue;
    rFirstRefType = "na";
    rFirstAdjValueIndex = -1;
    rSecondRefType = "na";
    rSecondAdjValueIndex = -1;
    if (sFullOOXShapeName.startsWith("ooxml-", &sKey))
    {
        sKey += "_" + OUString::number(nHandleIndex);
        HandleAdjRelHashMap::const_iterator aHashIter(s_HashMap.find(sKey));
        if (aHashIter != s_HashMap.end())
            sValue = (*aHashIter).second;
        else
            return;
    }
    else
        return;

    rFirstRefType = sValue.getToken(0, '|');
    rFirstAdjValueIndex = lcl_getAdjIndexFromToken(1, sValue);
    rSecondRefType = sValue.getToken(2, '|');
    rSecondAdjValueIndex = lcl_getAdjIndexFromToken(3, sValue);
    return;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
