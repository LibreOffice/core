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

#ifndef INCLUDED_SVX_MSDFFDEF_HXX
#define INCLUDED_SVX_MSDFFDEF_HXX

#include <limits.h>
#include <sal/types.h>
#include <svx/svxdllapi.h>

#define DFF_COMMON_RECORD_HEADER_SIZE           8

const sal_uInt32 nMaxLegalDffRecordLength =
    SAL_MAX_UINT32 - DFF_COMMON_RECORD_HEADER_SIZE;

#define DFF_PSFLAG_CONTAINER 0x0F         // If the version field of a record
                                          //  header takes on this value, the
                                          //  record header marks the start of
                                          //  a container.

#define DFF_PST_TextHeaderAtom              3999
#define DFF_PST_TextCharsAtom               4000
#define DFF_PST_TextBytesAtom               4008
#define DFF_PST_ExOleObjStg                 4113

//      Record Name                 FBT-Value Instance                  Contents                                                          Wrd Exl Ppt Ver

#define DFF_msofbtDggContainer      0xF000 //                           per-document data                                                  X   X   X
#define   DFF_msofbtDgg             0xF006 //                           an FDGG and several FIDCLs                                         X   X   X   0
#define   DFF_msofbtCLSID           0xF016 //                           the CLSID of the application that put the data on the clipboard    C   C   C   0
#define   DFF_msofbtOPT             0xF00B // count of properties       the document-wide default shape properties                         X   X   X   3
#define   DFF_msofbtColorMRU        0xF11A // count of colors           the colors in the MRU swatch                                       X   X   X   0
#define   DFF_msofbtSplitMenuColors 0xF11E // count of colors           the colors in the top-level split menus                            X   X   X   0
#define   DFF_msofbtBstoreContainer 0xF001 // count of BLIPs            all images in the document (JPEGs, metafiles, etc.)                X   X   X
#define     DFF_msofbtBSE           0xF007 // BLIP type                 an FBSE (one per BLIP)                                             X   X   X   2
#define     DFF_msofbtBlipFirst     0xF018 //                           range of fbts reserved for various kinds of BLIPs                  X   X   X
#define     DFF_msofbtBlipLast      0xF117 //                           range of fbts reserved for various kinds of BLIPs                  X   X   X

#define DFF_msofbtDgContainer       0xF002 //                           per-sheet/page/slide data                                          X   X   X
#define   DFF_msofbtDg              0xF008 // drawing ID                an FDG                                                             X   X   X   0
#define   DFF_msofbtRegroupItems    0xF118 // count of regroup entries  several FRITs                                                      X   X   X   0
#define   DFF_msofbtColorScheme     0xF120 // count of colors           the colors of the source host's color scheme                           C   C   0
#define   DFF_msofbtSpgrContainer   0xF003 //                           several SpContainers, the first of which is the group shape itself X   X   X
#define   DFF_msofbtSpContainer     0xF004 //                           a shape                                                            X   X   X
#define     DFF_msofbtSpgr          0xF009 //                           an FSPGR; only present if the shape is a group shape               X   X   X   1
#define     DFF_msofbtSp            0xF00A // shape type                an FSP                                                             X   X   X   2
#define     DFF_msofbtTextbox       0xF00C //                           RTF text                                                           C   C   C   0
#define     DFF_msofbtClientTextbox 0xF00D // host-defined              the text in the textbox, in host-defined format                    X   X   X
#define     DFF_msofbtAnchor        0xF00E //                           a RECT, in 100000ths of an inch                                    C   C   C   0
#define     DFF_msofbtChildAnchor   0xF00F //                           a RECT, in units relative to the parent group                      X   X   X   0
#define     DFF_msofbtClientAnchor  0xF010 // host-defined              the location of the shape, in a host-defined format                X   X   X
#define     DFF_msofbtClientData    0xF011 // host-defined              host-specific data                                                 X   X   X
#define     DFF_msofbtOleObject     0xF11F //                           a serialized IStorage for an OLE object                            C   C   C   0
#define     DFF_msofbtDeletedPspl   0xF11D //                           an FPSPL; only present in top-level deleted shapes                 X           0
#define     DFF_msofbtUDefProp      0xF122 //                           range of fbts reserved for various kinds of BLIPs                  X   X   X
#define   DFF_msofbtSolverContainer 0xF005 // count of rules            the rules governing shapes                                         X   X   X
#define     DFF_msofbtConnectorRule 0xF012 //                           an FConnectorRule                                                      X   X   1
#define     DFF_msofbtAlignRule     0xF013 //                           an FAlignRule                                                      X   X   X   0
#define     DFF_msofbtArcRule       0xF014 //                           an FARCRU                                                          X   X   X   0
#define     DFF_msofbtClientRule    0xF015 // host-defined              host-defined
#define     DFF_msofbtCalloutRule   0xF017 //                           an FCORU                                                           X   X   X   0


// Values for the ULONG in DFF_PST_TextHeaderAtom
enum DFF_TextHeader {
    DFFTH_TITLE,
    DFFTH_BODY,
    DFFTH_NOTES,
    DFFTH_NOTUSED,
    DFFTH_OTHER,       // Text in a Shape
    DFFTH_CENTERBODY,  // Subtitle in Title-Slide
    DFFTH_CENTERTITLE, // Title in Title-Slide
    DFFTH_HALFBODY,    // Body in two-column slide
    DFFTH_QUARTERBODY  // Body in four-body slide
};


// Shape Properties
// 1pt = 12700 EMU (English Metric Units)
// 1pt = 20 Twip = 20/1440" = 1/72"
// 1twip=635 EMU
// 1" = 12700*72 = 914400 EMU
// 1" = 25.4mm
// 1mm = 36000 EMU


