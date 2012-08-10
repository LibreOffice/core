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

#ifndef _SVX_ESCHEREX_HXX
#define _SVX_ESCHEREX_HXX

#include <memory>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <tools/solar.h>
#include <tools/gen.hxx>
#include <tools/stream.hxx>
#include <com/sun/star/uno/Reference.h>
#include <svtools/grfmgr.hxx>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <svx/msdffdef.hxx>
#include "filter/msfilter/msfilterdllapi.h"

        /*Record Name       FBT-Value   Instance                  Contents                                                          Wrd Exl PPt Ver*/
// In the Microsoft documentation the naming scheme is msofbt... instead of ESCHER_...
#define ESCHER_DggContainer     0xF000  /*                           per-document data                                                  X   X   X     */
#define ESCHER_Dgg              0xF006  /*                           an FDGG and several FIDCLs                                         X   X   X   0 */
#define ESCHER_CLSID            0xF016  /*                           the CLSID of the application that put the data on the clipboard    C   C   C   0 */
#define ESCHER_OPT              0xF00B  /* count of properties       the document-wide default shape properties                         X   X   X   3 */
#define ESCHER_ColorMRU         0xF11A  /* count of colors           the colors in the MRU swatch                                       X   X   X   0 */
#define ESCHER_SplitMenuColors  0xF11E  /* count of colors           the colors in the top-level split menus                            X   X   X   0 */
#define ESCHER_BstoreContainer  0xF001  /* count of BLIPs            all images in the document (JPEGs, metafiles, etc.)                X   X   X     */
#define ESCHER_BSE              0xF007  /* BLIP type                 an FBSE (one per BLIP)                                             X   X   X   2 */
#define ESCHER_BlipFirst        0xF018  /*                           range of fbts reserved for various kinds of BLIPs                  X   X   X     */
#define ESCHER_BlipLast         0xF117  /*                           range of fbts reserved for various kinds of BLIPs                  X   X   X     */

#define ESCHER_DgContainer      0xF002  /*                           per-sheet/page/slide data                                          X   X   X     */
#define ESCHER_Dg               0xF008  /* drawing ID                an FDG                                                             X   X   X   0 */
#define ESCHER_RegroupItems     0xF118  /* count of regroup entries  several FRITs                                                      X   X   X   0 */
#define ESCHER_ColorScheme      0xF120  /* count of colors           the colors of the source host's color scheme                           C   C   0 */
#define ESCHER_SpgrContainer    0xF003  /*                           several SpContainers, the first of which is the group shape itself X   X   X     */
#define ESCHER_SpContainer      0xF004  /*                           a shape                                                            X   X   X     */
#define ESCHER_Spgr             0xF009  /*                           an FSPGR; only present if the shape is a group shape               X   X   X   1 */
#define ESCHER_Sp               0xF00A  /* shape type                an FSP                                                             X   X   X   2 */
//#define     ESCHER_OPT           0xF00B /* count of properties       a shape property table                                             X   X   X   3 */
#define ESCHER_Textbox          0xF00C  /*                           RTF text                                                           C   C   C   0 */
#define ESCHER_ClientTextbox    0xF00D  /* host-defined              the text in the textbox, in host-defined format                    X   X   X     */
#define ESCHER_Anchor           0xF00E  /*                           a RECT, in 100000ths of an inch                                    C   C   C   0 */
#define ESCHER_ChildAnchor      0xF00F  /*                           a RECT, in units relative to the parent group                      X   X   X   0 */
#define ESCHER_ClientAnchor     0xF010  /* host-defined              the location of the shape, in a host-defined format                X   X   X     */
#define ESCHER_ClientData       0xF011  /* host-defined              host-specific data                                                 X   X   X     */
#define ESCHER_OleObject        0xF11F  /*                           a serialized IStorage for an OLE object                            C   C   C   0 */
#define ESCHER_DeletedPspl      0xF11D  /*                           an FPSPL; only present in top-level deleted shapes                 X           0 */
#define ESCHER_SolverContainer  0xF005  /* count of rules            the rules governing shapes                                         X   X   X     */
#define ESCHER_ConnectorRule    0xF012  /*                           an FConnectorRule                                                      X   X   1 */
#define ESCHER_AlignRule        0xF013  /*                           an FAlignRule                                                      X   X   X   0 */
#define ESCHER_ArcRule          0xF014  /*                           an FARCRU                                                          X   X   X   0 */
#define ESCHER_ClientRule       0xF015  /* host-defined              host-defined                                                                     */
#define ESCHER_CalloutRule      0xF017  /*                           an FCORU                                                           X   X   X   0 */
#define ESCHER_Selection        0xF119  /*                           an FDGSL followed by the SPIDs of the shapes in the selection              X   0 */
#define ESCHER_UDefProp         0xF122

#define SHAPEFLAG_GROUP         0x001   /* This shape is a group shape */
#define SHAPEFLAG_CHILD         0x002   /* Not a top-level shape */
#define SHAPEFLAG_PATRIARCH     0x004   /* This is the topmost group shape. Exactly one of these per drawing. */
#define SHAPEFLAG_DELETED       0x008   /* The shape has been deleted */
#define SHAPEFLAG_OLESHAPE      0x010   /* The shape is an OLE object */
#define SHAPEFLAG_HAVEMASTER    0x020   /* Shape has a hspMaster property */
#define SHAPEFLAG_FLIPH         0x040   /* Shape is flipped horizontally */
#define SHAPEFLAG_FLIPV         0x080   /* Shape is flipped vertically */
#define SHAPEFLAG_CONNECTOR     0x100   /* Connector type of shape */
#define SHAPEFLAG_HAVEANCHOR    0x200   /* Shape has an anchor of some kind */
#define SHAPEFLAG_BACKGROUND    0x400   /* Background shape */
#define SHAPEFLAG_HAVESPT       0x800   /* Shape has a shape type property */

