/*************************************************************************
 *
 *  $RCSfile: escherex.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __EscherEX_HXX
#define __EscherEX_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_GRADIENT_HPP_
#include <com/sun/star/awt/Gradient.hpp>
#endif

        /*Record Name       FBT-Value   Instance                  Contents                                                          Wrd Exl PPt Ver*/
// In der Mickysoft-Doku heissen die msofbt... statt _Escher_...
#define _Escher_DggContainer      0xF000 /*                           per-document data                                                  X   X   X     */
#define   _Escher_Dgg             0xF006 /*                           an FDGG and several FIDCLs                                         X   X   X   0 */
#define   _Escher_CLSID           0xF016 /*                           the CLSID of the application that put the data on the clipboard    C   C   C   0 */
#define   _Escher_OPT             0xF00B /* count of properties       the document-wide default shape properties                         X   X   X   3 */
#define   _Escher_ColorMRU        0xF11A /* count of colors           the colors in the MRU swatch                                       X   X   X   0 */
#define   _Escher_SplitMenuColors 0xF11E /* count of colors           the colors in the top-level split menus                            X   X   X   0 */
#define   _Escher_BstoreContainer 0xF001 /* count of BLIPs            all images in the document (JPEGs, metafiles, etc.)                X   X   X     */
#define     _Escher_BSE           0xF007 /* BLIP type                 an FBSE (one per BLIP)                                             X   X   X   2 */
#define     _Escher_BlipFirst     0xF018 /*                           range of fbts reserved for various kinds of BLIPs                  X   X   X     */
#define     _Escher_BlipLast      0xF117 /*                           range of fbts reserved for various kinds of BLIPs                  X   X   X     */

#define _Escher_DgContainer       0xF002 /*                           per-sheet/page/slide data                                          X   X   X     */
#define   _Escher_Dg              0xF008 /* drawing ID                an FDG                                                             X   X   X   0 */
#define   _Escher_RegroupItems    0xF118 /* count of regroup entries  several FRITs                                                      X   X   X   0 */
#define   _Escher_ColorScheme     0xF120 /* count of colors           the colors of the source host's color scheme                           C   C   0 */
#define   _Escher_SpgrContainer   0xF003 /*                           several SpContainers, the first of which is the group shape itself X   X   X     */
#define   _Escher_SpContainer     0xF004 /*                           a shape                                                            X   X   X     */
#define     _Escher_Spgr          0xF009 /*                           an FSPGR; only present if the shape is a group shape               X   X   X   1 */
#define     _Escher_Sp            0xF00A /* shape type                an FSP                                                             X   X   X   2 */
//#define     _Escher_OPT           0xF00B /* count of properties       a shape property table                                             X   X   X   3 */
#define     _Escher_Textbox       0xF00C /*                           RTF text                                                           C   C   C   0 */
#define     _Escher_ClientTextbox 0xF00D /* host-defined              the text in the textbox, in host-defined format                    X   X   X     */
#define     _Escher_Anchor        0xF00E /*                           a RECT, in 100000ths of an inch                                    C   C   C   0 */
#define     _Escher_ChildAnchor   0xF00F /*                           a RECT, in units relative to the parent group                      X   X   X   0 */
#define     _Escher_ClientAnchor  0xF010 /* host-defined              the location of the shape, in a host-defined format                X   X   X     */
#define     _Escher_ClientData    0xF011 /* host-defined              host-specific data                                                 X   X   X     */
#define     _Escher_OleObject     0xF11F /*                           a serialized IStorage for an OLE object                            C   C   C   0 */
#define     _Escher_DeletedPspl   0xF11D /*                           an FPSPL; only present in top-level deleted shapes                 X           0 */
#define   _Escher_SolverContainer 0xF005 /* count of rules            the rules governing shapes                                         X   X   X     */
#define     _Escher_ConnectorRule 0xF012 /*                           an FConnectorRule                                                      X   X   1 */
#define     _Escher_AlignRule     0xF013 /*                           an FAlignRule                                                      X   X   X   0 */
#define     _Escher_ArcRule       0xF014 /*                           an FARCRU                                                          X   X   X   0 */
#define     _Escher_ClientRule    0xF015 /* host-defined              host-defined                                                                     */
#define     _Escher_CalloutRule   0xF017 /*                           an FCORU                                                           X   X   X   0 */
#define _Escher_Selection         0xF119 /*                           an FDGSL followed by the SPIDs of the shapes in the selection              X   0 */

#define SHAPEFLAG_GROUP         0x001   // This shape is a group shape
#define SHAPEFLAG_CHILD         0x002   // Not a top-level shape
#define SHAPEFLAG_PATRIARCH     0x004   // This is the topmost group shape. Exactly one of these per drawing.
#define SHAPEFLAG_DELETED       0x008   // The shape has been deleted
#define SHAPEFLAG_OLESHAPE      0x010   // The shape is an OLE object
#define SHAPEFLAG_HAVEMASTER    0x020   // Shape has a hspMaster property
#define SHAPEFLAG_FLIPH         0x040   // Shape is flipped horizontally
#define SHAPEFLAG_FLIPV         0x080   // Shape is flipped vertically
#define SHAPEFLAG_CONNECTOR     0x100   // Connector type of shape
#define SHAPEFLAG_HAVEANCHOR    0x200   // Shape has an anchor of some kind
#define SHAPEFLAG_BACKGROUND    0x400   // Background shape
#define SHAPEFLAG_HAVESPT       0x800   // Shape has a shape type property