//      Record Name                     FBT-Value Instance                  Contents                                                          Wrd Exl Ppt Ver
// Transform
#define DFF_Prop_Rotation                      4  //  Fixed Point 16.16 degrees
// Protection
#define DFF_Prop_LockAgainstGrouping         127  //  sal_Bool              Do not group this shape
// Text
#define DFF_Prop_lTxid                       128  //  LONG                  id for the text, value determined by the host
#define DFF_Prop_dxTextLeft                  129  //  LONG                  margins relative to shape's inscribed text rectangle (in EMUs)
#define DFF_Prop_dyTextTop                   130  //  LONG
#define DFF_Prop_dxTextRight                 131  //  LONG
#define DFF_Prop_dyTextBottom                132  //  LONG
#define DFF_Prop_WrapText                    133  //  MSO_WRAPMODE          Wrap text at shape margins
#define DFF_Prop_anchorText                  135  //  MSO_ANCHOR            How to anchor the text
#define DFF_Prop_txflTextFlow                136  //  MSO_TXFL              Text flow
#define DFF_Prop_cdirFont                    137  //  MSO_CDIR              Font rotation
#define DFF_Prop_hspNext                     138  //  MSO_HSP               ID of the next shape (used by Word for linked textboxes)
#define DFF_Prop_AutoTextMargin              188  //  sal_Bool              use host's margin calculations
#define DFF_Prop_RotateText                  189  //  sal_Bool              Rotate text with shape
#define DFF_Prop_FitTextToShape              191  //  sal_Bool              Size text to fit shape size
// GeoText
#define DFF_Prop_gtextUNICODE                192  //  WCHAR*                UNICODE text string
#define DFF_Prop_gtextAlign                  194  //  MSO_GEOTEXTALIGN      alignment on curve
#define DFF_Prop_gtextSize                   195  //  LONG                  default point size
#define DFF_Prop_gtextSpacing                196  //  LONG                  fixed point 16.16
#define DFF_Prop_gtextFont                   197  //  WCHAR*                font family name
#define DFF_Prop_gtextFStretch               245  //  sal_Bool   400        Stretch to fit shape
#define DFF_Prop_gtextFBold                  250  //  sal_Bool    20        Bold font
#define DFF_Prop_gtextFItalic                251  //  sal_Bool    10        Italic font
#define DFF_Prop_gtextFStrikethrough         255  //  sal_Bool     1        Strike through font
// Blip
#define DFF_Prop_cropFromTop                 256  //  LONG                  16.16 fraction times total image width or height, as appropriate.
#define DFF_Prop_cropFromBottom              257  //  LONG
#define DFF_Prop_cropFromLeft                258  //  LONG
#define DFF_Prop_cropFromRight               259  //  LONG
#define DFF_Prop_pib                         260  //  IMsoBlip*             Blip to display
#define DFF_Prop_pibName                     261  //  WCHAR*                Blip file name
#define DFF_Prop_pibFlags                    262  //  MSO_BLIPFLAGS         Blip flags
#define DFF_Prop_pictureTransparent          263  //  LONG                  transparent color (none if ~0UL)
#define DFF_Prop_pictureContrast             264  //  LONG                  contrast setting
#define DFF_Prop_pictureBrightness           265  //  LONG                  brightness setting
#define DFF_Prop_pictureGamma                266  //  LONG                  16.16 gamma
#define DFF_Prop_pictureId                   267  //  LONG                  Host-defined ID for OLE objects (usually a pointer)
#define DFF_Prop_pictureActive               319  //  sal_Bool              Server is active (OLE objects only)
// Geometry
#define DFF_Prop_geoLeft                     320  //  LONG                  Defines the G (geometry) coordinate space.
#define DFF_Prop_geoTop                      321  //  LONG
#define DFF_Prop_geoRight                    322  //  LONG
#define DFF_Prop_geoBottom                   323  //  LONG
#define DFF_Prop_pVertices                   325  //  IMsoArray             An array of points, in G units.
#define DFF_Prop_pSegmentInfo                326  //  IMsoArray
#define DFF_Prop_adjustValue                 327  //  LONG                  Adjustment values corresponding to the positions of the
#define DFF_Prop_adjust2Value                328  //  LONG                  adjust handles of the shape. The number of values used
#define DFF_Prop_adjust3Value                329  //  LONG                  and their allowable ranges vary from shape type to shape type.
#define DFF_Prop_adjust4Value                330  //  LONG
#define DFF_Prop_adjust5Value                331  //  LONG
#define DFF_Prop_adjust6Value                332  //  LONG
#define DFF_Prop_adjust7Value                333  //  LONG
#define DFF_Prop_adjust8Value                334  //  LONG
#define DFF_Prop_adjust9Value                335  //  LONG
#define DFF_Prop_adjust10Value               336  //  LONG
#define DFF_Prop_connectorPoints             337  //  IMsoArray
#define DFF_Prop_stretchPointX               339  //  LONG
#define DFF_Prop_stretchPointY               340  //  LONG
#define DFF_Prop_Handles                     341  //  H*
#define DFF_Prop_pFormulas                   342  //  LONG
#define DFF_Prop_textRectangles              343  //  LONG
#define DFF_Prop_connectorType               344  //  LONG                  ->0=none, 1=segments, 2=custom, 3=rect
#define DFF_Prop_f3DOK                       379  //  sal_Bool              3D may be set
#define DFF_Prop_fGtextOK                    381  //  sal_Bool              Text effect (FontWork) supported
#define DFF_Prop_fFillShadeShapeOK           382  //  BOOL
#define DFF_Prop_fFillOK                     383  //  sal_Bool              OK to fill the shape through the UI or VBA?
// FillStyle
#define DFF_Prop_fillType                    384  //  MSO_FILLTYPE          Type of fill
#define DFF_Prop_fillColor                   385  //  MSO_CLR               Foreground color
#define DFF_Prop_fillOpacity                 386  //  LONG                  Fixed 16.16
#define DFF_Prop_fillBackColor               387  //  MSO_CLR               Background color
#define DFF_Prop_fillBackOpacity             388  //  LONG                  Shades only
#define DFF_Prop_fillBlip                    390  //  IMsoBlip*             Pattern/texture
#define DFF_Prop_fillWidth                   393  //  LONG                  How big (A units) to make a metafile texture.
#define DFF_Prop_fillHeight                  394  //  LONG
#define DFF_Prop_fillAngle                   395  //  LONG                  Fade angle - degrees in 16.16
#define DFF_Prop_fillFocus                   396  //  LONG                  Linear shaded fill focus percent
#define DFF_Prop_fillToRight                 399  //  LONG                  Fraction 16.16
#define DFF_Prop_fillToBottom                400  //  LONG                  Fraction 16.16
#define DFF_Prop_fillShadeColors             407  //  IMsoArray             a preset array of colors
#define DFF_Prop_fFilled                     443  //  sal_Bool              Is shape filled?
#define DFF_Prop_fNoFillHitTest              447  //  sal_Bool              Hit test a shape as though filled
// LineStyle
#define DFF_Prop_lineColor                   448  //  MSO_CLR               Color of line
#define DFF_Prop_lineOpacity                 449  //  LONG                  Not implemented
#define DFF_Prop_lineBackColor               450  //  MSO_CLR               Background color
#define DFF_Prop_lineWidth                   459  //  LONG                  A units; 1pt == 12700 EMUs
#define DFF_Prop_lineStyle                   461  //  MSO_LINESTYLE         Draw parallel lines?
#define DFF_Prop_lineDashing                 462  //  MSO_LINEDASHING       Can be overridden by:
#define DFF_Prop_lineDashStyle               463  //  IMsoArray             As Win32 ExtCreatePen
#define DFF_Prop_lineStartArrowhead          464  //  MSO_LINEEND           Arrow at start
#define DFF_Prop_lineEndArrowhead            465  //  MSO_LINEEND           Arrow at end
#define DFF_Prop_lineStartArrowWidth         466  //  MSO_LINEENDWIDTH      Arrow at start
#define DFF_Prop_lineStartArrowLength        467  //  MSO_LINEENDLENGTH     Arrow at end
#define DFF_Prop_lineEndArrowWidth           468  //  MSO_LINEENDWIDTH      Arrow at start
#define DFF_Prop_lineEndArrowLength          469  //  MSO_LINEENDLENGTH     Arrow at end
#define DFF_Prop_lineJoinStyle               470  //  MSO_LINEJOIN          How to join lines
#define DFF_Prop_lineEndCapStyle             471  //  MSO_LINECAP           How to end lines
#define DFF_Prop_fLine                       508  //  sal_Bool              Any line?
#define DFF_Prop_fNoLineDrawDash             511  //  sal_Bool              Draw a dashed line if no line
// ShadowStyle
#define DFF_Prop_shadowType                  512  //  MSO_SHADOWTYPE        Type of effect
#define DFF_Prop_shadowColor                 513  //  MSO_CLR               Foreground color
#define DFF_Prop_shadowOpacity               516  //  LONG                  Fixed 16.16
#define DFF_Prop_shadowOffsetX               517  //  LONG                  Offset shadow
#define DFF_Prop_shadowOffsetY               518  //  LONG                  Offset shadow
#define DFF_Prop_fshadowObscured             575  //  sal_Bool              Excel5-style shadow
// PerspectiveStyle
#define DFF_Prop_fPerspective                639  //  sal_Bool              On/off
// 3D Object
#define DFF_Prop_c3DSpecularAmt              640  //  LONG                  Fixed-point 16.16
#define DFF_Prop_c3DDiffuseAmt               641  //  LONG                  Fixed-point 16.16
#define DFF_Prop_c3DShininess                642  //  LONG                  Default gives OK results
#define DFF_Prop_c3DExtrudeForward           644  //  LONG                  Distance of extrusion in EMUs
#define DFF_Prop_c3DExtrudeBackward          645  //  LONG
#define DFF_Prop_c3DExtrusionColor           647  //  MSO_CLR               Basic color of extruded part of shape; the lighting model used will determine the exact shades used when rendering.
#define DFF_Prop_fc3DLightFace               703  //  BOOL
// 3D Style
#define DFF_Prop_c3DYRotationAngle           704  //  LONG                  degrees (16.16) about y axis
#define DFF_Prop_c3DXRotationAngle           705  //  LONG                  degrees (16.16) about x axis
#define DFF_Prop_c3DRotationCenterX          710  //  LONG                  rotation center x (16.16 or g-units)
#define DFF_Prop_c3DRotationCenterY          711  //  LONG                  rotation center y (16.16 or g-units)
#define DFF_Prop_c3DRotationCenterZ          712  //  LONG                  rotation center z (absolute (emus))
#define DFF_Prop_c3DRenderMode               713  //  MSO_3DRENDERMODE      Full,wireframe, or bcube
#define DFF_Prop_c3DTolerance                714  //  LONG                  pixels (16.16)
#define DFF_Prop_c3DXViewpoint               715  //  LONG                  X view point (emus)
#define DFF_Prop_c3DYViewpoint               716  //  LONG                  Y view point (emus)
#define DFF_Prop_c3DZViewpoint               717  //  LONG                  Z view distance (emus)
#define DFF_Prop_c3DOriginX                  718  //  LONG
#define DFF_Prop_c3DOriginY                  719  //  LONG
#define DFF_Prop_c3DSkewAngle                720  //  LONG                  degree (16.16) skew angle
#define DFF_Prop_c3DSkewAmount               721  //  LONG                  Percentage skew amount
#define DFF_Prop_c3DAmbientIntensity         722  //  LONG                  Fixed point intensity
#define DFF_Prop_c3DKeyX                     723  //  LONG                  Key light source direc-
#define DFF_Prop_c3DKeyY                     724  //  LONG                  tion; only their relative
#define DFF_Prop_c3DKeyZ                     725  //  LONG                  magnitudes matter
#define DFF_Prop_c3DKeyIntensity             726  //  LONG                  Fixed point intensity
#define DFF_Prop_c3DFillX                    727  //  LONG                  Fill light source direc-
#define DFF_Prop_c3DFillY                    728  //  LONG                  tion; only their relative
#define DFF_Prop_c3DFillZ                    729  //  LONG                  magnitudes matter
#define DFF_Prop_c3DFillIntensity            730  //  LONG                  Fixed point intensity
#define DFF_Prop_fc3DFillHarsh               767  //  sal_Bool              Is fill lighting harsh?
// Shape
#define DFF_Prop_hspMaster                   769  //  MSO_HSP               master shape
#define DFF_Prop_cxstyle                     771  //  MSO_CXSTYLE           Type of connector
#define DFF_Prop_fBackground                 831  //  sal_Bool              If sal_True, this is the background shape.