#define ESCHER_ShpInst_Min                          0
#define ESCHER_ShpInst_NotPrimitive                 ESCHER_ShpInst_Min
#define ESCHER_ShpInst_Rectangle                    1
#define ESCHER_ShpInst_RoundRectangle               2
#define ESCHER_ShpInst_Ellipse                      3
#define ESCHER_ShpInst_Diamond                      4
#define ESCHER_ShpInst_IsocelesTriangle             5
#define ESCHER_ShpInst_RightTriangle                6
#define ESCHER_ShpInst_Parallelogram                7
#define ESCHER_ShpInst_Trapezoid                    8
#define ESCHER_ShpInst_Hexagon                      9
#define ESCHER_ShpInst_Octagon                      10
#define ESCHER_ShpInst_Plus                         11
#define ESCHER_ShpInst_Star                         12
#define ESCHER_ShpInst_Arrow                        13
#define ESCHER_ShpInst_ThickArrow                   14
#define ESCHER_ShpInst_HomePlate                    15
#define ESCHER_ShpInst_Cube                         16
#define ESCHER_ShpInst_Balloon                      17
#define ESCHER_ShpInst_Seal                         18
#define ESCHER_ShpInst_Arc                          19
#define ESCHER_ShpInst_Line                         20
#define ESCHER_ShpInst_Plaque                       21
#define ESCHER_ShpInst_Can                          22
#define ESCHER_ShpInst_Donut                        23
#define ESCHER_ShpInst_TextSimple                   24
#define ESCHER_ShpInst_TextOctagon                  25
#define ESCHER_ShpInst_TextHexagon                  26
#define ESCHER_ShpInst_TextCurve                    27
#define ESCHER_ShpInst_TextWave                     28
#define ESCHER_ShpInst_TextRing                     29
#define ESCHER_ShpInst_TextOnCurve                  30
#define ESCHER_ShpInst_TextOnRing                   31
#define ESCHER_ShpInst_StraightConnector1           32
#define ESCHER_ShpInst_BentConnector2               33
#define ESCHER_ShpInst_BentConnector3               34
#define ESCHER_ShpInst_BentConnector4               35
#define ESCHER_ShpInst_BentConnector5               36
#define ESCHER_ShpInst_CurvedConnector2             37
#define ESCHER_ShpInst_CurvedConnector3             38
#define ESCHER_ShpInst_CurvedConnector4             39
#define ESCHER_ShpInst_CurvedConnector5             40
#define ESCHER_ShpInst_Callout1                     41
#define ESCHER_ShpInst_Callout2                     42
#define ESCHER_ShpInst_Callout3                     43
#define ESCHER_ShpInst_AccentCallout1               44
#define ESCHER_ShpInst_AccentCallout2               45
#define ESCHER_ShpInst_AccentCallout3               46
#define ESCHER_ShpInst_BorderCallout1               47
#define ESCHER_ShpInst_BorderCallout2               48
#define ESCHER_ShpInst_BorderCallout3               49
#define ESCHER_ShpInst_AccentBorderCallout1         50
#define ESCHER_ShpInst_AccentBorderCallout2         51
#define ESCHER_ShpInst_AccentBorderCallout3         52
#define ESCHER_ShpInst_Ribbon                       53
#define ESCHER_ShpInst_Ribbon2                      54
#define ESCHER_ShpInst_Chevron                      55
#define ESCHER_ShpInst_Pentagon                     56
#define ESCHER_ShpInst_NoSmoking                    57
#define ESCHER_ShpInst_Seal8                        58
#define ESCHER_ShpInst_Seal16                       59
#define ESCHER_ShpInst_Seal32                       60
#define ESCHER_ShpInst_WedgeRectCallout             61
#define ESCHER_ShpInst_WedgeRRectCallout            62
#define ESCHER_ShpInst_WedgeEllipseCallout          63
#define ESCHER_ShpInst_Wave                         64
#define ESCHER_ShpInst_FoldedCorner                 65
#define ESCHER_ShpInst_LeftArrow                    66
#define ESCHER_ShpInst_DownArrow                    67
#define ESCHER_ShpInst_UpArrow                      68
#define ESCHER_ShpInst_LeftRightArrow               69
#define ESCHER_ShpInst_UpDownArrow                  70
#define ESCHER_ShpInst_IrregularSeal1               71
#define ESCHER_ShpInst_IrregularSeal2               72
#define ESCHER_ShpInst_LightningBolt                73
#define ESCHER_ShpInst_Heart                        74
#define ESCHER_ShpInst_PictureFrame                 75
#define ESCHER_ShpInst_QuadArrow                    76
#define ESCHER_ShpInst_LeftArrowCallout             77
#define ESCHER_ShpInst_RightArrowCallout            78
#define ESCHER_ShpInst_UpArrowCallout               79
#define ESCHER_ShpInst_DownArrowCallout             80
#define ESCHER_ShpInst_LeftRightArrowCallout        81
#define ESCHER_ShpInst_UpDownArrowCallout           82
#define ESCHER_ShpInst_QuadArrowCallout             83
#define ESCHER_ShpInst_Bevel                        84
#define ESCHER_ShpInst_LeftBracket                  85
#define ESCHER_ShpInst_RightBracket                 86
#define ESCHER_ShpInst_LeftBrace                    87
#define ESCHER_ShpInst_RightBrace                   88
#define ESCHER_ShpInst_LeftUpArrow                  89
#define ESCHER_ShpInst_BentUpArrow                  90
#define ESCHER_ShpInst_BentArrow                    91
#define ESCHER_ShpInst_Seal24                       92
#define ESCHER_ShpInst_StripedRightArrow            93
#define ESCHER_ShpInst_NotchedRightArrow            94
#define ESCHER_ShpInst_BlockArc                     95
#define ESCHER_ShpInst_SmileyFace                   96
#define ESCHER_ShpInst_VerticalScroll               97
#define ESCHER_ShpInst_HorizontalScroll             98
#define ESCHER_ShpInst_CircularArrow                99
#define ESCHER_ShpInst_NotchedCircularArrow         100
#define ESCHER_ShpInst_UturnArrow                   101
#define ESCHER_ShpInst_CurvedRightArrow             102
#define ESCHER_ShpInst_CurvedLeftArrow              103
#define ESCHER_ShpInst_CurvedUpArrow                104
#define ESCHER_ShpInst_CurvedDownArrow              105
#define ESCHER_ShpInst_CloudCallout                 106
#define ESCHER_ShpInst_EllipseRibbon                107
#define ESCHER_ShpInst_EllipseRibbon2               108
#define ESCHER_ShpInst_FlowChartProcess             109
#define ESCHER_ShpInst_FlowChartDecision            110
#define ESCHER_ShpInst_FlowChartInputOutput         111
#define ESCHER_ShpInst_FlowChartPredefinedProcess   112
#define ESCHER_ShpInst_FlowChartInternalStorage     113
#define ESCHER_ShpInst_FlowChartDocument            114
#define ESCHER_ShpInst_FlowChartMultidocument       115
#define ESCHER_ShpInst_FlowChartTerminator          116
#define ESCHER_ShpInst_FlowChartPreparation         117
#define ESCHER_ShpInst_FlowChartManualInput         118
#define ESCHER_ShpInst_FlowChartManualOperation     119
#define ESCHER_ShpInst_FlowChartConnector           120
#define ESCHER_ShpInst_FlowChartPunchedCard         121
#define ESCHER_ShpInst_FlowChartPunchedTape         122
#define ESCHER_ShpInst_FlowChartSummingJunction     123
#define ESCHER_ShpInst_FlowChartOr                  124
#define ESCHER_ShpInst_FlowChartCollate             125
#define ESCHER_ShpInst_FlowChartSort                126
#define ESCHER_ShpInst_FlowChartExtract             127
#define ESCHER_ShpInst_FlowChartMerge               128
#define ESCHER_ShpInst_FlowChartOfflineStorage      129
#define ESCHER_ShpInst_FlowChartOnlineStorage       130
#define ESCHER_ShpInst_FlowChartMagneticTape        131
#define ESCHER_ShpInst_FlowChartMagneticDisk        132
#define ESCHER_ShpInst_FlowChartMagneticDrum        133
#define ESCHER_ShpInst_FlowChartDisplay             134
#define ESCHER_ShpInst_FlowChartDelay               135
#define ESCHER_ShpInst_TextPlainText                136
#define ESCHER_ShpInst_TextStop                     137
#define ESCHER_ShpInst_TextTriangle                 138
#define ESCHER_ShpInst_TextTriangleInverted         139
#define ESCHER_ShpInst_TextChevron                  140
#define ESCHER_ShpInst_TextChevronInverted          141
#define ESCHER_ShpInst_TextRingInside               142
#define ESCHER_ShpInst_TextRingOutside              143
#define ESCHER_ShpInst_TextArchUpCurve              144
#define ESCHER_ShpInst_TextArchDownCurve            145
#define ESCHER_ShpInst_TextCircleCurve              146
#define ESCHER_ShpInst_TextButtonCurve              147
#define ESCHER_ShpInst_TextArchUpPour               148
#define ESCHER_ShpInst_TextArchDownPour             149
#define ESCHER_ShpInst_TextCirclePour               150
#define ESCHER_ShpInst_TextButtonPour               151
#define ESCHER_ShpInst_TextCurveUp                  152
#define ESCHER_ShpInst_TextCurveDown                153
#define ESCHER_ShpInst_TextCascadeUp                154
#define ESCHER_ShpInst_TextCascadeDown              155
#define ESCHER_ShpInst_TextWave1                    156
#define ESCHER_ShpInst_TextWave2                    157
#define ESCHER_ShpInst_TextWave3                    158
#define ESCHER_ShpInst_TextWave4                    159
#define ESCHER_ShpInst_TextInflate                  160
#define ESCHER_ShpInst_TextDeflate                  161
#define ESCHER_ShpInst_TextInflateBottom            162
#define ESCHER_ShpInst_TextDeflateBottom            163
#define ESCHER_ShpInst_TextInflateTop               164
#define ESCHER_ShpInst_TextDeflateTop               165
#define ESCHER_ShpInst_TextDeflateInflate           166
#define ESCHER_ShpInst_TextDeflateInflateDeflate    167
#define ESCHER_ShpInst_TextFadeRight                168
#define ESCHER_ShpInst_TextFadeLeft                 169
#define ESCHER_ShpInst_TextFadeUp                   170
#define ESCHER_ShpInst_TextFadeDown                 171
#define ESCHER_ShpInst_TextSlantUp                  172
#define ESCHER_ShpInst_TextSlantDown                173
#define ESCHER_ShpInst_TextCanUp                    174
#define ESCHER_ShpInst_TextCanDown                  175
#define ESCHER_ShpInst_FlowChartAlternateProcess    176
#define ESCHER_ShpInst_FlowChartOffpageConnector    177
#define ESCHER_ShpInst_Callout90                    178
#define ESCHER_ShpInst_AccentCallout90              179
#define ESCHER_ShpInst_BorderCallout90              180
#define ESCHER_ShpInst_AccentBorderCallout90        181
#define ESCHER_ShpInst_LeftRightUpArrow             182
#define ESCHER_ShpInst_Sun                          183
#define ESCHER_ShpInst_Moon                         184
#define ESCHER_ShpInst_BracketPair                  185
#define ESCHER_ShpInst_BracePair                    186
#define ESCHER_ShpInst_Seal4                        187
#define ESCHER_ShpInst_DoubleWave                   188
#define ESCHER_ShpInst_ActionButtonBlank            189
#define ESCHER_ShpInst_ActionButtonHome             190
#define ESCHER_ShpInst_ActionButtonHelp             191
#define ESCHER_ShpInst_ActionButtonInformation      192
#define ESCHER_ShpInst_ActionButtonForwardNext      193
#define ESCHER_ShpInst_ActionButtonBackPrevious     194
#define ESCHER_ShpInst_ActionButtonEnd              195
#define ESCHER_ShpInst_ActionButtonBeginning        196
#define ESCHER_ShpInst_ActionButtonReturn           197
#define ESCHER_ShpInst_ActionButtonDocument         198
#define ESCHER_ShpInst_ActionButtonSound            199
#define ESCHER_ShpInst_ActionButtonMovie            200
#define ESCHER_ShpInst_HostControl                  201
#define ESCHER_ShpInst_TextBox                      202
#define ESCHER_ShpInst_COUNT                        203
#define ESCHER_ShpInst_Max                          0x0FFF
#define ESCHER_ShpInst_Nil                          ESCHER_ShpInst_Max

enum ESCHER_BlibType
{                           // GEL provided types...
   ERROR = 0,               // An error occurred during loading
   UNKNOWN,                 // An unknown blip type
   EMF,                     // Windows Enhanced Metafile
   WMF,                     // Windows Metafile
   PICT,                    // Macintosh PICT
   PEG,                     // JFIF
   PNG,                     // PNG
   DIB,                     // Windows DIB
   FirstClient = 32,        // First client defined blip type
   LastClient  = 255        // Last client defined blip type
};

enum ESCHER_FillStyle
{
    ESCHER_FillSolid,       // Fill with a solid color
    ESCHER_FillPattern,     // Fill with a pattern (bitmap)
    ESCHER_FillTexture,     // A texture (pattern with its own color map)
    ESCHER_FillPicture,     // Center a picture in the shape
    ESCHER_FillShade,       // Shade from start to end points
    ESCHER_FillShadeCenter, // Shade from bounding rectangle to end point
    ESCHER_FillShadeShape,  // Shade from shape outline to end point
    ESCHER_FillShadeScale,
    ESCHER_FillShadeTitle,
    ESCHER_FillBackground
};

enum ESCHER_wMode
{
    ESCHER_wColor,          // only used for predefined shades
    ESCHER_wAutomatic,      // depends on object type
    ESCHER_wGrayScale,      // shades of gray only
    ESCHER_wLightGrayScale, // shades of light gray only
    ESCHER_wInverseGray,    // dark gray mapped to light gray, etc.
    ESCHER_wGrayOutline,    // pure gray and white
    ESCHER_wBlackTextLine,  // black text and lines, all else grayscale
    ESCHER_wHighContrast,   // pure black and white mode (no grays)
    ESCHER_wBlack,          // solid black   msobwWhite,          // solid white
    ESCHER_wDontShow,       // object not drawn
    ESCHER_wNumModes        // number of Black and white modes
};


//
enum ESCHER_ShapePath
{
    ESCHER_ShapeLines,          // A line of straight segments
    ESCHER_ShapeLinesClosed,    // A closed polygonal object
    ESCHER_ShapeCurves,         // A line of Bezier curve segments
    ESCHER_ShapeCurvesClosed,   // A closed shape with curved edges
    ESCHER_ShapeComplex         // pSegmentInfo must be non-empty
};


enum ESCHER_WrapMode
{
    ESCHER_WrapSquare,
    ESCHER_WrapByPoints,
    ESCHER_WrapNone,
    ESCHER_WrapTopBottom,
    ESCHER_WrapThrough
};

//
enum ESCHER_bwMode
{
    ESCHER_bwColor,             // only used for predefined shades
    ESCHER_bwAutomatic,         // depends on object type
    ESCHER_bwGrayScale,         // shades of gray only
    ESCHER_bwLightGrayScale,    // shades of light gray only
    ESCHER_bwInverseGray,       // dark gray mapped to light gray, etc.
    ESCHER_bwGrayOutline,       // pure gray and white
    ESCHER_bwBlackTextLine,     // black text and lines, all else grayscale
    ESCHER_bwHighContrast,      // pure black and white mode (no grays)
    ESCHER_bwBlack,             // solid black
    ESCHER_bwWhite,             // solid white
    ESCHER_bwDontShow,          // object not drawn
    ESCHER_bwNumModes           // number of Black and white modes
};


enum ESCHER_AnchorText
{
    ESCHER_AnchorTop,
    ESCHER_AnchorMiddle,
    ESCHER_AnchorBottom,
    ESCHER_AnchorTopCentered,
    ESCHER_AnchorMiddleCentered,
    ESCHER_AnchorBottomCentered,
    ESCHER_AnchorTopBaseline,
    ESCHER_AnchorBottomBaseline,
    ESCHER_AnchorTopCenteredBaseline,
    ESCHER_AnchorBottomCenteredBaseline
};

enum ESCHER_cDir
{
    ESCHER_cDir0,       // Right
    ESCHER_cDir90,      // Down
    ESCHER_cDir180,     // Left
    ESCHER_cDir270      // Up
};

//  connector style
enum ESCHER_cxSTYLE
{
    ESCHER_cxstyleStraight = 0,
    ESCHER_cxstyleBent,
    ESCHER_cxstyleCurved,
    ESCHER_cxstyleNone
};

//  text flow
enum ESCHER_txfl
{
    ESCHER_txflHorzN,           // Horizontal non-@
    ESCHER_txflTtoBA,           // Top to Bottom @-font
    ESCHER_txflBtoT,            // Bottom to Top non-@
    ESCHER_txflTtoBN,           // Top to Bottom non-@
    ESCHER_txflHorzA,           // Horizontal @-font
    ESCHER_txflVertN            // Vertical, non-@
};

