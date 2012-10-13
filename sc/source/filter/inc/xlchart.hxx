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

#ifndef SC_XLCHART_HXX
#define SC_XLCHART_HXX

// disable/enable support for varied point colors property
#define EXC_CHART2_VARYCOLORSBY_PROP 0
// disable/enable restriction to hair lines in 3D bar charts (#i83151#)
#define EXC_CHART2_3DBAR_HAIRLINES_ONLY 1

#include <map>
#include <tools/gen.hxx>
#include "fapihelper.hxx"
#include <boost/shared_ptr.hpp>

namespace com { namespace sun { namespace star {
    namespace container { class XNameContainer; }
    namespace lang      { class XMultiServiceFactory; }
    namespace chart     { class XChartDocument; }
    namespace chart2    { class XChartDocument; }
    namespace drawing   { class XShape; }
} } }

class XclRoot;

// Property names =============================================================

// service names
#define SERVICE_DRAWING_BITMAPTABLE         "com.sun.star.drawing.BitmapTable"
#define SERVICE_DRAWING_DASHTABLE           "com.sun.star.drawing.DashTable"
#define SERVICE_DRAWING_GRADIENTTABLE       "com.sun.star.drawing.GradientTable"
#define SERVICE_DRAWING_HATCHTABLE          "com.sun.star.drawing.HatchTable"

#define SERVICE_CHART2_AXIS                 "com.sun.star.chart2.Axis"
#define SERVICE_CHART2_CARTESIANCOORDSYS2D  "com.sun.star.chart2.CartesianCoordinateSystem2d"
#define SERVICE_CHART2_CARTESIANCOORDSYS3D  "com.sun.star.chart2.CartesianCoordinateSystem3d"
#define SERVICE_CHART2_DATAPROVIDER         "com.sun.star.chart2.data.DataProvider"
#define SERVICE_CHART2_DATASERIES           "com.sun.star.chart2.DataSeries"
#define SERVICE_CHART2_DIAGRAM              "com.sun.star.chart2.Diagram"
#define SERVICE_CHART2_ERRORBAR             "com.sun.star.chart2.ErrorBar"
#define SERVICE_CHART2_EXPREGCURVE          "com.sun.star.chart2.ExponentialRegressionCurve"
#define SERVICE_CHART2_FORMATTEDSTRING      "com.sun.star.chart2.FormattedString"
#define SERVICE_CHART2_LABELEDDATASEQ       "com.sun.star.chart2.data.LabeledDataSequence"
#define SERVICE_CHART2_LEGEND               "com.sun.star.chart2.Legend"
#define SERVICE_CHART2_LINEARREGCURVE       "com.sun.star.chart2.LinearRegressionCurve"
#define SERVICE_CHART2_LINEARSCALING        "com.sun.star.chart2.LinearScaling"
#define SERVICE_CHART2_LOGREGCURVE          "com.sun.star.chart2.LogarithmicRegressionCurve"
#define SERVICE_CHART2_LOGSCALING           "com.sun.star.chart2.LogarithmicScaling"
#define SERVICE_CHART2_POLARCOORDSYS2D      "com.sun.star.chart2.PolarCoordinateSystem2d"
#define SERVICE_CHART2_POLARCOORDSYS3D      "com.sun.star.chart2.PolarCoordinateSystem3d"
#define SERVICE_CHART2_POTREGCURVE          "com.sun.star.chart2.PotentialRegressionCurve"
#define SERVICE_CHART2_TITLE                "com.sun.star.chart2.Title"

// property names
#define EXC_CHPROP_ADDITIONALSHAPES         "AdditionalShapes"
#define EXC_CHPROP_ANCHORPOSITION           "AnchorPosition"
#define EXC_CHPROP_ARRANGEORDER             "ArrangeOrder"
#define EXC_CHPROP_ATTAXISINDEX             "AttachedAxisIndex"
#define EXC_CHPROP_ATTRIBDATAPOINTS         "AttributedDataPoints"
#define EXC_CHPROP_BLACKDAY                 "BlackDay"
#define EXC_CHPROP_COLOR                    "Color"
#define EXC_CHPROP_CONNECTBARS              "ConnectBars"
#define EXC_CHPROP_CROSSOVERPOSITION        "CrossoverPosition"
#define EXC_CHPROP_CROSSOVERVALUE           "CrossoverValue"
#define EXC_CHPROP_CURVESTYLE               "CurveStyle"
#define EXC_CHPROP_D3DSCENEAMBIENTCOLOR     "D3DSceneAmbientColor"
#define EXC_CHPROP_D3DSCENELIGHTON1         "D3DSceneLightOn1"
#define EXC_CHPROP_D3DSCENELIGHTCOLOR2      "D3DSceneLightColor2"
#define EXC_CHPROP_D3DSCENELIGHTDIR2        "D3DSceneLightDirection2"
#define EXC_CHPROP_D3DSCENELIGHTON2         "D3DSceneLightOn2"
#define EXC_CHPROP_D3DSCENEPERSPECTIVE      "D3DScenePerspective"
#define EXC_CHPROP_D3DSCENESHADEMODE        "D3DSceneShadeMode"
#define EXC_CHPROP_DISPLAYLABELS            "DisplayLabels"
#define EXC_CHPROP_ERRORBARSTYLE            "ErrorBarStyle"
#define EXC_CHPROP_ERRORBARX                "ErrorBarX"
#define EXC_CHPROP_ERRORBARY                "ErrorBarY"
#define EXC_CHPROP_EXPANSION                "Expansion"
#define EXC_CHPROP_EXPTIMEINCREMENT         "ExplicitTimeIncrement"
#define EXC_CHPROP_GAPWIDTHSEQ              "GapwidthSequence"
#define EXC_CHPROP_GEOMETRY3D               "Geometry3D"
#define EXC_CHPROP_INCLUDEHIDDENCELLS       "IncludeHiddenCells"
#define EXC_CHPROP_JAPANESE                 "Japanese"
#define EXC_CHPROP_LABEL                    "Label"
#define EXC_CHPROP_LABELPLACEMENT           "LabelPlacement"
#define EXC_CHPROP_LABELPOSITION            "LabelPosition"
#define EXC_CHPROP_LABELSEPARATOR           "LabelSeparator"
#define EXC_CHPROP_MAJORTICKS               "MajorTickmarks"
#define EXC_CHPROP_MARKPOSITION             "MarkPosition"
#define EXC_CHPROP_MINORTICKS               "MinorTickmarks"
#define EXC_CHPROP_MISSINGVALUETREATMENT    "MissingValueTreatment"
#define EXC_CHPROP_NEGATIVEERROR            "NegativeError"
#define EXC_CHPROP_NUMBERFORMAT             "NumberFormat"
#define EXC_CHPROP_OFFSET                   "Offset"
#define EXC_CHPROP_OVERLAPSEQ               "OverlapSequence"
#define EXC_CHPROP_PERCENTAGENUMFMT         "PercentageNumberFormat"
#define EXC_CHPROP_PERCENTDIAGONAL          "PercentDiagonal"
#define EXC_CHPROP_PERSPECTIVE              "Perspective"
#define EXC_CHPROP_POSITIVEERROR            "PositiveError"
#define EXC_CHPROP_RELATIVEPOSITION         "RelativePosition"
#define EXC_CHPROP_RELATIVESIZE             "RelativeSize"
#define EXC_CHPROP_RIGHTANGLEDAXES          "RightAngledAxes"
#define EXC_CHPROP_ROLE                     "Role"
#define EXC_CHPROP_ROTATIONHORIZONTAL       "RotationHorizontal"
#define EXC_CHPROP_ROTATIONVERTICAL         "RotationVertical"
#define EXC_CHPROP_SHOW                     "Show"
#define EXC_CHPROP_SHOWCORRELATION          "ShowCorrelationCoefficient"
#define EXC_CHPROP_SHOWEQUATION             "ShowEquation"
#define EXC_CHPROP_SHOWFIRST                "ShowFirst"
#define EXC_CHPROP_SHOWHIGHLOW              "ShowHighLow"
#define EXC_CHPROP_SHOWNEGATIVEERROR        "ShowNegativeError"
#define EXC_CHPROP_SHOWPOSITIVEERROR        "ShowPositiveError"
#define EXC_CHPROP_STACKCHARACTERS          "StackCharacters"
#define EXC_CHPROP_STACKINGDIR              "StackingDirection"
#define EXC_CHPROP_STARTINGANGLE            "StartingAngle"
#define EXC_CHPROP_SWAPXANDYAXIS            "SwapXAndYAxis"
#define EXC_CHPROP_SYMBOL                   "Symbol"
#define EXC_CHPROP_TEXTBREAK                "TextBreak"
#define EXC_CHPROP_TEXTOVERLAP              "TextOverlap"
#define EXC_CHPROP_TEXTROTATION             "TextRotation"
#define EXC_CHPROP_USERINGS                 "UseRings"
#define EXC_CHPROP_VARYCOLORSBY             "VaryColorsByPoint"
#define EXC_CHPROP_WEIGHT                   "Weight"
#define EXC_CHPROP_WHITEDAY                 "WhiteDay"

// data series roles
#define EXC_CHPROP_ROLE_CATEG               "categories"
#define EXC_CHPROP_ROLE_ERRORBARS_NEGX      "error-bars-x-negative"
#define EXC_CHPROP_ROLE_ERRORBARS_NEGY      "error-bars-y-negative"
#define EXC_CHPROP_ROLE_ERRORBARS_POSX      "error-bars-x-positive"
#define EXC_CHPROP_ROLE_ERRORBARS_POSY      "error-bars-y-positive"
#define EXC_CHPROP_ROLE_LABEL               "label"
#define EXC_CHPROP_ROLE_XVALUES             "values-x"
#define EXC_CHPROP_ROLE_YVALUES             "values-y"
#define EXC_CHPROP_ROLE_OPENVALUES          "values-first"
#define EXC_CHPROP_ROLE_CLOSEVALUES         "values-last"
#define EXC_CHPROP_ROLE_LOWVALUES           "values-min"
#define EXC_CHPROP_ROLE_HIGHVALUES          "values-max"
#define EXC_CHPROP_ROLE_SIZEVALUES          "values-size"