// Callout
#define DFF_Prop_fCalloutLengthSpecified     895  //  sal_Bool              if true, we look at dxyCalloutLengthSpecified

// GroupShape
#define DFF_Prop_wzName                      896  //  WCHAR*                Shape Name (present only if explicitly set)
#define DFF_Prop_wzDescription               897  //  WCHAR*                alternate text
#define DFF_Prop_pihlShape                   898  //  IHlink*               The hyperlink in the shape.
#define DFF_Prop_pWrapPolygonVertices        899  //  IMsoArray             The polygon that text will be wrapped around (Word)
#define DFF_Prop_dxWrapDistLeft              900  //  LONG                  Left wrapping distance from text (Word)
#define DFF_Prop_dyWrapDistTop               901  //  LONG                  Top wrapping distance from text (Word)
#define DFF_Prop_dxWrapDistRight             902  //  LONG                  Right wrapping distance from text (Word)
#define DFF_Prop_dyWrapDistBottom            903  //  LONG                  Bottom wrapping distance from text (Word)
#define DFF_Prop_tableProperties             927  //  LONG
#define DFF_Prop_tableRowProperties          928  //  LONG*
#define DFF_Prop_fHidden                     958  //  sal_Bool              Do not display
#define DFF_Prop_fPrint                      959  //  sal_Bool              Print this shape
// entsprechende BitFlags