#define _Escher_ShpInst_Min             0
#define _Escher_ShpInst_NotPrimitive        _Escher_ShpInst_Min
#define _Escher_ShpInst_Rectangle       1
#define _Escher_ShpInst_RoundRectangle  2
#define _Escher_ShpInst_Ellipse         3
#define _Escher_ShpInst_Diamond         4
#define _Escher_ShpInst_IsocelesTriangle    5
#define _Escher_ShpInst_RightTriangle   6
#define _Escher_ShpInst_Parallelogram   7
#define _Escher_ShpInst_Trapezoid       8
#define _Escher_ShpInst_Hexagon         9
#define _Escher_ShpInst_Octagon         10
#define _Escher_ShpInst_Plus                11
#define _Escher_ShpInst_Star                12
#define _Escher_ShpInst_Arrow           13
#define _Escher_ShpInst_ThickArrow      14
#define _Escher_ShpInst_HomePlate       15
#define _Escher_ShpInst_Cube                16
#define _Escher_ShpInst_Balloon         17
#define _Escher_ShpInst_Seal                18
#define _Escher_ShpInst_Arc             19
#define _Escher_ShpInst_Line                20
#define _Escher_ShpInst_Plaque          21
#define _Escher_ShpInst_Can             22
#define _Escher_ShpInst_Donut           23
#define _Escher_ShpInst_TextSimple      24
#define _Escher_ShpInst_TextOctagon     25
#define _Escher_ShpInst_TextHexagon     26
#define _Escher_ShpInst_TextCurve       27
#define _Escher_ShpInst_TextWave            28
#define _Escher_ShpInst_TextRing            29
#define _Escher_ShpInst_TextOnCurve     30
#define _Escher_ShpInst_TextOnRing      31
#define _Escher_ShpInst_StraightConnector1 32
#define _Escher_ShpInst_BentConnector2  33
#define _Escher_ShpInst_BentConnector3  34
#define _Escher_ShpInst_BentConnector4  35
#define _Escher_ShpInst_BentConnector5  36
#define _Escher_ShpInst_CurvedConnector2    37
#define _Escher_ShpInst_CurvedConnector3    38
#define _Escher_ShpInst_CurvedConnector4    39
#define _Escher_ShpInst_CurvedConnector5    40
#define _Escher_ShpInst_Callout1            41
#define _Escher_ShpInst_Callout2            42
#define _Escher_ShpInst_Callout3            43
#define _Escher_ShpInst_AccentCallout1  44
#define _Escher_ShpInst_AccentCallout2  45
#define _Escher_ShpInst_AccentCallout3  46
#define _Escher_ShpInst_BorderCallout1  47
#define _Escher_ShpInst_BorderCallout2  48
#define _Escher_ShpInst_BorderCallout3  49
#define _Escher_ShpInst_AccentBorderCallout1    50
#define _Escher_ShpInst_AccentBorderCallout2    51
#define _Escher_ShpInst_AccentBorderCallout3    52
#define _Escher_ShpInst_Ribbon          53
#define _Escher_ShpInst_Ribbon2         54
#define _Escher_ShpInst_Chevron         55
#define _Escher_ShpInst_Pentagon            56
#define _Escher_ShpInst_NoSmoking       57
#define _Escher_ShpInst_Seal8           58
#define _Escher_ShpInst_Seal16          59
#define _Escher_ShpInst_Seal32          60
#define _Escher_ShpInst_WedgeRectCallout    61
#define _Escher_ShpInst_WedgeRRectCallout   62
#define _Escher_ShpInst_WedgeEllipseCallout 63
#define _Escher_ShpInst_Wave                64
#define _Escher_ShpInst_FoldedCorner        65
#define _Escher_ShpInst_LeftArrow       66
#define _Escher_ShpInst_DownArrow       67
#define _Escher_ShpInst_UpArrow         68
#define _Escher_ShpInst_LeftRightArrow  69
#define _Escher_ShpInst_UpDownArrow     70
#define _Escher_ShpInst_IrregularSeal1  71
#define _Escher_ShpInst_IrregularSeal2  72
#define _Escher_ShpInst_LightningBolt   73
#define _Escher_ShpInst_Heart           74
#define _Escher_ShpInst_PictureFrame        75
#define _Escher_ShpInst_QuadArrow       76
#define _Escher_ShpInst_LeftArrowCallout    77
#define _Escher_ShpInst_RightArrowCallout   78
#define _Escher_ShpInst_UpArrowCallout  79
#define _Escher_ShpInst_DownArrowCallout    80
#define _Escher_ShpInst_LeftRightArrowCallout   81
#define _Escher_ShpInst_UpDownArrowCallout  82
#define _Escher_ShpInst_QuadArrowCallout    83
#define _Escher_ShpInst_Bevel           84
#define _Escher_ShpInst_LeftBracket     85
#define _Escher_ShpInst_RightBracket        86
#define _Escher_ShpInst_LeftBrace       87
#define _Escher_ShpInst_RightBrace      88
#define _Escher_ShpInst_LeftUpArrow     89
#define _Escher_ShpInst_BentUpArrow     90
#define _Escher_ShpInst_BentArrow       91
#define _Escher_ShpInst_Seal24          92
#define _Escher_ShpInst_StripedRightArrow   93
#define _Escher_ShpInst_NotchedRightArrow   94
#define _Escher_ShpInst_BlockArc            95
#define _Escher_ShpInst_SmileyFace      96
#define _Escher_ShpInst_VerticalScroll  97
#define _Escher_ShpInst_HorizontalScroll    98
#define _Escher_ShpInst_CircularArrow   99
#define _Escher_ShpInst_NotchedCircularArrow    100
#define _Escher_ShpInst_UturnArrow      101
#define _Escher_ShpInst_CurvedRightArrow    102
#define _Escher_ShpInst_CurvedLeftArrow 103
#define _Escher_ShpInst_CurvedUpArrow   104
#define _Escher_ShpInst_CurvedDownArrow 105
#define _Escher_ShpInst_CloudCallout        106
#define _Escher_ShpInst_EllipseRibbon   107
#define _Escher_ShpInst_EllipseRibbon2  108
#define _Escher_ShpInst_FlowChartProcess    109
#define _Escher_ShpInst_FlowChartDecision   110
#define _Escher_ShpInst_FlowChartInputOutput    111
#define _Escher_ShpInst_FlowChartPredefinedProcess  112
#define _Escher_ShpInst_FlowChartInternalStorage    113
#define _Escher_ShpInst_FlowChartDocument   114
#define _Escher_ShpInst_FlowChartMultidocument  115
#define _Escher_ShpInst_FlowChartTerminator 116
#define _Escher_ShpInst_FlowChartPreparation    117
#define _Escher_ShpInst_FlowChartManualInput    118
#define _Escher_ShpInst_FlowChartManualOperation    119
#define _Escher_ShpInst_FlowChartConnector  120
#define _Escher_ShpInst_FlowChartPunchedCard    121
#define _Escher_ShpInst_FlowChartPunchedTape    122
#define _Escher_ShpInst_FlowChartSummingJunction    123
#define _Escher_ShpInst_FlowChartOr     124
#define _Escher_ShpInst_FlowChartCollate    125
#define _Escher_ShpInst_FlowChartSort   126
#define _Escher_ShpInst_FlowChartExtract    127
#define _Escher_ShpInst_FlowChartMerge  128
#define _Escher_ShpInst_FlowChartOfflineStorage 129
#define _Escher_ShpInst_FlowChartOnlineStorage  130
#define _Escher_ShpInst_FlowChartMagneticTape   131
#define _Escher_ShpInst_FlowChartMagneticDisk   132
#define _Escher_ShpInst_FlowChartMagneticDrum   133
#define _Escher_ShpInst_FlowChartDisplay    134
#define _Escher_ShpInst_FlowChartDelay  135
#define _Escher_ShpInst_TextPlainText   136
#define _Escher_ShpInst_TextStop            137
#define _Escher_ShpInst_TextTriangle        138
#define _Escher_ShpInst_TextTriangleInverted    139
#define _Escher_ShpInst_TextChevron     140
#define _Escher_ShpInst_TextChevronInverted 141
#define _Escher_ShpInst_TextRingInside  142
#define _Escher_ShpInst_TextRingOutside 143
#define _Escher_ShpInst_TextArchUpCurve 144
#define _Escher_ShpInst_TextArchDownCurve   145
#define _Escher_ShpInst_TextCircleCurve 146
#define _Escher_ShpInst_TextButtonCurve 147
#define _Escher_ShpInst_TextArchUpPour  148
#define _Escher_ShpInst_TextArchDownPour    149
#define _Escher_ShpInst_TextCirclePour  150
#define _Escher_ShpInst_TextButtonPour  151
#define _Escher_ShpInst_TextCurveUp     152
#define _Escher_ShpInst_TextCurveDown   153
#define _Escher_ShpInst_TextCascadeUp   154
#define _Escher_ShpInst_TextCascadeDown 155
#define _Escher_ShpInst_TextWave1       156
#define _Escher_ShpInst_TextWave2       157
#define _Escher_ShpInst_TextWave3       158
#define _Escher_ShpInst_TextWave4       159
#define _Escher_ShpInst_TextInflate     160
#define _Escher_ShpInst_TextDeflate     161
#define _Escher_ShpInst_TextInflateBottom   162
#define _Escher_ShpInst_TextDeflateBottom   163
#define _Escher_ShpInst_TextInflateTop      164
#define _Escher_ShpInst_TextDeflateTop      165
#define _Escher_ShpInst_TextDeflateInflate  166
#define _Escher_ShpInst_TextDeflateInflateDeflate   167
#define _Escher_ShpInst_TextFadeRight   168
#define _Escher_ShpInst_TextFadeLeft        169
#define _Escher_ShpInst_TextFadeUp      170
#define _Escher_ShpInst_TextFadeDown        171
#define _Escher_ShpInst_TextSlantUp     172
#define _Escher_ShpInst_TextSlantDown   173
#define _Escher_ShpInst_TextCanUp       174
#define _Escher_ShpInst_TextCanDown     175
#define _Escher_ShpInst_FlowChartAlternateProcess   176
#define _Escher_ShpInst_FlowChartOffpageConnector   177
#define _Escher_ShpInst_Callout90       178
#define _Escher_ShpInst_AccentCallout90 179
#define _Escher_ShpInst_BorderCallout90 180
#define _Escher_ShpInst_AccentBorderCallout90   181
#define _Escher_ShpInst_LeftRightUpArrow    182
#define _Escher_ShpInst_Sun             183
#define _Escher_ShpInst_Moon                184
#define _Escher_ShpInst_BracketPair     185
#define _Escher_ShpInst_BracePair       186
#define _Escher_ShpInst_Seal4           187
#define _Escher_ShpInst_DoubleWave      188
#define _Escher_ShpInst_ActionButtonBlank   189
#define _Escher_ShpInst_ActionButtonHome        190
#define _Escher_ShpInst_ActionButtonHelp        191
#define _Escher_ShpInst_ActionButtonInformation 192
#define _Escher_ShpInst_ActionButtonForwardNext 193
#define _Escher_ShpInst_ActionButtonBackPrevious    194
#define _Escher_ShpInst_ActionButtonEnd     195
#define _Escher_ShpInst_ActionButtonBeginning   196
#define _Escher_ShpInst_ActionButtonReturn  197
#define _Escher_ShpInst_ActionButtonDocument    198
#define _Escher_ShpInst_ActionButtonSound   199
#define _Escher_ShpInst_ActionButtonMovie   200
#define _Escher_ShpInst_HostControl         201
#define _Escher_ShpInst_TextBox             202
#define _Escher_ShpInst_Max                 0x0FFF
#define _Escher_ShpInst_Nil                 _Escher_ShpInst_Max

