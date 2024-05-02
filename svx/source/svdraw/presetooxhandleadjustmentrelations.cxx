/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <o3tl/string_view.hxx>
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
    OUString sShape_Handle;

    // 4 tokens with separator "|"
    // first: RefX or RefR, na if not exists
    // second: adj, or adj1 or adj2, etc. as in preset, na if not exists
    // third: RefY or RefAngle, na if not exists
    // forth: adj, or adj1 or adj2, etc. as in preset, na if not exists
    // e.g. The third handle in shape <circularArrow> has in the preset
    // the tag <ahPolar gdRefR="adj5" minR="0" maxR="25000"> .
    // The resulting value in the map here is "RefR|adj5|na|na"
    OUString sAdjReferences;
};

// The array initializer has been extracted from
// oox/source/drawingml/customshapes/presetShapeDefinitions.xml
// by using an XSLT file. That file is attached to tdf#126512.
constexpr HandleAdjRel aHandleAdjRelArray[]
    = { { u"accentBorderCallout1_0"_ustr, u"RefX|adj2|RefY|adj1"_ustr },
        { u"accentBorderCallout1_1"_ustr, u"RefX|adj4|RefY|adj3"_ustr },
        { u"accentBorderCallout2_0"_ustr, u"RefX|adj2|RefY|adj1"_ustr },
        { u"accentBorderCallout2_1"_ustr, u"RefX|adj4|RefY|adj3"_ustr },
        { u"accentBorderCallout2_2"_ustr, u"RefX|adj6|RefY|adj5"_ustr },
        { u"accentBorderCallout3_0"_ustr, u"RefX|adj2|RefY|adj1"_ustr },
        { u"accentBorderCallout3_1"_ustr, u"RefX|adj4|RefY|adj3"_ustr },
        { u"accentBorderCallout3_2"_ustr, u"RefX|adj6|RefY|adj5"_ustr },
        { u"accentBorderCallout3_3"_ustr, u"RefX|adj8|RefY|adj7"_ustr },
        { u"accentCallout1_0"_ustr, u"RefX|adj2|RefY|adj1"_ustr },
        { u"accentCallout1_1"_ustr, u"RefX|adj4|RefY|adj3"_ustr },
        { u"accentCallout2_0"_ustr, u"RefX|adj2|RefY|adj1"_ustr },
        { u"accentCallout2_1"_ustr, u"RefX|adj4|RefY|adj3"_ustr },
        { u"accentCallout2_2"_ustr, u"RefX|adj6|RefY|adj5"_ustr },
        { u"accentCallout3_0"_ustr, u"RefX|adj2|RefY|adj1"_ustr },
        { u"accentCallout3_1"_ustr, u"RefX|adj4|RefY|adj3"_ustr },
        { u"accentCallout3_2"_ustr, u"RefX|adj6|RefY|adj5"_ustr },
        { u"accentCallout3_3"_ustr, u"RefX|adj8|RefY|adj7"_ustr },
        { u"arc_0"_ustr, u"na|na|RefAngle|adj1"_ustr },
        { u"arc_1"_ustr, u"na|na|RefAngle|adj2"_ustr },
        { u"bentArrow_0"_ustr, u"RefX|adj1|na|na"_ustr },
        { u"bentArrow_1"_ustr, u"na|na|RefY|adj2"_ustr },
        { u"bentArrow_2"_ustr, u"RefX|adj3|na|na"_ustr },
        { u"bentArrow_3"_ustr, u"RefX|adj4|na|na"_ustr },
        { u"bentConnector3_0"_ustr, u"RefX|adj1|na|na"_ustr },
        { u"bentConnector4_0"_ustr, u"RefX|adj1|na|na"_ustr },
        { u"bentConnector4_1"_ustr, u"na|na|RefY|adj2"_ustr },
        { u"bentConnector5_0"_ustr, u"RefX|adj1|na|na"_ustr },
        { u"bentConnector5_1"_ustr, u"na|na|RefY|adj2"_ustr },
        { u"bentConnector5_2"_ustr, u"RefX|adj3|na|na"_ustr },
        { u"bentUpArrow_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"bentUpArrow_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"bentUpArrow_2"_ustr, u"na|na|RefY|adj3"_ustr },
        { u"bevel_0"_ustr, u"RefX|adj|na|na"_ustr },
        { u"blockArc_0"_ustr, u"na|na|RefAngle|adj1"_ustr },
        { u"blockArc_1"_ustr, u"RefR|adj3|RefAngle|adj2"_ustr },
        { u"borderCallout1_0"_ustr, u"RefX|adj2|RefY|adj1"_ustr },
        { u"borderCallout1_1"_ustr, u"RefX|adj4|RefY|adj3"_ustr },
        { u"borderCallout2_0"_ustr, u"RefX|adj2|RefY|adj1"_ustr },
        { u"borderCallout2_1"_ustr, u"RefX|adj4|RefY|adj3"_ustr },
        { u"borderCallout2_2"_ustr, u"RefX|adj6|RefY|adj5"_ustr },
        { u"borderCallout3_0"_ustr, u"RefX|adj2|RefY|adj1"_ustr },
        { u"borderCallout3_1"_ustr, u"RefX|adj4|RefY|adj3"_ustr },
        { u"borderCallout3_2"_ustr, u"RefX|adj6|RefY|adj5"_ustr },
        { u"borderCallout3_3"_ustr, u"RefX|adj8|RefY|adj7"_ustr },
        { u"bracePair_0"_ustr, u"na|na|RefY|adj"_ustr },
        { u"bracketPair_0"_ustr, u"na|na|RefY|adj"_ustr },
        { u"callout1_0"_ustr, u"RefX|adj2|RefY|adj1"_ustr },
        { u"callout1_1"_ustr, u"RefX|adj4|RefY|adj3"_ustr },
        { u"callout2_0"_ustr, u"RefX|adj2|RefY|adj1"_ustr },
        { u"callout2_1"_ustr, u"RefX|adj4|RefY|adj3"_ustr },
        { u"callout2_2"_ustr, u"RefX|adj6|RefY|adj5"_ustr },
        { u"callout3_0"_ustr, u"RefX|adj2|RefY|adj1"_ustr },
        { u"callout3_1"_ustr, u"RefX|adj4|RefY|adj3"_ustr },
        { u"callout3_2"_ustr, u"RefX|adj6|RefY|adj5"_ustr },
        { u"callout3_3"_ustr, u"RefX|adj8|RefY|adj7"_ustr },
        { u"can_0"_ustr, u"na|na|RefY|adj"_ustr },
        { u"chevron_0"_ustr, u"RefX|adj|na|na"_ustr },
        { u"chord_0"_ustr, u"na|na|RefAngle|adj1"_ustr },
        { u"chord_1"_ustr, u"na|na|RefAngle|adj2"_ustr },
        { u"circularArrow_0"_ustr, u"na|na|RefAngle|adj2"_ustr },
        { u"circularArrow_1"_ustr, u"na|na|RefAngle|adj4"_ustr },
        { u"circularArrow_2"_ustr, u"RefR|adj1|RefAngle|adj3"_ustr },
        { u"circularArrow_3"_ustr, u"RefR|adj5|na|na"_ustr },
        { u"cloudCallout_0"_ustr, u"RefX|adj1|RefY|adj2"_ustr },
        { u"corner_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"corner_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"cube_0"_ustr, u"na|na|RefY|adj"_ustr },
        { u"curvedConnector3_0"_ustr, u"RefX|adj1|na|na"_ustr },
        { u"curvedConnector4_0"_ustr, u"RefX|adj1|na|na"_ustr },
        { u"curvedConnector4_1"_ustr, u"na|na|RefY|adj2"_ustr },
        { u"curvedConnector5_0"_ustr, u"RefX|adj1|na|na"_ustr },
        { u"curvedConnector5_1"_ustr, u"na|na|RefY|adj2"_ustr },
        { u"curvedConnector5_2"_ustr, u"RefX|adj3|na|na"_ustr },
        { u"curvedDownArrow_0"_ustr, u"RefX|adj1|na|na"_ustr },
        { u"curvedDownArrow_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"curvedDownArrow_2"_ustr, u"na|na|RefY|adj3"_ustr },
        { u"curvedLeftArrow_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"curvedLeftArrow_1"_ustr, u"na|na|RefY|adj2"_ustr },
        { u"curvedLeftArrow_2"_ustr, u"RefX|adj3|na|na"_ustr },
        { u"curvedRightArrow_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"curvedRightArrow_1"_ustr, u"na|na|RefY|adj2"_ustr },
        { u"curvedRightArrow_2"_ustr, u"RefX|adj3|na|na"_ustr },
        { u"curvedUpArrow_0"_ustr, u"RefX|adj1|na|na"_ustr },
        { u"curvedUpArrow_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"curvedUpArrow_2"_ustr, u"na|na|RefY|adj3"_ustr },
        { u"diagStripe_0"_ustr, u"na|na|RefY|adj"_ustr },
        { u"donut_0"_ustr, u"RefR|adj|na|na"_ustr },
        { u"doubleWave_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"doubleWave_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"downArrow_0"_ustr, u"RefX|adj1|na|na"_ustr },
        { u"downArrow_1"_ustr, u"na|na|RefY|adj2"_ustr },
        { u"downArrowCallout_0"_ustr, u"RefX|adj1|na|na"_ustr },
        { u"downArrowCallout_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"downArrowCallout_2"_ustr, u"na|na|RefY|adj3"_ustr },
        { u"downArrowCallout_3"_ustr, u"na|na|RefY|adj4"_ustr },
        { u"ellipseRibbon_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"ellipseRibbon_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"ellipseRibbon_2"_ustr, u"na|na|RefY|adj3"_ustr },
        { u"ellipseRibbon2_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"ellipseRibbon2_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"ellipseRibbon2_2"_ustr, u"na|na|RefY|adj3"_ustr },
        { u"foldedCorner_0"_ustr, u"RefX|adj|na|na"_ustr },
        { u"frame_0"_ustr, u"RefX|adj1|na|na"_ustr },
        { u"gear6_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"gear6_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"gear9_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"gear9_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"halfFrame_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"halfFrame_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"hexagon_0"_ustr, u"RefX|adj|na|na"_ustr },
        { u"homePlate_0"_ustr, u"RefX|adj|na|na"_ustr },
        { u"horizontalScroll_0"_ustr, u"RefX|adj|na|na"_ustr },
        { u"leftArrow_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"leftArrow_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"leftArrowCallout_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"leftArrowCallout_1"_ustr, u"na|na|RefY|adj2"_ustr },
        { u"leftArrowCallout_2"_ustr, u"RefX|adj3|na|na"_ustr },
        { u"leftArrowCallout_3"_ustr, u"RefX|adj4|na|na"_ustr },
        { u"leftBrace_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"leftBrace_1"_ustr, u"na|na|RefY|adj2"_ustr },
        { u"leftBracket_0"_ustr, u"na|na|RefY|adj"_ustr },
        { u"leftCircularArrow_0"_ustr, u"na|na|RefAngle|adj2"_ustr },
        { u"leftCircularArrow_1"_ustr, u"na|na|RefAngle|adj4"_ustr },
        { u"leftCircularArrow_2"_ustr, u"RefR|adj1|RefAngle|adj3"_ustr },
        { u"leftCircularArrow_3"_ustr, u"RefR|adj5|na|na"_ustr },
        { u"leftRightArrow_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"leftRightArrow_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"leftRightArrowCallout_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"leftRightArrowCallout_1"_ustr, u"na|na|RefY|adj2"_ustr },
        { u"leftRightArrowCallout_2"_ustr, u"RefX|adj3|na|na"_ustr },
        { u"leftRightArrowCallout_3"_ustr, u"RefX|adj4|na|na"_ustr },
        { u"leftRightCircularArrow_0"_ustr, u"na|na|RefAngle|adj2"_ustr },
        { u"leftRightCircularArrow_1"_ustr, u"na|na|RefAngle|adj4"_ustr },
        { u"leftRightCircularArrow_2"_ustr, u"RefR|adj1|RefAngle|adj3"_ustr },
        { u"leftRightCircularArrow_3"_ustr, u"RefR|adj5|na|na"_ustr },
        { u"leftRightRibbon_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"leftRightRibbon_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"leftRightRibbon_2"_ustr, u"na|na|RefY|adj3"_ustr },
        { u"leftRightUpArrow_0"_ustr, u"RefX|adj1|na|na"_ustr },
        { u"leftRightUpArrow_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"leftRightUpArrow_2"_ustr, u"na|na|RefY|adj3"_ustr },
        { u"leftUpArrow_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"leftUpArrow_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"leftUpArrow_2"_ustr, u"na|na|RefY|adj3"_ustr },
        { u"mathDivide_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"mathDivide_1"_ustr, u"na|na|RefY|adj2"_ustr },
        { u"mathDivide_2"_ustr, u"RefX|adj3|na|na"_ustr },
        { u"mathEqual_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"mathEqual_1"_ustr, u"na|na|RefY|adj2"_ustr },
        { u"mathMinus_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"mathMultiply_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"mathNotEqual_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"mathNotEqual_1"_ustr, u"na|na|RefAngle|adj2"_ustr },
        { u"mathNotEqual_2"_ustr, u"na|na|RefY|adj3"_ustr },
        { u"mathPlus_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"moon_0"_ustr, u"RefX|adj|na|na"_ustr },
        { u"nonIsoscelesTrapezoid_0"_ustr, u"RefX|adj1|na|na"_ustr },
        { u"nonIsoscelesTrapezoid_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"noSmoking_0"_ustr, u"RefR|adj|na|na"_ustr },
        { u"notchedRightArrow_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"notchedRightArrow_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"octagon_0"_ustr, u"RefX|adj|na|na"_ustr },
        { u"parallelogram_0"_ustr, u"RefX|adj|na|na"_ustr },
        { u"pie_0"_ustr, u"na|na|RefAngle|adj1"_ustr },
        { u"pie_1"_ustr, u"na|na|RefAngle|adj2"_ustr },
        { u"plaque_0"_ustr, u"RefX|adj|na|na"_ustr },
        { u"plus_0"_ustr, u"RefX|adj|na|na"_ustr },
        { u"quadArrow_0"_ustr, u"RefX|adj1|na|na"_ustr },
        { u"quadArrow_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"quadArrow_2"_ustr, u"na|na|RefY|adj3"_ustr },
        { u"quadArrowCallout_0"_ustr, u"RefX|adj1|na|na"_ustr },
        { u"quadArrowCallout_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"quadArrowCallout_2"_ustr, u"na|na|RefY|adj3"_ustr },
        { u"quadArrowCallout_3"_ustr, u"na|na|RefY|adj4"_ustr },
        { u"ribbon_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"ribbon_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"ribbon2_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"ribbon2_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"rightArrow_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"rightArrow_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"rightArrowCallout_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"rightArrowCallout_1"_ustr, u"na|na|RefY|adj2"_ustr },
        { u"rightArrowCallout_2"_ustr, u"RefX|adj3|na|na"_ustr },
        { u"rightArrowCallout_3"_ustr, u"RefX|adj4|na|na"_ustr },
        { u"rightBrace_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"rightBrace_1"_ustr, u"na|na|RefY|adj2"_ustr },
        { u"rightBracket_0"_ustr, u"na|na|RefY|adj"_ustr },
        { u"round1Rect_0"_ustr, u"RefX|adj|na|na"_ustr },
        { u"round2DiagRect_0"_ustr, u"RefX|adj1|na|na"_ustr },
        { u"round2DiagRect_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"round2SameRect_0"_ustr, u"RefX|adj1|na|na"_ustr },
        { u"round2SameRect_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"roundRect_0"_ustr, u"RefX|adj|na|na"_ustr },
        { u"smileyFace_0"_ustr, u"na|na|RefY|adj"_ustr },
        { u"snip1Rect_0"_ustr, u"RefX|adj|na|na"_ustr },
        { u"snip2DiagRect_0"_ustr, u"RefX|adj1|na|na"_ustr },
        { u"snip2DiagRect_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"snip2SameRect_0"_ustr, u"RefX|adj1|na|na"_ustr },
        { u"snip2SameRect_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"snipRoundRect_0"_ustr, u"RefX|adj1|na|na"_ustr },
        { u"snipRoundRect_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"star10_0"_ustr, u"na|na|RefY|adj"_ustr },
        { u"star12_0"_ustr, u"na|na|RefY|adj"_ustr },
        { u"star16_0"_ustr, u"na|na|RefY|adj"_ustr },
        { u"star24_0"_ustr, u"na|na|RefY|adj"_ustr },
        { u"star32_0"_ustr, u"na|na|RefY|adj"_ustr },
        { u"star4_0"_ustr, u"na|na|RefY|adj"_ustr },
        { u"star5_0"_ustr, u"na|na|RefY|adj"_ustr },
        { u"star6_0"_ustr, u"na|na|RefY|adj"_ustr },
        { u"star7_0"_ustr, u"na|na|RefY|adj"_ustr },
        { u"star8_0"_ustr, u"na|na|RefY|adj"_ustr },
        { u"stripedRightArrow_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"stripedRightArrow_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"sun_0"_ustr, u"RefX|adj|na|na"_ustr },
        { u"swooshArrow_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"swooshArrow_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"teardrop_0"_ustr, u"RefX|adj|na|na"_ustr },
        { u"trapezoid_0"_ustr, u"RefX|adj|na|na"_ustr },
        { u"triangle_0"_ustr, u"RefX|adj|na|na"_ustr },
        { u"upArrowCallout_0"_ustr, u"RefX|adj1|na|na"_ustr },
        { u"upArrowCallout_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"upArrowCallout_2"_ustr, u"na|na|RefY|adj3"_ustr },
        { u"upArrowCallout_3"_ustr, u"na|na|RefY|adj4"_ustr },
        { u"upDownArrow_0"_ustr, u"RefX|adj1|na|na"_ustr },
        { u"upDownArrow_1"_ustr, u"na|na|RefY|adj2"_ustr },
        { u"upArrow_0"_ustr, u"RefX|adj1|na|na"_ustr },
        { u"upArrow_1"_ustr, u"na|na|RefY|adj2"_ustr },
        { u"upDownArrowCallout_0"_ustr, u"RefX|adj1|na|na"_ustr },
        { u"upDownArrowCallout_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"upDownArrowCallout_2"_ustr, u"na|na|RefY|adj3"_ustr },
        { u"upDownArrowCallout_3"_ustr, u"na|na|RefY|adj4"_ustr },
        { u"uturnArrow_0"_ustr, u"RefX|adj1|na|na"_ustr },
        { u"uturnArrow_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"uturnArrow_2"_ustr, u"na|na|RefY|adj3"_ustr },
        { u"uturnArrow_3"_ustr, u"RefX|adj4|na|na"_ustr },
        { u"uturnArrow_4"_ustr, u"na|na|RefY|adj5"_ustr },
        { u"verticalScroll_0"_ustr, u"na|na|RefY|adj"_ustr },
        { u"wave_0"_ustr, u"na|na|RefY|adj1"_ustr },
        { u"wave_1"_ustr, u"RefX|adj2|na|na"_ustr },
        { u"wedgeEllipseCallout_0"_ustr, u"RefX|adj1|RefY|adj2"_ustr },
        { u"wedgeRectCallout_0"_ustr, u"RefX|adj1|RefY|adj2"_ustr },
        { u"wedgeRoundRectCallout_0"_ustr, u"RefX|adj1|RefY|adj2"_ustr } };
}

static sal_Int32 lcl_getAdjIndexFromToken(const sal_Int32 nTokenPos, std::u16string_view rMapValue)
{
    std::u16string_view sAdjRef = o3tl::getToken(rMapValue, nTokenPos, '|');
    std::u16string_view sNumber; // number part from "adj1", "adj2" etc.
    if (o3tl::starts_with(sAdjRef, u"adj", &sNumber))
    {
        if (sNumber.empty() || sNumber == u"1")
            return 0;
        else
            return o3tl::toInt32(sNumber) - 1;
    }
    else
        return -1;
}

void PresetOOXHandleAdj::GetOOXHandleAdjRelation(
    std::u16string_view sFullOOXShapeName, const sal_Int32 nHandleIndex, OUString& rFirstRefType,
    sal_Int32& rFirstAdjValueIndex, OUString& rSecondRefType, sal_Int32& rSecondAdjValueIndex)
{
    static const HandleAdjRelHashMap s_HashMap = []() {
        HandleAdjRelHashMap aH;
        aH.reserve(std::size(aHandleAdjRelArray));
        for (const auto& item : aHandleAdjRelArray)
            aH.emplace(item.sShape_Handle, item.sAdjReferences);
        return aH;
    }();

    std::u16string_view sKey;
    OUString sValue;
    rFirstRefType = "na";
    rFirstAdjValueIndex = -1;
    rSecondRefType = "na";
    rSecondAdjValueIndex = -1;
    if (o3tl::starts_with(sFullOOXShapeName, u"ooxml-", &sKey))
    {
        HandleAdjRelHashMap::const_iterator aHashIter(
            s_HashMap.find(OUString::Concat(sKey) + "_" + OUString::number(nHandleIndex)));
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