// properties of the second property set
#define DFF_Prop_metroBlob                   937  // XML ZipPackage*

// linchpin: the shape type
enum MSO_SPT: sal_Int32 {
   mso_sptMin = 0,
   mso_sptNotPrimitive = mso_sptMin,
   mso_sptRectangle = 1,
   mso_sptRoundRectangle = 2,
   mso_sptEllipse = 3,
   mso_sptDiamond = 4,
   mso_sptIsocelesTriangle = 5,
   mso_sptRightTriangle = 6,
   mso_sptParallelogram = 7,
   mso_sptTrapezoid = 8,
   mso_sptHexagon = 9,
   mso_sptOctagon = 10,
   mso_sptPlus = 11,
   mso_sptStar = 12,
   mso_sptArrow = 13,
   mso_sptThickArrow = 14,
   mso_sptHomePlate = 15,
   mso_sptCube = 16,
   mso_sptBalloon = 17,
   mso_sptSeal = 18,
   mso_sptArc = 19,
   mso_sptLine = 20,
   mso_sptPlaque = 21,
   mso_sptCan = 22,
   mso_sptDonut = 23,
   mso_sptTextSimple = 24,
   mso_sptTextOctagon = 25,
   mso_sptTextHexagon = 26,
   mso_sptTextCurve = 27,
   mso_sptTextWave = 28,
   mso_sptTextRing = 29,
   mso_sptTextOnCurve = 30,
   mso_sptTextOnRing = 31,
   mso_sptStraightConnector1 = 32,
   mso_sptBentConnector2 = 33,
   mso_sptBentConnector3 = 34,
   mso_sptBentConnector4 = 35,
   mso_sptBentConnector5 = 36,
   mso_sptCurvedConnector2 = 37,
   mso_sptCurvedConnector3 = 38,
   mso_sptCurvedConnector4 = 39,
   mso_sptCurvedConnector5 = 40,
   mso_sptCallout1 = 41,
   mso_sptCallout2 = 42,
   mso_sptCallout3 = 43,
   mso_sptAccentCallout1 = 44,
   mso_sptAccentCallout2 = 45,
   mso_sptAccentCallout3 = 46,
   mso_sptBorderCallout1 = 47,
   mso_sptBorderCallout2 = 48,
   mso_sptBorderCallout3 = 49,
   mso_sptAccentBorderCallout1 = 50,
   mso_sptAccentBorderCallout2 = 51,
   mso_sptAccentBorderCallout3 = 52,
   mso_sptRibbon = 53,
   mso_sptRibbon2 = 54,
   mso_sptChevron = 55,
   mso_sptPentagon = 56,
   mso_sptNoSmoking = 57,
   mso_sptSeal8 = 58,
   mso_sptSeal16 = 59,
   mso_sptSeal32 = 60,
   mso_sptWedgeRectCallout = 61,
   mso_sptWedgeRRectCallout = 62,
   mso_sptWedgeEllipseCallout = 63,
   mso_sptWave = 64,
   mso_sptFoldedCorner = 65,
   mso_sptLeftArrow = 66,
   mso_sptDownArrow = 67,
   mso_sptUpArrow = 68,
   mso_sptLeftRightArrow = 69,
   mso_sptUpDownArrow = 70,
   mso_sptIrregularSeal1 = 71,
   mso_sptIrregularSeal2 = 72,
   mso_sptLightningBolt = 73,
   mso_sptHeart = 74,
   mso_sptPictureFrame = 75,
   mso_sptQuadArrow = 76,
   mso_sptLeftArrowCallout = 77,
   mso_sptRightArrowCallout = 78,
   mso_sptUpArrowCallout = 79,
   mso_sptDownArrowCallout = 80,
   mso_sptLeftRightArrowCallout = 81,
   mso_sptUpDownArrowCallout = 82,
   mso_sptQuadArrowCallout = 83,
   mso_sptBevel = 84,
   mso_sptLeftBracket = 85,
   mso_sptRightBracket = 86,
   mso_sptLeftBrace = 87,
   mso_sptRightBrace = 88,
   mso_sptLeftUpArrow = 89,
   mso_sptBentUpArrow = 90,
   mso_sptBentArrow = 91,
   mso_sptSeal24 = 92,
   mso_sptStripedRightArrow = 93,
   mso_sptNotchedRightArrow = 94,
   mso_sptBlockArc = 95,
   mso_sptSmileyFace = 96,
   mso_sptVerticalScroll = 97,
   mso_sptHorizontalScroll = 98,
   mso_sptCircularArrow = 99,
   mso_sptNotchedCircularArrow = 100,
   mso_sptUturnArrow = 101,
   mso_sptCurvedRightArrow = 102,
   mso_sptCurvedLeftArrow = 103,
   mso_sptCurvedUpArrow = 104,
   mso_sptCurvedDownArrow = 105,
   mso_sptCloudCallout = 106,
   mso_sptEllipseRibbon = 107,
   mso_sptEllipseRibbon2 = 108,
   mso_sptFlowChartProcess = 109,
   mso_sptFlowChartDecision = 110,
   mso_sptFlowChartInputOutput = 111,
   mso_sptFlowChartPredefinedProcess = 112,
   mso_sptFlowChartInternalStorage = 113,
   mso_sptFlowChartDocument = 114,
   mso_sptFlowChartMultidocument = 115,
   mso_sptFlowChartTerminator = 116,
   mso_sptFlowChartPreparation = 117,
   mso_sptFlowChartManualInput = 118,
   mso_sptFlowChartManualOperation = 119,
   mso_sptFlowChartConnector = 120,
   mso_sptFlowChartPunchedCard = 121,
   mso_sptFlowChartPunchedTape = 122,
   mso_sptFlowChartSummingJunction = 123,
   mso_sptFlowChartOr = 124,
   mso_sptFlowChartCollate = 125,
   mso_sptFlowChartSort = 126,
   mso_sptFlowChartExtract = 127,
   mso_sptFlowChartMerge = 128,
   mso_sptFlowChartOfflineStorage = 129,
   mso_sptFlowChartOnlineStorage = 130,
   mso_sptFlowChartMagneticTape = 131,
   mso_sptFlowChartMagneticDisk = 132,
   mso_sptFlowChartMagneticDrum = 133,
   mso_sptFlowChartDisplay = 134,
   mso_sptFlowChartDelay = 135,
   mso_sptTextPlainText = 136,
   mso_sptTextStop = 137,
   mso_sptTextTriangle = 138,
   mso_sptTextTriangleInverted = 139,
   mso_sptTextChevron = 140,
   mso_sptTextChevronInverted = 141,
   mso_sptTextRingInside = 142,
   mso_sptTextRingOutside = 143,
   mso_sptTextArchUpCurve = 144,
   mso_sptTextArchDownCurve = 145,
   mso_sptTextCircleCurve = 146,
   mso_sptTextButtonCurve = 147,
   mso_sptTextArchUpPour = 148,
   mso_sptTextArchDownPour = 149,
   mso_sptTextCirclePour = 150,
   mso_sptTextButtonPour = 151,
   mso_sptTextCurveUp = 152,
   mso_sptTextCurveDown = 153,
   mso_sptTextCascadeUp = 154,
   mso_sptTextCascadeDown = 155,
   mso_sptTextWave1 = 156,
   mso_sptTextWave2 = 157,
   mso_sptTextWave3 = 158,
   mso_sptTextWave4 = 159,
   mso_sptTextInflate = 160,
   mso_sptTextDeflate = 161,
   mso_sptTextInflateBottom = 162,
   mso_sptTextDeflateBottom = 163,
   mso_sptTextInflateTop = 164,
   mso_sptTextDeflateTop = 165,
   mso_sptTextDeflateInflate = 166,
   mso_sptTextDeflateInflateDeflate = 167,
   mso_sptTextFadeRight = 168,
   mso_sptTextFadeLeft = 169,
   mso_sptTextFadeUp = 170,
   mso_sptTextFadeDown = 171,
   mso_sptTextSlantUp = 172,
   mso_sptTextSlantDown = 173,
   mso_sptTextCanUp = 174,
   mso_sptTextCanDown = 175,
   mso_sptFlowChartAlternateProcess = 176,
   mso_sptFlowChartOffpageConnector = 177,
   mso_sptCallout90 = 178,
   mso_sptAccentCallout90 = 179,
   mso_sptBorderCallout90 = 180,
   mso_sptAccentBorderCallout90 = 181,
   mso_sptLeftRightUpArrow = 182,
   mso_sptSun = 183,
   mso_sptMoon = 184,
   mso_sptBracketPair = 185,
   mso_sptBracePair = 186,
   mso_sptSeal4 = 187,
   mso_sptDoubleWave = 188,
   mso_sptActionButtonBlank = 189,
   mso_sptActionButtonHome = 190,
   mso_sptActionButtonHelp = 191,
   mso_sptActionButtonInformation = 192,
   mso_sptActionButtonForwardNext = 193,
   mso_sptActionButtonBackPrevious = 194,
   mso_sptActionButtonEnd = 195,
   mso_sptActionButtonBeginning = 196,
   mso_sptActionButtonReturn = 197,
   mso_sptActionButtonDocument = 198,
   mso_sptActionButtonSound = 199,
   mso_sptActionButtonMovie = 200,
   mso_sptHostControl = 201,
   mso_sptTextBox = 202,
   //for pptx shape which doesn't exist in ppt
   mso_sptTearDrop = 203,
   mso_sptMax = 0x0FFF,
   mso_sptNil = mso_sptMax
};