// Werte fuer den ULONG im PPT_PST_TextHeaderAtom
enum PPT_TextHeader
{
    PPTTH_TITLE,
    PPTTH_BODY,
    PPTTH_NOTES,
    PPTTH_NOTUSED,
    PPTTH_OTHER,       // Text in a Shape
    PPTTH_CENTERBODY,  // Subtitle in Title-Slide
    PPTTH_CENTERTITLE, // Title in Title-Slide
    PPTTH_HALFBODY,    // Body in two-column slide
    PPTTH_QUARTERBODY  // Body in four-body slide
};

enum _Escher_BlibType
{                           // GEL provided types...
   ERROR = 0,               // An error occured during loading
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

enum _Escher_FillStyle
{
    _Escher_FillSolid,      // Fill with a solid color
    _Escher_FillPattern,     // Fill with a pattern (bitmap)
    _Escher_FillTexture,     // A texture (pattern with its own color map)
    _Escher_FillPicture,     // Center a picture in the shape
    _Escher_FillShade,       // Shade from start to end points
    _Escher_FillShadeCenter, // Shade from bounding rectangle to end point
    _Escher_FillShadeShape,  // Shade from shape outline to end point
    _Escher_FillShadeScale,
    _Escher_FillShadeTitle,
    _Escher_FillBackground
};

enum _Escher_wMode
{
    _Escher_wColor,          // only used for predefined shades
    _Escher_wAutomatic,      // depends on object type
    _Escher_wGrayScale,      // shades of gray only
    _Escher_wLightGrayScale, // shades of light gray only
    _Escher_wInverseGray,    // dark gray mapped to light gray, etc.
    _Escher_wGrayOutline,    // pure gray and white
    _Escher_wBlackTextLine,  // black text and lines, all else grayscale
    _Escher_wHighContrast,   // pure black and white mode (no grays)
    _Escher_wBlack,          // solid black   msobwWhite,          // solid white
    _Escher_wDontShow,       // object not drawn
    _Escher_wNumModes        // number of Black and white modes
};


//
enum _Escher_ShapePath
{
    _Escher_ShapeLines,        // A line of straight segments
    _Escher_ShapeLinesClosed,  // A closed polygonal object
    _Escher_ShapeCurves,       // A line of Bezier curve segments
    _Escher_ShapeCurvesClosed, // A closed shape with curved edges
    _Escher_ShapeComplex      // pSegmentInfo must be non-empty
};


enum _Escher_WrapMode
{
    _Escher_WrapSquare,
    _Escher_WrapByPoints,
    _Escher_WrapNone,
    _Escher_WrapTopBottom,
    _Escher_WrapThrough
};

//
enum _Escher_bwMode
{
    _Escher_bwColor,          // only used for predefined shades
    _Escher_bwAutomatic,      // depends on object type
    _Escher_bwGrayScale,      // shades of gray only
    _Escher_bwLightGrayScale, // shades of light gray only
    _Escher_bwInverseGray,    // dark gray mapped to light gray, etc.
    _Escher_bwGrayOutline,    // pure gray and white
    _Escher_bwBlackTextLine,  // black text and lines, all else grayscale
    _Escher_bwHighContrast,   // pure black and white mode (no grays)
    _Escher_bwBlack,          // solid black
    _Escher_bwWhite,          // solid white
    _Escher_bwDontShow,       // object not drawn
    _Escher_bwNumModes        // number of Black and white modes
};


enum _Escher_AnchorText
{
    _Escher_AnchorTop,
    _Escher_AnchorMiddle,
    _Escher_AnchorBottom,
    _Escher_AnchorTopCentered,
    _Escher_AnchorMiddleCentered,
    _Escher_AnchorBottomCentered,
    _Escher_AnchorTopBaseline,
    _Escher_AnchorBottomBaseline,
    _Escher_AnchorTopCenteredBaseline,
    _Escher_AnchorBottomCenteredBaseline
};

enum _Escher_cDir
{
    _Escher_cDir0,       // Right
    _Escher_cDir90,      // Down
    _Escher_cDir180,     // Left
    _Escher_cDir270      // Up
};

//  connector style
enum _Escher_cxSTYLE
{
    _Escher_cxstyleStraight = 0,
    _Escher_cxstyleBent,
    _Escher_cxstyleCurved,
    _Escher_cxstyleNone
};

//  text flow
enum _Escher_txfl
{
    _Escher_txflHorzN,           // Horizontal non-@
    _Escher_txflTtoBA,           // Top to Bottom @-font
    _Escher_txflBtoT,            // Bottom to Top non-@
    _Escher_txflTtoBN,           // Top to Bottom non-@
    _Escher_txflHorzA,           // Horizontal @-font
    _Escher_txflVertN           // Vertical, non-@
};

//  text direction (needed for Bi-Di support)
enum _Escher_txDir
{
    _Escher_txdirLTR,           // left-to-right text direction
    _Escher_txdirRTL,           // right-to-left text direction
    _Escher_txdirContext            // context text direction
};

// Callout Type
enum _Escher_spcot
{
    _Escher_spcotRightAngle = 1,
    _Escher_spcotOneSegment = 2,
    _Escher_spcotTwoSegment = 3,
    _Escher_spcotThreeSegment = 4
};

// Callout Angle
enum _Escher_spcoa
{
    _Escher_spcoaAny,
    _Escher_spcoa30,
    _Escher_spcoa45,
    _Escher_spcoa60,
    _Escher_spcoa90,
    _Escher_spcoa0
};

//  Callout Drop
enum _Escher_spcod
{
    _Escher_spcodTop,
    _Escher_spcodCenter,
    _Escher_spcodBottom,
    _Escher_spcodSpecified
};

// WordArt alignment
enum _Escher_GeoTextAlign
{
    _Escher_AlignTextStretch,      /* Stretch each line of text to fit width. */
    _Escher_AlignTextCenter,       /* Center text on width. */
    _Escher_AlignTextLeft,         /* Left justify. */
    _Escher_AlignTextRight,        /* Right justify. */
    _Escher_AlignTextLetterJust,   /* Spread letters out to fit width. */
    _Escher_AlignTextWordJust,     /* Spread words out to fit width. */
    _Escher_AlignTextInvalid       /* Invalid */
};

//  flags for pictures
enum _Escher_BlipFlags
{
    _Escher_BlipFlagDefault = 0,
    _Escher_BlipFlagComment = 0,   // Blip name is a comment
    _Escher_BlipFlagFile,          // Blip name is a file name
    _Escher_BlipFlagURL,           // Blip name is a full URL
    _Escher_BlipFlagType = 3,      // Mask to extract type
   /* Or the following flags with any of the above. */
    _Escher_BlipFlagDontSave = 4,  // A "dont" is the depression in the metal
                             // body work of an automobile caused when a
                             // cyclist violently thrusts his or her nose
                             // at it, thus a DontSave is another name for
                             // a cycle lane.
    _Escher_BlipFlagDoNotSave = 4, // For those who prefer English
    _Escher_BlipFlagLinkToFile = 8
};

//
enum _Escher_3DRenderMode
{
    _Escher_FullRender,      // Generate a full rendering
    _Escher_Wireframe,       // Generate a wireframe
    _Escher_BoundingCube        // Generate a bounding cube
};

//
enum _Escher_xFormType
{
    _Escher_xFormAbsolute,   // Apply transform in absolute space centered on shape
    _Escher_xFormShape,      // Apply transform to shape geometry
    _Escher_xFormDrawing     // Apply transform in drawing space
};

//
enum _Escher_ShadowType
{
    _Escher_ShadowOffset,    // N pixel offset shadow
    _Escher_ShadowDouble,    // Use second offset too
    _Escher_ShadowRich,      // Rich perspective shadow (cast relative to shape)
    _Escher_ShadowShape,     // Rich perspective shadow (cast in shape space)
    _Escher_ShadowDrawing,   // Perspective shadow cast in drawing space
    _Escher_ShadowEmbossOrEngrave
};

//  - the type of a (length) measurement
enum _Escher_dzType
   {
   _Escher_dzTypeMin          = 0,
   _Escher_dzTypeDefault      = 0,  // Default size, ignore the values
   _Escher_dzTypeA            = 1,  // Values are in EMUs
   _Escher_dzTypeV            = 2,  // Values are in pixels
   _Escher_dzTypeShape        = 3,  // Values are 16.16 fractions of shape size
   _Escher_dzTypeFixedAspect  = 4,  // Aspect ratio is fixed
   _Escher_dzTypeAFixed       = 5,  // EMUs, fixed aspect ratio
   _Escher_dzTypeVFixed       = 6,  // Pixels, fixed aspect ratio
   _Escher_dzTypeShapeFixed   = 7,  // Proportion of shape, fixed aspect ratio
   _Escher_dzTypeFixedAspectEnlarge= 8,  // Aspect ratio is fixed, favor larger size
   _Escher_dzTypeAFixedBig    = 9,  // EMUs, fixed aspect ratio
   _Escher_dzTypeVFixedBig    = 10, // Pixels, fixed aspect ratio
   _Escher_dzTypeShapeFixedBig= 11, // Proportion of shape, fixed aspect ratio
   _Escher_dzTypeMax         = 11
};

// how to interpret the colors in a shaded fill.
enum _Escher_ShadeType
{
    _Escher_ShadeNone  = 0,        // Interpolate without correction between RGBs
    _Escher_ShadeGamma = 1,        // Apply gamma correction to colors
    _Escher_ShadeSigma = 2,        // Apply a sigma transfer function to position
    _Escher_ShadeBand  = 4,        // Add a flat band at the start of the shade
    _Escher_ShadeOneColor = 8,     // This is a one color shade