// Constants and Enumerations =================================================

const sal_Size EXC_CHART_PROGRESS_SIZE          = 10;
const sal_uInt16 EXC_CHART_AUTOROTATION         = 0xFFFF;   /// Automatic rotation, e.g. axis labels (internal use only).

const sal_Int32 EXC_CHART_AXIS_NONE             = -1;       /// For internal use only.
const sal_Int32 EXC_CHART_AXIS_X                = 0;        /// API X axis index.
const sal_Int32 EXC_CHART_AXIS_Y                = 1;        /// API Y axis index.
const sal_Int32 EXC_CHART_AXIS_Z                = 2;        /// API Z axis index.
const sal_Int32 EXC_CHART_AXESSET_NONE          = -1;       /// For internal use only.
const sal_Int32 EXC_CHART_AXESSET_PRIMARY       = 0;        /// API primary axes set index.
const sal_Int32 EXC_CHART_AXESSET_SECONDARY     = 1;        /// API secondary axes set index.

const sal_Int32 EXC_CHART_TOTALUNITS            = 4000;     /// Most chart objects are positioned in 1/4000 of chart area.
const sal_Int32 EXC_CHART_PLOTAREAUNITS         = 1000;     /// For objects that are positioned in 1/1000 of plot area.

// (0x0850) CHFRINFO ----------------------------------------------------------

const sal_uInt16 EXC_ID_CHFRINFO                = 0x0850;

const sal_uInt8 EXC_CHFRINFO_EXCEL2000          = 9;
const sal_uInt8 EXC_CHFRINFO_EXCELXP2003        = 10;
const sal_uInt8 EXC_CHFRINFO_EXCEL2007          = 11;

// (0x0852, 0x0853) CHFRBLOCKBEGIN, CHFRBLOCKEND ------------------------------

const sal_uInt16 EXC_ID_CHFRBLOCKBEGIN          = 0x0852;
const sal_uInt16 EXC_ID_CHFRBLOCKEND            = 0x0853;

const sal_uInt16 EXC_CHFRBLOCK_TYPE_AXESSET     = 0;
const sal_uInt16 EXC_CHFRBLOCK_TYPE_TEXT        = 2;
const sal_uInt16 EXC_CHFRBLOCK_TYPE_AXIS        = 4;
const sal_uInt16 EXC_CHFRBLOCK_TYPE_TYPEGROUP   = 5;
const sal_uInt16 EXC_CHFRBLOCK_TYPE_DATATABLE   = 6;
const sal_uInt16 EXC_CHFRBLOCK_TYPE_FRAME       = 7;
const sal_uInt16 EXC_CHFRBLOCK_TYPE_LEGEND      = 9;
const sal_uInt16 EXC_CHFRBLOCK_TYPE_LEGENDEX    = 10;
const sal_uInt16 EXC_CHFRBLOCK_TYPE_SERIES      = 12;
const sal_uInt16 EXC_CHFRBLOCK_TYPE_CHART       = 13;
const sal_uInt16 EXC_CHFRBLOCK_TYPE_DATAFORMAT  = 14;
const sal_uInt16 EXC_CHFRBLOCK_TYPE_DROPBAR     = 15;
const sal_uInt16 EXC_CHFRBLOCK_TYPE_UNKNOWN     = 0xFFFF;   /// For internal use only.

const sal_uInt16 EXC_CHFRBLOCK_TEXT_TITLE       = 0;
const sal_uInt16 EXC_CHFRBLOCK_TEXT_DEFTEXT     = 2;
const sal_uInt16 EXC_CHFRBLOCK_TEXT_AXISTITLE   = 4;
const sal_uInt16 EXC_CHFRBLOCK_TEXT_DATALABEL   = 5;

const sal_uInt16 EXC_CHFRBLOCK_FRAME_STANDARD   = 0;
const sal_uInt16 EXC_CHFRBLOCK_FRAME_PLOTFRAME  = 1;
const sal_uInt16 EXC_CHFRBLOCK_FRAME_BACKGROUND = 2;

// (0x086B) CHFRLABELPROPS ----------------------------------------------------

const sal_uInt16 EXC_ID_CHFRLABELPROPS          = 0x086B;

const sal_uInt16 EXC_CHFRLABELPROPS_SHOWSERIES  = 0x0001;
const sal_uInt16 EXC_CHFRLABELPROPS_SHOWCATEG   = 0x0002;
const sal_uInt16 EXC_CHFRLABELPROPS_SHOWVALUE   = 0x0004;
const sal_uInt16 EXC_CHFRLABELPROPS_SHOWPERCENT = 0x0008;
const sal_uInt16 EXC_CHFRLABELPROPS_SHOWBUBBLE  = 0x0010;

// (0x1001) CHUNITS -----------------------------------------------------------

const sal_uInt16 EXC_ID_CHUNITS                 = 0x1001;

const sal_uInt16 EXC_CHUNITS_TWIPS              = 0;
const sal_uInt16 EXC_CHUNITS_PIXELS             = 1;

// (0x1002) CHCHART -----------------------------------------------------------

const sal_uInt16 EXC_ID_CHCHART                 = 0x1002;

// (0x1003) CHSERIES ----------------------------------------------------------

const sal_uInt16 EXC_ID_CHSERIES                = 0x1003;

const sal_uInt16 EXC_CHSERIES_DATE              = 0;
const sal_uInt16 EXC_CHSERIES_NUMERIC           = 1;
const sal_uInt16 EXC_CHSERIES_SEQUENCE          = 2;
const sal_uInt16 EXC_CHSERIES_TEXT              = 3;

const sal_uInt16 EXC_CHSERIES_MAXSERIES         = 255;      /// Maximum valid series index.
const sal_uInt16 EXC_CHSERIES_INVALID           = 0xFFFF;   /// Invalid series index (for internal use).

// (0x1006) CHDATAFORMAT ------------------------------------------------------

const sal_uInt16 EXC_ID_CHDATAFORMAT            = 0x1006;

const sal_uInt16 EXC_CHDATAFORMAT_MAXPOINTCOUNT = 32000;    /// Maximum number of data points.
const sal_uInt16 EXC_CHDATAFORMAT_DEFAULT       = 0xFFFD;   /// As format index: global default for an axes set.
const sal_uInt16 EXC_CHDATAFORMAT_UNKNOWN       = 0xFFFE;   /// As point index: unknown format, don't use.
const sal_uInt16 EXC_CHDATAFORMAT_ALLPOINTS     = 0xFFFF;   /// As point index: default for a series.

const sal_uInt16 EXC_CHDATAFORMAT_OLDCOLORS     = 0x0001;

// (0x1007) CHLINEFORMAT ------------------------------------------------------

const sal_uInt16 EXC_ID_CHLINEFORMAT            = 0x1007;

const sal_uInt16 EXC_CHLINEFORMAT_SOLID         = 0;
const sal_uInt16 EXC_CHLINEFORMAT_DASH          = 1;
const sal_uInt16 EXC_CHLINEFORMAT_DOT           = 2;
const sal_uInt16 EXC_CHLINEFORMAT_DASHDOT       = 3;
const sal_uInt16 EXC_CHLINEFORMAT_DASHDOTDOT    = 4;
const sal_uInt16 EXC_CHLINEFORMAT_NONE          = 5;
const sal_uInt16 EXC_CHLINEFORMAT_DARKTRANS     = 6;
const sal_uInt16 EXC_CHLINEFORMAT_MEDTRANS      = 7;
const sal_uInt16 EXC_CHLINEFORMAT_LIGHTTRANS    = 8;

const sal_Int16 EXC_CHLINEFORMAT_HAIR           = -1;
const sal_Int16 EXC_CHLINEFORMAT_SINGLE         = 0;
const sal_Int16 EXC_CHLINEFORMAT_DOUBLE         = 1;
const sal_Int16 EXC_CHLINEFORMAT_TRIPLE         = 2;

const sal_uInt16 EXC_CHLINEFORMAT_AUTO          = 0x0001;
const sal_uInt16 EXC_CHLINEFORMAT_SHOWAXIS      = 0x0004;

// (0x1009) CHMARKERFORMAT ----------------------------------------------------

const sal_uInt16 EXC_ID_CHMARKERFORMAT          = 0x1009;

const sal_uInt16 EXC_CHMARKERFORMAT_NOSYMBOL    = 0;
const sal_uInt16 EXC_CHMARKERFORMAT_SQUARE      = 1;
const sal_uInt16 EXC_CHMARKERFORMAT_DIAMOND     = 2;
const sal_uInt16 EXC_CHMARKERFORMAT_TRIANGLE    = 3;
const sal_uInt16 EXC_CHMARKERFORMAT_CROSS       = 4;
const sal_uInt16 EXC_CHMARKERFORMAT_STAR        = 5;
const sal_uInt16 EXC_CHMARKERFORMAT_DOWJ        = 6;
const sal_uInt16 EXC_CHMARKERFORMAT_STDDEV      = 7;
const sal_uInt16 EXC_CHMARKERFORMAT_CIRCLE      = 8;
const sal_uInt16 EXC_CHMARKERFORMAT_PLUS        = 9;

const sal_uInt32 EXC_CHMARKERFORMAT_HAIRSIZE    = 60;       /// Automatic symbol size for hair lines.
const sal_uInt32 EXC_CHMARKERFORMAT_SINGLESIZE  = 100;      /// Automatic symbol size for single lines.
const sal_uInt32 EXC_CHMARKERFORMAT_DOUBLESIZE  = 140;      /// Automatic symbol size for double lines.
const sal_uInt32 EXC_CHMARKERFORMAT_TRIPLESIZE  = 180;      /// Automatic symbol size for triple lines.

