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

#include "oox/core/xmlfilterbase.hxx"
#include "oox/export/shapes.hxx"
#include "oox/export/utils.hxx"
#include <oox/token/tokens.hxx>

#include <cstdio>
#include <com/sun/star/awt/CharSet.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/ConnectorType.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/text/XSimpleText.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/XMergeableCell.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <tools/stream.hxx>
#include <tools/string.hxx>
#include <vcl/cvtgrf.hxx>
#include <unotools/fontcvt.hxx>
#include <vcl/graph.hxx>
#include <vcl/outdev.hxx>
#include <svtools/grfmgr.hxx>
#include <rtl/strbuf.hxx>
#include <sfx2/app.hxx>
#include <svl/languageoptions.hxx>
#include <filter/msfilter/escherex.hxx>
#include <svx/svdoashp.hxx>
#include <editeng/svxenum.hxx>
#include <svx/unoapi.hxx>
#include <oox/export/chartexport.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::table;
using ::com::sun::star::beans::PropertyState;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertyState;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::drawing::FillStyle;
using ::com::sun::star::io::XOutputStream;
using ::com::sun::star::text::XSimpleText;
using ::com::sun::star::text::XText;
using ::com::sun::star::text::XTextContent;
using ::com::sun::star::text::XTextField;
using ::com::sun::star::text::XTextRange;
using ::oox::core::XmlFilterBase;
using ::com::sun::star::chart2::XChartDocument;
using ::com::sun::star::frame::XModel;
using ::oox::core::XmlFilterBase;
using ::rtl::OString;
using ::rtl::OStringBuffer;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::sax_fastparser::FSHelperPtr;

#define IDS(x) (OString(#x " ") + OString::valueOf( mnShapeIdMax++ )).getStr()


struct CustomShapeTypeTranslationTable
{
    const char* sOOo;
    const char* sMSO;
};