   /* A parameter for the band or sigma function can be stored in the top
      16 bits of the value - this is a proportion of *each* band of the
      shade to make flat (or the approximate equal value for a sigma
      function).  NOTE: the parameter is not used for the sigma function,
      instead a built in value is used.  This value should not be changed
      from the default! */
    _Escher_ShadeParameterShift = 16,
    _Escher_ShadeParameterMask  = 0xffff0000,

    _Escher_ShadeDefault = (_Escher_ShadeGamma|_Escher_ShadeSigma|
                     (16384<<_Escher_ShadeParameterShift))
};

// compound line style
enum _Escher_LineStyle
{
    _Escher_LineSimple,            // Single line (of width lineWidth)
    _Escher_LineDouble,            // Double lines of equal width
    _Escher_LineThickThin,         // Double lines, one thick, one thin
    _Escher_LineThinThick,         // Double lines, reverse order
    _Escher_LineTriple             // Three lines, thin, thick, thin
};

//  how to "fill" the line contour
enum _Escher_LineType
{
   _Escher_lineSolidType,         // Fill with a solid color
   _Escher_linePattern,           // Fill with a pattern (bitmap)
   _Escher_lineTexture,           // A texture (pattern with its own color map)
   _Escher_linePicture            // Center a picture in the shape
};

// dashed line style
enum _Escher_LineDashing
{
    _Escher_LineSolid,              // Solid (continuous) pen
    _Escher_LineDashSys,            // PS_DASH system   dash style
    _Escher_LineDotSys,             // PS_DOT system   dash style
    _Escher_LineDashDotSys,         // PS_DASHDOT system dash style
    _Escher_LineDashDotDotSys,      // PS_DASHDOTDOT system dash style
    _Escher_LineDotGEL,             // square dot style
    _Escher_LineDashGEL,            // dash style
    _Escher_LineLongDashGEL,        // long dash style
    _Escher_LineDashDotGEL,         // dash short dash
    _Escher_LineLongDashDotGEL,     // long dash short dash
    _Escher_LineLongDashDotDotGEL   // long dash short dash short dash
};

// line end effect
enum _Escher_LineEnd
{
    _Escher_LineNoEnd,
    _Escher_LineArrowEnd,
    _Escher_LineArrowStealthEnd,
    _Escher_LineArrowDiamondEnd,
    _Escher_LineArrowOvalEnd,
    _Escher_LineArrowOpenEnd
};

// size of arrowhead
enum _Escher_LineWidth
{
    _Escher_LineNarrowArrow,
    _Escher_LineMediumWidthArrow,
    _Escher_LineWideArrow
};

// size of arrowhead
enum _Escher_LineEndLenght
{
    _Escher_LineShortArrow,
    _Escher_LineMediumLenArrow,
    _Escher_LineLongArrow
};

// line join style.
enum _Escher_LineJoin
{
    _Escher_LineJoinBevel,     // Join edges by a straight line
    _Escher_LineJoinMiter,     // Extend edges until they join
    _Escher_LineJoinRound      // Draw an arc between the two edges
};

// line cap style (applies to ends of dash segments too).
enum _Escher_LineCap
{
    _Escher_LineEndCapRound,   // Rounded ends - the default
    _Escher_LineEndCapSquare,  // Square protrudes by half line width
    _Escher_LineEndCapFlat     // Line ends at end point
};
// Shape Properties
// 1pt = 12700 EMU (English Metric Units)
// 1pt = 20 Twip = 20/1440" = 1/72"
// 1twip=635 EMU
// 1" = 12700*72 = 914400 EMU
// 1" = 25.4mm
// 1mm = 36000 EMU
// Transform
#define _Escher_Prop_Rotation                      4  /*  Fixed Point 16.16 degrees                  */
// Protection
#define _Escher_Prop_LockRotation                119  /*  BOOL              No rotation                         */
#define _Escher_Prop_LockAspectRatio             120  /*  BOOL              Don't allow changes in aspect ratio */
#define _Escher_Prop_LockPosition                121  /*  BOOL              Don't allow the shape to be moved   */
#define _Escher_Prop_LockAgainstSelect           122  /*  BOOL              Shape may not be selected           */
#define _Escher_Prop_LockCropping                123  /*  BOOL              No cropping this shape              */
#define _Escher_Prop_LockVertices                124  /*  BOOL              Edit Points not allowed             */
#define _Escher_Prop_LockText                    125  /*  BOOL              Do not edit text                    */
#define _Escher_Prop_LockAdjustHandles           126  /*  BOOL              Do not adjust                       */
#define _Escher_Prop_LockAgainstGrouping         127  /*  BOOL              Do not group this shape             */
// Text
#define _Escher_Prop_lTxid                       128  /*  LONG              id for the text, value determined by the host            */
#define _Escher_Prop_dxTextLeft                  129  /*  LONG              margins relative to shape's inscribed                    */
#define _Escher_Prop_dyTextTop                   130  /*  LONG                text rectangle (in EMUs)                               */
#define _Escher_Prop_dxTextRight                 131  /*  LONG                                                                       */
#define _Escher_Prop_dyTextBottom                132  /*  LONG                                                                       */
#define _Escher_Prop_WrapText                    133  /*  MSOWRAPMODE       Wrap text at shape margins                               */
#define _Escher_Prop_scaleText                   134  /*  LONG              Text zoom/scale (used if fFitTextToShape)                */
#define _Escher_Prop_AnchorText                  135  /*  _Escher_AnchorText How to anchor the text                                   */
#define _Escher_Prop_txflTextFlow                136  /*  MSOTXFL           Text flow                                                */
#define _Escher_Prop_cdirFont                    137  /*  MSOCDIR           Font rotation                                            */
#define _Escher_Prop_hspNext                     138  /*  MSOHSP            ID of the next shape (used by Word for linked textboxes) */
#define _Escher_Prop_txdir                       139  /*  MSOTXDIR          Bi-Di Text direction                                     */
#define _Escher_Prop_SelectText                  187  /*  BOOL              TRUE if single click selects text, FALSE if two clicks   */
#define _Escher_Prop_AutoTextMargin              188  /*  BOOL              use host's margin calculations                           */
#define _Escher_Prop_RotateText                  189  /*  BOOL              Rotate text with shape                                   */
#define _Escher_Prop_FitShapeToText              190  /*  BOOL              Size shape to fit text size                              */
#define _Escher_Prop_FitTextToShape              191  /*  BOOL              Size text to fit shape size                              */
// GeoText
#define _Escher_Prop_gtextUNICODE                192  /*  WCHAR*            UNICODE text string       */
#define _Escher_Prop_gtextRTF                    193  /*  char*             RTF text string           */
#define _Escher_Prop_gtextAlign                  194  /*  MSOGEOTEXTALIGN   alignment on curve        */
#define _Escher_Prop_gtextSize                   195  /*  LONG              default point size        */
#define _Escher_Prop_gtextSpacing                196  /*  LONG              fixed point 16.16         */
#define _Escher_Prop_gtextFont                   197  /*  WCHAR*            font family name          */
#define _Escher_Prop_gtextFReverseRows           240  /*  BOOL              Reverse row order         */
#define _Escher_Prop_fGtext                      241  /*  BOOL              Has text effect           */
#define _Escher_Prop_gtextFVertical              242  /*  BOOL              Rotate characters         */
#define _Escher_Prop_gtextFKern                  243  /*  BOOL              Kern characters           */
#define _Escher_Prop_gtextFTight                 244  /*  BOOL              Tightening or tracking    */
#define _Escher_Prop_gtextFStretch               245  /*  BOOL              Stretch to fit shape      */
#define _Escher_Prop_gtextFShrinkFit             246  /*  BOOL              Char bounding box         */
#define _Escher_Prop_gtextFBestFit               247  /*  BOOL              Scale text-on-path        */
#define _Escher_Prop_gtextFNormalize             248  /*  BOOL              Stretch char height       */
#define _Escher_Prop_gtextFDxMeasure             249  /*  BOOL              Do not measure along path */
#define _Escher_Prop_gtextFBold                  250  /*  BOOL              Bold font                 */
#define _Escher_Prop_gtextFItalic                251  /*  BOOL              Italic font               */
#define _Escher_Prop_gtextFUnderline             252  /*  BOOL              Underline font            */
#define _Escher_Prop_gtextFShadow                253  /*  BOOL              Shadow font               */
#define _Escher_Prop_gtextFSmallcaps             254  /*  BOOL              Small caps font           */
#define _Escher_Prop_gtextFStrikethrough         255  /*  BOOL              Strike through font       */
// Blip
#define _Escher_Prop_cropFromTop                 256  /*  LONG              16.16 fraction times total                          */
#define _Escher_Prop_cropFromBottom             257  /*  LONG                image width or height,                            */
#define _Escher_Prop_cropFromLeft                258  /*  LONG                as appropriate.                                   */
#define _Escher_Prop_cropFromRight               259  /*  LONG                                                                  */
#define _Escher_Prop_pib                         260  /*  IMsoBlip*         Blip to display                                     */
#define _Escher_Prop_pibName                     261  /*  WCHAR*            Blip file name                                      */
#define _Escher_Prop_pibFlags                    262  /*  MSOBLIPFLAGS      Blip flags                                          */
#define _Escher_Prop_pictureTransparent          263  /*  LONG              transparent color (none if ~0UL)                    */
#define _Escher_Prop_pictureContrast             264  /*  LONG              contrast setting                                    */
#define _Escher_Prop_pictureBrightness           265  /*  LONG              brightness setting                                  */
#define _Escher_Prop_pictureGamma                266  /*  LONG              16.16 gamma                                         */
#define _Escher_Prop_pictureId                   267  /*  LONG              Host-defined ID for OLE objects (usually a pointer) */
#define _Escher_Prop_pictureDblCrMod             268  /*  MSOCLR            Modification used if shape has double shadow        */
#define _Escher_Prop_pictureFillCrMod            269  /*  MSOCLR                                                                */
#define _Escher_Prop_pictureLineCrMod            270  /*  MSOCLR                                                                */
#define _Escher_Prop_pibPrint                    271  /*  IMsoBlip*         Blip to display when printing                       */
#define _Escher_Prop_pibPrintName                272  /*  WCHAR*            Blip file name                                      */
#define _Escher_Prop_pibPrintFlags               273  /*  MSOBLIPFLAGS      Blip flags                                          */
#define _Escher_Prop_fNoHitTestPicture           316  /*  BOOL              Do not hit test the picture                         */
#define _Escher_Prop_pictureGray                 317  /*  BOOL              grayscale display                                   */
#define _Escher_Prop_pictureBiLevel              318  /*  BOOL              bi-level display                                    */
#define _Escher_Prop_pictureActive               319  /*  BOOL              Server is active (OLE objects only)                 */
// Geometry
#define _Escher_Prop_geoLeft                     320  /*  LONG              Defines the G (geometry) coordinate space.  */
#define _Escher_Prop_geoTop                      321  /*  LONG                                                          */
#define _Escher_Prop_geoRight                    322  /*  LONG                                                          */
#define _Escher_Prop_geoBottom                   323  /*  LONG                                                          */
#define _Escher_Prop_shapePath                   324  /*  MSOSHAPEPATH                                                  */
#define _Escher_Prop_pVertices                   325  /*  IMsoArray         An array of points, in G units.             */
#define _Escher_Prop_pSegmentInfo                326  /*  IMsoArray                                                     */
#define _Escher_Prop_adjustValue                 327  /*  LONG              Adjustment values corresponding to          */
#define _Escher_Prop_adjust2Value                328  /*  LONG                the positions of the adjust handles       */
#define _Escher_Prop_adjust3Value                329  /*  LONG                of the shape. The number of values        */
#define _Escher_Prop_adjust4Value                330  /*  LONG                used and their allowable ranges vary      */
#define _Escher_Prop_adjust5Value                331  /*  LONG                from shape type to shape type.            */
#define _Escher_Prop_adjust6Value                332  /*  LONG                                                          */
#define _Escher_Prop_adjust7Value                333  /*  LONG                                                          */
#define _Escher_Prop_adjust8Value                334  /*  LONG                                                          */
#define _Escher_Prop_adjust9Value                335  /*  LONG                                                          */
#define _Escher_Prop_adjust10Value               336  /*  LONG                                                          */
#define _Escher_Prop_fShadowOK                   378  /*  BOOL              Shadow may be set                           */
#define _Escher_Prop_f3DOK                       379  /*  BOOL              3D may be set                               */
#define _Escher_Prop_fLineOK                     380  /*  BOOL              Line style may be set                       */
#define _Escher_Prop_fGtextOK                    381  /*  BOOL              Text effect (WordArt) supported             */
#define _Escher_Prop_fFillShadeShapeOK           382  /*  BOOL                                                          */
#define _Escher_Prop_fFillOK                     383  /*  BOOL              OK to fill the shape through the UI or VBA? */
// FillStyle
#define _Escher_Prop_fillType                    384  /*  _Escher_FillStyle  Type of fill                                  */
#define _Escher_Prop_fillColor                   385  /*  MSOCLR            Foreground color                              */
#define _Escher_Prop_fillOpacity                 386  /*  LONG              Fixed 16.16                                   */
#define _Escher_Prop_fillBackColor               387  /*  MSOCLR            Background color                              */
#define _Escher_Prop_fillBackOpacity             388  /*  LONG              Shades only                                   */
#define _Escher_Prop_fillCrMod                   389  /*  MSOCLR            Modification for BW views                     */
#define _Escher_Prop_fillBlip                    390  /*  IMsoBlip*         Pattern/texture                               */
#define _Escher_Prop_fillBlipName                391  /*  WCHAR*            Blip file name                                */
#define _Escher_Prop_fillBlipFlags               392  /*  MSOBLIPFLAGS      Blip flags                                    */
#define _Escher_Prop_fillWidth                   393  /*  LONG              How big (A units) to make a metafile texture. */
#define _Escher_Prop_fillHeight                  394  /*  LONG                                                            */
#define _Escher_Prop_fillAngle                   395  /*  LONG              Fade angle - degrees in 16.16                 */
#define _Escher_Prop_fillFocus                   396  /*  LONG              Linear shaded fill focus percent              */
#define _Escher_Prop_fillToLeft                  397  /*  LONG              Fraction 16.16                                */
#define _Escher_Prop_fillToTop                   398  /*  LONG              Fraction 16.16                                */
#define _Escher_Prop_fillToRight                 399  /*  LONG              Fraction 16.16                                */
#define _Escher_Prop_fillToBottom                400  /*  LONG              Fraction 16.16                                */
#define _Escher_Prop_fillRectLeft                401  /*  LONG              For shaded fills, use the specified rectangle */
#define _Escher_Prop_fillRectTop                 402  /*  LONG                instead of the shape's bounding rect to     */
#define _Escher_Prop_fillRectRight               403  /*  LONG                define how large the fade is going to be.   */
#define _Escher_Prop_fillRectBottom              404  /*  LONG                                                            */
#define _Escher_Prop_fillDztype                  405  /*  MSODZTYPE                                                       */
#define _Escher_Prop_fillShadePreset             406  /*  LONG              Special shades                                */
#define _Escher_Prop_fillShadeColors             407  /*  IMsoArray         a preset array of colors                      */
#define _Escher_Prop_fillOriginX                 408  /*  LONG                                                            */
#define _Escher_Prop_fillOriginY                 409  /*  LONG                                                            */
#define _Escher_Prop_fillShapeOriginX            410  /*  LONG                                                            */
#define _Escher_Prop_fillShapeOriginY            411  /*  LONG                                                            */
#define _Escher_Prop_fillShadeType               412  /*  MSOSHADETYPE      Type of shading, if a shaded (gradient) fill. */
#define _Escher_Prop_fFilled                     443  /*  BOOL              Is shape filled?                              */
#define _Escher_Prop_fHitTestFill                444  /*  BOOL              Should we hit test fill?                      */
#define _Escher_Prop_fillShape                   445  /*  BOOL              Register pattern on shape                     */
#define _Escher_Prop_fillUseRect                 446  /*  BOOL              Use the large rect?                           */
#define _Escher_Prop_fNoFillHitTest              447  /*  BOOL              Hit test a shape as though filled             */
// LineStyle
#define _Escher_Prop_lineColor                   448  /*  MSOCLR            Color of line                              */
#define _Escher_Prop_lineOpacity                 449  /*  LONG              Not implemented                            */
#define _Escher_Prop_lineBackColor               450  /*  MSOCLR            Background color                           */
#define _Escher_Prop_lineCrMod                   451  /*  MSOCLR            Modification for BW views                  */
#define _Escher_Prop_lineType                    452  /*  MSOLINETYPE       Type of line                               */
#define _Escher_Prop_lineFillBlip                453  /*  IMsoBlip*         Pattern/texture                            */
#define _Escher_Prop_lineFillBlipName            454  /*  WCHAR*            Blip file name                             */
#define _Escher_Prop_lineFillBlipFlags           455  /*  MSOBLIPFLAGS      Blip flags                                 */
#define _Escher_Prop_lineFillWidth               456  /*  LONG              How big (A units) to make                  */
#define _Escher_Prop_lineFillHeight              457  /*  LONG                a metafile texture.                      */
#define _Escher_Prop_lineFillDztype              458  /*  MSODZTYPE         How to interpret fillWidth/Height numbers. */
#define _Escher_Prop_lineWidth                   459  /*  LONG              A units; 1pt == 12700 EMUs                 */
#define _Escher_Prop_lineMiterLimit              460  /*  LONG              ratio (16.16) of width                     */
#define _Escher_Prop_lineStyle                   461  /*  MSOLINESTYLE      Draw parallel lines?                       */
#define _Escher_Prop_lineDashing                 462  /*  MSOLINEDASHING    Can be overridden by:                      */
#define _Escher_Prop_lineDashStyle               463  /*  IMsoArray         As Win32 ExtCreatePen                      */
#define _Escher_Prop_lineStartArrowhead          464  /*  MSOLINEEND        Arrow at start                             */
#define _Escher_Prop_lineEndArrowhead            465  /*  MSOLINEEND        Arrow at end                               */
#define _Escher_Prop_lineStartArrowWidth         466  /*  MSOLINEENDWIDTH   Arrow at start                             */
#define _Escher_Prop_lineStartArrowLength        467  /*  MSOLINEENDLENGTH  Arrow at end                               */
#define _Escher_Prop_lineEndArrowWidth           468  /*  MSOLINEENDWIDTH   Arrow at start                             */
#define _Escher_Prop_lineEndArrowLength          469  /*  MSOLINEENDLENGTH  Arrow at end                               */
#define _Escher_Prop_lineJoinStyle               470  /*  MSOLINEJOIN       How to join lines                          */
#define _Escher_Prop_lineEndCapStyle             471  /*  MSOLINECAP        How to end lines                           */
#define _Escher_Prop_fArrowheadsOK               507  /*  BOOL              Allow arrowheads if prop. is set           */
#define _Escher_Prop_fLine                       508  /*  BOOL              Any line?                                  */
#define _Escher_Prop_fHitTestLine                509  /*  BOOL              Should we hit test lines?                  */
#define _Escher_Prop_lineFillShape               510  /*  BOOL              Register pattern on shape                  */
#define _Escher_Prop_fNoLineDrawDash             511  /*  BOOL              Draw a dashed line if no line              */
// ShadowStyle
#define _Escher_Prop_shadowType                  512  /*  MSOSHADOWTYPE     Type of effect            */
#define _Escher_Prop_shadowColor                 513  /*  MSOCLR            Foreground color          */
#define _Escher_Prop_shadowHighlight             514  /*  MSOCLR            Embossed color            */
#define _Escher_Prop_shadowCrMod                 515  /*  MSOCLR            Modification for BW views */
#define _Escher_Prop_shadowOpacity               516  /*  LONG              Fixed 16.16               */
#define _Escher_Prop_shadowOffsetX               517  /*  LONG              Offset shadow             */
#define _Escher_Prop_shadowOffsetY               518  /*  LONG              Offset shadow             */
#define _Escher_Prop_shadowSecondOffsetX         519  /*  LONG              Double offset shadow      */
#define _Escher_Prop_shadowSecondOffsetY         520  /*  LONG              Double offset shadow      */
#define _Escher_Prop_shadowScaleXToX             521  /*  LONG              16.16                     */
#define _Escher_Prop_shadowScaleYToX             522  /*  LONG              16.16                     */
#define _Escher_Prop_shadowScaleXToY             523  /*  LONG              16.16                     */
#define _Escher_Prop_shadowScaleYToY             524  /*  LONG              16.16                     */
#define _Escher_Prop_shadowPerspectiveX          525  /*  LONG              16.16 / weight            */
#define _Escher_Prop_shadowPerspectiveY          526  /*  LONG              16.16 / weight            */
#define _Escher_Prop_shadowWeight                527  /*  LONG              scaling factor            */
#define _Escher_Prop_shadowOriginX               528  /*  LONG                                        */
#define _Escher_Prop_shadowOriginY               529  /*  LONG                                        */
#define _Escher_Prop_fShadow                     574  /*  BOOL              Any shadow?               */
#define _Escher_Prop_fshadowObscured             575  /*  BOOL              Excel5-style shadow       */
// PerspectiveStyle
#define _Escher_Prop_perspectiveType             576  /*  MSOXFORMTYPE      Where transform applies        */
#define _Escher_Prop_perspectiveOffsetX          577  /*  LONG              The LONG values define a       */
#define _Escher_Prop_perspectiveOffsetY          578  /*  LONG                transformation matrix,       */
#define _Escher_Prop_perspectiveScaleXToX        579  /*  LONG                effectively, each value      */
#define _Escher_Prop_perspectiveScaleYToX        580  /*  LONG                is scaled by the             */
#define _Escher_Prop_perspectiveScaleXToY        581  /*  LONG                perspectiveWeight parameter. */
#define _Escher_Prop_perspectiveScaleYToY        582  /*  LONG                                             */
#define _Escher_Prop_perspectivePerspectiveX     583  /*  LONG                                             */
#define _Escher_Prop_perspectivePerspectiveY     584  /*  LONG                                             */
#define _Escher_Prop_perspectiveWeight           585  /*  LONG              Scaling factor                 */
#define _Escher_Prop_perspectiveOriginX          586  /*  LONG                                             */
#define _Escher_Prop_perspectiveOriginY          587  /*  LONG                                             */
#define _Escher_Prop_fPerspective                639  /*  BOOL              On/off                         */
// 3D Object
#define _Escher_Prop_c3DSpecularAmt              640  /*  LONG         Fixed-point 16.16                                                                                                   */
#define _Escher_Prop_c3DDiffuseAmt               641  /*  LONG         Fixed-point 16.16                                                                                                   */
#define _Escher_Prop_c3DShininess                642  /*  LONG         Default gives OK results                                                                                            */
#define _Escher_Prop_c3DEdgeThickness            643  /*  LONG         Specular edge thickness                                                                                             */
#define _Escher_Prop_c3DExtrudeForward           644  /*  LONG         Distance of extrusion in EMUs                                                                                       */
#define _Escher_Prop_c3DExtrudeBackward          645  /*  LONG                                                                                                                             */
#define _Escher_Prop_c3DExtrudePlane             646  /*  LONG         Extrusion direction                                                                                                 */
#define _Escher_Prop_c3DExtrusionColor           647  /*  MSOCLR       Basic color of extruded part of shape; the lighting model used will determine the exact shades used when rendering. */
#define _Escher_Prop_c3DCrMod                    648  /*  MSOCLR       Modification for BW views                                                                                           */
#define _Escher_Prop_f3D                         700  /*  BOOL         Does this shape have a 3D effect?                                                                                   */
#define _Escher_Prop_fc3DMetallic                701  /*  BOOL         Use metallic specularity?                                                                                           */
#define _Escher_Prop_fc3DUseExtrusionColor       702  /*  BOOL                                                                                                                             */
#define _Escher_Prop_fc3DLightFace               703  /*  BOOL                                                                                                                             */
// 3D Style
#define _Escher_Prop_c3DYRotationAngle           704  /*  LONG            degrees (16.16) about y axis         */
#define _Escher_Prop_c3DXRotationAngle           705  /*  LONG            degrees (16.16) about x axis         */
#define _Escher_Prop_c3DRotationAxisX            706  /*  LONG            These specify the rotation axis;     */
#define _Escher_Prop_c3DRotationAxisY            707  /*  LONG              only their relative magnitudes     */
#define _Escher_Prop_c3DRotationAxisZ            708  /*  LONG              matter.                            */
#define _Escher_Prop_c3DRotationAngle            709  /*  LONG            degrees (16.16) about axis           */
#define _Escher_Prop_c3DRotationCenterX          710  /*  LONG            rotation center x (16.16 or g-units) */
#define _Escher_Prop_c3DRotationCenterY          711  /*  LONG            rotation center y (16.16 or g-units) */
#define _Escher_Prop_c3DRotationCenterZ          712  /*  LONG            rotation center z (absolute (emus))  */
#define _Escher_Prop_c3DRenderMode               713  /*  MSO3DRENDERMODE Full,wireframe, or bcube             */
#define _Escher_Prop_c3DTolerance                714  /*  LONG            pixels (16.16)                       */
#define _Escher_Prop_c3DXViewpoint               715  /*  LONG            X view point (emus)                  */
#define _Escher_Prop_c3DYViewpoint               716  /*  LONG            Y view point (emus)                  */
#define _Escher_Prop_c3DZViewpoint               717  /*  LONG            Z view distance (emus)               */
#define _Escher_Prop_c3DOriginX                  718  /*  LONG                                                 */
#define _Escher_Prop_c3DOriginY                  719  /*  LONG                                                 */
#define _Escher_Prop_c3DSkewAngle                720  /*  LONG            degree (16.16) skew angle            */
#define _Escher_Prop_c3DSkewAmount               721  /*  LONG            Percentage skew amount               */
#define _Escher_Prop_c3DAmbientIntensity         722  /*  LONG            Fixed point intensity                */
#define _Escher_Prop_c3DKeyX                     723  /*  LONG            Key light source direc-              */
#define _Escher_Prop_c3DKeyY                     724  /*  LONG            tion; only their relative            */
#define _Escher_Prop_c3DKeyZ                     725  /*  LONG            magnitudes matter                    */
#define _Escher_Prop_c3DKeyIntensity             726  /*  LONG            Fixed point intensity                */
#define _Escher_Prop_c3DFillX                    727  /*  LONG            Fill light source direc-             */
#define _Escher_Prop_c3DFillY                    728  /*  LONG            tion; only their relative            */
#define _Escher_Prop_c3DFillZ                    729  /*  LONG            magnitudes matter                    */
#define _Escher_Prop_c3DFillIntensity            730  /*  LONG            Fixed point intensity                */
#define _Escher_Prop_fc3DConstrainRotation       763  /*  BOOL                                                 */
#define _Escher_Prop_fc3DRotationCenterAuto      764  /*  BOOL                                                 */
#define _Escher_Prop_fc3DParallel                765  /*  BOOL            Parallel projection?                 */
#define _Escher_Prop_fc3DKeyHarsh                766  /*  BOOL            Is key lighting harsh?               */
#define _Escher_Prop_fc3DFillHarsh               767  /*  BOOL            Is fill lighting harsh?              */
// Shape
#define _Escher_Prop_hspMaster                   769  /*  MSOHSP          master shape                                        */
#define _Escher_Prop_cxstyle                     771  /*  MSOCXSTYLE      Type of connector                                   */
#define _Escher_Prop_bWMode                      772  /*  _EscherwMode     Settings for modifications to                       */
#define _Escher_Prop_bWModePureBW                773  /*  _EscherwMode   be made when in different                         */
#define _Escher_Prop_bWModeBW                    774  /*  _EscherwMode   forms of black-and-white mode.                    */
#define _Escher_Prop_fOleIcon                    826  /*  BOOL            For OLE objects, whether the object is in icon form */
#define _Escher_Prop_fPreferRelativeResize       827  /*  BOOL            For UI only. Prefer relative resizing.              */
#define _Escher_Prop_fLockShapeType              828  /*  BOOL            Lock the shape type (don't allow Change Shape)      */
#define _Escher_Prop_fDeleteAttachedObject       830  /*  BOOL                                                                */
#define _Escher_Prop_fBackground                 831  /*  BOOL            If TRUE, this is the background shape.              */
// Callout
#define _Escher_Prop_spcot                       832  /*  MSOSPCOT        Callout type                                           */
#define _Escher_Prop_dxyCalloutGap               833  /*  LONG            Distance from box to first point.(EMUs)                */
#define _Escher_Prop_spcoa                       834  /*  MSOSPCOA        Callout angle                                          */
#define _Escher_Prop_spcod                       835  /*  MSOSPCOD        Callout drop type                                      */
#define _Escher_Prop_dxyCalloutDropSpecified     836  /*  LONG            if msospcodSpecified, the actual drop distance         */
#define _Escher_Prop_dxyCalloutLengthSpecified   837  /*  LONG            if fCalloutLengthSpecified, the actual distance        */
#define _Escher_Prop_fCallout                    889  /*  BOOL            Is the shape a callout?                                */
#define _Escher_Prop_fCalloutAccentBar           890  /*  BOOL            does callout have accent bar                           */
#define _Escher_Prop_fCalloutTextBorder          891  /*  BOOL            does callout have a text border                        */
#define _Escher_Prop_fCalloutMinusX              892  /*  BOOL                                                                   */
#define _Escher_Prop_fCalloutMinusY              893  /*  BOOL                                                                   */
#define _Escher_Prop_fCalloutDropAuto            894  /*  BOOL            If true, then we occasionally invert the drop distance */
#define _Escher_Prop_fCalloutLengthSpecified     895  /*  BOOL            if true, we look at dxyCalloutLengthSpecified          */
// GroupShape
#define _Escher_Prop_wzName                      896  /*  WCHAR*          Shape Name (present only if explicitly set)                                                            */
#define _Escher_Prop_wzDescription               897  /*  WCHAR*          alternate text                                                                                         */
#define _Escher_Prop_pihlShape                   898  /*  IHlink*         The hyperlink in the shape.                                                                            */
#define _Escher_Prop_pWrapPolygonVertices        899  /*  IMsoArray       The polygon that text will be wrapped around (Word)                                                    */
#define _Escher_Prop_dxWrapDistLeft              900  /*  LONG            Left wrapping distance from text (Word)                                                                */
#define _Escher_Prop_dyWrapDistTop               901  /*  LONG            Top wrapping distance from text (Word)                                                                 */
#define _Escher_Prop_dxWrapDistRight             902  /*  LONG            Right wrapping distance from text (Word)                                                               */
#define _Escher_Prop_dyWrapDistBottom            903  /*  LONG            Bottom wrapping distance from text (Word)                                                              */
#define _Escher_Prop_lidRegroup                  904  /*  LONG            Regroup ID                                                                                             */
#define _Escher_Prop_fEditedWrap                 953  /*  BOOL            Has the wrap polygon been edited?                                                                      */
#define _Escher_Prop_fBehindDocument             954  /*  BOOL            Word-only (shape is behind text)                                                                       */
#define _Escher_Prop_fOnDblClickNotify           955  /*  BOOL            Notify client on a double click                                                                        */
#define _Escher_Prop_fIsButton                   956  /*  BOOL            A button shape (i.e., clicking performs an action). Set for shapes with attached hyperlinks or macros. */
#define _Escher_Prop_fOneD                       957  /*  BOOL            1D adjustment                                                                                          */
#define _Escher_Prop_fHidden                     958  /*  BOOL            Do not display                                                                                         */
#define _Escher_Prop_fPrint                      959  /*  BOOL            Print this shape                                                                                       */


#define _Escher_PERSISTENTRY_PREALLOCATE        64

#define _Escher_Persist_PrivateEntry        0x80000000
#define _Escher_Persist_Dg                  0x00020000
#define _Escher_Persist_CurrentPosition     0x00040000
#define _Escher_Persist_Grouping_Snap       0x00050000
#define _Escher_Persist_Grouping_Logic      0x00060000

// ---------------------------------------------------------------------------------------------

struct _EscherPropSortStruct
{
    BYTE*       pBuf;
    UINT32      nPropSize;
    UINT32      nPropValue;
    UINT16      nPropId;
};

struct _Escher_GDIStruct
{
    Rectangle   GDIBoundRect;
    Size        GDISize;
    UINT32      GDIUncompressedSize;
};

// ---------------------------------------------------------------------------------------------

struct _EscherPersistEntry
{
    UINT32  mnID;
    UINT32  mnOffset;
            _EscherPersistEntry( UINT32 nId, UINT32 nOffset ) { mnID = nId; mnOffset = nOffset; };

};


// ---------------------------------------------------------------------------------------------

class SvMemoryStream;
class _EscherBlibEntry
{
        friend class _EscherGraphicProvider;
        friend class _EscherEx;

