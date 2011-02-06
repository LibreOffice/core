/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include "EnhancedCustomShapeTypeNames.hxx"
#include <osl/mutex.hxx>
#include <boost/unordered_map.hpp>
#include <hash_map>

struct TCheck
{
    bool operator()( const char* s1, const char* s2 ) const
    {
        return strcmp( s1, s2 ) == 0;
    }
};
typedef boost::unordered_map< const char*, MSO_SPT, std::hash<const char*>, TCheck> TypeNameHashMap;
static TypeNameHashMap* pHashMap = NULL;
static ::osl::Mutex& getHashMapMutex()
{
    static osl::Mutex s_aHashMapProtection;
    return s_aHashMapProtection;
}

struct NameTypeTable
{
    const char* pS;
    MSO_SPT     pE;
};
static const NameTypeTable pNameTypeTableArray[] =
{
    { "non-primitive", mso_sptMin },
    { "rectangle", mso_sptRectangle },
    { "round-rectangle", mso_sptRoundRectangle },
    { "ellipse", mso_sptEllipse },
    { "diamond", mso_sptDiamond },
    { "isosceles-triangle", mso_sptIsocelesTriangle },
    { "right-triangle", mso_sptRightTriangle },
    { "parallelogram", mso_sptParallelogram },
    { "trapezoid", mso_sptTrapezoid },
    { "hexagon", mso_sptHexagon },
    { "octagon", mso_sptOctagon },
    { "cross", mso_sptPlus },
    { "star5", mso_sptStar },
    { "right-arrow", mso_sptArrow },
    { "mso-spt14", mso_sptThickArrow },
    { "pentagon-right", mso_sptHomePlate },
    { "cube", mso_sptCube },
    { "mso-spt17", mso_sptBalloon },
    { "mso-spt18", mso_sptSeal },
    { "mso-spt19", mso_sptArc },
    { "mso-spt20", mso_sptLine },
    { "mso-spt21", mso_sptPlaque },
    { "can", mso_sptCan },
    { "ring", mso_sptDonut },
    { "mso-spt24", mso_sptTextSimple },
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
    { "mso-spt46", mso_sptAccentCallout3 },
    { "line-callout-1", mso_sptBorderCallout1 },
    { "line-callout-2", mso_sptBorderCallout2 },
    { "mso-spt49", mso_sptBorderCallout3 },
    { "mso-spt50", mso_sptAccentBorderCallout1 },
    { "mso-spt51", mso_sptAccentBorderCallout2 },
    { "mso-spt52", mso_sptAccentBorderCallout3 },
    { "mso-spt53", mso_sptRibbon },
    { "mso-spt54", mso_sptRibbon2 },
    { "chevron", mso_sptChevron },
    { "pentagon", mso_sptPentagon },
    { "forbidden", mso_sptNoSmoking },
    { "star8", mso_sptSeal8 },
    { "mso-spt59", mso_sptSeal16 },
    { "mso-spt60", mso_sptSeal32 },
    { "rectangular-callout", mso_sptWedgeRectCallout },
    { "round-rectangular-callout", mso_sptWedgeRRectCallout },
    { "round-callout", mso_sptWedgeEllipseCallout },
    { "mso-spt64", mso_sptWave },
    { "paper", mso_sptFoldedCorner },
    { "left-arrow", mso_sptLeftArrow },
    { "down-arrow", mso_sptDownArrow },
    { "up-arrow", mso_sptUpArrow },
    { "left-right-arrow", mso_sptLeftRightArrow },
    { "up-down-arrow", mso_sptUpDownArrow },
    { "mso-spt71", mso_sptIrregularSeal1 },
    { "bang", mso_sptIrregularSeal2 },
    { "lightning", mso_sptLightningBolt },
    { "heart", mso_sptHeart },
    { "mso-spt75", mso_sptPictureFrame },
    { "quad-arrow", mso_sptQuadArrow },
    { "left-arrow-callout", mso_sptLeftArrowCallout },
    { "right-arrow-callout", mso_sptRightArrowCallout },
    { "up-arrow-callout", mso_sptUpArrowCallout },
    { "down-arrow-callout", mso_sptDownArrowCallout },
    { "left-right-arrow-callout", mso_sptLeftRightArrowCallout },
    { "up-down-arrow-callout", mso_sptUpDownArrowCallout },
    { "quad-arrow-callout", mso_sptQuadArrowCallout },
    { "quad-bevel", mso_sptBevel },
    { "left-bracket", mso_sptLeftBracket },
    { "right-bracket", mso_sptRightBracket },
    { "left-brace", mso_sptLeftBrace },
    { "right-brace", mso_sptRightBrace },
    { "mso-spt89", mso_sptLeftUpArrow },
    { "mso-spt90", mso_sptBentUpArrow },
    { "mso-spt91", mso_sptBentArrow },
    { "star24", mso_sptSeal24 },
    { "striped-right-arrow", mso_sptStripedRightArrow },
    { "notched-right-arrow", mso_sptNotchedRightArrow },
    { "block-arc", mso_sptBlockArc },
    { "smiley", mso_sptSmileyFace },
    { "vertical-scroll", mso_sptVerticalScroll },
    { "horizontal-scroll", mso_sptHorizontalScroll },
    { "circular-arrow", mso_sptCircularArrow },
    { "mso-spt100", mso_sptNotchedCircularArrow },
    { "mso-spt101", mso_sptUturnArrow },
    { "mso-spt102", mso_sptCurvedRightArrow },
    { "mso-spt103", mso_sptCurvedLeftArrow },
    { "mso-spt104", mso_sptCurvedUpArrow },
    { "mso-spt105", mso_sptCurvedDownArrow },
    { "cloud-callout", mso_sptCloudCallout },
    { "mso-spt107", mso_sptEllipseRibbon },
    { "mso-spt108", mso_sptEllipseRibbon2 },
    { "flowchart-process", mso_sptFlowChartProcess },
    { "flowchart-decision", mso_sptFlowChartDecision },
    { "flowchart-data", mso_sptFlowChartInputOutput },
    { "flowchart-predefined-process", mso_sptFlowChartPredefinedProcess },
    { "flowchart-internal-storage", mso_sptFlowChartInternalStorage },
    { "flowchart-document", mso_sptFlowChartDocument },
    { "flowchart-multidocument", mso_sptFlowChartMultidocument },
    { "flowchart-terminator", mso_sptFlowChartTerminator },
    { "flowchart-preparation", mso_sptFlowChartPreparation },
    { "flowchart-manual-input", mso_sptFlowChartManualInput },
    { "flowchart-manual-operation", mso_sptFlowChartManualOperation },
    { "flowchart-connector", mso_sptFlowChartConnector },
    { "flowchart-card", mso_sptFlowChartPunchedCard },
    { "flowchart-punched-tape", mso_sptFlowChartPunchedTape },
    { "flowchart-summing-junction", mso_sptFlowChartSummingJunction },
    { "flowchart-or", mso_sptFlowChartOr },
    { "flowchart-collate", mso_sptFlowChartCollate },
    { "flowchart-sort", mso_sptFlowChartSort },
    { "flowchart-extract", mso_sptFlowChartExtract },
    { "flowchart-merge", mso_sptFlowChartMerge },
    { "mso-spt129", mso_sptFlowChartOfflineStorage },
    { "flowchart-stored-data", mso_sptFlowChartOnlineStorage },
    { "flowchart-sequential-access", mso_sptFlowChartMagneticTape },
    { "flowchart-magnetic-disk", mso_sptFlowChartMagneticDisk },
    { "flowchart-direct-access-storage", mso_sptFlowChartMagneticDrum },
    { "flowchart-display", mso_sptFlowChartDisplay },
    { "flowchart-delay", mso_sptFlowChartDelay },
    { "fontwork-plain-text", mso_sptTextPlainText },
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
    { "mso-spt175", mso_sptTextCanDown },
    { "flowchart-alternate-process", mso_sptFlowChartAlternateProcess },
    { "flowchart-off-page-connector", mso_sptFlowChartOffpageConnector },
    { "mso-spt178", mso_sptCallout90 },
    { "mso-spt179", mso_sptAccentCallout90 },
    { "mso-spt180", mso_sptBorderCallout90 },
    { "line-callout-3", mso_sptAccentBorderCallout90 },
    { "mso-spt182", mso_sptLeftRightUpArrow },
    { "sun", mso_sptSun },
    { "moon", mso_sptMoon },
    { "bracket-pair", mso_sptBracketPair },
    { "brace-pair", mso_sptBracePair },
    { "star4", mso_sptSeal4 },
    { "mso-spt188", mso_sptDoubleWave },
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
    { "mso-spt202", mso_sptTextBox }
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

MSO_SPT EnhancedCustomShapeTypeNames::Get( const rtl::OUString& rShapeType )
{
    if ( !pHashMap )
    {   // init hash map
        ::osl::MutexGuard aGuard( getHashMapMutex() );
        if ( !pHashMap )
        {
            TypeNameHashMap* pH = new TypeNameHashMap;
            const NameTypeTable* pPtr = pNameTypeTableArray;
            const NameTypeTable* pEnd = pPtr + ( sizeof( pNameTypeTableArray ) / sizeof( NameTypeTable ) );
            for ( ; pPtr < pEnd; pPtr++ )
                (*pH)[ pPtr->pS ] = pPtr->pE;
            pHashMap = pH;
        }
    }
    MSO_SPT eRetValue = mso_sptNil;
    int i, nLen = rShapeType.getLength();
    char* pBuf = new char[ nLen + 1 ];
    for ( i = 0; i < nLen; i++ )
        pBuf[ i ] = (char)rShapeType[ i ];
    pBuf[ i ] = 0;
    TypeNameHashMap::iterator aHashIter( pHashMap->find( pBuf ) );
    delete[] pBuf;
    if ( aHashIter != pHashMap->end() )
        eRetValue = (*aHashIter).second;
    return eRetValue;
}

rtl::OUString EnhancedCustomShapeTypeNames::Get( const MSO_SPT eShapeType )
{
    return eShapeType <= mso_sptTextBox
        ? rtl::OUString::createFromAscii( pNameTypeTableArray[ eShapeType ].pS )
        : rtl::OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