static const CustomShapeTypeTranslationTable pCustomShapeTypeTranslationTable[] =
{
    // { "non-primitive", mso_sptMin },
    { "rectangle", "rect" },
    { "round-rectangle", "roundRect" },
    { "ellipse", "ellipse" },
    { "diamond", "diamond" },
    { "isosceles-triangle", "triangle" },
    { "right-triangle", "rtTriangle" },
    { "parallelogram", "parallelogram" },
    { "trapezoid", "trapezoid" },
    { "hexagon", "hexagon" },
    { "octagon", "octagon" },
    { "cross", "plus" },
    { "star5", "star5" },
    { "right-arrow", "rightArrow" },
    // { "mso-spt14", mso_sptThickArrow },
    { "pentagon-right", "homePlate" },
    { "cube", "cube" },
    // { "mso-spt17", mso_sptBalloon },
    // { "mso-spt18", mso_sptSeal },
    { "mso-spt19", "arc" },
    { "mso-spt20", "line" },
    { "mso-spt21", "plaque" },
    { "can", "can" },
    { "ring", "donut" },
    { "mso-spt24", "textSimple" },
    { "mso-spt25", "textOctagon" },
    { "mso-spt26", "textHexagon" },
    { "mso-spt27", "textCurve" },
    { "mso-spt28", "textWave" },
    { "mso-spt29", "textRing" },
    { "mso-spt30", "textOnCurve" },
    { "mso-spt31", "textOnRing" },
    { "mso-spt32", "straightConnector1" },
    { "mso-spt33", "bentConnector2" },
    { "mso-spt34", "bentConnector3" },
    { "mso-spt35", "bentConnector4" },
    { "mso-spt36", "bentConnector5" },
    { "mso-spt37", "curvedConnector2" },
    { "mso-spt38", "curvedConnector3" },
    { "mso-spt39", "curvedConnector4" },
    { "mso-spt40", "curvedConnector5" },
    { "mso-spt41", "callout1" },
    { "mso-spt42", "callout2" },
    { "mso-spt43", "callout3" },
    { "mso-spt44", "accentCallout1" },
    { "mso-spt45", "accentCallout2" },
    { "mso-spt46", "accentCallout3" },
    { "line-callout-1", "borderCallout1" },
    { "line-callout-2", "borderCallout2" },
    { "line-callout-3", "borderCallout3" },
    { "mso-spt49", "accentBorderCallout90" },
    { "mso-spt50", "accentBorderCallout1" },
    { "mso-spt51", "accentBorderCallout2" },
    { "mso-spt52", "accentBorderCallout3" },
    { "mso-spt53", "ribbon" },
    { "mso-spt54", "ribbon2" },
    { "chevron", "chevron" },
    { "pentagon", "pentagon" },
    { "forbidden", "noSmoking" },
    { "star8", "seal8" },
    { "mso-spt59", "seal16" },
    { "mso-spt60", "seal32" },
    { "rectangular-callout", "wedgeRectCallout" },
    { "round-rectangular-callout", "wedgeRoundRectCallout" },
    { "round-callout", "wedgeEllipseCallout" },
    { "mso-spt64", "wave" },
    { "paper", "foldedCorner" },
    { "left-arrow", "leftArrow" },
    { "down-arrow", "downArrow" },
    { "up-arrow", "upArrow" },
    { "left-right-arrow", "leftRightArrow" },
    { "up-down-arrow", "upDownArrow" },
    { "mso-spt71", "irregularSeal1" },
    { "bang", "irregularSeal2" },
    { "lightning", "lightningBolt" },
    { "heart", "heart" },
    { "mso-spt75", "pictureFrame" },
    { "quad-arrow", "quadArrow" },
    { "left-arrow-callout", "leftArrowCallout" },
    { "right-arrow-callout", "rightArrowCallout" },
    { "up-arrow-callout", "upArrowCallout" },
    { "down-arrow-callout", "downArrowCallout" },
    { "left-right-arrow-callout", "leftRightArrowCallout" },
    { "up-down-arrow-callout", "upDownArrowCallout" },
    { "quad-arrow-callout", "quadArrowCallout" },
    { "quad-bevel", "bevel" },
    { "left-bracket", "leftBracket" },
    { "right-bracket", "rightBracket" },
    { "left-brace", "leftBrace" },
    { "right-brace", "rightBrace" },
    { "mso-spt89", "leftUpArrow" },
    { "mso-spt90", "bentUpArrow" },
    { "mso-spt91", "bentArrow" },
    { "star24", "seal24" },
    { "striped-right-arrow", "stripedRightArrow" },
    { "notched-right-arrow", "notchedRightArrow" },
    { "block-arc", "blockArc" },
    { "smiley", "smileyFace" },
    { "vertical-scroll", "verticalScroll" },
    { "horizontal-scroll", "horizontalScroll" },
    { "circular-arrow", "circularArrow" },
    { "mso-spt100", "pie" }, // looks like MSO_SPT is wrong here
    { "mso-spt101", "uturnArrow" },
    { "mso-spt102", "curvedRightArrow" },
    { "mso-spt103", "curvedLeftArrow" },
    { "mso-spt104", "curvedUpArrow" },
    { "mso-spt105", "curvedDownArrow" },
    { "cloud-callout", "cloudCallout" },
    { "mso-spt107", "ellipseRibbon" },
    { "mso-spt108", "ellipseRibbon2" },
    { "flowchart-process", "flowChartProcess" },
    { "flowchart-decision", "flowChartDecision" },
    { "flowchart-data", "flowChartInputOutput" },
    { "flowchart-predefined-process", "flowChartPredefinedProcess" },
    { "flowchart-internal-storage", "flowChartInternalStorage" },
    { "flowchart-document", "flowChartDocument" },
    { "flowchart-multidocument", "flowChartMultidocument" },
    { "flowchart-terminator", "flowChartTerminator" },
    { "flowchart-preparation", "flowChartPreparation" },
    { "flowchart-manual-input", "flowChartManualInput" },
    { "flowchart-manual-operation", "flowChartManualOperation" },
    { "flowchart-connector", "flowChartConnector" },
    { "flowchart-card", "flowChartPunchedCard" },
    { "flowchart-punched-tape", "flowChartPunchedTape" },
    { "flowchart-summing-junction", "flowChartSummingJunction" },
    { "flowchart-or", "flowChartOr" },
    { "flowchart-collate", "flowChartCollate" },
    { "flowchart-sort", "flowChartSort" },
    { "flowchart-extract", "flowChartExtract" },
    { "flowchart-merge", "flowChartMerge" },
    { "mso-spt129", "flowChartOfflineStorage" },
    { "flowchart-stored-data", "flowChartOnlineStorage" },
    { "flowchart-sequential-access", "flowChartMagneticTape" },
    { "flowchart-magnetic-disk", "flowChartMagneticDisk" },
    { "flowchart-direct-access-storage", "flowChartMagneticDrum" },
    { "flowchart-display", "flowChartDisplay" },
    { "flowchart-delay", "flowChartDelay" },
    { "fontwork-plain-text", "textPlainText" },
    { "fontwork-stop", "textStop" },
    { "fontwork-triangle-up", "textTriangle" },
    { "fontwork-triangle-down", "textTriangleInverted" },
    { "fontwork-chevron-up", "textChevron" },
    { "fontwork-chevron-down", "textChevronInverted" },
    { "mso-spt142", "textRingInside" },
    { "mso-spt143", "textRingOutside" },
    { "fontwork-arch-up-curve", "textArchUpCurve" },
    { "fontwork-arch-down-curve", "textArchDownCurve" },
    { "fontwork-circle-curve", "textCircleCurve" },
    { "fontwork-open-circle-curve", "textButtonCurve" },
    { "fontwork-arch-up-pour", "textArchUpPour" },
    { "fontwork-arch-down-pour", "textArchDownPour" },
    { "fontwork-circle-pour", "textCirclePour" },
    { "fontwork-open-circle-pour", "textButtonPour" },
    { "fontwork-curve-up", "textCurveUp" },
    { "fontwork-curve-down", "textCurveDown" },
    { "fontwork-fade-up-and-right", "textCascadeUp" },
    { "fontwork-fade-up-and-left", "textCascadeDown" },
    { "fontwork-wave", "textWave1" },
    { "mso-spt157", "textWave2" },
    { "mso-spt158", "textWave3" },
    { "mso-spt159", "textWave4" },
    { "fontwork-inflate", "textInflate" },
    { "mso-spt161", "textDeflate" },
    { "mso-spt162", "textInflateBottom" },
    { "mso-spt163", "textDeflateBottom" },
    { "mso-spt164", "textInflateTop" },
    { "mso-spt165", "textDeflateTop" },
    { "mso-spt166", "textDeflateInflate" },
    { "mso-spt167", "textDeflateInflateDeflate" },
    { "fontwork-fade-right", "textFadeRight" },
    { "fontwork-fade-left", "textFadeLeft" },
    { "fontwork-fade-up", "textFadeUp" },
    { "fontwork-fade-down", "textFadeDown" },
    { "fontwork-slant-up", "textSlantUp" },
    { "fontwork-slant-down", "textSlantDown" },
    { "mso-spt174", "textCanUp" },
    { "mso-spt175", "textCanDown" },
    { "flowchart-alternate-process", "flowChartAlternateProcess" },
    { "flowchart-off-page-connector", "flowChartOffpageConnector" },
    { "mso-spt178", "callout90" },
    { "mso-spt179", "accentCallout90" },
    { "mso-spt180", "borderCallout90" },
    { "mso-spt182", "leftRightUpArrow" },
    { "sun", "sun" },
    { "moon", "moon" },
    { "bracket-pair", "bracketPair" },
    { "brace-pair", "bracePair" },
    { "star4", "seal4" },
    { "mso-spt188", "doubleWave" },
    { "mso-spt189", "actionButtonBlank" },
    { "mso-spt190", "actionButtonHome" },
    { "mso-spt191", "actionButtonHelp" },
    { "mso-spt192", "actionButtonInformation" },
    { "mso-spt193", "actionButtonForwardNext" },
    { "mso-spt194", "actionButtonBackPrevious" },
    { "mso-spt195", "actionButtonEnd" },
    { "mso-spt196", "actionButtonBeginning" },
    { "mso-spt197", "actionButtonReturn" },
    { "mso-spt198", "actionButtonDocument" },
    { "mso-spt199", "actionButtonSound" },
    { "mso-spt200", "actionButtonMovie" },
    { "mso-spt201", "hostControl" },
    { "mso-spt202", "rect" },
    { "ooxml-actionButtonSound", "actionButtonSound" },
    { "ooxml-borderCallout1", "borderCallout1" },
    { "ooxml-plaqueTabs", "plaqueTabs" },
    { "ooxml-curvedLeftArrow", "curvedLeftArrow" },
    { "ooxml-octagon", "octagon" },
    { "ooxml-leftRightRibbon", "leftRightRibbon" },
    { "ooxml-actionButtonInformation", "actionButtonInformation" },
    { "ooxml-bentConnector5", "bentConnector5" },
    { "ooxml-circularArrow", "circularArrow" },
    { "ooxml-downArrowCallout", "downArrowCallout" },
    { "ooxml-mathMinus", "mathMinus" },
    { "ooxml-gear9", "gear9" },
    { "ooxml-round1Rect", "round1Rect" },
    { "ooxml-sun", "sun" },
    { "ooxml-plaque", "plaque" },
    { "ooxml-chevron", "chevron" },
    { "ooxml-flowChartPreparation", "flowChartPreparation" },
    { "ooxml-diagStripe", "diagStripe" },
    { "ooxml-pentagon", "pentagon" },
    { "ooxml-funnel", "funnel" },
    { "ooxml-chartStar", "chartStar" },
    { "ooxml-accentBorderCallout1", "accentBorderCallout1" },
    { "ooxml-notchedRightArrow", "notchedRightArrow" },
    { "ooxml-rightBracket", "rightBracket" },
    { "ooxml-flowChartOffpageConnector", "flowChartOffpageConnector" },
    { "ooxml-leftRightArrow", "leftRightArrow" },
    { "ooxml-decagon", "decagon" },
    { "ooxml-actionButtonHelp", "actionButtonHelp" },
    { "ooxml-star24", "star24" },
    { "ooxml-mathDivide", "mathDivide" },
    { "ooxml-curvedConnector4", "curvedConnector4" },
    { "ooxml-flowChartOr", "flowChartOr" },
    { "ooxml-borderCallout3", "borderCallout3" },
    { "ooxml-upDownArrowCallout", "upDownArrowCallout" },
    { "ooxml-flowChartDecision", "flowChartDecision" },
    { "ooxml-leftRightArrowCallout", "leftRightArrowCallout" },
    { "ooxml-flowChartManualOperation", "flowChartManualOperation" },
    { "ooxml-snipRoundRect", "snipRoundRect" },
    { "ooxml-mathPlus", "mathPlus" },
    { "ooxml-actionButtonForwardNext", "actionButtonForwardNext" },
    { "ooxml-can", "can" },
    { "ooxml-foldedCorner", "foldedCorner" },
    { "ooxml-star32", "star32" },
    { "ooxml-flowChartInternalStorage", "flowChartInternalStorage" },
    { "ooxml-upDownArrow", "upDownArrow" },
    { "ooxml-irregularSeal2", "irregularSeal2" },
    { "ooxml-mathEqual", "mathEqual" },
    { "ooxml-star12", "star12" },
    { "ooxml-uturnArrow", "uturnArrow" },
    { "ooxml-squareTabs", "squareTabs" },
    { "ooxml-leftRightUpArrow", "leftRightUpArrow" },
    { "ooxml-homePlate", "homePlate" },
    { "ooxml-dodecagon", "dodecagon" },
    { "ooxml-leftArrowCallout", "leftArrowCallout" },
    { "ooxml-chord", "chord" },
    { "ooxml-quadArrowCallout", "quadArrowCallout" },
    { "ooxml-actionButtonBeginning", "actionButtonBeginning" },
    { "ooxml-ellipse", "ellipse" },
    { "ooxml-actionButtonEnd", "actionButtonEnd" },
    { "ooxml-arc", "arc" },
    { "ooxml-star16", "star16" },
    { "ooxml-parallelogram", "parallelogram" },
    { "ooxml-bevel", "bevel" },
    { "ooxml-roundRect", "roundRect" },
    { "ooxml-accentCallout1", "accentCallout1" },
    { "ooxml-flowChartSort", "flowChartSort" },
    { "ooxml-star8", "star8" },
    { "ooxml-flowChartAlternateProcess", "flowChartAlternateProcess" },
    { "ooxml-moon", "moon" },
    { "ooxml-star6", "star6" },
    { "ooxml-round2SameRect", "round2SameRect" },
    { "ooxml-nonIsoscelesTrapezoid", "nonIsoscelesTrapezoid" },
    { "ooxml-diamond", "diamond" },
    { "ooxml-ellipseRibbon", "ellipseRibbon" },
    { "ooxml-callout2", "callout2" },
    { "ooxml-pie", "pie" },
    { "ooxml-star4", "star4" },
    { "ooxml-flowChartPredefinedProcess", "flowChartPredefinedProcess" },
    { "ooxml-flowChartPunchedTape", "flowChartPunchedTape" },
    { "ooxml-curvedConnector2", "curvedConnector2" },
    { "ooxml-bentConnector3", "bentConnector3" },
    { "ooxml-cornerTabs", "cornerTabs" },
    { "ooxml-hexagon", "hexagon" },
    { "ooxml-flowChartConnector", "flowChartConnector" },
    { "ooxml-flowChartMagneticDisk", "flowChartMagneticDisk" },
    { "ooxml-heart", "heart" },
    { "ooxml-ribbon2", "ribbon2" },
    { "ooxml-bracePair", "bracePair" },
    { "ooxml-flowChartExtract", "flowChartExtract" },
    { "ooxml-actionButtonHome", "actionButtonHome" },
    { "ooxml-accentBorderCallout3", "accentBorderCallout3" },
    { "ooxml-flowChartOfflineStorage", "flowChartOfflineStorage" },
    { "ooxml-irregularSeal1", "irregularSeal1" },
    { "ooxml-quadArrow", "quadArrow" },
    { "ooxml-leftBrace", "leftBrace" },
    { "ooxml-leftBracket", "leftBracket" },
    { "ooxml-blockArc", "blockArc" },
    { "ooxml-curvedConnector3", "curvedConnector3" },
    { "ooxml-wedgeRoundRectCallout", "wedgeRoundRectCallout" },
    { "ooxml-actionButtonMovie", "actionButtonMovie" },
    { "ooxml-flowChartOnlineStorage", "flowChartOnlineStorage" },
    { "ooxml-gear6", "gear6" },
    { "ooxml-halfFrame", "halfFrame" },
    { "ooxml-snip2SameRect", "snip2SameRect" },
    { "ooxml-triangle", "triangle" },
    { "ooxml-teardrop", "teardrop" },
    { "ooxml-flowChartDocument", "flowChartDocument" },
    { "ooxml-rightArrowCallout", "rightArrowCallout" },
    { "ooxml-rightBrace", "rightBrace" },
    { "ooxml-chartPlus", "chartPlus" },
    { "ooxml-flowChartManualInput", "flowChartManualInput" },
    { "ooxml-flowChartMerge", "flowChartMerge" },
    { "ooxml-line", "line" },
    { "ooxml-downArrow", "downArrow" },
    { "ooxml-curvedDownArrow", "curvedDownArrow" },
    { "ooxml-actionButtonReturn", "actionButtonReturn" },
    { "ooxml-flowChartInputOutput", "flowChartInputOutput" },
    { "ooxml-bracketPair", "bracketPair" },
    { "ooxml-smileyFace", "smileyFace" },
    { "ooxml-actionButtonBlank", "actionButtonBlank" },
    { "ooxml-wave", "wave" },
    { "ooxml-swooshArrow", "swooshArrow" },
    { "ooxml-flowChartSummingJunction", "flowChartSummingJunction" },
    { "ooxml-lightningBolt", "lightningBolt" },
    { "ooxml-flowChartDisplay", "flowChartDisplay" },
    { "ooxml-actionButtonBackPrevious", "actionButtonBackPrevious" },
    { "ooxml-frame", "frame" },
    { "ooxml-rtTriangle", "rtTriangle" },
    { "ooxml-flowChartMagneticTape", "flowChartMagneticTape" },
    { "ooxml-curvedRightArrow", "curvedRightArrow" },
    { "ooxml-leftUpArrow", "leftUpArrow" },
    { "ooxml-wedgeEllipseCallout", "wedgeEllipseCallout" },
    { "ooxml-doubleWave", "doubleWave" },
    { "ooxml-bentArrow", "bentArrow" },
    { "ooxml-star10", "star10" },
    { "ooxml-leftArrow", "leftArrow" },
    { "ooxml-curvedUpArrow", "curvedUpArrow" },
    { "ooxml-snip1Rect", "snip1Rect" },
    { "ooxml-ellipseRibbon2", "ellipseRibbon2" },
    { "ooxml-plus", "plus" },
    { "ooxml-accentCallout3", "accentCallout3" },
    { "ooxml-leftCircularArrow", "leftCircularArrow" },
    { "ooxml-rightArrow", "rightArrow" },
    { "ooxml-flowChartPunchedCard", "flowChartPunchedCard" },
    { "ooxml-snip2DiagRect", "snip2DiagRect" },
    { "ooxml-verticalScroll", "verticalScroll" },
    { "ooxml-star7", "star7" },
    { "ooxml-chartX", "chartX" },
    { "ooxml-cloud", "cloud" },
    { "ooxml-cube", "cube" },
    { "ooxml-round2DiagRect", "round2DiagRect" },
    { "ooxml-flowChartMultidocument", "flowChartMultidocument" },
    { "ooxml-actionButtonDocument", "actionButtonDocument" },
    { "ooxml-flowChartTerminator", "flowChartTerminator" },
    { "ooxml-flowChartDelay", "flowChartDelay" },
    { "ooxml-curvedConnector5", "curvedConnector5" },
    { "ooxml-horizontalScroll", "horizontalScroll" },
    { "ooxml-bentConnector4", "bentConnector4" },
    { "ooxml-leftRightCircularArrow", "leftRightCircularArrow" },
    { "ooxml-wedgeRectCallout", "wedgeRectCallout" },
    { "ooxml-accentCallout2", "accentCallout2" },
    { "ooxml-flowChartMagneticDrum", "flowChartMagneticDrum" },
    { "ooxml-corner", "corner" },
    { "ooxml-borderCallout2", "borderCallout2" },
    { "ooxml-donut", "donut" },
    { "ooxml-flowChartCollate", "flowChartCollate" },
    { "ooxml-mathNotEqual", "mathNotEqual" },
    { "ooxml-bentConnector2", "bentConnector2" },
    { "ooxml-mathMultiply", "mathMultiply" },
    { "ooxml-heptagon", "heptagon" },
    { "ooxml-rect", "rect" },
    { "ooxml-accentBorderCallout2", "accentBorderCallout2" },
    { "ooxml-pieWedge", "pieWedge" },
    { "ooxml-upArrowCallout", "upArrowCallout" },
    { "ooxml-flowChartProcess", "flowChartProcess" },
    { "ooxml-star5", "star5" },
    { "ooxml-lineInv", "lineInv" },
    { "ooxml-straightConnector1", "straightConnector1" },
    { "ooxml-stripedRightArrow", "stripedRightArrow" },
    { "ooxml-callout3", "callout3" },
    { "ooxml-bentUpArrow", "bentUpArrow" },
    { "ooxml-noSmoking", "noSmoking" },
    { "ooxml-trapezoid", "trapezoid" },
    { "ooxml-cloudCallout", "cloudCallout" },
    { "ooxml-callout1", "callout1" },
    { "ooxml-ribbon", "ribbon" },
};