    protected:

        UINT32              mnPictureOffset;        // offset auf die grafik im PictureStreams
        UINT32              mnSize;
        _Escher_BlibType    meBlibType;
        UINT32              mnIdentifier[ 4 ];

    public:

                        _EscherBlibEntry( SvMemoryStream& rStream, _Escher_BlibType eBlibType, UINT32 nPicOffset );
                        ~_EscherBlibEntry();

        BOOL            operator==( const _EscherBlibEntry& ) const;
};

// ---------------------------------------------------------------------------------------------

class _EscherPersistTable
{

    public:
        List    maPersistTable;

        BOOL    PtIsID( UINT32 nID );
        void    PtInsert( UINT32 nID, UINT32 nOfs );
        UINT32  PtDelete( UINT32 nID );
        UINT32  PtGetOffsetByID( UINT32 nID );
        UINT32  PtReplace( UINT32 nID, UINT32 nOfs );
        UINT32  PtReplaceOrInsert( UINT32 nID, UINT32 nOfs );
        UINT32  PtGetCount() const { return maPersistTable.Count(); };

        _EscherPersistTable();
        ~_EscherPersistTable();
};

// ---------------------------------------------------------------------------------------------

#define _E_GRAPH_PROV_USE_INSTANCES 1

class _EscherGraphicProvider
{
        UINT32      mnFlags;
        SvStream&   mrPicOutStrm;