const sal_uInt16 EXC_CHMARKERFORMAT_AUTO        = 0x0001;
const sal_uInt16 EXC_CHMARKERFORMAT_NOFILL      = 0x0010;
const sal_uInt16 EXC_CHMARKERFORMAT_NOLINE      = 0x0020;

// (0x100A) CHAREAFORMAT ------------------------------------------------------

const sal_uInt16 EXC_ID_CHAREAFORMAT            = 0x100A;

const sal_uInt16 EXC_CHAREAFORMAT_AUTO          = 0x0001;
const sal_uInt16 EXC_CHAREAFORMAT_INVERTNEG     = 0x0002;

// (0x100B) CHPIEFORMAT -------------------------------------------------------

const sal_uInt16 EXC_ID_CHPIEFORMAT             = 0x100B;

// (0x100C) CHATTACHEDLABEL ---------------------------------------------------

const sal_uInt16 EXC_ID_CHATTACHEDLABEL         = 0x100C;

const sal_uInt16 EXC_CHATTLABEL_SHOWVALUE       = 0x0001;
const sal_uInt16 EXC_CHATTLABEL_SHOWPERCENT     = 0x0002;
const sal_uInt16 EXC_CHATTLABEL_SHOWCATEGPERC   = 0x0004;
const sal_uInt16 EXC_CHATTLABEL_SMOOTHED        = 0x0008;   /// Smoothed line.
const sal_uInt16 EXC_CHATTLABEL_SHOWCATEG       = 0x0010;
const sal_uInt16 EXC_CHATTLABEL_SHOWBUBBLE      = 0x0020;

// (0x100D) CHSTRING ----------------------------------------------------------

const sal_uInt16 EXC_ID_CHSTRING                = 0x100D;

// (0x1014) CHTYPEGROUP -------------------------------------------------------

const sal_uInt16 EXC_ID_CHTYPEGROUP             = 0x1014;

const sal_uInt16 EXC_CHTYPEGROUP_VARIEDCOLORS   = 0x0001;   /// Varied colors for points.

// (0x1015) CHLEGEND ----------------------------------------------------------

const sal_uInt16 EXC_ID_CHLEGEND                = 0x1015;

const sal_uInt8 EXC_CHLEGEND_BOTTOM             = 0;
const sal_uInt8 EXC_CHLEGEND_CORNER             = 1;
const sal_uInt8 EXC_CHLEGEND_TOP                = 2;
const sal_uInt8 EXC_CHLEGEND_RIGHT              = 3;
const sal_uInt8 EXC_CHLEGEND_LEFT               = 4;
const sal_uInt8 EXC_CHLEGEND_NOTDOCKED          = 7;

const sal_uInt8 EXC_CHLEGEND_CLOSE              = 0;
const sal_uInt8 EXC_CHLEGEND_MEDIUM             = 1;
const sal_uInt8 EXC_CHLEGEND_OPEN               = 2;

const sal_uInt16 EXC_CHLEGEND_DOCKED            = 0x0001;
const sal_uInt16 EXC_CHLEGEND_AUTOSERIES        = 0x0002;
const sal_uInt16 EXC_CHLEGEND_AUTOPOSX          = 0x0004;
const sal_uInt16 EXC_CHLEGEND_AUTOPOSY          = 0x0008;
const sal_uInt16 EXC_CHLEGEND_STACKED           = 0x0010;
const sal_uInt16 EXC_CHLEGEND_DATATABLE         = 0x0020;

// (0x1017) CHBAR, CHCOLUMN ---------------------------------------------------

const sal_uInt16 EXC_ID_CHBAR                   = 0x1017;

const sal_uInt16 EXC_CHBAR_HORIZONTAL           = 0x0001;
const sal_uInt16 EXC_CHBAR_STACKED              = 0x0002;
const sal_uInt16 EXC_CHBAR_PERCENT              = 0x0004;
const sal_uInt16 EXC_CHBAR_SHADOW               = 0x0008;

// (0x1018, 0x101A) CHLINE, CHAREA --------------------------------------------

const sal_uInt16 EXC_ID_CHLINE                  = 0x1018;
const sal_uInt16 EXC_ID_CHAREA                  = 0x101A;

const sal_uInt16 EXC_CHLINE_STACKED             = 0x0001;
const sal_uInt16 EXC_CHLINE_PERCENT             = 0x0002;
const sal_uInt16 EXC_CHLINE_SHADOW              = 0x0004;

// (0x1019) CHPIE -------------------------------------------------------------

const sal_uInt16 EXC_ID_CHPIE                   = 0x1019;

const sal_uInt16 EXC_CHPIE_SHADOW               = 0x0001;
const sal_uInt16 EXC_CHPIE_LINES                = 0x0002;

// (0x101B) CHSCATTER ---------------------------------------------------------

const sal_uInt16 EXC_ID_CHSCATTER               = 0x101B;

const sal_uInt16 EXC_CHSCATTER_AREA             = 1;        /// Bubble area refers to value.
const sal_uInt16 EXC_CHSCATTER_WIDTH            = 2;        /// Bubble width refers to value.

const sal_uInt16 EXC_CHSCATTER_BUBBLES          = 0x0001;
const sal_uInt16 EXC_CHSCATTER_SHOWNEG          = 0x0002;
const sal_uInt16 EXC_CHSCATTER_SHADOW           = 0x0004;

// (0x001C) CHCHARTLINE -------------------------------------------------------

const sal_uInt16 EXC_ID_CHCHARTLINE             = 0x101C;

const sal_uInt16 EXC_CHCHARTLINE_DROP           = 0;        /// Drop lines.
const sal_uInt16 EXC_CHCHARTLINE_HILO           = 1;        /// Hi-lo lines.
const sal_uInt16 EXC_CHCHARTLINE_CONNECT        = 2;        /// Connector lines in stacked bar charts.

// (0x101D) CHAXIS ------------------------------------------------------------

const sal_uInt16 EXC_ID_CHAXIS                  = 0x101D;

const sal_uInt16 EXC_CHAXIS_X                   = 0;
const sal_uInt16 EXC_CHAXIS_Y                   = 1;
const sal_uInt16 EXC_CHAXIS_Z                   = 2;
const sal_uInt16 EXC_CHAXIS_NONE                = 0xFFFF;   /// For internal use only.

// (0x101E) CHTICK ------------------------------------------------------------

const sal_uInt16 EXC_ID_CHTICK                  = 0x101E;

const sal_uInt8 EXC_CHTICK_INSIDE               = 0x01;
const sal_uInt8 EXC_CHTICK_OUTSIDE              = 0x02;

const sal_uInt8 EXC_CHTICK_NOLABEL              = 0;
const sal_uInt8 EXC_CHTICK_LOW                  = 1;        /// Below diagram/right of diagram.
const sal_uInt8 EXC_CHTICK_HIGH                 = 2;        /// Above diagram/left of diagram.
const sal_uInt8 EXC_CHTICK_NEXT                 = 3;        /// Next to axis.

const sal_uInt8 EXC_CHTICK_TRANSPARENT          = 1;
const sal_uInt8 EXC_CHTICK_OPAQUE               = 2;

const sal_uInt16 EXC_CHTICK_AUTOCOLOR           = 0x0001;
const sal_uInt16 EXC_CHTICK_AUTOFILL            = 0x0002;
const sal_uInt16 EXC_CHTICK_AUTOROT             = 0x0020;

// (0x101F) CHVALUERANGE ------------------------------------------------------

const sal_uInt16 EXC_ID_CHVALUERANGE            = 0x101F;

const sal_uInt16 EXC_CHVALUERANGE_AUTOMIN       = 0x0001;
const sal_uInt16 EXC_CHVALUERANGE_AUTOMAX       = 0x0002;
const sal_uInt16 EXC_CHVALUERANGE_AUTOMAJOR     = 0x0004;
const sal_uInt16 EXC_CHVALUERANGE_AUTOMINOR     = 0x0008;
const sal_uInt16 EXC_CHVALUERANGE_AUTOCROSS     = 0x0010;
const sal_uInt16 EXC_CHVALUERANGE_LOGSCALE      = 0x0020;
const sal_uInt16 EXC_CHVALUERANGE_REVERSE       = 0x0040;   /// Axis direction reversed.
const sal_uInt16 EXC_CHVALUERANGE_MAXCROSS      = 0x0080;   /// Other axis crosses at own maximum.
const sal_uInt16 EXC_CHVALUERANGE_BIT8          = 0x0100;   /// This bit is always set in BIFF5+.

// (0x1020) CHLABELRANGE -----------------------------------------------------

const sal_uInt16 EXC_ID_CHLABELRANGE            = 0x1020;

const sal_uInt16 EXC_CHLABELRANGE_BETWEEN       = 0x0001;   /// Axis between categories.
const sal_uInt16 EXC_CHLABELRANGE_MAXCROSS      = 0x0002;   /// Other axis crosses at own maximum.
const sal_uInt16 EXC_CHLABELRANGE_REVERSE       = 0x0004;   /// Axis direction reversed.

// (0x1021) CHAXISLINE --------------------------------------------------------

const sal_uInt16 EXC_ID_CHAXISLINE              = 0x1021;

const sal_uInt16 EXC_CHAXISLINE_AXISLINE        = 0;        /// Axis line itself.
const sal_uInt16 EXC_CHAXISLINE_MAJORGRID       = 1;        /// Major grid line.
const sal_uInt16 EXC_CHAXISLINE_MINORGRID       = 2;        /// Minor grid line.
const sal_uInt16 EXC_CHAXISLINE_WALLS           = 3;        /// Walls (X, Z axis), floor (Y axis).

// (0x1024) CHDEFAULTTEXT -----------------------------------------------------

const sal_uInt16 EXC_ID_CHDEFAULTTEXT           = 0x1024;

