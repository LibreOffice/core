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

#include <svx/EnhancedCustomShapeTypeNames.hxx>
#include <unordered_map>

namespace {

struct NameTypeTable
{
    OUString    pS;
    MSO_SPT     pE;
};

}

constexpr NameTypeTable pNameTypeTableArray[] =
{
    { u"non-primitive"_ustr, mso_sptMin },
    { u"rectangle"_ustr, mso_sptRectangle },
    { u"round-rectangle"_ustr, mso_sptRoundRectangle },
    { u"ellipse"_ustr, mso_sptEllipse },
    { u"diamond"_ustr, mso_sptDiamond },
    { u"isosceles-triangle"_ustr, mso_sptIsocelesTriangle },
    { u"right-triangle"_ustr, mso_sptRightTriangle },
    { u"parallelogram"_ustr, mso_sptParallelogram },
    { u"trapezoid"_ustr, mso_sptTrapezoid },
    { u"hexagon"_ustr, mso_sptHexagon },
    { u"octagon"_ustr, mso_sptOctagon },
    { u"cross"_ustr, mso_sptPlus },
    { u"star5"_ustr, mso_sptStar },
    { u"right-arrow"_ustr, mso_sptArrow },
    { u"mso-spt14"_ustr, mso_sptThickArrow },
    { u"pentagon-right"_ustr, mso_sptHomePlate },
    { u"cube"_ustr, mso_sptCube },
    { u"mso-spt17"_ustr, mso_sptBalloon },
    { u"mso-spt18"_ustr, mso_sptSeal },
    { u"mso-spt19"_ustr, mso_sptArc },
    { u"mso-spt20"_ustr, mso_sptLine },
    { u"mso-spt21"_ustr, mso_sptPlaque },
    { u"can"_ustr, mso_sptCan },
    { u"ring"_ustr, mso_sptDonut },
    { u"mso-spt24"_ustr, mso_sptTextSimple },
    { u"mso-spt25"_ustr, mso_sptTextOctagon },
    { u"mso-spt26"_ustr, mso_sptTextHexagon },
    { u"mso-spt27"_ustr, mso_sptTextCurve },
    { u"mso-spt28"_ustr, mso_sptTextWave },
    { u"mso-spt29"_ustr, mso_sptTextRing },
    { u"mso-spt30"_ustr, mso_sptTextOnCurve },
    { u"mso-spt31"_ustr, mso_sptTextOnRing },
    { u"mso-spt32"_ustr, mso_sptStraightConnector1 },
    { u"mso-spt33"_ustr, mso_sptBentConnector2 },
    { u"mso-spt34"_ustr, mso_sptBentConnector3 },
    { u"mso-spt35"_ustr, mso_sptBentConnector4 },
    { u"mso-spt36"_ustr, mso_sptBentConnector5 },
    { u"mso-spt37"_ustr, mso_sptCurvedConnector2 },
    { u"mso-spt38"_ustr, mso_sptCurvedConnector3 },
    { u"mso-spt39"_ustr, mso_sptCurvedConnector4 },
    { u"mso-spt40"_ustr, mso_sptCurvedConnector5 },
    { u"mso-spt41"_ustr, mso_sptCallout1 },
    { u"mso-spt42"_ustr, mso_sptCallout2 },
    { u"mso-spt43"_ustr, mso_sptCallout3 },
    { u"mso-spt44"_ustr, mso_sptAccentCallout1 },
    { u"mso-spt45"_ustr, mso_sptAccentCallout2 },
    { u"mso-spt46"_ustr, mso_sptAccentCallout3 },
    { u"line-callout-1"_ustr, mso_sptBorderCallout1 },
    { u"line-callout-2"_ustr, mso_sptBorderCallout2 },
    { u"mso-spt49"_ustr, mso_sptBorderCallout3 },
    { u"mso-spt50"_ustr, mso_sptAccentBorderCallout1 },
    { u"mso-spt51"_ustr, mso_sptAccentBorderCallout2 },
    { u"mso-spt52"_ustr, mso_sptAccentBorderCallout3 },
    { u"mso-spt53"_ustr, mso_sptRibbon },
    { u"mso-spt54"_ustr, mso_sptRibbon2 },
    { u"chevron"_ustr, mso_sptChevron },
    { u"pentagon"_ustr, mso_sptPentagon },
    { u"forbidden"_ustr, mso_sptNoSmoking },
    { u"star8"_ustr, mso_sptSeal8 },
    { u"mso-spt59"_ustr, mso_sptSeal16 },
    { u"mso-spt60"_ustr, mso_sptSeal32 },
    { u"rectangular-callout"_ustr, mso_sptWedgeRectCallout },
    { u"round-rectangular-callout"_ustr, mso_sptWedgeRRectCallout },
    { u"round-callout"_ustr, mso_sptWedgeEllipseCallout },
    { u"mso-spt64"_ustr, mso_sptWave },
    { u"paper"_ustr, mso_sptFoldedCorner },
    { u"left-arrow"_ustr, mso_sptLeftArrow },
    { u"down-arrow"_ustr, mso_sptDownArrow },
    { u"up-arrow"_ustr, mso_sptUpArrow },
    { u"left-right-arrow"_ustr, mso_sptLeftRightArrow },
    { u"up-down-arrow"_ustr, mso_sptUpDownArrow },
    { u"mso-spt71"_ustr, mso_sptIrregularSeal1 },
    { u"bang"_ustr, mso_sptIrregularSeal2 },
    { u"lightning"_ustr, mso_sptLightningBolt },
    { u"heart"_ustr, mso_sptHeart },
    { u"mso-spt75"_ustr, mso_sptPictureFrame },
    { u"quad-arrow"_ustr, mso_sptQuadArrow },
    { u"left-arrow-callout"_ustr, mso_sptLeftArrowCallout },
    { u"right-arrow-callout"_ustr, mso_sptRightArrowCallout },
    { u"up-arrow-callout"_ustr, mso_sptUpArrowCallout },
    { u"down-arrow-callout"_ustr, mso_sptDownArrowCallout },
    { u"left-right-arrow-callout"_ustr, mso_sptLeftRightArrowCallout },
    { u"up-down-arrow-callout"_ustr, mso_sptUpDownArrowCallout },
    { u"quad-arrow-callout"_ustr, mso_sptQuadArrowCallout },
    { u"quad-bevel"_ustr, mso_sptBevel },
    { u"left-bracket"_ustr, mso_sptLeftBracket },
    { u"right-bracket"_ustr, mso_sptRightBracket },
    { u"left-brace"_ustr, mso_sptLeftBrace },
    { u"right-brace"_ustr, mso_sptRightBrace },
    { u"mso-spt89"_ustr, mso_sptLeftUpArrow },
    { u"mso-spt90"_ustr, mso_sptBentUpArrow },
    { u"mso-spt91"_ustr, mso_sptBentArrow },
    { u"star24"_ustr, mso_sptSeal24 },
    { u"striped-right-arrow"_ustr, mso_sptStripedRightArrow },
    { u"notched-right-arrow"_ustr, mso_sptNotchedRightArrow },
    { u"block-arc"_ustr, mso_sptBlockArc },
    { u"smiley"_ustr, mso_sptSmileyFace },
    { u"vertical-scroll"_ustr, mso_sptVerticalScroll },
    { u"horizontal-scroll"_ustr, mso_sptHorizontalScroll },
    { u"circular-arrow"_ustr, mso_sptCircularArrow },
    { u"mso-spt100"_ustr, mso_sptNotchedCircularArrow },
    { u"mso-spt101"_ustr, mso_sptUturnArrow },
    { u"mso-spt102"_ustr, mso_sptCurvedRightArrow },
    { u"mso-spt103"_ustr, mso_sptCurvedLeftArrow },
    { u"mso-spt104"_ustr, mso_sptCurvedUpArrow },
    { u"mso-spt105"_ustr, mso_sptCurvedDownArrow },
    { u"cloud-callout"_ustr, mso_sptCloudCallout },
    { u"mso-spt107"_ustr, mso_sptEllipseRibbon },
    { u"mso-spt108"_ustr, mso_sptEllipseRibbon2 },
    { u"flowchart-process"_ustr, mso_sptFlowChartProcess },
    { u"flowchart-decision"_ustr, mso_sptFlowChartDecision },
    { u"flowchart-data"_ustr, mso_sptFlowChartInputOutput },
    { u"flowchart-predefined-process"_ustr, mso_sptFlowChartPredefinedProcess },
    { u"flowchart-internal-storage"_ustr, mso_sptFlowChartInternalStorage },
    { u"flowchart-document"_ustr, mso_sptFlowChartDocument },
    { u"flowchart-multidocument"_ustr, mso_sptFlowChartMultidocument },
    { u"flowchart-terminator"_ustr, mso_sptFlowChartTerminator },
    { u"flowchart-preparation"_ustr, mso_sptFlowChartPreparation },
    { u"flowchart-manual-input"_ustr, mso_sptFlowChartManualInput },
    { u"flowchart-manual-operation"_ustr, mso_sptFlowChartManualOperation },
    { u"flowchart-connector"_ustr, mso_sptFlowChartConnector },
    { u"flowchart-card"_ustr, mso_sptFlowChartPunchedCard },
    { u"flowchart-punched-tape"_ustr, mso_sptFlowChartPunchedTape },
    { u"flowchart-summing-junction"_ustr, mso_sptFlowChartSummingJunction },
    { u"flowchart-or"_ustr, mso_sptFlowChartOr },
    { u"flowchart-collate"_ustr, mso_sptFlowChartCollate },
    { u"flowchart-sort"_ustr, mso_sptFlowChartSort },
    { u"flowchart-extract"_ustr, mso_sptFlowChartExtract },
    { u"flowchart-merge"_ustr, mso_sptFlowChartMerge },
    { u"mso-spt129"_ustr, mso_sptFlowChartOfflineStorage },
    { u"flowchart-stored-data"_ustr, mso_sptFlowChartOnlineStorage },
    { u"flowchart-sequential-access"_ustr, mso_sptFlowChartMagneticTape },
    { u"flowchart-magnetic-disk"_ustr, mso_sptFlowChartMagneticDisk },
    { u"flowchart-direct-access-storage"_ustr, mso_sptFlowChartMagneticDrum },
    { u"flowchart-display"_ustr, mso_sptFlowChartDisplay },
    { u"flowchart-delay"_ustr, mso_sptFlowChartDelay },
    { u"fontwork-plain-text"_ustr, mso_sptTextPlainText },
    { u"fontwork-stop"_ustr, mso_sptTextStop },
    { u"fontwork-triangle-up"_ustr, mso_sptTextTriangle },
    { u"fontwork-triangle-down"_ustr, mso_sptTextTriangleInverted },
    { u"fontwork-chevron-up"_ustr, mso_sptTextChevron },
    { u"fontwork-chevron-down"_ustr, mso_sptTextChevronInverted },
    { u"mso-spt142"_ustr, mso_sptTextRingInside },
    { u"mso-spt143"_ustr, mso_sptTextRingOutside },
    { u"fontwork-arch-up-curve"_ustr, mso_sptTextArchUpCurve },
    { u"fontwork-arch-down-curve"_ustr, mso_sptTextArchDownCurve },
    { u"fontwork-circle-curve"_ustr, mso_sptTextCircleCurve },
    { u"fontwork-open-circle-curve"_ustr, mso_sptTextButtonCurve },
    { u"fontwork-arch-up-pour"_ustr, mso_sptTextArchUpPour },
    { u"fontwork-arch-down-pour"_ustr, mso_sptTextArchDownPour },
    { u"fontwork-circle-pour"_ustr, mso_sptTextCirclePour },
    { u"fontwork-open-circle-pour"_ustr, mso_sptTextButtonPour },
    { u"fontwork-curve-up"_ustr, mso_sptTextCurveUp },
    { u"fontwork-curve-down"_ustr, mso_sptTextCurveDown },
    { u"fontwork-fade-up-and-right"_ustr, mso_sptTextCascadeUp },
    { u"fontwork-fade-up-and-left"_ustr, mso_sptTextCascadeDown },
    { u"fontwork-wave"_ustr, mso_sptTextWave1 },
    { u"mso-spt157"_ustr, mso_sptTextWave2 },
    { u"mso-spt158"_ustr, mso_sptTextWave3 },
    { u"mso-spt159"_ustr, mso_sptTextWave4 },
    { u"fontwork-inflate"_ustr, mso_sptTextInflate },
    { u"mso-spt161"_ustr, mso_sptTextDeflate },
    { u"mso-spt162"_ustr, mso_sptTextInflateBottom },
    { u"mso-spt163"_ustr, mso_sptTextDeflateBottom },
    { u"mso-spt164"_ustr, mso_sptTextInflateTop },
    { u"mso-spt165"_ustr, mso_sptTextDeflateTop },
    { u"mso-spt166"_ustr, mso_sptTextDeflateInflate },
    { u"mso-spt167"_ustr, mso_sptTextDeflateInflateDeflate },
    { u"fontwork-fade-right"_ustr, mso_sptTextFadeRight },
    { u"fontwork-fade-left"_ustr, mso_sptTextFadeLeft },
    { u"fontwork-fade-up"_ustr, mso_sptTextFadeUp },
    { u"fontwork-fade-down"_ustr, mso_sptTextFadeDown },
    { u"fontwork-slant-up"_ustr, mso_sptTextSlantUp },
    { u"fontwork-slant-down"_ustr, mso_sptTextSlantDown },
    { u"mso-spt174"_ustr, mso_sptTextCanUp },
    { u"mso-spt175"_ustr, mso_sptTextCanDown },
    { u"flowchart-alternate-process"_ustr, mso_sptFlowChartAlternateProcess },
    { u"flowchart-off-page-connector"_ustr, mso_sptFlowChartOffpageConnector },
    { u"mso-spt178"_ustr, mso_sptCallout90 },
    { u"mso-spt179"_ustr, mso_sptAccentCallout90 },
    { u"mso-spt180"_ustr, mso_sptBorderCallout90 },
    { u"line-callout-3"_ustr, mso_sptAccentBorderCallout90 },
    { u"mso-spt182"_ustr, mso_sptLeftRightUpArrow },
    { u"sun"_ustr, mso_sptSun },
    { u"moon"_ustr, mso_sptMoon },
    { u"bracket-pair"_ustr, mso_sptBracketPair },
    { u"brace-pair"_ustr, mso_sptBracePair },
    { u"star4"_ustr, mso_sptSeal4 },
    { u"mso-spt188"_ustr, mso_sptDoubleWave },
    { u"mso-spt189"_ustr, mso_sptActionButtonBlank },
    { u"mso-spt190"_ustr, mso_sptActionButtonHome },
    { u"mso-spt191"_ustr, mso_sptActionButtonHelp },
    { u"mso-spt192"_ustr, mso_sptActionButtonInformation },
    { u"mso-spt193"_ustr, mso_sptActionButtonForwardNext },
    { u"mso-spt194"_ustr, mso_sptActionButtonBackPrevious },
    { u"mso-spt195"_ustr, mso_sptActionButtonEnd },
    { u"mso-spt196"_ustr, mso_sptActionButtonBeginning },
    { u"mso-spt197"_ustr, mso_sptActionButtonReturn },
    { u"mso-spt198"_ustr, mso_sptActionButtonDocument },
    { u"mso-spt199"_ustr, mso_sptActionButtonSound },
    { u"mso-spt200"_ustr, mso_sptActionButtonMovie },
    { u"mso-spt201"_ustr, mso_sptHostControl },
    { u"mso-spt202"_ustr, mso_sptTextBox },
    { u"teardrop"_ustr,   mso_sptTearDrop },
    { u"ooxml-rect"_ustr, mso_sptRectangle },
    { u"sinusoid"_ustr, mso_sptSinusoid }
};

    // gallery: quadrat
    // gallery: round-quadrat
    // gallery: circle
    // gallery: circle-pie
    // gallery: frame
    // gallery: flower
    // gallery: cloud
    // gallery: puzzle
    // gallery: octagon-bevel
    // gallery: diamond-bevel
    // gallery: up-right-arrow
    // gallery: up-right-down-arrow
    // gallery: corner-right-arrow
    // gallery: split-arrow
    // gallery: up-right-arrow-callout
    // gallery: split-round-arrow
    // gallery: s-sharped-arrow
    // Gallery: star6
    // Gallery: star12
    // Gallery: concave-star6
    // Gallery: signet
    // Gallery: doorplate
    // gallery: fontwork-arch-left-curve
    // gallery: fontwork-arch-right-curve
    // gallery: fontwork-arch-left-pour
    // gallery: fontwork-arch-right-pour


