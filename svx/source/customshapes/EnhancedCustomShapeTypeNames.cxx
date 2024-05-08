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
    OUString pS;
    OUString pE;
};

}

constexpr ACCNameTypeTable pACCNameTypeTableArray[]
{
    { u"non-primitive"_ustr, u"Non Primitive Shape"_ustr },
    { u"rectangle"_ustr, u"Rectangle"_ustr },
    { u"round-rectangle"_ustr, u"Rounded Rectangle"_ustr },
    { u"ellipse"_ustr, u"Ellipse"_ustr },
    { u"diamond"_ustr, u"Diamond"_ustr },
    { u"isosceles-triangle"_ustr, u"Triangle"_ustr },
    { u"right-triangle"_ustr, u"Right Triangle"_ustr },
    { u"parallelogram"_ustr, u"Parallelogram"_ustr },
    { u"trapezoid"_ustr, u"Trapezoid"_ustr },
    { u"hexagon"_ustr, u"Hexagon"_ustr },
    { u"octagon"_ustr, u"Octagon"_ustr },
    { u"cross"_ustr, u"Cross"_ustr },
    { u"star5"_ustr, u"5-Point Star"_ustr },
    { u"right-arrow"_ustr, u"Right Arrow"_ustr },
    //{ u"mso-spt14", mso_sptThickArrow },
    { u"pentagon-right"_ustr, u"Pentagon"_ustr },
    { u"cube"_ustr, u"Cuboid"_ustr },
    { u"mso-spt21"_ustr, u"Doorplate"_ustr },
    /*{ u"mso-spt17", mso_sptBalloon },
    { u"mso-spt18", mso_sptSeal },
    { u"mso-spt19", mso_sptArc },
    { u"mso-spt20", mso_sptLine },
    { u"mso-spt21", mso_sptPlaque },
    { u"can", mso_sptCan },*/
    { u"ring"_ustr, u"Ring"_ustr },
    /*{ u"mso-spt24", mso_sptTextSimple },
    { u"mso-spt25", mso_sptTextOctagon },
    { u"mso-spt26", mso_sptTextHexagon },
    { u"mso-spt27", mso_sptTextCurve },
    { u"mso-spt28", mso_sptTextWave },
    { u"mso-spt29", mso_sptTextRing },
    { u"mso-spt30", mso_sptTextOnCurve },
    { u"mso-spt31", mso_sptTextOnRing },
    { u"mso-spt32", mso_sptStraightConnector1 },
    { u"mso-spt33", mso_sptBentConnector2 },
    { u"mso-spt34", mso_sptBentConnector3 },
    { u"mso-spt35", mso_sptBentConnector4 },
    { u"mso-spt36", mso_sptBentConnector5 },
    { u"mso-spt37", mso_sptCurvedConnector2 },
    { u"mso-spt38", mso_sptCurvedConnector3 },
    { u"mso-spt39", mso_sptCurvedConnector4 },
    { u"mso-spt40", mso_sptCurvedConnector5 },
    { u"mso-spt41", mso_sptCallout1 },
    { u"mso-spt42", mso_sptCallout2 },
    { u"mso-spt43", mso_sptCallout3 },
    { u"mso-spt44", mso_sptAccentCallout1 },
    { u"mso-spt45", mso_sptAccentCallout2 },
    { u"mso-spt46", mso_sptAccentCallout3 },*/
    { u"line-callout-1"_ustr, u"Line Callout 1"_ustr },
    { u"line-callout-2"_ustr, u"Line Callout 2"_ustr },
    /*{ u"mso-spt49", mso_sptBorderCallout3 },
    { u"mso-spt50", mso_sptAccentBorderCallout1 },
    { u"mso-spt51", mso_sptAccentBorderCallout2 },
    { u"mso-spt52", mso_sptAccentBorderCallout3 },
    { u"mso-spt53", mso_sptRibbon },
    { u"mso-spt54", mso_sptRibbon2 },*/
    { u"chevron"_ustr, u"Chevron"_ustr },
    { u"pentagon"_ustr, u"Pentagon"_ustr },
    { u"forbidden"_ustr, u"'No' Symbol"_ustr },
    { u"star8"_ustr, u"8-Point Star"_ustr },
    /*{ u"mso-spt59", mso_sptSeal16 },
    { u"mso-spt60", mso_sptSeal32 },*/
    { u"rectangular-callout"_ustr, u"Rectangular Callout"_ustr },
    { u"round-rectangular-callout"_ustr, u"Rounded Rectangular Callout"_ustr },
    { u"round-callout"_ustr, u"Round Callout"_ustr },
    //{ u"mso-spt64", mso_sptWave },
    { u"paper"_ustr, u"Folded Corner"_ustr },
    { u"left-arrow"_ustr, u"Left Arrow"_ustr },
    { u"down-arrow"_ustr, u"Down Arrow"_ustr },
    { u"up-arrow"_ustr, u"Up Arrow"_ustr },
    { u"left-right-arrow"_ustr, u"Left and Right Arrow"_ustr },
    { u"up-down-arrow"_ustr, u"Up and Down Arrow"_ustr },
    //{ u"mso-spt71", mso_sptIrregularSeal1 },
    { u"bang"_ustr, u"Explosion"_ustr },
    { u"lightning"_ustr, u"Lighting Bolt"_ustr },
    { u"heart"_ustr, u"Heart"_ustr },
    //{ u"mso-spt75", mso_sptPictureFrame },
    { u"quad-arrow"_ustr, u"4-Way Arrow"_ustr },
    { u"left-arrow-callout"_ustr, u"Left Arrow Callout"_ustr },
    { u"right-arrow-callout"_ustr, u"Right Arrow Callout"_ustr },
    { u"up-arrow-callout"_ustr, u"Up Arrow Callout"_ustr },
    { u"down-arrow-callout"_ustr, u"Down Arrow Callout"_ustr },
    { u"left-right-arrow-callout"_ustr, u"Left and Right Arrow Callout"_ustr },
    { u"up-down-arrow-callout"_ustr, u"Up and Down Arrow Callout"_ustr },
    { u"quad-arrow-callout"_ustr, u"4-Way Arrow Callout"_ustr },
    { u"quad-bevel"_ustr, u"Square Bevel"_ustr },
    { u"left-bracket"_ustr, u"Left Bracket"_ustr },
    { u"right-bracket"_ustr, u"Right Bracket"_ustr },
    { u"left-brace"_ustr, u"Left Brace"_ustr },
    { u"right-brace"_ustr, u"Right Brace"_ustr },
    { u"mso-spt89"_ustr, u"Up and Left Arrow"_ustr },
    //{ u"mso-spt90", mso_sptBentUpArrow },
    //{ u"mso-spt91", mso_sptBentArrow },
    { u"star24"_ustr, u"24-Point Star"_ustr },
    { u"striped-right-arrow"_ustr, u"Striped Right Arrow"_ustr },
    { u"notched-right-arrow"_ustr, u"Notched Right Arrow"_ustr },
    { u"block-arc"_ustr, u"Block Arc"_ustr },
    { u"smiley"_ustr, u"Smile Face"_ustr },
    { u"vertical-scroll"_ustr, u"Vertical Scroll"_ustr },
    { u"horizontal-scroll"_ustr, u"Horizontal Scroll"_ustr },
    { u"circular-arrow"_ustr, u"Circular Arrow"_ustr },
    { u"mso-spt100"_ustr, u"Notched Circular Arrow"_ustr },
    /*
    { u"mso-spt101", mso_sptUturnArrow },
    { u"mso-spt102", mso_sptCurvedRightArrow },
    { u"mso-spt103", mso_sptCurvedLeftArrow },
    { u"mso-spt104", mso_sptCurvedUpArrow },
    { u"mso-spt105", mso_sptCurvedDownArrow },*/
    { u"cloud-callout"_ustr, u"Cloud Callout"_ustr },
    /*{ u"mso-spt107", mso_sptEllipseRibbon },
    { u"mso-spt108", mso_sptEllipseRibbon2 },*/
    { u"flowchart-process"_ustr, u"Flowchart:Process"_ustr },
    { u"flowchart-decision"_ustr, u"Flowchart:Decision"_ustr },
    { u"flowchart-data"_ustr, u"Flowchart:Data"_ustr },
    { u"flowchart-predefined-process"_ustr, u"Flowchart:Predefined Process"_ustr },
    { u"flowchart-internal-storage"_ustr, u"Flowchart:Internal Storage"_ustr },
    { u"flowchart-document"_ustr, u"Flowchart:Document"_ustr },
    { u"flowchart-multidocument"_ustr, u"Flowchart:Multidocument"_ustr },
    { u"flowchart-terminator"_ustr, u"Flowchart:Terminator"_ustr },
    { u"flowchart-preparation"_ustr, u"Flowchart:Preparation"_ustr },
    { u"flowchart-manual-input"_ustr, u"Flowchart:Manual Input"_ustr },
    { u"flowchart-manual-operation"_ustr, u"Flowchart:Manual Operation"_ustr },
    { u"flowchart-connector"_ustr, u"Flowchart:Connector"_ustr },
    { u"flowchart-card"_ustr, u"Flowchart:Card"_ustr },
    { u"flowchart-punched-tape"_ustr, u"Flowchart:Punched Tape"_ustr },
    { u"flowchart-summing-junction"_ustr, u"Flowchart:Summing Junction"_ustr },
    { u"flowchart-or"_ustr, u"Flowchart:Or"_ustr },
    { u"flowchart-collate"_ustr, u"Flowchart:Collate"_ustr },
    { u"flowchart-sort"_ustr, u"Flowchart:Sort"_ustr },
    { u"flowchart-extract"_ustr, u"Flowchart:Extract"_ustr },
    { u"flowchart-merge"_ustr, u"Flowchart:Merge"_ustr },
    //{ u"mso-spt129", mso_sptFlowChartOfflineStorage },
    { u"flowchart-stored-data"_ustr, u"Flowchart:Stored Data"_ustr },
    { u"flowchart-sequential-access"_ustr, u"drawingbar.fc.25=Flowchart:Sequential Access"_ustr },
    { u"flowchart-magnetic-disk"_ustr, u"Flowchart:Magnetic Disk"_ustr },
    { u"flowchart-direct-access-storage"_ustr, u"Flowchart:Direct Access Storage"_ustr },
    { u"flowchart-display"_ustr, u"Flowchart:Display"_ustr },
    { u"flowchart-delay"_ustr, u"Flowchart:Delay"_ustr },
    /*{ u"fontwork-plain-text", mso_sptTextPlainText },
    { u"fontwork-stop", mso_sptTextStop },
    { u"fontwork-triangle-up", mso_sptTextTriangle },
    { u"fontwork-triangle-down", mso_sptTextTriangleInverted },
    { u"fontwork-chevron-up", mso_sptTextChevron },
    { u"fontwork-chevron-down", mso_sptTextChevronInverted },
    { u"mso-spt142", mso_sptTextRingInside },
    { u"mso-spt143", mso_sptTextRingOutside },
    { u"fontwork-arch-up-curve", mso_sptTextArchUpCurve },
    { u"fontwork-arch-down-curve", mso_sptTextArchDownCurve },
    { u"fontwork-circle-curve", mso_sptTextCircleCurve },
    { u"fontwork-open-circle-curve", mso_sptTextButtonCurve },
    { u"fontwork-arch-up-pour", mso_sptTextArchUpPour },
    { u"fontwork-arch-down-pour", mso_sptTextArchDownPour },
    { u"fontwork-circle-pour", mso_sptTextCirclePour },
    { u"fontwork-open-circle-pour", mso_sptTextButtonPour },
    { u"fontwork-curve-up", mso_sptTextCurveUp },
    { u"fontwork-curve-down", mso_sptTextCurveDown },
    { u"fontwork-fade-up-and-right", mso_sptTextCascadeUp },
    { u"fontwork-fade-up-and-left", mso_sptTextCascadeDown },
    { u"fontwork-wave", mso_sptTextWave1 },
    { u"mso-spt157", mso_sptTextWave2 },
    { u"mso-spt158", mso_sptTextWave3 },
    { u"mso-spt159", mso_sptTextWave4 },
    { u"fontwork-inflate", mso_sptTextInflate },
    { u"mso-spt161", mso_sptTextDeflate },
    { u"mso-spt162", mso_sptTextInflateBottom },
    { u"mso-spt163", mso_sptTextDeflateBottom },
    { u"mso-spt164", mso_sptTextInflateTop },
    { u"mso-spt165", mso_sptTextDeflateTop },
    { u"mso-spt166", mso_sptTextDeflateInflate },
    { u"mso-spt167", mso_sptTextDeflateInflateDeflate },
    { u"fontwork-fade-right", mso_sptTextFadeRight },
    { u"fontwork-fade-left", mso_sptTextFadeLeft },
    { u"fontwork-fade-up", mso_sptTextFadeUp },
    { u"fontwork-fade-down", mso_sptTextFadeDown },
    { u"fontwork-slant-up", mso_sptTextSlantUp },
    { u"fontwork-slant-down", mso_sptTextSlantDown },
    { u"mso-spt174", mso_sptTextCanUp },
    { u"mso-spt175", mso_sptTextCanDown },*/
    { u"flowchart-alternate-process"_ustr, u"Flowchart:Alternate Process "_ustr },
    { u"flowchart-off-page-connector"_ustr, u"Flowchart:Off-page Connector"_ustr },
    /*{ u"mso-spt178", mso_sptCallout90 },
    { u"mso-spt179", mso_sptAccentCallout90 },
    { u"mso-spt180", mso_sptBorderCallout90 },*/
    { u"line-callout-3"_ustr, u"Line Callout 3"_ustr },
    //{ u"mso-spt182", mso_sptLeftRightUpArrow },
    { u"sun"_ustr, u"Sun"_ustr },
    { u"moon"_ustr, u"Moon"_ustr },
    { u"bracket-pair"_ustr, u"Double Bracket"_ustr },
    { u"brace-pair"_ustr, u"Double Brace"_ustr },
    { u"star4"_ustr, u"4-Point Star"_ustr },
    /*{ u"mso-spt188", mso_sptDoubleWave },
    { u"mso-spt189", mso_sptActionButtonBlank },
    { u"mso-spt190", mso_sptActionButtonHome },
    { u"mso-spt191", mso_sptActionButtonHelp },
    { u"mso-spt192", mso_sptActionButtonInformation },
    { u"mso-spt193", mso_sptActionButtonForwardNext },
    { u"mso-spt194", mso_sptActionButtonBackPrevious },
    { u"mso-spt195", mso_sptActionButtonEnd },
    { u"mso-spt196", mso_sptActionButtonBeginning },
    { u"mso-spt197", mso_sptActionButtonReturn },
    { u"mso-spt198", mso_sptActionButtonDocument },
    { u"mso-spt199", mso_sptActionButtonSound },
    { u"mso-spt200", mso_sptActionButtonMovie },
    { u"mso-spt201", mso_sptHostControl },
    { u"mso-spt202", mso_sptTextBox },*/
    { u"frame"_ustr, u"Frame"_ustr },
    { u"col-60da8460"_ustr, u"Octagon Bevel"_ustr },
    { u"col-502ad400"_ustr, u"Diamond Bevel"_ustr },
    { u"sinusoid"_ustr, u"Sinusoid"_ustr }
};

typedef std::unordered_map<OUString, OUString> TypeACCNameHashMap;

static const TypeACCNameHashMap& GetACCHashMap()
{
    static TypeACCNameHashMap aMap = []()
    {
        TypeACCNameHashMap map;
        for (auto const & i : pACCNameTypeTableArray)
            map[i.pS] = i.pE;
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