//  text direction (needed for Bi-Di support)
enum ESCHER_txDir
{
    ESCHER_txdirLTR,            // left-to-right text direction
    ESCHER_txdirRTL,            // right-to-left text direction
    ESCHER_txdirContext         // context text direction
};

// Callout Type
enum ESCHER_spcot
{
    ESCHER_spcotRightAngle  = 1,
    ESCHER_spcotOneSegment  = 2,
    ESCHER_spcotTwoSegment  = 3,
    ESCHER_spcotThreeSegment= 4
};

// Callout Angle
enum ESCHER_spcoa
{
    ESCHER_spcoaAny,
    ESCHER_spcoa30,
    ESCHER_spcoa45,
    ESCHER_spcoa60,
    ESCHER_spcoa90,
    ESCHER_spcoa0
};

//  Callout Drop
enum ESCHER_spcod
{
    ESCHER_spcodTop,
    ESCHER_spcodCenter,
    ESCHER_spcodBottom,
    ESCHER_spcodSpecified
};

// FontWork alignment
enum ESCHER_GeoTextAlign
{
    ESCHER_AlignTextStretch,        // Stretch each line of text to fit width.
    ESCHER_AlignTextCenter,         // Center text on width.
    ESCHER_AlignTextLeft,           // Left justify.
    ESCHER_AlignTextRight,          // Right justify.
    ESCHER_AlignTextLetterJust,     // Spread letters out to fit width.
    ESCHER_AlignTextWordJust,       // Spread words out to fit width.
    ESCHER_AlignTextInvalid         // Invalid
};

//  flags for pictures
enum ESCHER_BlipFlags
{
    ESCHER_BlipFlagDefault = 0,
    ESCHER_BlipFlagComment = 0,     // Blip name is a comment
    ESCHER_BlipFlagFile,            // Blip name is a file name
    ESCHER_BlipFlagURL,             // Blip name is a full URL
    ESCHER_BlipFlagType = 3,        // Mask to extract type
   /* Or the following flags with any of the above. */
    ESCHER_BlipFlagDoNotSave = 4,
    ESCHER_BlipFlagLinkToFile = 8
};

//
enum ESCHER_3DRenderMode
{
    ESCHER_FullRender,      // Generate a full rendering
    ESCHER_Wireframe,       // Generate a wireframe
    ESCHER_BoundingCube     // Generate a bounding cube
};

//
enum ESCHER_xFormType
{
    ESCHER_xFormAbsolute,   // Apply transform in absolute space centered on shape
    ESCHER_xFormShape,      // Apply transform to shape geometry
    ESCHER_xFormDrawing     // Apply transform in drawing space
};

//
enum ESCHER_ShadowType
{
    ESCHER_ShadowOffset,    // N pixel offset shadow
    ESCHER_ShadowDouble,    // Use second offset too
    ESCHER_ShadowRich,      // Rich perspective shadow (cast relative to shape)
    ESCHER_ShadowShape,     // Rich perspective shadow (cast in shape space)
    ESCHER_ShadowDrawing,   // Perspective shadow cast in drawing space
    ESCHER_ShadowEmbossOrEngrave
};

//  - the type of a (length) measurement
enum ESCHER_dzType
   {
   ESCHER_dzTypeMin         = 0,
   ESCHER_dzTypeDefault     = 0,        // Default size, ignore the values
   ESCHER_dzTypeA           = 1,        // Values are in EMUs
   ESCHER_dzTypeV           = 2,        // Values are in pixels
   ESCHER_dzTypeShape       = 3,        // Values are 16.16 fractions of shape size
   ESCHER_dzTypeFixedAspect = 4,        // Aspect ratio is fixed
   ESCHER_dzTypeAFixed      = 5,        // EMUs, fixed aspect ratio
   ESCHER_dzTypeVFixed      = 6,        // Pixels, fixed aspect ratio
   ESCHER_dzTypeShapeFixed  = 7,        // Proportion of shape, fixed aspect ratio
   ESCHER_dzTypeFixedAspectEnlarge= 8,  // Aspect ratio is fixed, favor larger size
   ESCHER_dzTypeAFixedBig    = 9,       // EMUs, fixed aspect ratio
   ESCHER_dzTypeVFixedBig    = 10,      // Pixels, fixed aspect ratio
   ESCHER_dzTypeShapeFixedBig= 11,      // Proportion of shape, fixed aspect ratio
   ESCHER_dzTypeMax          = 11
};

// how to interpret the colors in a shaded fill.
enum ESCHER_ShadeType
{
    ESCHER_ShadeNone  = 0,        // Interpolate without correction between RGBs
    ESCHER_ShadeGamma = 1,        // Apply gamma correction to colors
    ESCHER_ShadeSigma = 2,        // Apply a sigma transfer function to position
    ESCHER_ShadeBand  = 4,        // Add a flat band at the start of the shade
    ESCHER_ShadeOneColor = 8,     // This is a one color shade

   /* A parameter for the band or sigma function can be stored in the top
      16 bits of the value - this is a proportion of *each* band of the
      shade to make flat (or the approximate equal value for a sigma
      function).  NOTE: the parameter is not used for the sigma function,
      instead a built in value is used.  This value should not be changed
      from the default! */
    ESCHER_ShadeParameterShift = 16,
    ESCHER_ShadeParameterMask  = 0xffff0000,

    ESCHER_ShadeDefault = (ESCHER_ShadeGamma|ESCHER_ShadeSigma|
                     (16384<<ESCHER_ShadeParameterShift))
};

// compound line style
enum ESCHER_LineStyle
{
    ESCHER_LineSimple,          // Single line (of width lineWidth)
    ESCHER_LineDouble,          // Double lines of equal width
    ESCHER_LineThickThin,       // Double lines, one thick, one thin
    ESCHER_LineThinThick,       // Double lines, reverse order
    ESCHER_LineTriple           // Three lines, thin, thick, thin
};

//  how to "fill" the line contour
enum ESCHER_LineType
{
   ESCHER_lineSolidType,        // Fill with a solid color
   ESCHER_linePattern,          // Fill with a pattern (bitmap)
   ESCHER_lineTexture,          // A texture (pattern with its own color map)
   ESCHER_linePicture           // Center a picture in the shape
};

// dashed line style
enum ESCHER_LineDashing
{
    ESCHER_LineSolid,               // Solid (continuous) pen
    ESCHER_LineDashSys,             // PS_DASH system   dash style
    ESCHER_LineDotSys,              // PS_DOT system   dash style
    ESCHER_LineDashDotSys,          // PS_DASHDOT system dash style
    ESCHER_LineDashDotDotSys,       // PS_DASHDOTDOT system dash style
    ESCHER_LineDotGEL,              // square dot style
    ESCHER_LineDashGEL,             // dash style
    ESCHER_LineLongDashGEL,         // long dash style
    ESCHER_LineDashDotGEL,          // dash short dash
    ESCHER_LineLongDashDotGEL,      // long dash short dash
    ESCHER_LineLongDashDotDotGEL    // long dash short dash short dash
};

// line end effect
enum ESCHER_LineEnd
{
    ESCHER_LineNoEnd,
    ESCHER_LineArrowEnd,
    ESCHER_LineArrowStealthEnd,
    ESCHER_LineArrowDiamondEnd,
    ESCHER_LineArrowOvalEnd,
    ESCHER_LineArrowOpenEnd
};

// size of arrowhead
enum ESCHER_LineWidth
{
    ESCHER_LineNarrowArrow,
    ESCHER_LineMediumWidthArrow,
    ESCHER_LineWideArrow
};

// size of arrowhead
enum ESCHER_LineEndLenght
{
    ESCHER_LineShortArrow,
    ESCHER_LineMediumLenArrow,
    ESCHER_LineLongArrow
};

// line join style.
enum ESCHER_LineJoin
{
    ESCHER_LineJoinBevel,     // Join edges by a straight line
    ESCHER_LineJoinMiter,     // Extend edges until they join
    ESCHER_LineJoinRound      // Draw an arc between the two edges
};

// line cap style (applies to ends of dash segments too).
enum ESCHER_LineCap
{
    ESCHER_LineEndCapRound,   // Rounded ends - the default
    ESCHER_LineEndCapSquare,  // Square protrudes by half line width
    ESCHER_LineEndCapFlat     // Line ends at end point
};