typedef std::unordered_map< OUString, MSO_SPT> TypeNameHashMap;

static const TypeNameHashMap& GetTypeNameHashMap()
{
    static TypeNameHashMap aMap = []()
    {
        TypeNameHashMap map;
        for (auto const & i : pNameTypeTableArray)
            map[i.pS] = i.pE;
        return map;
    }();
    return aMap;
}


MSO_SPT EnhancedCustomShapeTypeNames::Get( const OUString& rShapeType )
{
    const TypeNameHashMap & rTypeMap = GetTypeNameHashMap();
    MSO_SPT eRetValue = mso_sptNil;
    auto aHashIter = rTypeMap.find( rShapeType );
    if ( aHashIter != rTypeMap.end() )
        eRetValue = (*aHashIter).second;
    return eRetValue;
}

OUString EnhancedCustomShapeTypeNames::Get( const MSO_SPT eShapeType )
{
    return (eShapeType <= mso_sptTextBox && eShapeType >= mso_sptMin)
        ? pNameTypeTableArray[ eShapeType ].pS
        : OUString();
}

namespace {

struct ACCNameTypeTable
{
    const char* pS;
    const char* pE;
};

}

const ACCNameTypeTable pACCNameTypeTableArray[] =
{
    { "non-primitive", "Non Primitive Shape" },
    { "rectangle", "Rectangle" },
    { "round-rectangle", "Rounded Rectangle" },
    { "ellipse", "Ellipse" },
    { "diamond", "Diamond" },
    { "isosceles-triangle", "Triangle" },
    { "right-triangle", "Right Triangle" },
    { "parallelogram", "Parallelogram" },
    { "trapezoid", "Trapezoid" },
    { "hexagon", "Hexagon" },
    { "octagon", "Octagon" },
    { "cross", "Cross" },
    { "star5", "5-Point Star" },
    { "right-arrow", "Right Arrow" },
    //{ "mso-spt14", mso_sptThickArrow },
    { "pentagon-right", "Pentagon" },
    { "cube", "Cuboid" },
    { "mso-spt21", "Doorplate" },
    /*{ "mso-spt17", mso_sptBalloon },
    { "mso-spt18", mso_sptSeal },
    { "mso-spt19", mso_sptArc },
    { "mso-spt20", mso_sptLine },
    { "mso-spt21", mso_sptPlaque },
    { "can", mso_sptCan },*/
    { "ring", "Ring" },
    /*{ "mso-spt24", mso_sptTextSimple },
    { "mso-spt25", mso_sptTextOctagon },
    { "mso-spt26", mso_sptTextHexagon },
    { "mso-spt27", mso_sptTextCurve },
    { "mso-spt28", mso_sptTextWave },
    { "mso-spt29", mso_sptTextRing },
    { "mso-spt30", mso_sptTextOnCurve },
    { "mso-spt31", mso_sptTextOnRing },
    { "mso-spt32", mso_sptStraightConnector1 },
    { "mso-spt33", mso_sptBentConnector2 },
    { "mso-spt34", mso_sptBentConnector3 },
    { "mso-spt35", mso_sptBentConnector4 },
    { "mso-spt36", mso_sptBentConnector5 },
    { "mso-spt37", mso_sptCurvedConnector2 },
    { "mso-spt38", mso_sptCurvedConnector3 },
    { "mso-spt39", mso_sptCurvedConnector4 },
    { "mso-spt40", mso_sptCurvedConnector5 },
    { "mso-spt41", mso_sptCallout1 },
    { "mso-spt42", mso_sptCallout2 },
    { "mso-spt43", mso_sptCallout3 },
    { "mso-spt44", mso_sptAccentCallout1 },
    { "mso-spt45", mso_sptAccentCallout2 },
    { "mso-spt46", mso_sptAccentCallout3 },*/
    { "line-callout-1", "Line Callout 1" },
    { "line-callout-2", "Line Callout 2" },
    /*{ "mso-spt49", mso_sptBorderCallout3 },
    { "mso-spt50", mso_sptAccentBorderCallout1 },
    { "mso-spt51", mso_sptAccentBorderCallout2 },
    { "mso-spt52", mso_sptAccentBorderCallout3 },
    { "mso-spt53", mso_sptRibbon },
    { "mso-spt54", mso_sptRibbon2 },*/
    { "chevron", "Chevron" },
    { "pentagon", "Pentagon" },
    { "forbidden", "'No' Symbol" },
    { "star8", "8-Point Star" },
    /*{ "mso-spt59", mso_sptSeal16 },
    { "mso-spt60", mso_sptSeal32 },*/
    { "rectangular-callout", "Rectangular Callout" },
    { "round-rectangular-callout", "Rounded Rectangular Callout" },
    { "round-callout", "Round Callout" },
    //{ "mso-spt64", mso_sptWave },
    { "paper", "Folded Corner" },
    { "left-arrow", "Left Arrow" },
    { "down-arrow", "Down Arrow" },
    { "up-arrow", "Up Arrow" },
    { "left-right-arrow", "Left and Right Arrow" },
    { "up-down-arrow", "Up and Down Arrow" },
    //{ "mso-spt71", mso_sptIrregularSeal1 },
    { "bang", "Explosion" },
    { "lightning", "Lighting Bolt" },
    { "heart", "Heart" },
    //{ "mso-spt75", mso_sptPictureFrame },
    { "quad-arrow", "4-Way Arrow" },
    { "left-arrow-callout", "Left Arrow Callout" },
    { "right-arrow-callout", "Right Arrow Callout" },
    { "up-arrow-callout", "Up Arrow Callout" },
    { "down-arrow-callout", "Down Arrow Callout" },
    { "left-right-arrow-callout", "Left and Right Arrow Callout" },
    { "up-down-arrow-callout", "Up and Down Arrow Callout" },
    { "quad-arrow-callout", "4-Way Arrow Callout" },
    { "quad-bevel", "Square Bevel" },
    { "left-bracket", "Left Bracket" },
    { "right-bracket", "Right Bracket" },
    { "left-brace", "Left Brace" },
    { "right-brace", "Right Brace" },
    { "mso-spt89", "Up and Left Arrow" },
    //{ "mso-spt90", mso_sptBentUpArrow },
    //{ "mso-spt91", mso_sptBentArrow },
    { "star24", "24-Point Star" },
    { "striped-right-arrow", "Striped Right Arrow" },
    { "notched-right-arrow", "Notched Right Arrow" },
    { "block-arc", "Block Arc" },
    { "smiley", "Smile Face" },
    { "vertical-scroll", "Vertical Scroll" },
    { "horizontal-scroll", "Horizontal Scroll" },
    { "circular-arrow", "Circular Arrow" },
    { "mso-spt100", "Notched Circular Arrow" },
    /*
    { "mso-spt101", mso_sptUturnArrow },
    { "mso-spt102", mso_sptCurvedRightArrow },
    { "mso-spt103", mso_sptCurvedLeftArrow },
    { "mso-spt104", mso_sptCurvedUpArrow },
    { "mso-spt105", mso_sptCurvedDownArrow },*/
    { "cloud-callout", "Cloud Callout" },
    /*{ "mso-spt107", mso_sptEllipseRibbon },
    { "mso-spt108", mso_sptEllipseRibbon2 },*/
    { "flowchart-process", "Flowchart:Process" },
    { "flowchart-decision", "Flowchart:Decision" },
    { "flowchart-data", "Flowchart:Data" },
    { "flowchart-predefined-process", "Flowchart:Predefined Process" },
    { "flowchart-internal-storage", "Flowchart:Internal Storage" },
    { "flowchart-document", "Flowchart:Document" },
    { "flowchart-multidocument", "Flowchart:Multidocument" },
    { "flowchart-terminator", "Flowchart:Terminator" },
    { "flowchart-preparation", "Flowchart:Preparation" },
    { "flowchart-manual-input", "Flowchart:Manual Input" },
    { "flowchart-manual-operation", "Flowchart:Manual Operation" },
    { "flowchart-connector", "Flowchart:Connector" },
    { "flowchart-card", "Flowchart:Card" },
    { "flowchart-punched-tape", "Flowchart:Punched Tape" },
    { "flowchart-summing-junction", "Flowchart:Summing Junction" },
    { "flowchart-or", "Flowchart:Or" },
    { "flowchart-collate", "Flowchart:Collate" },
    { "flowchart-sort", "Flowchart:Sort" },
    { "flowchart-extract", "Flowchart:Extract" },
    { "flowchart-merge", "Flowchart:Merge" },
    //{ "mso-spt129", mso_sptFlowChartOfflineStorage },
    { "flowchart-stored-data", "Flowchart:Stored Data" },
    { "flowchart-sequential-access", "drawingbar.fc.25=Flowchart:Sequential Access" },
    { "flowchart-magnetic-disk", "Flowchart:Magnetic Disk" },
    { "flowchart-direct-access-storage", "Flowchart:Direct Access Storage" },
    { "flowchart-display", "Flowchart:Display" },
    { "flowchart-delay", "Flowchart:Delay" },
    /*{ "fontwork-plain-text", mso_sptTextPlainText },
    { "fontwork-stop", mso_sptTextStop },
    { "fontwork-triangle-up", mso_sptTextTriangle },
    { "fontwork-triangle-down", mso_sptTextTriangleInverted },
    { "fontwork-chevron-up", mso_sptTextChevron },
    { "fontwork-chevron-down", mso_sptTextChevronInverted },
    { "mso-spt142", mso_sptTextRingInside },
    { "mso-spt143", mso_sptTextRingOutside },
    { "fontwork-arch-up-curve", mso_sptTextArchUpCurve },
    { "fontwork-arch-down-curve", mso_sptTextArchDownCurve },
    { "fontwork-circle-curve", mso_sptTextCircleCurve },
    { "fontwork-open-circle-curve", mso_sptTextButtonCurve },
    { "fontwork-arch-up-pour", mso_sptTextArchUpPour },
    { "fontwork-arch-down-pour", mso_sptTextArchDownPour },
    { "fontwork-circle-pour", mso_sptTextCirclePour },
    { "fontwork-open-circle-pour", mso_sptTextButtonPour },
    { "fontwork-curve-up", mso_sptTextCurveUp },
    { "fontwork-curve-down", mso_sptTextCurveDown },
    { "fontwork-fade-up-and-right", mso_sptTextCascadeUp },
    { "fontwork-fade-up-and-left", mso_sptTextCascadeDown },
    { "fontwork-wave", mso_sptTextWave1 },
    { "mso-spt157", mso_sptTextWave2 },
    { "mso-spt158", mso_sptTextWave3 },
    { "mso-spt159", mso_sptTextWave4 },
    { "fontwork-inflate", mso_sptTextInflate },
    { "mso-spt161", mso_sptTextDeflate },
    { "mso-spt162", mso_sptTextInflateBottom },
    { "mso-spt163", mso_sptTextDeflateBottom },
    { "mso-spt164", mso_sptTextInflateTop },
    { "mso-spt165", mso_sptTextDeflateTop },
    { "mso-spt166", mso_sptTextDeflateInflate },
    { "mso-spt167", mso_sptTextDeflateInflateDeflate },
    { "fontwork-fade-right", mso_sptTextFadeRight },
    { "fontwork-fade-left", mso_sptTextFadeLeft },
    { "fontwork-fade-up", mso_sptTextFadeUp },
    { "fontwork-fade-down", mso_sptTextFadeDown },
    { "fontwork-slant-up", mso_sptTextSlantUp },
    { "fontwork-slant-down", mso_sptTextSlantDown },
    { "mso-spt174", mso_sptTextCanUp },
    { "mso-spt175", mso_sptTextCanDown },*/
    { "flowchart-alternate-process", "Flowchart:Alternate Process " },
    { "flowchart-off-page-connector", "Flowchart:Off-page Connector" },
    /*{ "mso-spt178", mso_sptCallout90 },
    { "mso-spt179", mso_sptAccentCallout90 },
    { "mso-spt180", mso_sptBorderCallout90 },*/
    { "line-callout-3", "Line Callout 3" },
    //{ "mso-spt182", mso_sptLeftRightUpArrow },
    { "sun", "Sun" },
    { "moon", "Moon" },
    { "bracket-pair", "Double Bracket" },
    { "brace-pair", "Double Brace" },
    { "star4", "4-Point Star" },
    /*{ "mso-spt188", mso_sptDoubleWave },
    { "mso-spt189", mso_sptActionButtonBlank },
    { "mso-spt190", mso_sptActionButtonHome },
    { "mso-spt191", mso_sptActionButtonHelp },
    { "mso-spt192", mso_sptActionButtonInformation },
    { "mso-spt193", mso_sptActionButtonForwardNext },
    { "mso-spt194", mso_sptActionButtonBackPrevious },
    { "mso-spt195", mso_sptActionButtonEnd },
    { "mso-spt196", mso_sptActionButtonBeginning },
    { "mso-spt197", mso_sptActionButtonReturn },
    { "mso-spt198", mso_sptActionButtonDocument },
    { "mso-spt199", mso_sptActionButtonSound },
    { "mso-spt200", mso_sptActionButtonMovie },
    { "mso-spt201", mso_sptHostControl },
    { "mso-spt202", mso_sptTextBox },*/
    { "frame", "Frame" },
    { "col-60da8460", "Octagon Bevel" },
    { "col-502ad400", "Diamond Bevel" },
    { "sinusoid", "Sinusoid" }
};

typedef std::unordered_map<OUString, OUString> TypeACCNameHashMap;

static const TypeACCNameHashMap& GetACCHashMap()
{
    static TypeACCNameHashMap aMap = []()
    {
        TypeACCNameHashMap map;
        for (auto const & i : pACCNameTypeTableArray)
            map[OUString::createFromAscii(i.pS)] = OUString::createFromAscii(i.pE);
        return map;
    }();
    return aMap;
}

const OUString & EnhancedCustomShapeTypeNames::GetAccName( const OUString& rShapeType )
{
    static const OUString EMPTY;
    const TypeACCNameHashMap& rACCMap = GetACCHashMap();
    auto aHashIter = rACCMap.find( rShapeType );
    if ( aHashIter != rACCMap.end() )
        return aHashIter->second;
    return EMPTY;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