struct StringHash
{
    size_t operator()( const char* s ) const
    {
        return rtl_str_hashCode(s);
    }
};

struct StringCheck
{
    bool operator()( const char* s1, const char* s2 ) const
    {
        return strcmp( s1, s2 ) == 0;
    }
};

typedef boost::unordered_map< const char*, const char*, StringHash, StringCheck> CustomShapeTypeTranslationHashMap;
static CustomShapeTypeTranslationHashMap* pCustomShapeTypeTranslationHashMap = NULL;

static const char* lcl_GetPresetGeometry( const char* sShapeType )
{
    const char* sPresetGeo;

    if( pCustomShapeTypeTranslationHashMap == NULL )
    {
        pCustomShapeTypeTranslationHashMap = new CustomShapeTypeTranslationHashMap ();
        for( unsigned int i = 0; i < sizeof( pCustomShapeTypeTranslationTable )/sizeof( CustomShapeTypeTranslationTable ); i ++ )
        {
            (*pCustomShapeTypeTranslationHashMap)[ pCustomShapeTypeTranslationTable[ i ].sOOo ] = pCustomShapeTypeTranslationTable[ i ].sMSO;
            //DBG(printf("type OOo: %s MSO: %s\n", pCustomShapeTypeTranslationTable[ i ].sOOo, pCustomShapeTypeTranslationTable[ i ].sMSO));
        }
    }

    sPresetGeo = (*pCustomShapeTypeTranslationHashMap)[ sShapeType ];

    if( sPresetGeo == NULL )
        sPresetGeo = "rect";

    return sPresetGeo;
}