// Shape Properties
// 1pt = 12700 EMU (English Metric Units)
// 1pt = 20 Twip = 20/1440" = 1/72"
// 1twip=635 EMU
// 1" = 12700*72 = 914400 EMU
// 1" = 25.4mm
// 1mm = 36000 EMU
// Transform
#define ESCHER_Prop_Rotation                      4  /*  Fixed Point 16.16 degrees                  */
// Protection
#define ESCHER_Prop_LockRotation                119  /*  sal_Bool              No rotation                         */
#define ESCHER_Prop_LockAspectRatio             120  /*  sal_Bool              Don't allow changes in aspect ratio */
#define ESCHER_Prop_LockPosition                121  /*  sal_Bool              Don't allow the shape to be moved   */
#define ESCHER_Prop_LockAgainstSelect           122  /*  sal_Bool              Shape may not be selected           */
#define ESCHER_Prop_LockCropping                123  /*  sal_Bool              No cropping this shape              */
#define ESCHER_Prop_LockVertices                124  /*  sal_Bool              Edit Points not allowed             */
#define ESCHER_Prop_LockText                    125  /*  sal_Bool              Do not edit text                    */
#define ESCHER_Prop_LockAdjustHandles           126  /*  sal_Bool              Do not adjust                       */
#define ESCHER_Prop_LockAgainstGrouping         127  /*  sal_Bool              Do not group this shape             */
// Text
#define ESCHER_Prop_lTxid                       128  /*  LONG              id for the text, value determined by the host            */
#define ESCHER_Prop_dxTextLeft                  129  /*  LONG              margins relative to shape's inscribed                    */
#define ESCHER_Prop_dyTextTop                   130  /*  LONG                text rectangle (in EMUs)                               */
#define ESCHER_Prop_dxTextRight                 131  /*  LONG                                                                       */
#define ESCHER_Prop_dyTextBottom                132  /*  LONG                                                                       */
#define ESCHER_Prop_WrapText                    133  /*  MSOWRAPMODE       Wrap text at shape margins                               */
#define ESCHER_Prop_scaleText                   134  /*  LONG              Text zoom/scale (used if fFitTextToShape)                */
#define ESCHER_Prop_AnchorText                  135  /*  ESCHER_AnchorText How to anchor the text                                   */
#define ESCHER_Prop_txflTextFlow                136  /*  MSOTXFL           Text flow                                                */
#define ESCHER_Prop_cdirFont                    137  /*  MSOCDIR           Font rotation                                            */
#define ESCHER_Prop_hspNext                     138  /*  MSOHSP            ID of the next shape (used by Word for linked textboxes) */
#define ESCHER_Prop_txdir                       139  /*  MSOTXDIR          Bi-Di Text direction                                     */
#define ESCHER_Prop_SelectText                  187  /*  sal_Bool              sal_True if single click selects text, sal_False if two clicks   */
#define ESCHER_Prop_AutoTextMargin              188  /*  sal_Bool              use host's margin calculations                           */
#define ESCHER_Prop_RotateText                  189  /*  sal_Bool              Rotate text with shape                                   */
#define ESCHER_Prop_FitShapeToText              190  /*  sal_Bool              Size shape to fit text size                              */
#define ESCHER_Prop_FitTextToShape              191  /*  sal_Bool              Size text to fit shape size                              */
// GeoText
#define ESCHER_Prop_gtextUNICODE                192  /*  WCHAR*            UNICODE text string       */
#define ESCHER_Prop_gtextRTF                    193  /*  char*             RTF text string           */
#define ESCHER_Prop_gtextAlign                  194  /*  MSOGEOTEXTALIGN   alignment on curve        */
#define ESCHER_Prop_gtextSize                   195  /*  LONG              default point size        */
#define ESCHER_Prop_gtextSpacing                196  /*  LONG              fixed point 16.16         */
#define ESCHER_Prop_gtextFont                   197  /*  WCHAR*            font family name          */
#define ESCHER_Prop_gtextFReverseRows           240  /*  sal_Bool              Reverse row order         */
#define ESCHER_Prop_fGtext                      241  /*  sal_Bool              Has text effect           */
#define ESCHER_Prop_gtextFVertical              242  /*  sal_Bool              Rotate characters         */
#define ESCHER_Prop_gtextFKern                  243  /*  sal_Bool              Kern characters           */
#define ESCHER_Prop_gtextFTight                 244  /*  sal_Bool              Tightening or tracking    */
#define ESCHER_Prop_gtextFStretch               245  /*  sal_Bool              Stretch to fit shape      */
#define ESCHER_Prop_gtextFShrinkFit             246  /*  sal_Bool              Char bounding box         */
#define ESCHER_Prop_gtextFBestFit               247  /*  sal_Bool              Scale text-on-path        */
#define ESCHER_Prop_gtextFNormalize             248  /*  sal_Bool              Stretch char height       */
#define ESCHER_Prop_gtextFDxMeasure             249  /*  sal_Bool              Do not measure along path */
#define ESCHER_Prop_gtextFBold                  250  /*  sal_Bool              Bold font                 */
#define ESCHER_Prop_gtextFItalic                251  /*  sal_Bool              Italic font               */
#define ESCHER_Prop_gtextFUnderline             252  /*  sal_Bool              Underline font            */
#define ESCHER_Prop_gtextFShadow                253  /*  sal_Bool              Shadow font               */
#define ESCHER_Prop_gtextFSmallcaps             254  /*  sal_Bool              Small caps font           */
#define ESCHER_Prop_gtextFStrikethrough         255  /*  sal_Bool              Strike through font       */
// Blip
#define ESCHER_Prop_cropFromTop                 256  /*  LONG              16.16 fraction times total                          */
#define ESCHER_Prop_cropFromBottom              257  /*  LONG                image width or height,                            */
#define ESCHER_Prop_cropFromLeft                258  /*  LONG                as appropriate.                                   */
#define ESCHER_Prop_cropFromRight               259  /*  LONG                                                                  */
#define ESCHER_Prop_pib                         260  /*  IMsoBlip*         Blip to display                                     */
#define ESCHER_Prop_pibName                     261  /*  WCHAR*            Blip file name                                      */
#define ESCHER_Prop_pibFlags                    262  /*  MSOBLIPFLAGS      Blip flags                                          */
#define ESCHER_Prop_pictureTransparent          263  /*  LONG              transparent color (none if ~0UL)                    */
#define ESCHER_Prop_pictureContrast             264  /*  LONG              contrast setting                                    */
#define ESCHER_Prop_pictureBrightness           265  /*  LONG              brightness setting                                  */
#define ESCHER_Prop_pictureGamma                266  /*  LONG              16.16 gamma                                         */
#define ESCHER_Prop_pictureId                   267  /*  LONG              Host-defined ID for OLE objects (usually a pointer) */
#define ESCHER_Prop_pictureDblCrMod             268  /*  MSOCLR            Modification used if shape has double shadow        */
#define ESCHER_Prop_pictureFillCrMod            269  /*  MSOCLR                                                                */
#define ESCHER_Prop_pictureLineCrMod            270  /*  MSOCLR                                                                */
#define ESCHER_Prop_pibPrint                    271  /*  IMsoBlip*         Blip to display when printing                       */
#define ESCHER_Prop_pibPrintName                272  /*  WCHAR*            Blip file name                                      */
#define ESCHER_Prop_pibPrintFlags               273  /*  MSOBLIPFLAGS      Blip flags                                          */
#define ESCHER_Prop_fNoHitTestPicture           316  /*  sal_Bool              Do not hit test the picture                         */
#define ESCHER_Prop_pictureGray                 317  /*  sal_Bool              grayscale display                                   */
#define ESCHER_Prop_pictureBiLevel              318  /*  sal_Bool              bi-level display                                    */
#define ESCHER_Prop_pictureActive               319  /*  sal_Bool              Server is active (OLE objects only)                 */
// Geometry
#define ESCHER_Prop_geoLeft                     320  /*  LONG              Defines the G (geometry) coordinate space.  */
#define ESCHER_Prop_geoTop                      321  /*  LONG                                                          */
#define ESCHER_Prop_geoRight                    322  /*  LONG                                                          */
#define ESCHER_Prop_geoBottom                   323  /*  LONG                                                          */
#define ESCHER_Prop_shapePath                   324  /*  MSOSHAPEPATH                                                  */
#define ESCHER_Prop_pVertices                   325  /*  IMsoArray         An array of points, in G units.             */
#define ESCHER_Prop_pSegmentInfo                326  /*  IMsoArray                                                     */
#define ESCHER_Prop_adjustValue                 327  /*  LONG              Adjustment values corresponding to          */
#define ESCHER_Prop_adjust2Value                328  /*  LONG                the positions of the adjust handles       */
#define ESCHER_Prop_adjust3Value                329  /*  LONG                of the shape. The number of values        */
#define ESCHER_Prop_adjust4Value                330  /*  LONG                used and their allowable ranges vary      */
#define ESCHER_Prop_adjust5Value                331  /*  LONG                from shape type to shape type.            */
#define ESCHER_Prop_adjust6Value                332  /*  LONG                                                          */
#define ESCHER_Prop_adjust7Value                333  /*  LONG                                                          */
#define ESCHER_Prop_adjust8Value                334  /*  LONG                                                          */
#define ESCHER_Prop_adjust9Value                335  /*  LONG                                                          */
#define ESCHER_Prop_adjust10Value               336  /*  LONG                                                          */
#define ESCHER_Prop_fShadowOK                   378  /*  sal_Bool              Shadow may be set                           */
#define ESCHER_Prop_f3DOK                       379  /*  sal_Bool              3D may be set                               */
#define ESCHER_Prop_fLineOK                     380  /*  sal_Bool              Line style may be set                       */
#define ESCHER_Prop_fGtextOK                    381  /*  sal_Bool              Text effect (FontWork) supported             */
#define ESCHER_Prop_fFillShadeShapeOK           382  /*  sal_Bool                                                          */
#define ESCHER_Prop_fFillOK                     383  /*  sal_Bool              OK to fill the shape through the UI or VBA? */
// FillStyle
#define ESCHER_Prop_fillType                    384  /*  ESCHER_FillStyle  Type of fill                                  */
#define ESCHER_Prop_fillColor                   385  /*  MSOCLR            Foreground color                              */
#define ESCHER_Prop_fillOpacity                 386  /*  LONG              Fixed 16.16                                   */
#define ESCHER_Prop_fillBackColor               387  /*  MSOCLR            Background color                              */
#define ESCHER_Prop_fillBackOpacity             388  /*  LONG              Shades only                                   */
#define ESCHER_Prop_fillCrMod                   389  /*  MSOCLR            Modification for BW views                     */
#define ESCHER_Prop_fillBlip                    390  /*  IMsoBlip*         Pattern/texture                               */
#define ESCHER_Prop_fillBlipName                391  /*  WCHAR*            Blip file name                                */
#define ESCHER_Prop_fillBlipFlags               392  /*  MSOBLIPFLAGS      Blip flags                                    */
#define ESCHER_Prop_fillWidth                   393  /*  LONG              How big (A units) to make a metafile texture. */
#define ESCHER_Prop_fillHeight                  394  /*  LONG                                                            */
#define ESCHER_Prop_fillAngle                   395  /*  LONG              Fade angle - degrees in 16.16                 */
#define ESCHER_Prop_fillFocus                   396  /*  LONG              Linear shaded fill focus percent              */
#define ESCHER_Prop_fillToLeft                  397  /*  LONG              Fraction 16.16                                */
#define ESCHER_Prop_fillToTop                   398  /*  LONG              Fraction 16.16                                */
#define ESCHER_Prop_fillToRight                 399  /*  LONG              Fraction 16.16                                */
#define ESCHER_Prop_fillToBottom                400  /*  LONG              Fraction 16.16                                */
#define ESCHER_Prop_fillRectLeft                401  /*  LONG              For shaded fills, use the specified rectangle */
#define ESCHER_Prop_fillRectTop                 402  /*  LONG                instead of the shape's bounding rect to     */
#define ESCHER_Prop_fillRectRight               403  /*  LONG                define how large the fade is going to be.   */
#define ESCHER_Prop_fillRectBottom              404  /*  LONG                                                            */
#define ESCHER_Prop_fillDztype                  405  /*  MSODZTYPE                                                       */
#define ESCHER_Prop_fillShadePreset             406  /*  LONG              Special shades                                */
#define ESCHER_Prop_fillShadeColors             407  /*  IMsoArray         a preset array of colors                      */
#define ESCHER_Prop_fillOriginX                 408  /*  LONG                                                            */
#define ESCHER_Prop_fillOriginY                 409  /*  LONG                                                            */
#define ESCHER_Prop_fillShapeOriginX            410  /*  LONG                                                            */
#define ESCHER_Prop_fillShapeOriginY            411  /*  LONG                                                            */
#define ESCHER_Prop_fillShadeType               412  /*  MSOSHADETYPE      Type of shading, if a shaded (gradient) fill. */
#define ESCHER_Prop_fFilled                     443  /*  sal_Bool              Is shape filled?                              */
#define ESCHER_Prop_fHitTestFill                444  /*  sal_Bool              Should we hit test fill?                      */
#define ESCHER_Prop_fillShape                   445  /*  sal_Bool              Register pattern on shape                     */
#define ESCHER_Prop_fillUseRect                 446  /*  sal_Bool              Use the large rect?                           */
#define ESCHER_Prop_fNoFillHitTest              447  /*  sal_Bool              Hit test a shape as though filled             */
// LineStyle
#define ESCHER_Prop_lineColor                   448  /*  MSOCLR            Color of line                              */
#define ESCHER_Prop_lineOpacity                 449  /*  LONG              Not implemented                            */
#define ESCHER_Prop_lineBackColor               450  /*  MSOCLR            Background color                           */
#define ESCHER_Prop_lineCrMod                   451  /*  MSOCLR            Modification for BW views                  */
#define ESCHER_Prop_lineType                    452  /*  MSOLINETYPE       Type of line                               */
#define ESCHER_Prop_lineFillBlip                453  /*  IMsoBlip*         Pattern/texture                            */
#define ESCHER_Prop_lineFillBlipName            454  /*  WCHAR*            Blip file name                             */
#define ESCHER_Prop_lineFillBlipFlags           455  /*  MSOBLIPFLAGS      Blip flags                                 */
#define ESCHER_Prop_lineFillWidth               456  /*  LONG              How big (A units) to make                  */
#define ESCHER_Prop_lineFillHeight              457  /*  LONG                a metafile texture.                      */
#define ESCHER_Prop_lineFillDztype              458  /*  MSODZTYPE         How to interpret fillWidth/Height numbers. */
#define ESCHER_Prop_lineWidth                   459  /*  LONG              A units; 1pt == 12700 EMUs                 */
#define ESCHER_Prop_lineMiterLimit              460  /*  LONG              ratio (16.16) of width                     */
#define ESCHER_Prop_lineStyle                   461  /*  MSOLINESTYLE      Draw parallel lines?                       */
#define ESCHER_Prop_lineDashing                 462  /*  MSOLINEDASHING    Can be overridden by:                      */
#define ESCHER_Prop_lineDashStyle               463  /*  IMsoArray         As Win32 ExtCreatePen                      */
#define ESCHER_Prop_lineStartArrowhead          464  /*  MSOLINEEND        Arrow at start                             */
#define ESCHER_Prop_lineEndArrowhead            465  /*  MSOLINEEND        Arrow at end                               */
#define ESCHER_Prop_lineStartArrowWidth         466  /*  MSOLINEENDWIDTH   Arrow at start                             */
#define ESCHER_Prop_lineStartArrowLength        467  /*  MSOLINEENDLENGTH  Arrow at end                               */
#define ESCHER_Prop_lineEndArrowWidth           468  /*  MSOLINEENDWIDTH   Arrow at start                             */
#define ESCHER_Prop_lineEndArrowLength          469  /*  MSOLINEENDLENGTH  Arrow at end                               */
#define ESCHER_Prop_lineJoinStyle               470  /*  MSOLINEJOIN       How to join lines                          */
#define ESCHER_Prop_lineEndCapStyle             471  /*  MSOLINECAP        How to end lines                           */
#define ESCHER_Prop_fArrowheadsOK               507  /*  sal_Bool              Allow arrowheads if prop. is set           */
#define ESCHER_Prop_fLine                       508  /*  sal_Bool              Any line?                                  */
#define ESCHER_Prop_fHitTestLine                509  /*  sal_Bool              Should we hit test lines?                  */
#define ESCHER_Prop_lineFillShape               510  /*  sal_Bool              Register pattern on shape                  */
#define ESCHER_Prop_fNoLineDrawDash             511  /*  sal_Bool              Draw a dashed line if no line              */
// ShadowStyle
#define ESCHER_Prop_shadowType                  512  /*  MSOSHADOWTYPE     Type of effect            */
#define ESCHER_Prop_shadowColor                 513  /*  MSOCLR            Foreground color          */
#define ESCHER_Prop_shadowHighlight             514  /*  MSOCLR            Embossed color            */
#define ESCHER_Prop_shadowCrMod                 515  /*  MSOCLR            Modification for BW views */
#define ESCHER_Prop_shadowOpacity               516  /*  LONG              Fixed 16.16               */
#define ESCHER_Prop_shadowOffsetX               517  /*  LONG              Offset shadow             */
#define ESCHER_Prop_shadowOffsetY               518  /*  LONG              Offset shadow             */
#define ESCHER_Prop_shadowSecondOffsetX         519  /*  LONG              Double offset shadow      */
#define ESCHER_Prop_shadowSecondOffsetY         520  /*  LONG              Double offset shadow      */
#define ESCHER_Prop_shadowScaleXToX             521  /*  LONG              16.16                     */
#define ESCHER_Prop_shadowScaleYToX             522  /*  LONG              16.16                     */
#define ESCHER_Prop_shadowScaleXToY             523  /*  LONG              16.16                     */
#define ESCHER_Prop_shadowScaleYToY             524  /*  LONG              16.16                     */
#define ESCHER_Prop_shadowPerspectiveX          525  /*  LONG              16.16 / weight            */
#define ESCHER_Prop_shadowPerspectiveY          526  /*  LONG              16.16 / weight            */
#define ESCHER_Prop_shadowWeight                527  /*  LONG              scaling factor            */
#define ESCHER_Prop_shadowOriginX               528  /*  LONG                                        */
#define ESCHER_Prop_shadowOriginY               529  /*  LONG                                        */
#define ESCHER_Prop_fShadow                     574  /*  sal_Bool              Any shadow?               */
#define ESCHER_Prop_fshadowObscured             575  /*  sal_Bool              Excel5-style shadow       */
// PerspectiveStyle
#define ESCHER_Prop_perspectiveType             576  /*  MSOXFORMTYPE      Where transform applies        */
#define ESCHER_Prop_perspectiveOffsetX          577  /*  LONG              The LONG values define a       */
#define ESCHER_Prop_perspectiveOffsetY          578  /*  LONG                transformation matrix,       */
#define ESCHER_Prop_perspectiveScaleXToX        579  /*  LONG                effectively, each value      */
#define ESCHER_Prop_perspectiveScaleYToX        580  /*  LONG                is scaled by the             */
#define ESCHER_Prop_perspectiveScaleXToY        581  /*  LONG                perspectiveWeight parameter. */
#define ESCHER_Prop_perspectiveScaleYToY        582  /*  LONG                                             */
#define ESCHER_Prop_perspectivePerspectiveX     583  /*  LONG                                             */
#define ESCHER_Prop_perspectivePerspectiveY     584  /*  LONG                                             */
#define ESCHER_Prop_perspectiveWeight           585  /*  LONG              Scaling factor                 */
#define ESCHER_Prop_perspectiveOriginX          586  /*  LONG                                             */
#define ESCHER_Prop_perspectiveOriginY          587  /*  LONG                                             */
#define ESCHER_Prop_fPerspective                639  /*  sal_Bool              On/off                         */
// 3D Object
#define ESCHER_Prop_c3DSpecularAmt              640  /*  LONG         Fixed-point 16.16                                                                                                   */
#define ESCHER_Prop_c3DDiffuseAmt               641  /*  LONG         Fixed-point 16.16                                                                                                   */
#define ESCHER_Prop_c3DShininess                642  /*  LONG         Default gives OK results                                                                                            */
#define ESCHER_Prop_c3DEdgeThickness            643  /*  LONG         Specular edge thickness                                                                                             */
#define ESCHER_Prop_c3DExtrudeForward           644  /*  LONG         Distance of extrusion in EMUs                                                                                       */
#define ESCHER_Prop_c3DExtrudeBackward          645  /*  LONG                                                                                                                             */
#define ESCHER_Prop_c3DExtrudePlane             646  /*  LONG         Extrusion direction                                                                                                 */
#define ESCHER_Prop_c3DExtrusionColor           647  /*  MSOCLR       Basic color of extruded part of shape; the lighting model used will determine the exact shades used when rendering. */
#define ESCHER_Prop_c3DCrMod                    648  /*  MSOCLR       Modification for BW views                                                                                           */
#define ESCHER_Prop_f3D                         700  /*  sal_Bool         Does this shape have a 3D effect?                                                                                   */
#define ESCHER_Prop_fc3DMetallic                701  /*  sal_Bool         Use metallic specularity?                                                                                           */
#define ESCHER_Prop_fc3DUseExtrusionColor       702  /*  sal_Bool                                                                                                                             */
#define ESCHER_Prop_fc3DLightFace               703  /*  sal_Bool                                                                                                                             */
// 3D Style
#define ESCHER_Prop_c3DYRotationAngle           704  /*  LONG            degrees (16.16) about y axis         */
#define ESCHER_Prop_c3DXRotationAngle           705  /*  LONG            degrees (16.16) about x axis         */
#define ESCHER_Prop_c3DRotationAxisX            706  /*  LONG            These specify the rotation axis;     */
#define ESCHER_Prop_c3DRotationAxisY            707  /*  LONG              only their relative magnitudes     */
#define ESCHER_Prop_c3DRotationAxisZ            708  /*  LONG              matter.                            */
#define ESCHER_Prop_c3DRotationAngle            709  /*  LONG            degrees (16.16) about axis           */
#define ESCHER_Prop_c3DRotationCenterX          710  /*  LONG            rotation center x (16.16 or g-units) */
#define ESCHER_Prop_c3DRotationCenterY          711  /*  LONG            rotation center y (16.16 or g-units) */
#define ESCHER_Prop_c3DRotationCenterZ          712  /*  LONG            rotation center z (absolute (emus))  */
#define ESCHER_Prop_c3DRenderMode               713  /*  MSO3DRENDERMODE Full,wireframe, or bcube             */
#define ESCHER_Prop_c3DTolerance                714  /*  LONG            pixels (16.16)                       */
#define ESCHER_Prop_c3DXViewpoint               715  /*  LONG            X view point (emus)                  */
#define ESCHER_Prop_c3DYViewpoint               716  /*  LONG            Y view point (emus)                  */
#define ESCHER_Prop_c3DZViewpoint               717  /*  LONG            Z view distance (emus)               */
#define ESCHER_Prop_c3DOriginX                  718  /*  LONG                                                 */
#define ESCHER_Prop_c3DOriginY                  719  /*  LONG                                                 */
#define ESCHER_Prop_c3DSkewAngle                720  /*  LONG            degree (16.16) skew angle            */
#define ESCHER_Prop_c3DSkewAmount               721  /*  LONG            Percentage skew amount               */
#define ESCHER_Prop_c3DAmbientIntensity         722  /*  LONG            Fixed point intensity                */
#define ESCHER_Prop_c3DKeyX                     723  /*  LONG            Key light source direc-              */
#define ESCHER_Prop_c3DKeyY                     724  /*  LONG            tion; only their relative            */
#define ESCHER_Prop_c3DKeyZ                     725  /*  LONG            magnitudes matter                    */
#define ESCHER_Prop_c3DKeyIntensity             726  /*  LONG            Fixed point intensity                */
#define ESCHER_Prop_c3DFillX                    727  /*  LONG            Fill light source direc-             */
#define ESCHER_Prop_c3DFillY                    728  /*  LONG            tion; only their relative            */
#define ESCHER_Prop_c3DFillZ                    729  /*  LONG            magnitudes matter                    */
#define ESCHER_Prop_c3DFillIntensity            730  /*  LONG            Fixed point intensity                */
#define ESCHER_Prop_fc3DConstrainRotation       763  /*  sal_Bool                                                 */
#define ESCHER_Prop_fc3DRotationCenterAuto      764  /*  sal_Bool                                                 */
#define ESCHER_Prop_fc3DParallel                765  /*  sal_Bool            Parallel projection?                 */
#define ESCHER_Prop_fc3DKeyHarsh                766  /*  sal_Bool            Is key lighting harsh?               */
#define ESCHER_Prop_fc3DFillHarsh               767  /*  sal_Bool            Is fill lighting harsh?              */
// Shape
#define ESCHER_Prop_hspMaster                   769  /*  MSOHSP          master shape                                        */
#define ESCHER_Prop_cxstyle                     771  /*  MSOCXSTYLE      Type of connector                                   */
#define ESCHER_Prop_bWMode                      772  /*  ESCHERwMode     Settings for modifications to                       */
#define ESCHER_Prop_bWModePureBW                773  /*  ESCHERwMode     be made when in different                         */
#define ESCHER_Prop_bWModeBW                    774  /*  ESCHERwMode     forms of black-and-white mode.                    */
#define ESCHER_Prop_fOleIcon                    826  /*  sal_Bool            For OLE objects, whether the object is in icon form */
#define ESCHER_Prop_fPreferRelativeResize       827  /*  sal_Bool            For UI only. Prefer relative resizing.              */
#define ESCHER_Prop_fLockShapeType              828  /*  sal_Bool            Lock the shape type (don't allow Change Shape)      */
#define ESCHER_Prop_fDeleteAttachedObject       830  /*  sal_Bool                                                                */
#define ESCHER_Prop_fBackground                 831  /*  sal_Bool            If sal_True, this is the background shape.              */
// Callout
#define ESCHER_Prop_spcot                       832  /*  MSOSPCOT        Callout type                                           */
#define ESCHER_Prop_dxyCalloutGap               833  /*  LONG            Distance from box to first point.(EMUs)                */
#define ESCHER_Prop_spcoa                       834  /*  MSOSPCOA        Callout angle                                          */
#define ESCHER_Prop_spcod                       835  /*  MSOSPCOD        Callout drop type                                      */
#define ESCHER_Prop_dxyCalloutDropSpecified     836  /*  LONG            if msospcodSpecified, the actual drop distance         */
#define ESCHER_Prop_dxyCalloutLengthSpecified   837  /*  LONG            if fCalloutLengthSpecified, the actual distance        */
#define ESCHER_Prop_fCallout                    889  /*  sal_Bool            Is the shape a callout?                                */
#define ESCHER_Prop_fCalloutAccentBar           890  /*  sal_Bool            does callout have accent bar                           */
#define ESCHER_Prop_fCalloutTextBorder          891  /*  sal_Bool            does callout have a text border                        */
#define ESCHER_Prop_fCalloutMinusX              892  /*  sal_Bool                                                                   */
#define ESCHER_Prop_fCalloutMinusY              893  /*  sal_Bool                                                                   */
#define ESCHER_Prop_fCalloutDropAuto            894  /*  sal_Bool            If true, then we occasionally invert the drop distance */
#define ESCHER_Prop_fCalloutLengthSpecified     895  /*  sal_Bool            if true, we look at dxyCalloutLengthSpecified          */
// GroupShape
#define ESCHER_Prop_wzName                      896  /*  WCHAR*          Shape Name (present only if explicitly set)                                                            */
#define ESCHER_Prop_wzDescription               897  /*  WCHAR*          alternate text                                                                                         */
#define ESCHER_Prop_pihlShape                   898  /*  IHlink*         The hyperlink in the shape.                                                                            */
#define ESCHER_Prop_pWrapPolygonVertices        899  /*  IMsoArray       The polygon that text will be wrapped around (Word)                                                    */
#define ESCHER_Prop_dxWrapDistLeft              900  /*  LONG            Left wrapping distance from text (Word)                                                                */
#define ESCHER_Prop_dyWrapDistTop               901  /*  LONG            Top wrapping distance from text (Word)                                                                 */
#define ESCHER_Prop_dxWrapDistRight             902  /*  LONG            Right wrapping distance from text (Word)                                                               */
#define ESCHER_Prop_dyWrapDistBottom            903  /*  LONG            Bottom wrapping distance from text (Word)                                                              */
#define ESCHER_Prop_lidRegroup                  904  /*  LONG            Regroup ID                                                                                             */
#define ESCHER_Prop_tableProperties             927
#define ESCHER_Prop_tableRowProperties          928
#define ESCHER_Prop_fEditedWrap                 953  /*  sal_Bool            Has the wrap polygon been edited?                                                                      */
#define ESCHER_Prop_fBehindDocument             954  /*  sal_Bool            Word-only (shape is behind text)                                                                       */
#define ESCHER_Prop_fOnDblClickNotify           955  /*  sal_Bool            Notify client on a double click                                                                        */
#define ESCHER_Prop_fIsButton                   956  /*  sal_Bool            A button shape (i.e., clicking performs an action). Set for shapes with attached hyperlinks or macros. */
#define ESCHER_Prop_fOneD                       957  /*  sal_Bool            1D adjustment                                                                                          */
#define ESCHER_Prop_fHidden                     958  /*  sal_Bool            Do not display                                                                                         */
#define ESCHER_Prop_fPrint                      959  /*  sal_Bool            Print this shape                                                                                       */