        UINT32      ImplInsertBlib( _EscherBlibEntry* p_EscherBlibEntry );

    public :

        _EscherBlibEntry**      mpBlibEntrys;
        UINT32      mnBlibBufSize;
        UINT32      mnBlibEntrys;

        UINT32      ImplGetBlibID( SvMemoryStream& rStrm, _Escher_BlibType eBlibType, const _Escher_GDIStruct* pStruct = NULL );

                    _EscherGraphicProvider( SvStream& rSt, UINT32 nFlags = 0 );
                    ~_EscherGraphicProvider();

};

// ---------------------------------------------------------------------------------------------

class Color;
class Graphic;
class SvMemoryStream;
class SvStorageStream;
class _EscherEx : public _EscherPersistTable
{
        SvStorageStream*        mpOutStrm;
        _EscherGraphicProvider* mpGraphicProvider;

        UINT32                  mnStrmStartOfs;
        int                     mnLevel;
        UINT32*                 mpOffsets;
        UINT32*                 mpSizes;
        UINT16*                 mpRecTypes;                 // nimmt die Container RecTypes auf

        UINT32                  mnDrawings;
        SvMemoryStream          maFIDCLs;
        UINT32                  mnFIDCLs;                   // anzahl der cluster ID's

        UINT32                  mnCurrentDg;
        UINT32                  mnCurrentShapeID;           // die naechste freie ID
        UINT32                  mnCurrentShapeMaximumID;    // die hoechste und auch benutzte ID
        UINT32                  mnTotalShapesDg;            // anzahl der shapes im Dg
        UINT32                  mnTotalShapeIdUsedDg;       // anzahl der benutzten shape Id's im Dg
        UINT32                  mnTotalShapesDgg;           // anzahl der shapes im Dgg
        UINT32                  mnCountOfs;
        UINT32                  mnSortCount;
        UINT32                  mnSortBufSize;
        _EscherPropSortStruct*  mpSortStruct;
        UINT32                  mnCountCount;
        UINT32                  mnCountSize;
        BOOL                    mb_EscherSpgr;
        BOOL                    mb_EscherDg;