namespace oox { namespace drawingml {

#define GETA(propName) \
    GetProperty( rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( #propName ) ) )

#define GETAD(propName) \
    ( GetPropertyAndState( rXPropSet, rXPropState, String( RTL_CONSTASCII_USTRINGPARAM( #propName ) ), eState ) && eState == beans::PropertyState_DIRECT_VALUE )

#define GET(variable, propName) \
    if ( GETA(propName) ) \
        mAny >>= variable;

ShapeExport::ShapeExport( sal_Int32 nXmlNamespace, FSHelperPtr pFS, ShapeHashMap* pShapeMap, XmlFilterBase* pFB, DocumentType eDocumentType )
    : DrawingML( pFS, pFB, eDocumentType )
    , mnShapeIdMax( 1 )
    , mnPictureIdMax( 1 )
    , mnXmlNamespace( nXmlNamespace )
    , maFraction( 1, 576 )
    , maMapModeSrc( MAP_100TH_MM )
    , maMapModeDest( MAP_INCH, Point(), maFraction, maFraction )
    , mpShapeMap( pShapeMap ? pShapeMap : &maShapeMap )
{
}

awt::Size ShapeExport::MapSize( const awt::Size& rSize ) const
{
    Size aRetSize( OutputDevice::LogicToLogic( Size( rSize.Width, rSize.Height ), maMapModeSrc, maMapModeDest ) );

    if ( !aRetSize.Width() )
        aRetSize.Width()++;
    if ( !aRetSize.Height() )
        aRetSize.Height()++;
    return awt::Size( aRetSize.Width(), aRetSize.Height() );
}

sal_Bool ShapeExport::NonEmptyText( Reference< XInterface > xIface )
{
    Reference< XPropertySet > xPropSet( xIface, UNO_QUERY );

    if( xPropSet.is() )
    {
        Reference< XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();
        if ( xPropSetInfo.is() )
        {
            if ( xPropSetInfo->hasPropertyByName( S( "IsEmptyPresentationObject" ) ) )
            {
                sal_Bool bIsEmptyPresObj = sal_False;
                if ( xPropSet->getPropertyValue( S( "IsEmptyPresentationObject" ) ) >>= bIsEmptyPresObj )
                {
                    DBG(printf("empty presentation object %d, props:\n", bIsEmptyPresObj));
                    if( bIsEmptyPresObj )
                       return sal_True;
                }
            }

            if ( xPropSetInfo->hasPropertyByName( S( "IsPresentationObject" ) ) )
            {
                sal_Bool bIsPresObj = sal_False;
                if ( xPropSet->getPropertyValue( S( "IsPresentationObject" ) ) >>= bIsPresObj )
                {
                    DBG(printf("presentation object %d, props:\n", bIsPresObj));
                    if( bIsPresObj )
                       return sal_True;
                }
            }
        }
    }

    Reference< XSimpleText > xText( xIface, UNO_QUERY );

    if( xText.is() )
        return xText->getString().getLength();

    return sal_False;
}

ShapeExport& ShapeExport::WriteBezierShape( Reference< XShape > xShape, sal_Bool bClosed )
{
    DBG(printf("write open bezier shape\n"));

    FSHelperPtr pFS = GetFS();
    pFS->startElementNS( mnXmlNamespace, XML_sp, FSEND );

    PolyPolygon aPolyPolygon = EscherPropertyContainer::GetPolyPolygon( xShape );
    Rectangle aRect( aPolyPolygon.GetBoundRect() );

#if OSL_DEBUG_LEVEL > 0
    awt::Size size = MapSize( awt::Size( aRect.GetWidth(), aRect.GetHeight() ) );
    DBG(printf("poly count %d\nsize: %d x %d", aPolyPolygon.Count(), int( size.Width ), int( size.Height )));
#endif

    // non visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_nvSpPr, FSEND );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvPr,
                          XML_id, I32S( GetNewShapeID( xShape ) ),
                          XML_name, IDS( Freeform ),
                          FSEND );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvSpPr, FSEND );
    WriteNonVisualProperties( xShape );
    pFS->endElementNS( mnXmlNamespace, XML_nvSpPr );

    // visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_spPr, FSEND );
    WriteTransformation( aRect, XML_a );
    WritePolyPolygon( aPolyPolygon );
    Reference< XPropertySet > xProps( xShape, UNO_QUERY );
    if( xProps.is() ) {
        if( bClosed )
            WriteFill( xProps );
        WriteOutline( xProps );
    }

    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    // write text
    WriteTextBox( xShape, mnXmlNamespace );

    pFS->endElementNS( mnXmlNamespace, XML_sp );

    return *this;
}

ShapeExport& ShapeExport::WriteClosedBezierShape( Reference< XShape > xShape )
{
    return WriteBezierShape( xShape, sal_True );
}

ShapeExport& ShapeExport::WriteOpenBezierShape( Reference< XShape > xShape )
{
    return WriteBezierShape( xShape, sal_False );
}

ShapeExport& ShapeExport::WriteCustomShape( Reference< XShape > xShape )
{
    DBG(printf("write custom shape\n"));

    Reference< XPropertySet > rXPropSet( xShape, UNO_QUERY );
    SdrObjCustomShape* pShape = (SdrObjCustomShape*) GetSdrObjectFromXShape( xShape );
    sal_Bool bIsDefaultObject = EscherPropertyContainer::IsDefaultObject( pShape );
    sal_Bool bPredefinedHandlesUsed = sal_True;
    OUString sShapeType;
    sal_uInt32 nMirrorFlags = 0;
    MSO_SPT eShapeType = EscherPropertyContainer::GetCustomShapeType( xShape, nMirrorFlags, sShapeType );
    const char* sPresetShape = lcl_GetPresetGeometry( USS( sShapeType ) );
    DBG(printf("custom shape type: %s ==> %s\n", USS( sShapeType ), sPresetShape));
    Sequence< PropertyValue > aGeometrySeq;
    sal_Int32 nAdjustmentValuesIndex = -1;

    sal_Bool bFlipH = false;
    sal_Bool bFlipV = false;

    if( GETA( CustomShapeGeometry ) ) {
        DBG(printf("got custom shape geometry\n"));
        if( mAny >>= aGeometrySeq ) {

            DBG(printf("got custom shape geometry sequence\n"));
            for( int i = 0; i < aGeometrySeq.getLength(); i++ ) {
                const PropertyValue& rProp = aGeometrySeq[ i ];
                DBG(printf("geometry property: %s\n", USS( rProp.Name )));

                if ( rProp.Name == "MirroredX" )
                    rProp.Value >>= bFlipH;

                if ( rProp.Name == "MirroredY" )
                    rProp.Value >>= bFlipV;
                if ( rProp.Name == "AdjustmentValues" )
                    nAdjustmentValuesIndex = i;
                else if ( rProp.Name == "Handles" ) {
                    if( !bIsDefaultObject )
                        bPredefinedHandlesUsed = sal_False;
                    // TODO: update nAdjustmentsWhichNeedsToBeConverted here
                }
            }
        }
    }

    FSHelperPtr pFS = GetFS();
    pFS->startElementNS( mnXmlNamespace, XML_sp, FSEND );

    // non visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_nvSpPr, FSEND );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvPr,
                          XML_id, I32S( GetNewShapeID( xShape ) ),
                          XML_name, IDS( CustomShape ),
                          FSEND );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvSpPr, FSEND );
    WriteNonVisualProperties( xShape );
    pFS->endElementNS( mnXmlNamespace, XML_nvSpPr );

    // visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_spPr, FSEND );
    WriteShapeTransformation( xShape, XML_a, bFlipH, bFlipV );
    if( nAdjustmentValuesIndex != -1 )
    {
        sal_Int32 nAdjustmentsWhichNeedsToBeConverted = 0;
        WritePresetShape( sPresetShape, eShapeType, bPredefinedHandlesUsed,
                          nAdjustmentsWhichNeedsToBeConverted, aGeometrySeq[ nAdjustmentValuesIndex ] );
    }
    else
        WritePresetShape( sPresetShape );
    if( rXPropSet.is() )
    {
        WriteFill( rXPropSet );
        WriteOutline( rXPropSet );
    }

    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    // write text
    WriteTextBox( xShape, mnXmlNamespace );

    pFS->endElementNS( mnXmlNamespace, XML_sp );

    return *this;
}

ShapeExport& ShapeExport::WriteEllipseShape( Reference< XShape > xShape )
{
    DBG(printf("write ellipse shape\n"));

    FSHelperPtr pFS = GetFS();

    pFS->startElementNS( mnXmlNamespace, XML_sp, FSEND );

    // TODO: arc, section, cut, connector

    // non visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_nvSpPr, FSEND );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvPr,
                          XML_id, I32S( GetNewShapeID( xShape ) ),
                          XML_name, IDS( Ellipse ),
                          FSEND );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvSpPr, FSEND );
    WriteNonVisualProperties( xShape );
    pFS->endElementNS( mnXmlNamespace, XML_nvSpPr );

    // visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_spPr, FSEND );
    WriteShapeTransformation( xShape, XML_a );
    WritePresetShape( "ellipse" );
    Reference< XPropertySet > xProps( xShape, UNO_QUERY );
    if( xProps.is() )
    {
        WriteFill( xProps );
        WriteOutline( xProps );
    }
    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    // write text
    WriteTextBox( xShape, mnXmlNamespace );

    pFS->endElementNS( mnXmlNamespace, XML_sp );

    return *this;
}

ShapeExport& ShapeExport::WriteGraphicObjectShape( Reference< XShape > xShape )
{
    DBG(printf("write graphic object shape\n"));

    if( NonEmptyText( xShape ) )
    {
        WriteTextShape( xShape );

        //DBG(dump_pset(mXPropSet));

        return *this;
    }

    DBG(printf("graphicObject without text\n"));

    OUString sGraphicURL;
    Reference< XPropertySet > xShapeProps( xShape, UNO_QUERY );
    if( !xShapeProps.is() || !( xShapeProps->getPropertyValue( S( "GraphicURL" ) ) >>= sGraphicURL ) )
    {
        DBG(printf("no graphic URL found\n"));
        return *this;
    }

    FSHelperPtr pFS = GetFS();

    pFS->startElementNS( mnXmlNamespace, XML_pic, FSEND );

    pFS->startElementNS( mnXmlNamespace, XML_nvPicPr, FSEND );

    OUString sName, sDescr;
    bool bHaveName = xShapeProps->getPropertyValue( S( "Name" ) ) >>= sName;
    bool bHaveDesc = xShapeProps->getPropertyValue( S( "Description" ) ) >>= sDescr;

    pFS->singleElementNS( mnXmlNamespace, XML_cNvPr,
                          XML_id,     I32S( GetNewShapeID( xShape ) ),
                          XML_name,   bHaveName ? USS( sName ) : (OString("Picture ") + OString::valueOf( mnPictureIdMax++ )).getStr(),
                          XML_descr,  bHaveDesc ? USS( sDescr ) : NULL,
                          FSEND );
    // OOXTODO: //cNvPr children: XML_extLst, XML_hlinkClick, XML_hlinkHover

    pFS->singleElementNS( mnXmlNamespace, XML_cNvPicPr,
                          // OOXTODO: XML_preferRelativeSize
                          FSEND );

    WriteNonVisualProperties( xShape );

    pFS->endElementNS( mnXmlNamespace, XML_nvPicPr );

    pFS->startElementNS( mnXmlNamespace, XML_blipFill, FSEND );

    WriteBlip( xShapeProps, sGraphicURL );

    bool bStretch = false;
    if( ( xShapeProps->getPropertyValue( S( "FillBitmapStretch" ) ) >>= bStretch ) && bStretch )
    {
        WriteStretch();
    }

    pFS->endElementNS( mnXmlNamespace, XML_blipFill );

    // visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_spPr, FSEND );
    WriteShapeTransformation( xShape, XML_a );
    WritePresetShape( "rect" );
    // graphic object can come with the frame (bnc#654525)
    WriteOutline( xShapeProps );
    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    pFS->endElementNS( mnXmlNamespace, XML_pic );

    return *this;
}

ShapeExport& ShapeExport::WriteConnectorShape( Reference< XShape > xShape )
{
    sal_Bool bFlipH = false;
    sal_Bool bFlipV = false;

    DBG(printf("write connector shape\n"));

    FSHelperPtr pFS = GetFS();

    const char* sGeometry = "line";
    Reference< XPropertySet > rXPropSet( xShape, UNO_QUERY );
    Reference< XPropertyState > rXPropState( xShape, UNO_QUERY );
    awt::Point aStartPoint, aEndPoint;
    Reference< XShape > rXShapeA;
    Reference< XShape > rXShapeB;
    PropertyState eState;
    ConnectorType eConnectorType;
    if( GETAD( EdgeKind ) ) {
        mAny >>= eConnectorType;

        switch( eConnectorType ) {
            case ConnectorType_CURVE:
                sGeometry = "curvedConnector3";
                break;
            case ConnectorType_STANDARD:
                sGeometry = "bentConnector3";
                break;
            default:
            case ConnectorType_LINE:
            case ConnectorType_LINES:
                sGeometry = "straightConnector1";
                break;
        }

        if( GETAD( EdgeStartPoint ) ) {
            mAny >>= aStartPoint;
            if( GETAD( EdgeEndPoint ) ) {
                mAny >>= aEndPoint;
            }
        }
        GET( rXShapeA, EdgeStartConnection );
        GET( rXShapeB, EdgeEndConnection );
    }
    EscherConnectorListEntry aConnectorEntry( xShape, aStartPoint, rXShapeA, aEndPoint, rXShapeB );

    Rectangle aRect( Point( aStartPoint.X, aStartPoint.Y ), Point( aEndPoint.X, aEndPoint.Y ) );
    if( aRect.getWidth() < 0 ) {
        bFlipH = sal_True;
        aRect.setX( aEndPoint.X );
        aRect.setWidth( aStartPoint.X - aEndPoint.X );
    }

    if( aRect.getHeight() < 0 ) {
        bFlipV = sal_True;
        aRect.setY( aEndPoint.Y );
        aRect.setHeight( aStartPoint.Y - aEndPoint.Y );
    }

    pFS->startElementNS( mnXmlNamespace, XML_cxnSp, FSEND );

    // non visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_nvCxnSpPr, FSEND );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvPr,
                          XML_id, I32S( GetNewShapeID( xShape ) ),
                          XML_name, IDS( Line ),
                          FSEND );
    // non visual connector shape drawing properties
    pFS->startElementNS( mnXmlNamespace, XML_cNvCxnSpPr, FSEND );
    WriteConnectorConnections( aConnectorEntry, GetShapeID( rXShapeA ), GetShapeID( rXShapeB ) );
    pFS->endElementNS( mnXmlNamespace, XML_cNvCxnSpPr );
    pFS->singleElementNS( mnXmlNamespace, XML_nvPr, FSEND );
    pFS->endElementNS( mnXmlNamespace, XML_nvCxnSpPr );

    // visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_spPr, FSEND );
    WriteTransformation( aRect, mnXmlNamespace, bFlipH, bFlipV );
    // TODO: write adjustments (ppt export doesn't work well there either)
    WritePresetShape( sGeometry );
    Reference< XPropertySet > xShapeProps( xShape, UNO_QUERY );
    if( xShapeProps.is() )
        WriteOutline( xShapeProps );
    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    // write text
    WriteTextBox( xShape, mnXmlNamespace );

    pFS->endElementNS( mnXmlNamespace, XML_cxnSp );

    return *this;
}

ShapeExport& ShapeExport::WriteLineShape( Reference< XShape > xShape )
{
    sal_Bool bFlipH = false;
    sal_Bool bFlipV = false;

    DBG(printf("write line shape\n"));

    FSHelperPtr pFS = GetFS();

    pFS->startElementNS( mnXmlNamespace, XML_sp, FSEND );

    PolyPolygon aPolyPolygon = EscherPropertyContainer::GetPolyPolygon( xShape );
    if( aPolyPolygon.Count() == 1 && aPolyPolygon[ 0 ].GetSize() == 2)
    {
        const Polygon& rPoly = aPolyPolygon[ 0 ];

        bFlipH = ( rPoly[ 0 ].X() > rPoly[ 1 ].X() );
        bFlipV = ( rPoly[ 0 ].Y() > rPoly[ 1 ].Y() );
    }

    // non visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_nvSpPr, FSEND );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvPr,
                          XML_id, I32S( GetNewShapeID( xShape ) ),
                          XML_name, IDS( Line ),
                          FSEND );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvSpPr, FSEND );
    WriteNonVisualProperties( xShape );
    pFS->endElementNS( mnXmlNamespace, XML_nvSpPr );

    // visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_spPr, FSEND );
    WriteShapeTransformation( xShape, XML_a, bFlipH, bFlipV );
    WritePresetShape( "line" );
    Reference< XPropertySet > xShapeProps( xShape, UNO_QUERY );
    if( xShapeProps.is() )
        WriteOutline( xShapeProps );
    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    // write text
    WriteTextBox( xShape, mnXmlNamespace );

    pFS->endElementNS( mnXmlNamespace, XML_sp );

    return *this;
}