#define ESCHER_PERSISTENTRY_PREALLOCATE     64

#define ESCHER_Persist_PrivateEntry         0x80000000
#define ESCHER_Persist_Dgg                  0x00010000
#define ESCHER_Persist_Dg                   0x00020000
#define ESCHER_Persist_CurrentPosition      0x00040000
#define ESCHER_Persist_Grouping_Snap        0x00050000
#define ESCHER_Persist_Grouping_Logic       0x00060000

const sal_uInt32 DFF_DGG_CLUSTER_SIZE       = 0x00000400;   /// Shape IDs per cluster in DGG atom.

// ---------------------------------------------------------------------------------------------

namespace com { namespace sun { namespace star {
    namespace awt {
        struct Gradient;
    }
    namespace drawing {
        struct EnhancedCustomShapeAdjustmentValue;
        class XShape;
        class XShapes;
    }
}}}

struct MSFILTER_DLLPUBLIC EscherConnectorListEntry
{
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >   mXConnector;
    ::com::sun::star::awt::Point            maPointA;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >   mXConnectToA;
    ::com::sun::star::awt::Point            maPointB;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >   mXConnectToB;

    sal_uInt32      GetConnectorRule( sal_Bool bFirst );

                    EscherConnectorListEntry( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rC,
                                        const ::com::sun::star::awt::Point& rPA,
                                        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rSA ,
                                        const ::com::sun::star::awt::Point& rPB,
                                        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rSB ) :
                                            mXConnector ( rC ),
                                            maPointA    ( rPA ),
                                            mXConnectToA( rSA ),
                                            maPointB    ( rPB ),
                                            mXConnectToB( rSB ) {}