// different enums from the OfficeDraw documentation, Appendix D - beginning

// note: in opposite to the MS documentation, they all
// start with "mso_" instead of "mso"

enum MSO_ShapePath {
   mso_shapeLines,        // A line of straight segments
   mso_shapeLinesClosed,  // A closed polygonal object
   mso_shapeCurves,       // A line of Bezier curve segments
   mso_shapeCurvesClosed, // A closed shape with curved edges
   mso_shapeComplex       // pSegmentInfo must be non-empty
};

enum MSO_WrapMode {
   mso_wrapSquare,
   mso_wrapByPoints,
   mso_wrapNone,
   mso_wrapTopBottom,
   mso_wrapThrough
};

enum MSO_BWMode {
   mso_bwColor,          // only used for predefined shades
   mso_bwAutomatic,      // depends on object type
   mso_bwGrayScale,      // shades of gray only
   mso_bwLightGrayScale, // shades of light gray only
   mso_bwInverseGray,    // dark gray mapped to light gray, etc.
   mso_bwGrayOutline,    // pure gray and white
   mso_bwBlackTextLine,  // black text and lines, all else grayscale
   mso_bwHighContrast,   // pure black and white mode (no grays)
   mso_bwBlack,          // solid black
   mso_bwWhite,          // solid white
   mso_bwDontShow,       // object not drawn
   mso_bwNumModes        // number of Black and white modes
};