ShapeExport& ShapeExport::WriteNonVisualDrawingProperties( Reference< XShape > xShape, const char* pName )
{
    GetFS()->singleElementNS( mnXmlNamespace, XML_cNvPr,
                              XML_id, I32S( GetNewShapeID( xShape ) ),
                              XML_name, pName,
                              FSEND );

    return *this;
}

ShapeExport& ShapeExport::WriteNonVisualProperties( Reference< XShape > )
{
    // Override to generate //nvPr elements.
    return *this;
}

ShapeExport& ShapeExport::WriteRectangleShape( Reference< XShape > xShape )
{
    DBG(printf("write rectangle shape\n"));

    FSHelperPtr pFS = GetFS();

    pFS->startElementNS( mnXmlNamespace, XML_sp, FSEND );

    sal_Int32 nRadius = 0;

    Reference< XPropertySet > xShapeProps( xShape, UNO_QUERY );
    if( xShapeProps.is() )
    {
        xShapeProps->getPropertyValue( S( "CornerRadius" ) ) >>= nRadius;
    }

    if( nRadius )
    {
        nRadius = MapSize( awt::Size( nRadius, 0 ) ).Width;
    }

    // non visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_nvSpPr, FSEND );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvPr,
                          XML_id, I32S( GetNewShapeID( xShape ) ),
                          XML_name, IDS( Rectangle ),
                          FSEND );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvSpPr, FSEND );
    WriteNonVisualProperties( xShape );
    pFS->endElementNS( mnXmlNamespace, XML_nvSpPr );

    // visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_spPr, FSEND );
    WriteShapeTransformation( xShape, XML_a );
    WritePresetShape( "rect" );
    Reference< XPropertySet > xProps( xShape, UNO_QUERY );
    if( xProps.is() )
    {
        WriteFill( xProps );
        WriteOutline( xProps );
    }
    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    // write text
    WriteTextBox( xShape, mnXmlNamespace );

    pFS->endElementNS( mnXmlNamespace, XML_sp );

    return *this;
}