        UINT32                  mnGroupLevel;

        BOOL    ImplSeek( UINT32 nKey );

        // ist die graphic noch nicht vorhanden, so wird sie eingefuegt

        UINT32  ImplDggContainerSize();
        void    ImplWriteDggContainer( SvStream& rSt );

        UINT32  ImplDggAtomSize();
        void    ImplWriteDggAtom( SvStream& rSt );

        UINT32  ImplBlibStoreContainerSize();
        void    ImplWriteBlibStoreContainer( SvStream& rSt );

        UINT32  ImplOptAtomSize();
        void    ImplWriteOptAtom( SvStream& rSt );

        UINT32  ImplSplitMenuColorsAtomSize();
        void    ImplWriteSplitMenuColorsAtom( SvStream& rSt );

    public:

                _EscherEx( SvStorageStream& rOut, UINT32 nDrawings );
                ~_EscherEx();

        void    InsertAtCurrentPos( UINT32 nBytes, BOOL bCont = FALSE );// es werden nBytes an der aktuellen Stream Position eingefuegt,
                                                                    // die PersistantTable und interne Zeiger angepasst

        void    InsertPersistOffset( UINT32 nKey, UINT32 nOffset ); // Es wird nicht geprueft, ob sich jener schluessel schon in der PersistantTable befindet
        BOOL    SeekToPersistOffset( UINT32 nKey );
        BOOL    InsertAtPersistOffset( UINT32 nKey, UINT32 nValue );// nValue wird im Stream an entrsprechender Stelle eingefuegt(overwrite modus), ohne dass sich die
                                                                    // aktuelle StreamPosition aendert