const sal_uInt16 EXC_CHDEFTEXT_TEXTLABEL        = 0;        /// Default for text data labels (not used?).
const sal_uInt16 EXC_CHDEFTEXT_NUMLABEL         = 1;        /// Default for numeric data labels (not used?).
const sal_uInt16 EXC_CHDEFTEXT_GLOBAL           = 2;        /// Default text for all chart objects.
const sal_uInt16 EXC_CHDEFTEXT_AXESSET          = 3;        /// Default text for axes and data points (BIFF8 only).
const sal_uInt16 EXC_CHDEFTEXT_NONE             = 0xFFFF;   /// No default text available.

// (0x1025) CHTEXT ------------------------------------------------------------

const sal_uInt16 EXC_ID_CHTEXT                  = 0x1025;

const sal_uInt8 EXC_CHTEXT_ALIGN_TOPLEFT        = 1;        /// Horizontal: left, vertical: top.
const sal_uInt8 EXC_CHTEXT_ALIGN_CENTER         = 2;
const sal_uInt8 EXC_CHTEXT_ALIGN_BOTTOMRIGHT    = 3;        /// Horizontal: right, vertical: bottom.
const sal_uInt8 EXC_CHTEXT_ALIGN_JUSTIFY        = 4;
const sal_uInt8 EXC_CHTEXT_ALIGN_DISTRIBUTE     = 5;

const sal_uInt16 EXC_CHTEXT_TRANSPARENT         = 1;
const sal_uInt16 EXC_CHTEXT_OPAQUE              = 2;

const sal_uInt16 EXC_CHTEXT_AUTOCOLOR           = 0x0001;   /// Automatic text color.
const sal_uInt16 EXC_CHTEXT_SHOWSYMBOL          = 0x0002;   /// Legend symbol for data point caption.
const sal_uInt16 EXC_CHTEXT_SHOWVALUE           = 0x0004;   /// Data point caption is the value.
const sal_uInt16 EXC_CHTEXT_VERTICAL            = 0x0008;
const sal_uInt16 EXC_CHTEXT_AUTOTEXT            = 0x0010;   /// Label text generated from chart data.
const sal_uInt16 EXC_CHTEXT_AUTOGEN             = 0x0020;   /// Text object is inserted automatically.
const sal_uInt16 EXC_CHTEXT_DELETED             = 0x0040;   /// Text object is removed.
const sal_uInt16 EXC_CHTEXT_AUTOFILL            = 0x0080;   /// Automatic text background mode (transparent/opaque).
const sal_uInt16 EXC_CHTEXT_SHOWCATEGPERC       = 0x0800;   /// Data point caption is category and percent.
const sal_uInt16 EXC_CHTEXT_SHOWPERCENT         = 0x1000;   /// Data point caption as percent.
const sal_uInt16 EXC_CHTEXT_SHOWBUBBLE          = 0x2000;   /// Show bubble size.
const sal_uInt16 EXC_CHTEXT_SHOWCATEG           = 0x4000;   /// Data point caption is category name.

const sal_uInt16 EXC_CHTEXT_POS_DEFAULT         = 0;
const sal_uInt16 EXC_CHTEXT_POS_OUTSIDE         = 1;
const sal_uInt16 EXC_CHTEXT_POS_INSIDE          = 2;
const sal_uInt16 EXC_CHTEXT_POS_CENTER          = 3;
const sal_uInt16 EXC_CHTEXT_POS_AXIS            = 4;
const sal_uInt16 EXC_CHTEXT_POS_ABOVE           = 5;
const sal_uInt16 EXC_CHTEXT_POS_BELOW           = 6;
const sal_uInt16 EXC_CHTEXT_POS_LEFT            = 7;
const sal_uInt16 EXC_CHTEXT_POS_RIGHT           = 8;
const sal_uInt16 EXC_CHTEXT_POS_AUTO            = 9;
const sal_uInt16 EXC_CHTEXT_POS_MOVED           = 10;

// (0x1026) CHFONT ------------------------------------------------------------

const sal_uInt16 EXC_ID_CHFONT                  = 0x1026;

// (0x1027) CHOBJECTLINK ------------------------------------------------------

const sal_uInt16 EXC_ID_CHOBJECTLINK            = 0x1027;

// link targets
const sal_uInt16 EXC_CHOBJLINK_NONE             = 0;        /// No link target.
const sal_uInt16 EXC_CHOBJLINK_TITLE            = 1;        /// Chart title.
const sal_uInt16 EXC_CHOBJLINK_YAXIS            = 2;        /// Value axis (Y axis).
const sal_uInt16 EXC_CHOBJLINK_XAXIS            = 3;        /// Category axis (X axis).
const sal_uInt16 EXC_CHOBJLINK_DATA             = 4;        /// Data series/point.
const sal_uInt16 EXC_CHOBJLINK_ZAXIS            = 7;        /// Series axis (Z axis).
const sal_uInt16 EXC_CHOBJLINK_AXISUNIT         = 12;       /// Unit name for axis labels.

// (0x1032) CHFRAME -----------------------------------------------------------

const sal_uInt16 EXC_ID_CHFRAME                 = 0x1032;

const sal_uInt16 EXC_CHFRAME_STANDARD           = 0;
const sal_uInt16 EXC_CHFRAME_SHADOW             = 4;

const sal_uInt16 EXC_CHFRAME_AUTOSIZE           = 0x0001;
const sal_uInt16 EXC_CHFRAME_AUTOPOS            = 0x0002;

// (0x1033, 0x1034) CHBEGIN, CHEND --------------------------------------------

const sal_uInt16 EXC_ID_CHBEGIN                 = 0x1033;
const sal_uInt16 EXC_ID_CHEND                   = 0x1034;

// (0x1035) CHPLOTFRAME -------------------------------------------------------

const sal_uInt16 EXC_ID_CHPLOTFRAME             = 0x1035;

// (0x103A) CHCHART3D ---------------------------------------------------------

const sal_uInt16 EXC_ID_CHCHART3D               = 0x103A;

const sal_uInt16 EXC_CHCHART3D_REAL3D           = 0x0001;   /// true = real 3d perspective.
const sal_uInt16 EXC_CHCHART3D_CLUSTER          = 0x0002;   /// false = Z axis, true = clustered/stacked.
const sal_uInt16 EXC_CHCHART3D_AUTOHEIGHT       = 0x0004;   /// true = automatic height to width ratio.
const sal_uInt16 EXC_CHCHART3D_HASWALLS         = 0x0010;   /// true = 3d chart has walls/floor.
const sal_uInt16 EXC_CHCHART3D_2DWALLS          = 0x0020;   /// true = 2d wall/gridlines, no floor.

// (0x103C) CHPICFORMAT -------------------------------------------------------

const sal_uInt16 EXC_ID_CHPICFORMAT             = 0x103C;

const sal_uInt16 EXC_CHPICFORMAT_NONE           = 0;        /// For internal use only.
const sal_uInt16 EXC_CHPICFORMAT_STRETCH        = 1;        /// Bitmap stretched to area.
const sal_uInt16 EXC_CHPICFORMAT_STACK          = 2;        /// Bitmap stacked.
const sal_uInt16 EXC_CHPICFORMAT_SCALE          = 3;        /// Bitmap scaled to axis scale.

const sal_uInt16 EXC_CHPICFORMAT_TOPBOTTOM      = 0x0200;
const sal_uInt16 EXC_CHPICFORMAT_FRONTBACK      = 0x0400;
const sal_uInt16 EXC_CHPICFORMAT_LEFTRIGHT      = 0x0800;

// (0x103D) CHDROPBAR ---------------------------------------------------------

const sal_uInt16 EXC_ID_CHDROPBAR               = 0x103D;

const sal_uInt16 EXC_CHDROPBAR_UP               = 0;
const sal_uInt16 EXC_CHDROPBAR_DOWN             = 1;
const sal_uInt16 EXC_CHDROPBAR_NONE             = 0xFFFF;

// (0x103E, 0x1040) CHRADARLINE, CHRADARAREA ----------------------------------

const sal_uInt16 EXC_ID_CHRADARLINE             = 0x103E;
const sal_uInt16 EXC_ID_CHRADARAREA             = 0x1040;

const sal_uInt16 EXC_CHRADAR_AXISLABELS         = 0x0001;
const sal_uInt16 EXC_CHRADAR_SHADOW             = 0x0002;

// (0x103F) CHSURFACE ---------------------------------------------------------

const sal_uInt16 EXC_ID_CHSURFACE               = 0x103F;

const sal_uInt16 EXC_CHSURFACE_FILLED           = 0x0001;
const sal_uInt16 EXC_CHSURFACE_SHADING          = 0x0002;

// (0x1041) CHAXESSET ---------------------------------------------------------

const sal_uInt16 EXC_ID_CHAXESSET               = 0x1041;

const sal_uInt16 EXC_CHAXESSET_PRIMARY          = 0;
const sal_uInt16 EXC_CHAXESSET_SECONDARY        = 1;
const sal_uInt16 EXC_CHAXESSET_NONE             = 0xFFFF;   /// For internal use.

// (0x1044) CHPROPERTIES ------------------------------------------------------

const sal_uInt16 EXC_ID_CHPROPERTIES            = 0x1044;

const sal_uInt16 EXC_CHPROPS_MANSERIES          = 0x0001;   /// Manual series allocation.
const sal_uInt16 EXC_CHPROPS_SHOWVISIBLEONLY    = 0x0002;   /// Show visible cells only.
const sal_uInt16 EXC_CHPROPS_NORESIZE           = 0x0004;   /// Do not resize chart with window.
const sal_uInt16 EXC_CHPROPS_MANPLOTAREA        = 0x0008;   /// Manual plot area mode.
const sal_uInt16 EXC_CHPROPS_USEMANPLOTAREA     = 0x0010;   /// Manual plot area layout in CHFRAMEPOS record.