typedef ShapeExport& (ShapeExport::*ShapeConverter)( Reference< XShape > );
typedef boost::unordered_map< const char*, ShapeConverter, StringHash, StringCheck> NameToConvertMapType;

static const NameToConvertMapType& lcl_GetConverters()
{
    static bool shape_map_inited = false;
    static NameToConvertMapType shape_converters;
    if( shape_map_inited )
    {
        return shape_converters;
    }

    shape_converters[ "com.sun.star.drawing.ClosedBezierShape" ]        = &ShapeExport::WriteClosedBezierShape;
    shape_converters[ "com.sun.star.drawing.ConnectorShape" ]           = &ShapeExport::WriteConnectorShape;
    shape_converters[ "com.sun.star.drawing.CustomShape" ]              = &ShapeExport::WriteCustomShape;
    shape_converters[ "com.sun.star.drawing.EllipseShape" ]             = &ShapeExport::WriteEllipseShape;
    shape_converters[ "com.sun.star.drawing.GraphicObjectShape" ]       = &ShapeExport::WriteGraphicObjectShape;
    shape_converters[ "com.sun.star.drawing.LineShape" ]                = &ShapeExport::WriteLineShape;
    shape_converters[ "com.sun.star.drawing.OpenBezierShape" ]          = &ShapeExport::WriteOpenBezierShape;
    shape_converters[ "com.sun.star.drawing.RectangleShape" ]           = &ShapeExport::WriteRectangleShape;
    shape_converters[ "com.sun.star.drawing.OLE2Shape" ]                = &ShapeExport::WriteOLE2Shape;
    shape_converters[ "com.sun.star.drawing.TableShape" ]               = &ShapeExport::WriteTableShape;
    shape_converters[ "com.sun.star.drawing.TextShape" ]                = &ShapeExport::WriteTextShape;
    shape_converters[ "com.sun.star.presentation.DateTimeShape" ]       = &ShapeExport::WriteTextShape;
    shape_converters[ "com.sun.star.presentation.FooterShape" ]         = &ShapeExport::WriteTextShape;
    shape_converters[ "com.sun.star.presentation.HeaderShape" ]         = &ShapeExport::WriteTextShape;
    shape_converters[ "com.sun.star.presentation.NotesShape" ]          = &ShapeExport::WriteTextShape;
    shape_converters[ "com.sun.star.presentation.OutlinerShape" ]       = &ShapeExport::WriteTextShape;
    shape_converters[ "com.sun.star.presentation.SlideNumberShape" ]    = &ShapeExport::WriteTextShape;
    shape_converters[ "com.sun.star.presentation.TitleTextShape" ]      = &ShapeExport::WriteTextShape;
    shape_map_inited = true;

    return shape_converters;
}

ShapeExport& ShapeExport::WriteShape( Reference< XShape > xShape )
{
    OUString sShapeType = xShape->getShapeType();
    DBG( printf( "write shape: %s\n", USS( sShapeType ) ) );
    NameToConvertMapType::const_iterator aConverter = lcl_GetConverters().find( USS( sShapeType ) );
    if( aConverter == lcl_GetConverters().end() )
    {
        DBG( printf( "unknown shape\n" ) );
        return WriteUnknownShape( xShape );
    }
    (this->*(aConverter->second))( xShape );

    return *this;
}

ShapeExport& ShapeExport::WriteTextBox( Reference< XInterface > xIface, sal_Int32 nXmlNamespace )
{
    if( NonEmptyText( xIface ) )
    {
        FSHelperPtr pFS = GetFS();

        pFS->startElementNS( nXmlNamespace, XML_txBody, FSEND );
        WriteText( xIface );
        pFS->endElementNS( nXmlNamespace, XML_txBody );
    }

    return *this;
}