        BOOL    SeekBehindRecHeader( UINT16 nRecType );             // der stream muss vor einem gueltigen Record Header oder Atom stehen

                // features beim erzeugen folgender Container:
                //
                //      _Escher_DggContainer:   ein _EscherDgg Atom wird automatisch erzeugt und verwaltet
                //      _Escher_DgContainer:        ein _EscherDg Atom wird automatisch erzeugt und verwaltet
                //      _Escher_SpgrContainer:
                //      _Escher_SpContainer:

        void    OpenContainer( UINT16 n_EscherContainer, int nRecInstance = 0 );
        void    CloseContainer();

        void    BeginAtom();
        void    EndAtom( UINT16 nRecType, int nRecVersion = 0, int nRecInstance = 0 );
        void    AddAtom( UINT32 nAtomSitze, UINT16 nRecType, int nRecVersion = 0, int nRecInstance = 0 );
        void    AddClientAnchor( const Rectangle& rRectangle );

        void    EnterGroup( Rectangle* pBoundRect = NULL );
        UINT32  GetGroupLevel() const { return mnGroupLevel; };
        BOOL    SetGroupSnapRect( UINT32 nGroupLevel, const Rectangle& rRect );
        BOOL    SetGroupLogicRect( UINT32 nGroupLevel, const Rectangle& rRect );
        void    LeaveGroup();

                // ein _Escher_Sp wird geschrieben ( Ein _Escher_DgContainer muss dazu geoeffnet sein !!)
        void    AddShape( UINT32 nShpInstance, UINT32 nFlagIds, UINT32 nShapeID = 0 );
                // reserviert eine ShapeId
        UINT32  GetShapeID();

        void    BeginCount();
        void    AddOpt( UINT16 nPropertyID, UINT32 nPropValue, BOOL bBlib = FALSE );
                // der Buffer pProp wird spaeter bei einem EndCount automatisch freigegeben!!
        BOOL    GetOpt( UINT16 nPropertyID, UINT32& rPropValue );
        void    AddOpt( UINT16 nPropertyID, BOOL bBlib, UINT32 nPropValue, BYTE* pProp, UINT32 nPropSize );
        void    AddColor( UINT32 nColor );
        void    EndCount( UINT16 nRecType, UINT16 nRecVersion = 0 );

        UINT32  GetColor( const UINT32 nColor, BOOL bSwap = TRUE );
        UINT32  GetColor( const Color& rColor, BOOL bSwap = TRUE );
        UINT32  GetGradientColor( const ::com::sun::star::awt::Gradient* pGradient, UINT32 bStartColor );
        void    WriteGradient( const ::com::sun::star::awt::Gradient* pGradient );
        UINT32  AddGraphic( SvStorageStream& rPicStrm, const Graphic& rGraphic );
        UINT32  AddGraphic( SvStorageStream& rPicStrm, const BYTE* pSource,
                    UINT32 nSize, const Rectangle&, _Escher_BlibType eBlipType );

        UINT32  DrawingGroupContainerSize();
        void    WriteDrawingGroupContainer( SvStream& rSt );
};


#endif