    sal_uInt32      GetClosestPoint( const Polygon& rPoly, const ::com::sun::star::awt::Point& rP );
};

struct MSFILTER_DLLPUBLIC EscherExContainer
{
    sal_uInt32  nContPos;
    SvStream&   rStrm;

    EscherExContainer( SvStream& rSt, const sal_uInt16 nRecType, const sal_uInt16 nInstance = 0 );
    ~EscherExContainer();
};

struct MSFILTER_DLLPUBLIC EscherExAtom
{
    sal_uInt32  nContPos;
    SvStream&   rStrm;

    EscherExAtom( SvStream& rSt, const sal_uInt16 nRecType, const sal_uInt16 nInstance = 0, const sal_uInt8 nVersion = 0 );
    ~EscherExAtom();
};

struct EscherPropertyValueHelper
{
    static sal_Bool GetPropertyValue(
        ::com::sun::star::uno::Any& rAny,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > &,
        const String& rPropertyName,
        sal_Bool bTestPropertyAvailability = sal_False
    );

    static ::com::sun::star::beans::PropertyState GetPropertyState(
        const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet > &,
        const String& rPropertyName
    );
};

// ---------------------------------------------------------------------------------------------

struct EscherPersistEntry
{
    sal_uInt32  mnID;
    sal_uInt32  mnOffset;

    EscherPersistEntry( sal_uInt32 nId, sal_uInt32 nOffset ) { mnID = nId; mnOffset = nOffset; };

};

// ---------------------------------------------------------------------------------------------

class EscherBlibEntry
{

    friend class EscherGraphicProvider;
    friend class EscherEx;

protected:

    sal_uInt32      mnIdentifier[ 4 ];
    sal_uInt32      mnPictureOffset;        // offset to the graphic im PictureStreams
    sal_uInt32      mnSize;                 // size of real graphic

    sal_uInt32      mnRefCount;             // !! reference count
    sal_uInt32      mnSizeExtra;            // !! size of preceding header

    ESCHER_BlibType meBlibType;

    Size            maPrefSize;
    MapMode         maPrefMapMode;

    sal_Bool        mbIsEmpty;
    sal_Bool        mbIsNativeGraphicPossible;

public:

                    EscherBlibEntry(
                        sal_uInt32 nPictureOffset,
                        const GraphicObject& rObj,
                        const rtl::OString& rId,
                        const GraphicAttr* pAttr = NULL
                    );

                    ~EscherBlibEntry();

    void            WriteBlibEntry( SvStream& rSt, sal_Bool bWritePictureOffset, sal_uInt32 nResize = 0 );
    sal_Bool        IsEmpty() const { return mbIsEmpty; };

    sal_Bool        operator==( const EscherBlibEntry& ) const;
};

// ---------------------------------------------------------------------------------------------

#define _E_GRAPH_PROV_USE_INSTANCES             1
#define _E_GRAPH_PROV_DO_NOT_ROTATE_METAFILES   2

class MSFILTER_DLLPUBLIC EscherGraphicProvider
{
    sal_uInt32              mnFlags;

    EscherBlibEntry**       mpBlibEntrys;
    sal_uInt32              mnBlibBufSize;
    sal_uInt32              mnBlibEntrys;

    rtl::OUString           maBaseURI;

protected:

    sal_uInt32              ImplInsertBlib( EscherBlibEntry* p_EscherBlibEntry );

public:

    sal_uInt32  GetBlibStoreContainerSize( SvStream* pMergePicStreamBSE = NULL ) const;
    void        WriteBlibStoreContainer( SvStream& rStrm, SvStream* pMergePicStreamBSE = NULL  );
    sal_Bool    WriteBlibStoreEntry(SvStream& rStrm, sal_uInt32 nBlipId,
    sal_Bool    bWritePictureOffset, sal_uInt32 nResize = 0);
    sal_uInt32  GetBlibID(
                    SvStream& rPicOutStream,
                    const rtl::OString& rGraphicId,
                    const Rectangle& rBoundRect,
                    const com::sun::star::awt::Rectangle* pVisArea = NULL,
                    const GraphicAttr* pGrafikAttr = NULL
                );
    sal_Bool    HasGraphics() const { return mnBlibEntrys != 0; };

    void        SetNewBlipStreamOffset( sal_Int32 nOffset );

    sal_Bool    GetPrefSize( const sal_uInt32 nBlibId, Size& rSize, MapMode& rMapMode );

    void        SetBaseURI( const rtl::OUString& rBaseURI ) { maBaseURI = rBaseURI; };
    const rtl::OUString& GetBaseURI() { return maBaseURI; };

                EscherGraphicProvider( sal_uInt32 nFlags = _E_GRAPH_PROV_DO_NOT_ROTATE_METAFILES );
                ~EscherGraphicProvider();

};

struct EscherShapeListEntry;
typedef ::std::vector< EscherShapeListEntry* >      EscherShapeList_impl;
typedef ::std::vector< EscherConnectorListEntry* >  EscherConnectorList_impl;

class MSFILTER_DLLPUBLIC EscherSolverContainer
{
    EscherShapeList_impl        maShapeList;
    EscherConnectorList_impl    maConnectorList;

public:

    sal_uInt32      GetShapeId(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rShape
                    ) const;

    void            AddShape(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > &,
                        sal_uInt32 nId
                    );

    void            AddConnector(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > &,
                        const ::com::sun::star::awt::Point& rA,
                        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > &,
                        const ::com::sun::star::awt::Point& rB,
                        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rConB
                    );

    void            WriteSolver( SvStream& );

                    EscherSolverContainer(){};
                    ~EscherSolverContainer();
};

// ---------------------------------------------------------------------------------------------

#define ESCHER_CREATEPOLYGON_LINE           1
#define ESCHER_CREATEPOLYGON_POLYLINE       2
#define ESCHER_CREATEPOLYGON_POLYPOLYGON    4

class GraphicAttr;
class SdrObjCustomShape;

struct EscherPropSortStruct
{
    sal_uInt8*  pBuf;
    sal_uInt32  nPropSize;
    sal_uInt32  nPropValue;
    sal_uInt16  nPropId;
};

typedef std::vector< EscherPropSortStruct > EscherProperties;

class MSFILTER_DLLPUBLIC EscherPropertyContainer
{
    EscherGraphicProvider*  pGraphicProvider;
    SvStream*               pPicOutStrm;
    Rectangle*              pShapeBoundRect;

    EscherPropSortStruct*   pSortStruct;

    sal_uInt32              nSortCount;
    sal_uInt32              nSortBufSize;
    sal_uInt32              nCountCount;
    sal_uInt32              nCountSize;