enum MSO_Anchor {
   mso_anchorTop,
   mso_anchorMiddle,
   mso_anchorBottom,
   mso_anchorTopCentered,
   mso_anchorMiddleCentered,
   mso_anchorBottomCentered,
   mso_anchorTopBaseline,
   mso_anchorBottomBaseline,
   mso_anchorTopCenteredBaseline,
   mso_anchorBottomCenteredBaseline
};

enum MSO_CDir {
   mso_cdir0,       // Right
   mso_cdir90,      // Down
   mso_cdir180,     // Left
   mso_cdir270      // Up
};

// connector style
enum MSO_ConnectorStyle {
   mso_cxstyleStraight = 0,
   mso_cxstyleBent,
   mso_cxstyleCurved,
   mso_cxstyleNone
};

// MSO_TXFL -- text flow
enum MSO_TextFlow {
   mso_txflHorzN,           // Horizontal non-@
   mso_txflTtoBA,           // Top to Bottom @-font
   mso_txflBtoT,            // Bottom to Top non-@
   mso_txflTtoBN,           // Top to Bottom non-@
   mso_txflHorzA,           // Horizontal @-font
   mso_txflVertN            // Vertical, non-@
};

// MSO_TXDIR - text direction (needed for Bi-Di support)
enum MSO_TextDir {
   mso_txdirLTR,            // left-to-right text direction
   mso_txdirRTL,            // right-to-left text direction
   mso_txdirContext         // context text direction
};

// MSO_SPCOT -- Callout Type
enum MSO_SPCalloutType {
   mso_spcotRightAngle = 1,
   mso_spcotOneSegment = 2,
   mso_spcotTwoSegment = 3,
   mso_spcotThreeSegment = 4
};

// MSO_SPCOA -- Callout Angle
enum MSO_SPCalloutAngle {
   mso_spcoaAny,
   mso_spcoa30,
   mso_spcoa45,
   mso_spcoa60,
   mso_spcoa90,
   mso_spcoa0
};

// MSO_SPCOD -- Callout Drop
enum MSO_SPCalloutDrop {
   mso_spcodTop,
   mso_spcodCenter,
   mso_spcodBottom,
   mso_spcodSpecified
};

// MSO_GEOTEXTALIGN - FontWork alignment
enum MSO_GeoTextAlign {
   mso_alignTextStretch,      /* Stretch each line of text to fit width. */
   mso_alignTextCenter,       /* Center text on width. */
   mso_alignTextLeft,         /* Left justify. */
   mso_alignTextRight,        /* Right justify. */
   mso_alignTextLetterJust,   /* Spread letters out to fit width. */
   mso_alignTextWordJust,     /* Spread words out to fit width. */
   mso_alignTextInvalid       /* Invalid */
};

// MSO_BLIPFLAGS - flags for pictures
enum MSO_BlipFlags {
   mso_blipflagDefault = 0,
   mso_blipflagComment = 0,   // Blip name is a comment
   mso_blipflagFile,          // Blip name is a file name
   mso_blipflagURL,           // Blip name is a full URL
   mso_blipflagType = 3,      // Mask to extract type
   /* Or the following flags with any of the above. */
   mso_blipflagDontSave = 4,  // A "dont" is the depression in the metal
                              // body work of an automobile caused when a
                              // cyclist violently thrusts his or her nose
                              // at it, thus a DontSave is another name for
                              // a cycle lane.
   mso_blipflagDoNotSave = 4, // For those who prefer English
   mso_blipflagLinkToFile = 8
};