const sal_uInt8 EXC_CHPROPS_EMPTY_SKIP          = 0;        /// Skip empty values.
const sal_uInt8 EXC_CHPROPS_EMPTY_ZERO          = 1;        /// Plot empty values as zero.
const sal_uInt8 EXC_CHPROPS_EMPTY_INTERPOLATE   = 2;        /// Interpolate empty values.

// (0x1045) CHSERGROUP --------------------------------------------------------

const sal_uInt16 EXC_ID_CHSERGROUP              = 0x1045;

const sal_uInt16 EXC_CHSERGROUP_NONE            = 0xFFFF;   /// For internal use: no chart type group.

// (0x1048, 0x0858) CHPIVOTREF ------------------------------------------------

const sal_uInt16 EXC_ID5_CHPIVOTREF             = 0x1048;
const sal_uInt16 EXC_ID8_CHPIVOTREF             = 0x0858;

// (0x104A) CHSERPARENT -------------------------------------------------------

const sal_uInt16 EXC_ID_CHSERPARENT             = 0x104A;

// (0x104B) CHSERTRENDLINE ----------------------------------------------------

const sal_uInt16 EXC_ID_CHSERTRENDLINE          = 0x104B;

const sal_uInt8 EXC_CHSERTREND_POLYNOMIAL       = 0;    /// If order is 1, trend line is linear.
const sal_uInt8 EXC_CHSERTREND_EXPONENTIAL      = 1;
const sal_uInt8 EXC_CHSERTREND_LOGARITHMIC      = 2;
const sal_uInt8 EXC_CHSERTREND_POWER            = 3;
const sal_uInt8 EXC_CHSERTREND_MOVING_AVG       = 4;

// (0x104E) CHFORMAT ----------------------------------------------------------

const sal_uInt16 EXC_ID_CHFORMAT                = 0x104E;

// (0x104F) CHFRAMEPOS --------------------------------------------------------

const sal_uInt16 EXC_ID_CHFRAMEPOS              = 0x104F;

const sal_uInt16 EXC_CHFRAMEPOS_POINTS          = 0;
const sal_uInt16 EXC_CHFRAMEPOS_ABSSIZE_POINTS  = 1;
const sal_uInt16 EXC_CHFRAMEPOS_PARENT          = 2;
const sal_uInt16 EXC_CHFRAMEPOS_DEFOFFSET_PLOT  = 3;
const sal_uInt16 EXC_CHFRAMEPOS_CHARTSIZE       = 5;

// (0x1050) CHFORMATRUNS ------------------------------------------------------

const sal_uInt16 EXC_ID_CHFORMATRUNS            = 0x1050;

// (0x1051) CHSOURCELINK ------------------------------------------------------

const sal_uInt16 EXC_ID_CHSOURCELINK            = 0x1051;

const sal_uInt8 EXC_CHSRCLINK_TITLE             = 0;
const sal_uInt8 EXC_CHSRCLINK_VALUES            = 1;
const sal_uInt8 EXC_CHSRCLINK_CATEGORY          = 2;
const sal_uInt8 EXC_CHSRCLINK_BUBBLES           = 3;

const sal_uInt8 EXC_CHSRCLINK_DEFAULT           = 0;
const sal_uInt8 EXC_CHSRCLINK_DIRECTLY          = 1;
const sal_uInt8 EXC_CHSRCLINK_WORKSHEET         = 2;

const sal_uInt16 EXC_CHSRCLINK_NUMFMT           = 0x0001;

// (0x105B) CHSERERRORBAR -----------------------------------------------------

const sal_uInt16 EXC_ID_CHSERERRORBAR           = 0x105B;

const sal_uInt8 EXC_CHSERERR_NONE               = 0;    /// For internal use only.
const sal_uInt8 EXC_CHSERERR_XPLUS              = 1;
const sal_uInt8 EXC_CHSERERR_XMINUS             = 2;
const sal_uInt8 EXC_CHSERERR_YPLUS              = 3;
const sal_uInt8 EXC_CHSERERR_YMINUS             = 4;

const sal_uInt8 EXC_CHSERERR_PERCENT            = 1;
const sal_uInt8 EXC_CHSERERR_FIXED              = 2;
const sal_uInt8 EXC_CHSERERR_STDDEV             = 3;
const sal_uInt8 EXC_CHSERERR_CUSTOM             = 4;
const sal_uInt8 EXC_CHSERERR_STDERR             = 5;

const sal_uInt8 EXC_CHSERERR_END_BLANK          = 0;    /// Line end: blank.
const sal_uInt8 EXC_CHSERERR_END_TSHAPE         = 1;    /// Line end: t-shape.

// (0x105D) CHSERIESFORMAT ----------------------------------------------------

const sal_uInt16 EXC_ID_CHSERIESFORMAT          = 0x105D;

const sal_uInt16 EXC_CHSERIESFORMAT_SMOOTHED    = 0x0001;
const sal_uInt16 EXC_CHSERIESFORMAT_BUBBLE3D    = 0x0002;
const sal_uInt16 EXC_CHSERIESFORMAT_SHADOW      = 0x0004;

// (0x105F) CH3DDATAFORMAT ----------------------------------------------------

const sal_uInt16 EXC_ID_CH3DDATAFORMAT          = 0x105F;

const sal_uInt8 EXC_CH3DDATAFORMAT_RECT         = 0;        /// Rectangular base.
const sal_uInt8 EXC_CH3DDATAFORMAT_CIRC         = 1;        /// Circular base.

const sal_uInt8 EXC_CH3DDATAFORMAT_STRAIGHT     = 0;        /// Straight to top.
const sal_uInt8 EXC_CH3DDATAFORMAT_SHARP        = 1;        /// Sharp top.
const sal_uInt8 EXC_CH3DDATAFORMAT_TRUNC        = 2;        /// Shart top, truncated.

// (0x1061) CHPIEEXT ----------------------------------------------------------

const sal_uInt16 EXC_ID_CHPIEEXT                = 0x1061;

// (0x1062) CHDATERANGE -------------------------------------------------------

const sal_uInt16 EXC_ID_CHDATERANGE             = 0x1062;

const sal_uInt16 EXC_CHDATERANGE_AUTOMIN        = 0x0001;
const sal_uInt16 EXC_CHDATERANGE_AUTOMAX        = 0x0002;
const sal_uInt16 EXC_CHDATERANGE_AUTOMAJOR      = 0x0004;
const sal_uInt16 EXC_CHDATERANGE_AUTOMINOR      = 0x0008;
const sal_uInt16 EXC_CHDATERANGE_DATEAXIS       = 0x0010;
const sal_uInt16 EXC_CHDATERANGE_AUTOBASE       = 0x0020;
const sal_uInt16 EXC_CHDATERANGE_AUTOCROSS      = 0x0040;   /// Other axis crosses at own maximum.
const sal_uInt16 EXC_CHDATERANGE_AUTODATE       = 0x0080;   /// Recognize date/text automatically.

const sal_uInt16 EXC_CHDATERANGE_DAYS           = 0;
const sal_uInt16 EXC_CHDATERANGE_MONTHS         = 1;
const sal_uInt16 EXC_CHDATERANGE_YEARS          = 2;

// (0x1066) CHESCHERFORMAT ----------------------------------------------------

const sal_uInt16 EXC_ID_CHESCHERFORMAT          = 0x1066;

// Other record IDs -----------------------------------------------------------

const sal_uInt16 EXC_ID_CHWRAPPEDRECORD         = 0x0851;
const sal_uInt16 EXC_ID_CHUNITPROPERTIES        = 0x0857;
const sal_uInt16 EXC_ID_CHUSEDAXESSETS          = 0x1046;
const sal_uInt16 EXC_ID_CHLABELRANGE2           = 0x1062;
const sal_uInt16 EXC_ID_CHPLOTGROWTH            = 0x1064;
const sal_uInt16 EXC_ID_CHSERINDEX              = 0x1065;
const sal_uInt16 EXC_ID_CHUNKNOWN               = 0xFFFF;

// ============================================================================
// Structs and classes
// ============================================================================

// Common =====================================================================

struct XclChRectangle
{
    sal_Int32           mnX;                /// X position of the object in 1/4000 of chart width.
    sal_Int32           mnY;                /// Y position of the object in 1/4000 of chart height.
    sal_Int32           mnWidth;            /// Width of the object in 1/4000 of chart width.
    sal_Int32           mnHeight;           /// Height of the object in 1/4000 of chart height.

    explicit            XclChRectangle();
};

// ----------------------------------------------------------------------------

/** Specifies the position of a data series or data point. */
struct XclChDataPointPos
{
    sal_uInt16          mnSeriesIdx;        /// Series index of series or a data point.
    sal_uInt16          mnPointIdx;         /// Index of a data point inside a series.

    explicit            XclChDataPointPos(
                            sal_uInt16 nSeriesIdx = EXC_CHSERIES_INVALID,
                            sal_uInt16 nPointIdx = EXC_CHDATAFORMAT_ALLPOINTS );
};

bool operator<( const XclChDataPointPos& rL, const XclChDataPointPos& rR );

// ----------------------------------------------------------------------------

/** Contains the type and context of a block of future records which are
    guarded by CHFRBLOCKBEGIN and CHFRBLOCKEND records. */
struct XclChFrBlock
{
    sal_uInt16          mnType;             /// Type of the future record block.
    sal_uInt16          mnContext;          /// Context dependent on type.
    sal_uInt16          mnValue1;           /// Optional primary value for current context.
    sal_uInt16          mnValue2;           /// Optional secondary value for current context.

    explicit            XclChFrBlock( sal_uInt16 nType );
};

// Frame formatting ===========================================================

struct XclChFramePos
{
    XclChRectangle      maRect;             /// Object dependent position data.
    sal_uInt16          mnTLMode;           /// Top-left position mode.
    sal_uInt16          mnBRMode;           /// Bottom-right position mode.

    explicit            XclChFramePos();
};

// ----------------------------------------------------------------------------