    sal_Bool                bHasComplexData;
    sal_Bool                bSuppressRotation;


    sal_uInt32  ImplGetColor( const sal_uInt32 rColor, sal_Bool bSwap = sal_True );
    void        ImplCreateGraphicAttributes(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
                    sal_uInt32 nBlibId,
                    sal_Bool bCreateCroppingAttributes
                );
    sal_Bool    ImplCreateEmbeddedBmp( const rtl::OString& rUniqueId );
    void        ImplInit();

public:

    EscherPropertyContainer();
    EscherPropertyContainer(
        EscherGraphicProvider& rGraphicProvider,    // the PropertyContainer needs to know
        SvStream* pPicOutStrm,                      // the GraphicProvider to be able to write
        Rectangle& rShapeBoundRect                  // FillBitmaps or GraphicObjects.
    );                                              // under some circumstances the ShapeBoundRect
                                                    // is adjusted this will happen when rotated
                                                    // GraphicObjects are saved to PowerPoint
    ~EscherPropertyContainer();

    void        AddOpt( sal_uInt16 nPropertyID, const rtl::OUString& rString );

    void        AddOpt(
                    sal_uInt16 nPropertyID,
                    sal_uInt32 nPropValue,
                    sal_Bool bBlib = sal_False
                );

    void        AddOpt(
                    sal_uInt16 nPropertyID,
                    sal_Bool bBlib,
                    sal_uInt32 nPropValue,
                    sal_uInt8* pProp,
                    sal_uInt32 nPropSize
                );

    sal_Bool    GetOpt( sal_uInt16 nPropertyID, sal_uInt32& rPropValue ) const;

    sal_Bool    GetOpt( sal_uInt16 nPropertyID, EscherPropSortStruct& rPropValue ) const;

    EscherProperties GetOpts() const;

    void        Commit( SvStream& rSt, sal_uInt16 nVersion = 3, sal_uInt16 nRecType = ESCHER_OPT );

    sal_Bool    CreateShapeProperties(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rXShape
                );
    sal_Bool    CreateOLEGraphicProperties(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rXOleObject
                );

    /** Creates a complex ESCHER_Prop_fillBlip containing the BLIP directly (for Excel charts). */
    sal_Bool    CreateEmbeddedBitmapProperties(
                    const ::rtl::OUString& rBitmapUrl,
                    ::com::sun::star::drawing::BitmapMode eBitmapMode
                );
    /** Creates a complex ESCHER_Prop_fillBlip containing a hatch style (for Excel charts). */
    sal_Bool    CreateEmbeddedHatchProperties(
                    const ::com::sun::star::drawing::Hatch& rHatch,
                    const Color& rBackColor,
                    bool bFillBackground
                );

                    // the GraphicProperties will only be created if a GraphicProvider and PicOutStrm is known
                    // DR: #99897# if no GraphicProvider is present, a complex ESCHER_Prop_fillBlip
                    //             will be created, containing the BLIP directly (e.g. for Excel charts).
    sal_Bool    CreateGraphicProperties(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
                    const String& rSource,
                    const sal_Bool bCreateFillBitmap,
                    const sal_Bool bCreateCroppingAttributes = sal_False,
                    const sal_Bool bFillBitmapModeAllowed = sal_True
                );

    sal_Bool    CreatePolygonProperties(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
                    sal_uInt32 nFlags,
                    sal_Bool bBezier,
                    ::com::sun::star::awt::Rectangle& rGeoRect,
                    Polygon* pPolygon = NULL
                );

    static sal_uInt32 GetGradientColor(
                    const ::com::sun::star::awt::Gradient* pGradient,
                    sal_uInt32 nStartColor
                );

    void        CreateGradientProperties( const ::com::sun::star::awt::Gradient & rGradient );
    void        CreateGradientProperties(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > &
                );
    void        CreateLineProperties(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > &,
                    sal_Bool bEdge
                );
    void        CreateFillProperties( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > &, sal_Bool bEdge );
    void        CreateTextProperties(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > &,
                    sal_uInt32 nText,
                    const sal_Bool bIsCustomShape = sal_False,
                    const sal_Bool bIsTextFrame = sal_True
                );

    sal_Bool    CreateConnectorProperties(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rXShape,
                    EscherSolverContainer& rSolver,
                    ::com::sun::star::awt::Rectangle& rGeoRect,
                    sal_uInt16& rShapeType,
                    sal_uInt16& rShapeFlags
                );

                // Because shadow properties depends to the line and fillstyle, the CreateShadowProperties method should be called at last.
                // It activ only when at least a FillStyle or LineStyle is set.
    sal_Bool    CreateShadowProperties(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > &
                );

        // creates all necessary CustomShape properties, this includes also Text-, Shadow-, Fill-, and LineProperties
    void        CreateCustomShapeProperties(
                    const MSO_SPT eShapeType,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > &
                );
    sal_Bool    IsFontWork() const;

    // helper functions which are also used by the escher import
    static PolyPolygon  GetPolyPolygon(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rXShape
                        );
    static PolyPolygon  GetPolyPolygon( const ::com::sun::star::uno::Any& rSource );
    static MSO_SPT      GetCustomShapeType(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rXShape,
                            sal_uInt32& nMirrorFlags
                        );
    static MSO_SPT      GetCustomShapeType(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rXShape,
                            sal_uInt32& nMirrorFlags,
                            rtl::OUString& rShapeType
                        );

    // helper functions which are also used in ooxml export
    static sal_Bool     GetLineArrow(
                            const sal_Bool bLineStart,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
                            ESCHER_LineEnd& reLineEnd,
                            sal_Int32& rnArrowLength,
                            sal_Int32& rnArrowWidth
                        );
    static sal_Bool     IsDefaultObject( SdrObjCustomShape* pCustoShape );
    static void         LookForPolarHandles(
                            const MSO_SPT eShapeType,
                            sal_Int32& nAdjustmentsWhichNeedsToBeConverted
                        );
    static sal_Bool     GetAdjustmentValue( const com::sun::star::drawing::EnhancedCustomShapeAdjustmentValue & rkProp, sal_Int32 nIndex, sal_Int32 nAdjustmentsWhichNeedsToBeConverted, sal_Int32& nValue );
};

// ---------------------------------------------------------------------------------------------

typedef ::std::vector< EscherPersistEntry* > EscherPersistTable_impl;

class MSFILTER_DLLPUBLIC EscherPersistTable
{

public:
    EscherPersistTable_impl maPersistTable;

    sal_Bool    PtIsID( sal_uInt32 nID );
    void        PtInsert( sal_uInt32 nID, sal_uInt32 nOfs );
    sal_uInt32  PtDelete( sal_uInt32 nID );
    sal_uInt32  PtGetOffsetByID( sal_uInt32 nID );
    sal_uInt32  PtReplace( sal_uInt32 nID, sal_uInt32 nOfs );
    sal_uInt32  PtReplaceOrInsert( sal_uInt32 nID, sal_uInt32 nOfs );
    sal_uInt32  PtGetCount() const { return maPersistTable.size(); };

                EscherPersistTable();
    virtual     ~EscherPersistTable();
};

// ---------------------------------------------------------------------------------------------

class EscherEx;

/// abstract base class for ESCHER_ClientTextbox, ESCHER_ClientData
class MSFILTER_DLLPUBLIC EscherExClientRecord_Base
{
public:
                                EscherExClientRecord_Base() {}
    virtual                     ~EscherExClientRecord_Base();

                                /// Application writes the record header
                                /// using rEx.AddAtom(...) followed by
                                /// record data written to rEx.GetStream()
    virtual void                WriteData( EscherEx& rEx ) const = 0;
};


/// abstract base class for ESCHER_ClientAnchor
class MSFILTER_DLLPUBLIC EscherExClientAnchor_Base
{
public:
                                EscherExClientAnchor_Base() {}
    virtual                     ~EscherExClientAnchor_Base();

                                /// Application writes the record header
                                /// using rEx.AddAtom(...) followed by
                                /// record data written to rEx.GetStream()
    virtual void                WriteData( EscherEx& rEx,
                                    const Rectangle& rRect ) = 0;
};

class InteractionInfo
{
    bool mbHasInteraction;
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr<SvMemoryStream>       mpHyperlinkRecord;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    InteractionInfo();

public:
    InteractionInfo( SvMemoryStream* pStream, bool bInteraction ) : mbHasInteraction( bInteraction )
    {
        mpHyperlinkRecord.reset( pStream );
    }
    bool    hasInteraction() { return mbHasInteraction; }
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    const std::auto_ptr< SvMemoryStream >&  getHyperlinkRecord() { return mpHyperlinkRecord; }
    SAL_WNODEPRECATED_DECLARATIONS_POP
};

class EscherExHostAppData
{
private:
        EscherExClientAnchor_Base*  pClientAnchor;
        EscherExClientRecord_Base*  pClientData;
        EscherExClientRecord_Base*  pClientTextbox;
        InteractionInfo*        pInteractionInfo;
        // ignore single shape if entire pages are written
        sal_Bool                        bDontWriteShape;

public:
        EscherExHostAppData() : pClientAnchor(0), pClientData(0),
                pClientTextbox(0), pInteractionInfo(0), bDontWriteShape(sal_False)
        {}

        void SetInteractionInfo( InteractionInfo* p )
            { pInteractionInfo = p; }
        void SetClientAnchor( EscherExClientAnchor_Base* p )
            { pClientAnchor = p; }
        void SetClientData( EscherExClientRecord_Base* p )
            { pClientData = p; }
        void SetClientTextbox( EscherExClientRecord_Base* p )
            { pClientTextbox = p; }
        void SetDontWriteShape( sal_Bool b )
            { bDontWriteShape = b; }
        InteractionInfo* GetInteractionInfo() const
            { return pInteractionInfo; }
        EscherExClientAnchor_Base* GetClientAnchor() const
            { return pClientAnchor; }
        EscherExClientRecord_Base* GetClientData() const
            { return pClientData; }
        EscherExClientRecord_Base* GetClientTextbox() const
            { return pClientTextbox; }

        void WriteClientAnchor( EscherEx& rEx, const Rectangle& rRect )
            { if( pClientAnchor )  pClientAnchor->WriteData( rEx, rRect ); }
        void WriteClientData( EscherEx& rEx )
            { if( pClientData ) pClientData->WriteData( rEx ); }
        void WriteClientTextbox( EscherEx& rEx )
            { if( pClientTextbox ) pClientTextbox->WriteData( rEx ); }

        sal_Bool DontWriteShape() const { return bDontWriteShape; }
};


// ============================================================================

/** Instance for global DFF data, shared through various instances of EscherEx. */
class MSFILTER_DLLPUBLIC EscherExGlobal : public EscherGraphicProvider
{
public:
    explicit            EscherExGlobal( sal_uInt32 nGraphicProvFlags = _E_GRAPH_PROV_DO_NOT_ROTATE_METAFILES );
    virtual             ~EscherExGlobal();

    /** Returns a new drawing ID for a new drawing container (DGCONTAINER). */
    sal_uInt32          GenerateDrawingId();
    /** Creates and returns a new shape identifier, updates the internal shape
        counters and registers the identifier in the DGG cluster table.
        @param nDrawingId  Drawing identifier has to be passed to be able to
            generate shape identifiers for multiple drawings simultaniously. */
    sal_uInt32          GenerateShapeId( sal_uInt32 nDrawingId, bool bIsInSpgr );
    /** Returns the number of shapes in the current drawing, based on number of
        calls to the GenerateShapeId() function. */
    sal_uInt32          GetDrawingShapeCount( sal_uInt32 nDrawingId ) const;
    /** Returns the last shape identifier generated by the GenerateShapeId()
        function. */
    sal_uInt32          GetLastShapeId( sal_uInt32 nDrawingId ) const;