void ShapeExport::WriteTable( Reference< XShape > rXShape  )
{
    OSL_TRACE("write table");

    Reference< XTable > xTable;
    Reference< XPropertySet > xPropSet( rXShape, UNO_QUERY );

    mpFS->startElementNS( XML_a, XML_graphic, FSEND );
    mpFS->startElementNS( XML_a, XML_graphicData, XML_uri, "http://schemas.openxmlformats.org/drawingml/2006/table", FSEND );

    if ( xPropSet.is() && ( xPropSet->getPropertyValue( S("Model") ) >>= xTable ) )
    {
        mpFS->startElementNS( XML_a, XML_tbl, FSEND );
        mpFS->singleElementNS( XML_a, XML_tblPr, FSEND );

        Reference< XColumnRowRange > xColumnRowRange( xTable, UNO_QUERY_THROW );
        Reference< container::XIndexAccess > xColumns( xColumnRowRange->getColumns(), UNO_QUERY_THROW );
        Reference< container::XIndexAccess > xRows( xColumnRowRange->getRows(), UNO_QUERY_THROW );
        sal_uInt16 nRowCount = static_cast< sal_uInt16 >( xRows->getCount() );
        sal_uInt16 nColumnCount = static_cast< sal_uInt16 >( xColumns->getCount() );

        mpFS->startElementNS( XML_a, XML_tblGrid, FSEND );

        for ( sal_Int32 x = 0; x < nColumnCount; x++ )
        {
            Reference< XPropertySet > xColPropSet( xColumns->getByIndex( x ), UNO_QUERY_THROW );
            sal_Int32 nWidth(0);
            xColPropSet->getPropertyValue( S("Width") ) >>= nWidth;

            mpFS->singleElementNS( XML_a, XML_gridCol, XML_w, I64S(MM100toEMU(nWidth)), FSEND );
        }

        mpFS->endElementNS( XML_a, XML_tblGrid );

        Reference< XCellRange > xCellRange( xTable, UNO_QUERY_THROW );
        for( sal_Int32 nRow = 0; nRow < nRowCount; nRow++ )
        {
            Reference< XPropertySet > xRowPropSet( xRows->getByIndex( nRow ), UNO_QUERY_THROW );
            sal_Int32 nRowHeight(0);

            xRowPropSet->getPropertyValue( S("Height") ) >>= nRowHeight;

            mpFS->startElementNS( XML_a, XML_tr, XML_h, I64S( MM100toEMU( nRowHeight ) ), FSEND );

            for( sal_Int32 nColumn = 0; nColumn < nColumnCount; nColumn++ )
            {
                Reference< XMergeableCell > xCell( xCellRange->getCellByPosition( nColumn, nRow ), UNO_QUERY_THROW );
                if ( !xCell->isMerged() )
                {
                    mpFS->startElementNS( XML_a, XML_tc, FSEND );

                    WriteTextBox( xCell, XML_a );

                    mpFS->singleElementNS( XML_a, XML_tcPr, FSEND );
                    mpFS->endElementNS( XML_a, XML_tc );
                }
            }

            mpFS->endElementNS( XML_a, XML_tr );
        }

        mpFS->endElementNS( XML_a, XML_tbl );
    }

    mpFS->endElementNS( XML_a, XML_graphicData );
    mpFS->endElementNS( XML_a, XML_graphic );
}

ShapeExport& ShapeExport::WriteTableShape( Reference< XShape > xShape )
{
    FSHelperPtr pFS = GetFS();

    OSL_TRACE("write table shape");

    pFS->startElementNS( mnXmlNamespace, XML_graphicFrame, FSEND );

    pFS->startElementNS( mnXmlNamespace, XML_nvGraphicFramePr, FSEND );

    pFS->singleElementNS( mnXmlNamespace, XML_cNvPr,
                          XML_id,     I32S( GetNewShapeID( xShape ) ),
                          XML_name,   IDS(Table),
                          FSEND );

    pFS->singleElementNS( mnXmlNamespace, XML_cNvGraphicFramePr,
                          FSEND );

    if( GetDocumentType() == DOCUMENT_PPTX )
        pFS->singleElementNS( mnXmlNamespace, XML_nvPr,
                          FSEND );
    pFS->endElementNS( mnXmlNamespace, XML_nvGraphicFramePr );

    WriteShapeTransformation( xShape, mnXmlNamespace );
    WriteTable( xShape );

    pFS->endElementNS( mnXmlNamespace, XML_graphicFrame );

    return *this;
}

ShapeExport& ShapeExport::WriteTextShape( Reference< XShape > xShape )
{
    FSHelperPtr pFS = GetFS();

    pFS->startElementNS( mnXmlNamespace, XML_sp, FSEND );

    // non visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_nvSpPr, FSEND );
    WriteNonVisualDrawingProperties( xShape, IDS( TextShape ) );
    pFS->singleElementNS( mnXmlNamespace, XML_cNvSpPr, XML_txBox, "1", FSEND );
    WriteNonVisualProperties( xShape );
    pFS->endElementNS( mnXmlNamespace, XML_nvSpPr );

    // visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_spPr, FSEND );
    WriteShapeTransformation( xShape, XML_a );
    WritePresetShape( "rect" );
    WriteBlipFill( Reference< XPropertySet >(xShape, UNO_QUERY ), S( "GraphicURL" ) );
    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    WriteTextBox( xShape, mnXmlNamespace );

    pFS->endElementNS( mnXmlNamespace, XML_sp );

    return *this;
}

ShapeExport& ShapeExport::WriteOLE2Shape( Reference< XShape > xShape )
{
    Reference< XPropertySet > xPropSet( xShape, UNO_QUERY );
    if( xPropSet.is() && GetProperty( xPropSet, S("Model") ) )
    {
        Reference< XChartDocument > xChartDoc;
        mAny >>= xChartDoc;
        if( xChartDoc.is() )
        {
            //export the chart
            Reference< XModel > xModel( xChartDoc, UNO_QUERY );
            ChartExport aChartExport( mnXmlNamespace, GetFS(), xModel, GetFB(), GetDocumentType() );
            static sal_Int32 nChartCount = 0;
            aChartExport.WriteChartObj( xShape, ++nChartCount );
        }
    }
    return *this;
}

ShapeExport& ShapeExport::WriteUnknownShape( Reference< XShape > )
{
    // Override this method to do something useful.
    return *this;
}

size_t ShapeExport::ShapeHash::operator()( const Reference < XShape > rXShape ) const
{
    return rXShape->getShapeType().hashCode();
}

sal_Int32 ShapeExport::GetNewShapeID( const Reference< XShape > rXShape )
{
    return GetNewShapeID( rXShape, GetFB() );
}

sal_Int32 ShapeExport::GetNewShapeID( const Reference< XShape > rXShape, XmlFilterBase* pFB )
{
    if( !rXShape.is() )
        return -1;

    sal_Int32 nID = pFB->GetUniqueId();

    (*mpShapeMap)[ rXShape ] = nID;

    return nID;
}

sal_Int32 ShapeExport::GetShapeID( const Reference< XShape > rXShape )
{
    return GetShapeID( rXShape, mpShapeMap );
}

sal_Int32 ShapeExport::GetShapeID( const Reference< XShape > rXShape, ShapeHashMap* pShapeMap )
{
    if( !rXShape.is() )
        return -1;

    ShapeHashMap::const_iterator aIter = pShapeMap->find( rXShape );

    if( aIter == pShapeMap->end() )
        return -1;

    return aIter->second;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