enum MSO_3DRenderMode {
   mso_FullRender,      // Generate a full rendering
   mso_Wireframe,       // Generate a wireframe
   mso_BoundingCube     // Generate a bounding cube
};

enum MSO_XFormType {
   mso_xformAbsolute,   // Apply transform in absolute space centered on shape
   mso_xformShape,      // Apply transform to shape geometry
   mso_xformDrawing     // Apply transform in drawing space
};

enum MSO_ShadowType {
   mso_shadowOffset,    // N pixel offset shadow
   mso_shadowDouble,    // Use second offset too
   mso_shadowRich,      // Rich perspective shadow (cast relative to shape)
   mso_shadowShape,     // Rich perspective shadow (cast in shape space)
   mso_shadowDrawing,   // Perspective shadow cast in drawing space
   mso_shadowEmbossOrEngrave
};

// MSO_DZTYPE - the type of a (length) measurement
enum MSO_DZType {
   mso_dztypeMin          = 0,
   mso_dztypeDefault      = 0,  // Default size, ignore the values
   mso_dztypeA            = 1,  // Values are in EMUs
   mso_dztypeV            = 2,  // Values are in pixels
   mso_dztypeShape        = 3,  // Values are 16.16 fractions of shape size
   mso_dztypeFixedAspect  = 4,  // Aspect ratio is fixed
   mso_dztypeAFixed       = 5,  // EMUs, fixed aspect ratio
   mso_dztypeVFixed       = 6,  // Pixels, fixed aspect ratio
   mso_dztypeShapeFixed   = 7,  // Proportion of shape, fixed aspect ratio
   mso_dztypeFixedAspectEnlarge
                         = 8,   // Aspect ratio is fixed, favor larger size
   mso_dztypeAFixedBig    = 9,  // EMUs, fixed aspect ratio
   mso_dztypeVFixedBig    = 10, // Pixels, fixed aspect ratio
   mso_dztypeShapeFixedBig= 11, // Proportion of shape, fixed aspect ratio
   mso_dztypeMax          = 11
};

enum MSO_FillType {
   mso_fillSolid,             // Fill with a solid color
   mso_fillPattern,           // Fill with a pattern (bitmap)
   mso_fillTexture,           // A texture (pattern with its own color map)
   mso_fillPicture,           // Center a picture in the shape
   mso_fillShade,             // Shade from start to end points
   mso_fillShadeCenter,       // Shade from bounding rectangle to end point
   mso_fillShadeShape,        // Shade from shape outline to end point
   mso_fillShadeScale,        // Similar to mso_fillShade, but the fillAngle
                              // is additionally scaled by the aspect ratio of
                              // the shape. If shape is square, it is the
                              // same as mso_fillShade.
   mso_fillShadeTitle,        // special type - shade to title ---  for PP
   mso_fillBackground         // Use the background fill color/pattern
};

// MSOLINESTYLE - compound line style
enum MSO_LineStyle {
   mso_lineSimple,          // Single line (of width lineWidth)
   mso_lineDouble,          // Double lines of equal width
   mso_lineThickThin,       // Double lines, one thick, one thin
   mso_lineThinThick,       // Double lines, reverse order
   mso_lineTriple,          // Three lines, thin, thick, thin
   MSO_LineStyle_NONE = USHRT_MAX
};

// MSO_LINETYPE - how to "fill" the line contour
enum MSO_LineType {
   mso_lineSolidType,         // Fill with a solid color
   mso_linePattern,           // Fill with a pattern (bitmap)
   mso_lineTexture,           // A texture (pattern with its own color map)
   mso_linePicture            // Center a picture in the shape
};

// MSO_LINEDASHING - dashed line style
enum MSO_LineDashing {
   mso_lineSolid,              // Solid (continuous) pen
   mso_lineDashSys,            // PS_DASH system   dash style
   mso_lineDotSys,             // PS_DOT system   dash style
   mso_lineDashDotSys,         // PS_DASHDOT system dash style
   mso_lineDashDotDotSys,      // PS_DASHDOTDOT system dash style
   mso_lineDotGEL,             // square dot style
   mso_lineDashGEL,            // dash style
   mso_lineLongDashGEL,        // long dash style
   mso_lineDashDotGEL,         // dash short dash
   mso_lineLongDashDotGEL,     // long dash short dash
   mso_lineLongDashDotDotGEL   // long dash short dash short dash
};

// MSO_LINEEND - line end effect
enum MSO_LineEnd {
   mso_lineNoEnd,
   mso_lineArrowEnd,
   mso_lineArrowStealthEnd,
   mso_lineArrowDiamondEnd,
   mso_lineArrowOvalEnd,
   mso_lineArrowOpenEnd
};

// MSO_LINEENDWIDTH - size of arrowhead
enum MSO_LineEndWidth {
   mso_lineNarrowArrow,
   mso_lineMediumWidthArrow,
   mso_lineWideArrow
};

// MSO_LINEENDLENGTH - size of arrowhead
enum MSO_LineEndLength {
   mso_lineShortArrow,
   mso_lineMediumLenArrow,
   mso_lineLongArrow
};

// MSO_LINEJOIN - line join style.
enum MSO_LineJoin {
   mso_lineJoinBevel,     // Join edges by a straight line
   mso_lineJoinMiter,     // Extend edges until they join
   mso_lineJoinRound      // Draw an arc between the two edges
};

// MSO_LINECAP - line cap style (applies to ends of dash segments too).
enum MSO_LineCap {
   mso_lineEndCapRound,   // Rounded ends - the default
   mso_lineEndCapSquare,  // Square protrudes by half line width
   mso_lineEndCapFlat     // Line ends at end point
};
// Various enums from the OfficeDraw documentation Appendix D - End

// BStore-Container