    /** Sets the flag indicating that the DGGCONTAINER exists. */
    inline void         SetDggContainer() { mbHasDggCont = true; }
    /** Sets the flag indicating that the DGGCONTAINER exists. */
    inline bool         HasDggContainer() const { return mbHasDggCont; }
    /** Returns the total size of the DGG atom (including header). */
    sal_uInt32          GetDggAtomSize() const;
    /** Writes the complete DGG atom to the passed stream (overwrites existing data!). */
    void                WriteDggAtom( SvStream& rStrm ) const;

    /** Called if a picture shall be written and no picture stream is set at
        class ImplEscherExSdr.

        On first invokation, this function calls the virtual member function
        ImplQueryPictureStream(). The return value will be cached internally
        for subsequent calls and for the GetPictureStream() function.
     */
    SvStream*           QueryPictureStream();

    /** Returns the picture stream if existing (queried), otherwise null. */
    inline SvStream*    GetPictureStream() { return mpPicStrm; }

private:
    /** Derived classes may implement to create a new stream used to store the
        picture data.

        The implementation has to take care about lifetime of the returned
        stream (it will not be destructed automatically). This function is
        called exactly once. The return value will be cached internally for
        repeated calls of the public QueryPictureStream() function.
     */
    virtual SvStream*   ImplQueryPictureStream();

private:
    struct ClusterEntry
    {
        sal_uInt32          mnDrawingId;        /// Identifier of drawing this cluster belongs to (one-based index into maDrawingInfos).
        sal_uInt32          mnNextShapeId;      /// Next free shape identifier in this cluster.
        inline explicit     ClusterEntry( sal_uInt32 nDrawingId ) : mnDrawingId( nDrawingId ), mnNextShapeId( 0 ) {}
    };
    typedef ::std::vector< ClusterEntry > ClusterTable;

    struct DrawingInfo
    {
        sal_uInt32          mnClusterId;        /// Currently used cluster (one-based index into maClusterTable).
        sal_uInt32          mnShapeCount;       /// Current number of shapes in this drawing.
        sal_uInt32          mnLastShapeId;      /// Last shape identifier generated for this drawing.
        inline explicit     DrawingInfo( sal_uInt32 nClusterId ) : mnClusterId( nClusterId ), mnShapeCount( 0 ), mnLastShapeId( 0 ) {}
    };
    typedef ::std::vector< DrawingInfo > DrawingInfoVector;

    ClusterTable        maClusterTable;     /// List with cluster IDs (used object IDs in drawings).
    DrawingInfoVector   maDrawingInfos;     /// Data about all used drawings.
    SvStream*           mpPicStrm;          /// Cached result of ImplQueryPictureStream().
    bool                mbHasDggCont;       /// True = the DGGCONTAINER has been initialized.
    bool                mbPicStrmQueried;   /// True = ImplQueryPictureStream() has been called.
};

typedef ::boost::shared_ptr< EscherExGlobal > EscherExGlobalRef;

// ---------------------------------------------------------------------------------------------

class SdrObject;
class SdrPage;
class ImplEscherExSdr;

class MSFILTER_DLLPUBLIC EscherEx : public EscherPersistTable
{
    protected:
        typedef ::std::auto_ptr< ImplEscherExSdr > ImplEscherExSdrPtr;

        EscherExGlobalRef           mxGlobal;
        ImplEscherExSdrPtr          mpImplEscherExSdr;
        SvStream*                   mpOutStrm;
        bool                        mbOwnsStrm;
        sal_uInt32                  mnStrmStartOfs;
        std::vector< sal_uInt32 >   mOffsets;
        std::vector< sal_uInt16 >   mRecTypes;

        sal_uInt32                  mnCurrentDg;
        sal_uInt32                  mnCountOfs;

        sal_uInt32                  mnGroupLevel;
        sal_uInt16                  mnHellLayerId;

        sal_Bool                    mbEscherSpgr;
        sal_Bool                    mbEscherDg;
        sal_Bool                    mbOleEmf;                   // OLE is EMF instead of WMF


        virtual sal_Bool DoSeek( sal_uInt32 nKey );

public:
    explicit            EscherEx( const EscherExGlobalRef& rxGlobal, SvStream* pOutStrm );
    virtual             ~EscherEx();

    /** Creates and returns a new shape identifier, updates the internal shape
        counters and registers the identifier in the DGG cluster table. */
    inline sal_uInt32   GenerateShapeId() { return mxGlobal->GenerateShapeId( mnCurrentDg, mbEscherSpgr ); }

    /** Returns the graphic provider from the global object that has been
        passed to the constructor.
     */
    inline EscherGraphicProvider& GetGraphicProvider() { return *mxGlobal; }

    /** Called if a picture shall be written and no picture stream is set at
        class ImplEscherExSdr.
     */
    inline SvStream*    QueryPictureStream() { return mxGlobal->QueryPictureStream(); }
    
                /// Inserts internal data into the EscherStream, this process
                /// may and has to be executed only once
                /// If pPicStreamMergeBSE is known, the BLIPs from this stream are being
                /// merged into the MsofbtBSE Records of the EscherStream like it's
                /// required for Excel (and maybe Word?)
        virtual void Flush( SvStream* pPicStreamMergeBSE = NULL );

    /** Inserts the passed number of bytes at the current position of the
        output stream.

        Inserts dummy bytes and moves all following stream data, and updates
        all internal stream offsets stored in the PersistTable and the affected
        container sizes, which makes this operation very expensive. (!)

        @param nBytes  The number of bytes to be inserted into the stream.

        @param bExpandEndOfAtom  If set to true, an atom that currently ends
            exactly at the current stream position will be expanded to include
            the inserted data. If set to false, an atom that currently ends
            exactly at the current stream position will not be expanded to
            include the inserted data (used to insert e.g. a new atom after an
            existing atom). Note that containers that end exactly at the
            current stream position are always expanded to include the inserted
            data.
     */
    void            InsertAtCurrentPos( sal_uInt32 nBytes, bool bExpandEndOfAtom );

    void            InsertPersistOffset( sal_uInt32 nKey, sal_uInt32 nOffset ); // It is not being checked if this key is already in the PersistantTable
    void            ReplacePersistOffset( sal_uInt32 nKey, sal_uInt32 nOffset );
    sal_uInt32      GetPersistOffset( sal_uInt32 nKey );
    sal_Bool        SeekToPersistOffset( sal_uInt32 nKey );
    virtual sal_Bool InsertAtPersistOffset( sal_uInt32 nKey, sal_uInt32 nValue );   // nValue is being inserted into the Stream where it's appropriate (overwrite modus), without that the
                                                                                    // current StreamPosition changes

    SvStream&       GetStream() const   { return *mpOutStrm; }
    sal_uLong       GetStreamPos() const    { return mpOutStrm->Tell(); }

    virtual sal_Bool SeekBehindRecHeader( sal_uInt16 nRecType );                // the stream has to be in front of a valid Record Headeror Atom

                // features during the creation of the following Containers:
                //
                //      ESCHER_DggContainer:    a EscherDgg Atom is automatically being created and managed
                //      ESCHER_DgContainer:     a EscherDg Atom is automatically being created and managed
                //      ESCHER_SpgrContainer:
                //      ESCHER_SpContainer:

    virtual void OpenContainer( sal_uInt16 nEscherContainer, int nRecInstance = 0 );
    virtual void CloseContainer();

    virtual void BeginAtom();
    virtual void EndAtom( sal_uInt16 nRecType, int nRecVersion = 0, int nRecInstance = 0 );
    virtual void AddAtom( sal_uInt32 nAtomSitze, sal_uInt16 nRecType, int nRecVersion = 0, int nRecInstance = 0 );
    virtual void AddChildAnchor( const Rectangle& rRectangle );
    virtual void AddClientAnchor( const Rectangle& rRectangle );

    virtual sal_uInt32 EnterGroup( const String& rShapeName, const Rectangle* pBoundRect = 0 );
    sal_uInt32  EnterGroup( const Rectangle* pBoundRect = NULL );
    sal_uInt32  GetGroupLevel() const { return mnGroupLevel; };
    virtual sal_Bool SetGroupSnapRect( sal_uInt32 nGroupLevel, const Rectangle& rRect );
    virtual sal_Bool SetGroupLogicRect( sal_uInt32 nGroupLevel, const Rectangle& rRect );
    virtual void LeaveGroup();

                // a ESCHER_Sp is being written ( a ESCHER_DgContainer has to be opened for this purpose!)
    virtual void AddShape( sal_uInt32 nShpInstance, sal_uInt32 nFlagIds, sal_uInt32 nShapeID = 0 );

    virtual void Commit( EscherPropertyContainer& rProps, const Rectangle& rRect );

    sal_uInt32  GetColor( const sal_uInt32 nColor, sal_Bool bSwap = sal_True );
    sal_uInt32  GetColor( const Color& rColor, sal_Bool bSwap = sal_True );

                // ...Sdr... implemented in eschesdo.cxx

    void    AddSdrPage( const SdrPage& rPage );
    void    AddUnoShapes( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes );

                /// returns the ShapeID
    sal_uInt32  AddSdrObject( const SdrObject& rObj );

                /// If objects are written through AddSdrObject the
                /// SolverContainer has to be written, and maybe some
                /// maintenance to be done.
    void    EndSdrObjectPage();

                /// Called before a shape is written, application supplies
                /// ClientRecords. May set AppData::bDontWriteShape so the
                /// shape is ignored.
    virtual EscherExHostAppData* StartShape(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rShape,
                            const Rectangle* pChildAnchor );

                /// Called after a shape is written to inform the application
                /// of the resulted shape type and ID.
    virtual void    EndShape( sal_uInt16 nShapeType, sal_uInt32 nShapeID );

                /// Called before an AdditionalText EnterGroup occurs.
                /// The current shape will be written in three parts:
                /// a group shape, the shape itself, and an extra textbox shape.
                /// The complete flow is:
                /// StartShape sets HostData1.
                /// EnterAdditionalTextGroup sets HostData2, App may modify
                ///   HostData1 and keep track of the change.
                /// The group shape is written with HostData2.
                /// Another StartShape with the same (!) object sets HostData3.
                /// The current shape is written with HostData3.
                /// EndShape is called for the current shape.
                /// Another StartShape with the same (!) object sets HostData4.
                /// The textbox shape is written with HostData4.
                /// EndShape is called for the textbox shape.
                /// EndShape is called for the group shape, this provides
                ///   the same functionality as an ordinary recursive group.
    virtual EscherExHostAppData*    EnterAdditionalTextGroup();

                /// Called if an ESCHER_Prop_lTxid shall be written
    virtual sal_uInt32  QueryTextID( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >&, sal_uInt32 nShapeId );
            // add an dummy rectangle shape into the escher stream
        sal_uInt32  AddDummyShape();

    static const SdrObject* GetSdrObject( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rXShape );

    void SetHellLayerId( sal_uInt16 nId )       { mnHellLayerId = nId; }
    sal_uInt16 GetHellLayerId() const           { return mnHellLayerId; }

private:
                        EscherEx( const EscherEx& );
    EscherEx&           operator=( const EscherEx& );

    // prevent C-style cast to former base class EscherGraphicProvider
    operator EscherGraphicProvider&();
    operator EscherGraphicProvider const&();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