struct XclChLineFormat
{
    Color               maColor;            /// Line color.
    sal_uInt16          mnPattern;          /// Line pattern (solid, dashed, ...).
    sal_Int16           mnWeight;           /// Line weight (hairline, single, ...).
    sal_uInt16          mnFlags;            /// Additional flags.

    explicit            XclChLineFormat();
};

// ----------------------------------------------------------------------------

struct XclChAreaFormat
{
    Color               maPattColor;        /// Pattern color.
    Color               maBackColor;        /// Pattern background color.
    sal_uInt16          mnPattern;          /// Fill pattern.
    sal_uInt16          mnFlags;            /// Additional flags.

    explicit            XclChAreaFormat();
};

// ----------------------------------------------------------------------------

class SfxItemSet;
class EscherPropertyContainer;

struct XclChEscherFormat
{
    typedef boost::shared_ptr< SfxItemSet >                SfxItemSetRef;
    typedef boost::shared_ptr< EscherPropertyContainer >   EscherPropSetRef;

    SfxItemSetRef       mxItemSet;          /// Item set for Escher properties import.
    EscherPropSetRef    mxEscherSet;        /// Container for Escher properties export.

    explicit            XclChEscherFormat();
                        ~XclChEscherFormat();
};

// ----------------------------------------------------------------------------

struct XclChPicFormat
{
    sal_uInt16          mnBmpMode;          /// Bitmap mode, e.g. stretched, stacked.
    sal_uInt16          mnFlags;            /// Additional flags.
    double              mfScale;            /// Picture scaling (units).

    explicit            XclChPicFormat();
};

// ----------------------------------------------------------------------------

struct XclChFrame
{
    sal_uInt16          mnFormat;           /// Format type of the frame.
    sal_uInt16          mnFlags;            /// Additional flags.

    explicit            XclChFrame();
};

// Source links ===============================================================

struct XclChSourceLink
{
    sal_uInt8           mnDestType;         /// Type of the destination (title, values, ...).
    sal_uInt8           mnLinkType;         /// Link type (directly, linked to worksheet, ...).
    sal_uInt16          mnFlags;            /// Additional flags.
    sal_uInt16          mnNumFmtIdx;        /// Number format index.

    explicit            XclChSourceLink();
};

// Text =======================================================================

struct XclChObjectLink
{
    XclChDataPointPos   maPointPos;         /// Position of the data point.
    sal_uInt16          mnTarget;           /// Target of the link.

    explicit            XclChObjectLink();
};

// ----------------------------------------------------------------------------

struct XclChFrLabelProps
{
    rtl::OUString       maSeparator;        /// Separator between label values.
    sal_uInt16          mnFlags;            /// Flags indicating which values to be displayed.

    explicit            XclChFrLabelProps();
};

// ----------------------------------------------------------------------------

struct XclChText
{
    XclChRectangle      maRect;             /// Position of the text object.
    Color               maTextColor;        /// Text color.
    sal_uInt8           mnHAlign;           /// Horizontal alignment.
    sal_uInt8           mnVAlign;           /// Vertical alignment.
    sal_uInt16          mnBackMode;         /// Background mode: transparent, opaque.
    sal_uInt16          mnFlags;            /// Additional flags.
    sal_uInt16          mnFlags2;           /// Text object placement and text direction (BIFF8+).
    sal_uInt16          mnRotation;         /// Text object rotation (BIFF8+).

    explicit            XclChText();
};

// Data series ================================================================

struct XclChMarkerFormat
{
    Color               maLineColor;        /// Border line color.
    Color               maFillColor;        /// Fill color.
    sal_uInt32          mnMarkerSize;       /// Size of a marker
    sal_uInt16          mnMarkerType;       /// Marker type (none, diamond, ...).
    sal_uInt16          mnFlags;            /// Additional flags.

    explicit            XclChMarkerFormat();
};

// ----------------------------------------------------------------------------

struct XclCh3dDataFormat
{
    sal_uInt8           mnBase;             /// Base form.
    sal_uInt8           mnTop;              /// Top egde mode.

    explicit            XclCh3dDataFormat();
};

// ----------------------------------------------------------------------------

struct XclChDataFormat
{
    XclChDataPointPos   maPointPos;         /// Position of the data point or series.
    sal_uInt16          mnFormatIdx;        /// Formatting index for automatic colors.
    sal_uInt16          mnFlags;            /// Additional flags.

    explicit            XclChDataFormat();
};

// ----------------------------------------------------------------------------

struct XclChSerTrendLine
{
    double              mfIntercept;        /// Forced intercept.
    double              mfForecastFor;      /// Counter to forecast forward.
    double              mfForecastBack;     /// Counter to forecast backward.
    sal_uInt8           mnLineType;         /// Type of the trend line.
    sal_uInt8           mnOrder;            /// Polynomial order or moving average counter.
    sal_uInt8           mnShowEquation;     /// 1 = Show equation.
    sal_uInt8           mnShowRSquared;     /// 1 = Show R-squared.

    explicit            XclChSerTrendLine();
};

// ----------------------------------------------------------------------------

struct XclChSerErrorBar
{
    double              mfValue;            /// Fixed value for several source types.
    sal_uInt16          mnValueCount;       /// Number of custom error values.
    sal_uInt8           mnBarType;          /// Type of the error bar (X/Y).
    sal_uInt8           mnSourceType;       /// Type of source values.
    sal_uInt8           mnLineEnd;          /// Type of the line ends.

    explicit            XclChSerErrorBar();
};

// ----------------------------------------------------------------------------

struct XclChSeries
{
    sal_uInt16          mnCategType;        /// Data type for category entries.
    sal_uInt16          mnValueType;        /// Data type for value entries.
    sal_uInt16          mnBubbleType;       /// Data type for bubble entries.
    sal_uInt16          mnCategCount;       /// Number of category entries.
    sal_uInt16          mnValueCount;       /// Number of value entries.
    sal_uInt16          mnBubbleCount;      /// Number of bubble entries.

    explicit            XclChSeries();
};

// Chart type groups ==========================================================

struct XclChType
{
    sal_Int16           mnOverlap;          /// Bar overlap width (CHBAR).
    sal_Int16           mnGap;              /// Gap between bars (CHBAR).
    sal_uInt16          mnRotation;         /// Rotation angle of first pie (CHPIE).
    sal_uInt16          mnPieHole;          /// Hole size in donut chart (CHPIE).
    sal_uInt16          mnBubbleSize;       /// Bubble size in bubble chart (CHSCATTER).
    sal_uInt16          mnBubbleType;       /// Bubble type in bubble chart (CHSCATTER).
    sal_uInt16          mnFlags;            /// Additional flags (all chart types).

    explicit            XclChType();
};

// ----------------------------------------------------------------------------

struct XclChChart3d
{
    sal_uInt16          mnRotation;         /// Rotation (0...359deg).
    sal_Int16           mnElevation;        /// Elevation (-90...+90deg).
    sal_uInt16          mnEyeDist;          /// Eye distance to chart (0...100).
    sal_uInt16          mnRelHeight;        /// Height relative to width.
    sal_uInt16          mnRelDepth;         /// Depth relative to width.
    sal_uInt16          mnDepthGap;         /// Space between series.
    sal_uInt16          mnFlags;            /// Additional flags.

    explicit            XclChChart3d();
};

// ----------------------------------------------------------------------------

struct XclChLegend
{
    XclChRectangle      maRect;             /// Position of the legend.
    sal_uInt8           mnDockMode;         /// Docking mode.
    sal_uInt8           mnSpacing;          /// Spacing between elements.
    sal_uInt16          mnFlags;            /// Additional flags.

    explicit            XclChLegend();
};

// ----------------------------------------------------------------------------

struct XclChTypeGroup
{
    sal_uInt16          mnFlags;            /// Additional flags.
    sal_uInt16          mnGroupIdx;         /// Chart type group index.

    explicit            XclChTypeGroup();
};

// ----------------------------------------------------------------------------

struct XclChProperties
{
    sal_uInt16          mnFlags;            /// Additional flags.
    sal_uInt8           mnEmptyMode;        /// Plotting mode for empty cells.

    explicit            XclChProperties();
};

// Axes =======================================================================

struct XclChLabelRange
{
    sal_uInt16          mnCross;            /// Crossing position of other axis.
    sal_uInt16          mnLabelFreq;        /// Frequency of labels.
    sal_uInt16          mnTickFreq;         /// Frequency of ticks.
    sal_uInt16          mnFlags;            /// Additional flags.

    explicit            XclChLabelRange();
};

// ----------------------------------------------------------------------------

struct XclChDateRange
{
    sal_uInt16          mnMinDate;          /// Minimum value on axis.
    sal_uInt16          mnMaxDate;          /// Maximum value on axis.
    sal_uInt16          mnMajorStep;        /// Distance for major grid lines.
    sal_uInt16          mnMajorUnit;        /// Time unit for major step.
    sal_uInt16          mnMinorStep;        /// Distance for minor grid lines.
    sal_uInt16          mnMinorUnit;        /// Time unit for minor step.
    sal_uInt16          mnBaseUnit;         /// Time unit for axis values.
    sal_uInt16          mnCross;            /// Crossing position of other axis.
    sal_uInt16          mnFlags;            /// Additional flags.

    explicit            XclChDateRange();
};

// ----------------------------------------------------------------------------

struct XclChValueRange
{
    double              mfMin;              /// Minimum value on axis.
    double              mfMax;              /// Maximum value on axis.
    double              mfMajorStep;        /// Distance for major grid lines.
    double              mfMinorStep;        /// Distance for minor grid lines.
    double              mfCross;            /// Crossing position of other axis.
    sal_uInt16          mnFlags;            /// Additional flags.

    explicit            XclChValueRange();
};

// ----------------------------------------------------------------------------