enum MSO_BLIPUSAGE {
   mso_blipUsageDefault,  // All non-texture fill blips get this.
   mso_blipUsageTexture,
   mso_blipUsageMax = 255 // Since this is stored in a byte
};

enum MSO_BLIPTYPE {            // GEL provided types...
   mso_blipERROR = 0,          // An error occurred during loading
   mso_blipUNKNOWN,            // An unknown blip type
   mso_blipEMF,                // Windows Enhanced Metafile
   mso_blipWMF,                // Windows Metafile
   mso_blipPICT,               // Macintosh PICT
   mso_blipJPEG,               // JFIF
   mso_blipPNG,                // PNG
   mso_blipDIB,                // Windows DIB
   mso_blipFirstClient = 32,   // First client defined blip type
   mso_blipLastClient  = 255   // Last client defined blip type
};

enum MSO_BI {
   mso_biUNKNOWN = 0,
   mso_biWMF  = 0x216,      // Metafile header then compressed WMF
   mso_biEMF  = 0x3D4,      // Metafile header then compressed EMF
   mso_biPICT = 0x542,      // Metafile header then compressed PICT
   mso_biPNG  = 0x6E0,      // One byte tag then PNG data
   mso_biJFIF = 0x46A,      // One byte tag then JFIF data
   mso_biJPEG = mso_biJFIF,
   mso_biDIB  = 0x7A8,      // One byte tag then DIB data
   mso_biClient=0x800       // Clients should set this bit
};                          // Blip signature as encoded in the MSO_FBH.inst

enum MSO_BLIPCOMPRESSION {
   mso_compressionDeflate = 0,
   mso_compressionNone = 254,    // Used only if compression fails
   mso_compressionTest = 255     // For testing only
};

enum MSO_BLIPFILTER {
   mso_filterAdaptive = 0,       // PNG type - not used/supported for metafile
   mso_filterNone = 254,
   mso_filterTest = 255          // For testing only
};

enum MSO_SYSCOLORINDEX {
    mso_syscolorButtonFace,             // COLOR_BTNFACE
    mso_syscolorWindowText,             // COLOR_WINDOWTEXT
    mso_syscolorMenu,                   // COLOR_MENU
    mso_syscolorHighlight,              // COLOR_HIGHLIGHT
    mso_syscolorHighlightText,          // COLOR_HIGHLIGHTTEXT
    mso_syscolorCaptionText,            // COLOR_CAPTIONTEXT
    mso_syscolorActiveCaption,          // COLOR_ACTIVECAPTION
    mso_syscolorButtonHighlight,        // COLOR_BTNHIGHLIGHT
    mso_syscolorButtonShadow,           // COLOR_BTNSHADOW
    mso_syscolorButtonText,             // COLOR_BTNTEXT
    mso_syscolorGrayText,               // COLOR_GRAYTEXT
    mso_syscolorInactiveCaption,        // COLOR_INACTIVECAPTION
    mso_syscolorInactiveCaptionText,    // COLOR_INACTIVECAPTIONTEXT
    mso_syscolorInfoBackground,         // COLOR_INFOBK
    mso_syscolorInfoText,               // COLOR_INFOTEXT
    mso_syscolorMenuText,               // COLOR_MENUTEXT
    mso_syscolorScrollbar,              // COLOR_SCROLLBAR
    mso_syscolorWindow,                 // COLOR_WINDOW
    mso_syscolorWindowFrame,            // COLOR_WINDOWFRAME
    mso_syscolor3DLight,                // COLOR_3DLIGHT
    mso_syscolorMax,                    // Count of system colors

    mso_colorFillColor = 0xF0,          // Use the fillColor property
    mso_colorLineOrFillColor,           // Use the line color only if there is a line
    mso_colorLineColor,                 // Use the lineColor property
    mso_colorShadowColor,               // Use the shadow color
    mso_colorThis,                      // Use this color (only valid as described below)
    mso_colorFillBackColor,             // Use the fillBackColor property
    mso_colorLineBackColor,             // Use the lineBackColor property
    mso_colorFillThenLine,              // Use the fillColor unless no fill and line
    mso_colorIndexMask = 0xFF,          // Extract the color index

    mso_colorProcessMask      =0xFFFF00,// All the processing bits
    mso_colorModificationMask =0x0F00,  // Just the function
    mso_colorModFlagMask      =0xF000,  // Just the additional flags
    mso_colorDarken           =0x0100,  // Darken color by parameter/255
    mso_colorLighten          =0x0200,  // Lighten color by parameter/255
    mso_colorAdd              =0x0300,  // Add grey level RGB(param,param,param)
    mso_colorSubtract         =0x0400,  // Subtract grey level RGB(p,p,p)
    mso_colorReverseSubtract  =0x0500,  // Subtract from grey level RGB(p,p,p)
    /* In the following "black" means maximum component value, white minimum.
       The operation is per component, to guarantee white combine with
    mso_colorGray */
    mso_colorBlackWhite       =0x0600,  // Black if < uParam, else white (>=)
    mso_colorInvert           =0x2000,  // Invert color (at the *end*)
    mso_colorInvert128        =0x4000,  // Invert by toggling the top bit
    mso_colorGray             =0x8000,  // Make the color gray (before the above!)
    mso_colorBParamMask       =0xFF0000,// Parameter used as above
    mso_colorBParamShift = 16           // To extract the parameter value
};

enum MSO_TextGeometryProperties {
    use_gtextFBestFit   = 0x00000100,
    use_gtextFStretch   = 0x00000400,
    gtextFBestFit       = 0x01000000,
    gtextFStretch       = 0x04000000
};

//ALT_TXT_MSINTEROP
#define MSPROP_DESCRIPTION_MAX_LEN  4096

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