struct XclChTick
{
    Color               maTextColor;        /// Tick labels color.
    sal_uInt8           mnMajor;            /// Type of tick marks of major grid.
    sal_uInt8           mnMinor;            /// Type of tick marks of minor grid.
    sal_uInt8           mnLabelPos;         /// Position of labels relative to axis.
    sal_uInt8           mnBackMode;         /// Background mode: transparent, opaque.
    sal_uInt16          mnFlags;            /// Additional flags.
    sal_uInt16          mnRotation;         /// Tick labels angle (BIFF8+).

    explicit            XclChTick();
};

// ----------------------------------------------------------------------------

struct XclChAxis
{
    sal_uInt16          mnType;             /// Axis type.

    explicit            XclChAxis();

    /** Returns the axis dimension index used by the chart API. */
    sal_Int32           GetApiAxisDimension() const;
};

// ----------------------------------------------------------------------------

struct XclChAxesSet
{
    XclChRectangle      maRect;             /// Position of the axes set (inner plot area).
    sal_uInt16          mnAxesSetId;        /// Primary/secondary axes set.

    explicit            XclChAxesSet();

    /** Returns the axes set index used by the chart API. */
    sal_Int32           GetApiAxesSetIndex() const;
};

// Property mode ==============================================================

/** Specifies the type of a formatting. This results in different property names. */
enum XclChPropertyMode
{
    EXC_CHPROPMODE_COMMON,          /// Common objects, no special handling.
    EXC_CHPROPMODE_LINEARSERIES,    /// Specific to data series drawn as lines.
    EXC_CHPROPMODE_FILLEDSERIES     /// Specific to data series drawn as areas.
};

// Static helper functions ====================================================

/** Contains static helper functions for the chart filters. */
class XclChartHelper
{
public:
    /** Returns a palette index for automatic series line colors. */
    static sal_uInt16   GetSeriesLineAutoColorIdx( sal_uInt16 nFormatIdx );
    /** Returns a palette index for automatic series fill colors. */
    static sal_uInt16   GetSeriesFillAutoColorIdx( sal_uInt16 nFormatIdx );
    /** Returns a transparency value for automatic series fill colors. */
    static sal_uInt8    GetSeriesFillAutoTransp( sal_uInt16 nFormatIdx );
    /** Returns an automatic symbol index for the passed format index. */
    static sal_uInt16   GetAutoMarkerType( sal_uInt16 nFormatIdx );
    /** Returns true, if the passed marker type is filled. */
    static bool         HasMarkerFillColor( sal_uInt16 nMarkerType );
    /** Returns the role name for a manual data source for error bars. */
    static ::rtl::OUString GetErrorBarValuesRole( sal_uInt8 nBarType );
};

// Chart formatting info provider =============================================

/** Enumerates different object types for specific automatic formatting behaviour. */
enum XclChObjectType
{
    EXC_CHOBJTYPE_BACKGROUND,       /// Chart background.
    EXC_CHOBJTYPE_PLOTFRAME,        /// Wall formatting in 2d charts.
    EXC_CHOBJTYPE_WALL3D,           /// Wall formatting in 3d charts.
    EXC_CHOBJTYPE_FLOOR3D,          /// Floor formatting in 3d charts.
    EXC_CHOBJTYPE_TEXT,             /// Text boxes (titles, data point labels).
    EXC_CHOBJTYPE_LEGEND,           /// Chart legend.
    EXC_CHOBJTYPE_LINEARSERIES,     /// Series formatting in a chart supporting line formatting only.
    EXC_CHOBJTYPE_FILLEDSERIES,     /// Series formatting in a chart supporting area formatting.
    EXC_CHOBJTYPE_AXISLINE,         /// Axis line format.
    EXC_CHOBJTYPE_GRIDLINE,         /// Axis grid line format.
    EXC_CHOBJTYPE_TRENDLINE,        /// Series trend line.
    EXC_CHOBJTYPE_ERRORBAR,         /// Series error bar.
    EXC_CHOBJTYPE_CONNECTLINE,      /// Data point connector line.
    EXC_CHOBJTYPE_HILOLINE,         /// High/low lines in stock charts.
    EXC_CHOBJTYPE_WHITEDROPBAR,     /// White-day drop bar in stock charts.
    EXC_CHOBJTYPE_BLACKDROPBAR      /// Black-day drop bar in stock charts.
};

/** Enumerates different types to handle missing frame objects. */
enum XclChFrameType
{
     EXC_CHFRAMETYPE_AUTO,          /// Missing frame represents automatic formatting.
     EXC_CHFRAMETYPE_INVISIBLE      /// Missing frame represents invisible formatting.
};

/** Contains information about auto formatting of a specific chart object type. */
struct XclChFormatInfo
{
    XclChObjectType     meObjType;          /// Object type for automatic format.
    XclChPropertyMode   mePropMode;         /// Property mode for property set helper.
    sal_uInt16          mnAutoLineColorIdx; /// Automatic line color index.
    sal_Int16           mnAutoLineWeight;   /// Automatic line weight (hairline, single, ...).
    sal_uInt16          mnAutoPattColorIdx; /// Automatic fill pattern color index.
    XclChFrameType      meDefFrameType;     /// Default format type for missing frame objects.
    bool                mbCreateDefFrame;   /// true = Create missing frame objects on import.
    bool                mbDeleteDefFrame;   /// true = Delete default frame formatting on export.
    bool                mbIsFrame;          /// true = Object is a frame, false = Object is a line.
};

// ----------------------------------------------------------------------------

/** Provides access to chart auto formatting for all available object types. */
class XclChFormatInfoProvider
{
public:
    explicit            XclChFormatInfoProvider();

    /** Returns an info struct about auto formatting for the passed object type. */
    const XclChFormatInfo& GetFormatInfo( XclChObjectType eObjType ) const;

private:
    typedef ::std::map< XclChObjectType, const XclChFormatInfo* > XclFmtInfoMap;
    XclFmtInfoMap       maInfoMap;          /// Maps object type to formatting data.
};

// Chart type info provider ===================================================

/** Enumerates all kinds of different chart types. */
enum XclChTypeId
{
    EXC_CHTYPEID_BAR,               /// Vertical bar chart.
    EXC_CHTYPEID_HORBAR,            /// Horizontal bar chart.
    EXC_CHTYPEID_LINE,              /// Line chart.
    EXC_CHTYPEID_AREA,              /// Area chart.
    EXC_CHTYPEID_STOCK,             /// Stock chart.
    EXC_CHTYPEID_RADARLINE,         /// Linear radar chart.
    EXC_CHTYPEID_RADARAREA,         /// Filled radar chart.
    EXC_CHTYPEID_PIE,               /// Pie chart.
    EXC_CHTYPEID_DONUT,             /// Donut chart.
    EXC_CHTYPEID_PIEEXT,            /// Pie-to-pie or pie-to-bar chart.
    EXC_CHTYPEID_SCATTER,           /// Scatter (XY) chart.
    EXC_CHTYPEID_BUBBLES,           /// Bubble chart.
    EXC_CHTYPEID_SURFACE,           /// Surface chart.
    EXC_CHTYPEID_UNKNOWN            /// Default for unknown chart types.
};

/** Enumerates different categories of similar chart types. */
enum XclChTypeCateg
{
    EXC_CHTYPECATEG_BAR,            /// Bar charts (horizontal or vertical).
    EXC_CHTYPECATEG_LINE,           /// Line charts (line, area, stock charts).
    EXC_CHTYPECATEG_RADAR,          /// Radar charts (linear or filled).
    EXC_CHTYPECATEG_PIE,            /// Pie and donut charts.
    EXC_CHTYPECATEG_SCATTER,        /// Scatter and bubble charts.
    EXC_CHTYPECATEG_SURFACE         /// Surface charts.
};

/** Enumerates modes for varying point colors in a series. */
enum XclChVarPointMode
{
    EXC_CHVARPOINT_NONE,            /// No varied colors supported.
    EXC_CHVARPOINT_SINGLE,          /// Only supported, if type group contains only one series.
    EXC_CHVARPOINT_MULTI            /// Supported for multiple series in a chart type group.
};

/** Contains information for a chart type. */
struct XclChTypeInfo
{
    XclChTypeId         meTypeId;               /// Unique chart type identifier.
    XclChTypeCateg      meTypeCateg;            /// Chart type category this type belongs to.
    sal_uInt16          mnRecId;                /// Record identifier written to the file.
    const sal_Char*     mpcServiceName;         /// Service name of the type.
    XclChVarPointMode   meVarPointMode;         /// Mode for varying point colors.
    sal_Int32           mnDefaultLabelPos;      /// Default data label position (API constant).
    bool                mbCombinable2d;         /// true = Types can be combined in one axes set.
    bool                mbSupports3d;           /// true = 3d type allowed, false = Only 2d type.
    bool                mbPolarCoordSystem;     /// true = Polar, false = Cartesian.
    bool                mbSeriesIsFrame2d;      /// true = Series with area formatting (2d charts).
    bool                mbSeriesIsFrame3d;      /// true = Series with area formatting (3d charts).
    bool                mbSingleSeriesVis;      /// true = Only first series visible.
    bool                mbCategoryAxis;         /// true = X axis contains categories.
    bool                mbSwappedAxesSet;       /// true = X and Y axes are swapped.
    bool                mbSupportsStacking;     /// true = Series can be stacked on each other.
    bool                mbReverseSeries;        /// true = Insert unstacked series in reverse order.
    bool                mbTicksBetweenCateg;    /// true = X axis ticks between categories.
};

/** Extended chart type information and access functions. */
struct XclChExtTypeInfo : public XclChTypeInfo
{
    bool                mb3dChart;              /// Chart is actually a 3D chart.
    bool                mbSpline;               /// Series lines are smoothed.

    explicit            XclChExtTypeInfo( const XclChTypeInfo& rTypeInfo );

    void                Set( const XclChTypeInfo& rTypeInfo, bool b3dChart, bool bSpline );

    /** Returns true, if this chart type supports area formatting for its series. */
    inline bool         IsSeriesFrameFormat() const
                            { return mb3dChart ? mbSeriesIsFrame3d : mbSeriesIsFrame2d; }
    /** Returns the correct object type identifier for series and data points. */
    inline XclChObjectType GetSeriesObjectType() const
                            { return IsSeriesFrameFormat() ? EXC_CHOBJTYPE_FILLEDSERIES : EXC_CHOBJTYPE_LINEARSERIES; }
};

// ----------------------------------------------------------------------------

/** Provides access to chart type info structs for all available chart types. */
class XclChTypeInfoProvider
{
public:
    explicit            XclChTypeInfoProvider();

    /** Returns chart type info for a unique chart type identifier. */
    const XclChTypeInfo& GetTypeInfo( XclChTypeId eType ) const;
    /** Returns the first fitting chart type info for an Excel chart type record identifier. */
    const XclChTypeInfo& GetTypeInfoFromRecId( sal_uInt16 nRecId ) const;
    /** Returns the first fitting chart type info for the passed service name. */
    const XclChTypeInfo& GetTypeInfoFromService( const ::rtl::OUString& rServiceName ) const;

private:
    typedef ::std::map< XclChTypeId, const XclChTypeInfo* > XclChTypeInfoMap;
    XclChTypeInfoMap    maInfoMap;          /// Maps chart types to type info data.
};

// Chart text and title object helpers ========================================

/** Enumerates different text box types for default text formatting and title
    positioning. */
enum XclChTextType
{
    EXC_CHTEXTTYPE_TITLE,           /// Chart title.
    EXC_CHTEXTTYPE_LEGEND,          /// Chart legend.
    EXC_CHTEXTTYPE_AXISTITLE,       /// Chart axis titles.
    EXC_CHTEXTTYPE_AXISLABEL,       /// Chart axis labels.
    EXC_CHTEXTTYPE_DATALABEL        /// Data point labels.
};

/** A map key for text and title objects. */
struct XclChTextKey : public ::std::pair< XclChTextType, ::std::pair< sal_uInt16, sal_uInt16 > >
{
    inline explicit     XclChTextKey( XclChTextType eTextType, sal_uInt16 nMainIdx = 0, sal_uInt16 nSubIdx = 0 )
                            { first = eTextType; second.first = nMainIdx; second.second = nSubIdx; }
};

/** Function prototype receiving a chart document and returning a title shape. */
typedef ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
    (*XclChGetShapeFunc)( const ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartDocument >& );

// Property helpers ===========================================================

class XclChObjectTable
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >     XNameContainerRef;
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    XServiceFactoryRef;

public:
    explicit            XclChObjectTable( XServiceFactoryRef xFactory,
                            const ::rtl::OUString& rServiceName, const ::rtl::OUString& rObjNameBase );

    /** Returns a named formatting object from the chart document. */
    ::com::sun::star::uno::Any GetObject( const ::rtl::OUString& rObjName );
    /** Insertes a named formatting object into the chart document. */
    ::rtl::OUString     InsertObject( const ::com::sun::star::uno::Any& rObj );

private:
    XServiceFactoryRef  mxFactory;              /// Factory to create the container.
    XNameContainerRef   mxContainer;            /// Container for the objects.
    ::rtl::OUString     maServiceName;          /// Service name to create the container.
    ::rtl::OUString     maObjNameBase;          /// Base of names for inserted objects.
    sal_Int32           mnIndex;                /// Index to create unique identifiers.
};

// ----------------------------------------------------------------------------

struct XclFontData;

/** Helper class for usage of property sets. */
class XclChPropSetHelper
{
public:
    explicit            XclChPropSetHelper();

    /** Reads all line properties from the passed property set. */
    void                ReadLineProperties(
                            XclChLineFormat& rLineFmt,
                            XclChObjectTable& rDashTable,
                            const ScfPropertySet& rPropSet,
                            XclChPropertyMode ePropMode );
    /** Reads solid area properties from the passed property set.
        @return  true = object contains complex fill properties. */
    bool                ReadAreaProperties(
                            XclChAreaFormat& rAreaFmt,
                            const ScfPropertySet& rPropSet,
                            XclChPropertyMode ePropMode );
    /** Reads gradient or bitmap area properties from the passed property set. */
    void                ReadEscherProperties(
                            XclChEscherFormat& rEscherFmt,
                            XclChPicFormat& rPicFmt,
                            XclChObjectTable& rGradientTable,
                            XclChObjectTable& rHatchTable,
                            XclChObjectTable& rBitmapTable,
                            const ScfPropertySet& rPropSet,
                            XclChPropertyMode ePropMode );
    /** Reads all marker properties from the passed property set. */
    void                ReadMarkerProperties(
                            XclChMarkerFormat& rMarkerFmt,
                            const ScfPropertySet& rPropSet,
                            sal_uInt16 nFormatIdx );
    /** Reads rotation properties from the passed property set. */
    sal_uInt16          ReadRotationProperties(
                            const ScfPropertySet& rPropSet,
                            bool bSupportsStacked );

    /** Writes all line properties to the passed property set. */
    void                WriteLineProperties(
                            ScfPropertySet& rPropSet,
                            XclChObjectTable& rDashTable,
                            const XclChLineFormat& rLineFmt,
                            XclChPropertyMode ePropMode );
    /** Writes solid area properties to the passed property set. */
    void                WriteAreaProperties(
                            ScfPropertySet& rPropSet,
                            const XclChAreaFormat& rAreaFmt,
                            XclChPropertyMode ePropMode );
    /** Writes gradient or bitmap area properties to the passed property set. */
    void                WriteEscherProperties(
                            ScfPropertySet& rPropSet,
                            XclChObjectTable& rGradientTable,
                            XclChObjectTable& rHatchTable,
                            XclChObjectTable& rBitmapTable,
                            const XclChEscherFormat& rEscherFmt,
                            const XclChPicFormat* pPicFmt,
                            sal_uInt32 nDffFillType,
                            XclChPropertyMode ePropMode );
    /** Writes all marker properties to the passed property set. */
    void                WriteMarkerProperties(
                            ScfPropertySet& rPropSet,
                            const XclChMarkerFormat& rMarkerFmt );
    /** Writes rotation properties to the passed property set. */
    void                WriteRotationProperties(
                            ScfPropertySet& rPropSet,
                            sal_uInt16 nRotation,
                            bool bSupportsStacked );

private:
    /** Returns a line property set helper according to the passed property mode. */
    ScfPropSetHelper&   GetLineHelper( XclChPropertyMode ePropMode );
    /** Returns an area property set helper according to the passed property mode. */
    ScfPropSetHelper&   GetAreaHelper( XclChPropertyMode ePropMode );
    /** Returns a gradient property set helper according to the passed property mode. */
    ScfPropSetHelper&   GetGradientHelper( XclChPropertyMode ePropMode );
    /** Returns a hatch property set helper according to the passed property mode. */
    ScfPropSetHelper&   GetHatchHelper( XclChPropertyMode ePropMode );

private:
    ScfPropSetHelper    maLineHlpCommon;    /// Properties for lines in common objects.
    ScfPropSetHelper    maLineHlpLinear;    /// Properties for lines in linear series.
    ScfPropSetHelper    maLineHlpFilled;    /// Properties for lines in filled series.
    ScfPropSetHelper    maAreaHlpCommon;    /// Properties for areas in common objects.
    ScfPropSetHelper    maAreaHlpFilled;    /// Properties for areas in filled series.
    ScfPropSetHelper    maGradHlpCommon;    /// Properties for gradients in common objects.
    ScfPropSetHelper    maGradHlpFilled;    /// Properties for gradients in filled series.
    ScfPropSetHelper    maHatchHlpCommon;   /// Properties for hatches in common objects.
    ScfPropSetHelper    maHatchHlpFilled;   /// Properties for hatches in filled series.
    ScfPropSetHelper    maBitmapHlp;        /// Properties for bitmaps.
};

// ============================================================================

/** Base struct for internal root data structs for import and export. */
struct XclChRootData
{
    typedef boost::shared_ptr< XclChTypeInfoProvider >      XclChTypeProvRef;
    typedef boost::shared_ptr< XclChFormatInfoProvider >    XclChFmtInfoProvRef;
    typedef boost::shared_ptr< XclChObjectTable >           XclChObjectTableRef;
    typedef ::std::map< XclChTextKey, XclChGetShapeFunc >   XclChGetShapeFuncMap;

    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >
                        mxChartDoc;             /// The chart document.
    Rectangle           maChartRect;            /// Position and size of the chart shape.
    XclChTypeProvRef    mxTypeInfoProv;         /// Provides info about chart types.
    XclChFmtInfoProvRef mxFmtInfoProv;          /// Provides info about auto formatting.
    XclChObjectTableRef mxLineDashTable;        /// Container for line dash styles.
    XclChObjectTableRef mxGradientTable;        /// Container for gradient fill styles.
    XclChObjectTableRef mxHatchTable;           /// Container for hatch fill styles.
    XclChObjectTableRef mxBitmapTable;          /// Container for bitmap fill styles.
    XclChGetShapeFuncMap maGetShapeFuncs;       /// Maps title shape getter functions.
    sal_Int32           mnBorderGapX;           /// Border gap to chart space in 1/100mm.
    sal_Int32           mnBorderGapY;           /// Border gap to chart space in 1/100mm.
    double              mfUnitSizeX;            /// Size of a chart X unit (1/4000 of chart width) in 1/100 mm.
    double              mfUnitSizeY;            /// Size of a chart Y unit (1/4000 of chart height) in 1/100 mm.

    explicit            XclChRootData();
    virtual             ~XclChRootData();

    /** Starts the API chart document conversion. Must be called once before any API access. */
    void                InitConversion(
                            const XclRoot& rRoot,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& rxChartDoc,
                            const Rectangle& rChartRect );
    /** Finishes the API chart document conversion. Must be called once before any API access. */
    void                FinishConversion();

    /** Returns the drawing shape interface of the specified title object. */
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        GetTitleShape( const XclChTextKey& rTitleKey ) const;
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
